
#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../app/appmain.h"
#include "../texture.h"
#include "compframes.h"
#include "procvid.h"
#include "estview.h"

// http://dranger.com/ffmpeg/tutorial01.html

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "out/frame%d.ppm", iFrame);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	for(y=0; y<height; y++)
		fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

	// Close file
	fclose(pFile);
}

void DrawLine(LoadedTex* ltex, Vec2i from, Vec2i to)
{
	float slope = (float)(to.y-from.y) / (float)(to.x-from.x);
	float dist = sqrtf( (to.x-from.x)*(to.x-from.x) + (to.y-from.y)*(to.y-from.y) );

	float x = from.x;
	float y = from.y;

	for(float i=0; i<=dist; i+=1)
	{
		if( x >= 0 && y >= 0 &&
			(int)x < ltex->sizex &&
			(int)y < ltex->sizey )
		{
			unsigned char* pix = &ltex->data[ ((int)x + (int)y * ltex->sizex) * ltex->channels ];

			//pix[2] = (dist - i) / dist * 255.0f;
			//pix[0] = (dist - i) / dist * 255.0f;
			//pix[1] = (dist - i) / dist * 255.0f;

			pix[2] = 255;
			pix[0] = 255;
			pix[1] = (dist - i) / dist * 255.0f;
		}

		y += (float)(to.y - from.y) / dist;
		x += (float)(to.x - from.x) / dist;
	}

}

bool ConfirmBigShift(LoadedTex* comptex, int fromx, int fromy, Vec2f pixshift)
{
	float dist = Magnitude(pixshift);

	float i = 1;

	Vec2f curr = Vec2f(fromx,fromy);

	float totaldot = 0;

	for(; i<=dist; i+=1.0f)
	{
		curr = Vec2f(fromx,fromy) + pixshift * i / dist;

		unsigned char* shiftvec8 = &comptex->data[ comptex->channels * ((int)curr.x + (int)curr.y*comptex->sizex) ];

		if( (!shiftvec8[0] && !shiftvec8[0]) )
			continue;

		Vec2f pixshift2 = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) );

		float dot = Dot(Normalize(pixshift), pixshift2);

		if(dot < 0.0f)
			return false;

		totaldot += dot;
	}

	if(totaldot < dist * dist * 0.9f)
		return false;

	return true;
}

void ProcFrame2(LoadedTex* comptex, LoadedTex* colortex)
{
	Vec2f avgmatchvec;

	Vec2f avgshift;

	//to get focal point center, 
	//Vec2f focalcen;

	//for each pixel, get dot product along a unit vector pointing to image center and average it to get in-out factor
	////for each pixel, get dot product along a unit vector pointing to focal point center and average it to get in-out factor
	float infac;

	//for each pixel, get average to get translation vector
	Vec2f translvec;

	//for each pixel, get cross product with respect to translvec, multiplying by sign of direction along translvec from image center to get the factor of expansion along one side (rotation indicator)
	float sideexpanfac = 0;

	float counted = 0;

	//translvec, infac...
	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			unsigned char* shiftvec8 = &comptex->data[ comptex->channels * (x + y*comptex->sizex) ];

			if( (!shiftvec8[0] && !shiftvec8[1]) || (shiftvec8[0] == 127 && shiftvec8[1] == 127) )
				continue;

			Vec2f pixshift = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) );

			translvec = ( translvec * counted + pixshift ) / (counted + 1.0f);
			avgshift = ( avgshift * counted + pixshift ) / (counted + 1.0f);

			Vec2f offcen = Vec2f(comptex->sizex/2, comptex->sizey/2) - Vec2f(x,y);
			Vec2f invec = Normalize(offcen);

			float pixinfac = Dot(pixshift, invec);

			infac = ( infac + pixinfac * counted ) / (counted + 1.0f);

			counted += 1.0f;
		}
	}

	sideexpanfac = 0;

	//sideexpanfac...
	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			unsigned char* shiftvec8 = &comptex->data[ comptex->channels * (x + y*comptex->sizex) ];

			if( (!shiftvec8[0] && !shiftvec8[1]) || (shiftvec8[0] == 127 && shiftvec8[1] == 127) )
				continue;

			if( Magnitude(translvec) <= 1 )
				continue;

			Vec2f pixshift = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) );

			Vec2f offcen = Vec2f(comptex->sizex/2, comptex->sizey/2) - Vec2f(x,y);
			Vec2f invec = Normalize(offcen);

			//plane = ax + by + c

			float planesidedist = - Dot(Vec2f(comptex->sizex/2, comptex->sizey/2), Normalize(translvec) );

			//distance along translation shift vector
			float pixsidedist = Dot( Vec2f(x,y)-Vec2f(comptex->sizex/2, comptex->sizey/2), Normalize(translvec) ) /* + planesidedist */;

			//normal of translation vec
			Vec2f expanvec = Cross( translvec );
			expanvec = Normalize(expanvec);

			//distance off translation shift vector line
			float expanplanedist = - Dot( Vec2f(comptex->sizex/2, comptex->sizey/2), Normalize(expanvec) );
			//distance along expansion vector of the pixel coordinate (+ = above translation vector line, - below translation vector line, if translation vector leftward)
			float pixcoorddistalongexpan = Dot( Vec2f(x,y)-Vec2f(comptex->sizex/2, comptex->sizey/2), Normalize(expanvec) ) /* + expanplanedist */;
			//distance along expansion vector of the shift vector (+ = going up, - going down, if translation vector leftward)
			float shiftdistalongexpan = Dot( pixshift, Normalize(expanvec) );

#if 0
			//pixel on left side of image center if translation shift is leftward
			//if(pixsidedist > 0)
			//left of image center and above expansion vec
			if(pixsidedist > 0 && pixcoorddistalongexpan <= 0)
			{
				sideexpanfac += (shiftdistalongexpan) / counted;
			}
			//left of image center and below expansion vec
			else if(pixsidedist > 0 && pixcoorddistalongexpan > 0)
			{
				sideexpanfac += (-shiftdistalongexpan) / counted;
			}
			//right of image center and below expansion vec
			else if(pixsidedist <= 0 && pixcoorddistalongexpan > 0)
			{
				sideexpanfac += (-shiftdistalongexpan) / counted;
			}
			//right of image center and above expansion vec
			else
			{
				sideexpanfac += (shiftdistalongexpan) / counted;
			}
#else
			if( pixcoorddistalongexpan > 0 )
				sideexpanfac += shiftdistalongexpan / counted;
			else if( pixcoorddistalongexpan < 0 )
				sideexpanfac += - shiftdistalongexpan / counted;
#endif

			//shiftvec8[0] = 127 + 127 * pixshift.x / MAXPIXSEARCH;
			//shiftvec8[1] = 127 + 127 * pixshift.y / MAXPIXSEARCH;

			//if(x % 10 == 0 && y % 10 == 0)
			//	DrawLine(&comptex, Vec2i(x,y), Vec2i(x,y)+Vec2i(pixshift.x,pixshift.y));
		}
	}

	/*
	clean up erroneous 'opposing' vectors
	when there's a big shift supposedly, all the pixels covered up in the shift should have 
	had a similar shift in the same direction. if the big shift was opposite to the covered
	shift pixels, unset the big shift pixel.
	*/
	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			unsigned char* shiftvec8 = &comptex->data[ comptex->channels * (x + y*comptex->sizex) ];

			if( (!shiftvec8[0] && !shiftvec8[1]) || (shiftvec8[0] == 127 && shiftvec8[1] == 127) )
				continue;

			Vec2f pixshift = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) );

			Vec2i from = Vec2i(x,y);
			Vec2i to = Vec2i(x,y) + Vec2i(pixshift.x,pixshift.y);

			float dist = Magnitude(pixshift);

			if(dist < 4)
				continue;

			if(!ConfirmBigShift(comptex, x, y, pixshift))
			{
				shiftvec8[0] = 127;
				shiftvec8[1] = 127;
				shiftvec8[0] = 0;
				shiftvec8[1] = 0;
			}

			//float dist = Magnitude(pixshift);

			//shiftvec8[0] = shiftvec8[1] = shiftvec8[2] = dist * 255 / Magnitude(Vec2f(MAXPIXSEARCH,MAXPIXSEARCH));
		}
	}

#if 1
	//set final colors
	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			unsigned char* shiftvec8 = &comptex->data[ comptex->channels * (x + y*comptex->sizex) ];

			if( (!shiftvec8[0] && !shiftvec8[1]) || (shiftvec8[0] == 127 && shiftvec8[1] == 127) )
				//if( (!shiftvec8[0] && !shiftvec8[1]) /* || (shiftvec8[0] == 127 && shiftvec8[1] == 127) */ )
					continue;

			Vec2f pixshift = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) );

			shiftvec8[0] = 127 + 127 * pixshift.x / MAXPIXSEARCH;
			shiftvec8[1] = 127 + 127 * pixshift.y / MAXPIXSEARCH;

			//float dist = Magnitude(pixshift);

			//shiftvec8[0] = shiftvec8[1] = shiftvec8[2] = dist * 255 / Magnitude(Vec2f(MAXPIXSEARCH,MAXPIXSEARCH));
		}
	}
#endif

#if 1
	//draw shift vector lines
	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			unsigned char* shiftvec8 = &comptex->data[ comptex->channels * (x + y*comptex->sizex) ];

			if( (!shiftvec8[0] && !shiftvec8[1]) || (shiftvec8[0] == 127 && shiftvec8[1] == 127) )
				continue;

			Vec2f pixshift = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) ) / 127.0f * MAXPIXSEARCH;

			if(x % 10 == 0 && y % 10 == 0)
				DrawLine(comptex, Vec2i(x,y), Vec2i(x,y)+Vec2i(pixshift.x,pixshift.y));
		}
	}
#endif

#if 1
	DrawLine(comptex, Vec2i(127,127), Vec2i(127,127) + Vec2i(translvec.x * 3, translvec.y * 3));
	DrawLine(comptex, Vec2i(255,255), Vec2i(255,255) + Vec2i(infac * 3, infac * 3));
	DrawLine(comptex, Vec2i(127,255), Vec2i(127,255) + Vec2i(sideexpanfac * 3, sideexpanfac * 3));
#endif
}

void CompareFrames(LoadedTex* prev, LoadedTex* curr, int fin, LoadedTex* outtex)
{	
	char szFilename[32];
	sprintf(szFilename, "out/frame%06d.png", fin);
	//sprintf(szFilename, "out/depthframe%06d.png", fin);

	LoadedTex comptex;

	AllocTex(&comptex, prev->sizex, prev->sizey, prev->channels);

#if 0
	Vec2f avgmatchvec;

	//to get focal point center, 
	Vec2f focalcen;

	//for each pixel, get dot product along a unit vector pointing to image center and average it to get in-out factor
	////for each pixel, get dot product along a unit vector pointing to focal point center and average it to get in-out factor
	float inoutfac;

	//for each pixel, get average to get translation vector
	Vec2f translvec;

	//for each pixel, get cross product with respect to translvec, multiplying by sign of direction along translvec from image center to get the factor of expansion along one side (rotation indicator)
	float expanfac;
#endif

	/*
	the colors RG depend on the surface normal vector. 
	really they are the shift vector of the pixel from frame to 
	frame, but surfaces facing in the same direction will have 
	similar RG's during rotation and translation.
	*/

	//shift picture element.. shixel

	//for(int pass=0; pass<3; pass++)
	{
		for(int x=0; x<prev->sizex-(QUADSZ-1); x++)
		{
			for(int y=0; y<prev->sizey-(QUADSZ-1); y++)
			{
				//compare 4 pixel quads
				unsigned char* comppix;

				comppix = &comptex.data[ ((x) + (y)*prev->sizex) * prev->channels ];

				comppix[0] = 0;
				comppix[1] = 0;
				comppix[2] = 0;

				int searchminx = imax(0, x-MAXPIXSEARCH/2);
				int searchmaxx = imin(prev->sizex-1 - QUADSZ, x+MAXPIXSEARCH/2);
				int searchminy = imax(0, y-MAXPIXSEARCH/2);
				int searchmaxy = imin(prev->sizey-1 - QUADSZ, y+MAXPIXSEARCH/2);

				unsigned char startrgb[QUADSZ*QUADSZ][3];
				unsigned int avgrgb[3] = {0,0,0};
				int contrast = 0;

				for(int quadx=0; quadx<QUADSZ; quadx++)
				{
					for(int quady=0; quady<QUADSZ; quady++)
					{
						int quadi = quadx+quady*QUADSZ;

						startrgb[quadi][0] = prev->data[ ((x+quadx) + (y+quady)*prev->sizex) * prev->channels + 0 ];
						startrgb[quadi][1] = prev->data[ ((x+quadx) + (y+quady)*prev->sizex) * prev->channels + 1 ];
						startrgb[quadi][2] = prev->data[ ((x+quadx) + (y+quady)*prev->sizex) * prev->channels + 2 ];

						avgrgb[0] += startrgb[quadi][0];
						avgrgb[1] += startrgb[quadi][1];
						avgrgb[2] += startrgb[quadi][2];
					}
				}

				avgrgb[0] /= QUADSZ*QUADSZ;
				avgrgb[1] /= QUADSZ*QUADSZ;
				avgrgb[2] /= QUADSZ*QUADSZ;

				contrast = 0;

				//try to subtract the most different colored pixels
				for(int quadx=0; quadx<QUADSZ; quadx++)
				{
					for(int quady=0; quady<QUADSZ; quady++)
					{
						int quadi = quadx+quady*QUADSZ;

						int maxpixcontrast = 0;

						for(int quadx2=0; quadx2<QUADSZ; quadx2++)
						{
							for(int quady2=0; quady2<QUADSZ; quady2++)
							{
								int quadi2 = quadx2+quady2*QUADSZ;

								int pixcontrast = 0;

								pixcontrast += iabs( (int)startrgb[quadi][0] - (int)startrgb[quadi2][0] );
								pixcontrast += iabs( (int)startrgb[quadi][1] - (int)startrgb[quadi2][0] );
								pixcontrast += iabs( (int)startrgb[quadi][2] - (int)startrgb[quadi2][0] );

								if(pixcontrast > maxpixcontrast)
									maxpixcontrast = pixcontrast;
							}
						}

						//divide by two because the difference between two contrasting pixels will be counted twice as we go through them
						contrast += maxpixcontrast/2;
					}
				}

				if(contrast < MINCONTRAST*3*QUADSZ*QUADSZ && QUADSZ*QUADSZ > 1)
					continue;

				unsigned char bestmatchrgb[QUADSZ*QUADSZ][3];
				unsigned char bestmatchrgbdiff[QUADSZ*QUADSZ][3];
				int bestmatchdiff;
				int bestmatchx = -1;
				int bestmatchy = -1;
				int bestmaxpixdiff = 255;

				for(int searchx=searchminx; searchx<=searchmaxx; searchx++)
				{
					for(int searchy=searchminy; searchy<=searchmaxy; searchy++)
					{
						unsigned char searchrgb[QUADSZ*QUADSZ][3];
						unsigned char searchrgbdiff[QUADSZ*QUADSZ][3];
						int searchdiff = 0;
						int searchmaxpixdiff = 0;

						for(int quadx=0; quadx<QUADSZ; quadx++)
						{
							for(int quady=0; quady<QUADSZ; quady++)
							{
								int quadi = quadx+quady*QUADSZ;

								searchrgb[quadi][0] = curr->data[ ((searchx+quadx) + (searchy+quady)*curr->sizex) * curr->channels + 0 ];
								searchrgb[quadi][1] = curr->data[ ((searchx+quadx) + (searchy+quady)*curr->sizex) * curr->channels + 1 ];
								searchrgb[quadi][2] = curr->data[ ((searchx+quadx) + (searchy+quady)*curr->sizex) * curr->channels + 2 ];

								searchrgbdiff[quadi][0] = iabs( (int)startrgb[quadi][0] - (int)searchrgb[quadi][0] );
								searchrgbdiff[quadi][1] = iabs( (int)startrgb[quadi][1] - (int)searchrgb[quadi][1] );
								searchrgbdiff[quadi][2] = iabs( (int)startrgb[quadi][2] - (int)searchrgb[quadi][2] );

								searchdiff += searchrgbdiff[quadi][0];
								searchdiff += searchrgbdiff[quadi][1];
								searchdiff += searchrgbdiff[quadi][2];

								if(searchrgbdiff[quadi][0] > searchmaxpixdiff)
									searchmaxpixdiff = searchrgbdiff[quadi][0];
								if(searchrgbdiff[quadi][1] > searchmaxpixdiff)
									searchmaxpixdiff = searchrgbdiff[quadi][1];
								if(searchrgbdiff[quadi][2] > searchmaxpixdiff)
									searchmaxpixdiff = searchrgbdiff[quadi][2];
							}
						}

						int bestdiffx = iabs( x - bestmatchx );
						int bestdiffy = iabs( y - bestmatchy );

						int diffx = iabs( x - searchx );
						int diffy = iabs( y - searchy );

						//smaller rgb difference overall = bigger score
						float rgbdiffscore = bestmatchdiff - searchdiff;

						//smaller pixel rgb difference = bigger score
						float pixrgbscore = bestmaxpixdiff - searchmaxpixdiff;

						//int xposdiffscore = bestdiffx - diffx;
						//int yposdiffscore = bestdiffy - diffy;

						float bestdiffdist = sqrtf( bestdiffx*bestdiffx + bestdiffy*bestdiffy );
						float diffdist = sqrtf( diffx*diffx + diffy*diffy );

						//smaller distance = bigger score
						float distdiffscore = bestdiffdist - diffdist;

						//exact pixel match regardless of distance
						//float score1 = ;

						//
						//float score2 = 1;

						if( //bestmatchx < 0 || 
							// !( bestmatchx >= 0 && ( distdiffscore <= 0 || rgbdiffscore <= 0 || pixrgbscore <= 0 ) ) && //make sure it's not worse in every way than another match
								!( bestmatchx >= 0 && ( distdiffscore <= 0 && rgbdiffscore <= 0 && pixrgbscore <= 0 ) ) && //make sure it's not worse in every way than another match
								(
								/* ( searchrgbdiff[0] <= bestmatchrgbdiff[0] &&
								searchrgbdiff[1] <= bestmatchrgbdiff[1] &&
								searchrgbdiff[2] <= bestmatchrgbdiff[2] && */
								/* searchdiff <= bestmatchdiff &&
								diffx <= bestdiffx &&
								diffy <= bestdiffy */ /* &&
								searchmaxpixdiff <= bestmaxpixdiff */
								( bestmatchx < 0 && searchmaxpixdiff < 3 ) ||	//exact pixel match regardless of distance
								( bestmatchx < 0 && searchmaxpixdiff < 4 && diffdist < 5 ) ||	//almost exact pixel match a little distance off
								( bestmatchx < 0 && searchmaxpixdiff <= 256 && diffdist < 2 && searchdiff <= /* 100+5*QUADSZ*QUADSZ */ searchmaxpixdiff + 60 ) ||	// substantial pixel difference but overall better match, small shift vector
								( bestmatchx >= 0 && distdiffscore >= 0 && rgbdiffscore >= 0 && pixrgbscore >= 0 ) || 	//or better match overall
								( bestmatchx < 0 && searchdiff < QUADSZ*QUADSZ*3*MATCHCONTRAST /* MATCHCONTRAST */ && searchmaxpixdiff < 5 ) //or a pretty good overall pixel match
								) )
						{
							for(int quadx=0; quadx<QUADSZ; quadx++)
							{
								for(int quady=0; quady<QUADSZ; quady++)
								{
									int quadi = quadx+quady*QUADSZ;

									bestmatchrgb[quadi][0] = searchrgb[quadi][0];
									bestmatchrgb[quadi][1] = searchrgb[quadi][1];
									bestmatchrgb[quadi][2] = searchrgb[quadi][2];

									bestmatchrgbdiff[quadi][0] = searchrgbdiff[quadi][0];
									bestmatchrgbdiff[quadi][1] = searchrgbdiff[quadi][1];
									bestmatchrgbdiff[quadi][2] = searchrgbdiff[quadi][2];
								}
							}

							bestmatchx = searchx;
							bestmatchy = searchy;

							bestmatchdiff = searchdiff;

							bestmaxpixdiff = searchdiff;
						}
					}
				}

				//if we got a match, write the shift vector color to comparison texture
				if(bestmatchx >= 0 /* && 
								   bestmatchdiff < QUADSZ*QUADSZ*3*MATCHCONTRAST && 
								   bestmaxpixdiff <= PIXMATCHCONTRAST */
								   /* ( 
								   ( bestmaxpixdiff < 1 ) ||	//exact pixel match regardless of distance
								   ( bestmaxpixdiff < 3 && bestdiffdist < 5 ) ||	//almost exact pixel match a little distance off
								   ( bestmaxpixdiff < 200 && bestdiffdist < 10 && searchdiff < 200+5*QUADSZ*QUADSZ && rgbdiffscore > 0) 
								   ) */ /* &&
								   ( iabs( bestmatchx - x ) > 2 || iabs( bestmatchy - y ) > 2 ) */	//ignore shift vecs of (0,0)
								   )
				{
					int bestdiffx = bestmatchx - x;
					int bestdiffy = bestmatchy - y;

#if 0
					comppix[0] = 127 + bestdiffx * 127 / (MAXPIXSEARCH);
					comppix[1] = 127 + bestdiffy * 127 / (MAXPIXSEARCH);
					//comppix[0] = (unsigned char)(bestdiffx * 127 / MAXPIXSEARCH);
					//comppix[1] = (unsigned char)(bestdiffy * 127 / MAXPIXSEARCH);

					if(x % 10 == 0 && y % 10 == 0)
						DrawLine(&comptex, Vec2i(x,y), Vec2i(bestmatchx,bestmatchy));
					//DrawLine(&comptex, Vec2i(x,y), Vec2i(x,y) + (Vec2i(bestmatchx,bestmatchy)-Vec2i(x,y)) * 10);
#else

					comppix[0] = 127 + bestdiffx;
					comppix[1] = 127 + bestdiffy;

					//comppix[0] = comppix[1] = comppix[2] = contrast/3/QUADSZ/QUADSZ;
#endif
				}
			}
		}
	}


#if 0
	//CompFrames(&comptex, prev, fin);
	ProcFrame2(&comptex, prev);
#endif

#if 1
	//set non-shifted pixels to 0
	for(int x=0; x<prev->sizex-(QUADSZ-1); x++)
	{
		for(int y=0; y<prev->sizey-(QUADSZ-1); y++)
		{
			//compare 4 pixel quads
			unsigned char* comppix;

			comppix = &comptex.data[ ((x) + (y)*prev->sizex) * prev->channels ];

			if( comppix[0] == 127 && comppix[1] == 127 )
				comppix[0] = comppix[1] = 0;
		}
	}
#endif

	EstRotTl(&comptex, prev, curr, fin);

	//SavePNG(szFilename, &comptex);

#if 0
	if(!outtex->data)
	{
		AllocTex(outtex, comptex.sizex, comptex.sizey, comptex.channels);
	}

	memcpy(outtex->data, comptex.data, comptex.channels * comptex.sizex * comptex.sizey);
#endif
}

void ProcFrame(AVFrame *pFrame, int width, int height, int iFrame, LoadedTex* outtex)
{
	//FILE *pFile;
	//char szFilename[32];
	int  y;
	LoadedTex ltex;
	static LoadedTex prevltex;
	//LoadedTex procltex;

	//if(iFrame % 2 == 1)
	//	return;

	// Open file
	//sprintf(szFilename, "out/frame%06d.png", iFrame);
	//pFile=fopen(szFilename, "wb");
	//if(pFile==NULL)
	//	return;

	// Write header
	//fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	AllocTex(&ltex, width, height, 3);

	// Write pixel data
	//for(y=0; y<height; y++)
	//	fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

	for(y=0; y<height; y++)
		memcpy(&ltex.data[3*y*width], pFrame->data[0]+y*pFrame->linesize[0], width*3);

	// Close file
	//fclose(pFile);

	//first frame?
	if(!prevltex.data)
	{
		AllocTex(&prevltex, width, height, 3);

		for(y=0; y<height; y++)
			memcpy(&prevltex.data[3*y*width], &ltex.data[3*y*width], width*3);

		return;
	}
	//enough data available to compare frames?
	else
	{
		CompareFrames(&prevltex, &ltex, iFrame, outtex);

		//update previous frame to current
		for(y=0; y<height; y++)
			memcpy(&prevltex.data[3*y*width], &ltex.data[3*y*width], width*3);
	}

	//SavePNG(szFilename, &ltex);

	ltex.destroy();
}

#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

/* add a video output stream */
static AVStream *add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id, int width, int height)
{
	AVCodecContext *c;
	AVStream *st;

	st = avformat_new_stream(oc, NULL);
	if (!st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}
	st->id = 0;

	c = st->codec;
	c->codec_id = codec_id;
	c->codec_type = AVMEDIA_TYPE_VIDEO;

	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = width;
	c->height = height;
	/* time base: this is the fundamental unit of time (in seconds) in terms
	of which frame timestamps are represented. for fixed-fps content,
	timebase should be 1/framerate and timestamp increments should be
	identically 1. */
	c->time_base.den = STREAM_FRAME_RATE;
	c->time_base.num = 1;
	c->gop_size = 12; /* emit one intra frame every twelve frames at most */
	c->pix_fmt = STREAM_PIX_FMT;
	if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
		/* just for testing, we also add B frames */
		c->max_b_frames = 2;
	}
	if (c->codec_id == CODEC_ID_MPEG1VIDEO){
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		This does not happen with normal video, it just happens here as
		the motion of the chroma plane does not match the luma plane. */
		c->mb_decision=2;
	}
	// some formats want stream headers to be separate
	if(oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}

void YUVfromRGB(double& Y, double& U, double& V, const double R, const double G, const double B)
{
	Y =  0.257 * R + 0.504 * G + 0.098 * B +  16;
	U = -0.148 * R - 0.291 * G + 0.439 * B + 128;
	V =  0.439 * R - 0.368 * G - 0.071 * B + 128;
}

void ProcVid(const char* fullpath)
{
	av_register_all();
	avfilter_register_all();

	AVFormatContext *pFormatCtx = NULL;
	//AVOutputFormat *fmt;

#if 0
	fmt = av_guess_format(NULL, fullpath, NULL);

	if(!fmt)
	{
		ErrMess("Error", "Failed to guess video format");
		g_quit = true;
		return;
	}
#endif

#if 1
	// Open video file
	if( avformat_open_input(&pFormatCtx, fullpath, NULL, 0) != 0 )
		//if(av_open_input_file(&pFormatCtx, fullpath, NULL, 0, NULL) != 0)
	{
		char m[1024 + DMD_MAX_PATH];
		sprintf(m, "Failed to open input video file: %s", fullpath);
		ErrMess("Error", m);
		printf("%s\n", m);
		g_quit = true;
		return;
	}
#endif

#if 0
	pFormatCtx = avformat_alloc_context();

	if(!pFormatCtx)
	{
		ErrMess("Error", "Error allocating format context");
		g_quit = true;
		return;
	}

	pFormatCtx->oformat = fmt;
#endif



	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
	{
		// Couldn't find stream information
		ErrMess("Error", "Couldn't find stream information");
		printf("Couldn't find stream information\n");
		g_quit = true;
		return;
	}

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, fullpath, 0);

	int i;
	AVCodecContext *pCodecCtxOrig = NULL;
	AVCodecContext *pCodecCtx = NULL;

	// Find the first video stream
	int videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
		{
			videoStream=i;
			break;
		}
	}

	if(videoStream==-1)
	{
		// Didn't find a video stream
		ErrMess("Error", "Didn't find a video stream");
		printf("Didn't find a video stream\n");
		g_quit = true;
		return;
	}

	// Get a pointer to the codec context for the video stream
	//pCodecCtx=pFormatCtx->streams[videoStream]->codec;
	pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;



	AVCodec *pCodec = NULL;

	// Find the decoder for the video stream
	//pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if(pCodec==NULL) {
		ErrMess("Error", "Unsupported codec!");
		fprintf(stderr, "Unsupported codec!\n");
		return; // Codec not found
	}
	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);

	int asd = 0;

	if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		ErrMess("Error", "Couldn't copy codec context");
		fprintf(stderr, "Couldn't copy codec context");
		return; // Error copying codec context
	}
	// Open codec
	//if(avcodec_open2(pCodecCtx, pCodec)<0)
	if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
		return; // Could not open codec




	AVFrame *pFrame = NULL;
	AVFrame *pFrameRGB = NULL;

	// Allocate video frame
	pFrame=av_frame_alloc();



	// Allocate an AVFrame structure
	pFrameRGB=av_frame_alloc();
	if(pFrameRGB==NULL)
		return;



	uint8_t *buffer = NULL;
	int numBytes;
	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
		pCodecCtx->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));



	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height);




	struct SwsContext *sws_ctx = NULL;
	int frameFinished;
	AVPacket packet;
	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		PIX_FMT_RGB24,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);






	AVCodec *outcodec;
	AVCodecContext *outc= NULL;
	int out_frame_size, outx, outy, outbuf_size, outsize;
	FILE *outf;
	AVFrame *outpicture;
	uint8_t *outbuf, *out_picture_buf;
	AVStream *st;
	AVFormatContext *oc;
	AVOutputFormat *fmt;


#if 00
	//fmt = av_guess_format("mpeg", NULL, NULL);
	//oc = avformat_alloc_context();
	avformat_alloc_output_context2(&oc, NULL, NULL, "out/out.mpg");

	if (!oc) {
         printf("Could not deduce output format from file extension: using MPEG.\n");
         avformat_alloc_output_context2(&oc, NULL, "mpeg", "out/out.mpg");
     }
     if (!oc) {
         return;
     }

	//oc->oformat = fmt;
	//sprintf(oc->filename, "out/%s.mpg", FileDateTime().c_str());
	fmt = oc->oformat;
	st = add_video_stream(oc, fmt->video_codec, pCodecCtx->width, pCodecCtx->height);

#if 0
	if (av_set_parameters(oc, NULL) < 0) {
		fprintf(stderr, "Invalid output format parameters\n");
		exit(1);
	}
#endif

	/* Write the stream header, if any. */
     if (avformat_write_header(oc, NULL) < 0) {
         fprintf(stderr, "Error occurred when opening output file\n");
         return;
     }
#endif




#if 0
	outcodec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
	outc = avcodec_alloc_context3(outcodec);
	outpicture= avcodec_alloc_frame();

	outc->bit_rate = 400000;

	/* resolution must be a multiple of two */
	outc->width = pCodecCtx->width/2*2;
	outc->height = pCodecCtx->height/2*2;
	AVRational avr;
	avr.den = 1;
	avr.num = 25;
	outc->time_base= avr;

	outc->gop_size = 10; /* emit one intra frame every ten frames */
	outc->max_b_frames=0;//1;
	outc->pix_fmt = PIX_FMT_YUV420P;

#if 0
	outc->keyint_min = 1;                                      // minimum GOP size
	outc->i_quant_factor = (float)0.71;                        // qscale factor between P and I frames
	outc->b_frame_strategy = 20;                               ///// find out exactly what this does
	outc->qcompress = (float)0.6;                              ///// find out exactly what this does
	outc->qmin = 20;                                           // minimum quantizer
	outc->qmax = 51;                                           // maximum quantizer
	outc->max_qdiff = 4;                                       // maximum quantizer difference between frames
	outc->refs = 4;                                            // number of reference frames
	outc->trellis = 1;                                         // trellis RD Quantization
	outc->pix_fmt = PIX_FMT_YUV420P;                           // universal pixel format for video encoding
	outc->codec_id = CODEC_ID_H264;
	outc->codec_type = AVMEDIA_TYPE_VIDEO;
#endif

	int avcodecopenerr = avcodec_open2(outc, outcodec, NULL);

	if (avcodecopenerr < 0)
	{
		char m[128];
		sprintf(m, "Out context fail \n err=%d", avcodecopenerr);
		ErrMess("Error", m);
		g_quit = true;
		return;
	}

	outf = fopen("out/out.mpg", "wb");



	outbuf_size = 100000;
	outbuf = (uint8_t*)malloc(outbuf_size);
	outsize = outc->width * outc->height;
	out_picture_buf = (uint8_t*)malloc((outsize * 3) / 2); /* size for YUV 420 */

#if 1
	outpicture->data[0] = out_picture_buf;
	outpicture->data[1] = outpicture->data[0] + outsize;
	outpicture->data[2] = outpicture->data[1] + outsize / 4;
	outpicture->linesize[0] = outc->width;
	outpicture->linesize[1] = outc->width / 2;
	outpicture->linesize[2] = outc->width / 2;
#else
	/* the image can be allocated by any means and av_image_alloc() is
	* just the most convenient way if av_malloc() is to be used */
	av_image_alloc(outpicture->data, outpicture->linesize,
		outc->width, outc->height, outc->pix_fmt, 1);
#endif

	int out_size;
#endif


	i=0;
	while(av_read_frame(pFormatCtx, &packet)>=0)
	{
		// Is this a packet from the video stream?
		if(packet.stream_index==videoStream)
		{
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			// Did we get a video frame?
			if(frameFinished)
			{
				// Convert the image from its native format to RGB
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);

				// Save the frame to disk
				//if(++i<=5)
				//	SaveFrame(pFrameRGB, pCodecCtx->width, 
				//	pCodecCtx->height, i);

				LoadedTex outtex;

				//procframe
				//if(i > 90)
				ProcFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i, &outtex);

#if 0
				if(outtex.data)
				{

					/* Y */
					for(outy=0;outy<outc->height;outy++) {
						for(outx=0;outx<outc->width;outx++) {
							double Y, Cb, Cr;
							double R = outtex.data[ (outx + outy * outtex.sizex) * outtex.channels + 0 ];
							double G = outtex.data[ (outx + outy * outtex.sizex) * outtex.channels + 1 ];
							double B = outtex.data[ (outx + outy * outtex.sizex) * outtex.channels + 2 ];
							YUVfromRGB(Y, Cb, Cr, R, G, B);
							outpicture->data[0][outy * outpicture->linesize[0] + outx] = Y;
						}
					}


					/* Cb and Cr */
					for(outy=0;outy<outc->height/2;outy++) {
						for(outx=0;outx<outc->width/2;outx++) {
							double Y, Cb, Cr;
							double R = outtex.data[ (outx*2 + outy*2 * outtex.sizex) * outtex.channels + 0 ];
							double G = outtex.data[ (outx*2 + outy*2 * outtex.sizex) * outtex.channels + 1 ];
							double B = outtex.data[ (outx*2 + outy*2 * outtex.sizex) * outtex.channels + 2 ];
							YUVfromRGB(Y, Cb, Cr, R, G, B);
							outpicture->data[1][outy * outpicture->linesize[1] + outx] = Cb;
							outpicture->data[2][outy * outpicture->linesize[2] + outx] = Cr;
						}
					}
#endif

#if 0
					/* encode the image */
					out_size = avcodec_encode_video(outc, outbuf, outbuf_size, outpicture);
					fwrite(outbuf, 1, out_size, outf);

#if 0
					if (out_size > 0) {
						AVPacket pkt;
						av_init_packet(&pkt);

						if (c->coded_frame->pts != AV_NOPTS_VALUE)
							pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
						if(c->coded_frame->key_frame)
							pkt.flags |= AV_PKT_FLAG_KEY;
						pkt.stream_index= st->index;
						pkt.data= outbuf;
						pkt.size= out_size;

						/* write the compressed frame in the media file */
						int ret = av_interleaved_write_frame(oc, &pkt);

					}
#endif

#elif 00
					int ret;

					if (filter_ctx[stream_index].filter_graph) {
						av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
						frame = av_frame_alloc();
						if (!frame) {
							ret = AVERROR(ENOMEM);
							break;
						}
						av_packet_rescale_ts(&packet,
							ifmt_ctx->streams[stream_index]->time_base,
							ifmt_ctx->streams[stream_index]->codec->time_base);
						dec_func = (type == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 :
							avcodec_decode_audio4;
						ret = dec_func(ifmt_ctx->streams[stream_index]->codec, frame,
							&got_frame, &packet);
						if (ret < 0) {
							av_frame_free(&frame);
							av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
							break;
						}

						if (got_frame) {
							frame->pts = av_frame_get_best_effort_timestamp(frame);
							ret = filter_encode_write_frame(frame, stream_index);
							av_frame_free(&frame);
							if (ret < 0)
								goto end;
						} else {
							av_frame_free(&frame);
						}
					} else {
						/* remux this frame without reencoding */
						av_packet_rescale_ts(&packet,
							ifmt_ctx->streams[stream_index]->time_base,
							ofmt_ctx->streams[stream_index]->time_base);

						ret = av_interleaved_write_frame(ofmt_ctx, &packet);
						if (ret < 0)
							goto end;
					}
#endif

					i++;
				//}
			}
end:
			// Free the packet that was allocated by av_read_frame
			av_free_packet(&packet);
		}
	}

#if 0
	/* get the delayed frames */
	for(; out_size; i++) {
		out_size = avcodec_encode_video(outc, outbuf, outbuf_size, NULL);
		fwrite(outbuf, 1, out_size, outf);
	}


	/* add sequence end code to have a real mpeg file */
	outbuf[0] = 0x00;
	outbuf[1] = 0x00;
	outbuf[2] = 0x01;
	outbuf[3] = 0xb7;

	fwrite(outbuf, 1, 4, outf);

	fclose(outf);

	free(out_picture_buf);
	free(outbuf);

	//avcodec_close(st->codec);
	avcodec_close(outc);
	av_free(outc);
	av_free(outpicture);
	
#endif






	// Free the RGB image
	av_free(buffer);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);
}