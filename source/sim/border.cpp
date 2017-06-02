

#include "border.h"
#include "../platform.h"
#include "../gui/draw2d.h"
#include "../gui/gui.h"
#include "../render/heightmap.h"
#include "map.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "../math/isomath.h"
#include "../render/fogofwar.h"

signed char* g_border = NULL;

void DrawBord(Vec3i cmfrom, Vec3i cmto, Vec2i frompixoff, Vec2i topixoff, float* color)
{				
	//cmfrom.z = Bilerp(&g_hmap, cmfrom.x, cmfrom.y) * TILE_RISE;
	//cmto.z = Bilerp(&g_hmap, cmto.x, cmto.y) * TILE_RISE;
	
	cmfrom.z = g_hmap.getheight(cmfrom.x/TILE_SIZE, cmfrom.y/TILE_SIZE) * TILE_RISE;
	cmto.z = g_hmap.getheight(cmto.x/TILE_SIZE, cmto.y/TILE_SIZE) * TILE_RISE;

	Vec2i screenfrom = CartToIso(cmfrom) - g_scroll + frompixoff;
	Vec2i screento = CartToIso(cmto) - g_scroll + topixoff;

	DrawLine(color[0], color[1], color[2], color[3], screenfrom.x, screenfrom.y, screento.x, screento.y, g_gui.m_crop);
}

#define BORDERPIXOFF	1

void DrawBords(Vec2i tmin, Vec2i tmax)
{
	float* crop = g_gui.m_crop;
	Vec3i cmfrom, cmto;
	Vec2i frompixoff, topixoff;
	int i, i2;
	signed char owner, owner2;
	float* color;

	for(int tx=tmin.x; tx<=tmax.x; tx++)
		for(int ty=tmin.y; ty<=tmax.y; ty++)
		{
			int i = tx + ty * g_mapsz.x;

			owner = g_border[i];

			if(owner < 0)
				continue;

			if(!IsTileVis(g_localP, tx, ty))
			//if(!Explored(g_localP, tx, ty))
				continue;

			color = g_player[owner].color;

			bool drawne = false;
			bool drawnw = false;
			bool drawse = false;
			bool drawsw = false;

			//north-east
			if(ty > 0)
			{
				i2 = tx + (ty-1) * g_mapsz.x;
				owner2 = g_border[i2];

				if(owner != owner2)
					drawne = true;
			}
			else
				drawne = true;

			if(drawne)
			{
				cmfrom.x = tx * TILE_SIZE;
				cmfrom.y = ty * TILE_SIZE;
				cmto.x = (tx+1) * TILE_SIZE;
				cmto.y = ty * TILE_SIZE;

				frompixoff.x = -BORDERPIXOFF;
				frompixoff.y = BORDERPIXOFF;
				topixoff.x = -BORDERPIXOFF;
				topixoff.y = BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}
		
			//north-west
			if(tx > 0)
			{
				i2 = (tx-1) + ty * g_mapsz.x;
				owner2 = g_border[i2];
				
				if(owner != owner2)
					drawnw = true;
			}
			else
				drawnw = true;

			if(drawnw)
			{
				cmfrom.x = tx * TILE_SIZE;
				cmfrom.y = ty * TILE_SIZE;
				cmto.x = tx * TILE_SIZE;
				cmto.y = (ty+1) * TILE_SIZE;

				frompixoff.x = BORDERPIXOFF;
				frompixoff.y = BORDERPIXOFF;
				topixoff.x = BORDERPIXOFF;
				topixoff.y = BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}

			//south-west
			if(ty < g_mapsz.y-1)
			{
				i2 = tx + (ty+1) * g_mapsz.x;
				owner2 = g_border[i2];

				if(owner != owner2)
					drawsw = true;
			}
			else
				drawsw = true;

			if(drawsw)
			{
				cmfrom.x = tx * TILE_SIZE;
				cmfrom.y = (ty+1) * TILE_SIZE;
				cmto.x = (tx+1) * TILE_SIZE;
				cmto.y = (ty+1) * TILE_SIZE;

				frompixoff.x = BORDERPIXOFF;
				frompixoff.y = -BORDERPIXOFF;
				topixoff.x = BORDERPIXOFF;
				topixoff.y = -BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}

			//south-east
			if(tx < g_mapsz.x-1)
			{
				i2 = (tx+1) + ty * g_mapsz.x;
				owner2 = g_border[i2];

				if(owner != owner2)
					drawse = true;
			}
			else
				drawse = true;

			if(drawse)
			{
				cmfrom.x = (tx+1) * TILE_SIZE;
				cmfrom.y = ty * TILE_SIZE;
				cmto.x = (tx+1) * TILE_SIZE;
				cmto.y = (ty+1) * TILE_SIZE;

				frompixoff.x = -BORDERPIXOFF;
				frompixoff.y = -BORDERPIXOFF;
				topixoff.x = -BORDERPIXOFF;
				topixoff.y = -BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}
		}

	//DrawLine(
}