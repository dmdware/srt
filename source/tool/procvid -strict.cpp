
#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../app/appmain.h"
#include "../texture.h"

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

//#define MAXPIXSEARCH 24
#define MAXPIXSEARCH 24
#define QUADSZ	3
#define MINCONTRAST 3
#define MATCHCONTRAST	16
#define PIXMATCHCONTRAST	190

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

void ProcFrame2(LoadedTex* comptex, LoadedTex* colortex)
{
	Vec2f avgmatchvec;
	
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

			translvec = ( translvec + pixshift * counted ) / (counted + 1.0f);

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

	//set final colors
	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			unsigned char* shiftvec8 = &comptex->data[ comptex->channels * (x + y*comptex->sizex) ];

			if( (!shiftvec8[0] && !shiftvec8[1]) || (shiftvec8[0] == 127 && shiftvec8[1] == 127) )
				continue;

			Vec2f pixshift = Vec2f( (shiftvec8[0]-127), (shiftvec8[1]-127) );

			shiftvec8[0] = 127 + 127 * pixshift.x / MAXPIXSEARCH;
			shiftvec8[1] = 127 + 127 * pixshift.y / MAXPIXSEARCH;
		}
	}

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

	DrawLine(comptex, Vec2i(127,127), Vec2i(127,127) + Vec2i(translvec.x * 3, translvec.y * 3));
	DrawLine(comptex, Vec2i(255,255), Vec2i(255,255) + Vec2i(infac * 3, infac * 3));
	DrawLine(comptex, Vec2i(127,255), Vec2i(127,255) + Vec2i(sideexpanfac * 3, sideexpanfac * 3));
}

void CompareFrames(LoadedTex* prev, LoadedTex* curr, int fin)
{	
	char szFilename[32];
	sprintf(szFilename, "out/frame%06d.png", fin);

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
								( bestmatchx < 0 && searchmaxpixdiff < 1 && diffdist < 6 ) ||	//exact pixel match regardless of distance
								( bestmatchx < 0 && searchmaxpixdiff < 3 && diffdist < 5 ) ||	//almost exact pixel match a little distance off
								( bestmatchx < 0 && searchmaxpixdiff <= 256 && diffdist < 4 && searchdiff < 256 ) ||	// substantial pixel difference but overall better match, substantial shift vector
								( bestmatchx >= 0 && distdiffscore >= 0 && rgbdiffscore >= 0 && pixrgbscore >= 0 ) || 	//or better match overall
								( bestmatchx < 0 && searchdiff < QUADSZ*QUADSZ*3 && searchmaxpixdiff < 3 ) //or a pretty good overall pixel match
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
#endif
				}
			}
		}
	}

	ProcFrame2(&comptex, prev);

	SavePNG(szFilename, &comptex);
}

void ProcFrame(AVFrame *pFrame, int width, int height, int iFrame)
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
		CompareFrames(&prevltex, &ltex, iFrame);

		//update previous frame to current
		for(y=0; y<height; y++)
			memcpy(&prevltex.data[3*y*width], &ltex.data[3*y*width], width*3);
	}

	//SavePNG(szFilename, &ltex);

	ltex.destroy();
}

void ProcVid(const char* fullpath)
{
	av_register_all();

	AVFormatContext *pFormatCtx = NULL;
	AVOutputFormat *fmt;

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

				//procframe
				ProcFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
				i++;
			}
		}

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}





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