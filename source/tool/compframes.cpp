

#include "compframes.h"
#include "procvid.h"
#include "../platform.h"
#include "../texture.h"
#include "../math/vec2f.h"

#define PIXTRACKFRAMES	5

LoadedTex* pixtrackfr[PIXTRACKFRAMES] = {0};

void CompFrames(LoadedTex* comptex, LoadedTex* colortex, int fin)
{
	if(pixtrackfr[0])
	{
		delete pixtrackfr[0];
		pixtrackfr[0] = NULL;
	}

	for(int i=0; i<PIXTRACKFRAMES-1; i++)
	{
		int i2 = i+1;

		pixtrackfr[i] = pixtrackfr[i2];
	}

	pixtrackfr[PIXTRACKFRAMES-1] = new LoadedTex();

	LoadedTex* last = pixtrackfr[PIXTRACKFRAMES-1];

	AllocTex(last, comptex->sizex, comptex->sizey, comptex->channels);

	memcpy(last->data, comptex->data, comptex->sizex * comptex->sizey * comptex->channels);

	LoadedTex* first = pixtrackfr[0];

	if(!first)
		return;

	LoadedTex trackfinal;
	AllocTex(&trackfinal, first->sizex, first->sizey, first->channels);

	Vec2f start;
	Vec2f curr;
	Vec2f shift;
	unsigned char* shift8;
	LoadedTex* cf;
	Vec2f allshift;

	for(int x=0; x<comptex->sizex; x++)
	{
		for(int y=0; y<comptex->sizey; y++)
		{
			start = Vec2f(x,y);
			curr = start;
			cf = pixtrackfr[0];
			shift8 = &cf->data[ ((int)curr.x + (int)curr.y * cf->sizex) * cf->channels ];
			shift = Vec2f(shift8[0] - 127, shift8[1] - 127);
			allshift = shift;
			unsigned char* track8 = &trackfinal.data[ ((int)curr.x + (int)curr.y * cf->sizex) * cf->channels ];

			if( (!shift8[0] || !shift8[1]) )
			{
				track8[0] = track8[1] = 0;
				continue;
			}

			for(int cfin=1; cfin<PIXTRACKFRAMES; cfin++)
			{
				curr = curr + shift;

				int ix = (int)curr.x;
				int iy = (int)curr.y;

				if(ix < 0)
					goto stoppix;
				if(iy < 0)
					goto stoppix;
				if(ix >= cf->sizex)
					goto stoppix;
				if(iy >= cf->sizey)
					goto stoppix;

				//now read next shift

				shift8 = &cf->data[ ((int)curr.x + (int)curr.y * cf->sizex) * cf->channels ];

				if( (!shift8[0] || !shift8[1]) )
					goto stoppix;

				shift = Vec2f(shift8[0] - 127, shift8[1] - 127);

				allshift = allshift + shift;

				continue;

				//end of the road?
stoppix:
				//allshift = ( allshift * (float)cfin + shift ) / ((float)cfin + 1.0f);
				//allshift = allshift + shift;
				allshift = Vec2f(0,0);
				break;
			}

			track8[0] = 127 + allshift.x * 127 / (MAXPIXSEARCH * PIXTRACKFRAMES);
			track8[1] = 127 + allshift.y * 127 / (MAXPIXSEARCH * PIXTRACKFRAMES);
		}
	}

	//ProcFrame2(&trackfinal, colortex);

	char outfile[32];
	sprintf(outfile, "out/track%06d.png", fin);
	SavePNG(outfile, &trackfinal);
}