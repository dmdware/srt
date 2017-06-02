


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




#include "build.h"
#include "bltype.h"
#include "../render/shader.h"
#include "../platform.h"
#include "../window.h"
#include "../math/camera.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "building.h"
#include "../utils.h"
#include "utype.h"
#include "unit.h"
#include "../render/water.h"
#include "../phys/collision.h"
#include "../gui/richtext.h"
#include "../gui/font.h"
#include "../math/vec4f.h"
#include "../gui/icon.h"
#include "player.h"
#include "../app/appmain.h"
#include "../gui/gui.h"
#include "../gui/widget.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../render/foliage.h"
#include "umove.h"
#include "simdef.h"
#include "simflow.h"
#include "../math/fixmath.h"
#include "../path/pathnode.h"
#include "job.h"
#include "../render/drawsort.h"
#include "map.h"
#include "../render/fogofwar.h"

void UpdSBl()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	if(g_build == BL_NONE)
		return;

	//g_vdrag[0] = Vec3f(-1,-1,-1);
	//g_vdrag[1] = Vec3f(-1,-1,-1);

#if 0
	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersect;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;
#else
	Vec3f ray;
	Vec3f point;
	IsoToCart(g_mouse+g_scroll, &ray, &point);
	Vec3i intersect;
    Vec3f fint;
    Vec3f line[2];
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	//if(!MapInter(&g_hmap, ray, point, &intersect))
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		return;
    intersect = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
#endif

	if(!g_mousekeys[MOUSE_LEFT])
		g_vdrag[0] = intersect;
	else
		g_vdrag[1] = intersect;

	g_canplace = true;

	if(g_build < BL_TYPES)
	{
		Vec2i tpos (intersect.x/TILE_SIZE, intersect.y/TILE_SIZE);

		g_vdrag[0].x = tpos.x * TILE_SIZE;
		g_vdrag[0].y = tpos.y * TILE_SIZE;

		BlType* t = &g_bltype[g_build];

		if(t->width.x%2 == 1)
			g_vdrag[0].x += TILE_SIZE/2;
		if(t->width.y%2 == 1)
			g_vdrag[0].y += TILE_SIZE/2;

		//g_vdrag[0].y = Lowest(tpos.x, tpos.y, tpos.x, tpos.y);

		if(!CheckCanPlace(g_build, tpos, -1))
		{
			g_canplace = false;
			g_bpcol = g_collidertype;
		}
		//PlaceBl(type, tpos, true, -1, -1, -1);
	}
	else if(g_build >= BL_TYPES && g_build < BL_TYPES+CD_TYPES)
	{
		int ctype = g_build - BL_TYPES;

		if(g_mousekeys[0])
			UpdCdPlans(ctype, g_localP, g_vdrag[0], g_vdrag[1]);
		else
			UpdCdPlans(ctype, g_localP, g_vdrag[0], g_vdrag[0]);
	}
}

void DrawSBl()
{
	Player* py = &g_player[g_localP];

	if(g_build == BL_NONE)
		return;

	Shader* s = &g_shader[g_curS];

	if(g_build < BL_TYPES)
	{
		//Log("dr");

		if(g_canplace)
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
		else
			glUniform4f(s->slot[SSLOT_COLOR], 1, 0, 0, 0.5f);
		//m->draw(0, g_vdrag[0], 0);

		Sprite* sp = NULL;

		BlType* t = &g_bltype[g_build];
		Vec2i tpos = Vec2i((int)g_vdrag[0].x / TILE_SIZE, (int)g_vdrag[0].y / TILE_SIZE);

		//Vec3f vmin(b->drawpos.x - t->width.x*TILE_SIZE/2, b->drawpos.y, b->drawpos.y - t->width.y*TILE_SIZE/2);
		//Vec3f vmax(b->drawpos.x + t->width.x*TILE_SIZE/2, b->drawpos.y + (t->width.x+t->width.y)*TILE_SIZE/2, b->drawpos.y + t->width.y*TILE_SIZE/2);

		//if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.y, vmax.x, vmax.y, vmax.y))
		//	continue;

		unsigned int* spi = NULL;

		//if(b->finished)
			spi = t->sprite;
		//else
		//	spi = t->csprite;

		Tile& surf = SurfTile(tpos.x, tpos.y);

		if(t->hugterr)
		{
			unsigned char inc = surf.incltype;
			sp = &g_sprite[ spi[inc] ];
		}
		else
		{
			//TODO frame
			sp = &g_sprite[ spi[0] ];
		}

		Player* py = &g_player[g_localP];
		float* color = py->color;
		glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

		Vec3i cmpos = Vec3i( tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0), tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0), surf.elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;
		
		Texture* difftex = &g_texture[ sp->difftexi ];
		Texture* depthtex = &g_texture[ sp->depthtexi ];

		int rendz;

		CartToDepth(cmpos, &rendz);

#if 0
		DrawImage(tex->texname,
			(float)screenpos.x + sp->offset[0],
			(float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2],
			(float)screenpos.y + sp->offset[3], 
			0,0,1,1, g_gui.m_crop);
#else	
		DrawDeep(difftex->texname, depthtex->texname, rendz,
			(float)screenpos.x + sp->cropoff[0],
			(float)screenpos.y + sp->cropoff[1],
			(float)screenpos.x + sp->cropoff[2],
			(float)screenpos.y + sp->cropoff[3], 
			sp->crop[0], sp->crop[1], 
			sp->crop[2], sp->crop[3]);
#endif
	}
	else if(g_build == BL_ROAD)
	{
	}
	else if(g_build == BL_POWL)
	{
	}
	else if(g_build == BL_CRPIPE)
	{
	}
}

void DrawBReason(Matrix* mvp, float width, float height, bool persp)
{
	Player* py = &g_player[g_localP];

	if(g_canplace || g_build == BL_NONE)
		return;

	Vec3f pos3 = Vec3f(g_vdrag[0].x, g_vdrag[0].y, g_vdrag[0].y);

	if(g_build >= BL_TYPES)
		pos3 = Vec3f(g_vdrag[1].x, g_vdrag[1].y, g_vdrag[1].y);

	RichText reason;

	Vec4f pos4 = ScreenPos(mvp, pos3, width, height, persp);

	switch(g_bpcol)
	{
	case COLLIDER_NONE:
		reason.m_part.push_back(RichPart(UStr("")));
		break;
	case COLLIDER_UNIT:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" A unit is in the way.")));
		break;
	case COLLIDER_BUILDING:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" Another building is in the way.")));
		break;
	case COLLIDER_TERRAIN:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" Buildings must be placed on even terrain.")));
		break;
	case COLLIDER_NOROAD:
		reason.m_part.push_back(RichPart(UStr("")));
		break;
	case COLLIDER_OTHER:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" Can't place here.")));
		break;
	case COLLIDER_NOLAND:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" This building must be placed on land.")));
		break;
	case COLLIDER_NOSEA:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" This structure must be placed in the sea.")));
		break;
	case COLLIDER_NOCOAST:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" This building must be placed along the coast.")));
		break;
	case COLLIDER_ROAD:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" A road is in the way.")));
		break;
	case COLLIDER_OFFMAP:
		reason.m_part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.m_part.push_back(RichPart(UStr(" Building is out of bounds.")));
		break;
	}

	float color[] = {0.9f,0.7f,0.2f,0.8f};
	//DrawCenterShadText(MAINFONT32, pos4.x, pos4.y-64, &reason, color, -1);
	DrawBoxShadText(MAINFONT32, pos4.x-200, pos4.y-64-100, 400, 200, &reason, color, 0, -1);
}

bool BlLevel(int type, Vec2i tpos)
{
#if 1
	BlType* t = &g_bltype[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = tpos.x - t->width.x/2;
	tmin.y = tpos.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x;
	tmax.y = tmin.y + t->width.y;

	float miny = g_hmap.getheight(tmin.x, tmin.y);
	float maxy = g_hmap.getheight(tmin.x, tmin.y);

	bool haswater = false;
	bool hasland = false;

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
		{
			float thisy = g_hmap.getheight(x, y);

			if(thisy < miny)
				miny = thisy;

			if(thisy > maxy)
				maxy = thisy;

			// Must have two adject water tiles to be water-vessel-accessible
			if(thisy < WATER_LEVEL)
			{
				// If y is along building edge and x and x+1 are water tiles
				if((y==tmin.y || y==tmax.y) && x+1 <= g_mapsz.x && x+1 <= tmax.x && g_hmap.getheight(x+1, y) < WATER_LEVEL)
					haswater = true;
				// If x is along building edge and y and y+1 are water tiles
				if((x==tmin.x || x==tmax.x) && y+1 <= g_mapsz.y && y+1 <= tmax.y && g_hmap.getheight(x, y+1) < WATER_LEVEL)
					haswater = true;
			}
			// Must have two adjacent land tiles to be road-accessible
			else if(thisy > WATER_LEVEL)
			{
				// If y is along building edge and x and x+1 are land tiles
				if((y==tmin.y || y==tmax.y) && x+1 <= g_mapsz.x && x+1 <= tmax.x && g_hmap.getheight(x+1, y) > WATER_LEVEL)
					hasland = true;
				// If x is along building edge and y and y+1 are land tiles
				if((x==tmin.x || x==tmax.x) && y+1 <= g_mapsz.y && y+1 <= tmax.y && g_hmap.getheight(x, y+1) > WATER_LEVEL)
					hasland = true;
			}
		}

	if(miny != maxy && !t->hugterr)
		return false;

	if(miny < WATER_LEVEL)
	{
#if 0
		haswater = true;
#endif
		miny = WATER_LEVEL;
	}

#if 0
	if(maxy > WATER_LEVEL)
		hasland = true;
#endif

	if(maxy - miny > MAX_CLIMB_INCLINE)
	{
		g_collidertype = COLLIDER_TERRAIN;
		return false;
	}

	if(t->foundation == FD_LAND)
	{
		if(haswater)
		{
			g_collidertype = COLLIDER_NOLAND;
			return false;
		}
		if(!hasland)
		{
			g_collidertype = COLLIDER_NOLAND;
			return false;
		}
	}
	else if(t->foundation == FD_SEA)
	{
		if(!haswater)
		{
			g_collidertype = COLLIDER_NOSEA;
			return false;
		}
		if(hasland)
		{
			g_collidertype = COLLIDER_NOSEA;
			return false;
		}
	}
	else if(t->foundation == FD_COAST)
	{
		if(!haswater || !hasland)
		{
			g_collidertype = COLLIDER_NOCOAST;
			return false;
		}
	}

#if 0
	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
		{
			if(g_hmap.getheight(x, y) != compare)
				return false;

			if(g_hmap.getheight(x, y) <= WATER_LEVEL)
				return false;
		}
#endif
#endif

	return true;
}

bool Offmap(int minx, int miny, int maxx, int maxy)
{
	if(minx < 0 || miny < 0
	                || maxx >= g_mapsz.x*TILE_SIZE
	                || maxy >= g_mapsz.y*TILE_SIZE)
	{
		g_collidertype = COLLIDER_OFFMAP;
		return true;
	}

	return false;
}

bool Collides(Vec2i cmmin, Vec2i cmmax, int ignoreb)
{
	if(Offmap(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return true;

	Vec2i tmin = cmmin / TILE_SIZE;
	Vec2i tmax = cmmax / TILE_SIZE;

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
			if(GetCd(CD_ROAD, x, y, false)->on)
			{
				g_collidertype = COLLIDER_ROAD;
				return true;
			}

		//return false;

	if(CollidesWithBuildings(cmmin.x, cmmin.y, cmmax.x, cmmax.y, ignoreb))
		return true;

	if(CollidesWithUnits(cmmin.x, cmmin.y, cmmax.x, cmmax.y, false, NULL, NULL))
		return true;

	if(CollidesWithTerr(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return true;

	return false;
}

bool BlCollides(int type, Vec2i tpos, int ignoreb)
{
	BlType* t = &g_bltype[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = tpos.x - t->width.x/2;
	tmin.y = tpos.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x - 1;
	tmax.y = tmin.y + t->width.y - 1;

	Vec2i cmmin;
	Vec2i cmmax;

	cmmin.x = tmin.x * TILE_SIZE;
	cmmin.y = tmin.y * TILE_SIZE;
	cmmax.x = cmmin.x + t->width.x*TILE_SIZE - 1;
	cmmax.y = cmmin.y + t->width.y*TILE_SIZE - 1;

	if(Offmap(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return true;

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
			if(GetCd(CD_ROAD, x, y, false)->on)
			{
				g_collidertype = COLLIDER_ROAD;
				return true;
			}

		//return false;

	if(CollidesWithBuildings(cmmin.x, cmmin.y, cmmax.x, cmmax.y, ignoreb))
		return true;

	if(CollidesWithUnits(cmmin.x, cmmin.y, cmmax.x, cmmax.y, false, NULL, NULL))
		return true;

	if(CollidesWithTerr(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return true;

	return false;
}

bool CheckCanPlace(int type, Vec2i tpos, int ignoreb)
{
	if(!BlLevel(type, tpos))
		return false;

	if(BlCollides(type, tpos, ignoreb))
		return false;

	return true;
}

/*
 Check each building, conduit, unit if they can still
 stand with given elevation change. If not, remove them.
 */

void RecheckStand()
{
    for(int i=0; i<BUILDINGS; i++)
    {
        Building* b = &g_building[i];
        
        if(!b->on)
            continue;
        
        if(CheckCanPlace(b->type, b->tpos, i))
            continue;
        
        b->destroy();
        b->on = false;
    }
    
    for(int ctype=0; ctype<CD_TYPES; ctype++)
        for(int tx=0; tx<g_mapsz.x; tx++)
            for(int ty=0; ty<g_mapsz.y; ty++)
            {
                CdTile* ctile = GetCd(ctype, tx, ty, false);
                if(!ctile->on)
                    continue;
                if(CdLevel(ctype, tx, ty, tx, ty, 0, 0, 0, 1, true))
                    continue;
                ctile->destroy();
                ctile->on = false;
                ConnectCdAround(ctype, tx, ty, false);
            }
    
    for(int i=0; i<UNITS; i++)
    {
        Unit* u = &g_unit[i];
        
        if(!u->on)
            continue;
        
        if(!UnitCollides(u, u->cmpos, u->type))
            continue;
        
        u->destroy();
        u->on = false;
    }
    
    for(int i=0; i<CD_TYPES; i++)
        ReNetw(i);
}

bool PlaceBl(int type, Vec2i pos, bool finished, int owner, int* bid)
{
	int i = NewBl();

	if(bid)
		*bid = i;

	if(i < 0)
		return false;

	Building* b = &g_building[i];
	b->on = true;
	b->type = type;
	b->tpos = pos;

	BlType* t = &g_bltype[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = pos.x - t->width.x/2;
	tmin.y = pos.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x;
	tmax.y = tmin.y + t->width.y;

#if 0
	b->drawpos = Vec3f(
		pos.x*TILE_SIZE,
		pos.y*TILE_SIZE,
		0.0f);
#endif

	//if(t->foundation == FD_SEA)
	//	b->drawpos.y = WATER_LEVEL;

#if 1
	if(t->width.x % 2 == 1)
		b->drawpos.x += TILE_SIZE/2;

	if(t->width.y % 2 == 1)
		b->drawpos.y += TILE_SIZE/2;

	//b->drawpos.z = Bilerp(&g_hmap, b->drawpos.x, b->drawpos.y);

	b->owner = owner;

	b->finished = finished;

	Zero(b->conmat);
	Zero(b->stocked);
	Zero(b->price);
	b->propprice = 0;
	b->forsale = false;
	b->demolish = false;	//TODO one state var unsigned char instead of all these bools finished, forsale, demolish, construction, whatev. or bit field.
	for(int ui=0; ui<UNIT_TYPES; ui++)
		b->manufprc[ui] = 0;
	for(int ri=0; ri<RESOURCES; ++ri)
		b->price[ri] = t->price[ri];

	b->occupier.clear();
	b->worker.clear();
	b->conwage = DEFL_CSWAGE;
	b->opwage = t->opwage;
	b->cydelay = SIM_FRAME_RATE * 60;	//TODO variable length cy delay for farms etc.
	b->cymet = 0;
	b->lastcy = g_simframe;
	b->prodlevel = RATIO_DENOM;
	b->capsup.clear();

	for(signed char ri=0; ri<RESOURCES; ri++)
		b->transporter[ri] = -1;

	b->hp = 1;
	
	b->cyclehist.push_back(CycleHist());

#if 0
	int cmminx = tmin.x*TILE_SIZE;
	int cmminy = tmin.y*TILE_SIZE;
	int cmmaxx = cmminx + t->width.x*TILE_SIZE;
	int cmmaxy = cmminy + t->width.y*TILE_SIZE;

	ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);
#endif
	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		PruneCd(ctype);
		ReNetw(ctype);
	}
#if 0
	ClearPowerlines(cmminx, cmminy, cmmaxx, cmmaxy);
	ClearPipelines(cmminx, cmminy, cmmaxx, cmmaxy);
	RePow();
	RePipe();
	ReRoadNetw();
#endif

#endif

	b->fillcollider();

	g_drawlist.push_back(Depthable());
	Depthable* d = &*g_drawlist.rbegin();
	b->depth = d;
	d->dtype = DEPTH_BL;
	d->index = i;
	UpDraw(b);

	AddVis(b);
	Explore(b);

	if(g_mode == APPMODE_PLAY)
	{
		b->allocres();
		b->inoperation = false;

		if(!b->finished && owner == g_localP)
		{
			Player* py = &g_player[g_localP];

			ClearSel(&g_sel);
			g_sel.buildings.push_back(i);

			GUI* gui = &g_gui;
			CstrView* cv = (CstrView*)gui->get("cstr view");
			cv->regen(&g_sel);
			gui->show("cstr view");

			NewJob(UMODE_GOCSTJOB, i, -1, CD_NONE);
		}
	}
	else
	{
		b->inoperation = true;
	}

	return true;
}

//find to place building about certain tile
bool PlaceBAb(int btype, Vec2i tabout, Vec2i* tplace)
{
	//Log("PlaceBAround "<<player);
	//

	BlType* t = &g_bltype[btype];
	int shell = 1;

	//char msg[128];
	//sprintf(msg, "place b a %f,%f,%f", vAround.x/16, vAround.y/16, vAround.y/16);
	//Chat(msg);

	do
	{
		std::vector<Vec2i> canplace;
		Vec2i ttry;
		int tilex, tilez;
		int left, right, top, bottom;
		left = tabout.x - shell;
		top = tabout.y - shell;
		right = tabout.x + shell;
		bottom = tabout.y + shell;

		canplace.reserve( (right-left)*2/TILE_SIZE + (bottom-top)*2/TILE_SIZE );

		tilez = top;
		for(tilex=left; tilex<right; tilex++)
		{
			ttry = Vec2i(tilex, tilez);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendz = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendz += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilex = right;
		for(tilez=top; tilez<bottom; tilez++)
		{
			ttry = Vec2i(tilex, tilez);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendz = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendz += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilez = bottom;
		for(tilex=right; tilex>left; tilex--)
		{
			ttry = Vec2i(tilex, tilez);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendz = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendz += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilex = left;
		for(tilez=bottom; tilez>top; tilez--)
		{
			ttry = Vec2i(tilex, tilez);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendz = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendz += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		if(canplace.size() > 0)
		{
			//Chat("placing");
			//Log("placeb t="<<btype<<" "<<vTile.x<<","<<vTile.y<<","<<vTile.y<<"("<<(vTile.x/16)<<","<<(vTile.y/16)<<","<<(vTile.y/16)<<")");
			//
			//*tpos = canplace[ rand()%canplace.size() ];
			*tplace = canplace[ 0 ];

			return true;
		}

		//char msg[128];
		//sprintf(msg, "shell %d", shell);
		//Chat(msg);

		shell++;
	} while(shell < g_mapsz.x || shell < g_mapsz.y);

	return false;
}

//find to place unit about certain position
bool PlaceUAb(int utype, Vec2i cmabout, Vec2i* cmplace)
{
	UType* t = &g_utype[utype];
	int shell = 1;
	//important: units must be occupying a single free pathnode,
	//or have a space of free pathnodes about them
	int size = imax(t->size.x, PATHNODE_SIZE)*2;
	//int size = PATHNODE_SIZE * 2;

	do
	{
		std::vector<Vec2i> canplace;
		Vec2i cmtry;
		int cmx, cmz;
		int left, right, top, bottom;
		left = cmabout.x - shell*size;
		top = cmabout.y - shell*size;
		right = cmabout.x + shell*size;
		bottom = cmabout.y + shell*size;

		//canplace.reserve( (right-left)*2/size + (bottom-top)*2/size );

		cmz = top;
		for(cmx=left; cmx<right; cmx+=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//align to boundary. only works for odd multiple of PATHNODE_SIZE.
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendz = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			//if(!CheckCanPlace(btype, cmtry))
			if(UnitCollides(NULL, cmtry, utype))
				continue;
			canplace.push_back(cmtry);
		}

		cmx = right;
		for(cmz=top; cmz<bottom; cmz+=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendz = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			if(UnitCollides(NULL, cmtry, utype))
				continue;
			canplace.push_back(cmtry);
		}

		cmz = bottom;
		for(cmx=right; cmx>left; cmx-=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendz = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			if(UnitCollides(NULL, cmtry, utype))
				continue;
			canplace.push_back(cmtry);
		}

		cmx = left;
		for(cmz=bottom; cmz>top; cmz-=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendz = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendz >= g_mapsz.y * TILE_SIZE)
				continue;

			if(UnitCollides(NULL, cmtry, utype))
				continue;
			canplace.push_back(cmtry);
		}

		if(canplace.size() > 0)
		{
			*cmplace = canplace[ 0 ];

			return true;
		}

		shell++;
	} while(shell < g_mapsz.x || shell < g_mapsz.y);

	return false;
}
