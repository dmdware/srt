


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



#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../bsp/trace.h"
#include "../algo/binheap.h"
#include "pathnode.h"
#include "../math/vec2i.h"
#include "pathdebug.h"
#include "pathjob.h"
#include "../debug.h"
#include "../sim/conduit.h"
#include "../sim/transport.h"
#include "../phys/collision.h"
#include "fillbodies.h"
#include "tilepath.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/chattext.h"
#include "../render/drawsort.h"

ColliderTile *g_collidertile = NULL;

ColliderTile::ColliderTile()
{
#if 0
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	std::list<int> units;
	std::list<int> foliage;
	int building;
#endif

	//hasroad = false;
	//hasland = false;
	//haswater = false;
	//abrupt = false;
	flags = 0;
	building = -1;
#ifdef TILESIZECOLLIDER
	units.clear();
	foliages.clear();
#else
	for(int i=0; i<MAX_COLLIDER_UNITS; i++)
		units[i] = -1;
	foliage = USHRT_MAX;
#endif
}

inline Vec2i PathNodePos(int cmposx, int cmposy)
{
	return Vec2i(cmposx/PATHNODE_SIZE, cmposy/PATHNODE_SIZE);
}

void FreePathGrid()
{
	Log("free path gr");

	if(g_collidertile)
	{
		delete [] g_collidertile;
		g_collidertile = NULL;
	}

	g_pathdim = Vec2i(0,0);

	if(g_pathnode)
	{
		delete [] g_pathnode;
		g_pathnode = NULL;
	}

	g_openlist.freemem();

	if(g_tilepass)
	{
		delete [] g_tilepass;
		g_tilepass = NULL;
	}

	if(g_tregs)
	{
		delete [] g_tregs;
		g_tregs = NULL;
	}

	if(g_tilenode)
	{
		delete [] g_tilenode;
		g_tilenode = NULL;
	}
}

void AllocPathGrid(int cmwx, int cmwz)
{
	FreePathGrid();
	g_pathdim.x = cmwx / PATHNODE_SIZE;
	g_pathdim.y = cmwz / PATHNODE_SIZE;
	g_collidertile = new ColliderTile [ g_pathdim.x * g_pathdim.y ];

	Log("path gr allc %d,%d\r\n", g_pathdim.x, g_pathdim.y);

	int cwx = g_pathdim.x;
	int cwy = g_pathdim.y;

	g_pathnode = new PathNode [ cwx * cwy ];
	if(!g_pathnode) OUTOFMEM();

	g_openlist.alloc( cwx * cwy );

	for(int x=0; x<cwx; x++)
		for(int y=0; y<cwy; y++)
		{
			PathNode* n = PathNodeAt(x, y);
			//n->nx = x;
			//n->ny = z;
			n->opened = false;
			n->closed = false;
		}

	g_tilepass = new TileRegs [ (cmwx / TILE_SIZE) * (cmwz / TILE_SIZE) ];
	if(!g_tilepass) OUTOFMEM();

	g_tregs = new unsigned char [ g_pathdim.x * g_pathdim.y ];
	if(!g_tregs) OUTOFMEM();

	g_tilenode = new TileNode [ (cmwx / TILE_SIZE) * (cmwz / TILE_SIZE) ];
	if(!g_tilenode) OUTOFMEM();

	//g_lastpath = g_simframe;
}

ColliderTile* ColliderAt(int nx, int ny)
{
	return &g_collidertile[ PathNodeIndex(nx, ny) ];
}

void FillColliderGrid()
{
	const int cwx = g_pathdim.x;
	const int cwy = g_pathdim.y;

	//Log("path gr "<<cwx<<","<<cwy);

	for(int x=0; x<cwx; x++)
		for(int y=0; y<cwy; y++)
		{
			int cmx = x*PATHNODE_SIZE + PATHNODE_SIZE/2;
			int cmy = y*PATHNODE_SIZE + PATHNODE_SIZE/2;
			ColliderTile* cell = ColliderAt(x, y);

			//Log("cell "<<x<<","<<y<<" cmpos="<<cmx<<","<<cmy<<" y="<<g_hmap.accheight(cmx, cmy));

			//if(AtLand(cmx, cmy))
			{
				//cell->hasland = true;
				cell->flags |= FLAG_HASLAND;
				//Log("land "<<(cmx/TILE_SIZE)<<","<<(cmy/TILE_SIZE)<<" flag="<<(cell->flags & FLAG_HASLAND)<<"="<<(unsigned int)cell->flags);
				//Log("land");
			}
#if 0
			else
			{
				//cell->hasland = false;
				cell->flags &= ~FLAG_HASLAND;
			}
#endif

#if 0
			if(AtWater(cmx, cmy))
				cell->haswater = true;
			else
				cell->haswater = false;
#endif

#if 0
			if(TileUnclimable(cmx, cmy) && (cell->flags & FLAG_HASLAND))
			{
				//cell->abrupt = true;
				cell->flags |= FLAG_ABRUPT;
			}
			else
#endif
			{
				//cell->abrupt = false;
				cell->flags &= ~FLAG_ABRUPT;
			}

			int tx = cmx/TILE_SIZE;
			int ty = cmy/TILE_SIZE;

			CdTile* r = GetCd(CD_ROAD, tx, ty, false);

#if 0
			//if(r->on /* && r->finished */ )
			if(r->on && r->finished)
			{
				//cell->hasroad = true;
				cell->flags |= FLAG_HASROAD;
			}
			else
			{
				//cell->hasroad = false;
				cell->flags &= ~FLAG_HASROAD;
			}
#endif
		}


	for(int x=0; x<LARGEST_UNIT_NODES; x++)
		for(int y=0; y<cwy; y++)
		{
			ColliderTile* cell = ColliderAt(x, y);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int x=cwx-LARGEST_UNIT_NODES-1; x<cwx; x++)
		for(int y=0; y<cwy; y++)
		{
			ColliderTile* cell = ColliderAt(x, y);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int x=0; x<cwx; x++)
		for(int y=0; y<LARGEST_UNIT_NODES; y++)
		{
			ColliderTile* cell = ColliderAt(x, y);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int x=0; x<cwx; x++)
		for(int y=cwy-LARGEST_UNIT_NODES-1; y<cwy; y++)
		{
			ColliderTile* cell = ColliderAt(x, y);
			cell->flags |= FLAG_ABRUPT;
		}

#if 1
	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		g_drawlist.push_back(Depthable());
		Depthable* d = &*g_drawlist.rbegin();
		u->depth = d;
		d->dtype = DEPTH_U;
		d->index = i;
		UpDraw(u);

		if(u->hidden())
			continue;

		u->fillcollider();

		AddVis(u);
		Explore(u);
	}
#endif

	//Log("fill cb");
	//

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		//Log("fill cb "<<i<<" c");
		//

		b->fillcollider();

		g_drawlist.push_back(Depthable());
		Depthable* d = &*g_drawlist.rbegin();
		b->depth = d;
		d->dtype = DEPTH_BL;
		d->index = i;
		UpDraw(b);

		//Log("fill cb "<<i<<" v");
		//
		//if(b->finished)
		AddVis(b);

		//Log("fill cb "<<i<<" e");
		//

		Explore(b);
	}

	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		f->fillcollider();

		g_drawlist.push_back(Depthable());
		Depthable* d = &*g_drawlist.rbegin();
		f->depth = d;
		d->dtype = DEPTH_FOL;
		d->index = i;
		UpDraw(f);
	}

	ResetPathNodes();
	FillBodies();
}

void Foliage::fillcollider()
{
	FlType* ft = &g_fltype[type];

	//cm = centimeter position
	int cmminx = cmpos.x - ft->size.x/2;
	int cmminy = cmpos.y - ft->size.x/2;
	int cmmaxx = cmminx + ft->size.x - 1;
	int cmmaxy = cmminy + ft->size.x - 1;

#if 0
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);
#endif

#ifdef TILESIZECOLLIDER
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//t = tile position
	int tminx = cmminx / TILE_SIZE;
	int tminy = cmminy / TILE_SIZE;
	int tmaxx = cmmaxx / TILE_SIZE;
	int tmaxy = cmmaxy / TILE_SIZE;

	unsigned short fi = this - g_foliage;

	for(int ty = tminy; ty <= tmaxy; ty++)
		for(int tx = tminx; tx <= tmaxx; tx++)
		{
			ColliderTile* c = ColliderAt(tx, ty);
			c->foliages.push_back( fi );
		}

#elif 0
	//c = cell position
	int nminx = imax(0, cmminx / PATHNODE_SIZE);
	int nminy = imax(0, cmminy / PATHNODE_SIZE);
	int nmaxx = imin(g_pathdim.x-1, cmmaxx / PATHNODE_SIZE);
	int nmaxy = imin(g_pathdim.y-1, cmmaxy / PATHNODE_SIZE);

	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);
			c->foliage = this - g_foliage;
		}
#else
	int nx = cmpos.x / PATHNODE_SIZE;
	int ny = cmpos.y / PATHNODE_SIZE;
	ColliderTile* c = ColliderAt(nx, ny);
	c->foliage = this - g_foliage;
#endif
}

void Foliage::freecollider()
{
	FlType* ft = &g_fltype[type];

	//cm = centimeter position
	int cmminx = cmpos.x - ft->size.x/2;
	int cmminy = cmpos.y - ft->size.x/2;
	int cmmaxx = cmminx + ft->size.x - 1;
	int cmmaxy = cmminy + ft->size.x - 1;

#if 0
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);
#endif

#ifdef TILESIZECOLLIDER
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//t = tile position
	int tminx = cmminx / TILE_SIZE;
	int tminy = cmminy / TILE_SIZE;
	int tmaxx = cmmaxx / TILE_SIZE;
	int tmaxy = cmmaxy / TILE_SIZE;

	unsigned short fi = this - g_foliage;

	for(int ty = tminy; ty <= tmaxy; ty++)
		for(int tx = tminx; tx <= tmaxx; tx++)
		{
			ColliderTile* c = ColliderAt(tx, ty);

			auto fit=c->foliages.begin();
			while(fit!=c->foliages.end())
			{
				if(*fit != fi)
				{
					fit++;
					continue;
				}

				fit = c->foliages.erase(fit);
			}
		}
#elif 0
	//c = cell position
	int nminx = imax(0, cmminx / PATHNODE_SIZE);
	int nminy = imax(0, cmminy / PATHNODE_SIZE);
	int nmaxx = imin(g_pathdim.x-1, cmmaxx / PATHNODE_SIZE);
	int nmaxy = imin(g_pathdim.y-1, cmmaxy / PATHNODE_SIZE);

	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);
			c->foliage = USHRT_MAX;
		}
#else
	int nx = cmpos.x / PATHNODE_SIZE;
	int ny = cmpos.y / PATHNODE_SIZE;
	ColliderTile* c = ColliderAt(nx, ny);
	c->foliage = USHRT_MAX;
#endif
}

void Unit::fillcollider()
{
	UType* t = &g_utype[type];
	int ui = this - g_unit;

	//cm = centimeter position
	int cmminx = cmpos.x - t->size.x/2;
	int cmminy = cmpos.y - t->size.x/2;
	int cmmaxx = cmminx + t->size.x - 1;
	int cmmaxy = cmminy + t->size.x - 1;

	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//c = cell position
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

#ifdef TILESIZECOLLIDER
	//t = tile position
	int tminx = cmminx / TILE_SIZE;
	int tminy = cmminy / TILE_SIZE;
	int tmaxx = cmmaxx / TILE_SIZE;
	int tmaxy = cmmaxy / TILE_SIZE;

	//unsigned short ui = this - g_unit;

	for(int ty = tminy; ty <= tmaxy; ty++)
		for(int tx = tminx; tx <= tmaxx; tx++)
		{
			ColliderTile* c = ColliderAt(tx, ty);
			c->units.push_back( ui );
		}

#else
	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				if(c->units[uiter] < 0)
				{
					c->units[uiter] = ui;
#if 0
					if(ui > 100)
					{
						char msg[128];
						sprintf(msg, "ui > 100 = %d", (int)ui);
						ErrMess(msg,msg);
					}
#endif
					break;
				}
			}
		}
#endif
}

void Building::fillcollider()
{
	BlType* t = &g_bltype[type];
	int bi = this - g_building;

	//t = tile position
	int tminx = tpos.x - t->width.x/2;
	int tminz = tpos.y - t->width.y/2;
	int tmaxx = tminx + t->width.x;
	int tmaxz = tminz + t->width.y;

	//cm = centimeter position
	int cmminx = tminx*TILE_SIZE;
	int cmminy = tminz*TILE_SIZE;
	int cmmaxx = tmaxx*TILE_SIZE - 1;
	int cmmaxy = tmaxz*TILE_SIZE - 1;

	//c = cell position
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);
			c->building = bi;
		}
}

void Unit::freecollider()
{
	UType* t = &g_utype[type];
	int ui = this - g_unit;

	//cm = centimeter position
	int cmminx = cmpos.x - t->size.x/2;
	int cmminy = cmpos.y - t->size.x/2;
	int cmmaxx = cmminx + t->size.x - 1;
	int cmmaxy = cmminy + t->size.x - 1;

	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

#ifdef TILESIZECOLLIDER
	//t = tile position
	int tminx = cmminx / TILE_SIZE;
	int tminy = cmminy / TILE_SIZE;
	int tmaxx = cmmaxx / TILE_SIZE;
	int tmaxy = cmmaxy / TILE_SIZE;

	//unsigned short ui = this - g_unit;

	for(int ty = tminy; ty <= tmaxy; ty++)
		for(int tx = tminx; tx <= tmaxx; tx++)
		{
			ColliderTile* c = ColliderAt(tx, ty);

			auto uit=c->units.begin();
			while(uit!=c->units.end())
			{
				if(*uit != ui)
				{
					uit++;
					continue;
				}

				uit = c->units.erase(uit);
			}
		}
#else
	//c = cell position
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				if(c->units[uiter] == ui)
					c->units[uiter] = -1;
			}
		}
#endif
}

void Building::freecollider()
{
	BlType* t = &g_bltype[type];
	int bi = this - g_building;

	//t = tile position
	int tminx = tpos.x - t->width.x/2;
	int tminy = tpos.y - t->width.y/2;
	int tmaxx = tminx + t->width.x;
	int tmaxy = tminy + t->width.y;

	//cm = centimeter position
	int cmminx = tminx*TILE_SIZE;
	int cmminy = tminy*TILE_SIZE;
	int cmmaxx = tmaxx*TILE_SIZE - 1;
	int cmmaxy = tmaxy*TILE_SIZE - 1;

	//c = cell position
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);

			if(c->building == bi)
				c->building = -1;
		}
}

// Uses cm pos instead of pathnode pos
// Uses cm-accurate intersect checks
bool Standable2(const PathJob* pj, int cmposx, int cmposy)
{
#ifdef TILESIZECOLLIDER
	const int tx = cmposx / TILE_SIZE;
	const int ty = cmposy / TILE_SIZE;

	ColliderTile* cell = ColliderAt( tx, ty );
#else
	const int nx = cmposx / PATHNODE_SIZE;
	const int ny = cmposy / PATHNODE_SIZE;

#if 0
	if(nx < 0 || ny < 0 || nx >= g_pathdim.x || ny >= g_pathdim.y)
		return false;
#endif

	ColliderTile* cell = ColliderAt( nx, ny );
#endif

#if 1
	if(cell->flags & FLAG_ABRUPT)
	{
		//Log("abrupt");
		return false;
	}

	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{
		//Log("!land flag="<<(cell->flags & FLAG_HASLAND)<<"="<<(unsigned int)cell->flags);
		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		//Log("!sea");
		return false;
	}
#endif

	UType* ut = &g_utype[pj->utype];

	int cmminx = cmposx - ut->size.x/2;
	int cmminy = cmposy - ut->size.x/2;
	int cmmaxx = cmminx + ut->size.x - 1;
	int cmmaxy = cmminy + ut->size.x - 1;

#ifndef TILESIZECOLLIDER
	int nminx = cmminx/PATHNODE_SIZE;
	int nminy = cmminy/PATHNODE_SIZE;
	int nmaxx = cmmaxx/PATHNODE_SIZE;
	int nmaxy = cmmaxy/PATHNODE_SIZE;
#else
	int tminx = cmminx /. TILE_SIZE;
	int tminy = cmminy /. TILE_SIZE;
	int tmaxx = cmmaxx /. TILE_SIZE;
	int tmaxy = cmmaxy /. TILE_SIZE;
#endif

#if 0
	if(nminx < 0 || nminy < 0 || nmaxx >= g_pathdim.x || nmaxy >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;
	bool bcollided = false;
	bool hasroad = false;

	CdTile* ctile = GetCd(CD_ROAD, 
		cmposx / TILE_SIZE, 
		cmposy / TILE_SIZE, 
		false);

	if(ctile->on && ctile->finished)
		hasroad = true;

#ifdef TILESIZECOLLIDER
	for(int y=tminy; y<=tmaxy; y++)
		for(int x=tminx; x<=tmaxx; x++)
#else
	for(int y=nminy; y<=nmaxy; y++)
		for(int x=nminx; x<=nmaxx; x++)
#endif
		{
			cell = ColliderAt(x, y);
			
#if 0
			CdTile* ctile = GetCd(CD_ROAD, 
				x * PATHNODE_SIZE / TILE_SIZE, 
				y * PATHNODE_SIZE / TILE_SIZE, 
				false);

			if(ctile->on && ctile->finished)
				hasroad = true;
#endif

#ifdef TILESIZECOLLIDER
			for(auto fit=cell->foliages.begin(); fit!=cell->foliages.end(); fit++)
			{
				Foliage* f = &g_foliage[*fit];
				FlType* ft = &g_foltype[f->type];

				int fcmminx = f->cmpos.x - ft->size.x/2;
				int fcmminy = f->cmpos.y - ft->size.y/2;
				int fcmmaxx = fcmminx + ft->size.x;
				int fcmmaxy = fcmminy + ft->size.y;

				if(fcmminx > cmmaxx)
					continue;

				if(fcmminy > cmmaxy)
					continue;

				if(fcmmaxx < cmminx)
					continue;

				if(fcmmaxy < cmminy)
					continue;

				collided = true;
				break;
			}
#else
			if(cell->foliage != USHRT_MAX)
				collided = true;
#endif

#ifdef TILESIZECOLLIDER
			if(cell->building >= 0)
			{
				collided = true;

				if(cell->building != pj->ignoreb)
				{
					bcollided = true;
					return false;
				}
				else
					ignoredb = true;
			}
#else
			if(cell->building >= 0)
			{
				Building* b = &g_building[cell->building];
				BlType* t2 = &g_bltype[b->type];

				int tminx = b->tpos.x - t2->width.x/2;
				int tminy = b->tpos.y - t2->width.y/2;
				int tmaxx = tminx + t2->width.x;
				int tmaxy = tminy + t2->width.y;

				int minx2 = tminx*TILE_SIZE;
				int minz2 = tminy*TILE_SIZE;
				int maxx2 = tmaxx*TILE_SIZE - 1;
				int maxz2 = tmaxy*TILE_SIZE - 1;

				if(cmminx <= maxx2 && cmminy <= maxz2 && cmmaxx >= minx2 && cmmaxy >= minz2)
				{
					//Log("bld");
					collided = true;

					if(cell->building != pj->ignoreb)
					{
						bcollided = true;
						return false;
					}
					else
						ignoredb = true;
				}
			}
#endif

#ifdef TILESIZECOLLIDER
			for(auto uit=cell->units.begin(); uit!=cell->units.end(); uit++)
			{
				short uin = *uit;

				Unit* u = &g_unit[uin];

				if(uin != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{
#if 1
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminy2 = u->cmpos.y - t->size.x/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{
						//Log("u");
						//return false;
						collided = true;

						if(uin == pj->ignoreu)
							ignoredu = true;
					}
#else
					return false;
#endif
				}
			}
#else
			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{
#if 1
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminy2 = u->cmpos.y - t->size.x/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{
						//Log("u");
						//return false;
						collided = true;

						if(uindex == pj->ignoreu)
							ignoredu = true;
					}
#else
					return false;
#endif
				}
			}
		}
#endif

	bool arrived = false;

	if(cmminx < pj->goalmaxx &&
		cmminy < pj->goalmaxy &&
		cmmaxx > pj->goalminx &&
		cmmaxy > pj->goalminy)
		arrived = true;

//#ifndef HIERPATH
	//needs to be not set to false for HIERPATH because otherwise can only path to exact cm pos for tile goal (?)
	arrived = false;	//ditto
//#endif

	if(pj->roaded)
	{
		//CdTile* cdtile = GetCd(CD_ROAD, cmposx / TILE_SIZE, cmposy / TILE_SIZE, false);
		//if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu && (!arrived || bcollided))
		//Log("!road");
		if(!hasroad && !ignoredb && !ignoredu && (!arrived || bcollided))
			return false;
	}

	if(collided && 
		!ignoredb && !ignoredu 
#ifndef HIERPATH
		&& (!arrived || bcollided)
#endif
		)
		return false;

	return true;
}

//#define TRANSPORT_DEBUG
//#define POWCD_DEBUG

bool Standable(const PathJob* pj, const int nx, const int ny)
{
#ifdef TILESIZECOLLIDER
	int tx = nx * PATHNODE_SIZE / TILE_SIZE;
	int ty = ny * PATHNODE_SIZE / TILE_SIZE;

	ColliderTile* cell = ColliderAt( tx, ty );
#else
#if 0
	if(nx < 0 || ny < 0 || nx >= g_pathdim.x || ny >= g_pathdim.y)
		return false;
#endif

	ColliderTile* cell = ColliderAt( nx, ny );
#endif

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("stand? u"<<pj->thisu<<" n"<<nx<<","<<ny<<" dn("<<(nx-pj->goalx)<<","<<(ny-pj->goaly)<<")");

		char add[1280];
		sprintf(add, "std? u%d n%d,%d dn%d,%d \r\n", (int)pj->thisu, (int)nx, (int)ny, (int)(nx-pj->goalx), (int)(ny-pj->goaly));
		powcdstr += add;
	}
#endif

#if 1
	if(cell->flags & FLAG_ABRUPT)
		return false;

	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t !land");
		char add[1280] = "\t !land \r\n";
		powcdstr += add;
	}
#endif

		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		return false;
	}
#endif

	const UType* ut = &g_utype[pj->utype];

	const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
	const int cmposy = ny * PATHNODE_SIZE + PATHNODE_SIZE/2;

	const int cmminx = cmposx - ut->size.x/2;
	const int cmminy = cmposy - ut->size.x/2;
	const int cmmaxx = cmminx + ut->size.x - 1;
	const int cmmaxy = cmminy + ut->size.x - 1;

#ifndef TILESIZECOLLIDER
	int nminx = cmminx/PATHNODE_SIZE;
	int nminy = cmminy/PATHNODE_SIZE;
	int nmaxx = cmmaxx/PATHNODE_SIZE;
	int nmaxy = cmmaxy/PATHNODE_SIZE;
#else
	int tminx = cmminx /. TILE_SIZE;
	int tminy = cmminy /. TILE_SIZE;
	int tmaxx = cmmaxx /. TILE_SIZE;
	int tmaxy = cmmaxy /. TILE_SIZE;
#endif

#if 0
	//Done more efficiently in PathJob::process()
	if(pj->pjtype == PATHJOB_BOUNDJPS ||
		pj->pjtype == PATHJOB_BOUNDASTAR)
	{
		if(nx < pj->nminx ||
			nx > pj->nmaxx ||
			ny < pj->nminy ||
			ny > pj->nmaxy)
			return false;
	}
#endif

#if 0
	if(nminx < 0 || nminy < 0 || nmaxx >= g_pathdim.x || nmaxy >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;
	bool bcollided = false;
	bool hasroad = false;

	CdTile* ctile = GetCd(CD_ROAD, 
		cmposx / TILE_SIZE, 
		cmposy / TILE_SIZE, 
		false);

	if(ctile->on && ctile->finished)
		hasroad = true;

#ifdef TILESIZECOLLIDER
	for(int y=tminy; y<=tmaxy; y++)
		for(int x=tminx; x<=tmaxx; x++)
#else
	for(int y=nminy; y<=nmaxy; y++)
		for(int x=nminx; x<=nmaxx; x++)
#endif
		{
			cell = ColliderAt(x, y);

#if 0
			CdTile* ctile = GetCd(CD_ROAD, 
				x * PATHNODE_SIZE / TILE_SIZE, 
				y * PATHNODE_SIZE / TILE_SIZE, 
				false);

			if(ctile->on && ctile->finished)
				hasroad = true;
#endif

#ifdef TILESIZECOLLIDER
			for(auto fit=cell->foliages.begin(); fit!=cell->foliages.end(); fit++)
			{
				Foliage* f = &g_foliage[*fit];
				FlType* ft = &g_foltype[f->type];

				int fcmminx = f->cmpos.x - ft->size.x/2;
				int fcmminy = f->cmpos.y - ft->size.y/2;
				int fcmmaxx = fcmminx + ft->size.x;
				int fcmmaxy = fcmminy + ft->size.y;

				if(fcmminx > cmmaxx)
					continue;

				if(fcmminy > cmmaxy)
					continue;

				if(fcmmaxx < cmminx)
					continue;

				if(fcmmaxy < cmminy)
					continue;

				collided = true;
				break;
			}
#else
			if(cell->foliage != USHRT_MAX)
				collided = true;
#endif

#ifdef TILESIZECOLLIDER
			if(cell->building >= 0)
			{
				collided = true;

				if(cell->building != pj->ignoreb)
				{
					bcollided = true;
					return false;
				}
				else
					ignoredb = true;
			}
#else
			if(cell->building >= 0)
			{
				if(cell->building != pj->ignoreb)
				{


#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t collB");
		char add[1280] = "\t colB \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "Bhit bt=%s b%d ib%d d(%d,%d) u%d", g_bltype[g_building[cell->building].type].name, (int)cell->building, (int)pj->ignoreb, (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						Log(t);
						AddChat(&rt);
					}
#endif

					//return false;
					collided = true;
					bcollided = true;
				}
				else
				{
					ignoredb = true;


#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "ignoreB d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						Log(t);
						AddChat(&rt);
					}
#endif
				}
			}
#endif

#ifdef TILESIZECOLLIDER
			for(auto uit=cell->units.begin(); uit!=cell->units.end(); uit++)
			{
				short uin = *uit;

				Unit* u = &g_unit[uin];

				if(uin != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{
#if 1
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminy2 = u->cmpos.y - t->size.x/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{
						//Log("u");
						//return false;
						collided = true;

						if(uin == pj->ignoreu)
							ignoredu = true;
					}
#else
					return false;
#endif
				}
			}
#else
			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex == pj->ignoreu)
				{
					ignoredu = true;
				}

				if(uindex != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t collU");
		char add[1280] = "\t colU \r\n";
		powcdstr += add;
	}
#endif

#if 0
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminy2 = u->cmpos.y - t->size.x/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{

						return false;
					}
#else
					//return false;
					collided = true;
#endif
				}
			}
#endif
		}

	bool arrived = false;

	if(cmminx < pj->goalmaxx &&
		cmminy < pj->goalmaxy &&
		cmmaxx > pj->goalminx &&
		cmmaxy > pj->goalminy)
		arrived = true;

	//TODO fix this
	arrived = false;

	/*
	Need to comment out arrived for now for the sake of tile hierarchical pathfinding.
	Otherwise, when PartialPath computes a path around a corner, it will say it is
	okay to collide with a building corner because we arrive at the next tile.
	But it was originally added here to fix trucks being stuck when they were right
	next to a building but clumped up, unable to deliver or recieve loads.
	Edit: experiencing this again after going back to astar from hierarchical.
	Near one building's corner hide to target building, colliding with other bl's corner.
	So the problem is the calling function that expands path nodes checks for
	goal bounds and immediately returns success if goal touched even though there
	is a collision.
	Edit: no, that was not the problem at all, some other bug like Disembark is making
	the truck change spots.
	*/

#if 1
	if(pj->roaded)
	{
#if 0
		CdTile* cdtile = GetCd(CD_ROAD, cmposx / TILE_SIZE, cmposy / TILE_SIZE, false);
		if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu && (!arrived || bcollided))
		//if(!hasroad && !ignoredb && !ignoredu && (!arrived || bcollided))
		//Log("!road");

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t !road");
		char add[1280] = "\t !road \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[pj->thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "!roaded d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
		rt.m_part.push_back(t);
		AddChat(&rt);
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		Log("\t !r");
	}
#endif
#else
		if(!hasroad && !ignoredb && !ignoredu && (!arrived || bcollided))
#endif

		return false;
	}
#endif

	if(collided 
		&& !ignoredu && !ignoredb 
#ifndef HIERPATH
		&& (!arrived || bcollided)
#endif
		)
		return false;

	return true;
}

bool TileStandable(const PathJob* pj, const int nx, const int ny)
{
#if 0
	if(nx < 0 || ny < 0 || nx >= g_pathdim.x || ny >= g_pathdim.y)
		return false;
#endif

#ifdef HIERDEBUG
	if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && ny == 68*TILE_SIZE/PATHNODE_SIZE)
	{
		Log("\t TileStandable? nxy "<<nx<<","<<ny<<" (t "<<(nx*PATHNODE_SIZE/TILE_SIZE)<<","<<(ny*PATHNODE_SIZE/TILE_SIZE)<<")");
	}
#endif

	ColliderTile* cell = ColliderAt( nx, ny );

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("stand? u"<<pj->thisu<<" n"<<nx<<","<<ny<<" dn("<<(nx-pj->goalx)<<","<<(ny-pj->goaly)<<")");

		char add[1280];
		sprintf(add, "std? u%d n%d,%d dn%d,%d \r\n", (int)pj->thisu, (int)nx, (int)ny, (int)(nx-pj->goalx), (int)(ny-pj->goaly));
		powcdstr += add;
	}
#endif

#if 1
	//needed for trucks to drive to unfinished road tiles
	Vec2s tpos = Vec2s(nx, ny) * PATHNODE_SIZE / TILE_SIZE;

	//If at goal...
	unsigned int cmminx = tpos.x * TILE_SIZE;
	unsigned int cmminy = tpos.y * TILE_SIZE;
	unsigned int cmmaxx = (tpos.x+1) * TILE_SIZE - 1;
	unsigned int cmmaxy = (tpos.y+1) * TILE_SIZE - 1;
	
#if 0
	{
		const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		const int cmposy = ny * PATHNODE_SIZE + PATHNODE_SIZE/2;
	
		if(pj->thisu == 36)
		{
			int dtx = (int)(cmposx/TILE_SIZE-pj->goalx);
			int dty = (int)(cmposy/TILE_SIZE-pj->goaly);
			char cm[128];
			sprintf(cm, "36 at dt%d,%d", dtx, dty);

			if(dtx == 0 && dty < 2)
				AddChat(&RichText(cm));
		}
	}
#endif

	if(cmminx <= pj->goalmaxx &&
		cmminy <= pj->goalmaxy &&
		cmmaxx >= pj->goalminx &&
		cmmaxy >= pj->goalminy)
	{
		
#if 0
	{
		const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
		const int cmposy = ny * PATHNODE_SIZE + PATHNODE_SIZE/2;
		if(pj->thisu == 36)
		{
			int dtx = (int)(cmposx/TILE_SIZE-pj->goalx);
			int dty = (int)(cmposy/TILE_SIZE-pj->goaly);
			char cm[128];
			sprintf(cm, "ARR 36 at dt%d,%d", dtx, dty);

			if(dtx == 0 && dty < 2)
				AddChat(&RichText(cm));
		}
	}
#endif

		return true;
	}
#endif

#if 1
	if(cell->flags & FLAG_ABRUPT)
		return false;

	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t !land");
		char add[1280] = "\t !land \r\n";
		powcdstr += add;
	}
#endif

		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		return false;
	}
#endif

	const UType* ut = &g_utype[pj->utype];

	const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
	const int cmposy = ny * PATHNODE_SIZE + PATHNODE_SIZE/2;

#if 0
	const int cmminx = cmposx - ut->size.x/2;
	const int cmminy = cmposy - ut->size.x/2;
	const int cmmaxx = cmminx + ut->size.x - 1;
	const int cmmaxy = cmminy + ut->size.x - 1;

	const int nminx = cmminx/PATHNODE_SIZE;
	const int nminy = cmminy/PATHNODE_SIZE;
	const int nmaxx = cmmaxx/PATHNODE_SIZE;
	const int nmaxy = cmmaxy/PATHNODE_SIZE;
#endif

#if 0
	if(nminx < 0 || nminy < 0 || nmaxx >= g_pathdim.x || nmaxy >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;

#if 0
	for(int y=nminy; y<=nmaxy; y++)
		for(int x=nminx; x<=nmaxx; x++)
#else
	int y = ny;
	int x = nx;
#endif
		{
			cell = ColliderAt(x, y);

			//if(cell->foliage != USHRT_MAX)
			//	collided = true;


#ifdef HIERDEBUG
			if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && ny == 68*TILE_SIZE/PATHNODE_SIZE)
				Log("cb = "<<cell->building);
#endif

			if(cell->building >= 0)
			{

#ifdef HIERDEBUG
				if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && ny == 68*TILE_SIZE/PATHNODE_SIZE)
					Log("ib = "<<(pj->ignoreb)<<" cb = "<<cell->building);
#endif

				if(cell->building != pj->ignoreb)
				{


#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t collB");
		char add[1280] = "\t colB \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "Bhit bt=%s b%d ib%d d(%d,%d) u%d", g_bltype[g_building[cell->building].type].name, (int)cell->building, (int)pj->ignoreb, (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						Log(t);
						AddChat(&rt);
					}
#endif

					//return false;
					collided = true;
				}
				else
				{
					ignoredb = true;


#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "ignoreB d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						Log(t);
						AddChat(&rt);
					}
#endif
				}
			}

#if 0
			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex == pj->ignoreu)
				{
					ignoredu = true;
				}

				if(uindex != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t collU");
		char add[1280] = "\t colU \r\n";
		powcdstr += add;
	}
#endif

#if 0
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminy2 = u->cmpos.y - t->size.x/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{

						return false;
					}
#else
					//return false;
					collided = true;
#endif
				}
			}
#endif
		}

	if(pj->roaded)
	{
		CdTile* cdtile = GetCd(CD_ROAD, cmposx / TILE_SIZE, cmposy / TILE_SIZE, false);
		if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu)
		//Log("!road");
			return false;

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t !road");
		char add[1280] = "\t !road \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[pj->thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "!roaded d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
		rt.m_part.push_back(t);
		AddChat(&rt);
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		Log("\t !r");
	}
#endif

		//return false;
	}

	if(collided && !ignoredu && !ignoredb)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && ny
#endif

	return true;
}
