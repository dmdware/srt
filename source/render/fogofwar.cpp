


/*******************************************************
 * Copyright (C) 2015 DMD 'Ware <dmdware@gmail.com>
 * 
 * This file is part of States, Firms, & Households.
 * 
 * You are entitled to use this source code to learn.
 *
 * You are not entitled to duplicate or copy this source code 
 * into your own projects, commercial or personal, UNLESS you 
 * give credit.
 *
 *******************************************************/






#include "fogofwar.h"
#include "../sim/utype.h"
#include "../sim/unit.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "heightmap.h"
#include "../utils.h"
#include "../math/fixmath.h"
#include "../app/appmain.h"

VisTile* g_vistile = NULL;


void AddVis(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int vr = ut->visrange;
	int cmminx = u->cmpos.x - vr;
	int cmminy = u->cmpos.y - vr;
	int cmmaxx = u->cmpos.x + vr;
	int cmmaxy = u->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	///Log("cymmp "<<u->cmpos.y<<","<<vr);
	//Log("cymm "<<cmminy<<","<<cmmaxy);
	//Log("tymm "<<tminy<<","<<tmaxy);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int ui = u - g_unit;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->uvis.push_back( ui );

			//Log("add vis "<<tx<<","<<ty);
		}
}

void RemVis(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int vr = ut->visrange;
	int cmminx = u->cmpos.x - vr;
	int cmminy = u->cmpos.y - vr;
	int cmmaxx = u->cmpos.x + vr;
	int cmmaxy = u->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			int ui = u - g_unit;

			auto vit=v->uvis.begin();
			while(vit!=v->uvis.end())
			{
				if(*vit == ui)
				{
					vit = v->uvis.erase(vit);
					//Log("rem vis "<<tx<<","<<ty);
					continue;
				}

				vit++;
			}
		}
}

void AddVis(Building* b)
{
	//Log("bt "<<b->type);
	//

	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	//Log("bav "<<bcmpos.x<<","<<bcmpos.y);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - bcmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - bcmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - bcmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - bcmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible

			/*
			Using int bi :

==7857== Invalid write of size 8
==7857==    at 0x5F73B3F: std::__detail::_List_node_base::_M_hook(std::__detail::_List_node_base*) (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.18)
==7857==    by 0x47DE74: void std::list<short, std::allocator<short> >::_M_insert<short>(std::_List_iterator<short>, short&&) (stl_list.h:1562)
==7857==    by 0x47DCDE: std::list<short, std::allocator<short> >::push_back(short&&) (stl_list.h:1021)
==7857==    by 0x47D086: AddVis(Building*) (fogofwar.cpp:160)
==7857==    by 0x46C514: FillColliderGrid() (collidertile.cpp:281)
==7857==    by 0x4A54B9: LoadMap(char const*) (savemap.cpp:1310)
==7857==    by 0x4368F6: Click_LV_Load() (loadview.cpp:78)
==7857==    by 0x421279: Button::inev(InEv*) (button.cpp:119)
==7857==    by 0x4371F7: LoadView::subinev(InEv*) (loadview.cpp:134)
==7857==    by 0x44C16A: Win::inev(InEv*) (windoww.cpp:347)
==7857==    by 0x41B127: GUI::inev(InEv*) (gui.cpp:100)
==7857==    by 0x550256: EventLoop() (appmain.cpp:1319)
==7857==  Address 0x10000003e2d3148 is not stack'd, malloc'd or (recently) free'd
			*/

			int bi = b - g_building;

			//Log("v "<<(tx + ty * g_mapsz.x)<<"/"<<(g_mapsz.x*g_mapsz.y)<<" ");
			//

			//Log("(g_vistile == NULL) ?= " << (g_vistile == NULL) << std::endl;
			//

			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

			//Log("((&g_vistile[ tx + ty * g_mapsz.x ]) == (&(g_vistile[ tx + ty * g_mapsz.x ]))) ?= " <<
			//((&g_vistile[ tx + ty * g_mapsz.x ]) == (&(g_vistile[ tx + ty * g_mapsz.x ]))) << std::endl;
			//

			//Log("(v == NULL) ?= " << (v == NULL) << std::endl;
			//

			//Log("v s="<<v->bvis.size());
			//

			v->bvis.push_back( bi );

			//Log("bav "<<tx<<","<<ty);

			//Log("v s="<<v->bvis.size());
			//
		}
}

void RemVis(Building* b)
{
	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - bcmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - bcmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - bcmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - bcmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			int bi = b - g_building;

			auto vit=v->bvis.begin();
			while(vit!=v->bvis.end())
			{
				if(*vit == bi)
				{
					vit = v->bvis.erase(vit);
					continue;
				}

				vit++;
			}
		}
}

void Explore(Unit* u)
{
	if(u->owner < 0)
		return;

	UType* ut = &g_utype[u->type];
	int vr = ut->visrange;
	int cmminx = u->cmpos.x - vr;
	int cmminy = u->cmpos.y - vr;
	int cmmaxx = u->cmpos.x + vr;
	int cmmaxy = u->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	///Log("cymmp "<<u->cmpos.y<<","<<vr);
	//Log("cymm "<<cmminy<<","<<cmmaxy);
	//Log("tymm "<<tminy<<","<<tmaxy);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int ui = u - g_unit;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->explored[u->owner] = true;

			//Log("add vis "<<tx<<","<<ty);
		}
}

void Explore(Building* b)
{
	if(b->owner < 0)
		return;

	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	//Log("bav "<<bcmpos.x<<","<<bcmpos.y);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - bcmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - bcmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - bcmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - bcmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int bi = b - g_building;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->explored[b->owner] = true;

			//Log("bav "<<tx<<","<<ty);
		}
}

bool IsTileVis(short py, short tx, short ty)
{
	if(g_mode == APPMODE_EDITOR)
		return true;

	if(g_netmode != NETM_SINGLE)
	return true;	//for now

	VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

	for(auto vit=v->bvis.begin(); vit!=v->bvis.end(); vit++)
	{
		Building* b = &g_building[ *vit ];

		if(b->owner == py)
			return true;
	}

	for(auto vit=v->uvis.begin(); vit!=v->uvis.end(); vit++)
	{
		Unit* u = &g_unit[ *vit ];

		//InfoMess("v","v");
		if(u->owner == py)
			return true;
	}

	return false;
}

bool Explored(short py, short tx, short ty)
{
	VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
	return v->explored[py];
}
