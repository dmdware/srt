


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





#include "drawqueue.h"
#include "shader.h"
#include "tile.h"
#include "sprite.h"
#include "../texture.h"
#include "../platform.h"
#include "../utils.h"
#include "../math/isomath.h"
#include "../sim/map.h"
#include "../gui/gui.h"
#include "tile.h"
#include "../save/savemap.h"
#include "../sim/unit.h"
#include "../debug.h"
#include "heightmap.h"
#include "../sim/building.h"
#include "drawsort.h"
#include "depthable.h"
#include "../sim/build.h"
#include "../math/hmapmath.h"
#include "../path/collidertile.h"
#include "foliage.h"
#include "../gui/font.h"
#include "../sim/player.h"
#include "fogofwar.h"
#include "../sim/border.h"

int rendtrees = 0;

void DrawQueue()
{
    //return;
    
	rendtrees = 0;

	static Vec3i topleft;
	static Vec3i topright;
	static Vec3i bottomleft;
	static Vec3i bottomright;

	Vec3f ray;
	Vec3f point;
    Vec3f fint;
    Vec3f line[2];

	IsoToCart(g_scroll, &ray, &point);
	//if(!MapInter(&g_hmap, ray, point, &topleft))
	//	topleft = Vec3i(0,0,0);
	//MapInter(&g_hmap, ray, point, &topleft);
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		MapBoundsIntersect(g_mapsz, line, &fint);
    topleft = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
    
	IsoToCart(g_scroll+Vec2i(g_width,0), &ray, &point);
	//if(!MapInter(&g_hmap, ray, point, &topright))
	//	topright = Vec3i(g_mapsz.x*TILE_SIZE-1,0,0);
	//MapInter(&g_hmap, ray, point, &topright);
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		MapBoundsIntersect(g_mapsz, line, &fint);
    topright = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
    
	IsoToCart(g_scroll+Vec2i(0,g_height), &ray, &point);
	//if(!MapInter(&g_hmap, ray, point, &bottomleft))
	//	bottomleft = Vec3i(0,g_mapsz.y*TILE_SIZE-1,0);
    //MapInter(&g_hmap, ray, point, &bottomleft);
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		MapBoundsIntersect(g_mapsz, line, &fint);
    bottomleft = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
    
	IsoToCart(g_scroll+Vec2i(g_width,g_height), &ray, &point);
	//if(!MapInter(&g_hmap, ray, point, &bottomright))
	//	bottomright = Vec3i(g_mapsz.x*TILE_SIZE-1,g_mapsz.y*TILE_SIZE-1,0);
    //MapInter(&g_hmap, ray, point, &bottomright);
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		MapBoundsIntersect(g_mapsz, line, &fint);
    bottomright = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);

	int cmminx = imin(topleft.x,imin(topright.x,imin(bottomleft.x,bottomright.x)));
	int cmminy = imin(topleft.y,imin(topright.y,imin(bottomleft.y,bottomright.y)));
	int cmmaxx = imax(topleft.x,imax(topright.x,imax(bottomleft.x,bottomright.x)));
	int cmmaxy = imax(topleft.y,imax(topright.y,imax(bottomleft.y,bottomright.y)));

	cmminx = imax(0, imin(g_mapsz.x*TILE_SIZE-1, cmminx));
	cmminy = imax(0, imin(g_mapsz.y*TILE_SIZE-1, cmminy));
	cmmaxx = imax(0, imin(g_mapsz.x*TILE_SIZE-1, cmmaxx));
	cmmaxy = imax(0, imin(g_mapsz.y*TILE_SIZE-1, cmmaxy));
	
	g_mapview[0] = Vec2i(cmminx, cmminy);
	g_mapview[1] = Vec2i(cmmaxx, cmmaxy);

	Vec2i tmin = Vec2i(cmminx,cmminy)/TILE_SIZE;
	Vec2i tmax = Vec2i(cmmaxx,cmmaxy)/TILE_SIZE;
	
	UseS(SHADER_DEEPORTHO);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

	//for(int x=0; x<g_mapsz.x; x++)
	//	for(int y=0; y<g_mapsz.y; y++)
	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
		{
#if 0
			Vec2i screenpos = CartToIso(Vec3i( x * TILE_SIZE, y * TILE_SIZE / 2, z * TILE_SIZE ));
			DrawImage( g_texture[texindex].texname, screenpos.x, screenpos.y, screenpos.x + TILE_PIXEL_WIDTH, screenpos.y + TILE_PIXEL_WIDTH/2);
#endif
#if 1
			Tile& tile = SurfTile(x, y);
			InType* incltype = &g_intype[tile.incltype];
			//incltype = &g_intype[IN_0000];
			Vec3i cmpos( x * TILE_SIZE + TILE_SIZE/2, y * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE);
			//cmpos.y = 0;
			Vec2i screenpos = CartToIso2(cmpos) - g_scroll;
			//Vec2i screenpos = CartToIso(cmpos);
			//screenpos = CartToIso(Vec3i( x * TILE_SIZE, y * TILE_SIZE / 2, z * TILE_SIZE ));
			Sprite* sp = &g_sprite[incltype->sprite];
			Texture* difftex = &g_texture[sp->difftexi];
			Texture* depthtex = &g_texture[sp->depthtexi];

			//Log("xyz "<<cmpos.x<<","<<cmpos.y<<","<<cmpos.z<<" screen "<<screenpos.x<<","<<screenpos.y);

			if(IsTileVis(g_localP, x, y))
				glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
			else if(Explored(g_localP, x, y))
				glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, 1.0f);
			else
				continue;

			int rendz;

			CartToDepth(cmpos, &rendz);

			//to prevent z-fighting with roads on top of tiles when using 20-cm gradiations of depth
			//rendz += TILE_SIZE;
			//rendz += 25;	//25 centimeters, road depth?
			//rendz += 550;
			//rendz += TILE_SIZE/2;
#if 0
			if(x == 0 && z == 0)
			{

				//Log("00dr "<<(screenpos.x + sprite->offset[0])<<","<<(screenpos.y + sprite->offset[1]));
				Log("tile 00 pos "<<screenpos.x<<","<<screenpos.y<<" hy "<<((int)Height(0,0)*TILE_SIZE/2));
			}
#endif
#if 0
			DrawImage( tex->texname,
				screenpos.x + sprite->offset[0],
				screenpos.y + sprite->offset[1],
				screenpos.x + sprite->offset[2],
				screenpos.y + sprite->offset[3], 
				0,0,1,1, g_gui.m_crop);
#elif 0
			DrawImage( tex->texname, screenpos.x, screenpos.y, screenpos.x + TILE_PIXEL_WIDTH, screenpos.y + TILE_PIXEL_WIDTH/2);
#else	
			DrawDeep(difftex->texname, depthtex->texname, rendz,
				(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
				(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
				sp->crop[0], sp->crop[1],
				sp->crop[2], sp->crop[3]);
#endif
#endif
			CHECKGLERROR();
		}
		
		/*
		necessary for some inclined road tile and
		for buildings with garage roads that go below ground.
		but then what about tree models with roots that won't
		be covered by ground now?
		*/
	//glClear(GL_DEPTH_BUFFER);

	//nmax.x = imin(g_pathdim.x-1,nmax.x);
	//nmax.y = imin(g_pathdim.y-1,nmax.y);

	//TODO get sub list to draw based on PathNodes in view.
	//Right now it goes through all, which is expensive with 6,000+ foliages.

	StartTimer(TIMER_DRAWLIST);

	g_subdrawq.clear();
	std::list<Depthable*> drawlist;

	Vec2i nmin = Vec2i(cmminx,cmminy)/PATHNODE_SIZE;
	Vec2i nmax = Vec2i(cmmaxx,cmmaxy)/PATHNODE_SIZE;

	Vec2i pixmin = g_scroll;
	Vec2i pixmax = g_scroll + Vec2i(g_width,g_height);
		
	for(int x=nmin.x; x<=nmax.x; x++)
		for(int y=nmin.y; y<=nmax.y; y++)
		{
			ColliderTile* cl = ColliderAt(x,y);

#if 0
	unsigned char flags;
	short building;
	short units[MAX_COLLIDER_UNITS];
	unsigned short foliage;
#endif

			//index to unit indices = uii
			for(unsigned char uii=0; uii<MAX_COLLIDER_UNITS; uii++)
			{
				short ui = cl->units[uii];

				if(ui < 0)
					continue;

				Unit* u = &g_unit[ui];
				Depthable* d = u->depth;
				
				if(d->pixmin.x > pixmax.x ||
					d->pixmin.y > pixmax.y ||
					d->pixmax.x < pixmin.x ||
					d->pixmax.y < pixmin.y)
					continue;

				bool added = false;

#if 0
				if(!d)
				{
					char msg[128];
					sprintf(msg, "u %d u->on=%d d NULL n%d,%d,%d,%d t%d,%d,%d,%d", (int)ui, (int)u->on, nmin.x, nmin.y, nmax.x, nmax.y, tmin.x, tmin.y, tmax.x, tmax.y);
					ErrMess(msg,msg);
				}
#endif
				
#if 1
				//for(auto qit=drawlist.begin(); qit!=drawlist.end(); qit++)
				for(auto qit=g_subdrawq.begin(); qit!=g_subdrawq.end(); qit++)
				{
					if(*qit != d)
						continue;

					added = true;
					break;
				}

				if(!added)
#endif
				{
#ifdef PLATFORM_MOBILE
					drawlist.push_back(d);
#else
					g_subdrawq.push_back(d);
					DrawUnit(u, d->rendz);
#endif
				}
			}
		}

#if 0

	DrawBl();
	DrawUnits();

#if 0
	Vec2i screensz;
	screensz.x = Max2Pow(g_width);
	screensz.y = Max2Pow(g_height);
	//DrawImage(g_screentex, 0, 0, screensz.x, screensz.y);
#endif
#else

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
		{
			ColliderTile* cl = ColliderAt(x*TILE_SIZE/PATHNODE_SIZE,y*TILE_SIZE/PATHNODE_SIZE);

	#if 0
			unsigned char flags;
			short building;
			short units[MAX_COLLIDER_UNITS];
			unsigned short foliage;
	#endif

			short bi = cl->building;

			if(bi >= 0)
			{
				Building* b = &g_building[bi];
				Depthable* d = b->depth;

#if 0
				if(!d)
				{
					char msg[128];
					sprintf(msg, "bl %d d NULL", (int)bi);
					ErrMess(msg,msg);
				}
#endif

				if(d->pixmin.x > pixmax.x ||
					d->pixmin.y > pixmax.y ||
					d->pixmax.x < pixmin.x ||
					d->pixmax.y < pixmin.y)
					continue;

				bool added = false;
				
#if 1
				//for(auto qit=drawlist.begin(); qit!=drawlist.end(); qit++)
				for(auto qit=g_subdrawq.begin(); qit!=g_subdrawq.end(); qit++)
				{
					if(*qit != d)
						continue;

					added = true;
					break;
				}

				if(!added)
#endif
				{
#ifdef PLATFORM_MOBILE
					drawlist.push_back(d);
#else
					g_subdrawq.push_back(d);
					DrawBl(b, d->rendz);
#endif
				}
			}

			for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
			{
				CdTile* ctile = GetCd(ctype, x, y, false);

				if(!ctile->on)
					continue;

				Depthable* d = ctile->depth;
				
				if(d->pixmin.x > pixmax.x ||
					d->pixmin.y > pixmax.y ||
					d->pixmax.x < pixmin.x ||
					d->pixmax.y < pixmin.y)
					continue;

#ifdef PLATFORM_MOBILE
				drawlist.push_back(d);
#else
				g_subdrawq.push_back(d);
				DrawCd(ctile, x, y, ctype, false, d->rendz);
#endif

#if 0
				if(!d)
				{
					char msg[128];
					sprintf(msg, "cd t%d xy%d,%d d NULL", (int)ctype, x, y);
					ErrMess(msg,msg);
				}
#endif
			}

			for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
			{
				CdTile* ctile = GetCd(ctype, x, y, true);

				if(!ctile->on)
					continue;

				Depthable* d = ctile->depth;

				if(d->pixmin.x > pixmax.x ||
					d->pixmin.y > pixmax.y ||
					d->pixmax.x < pixmin.x ||
					d->pixmax.y < pixmin.y)
					continue;

#ifdef PLATFORM_MOBILE
				drawlist.push_back(d);
#else
				g_subdrawq.push_back(d);
				DrawCd(ctile, x, y, ctype, true, d->rendz);
#endif
				
#if 0
				if(!d)
				{
					char msg[128];
					sprintf(msg, "cd t%d xy%d,%d d NULL", (int)ctype, x, y);
					ErrMess(msg,msg);
				}
#endif
			}
		}

//#ifndef PLATFORM_MOBILE
#if 1
	for(int x=nmin.x; x<=nmax.x; x++)
		for(int y=nmin.y; y<=nmax.y; y++)
		{
			ColliderTile* cl = ColliderAt(x,y);

#if 1
			unsigned short fi = cl->foliage;

			//if(fi >= 0)
			if(fi != USHRT_MAX)
			{
				Foliage* f = &g_foliage[fi];

				if(!f->on)
					continue;	//corpd fix

				Depthable* d = f->depth;
				
#if 1
				if(d->pixmin.x > pixmax.x ||
					d->pixmin.y > pixmax.y ||
					d->pixmax.x < pixmin.x ||
					d->pixmax.y < pixmin.y)
					continue;
#endif

				bool added = false;
				
#if 0
				//for(auto qit=drawlist.begin(); qit!=drawlist.end(); qit++)
				for(auto qit=g_subdrawq.begin(); qit!=g_subdrawq.end(); qit++)
				{
					if(*qit != d)
						continue;

					added = true;
					break;
				}

				if(!added)
#endif
				{
#ifdef PLATFORM_MOBILE
					drawlist.push_back(d);
#else
					g_subdrawq.push_back(d);
					DrawFol(f, d->rendz);
#endif
				}
			}
#endif
		}
#endif

#if 0
	for(auto qit=g_drawlist.begin(); qit!=g_drawlist.end(); qit++)
	{
		Depthable* d = &*qit;

		if(d->pixmin.x > g_scroll.x + g_width)
			continue;
		if(d->pixmin.y > g_scroll.y + g_height)
			continue;
		if(d->pixmax.x < g_scroll.x)
			continue;
		if(d->pixmax.y < g_scroll.y)
			continue;

		g_subdrawq.push_back(d);
	}
#endif

#ifdef PLATFORM_MOBILE
	//g_subdrawq.sort(CompareDepth);
	DrawSort(drawlist, g_subdrawq);
	//DrawSort2(drawlist, g_subdrawq);
#endif
	
	//Log("---------drawq-------------");

#if 0
	float rendz = 0.0f;

	for(auto qit=g_subdrawq.begin(); qit!=g_subdrawq.end(); qit++)
	{
		Depthable* d = *qit;

		if(d->dtype == DEPTH_FOL)
			//rendtrees++;
			DrawFol(&g_foliage[d->index], rendz);
		else if(d->dtype == DEPTH_BL)
			DrawBl(&g_building[d->index], rendz);
		else if(d->dtype == DEPTH_U)
			DrawUnit(&g_unit[d->index], rendz);
		else if(d->dtype == DEPTH_CD)
		{
			unsigned char tx = d->index % g_mapsz.x;
			unsigned char ty = d->index / g_mapsz.x;
			DrawCd(GetCd(d->cdtype, tx, ty, d->plan), tx, ty, d->cdtype, d->plan, rendz);
		}

		//Log("d->dtype="<<(int)d->dtype<<" max"<<d->cmmax.x<<","<<d->cmmax.y<<","<<d->cmmax.z);
	}
#elif 1
#ifdef PLATFORM_MOBILE
	float div = (float)(g_subdrawq.size() + 1);	//make sure there's enough room in the (0...-1) interval
	//float rendz = -0.0f;
	float rendz = 0.0f;

#ifndef PLATFORM_MOBILE
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//glClearDepth(1.0f);
#endif
	
#if 0
	EndS();
	
	UseS(SHADER_DEEPORTHO);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
#endif
	
	//depth sort overdraw optimization
#ifdef PLATFORM_MOBILE
	for(auto qit=g_subdrawq.begin(); qit!=g_subdrawq.end(); qit++)
#else
	for(auto qit=g_subdrawq.rbegin(); qit!=g_subdrawq.rend(); qit++)
#endif
	{
		Depthable* d = *qit;

		if(d->dtype == DEPTH_FOL)
			//rendtrees++;
			DrawFol(&g_foliage[d->index], rendz);
		else if(d->dtype == DEPTH_BL)
			DrawBl(&g_building[d->index], rendz);
		else if(d->dtype == DEPTH_U)
			DrawUnit(&g_unit[d->index], rendz);
		else if(d->dtype == DEPTH_CD)
		{
			unsigned char tx = d->index % g_mapsz.x;
			unsigned char ty = d->index / g_mapsz.x;
			DrawCd(GetCd(d->cdtype, tx, ty, d->plan), tx, ty, d->cdtype, d->plan, rendz);
		}

		rendz += 1.0f / div;

		//Log("d->dtype="<<(int)d->dtype<<" max"<<d->cmmax.x<<","<<d->cmmax.y<<","<<d->cmmax.z);
	}

#if 0
	glDisable(GL_DEPTH_TEST);
	CHECKGLERROR();
	Ortho(g_width, g_height, 1.0f, 1.0f, 1.0f, 1.0f);
#endif
	
#elif 0
	
	//for(auto qit=drawlist.begin(); qit!=drawlist.end(); qit++)
	for(auto qit=g_subdrawq.begin(); qit!=g_subdrawq.end(); qit++)
	{
		Depthable* d = *qit;

		if(d->dtype == DEPTH_FOL)
			//rendtrees++;
			DrawFol(&g_foliage[d->index], d->rendz);
		else if(d->dtype == DEPTH_BL)
			DrawBl(&g_building[d->index], d->rendz);
		else if(d->dtype == DEPTH_U)
			DrawUnit(&g_unit[d->index], d->rendz);
		else if(d->dtype == DEPTH_CD)
		{
			unsigned char tx = d->index % g_mapsz.x;
			unsigned char ty = d->index / g_mapsz.x;
			DrawCd(GetCd(d->cdtype, tx, ty, d->plan), tx, ty, d->cdtype, d->plan, d->rendz);
		}

		//Log("d->dtype="<<(int)d->dtype<<" max"<<d->cmmax.x<<","<<d->cmmax.y<<","<<d->cmmax.z);
	}
#endif
	
	StopTimer(TIMER_DRAWLIST);

	DrawSBl();

#if 0
	Vec3f point;
	Vec3f ray;

	IsoToCart(Vec2i(g_scroll.x + g_width/2, g_scroll.y + g_height/2), &ray, &point);
	Vec2i screenpos = CartToIso(Vec3i((int)point.x, (int)point.y, (int)point.z)) - g_scroll;

	//IsoToCart(Vec2i(g_width/2, g_height/2), &ray, &point);
	//Vec2i screenpos = CartToIso(Vec3i((int)point.x, (int)point.y, (int)point.z));

	DrawImage(g_texture[0].texname,
		screenpos.x - 100, screenpos.y - 100,
		screenpos.x + 100, screenpos.y + 100);
#endif

	//for(int i=0; i<CD_TYPES; i++)
	//	DrawCd(i);
	
#endif	//PLATFORM_MOBILE
#endif	//0,1

#if 0
	char rtstr[128];
	sprintf(rtstr, "trees %d", rendtrees);
	RichText rt(rtstr);
	DrawLine(MAINFONT32, 10, 30, &rt);
#endif

	EndS();

	//return;

	glDisable(GL_DEPTH_TEST);
	UseS(SHADER_COLOR2D);
	s = &g_shader[g_curS];
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
	DrawBords(tmin, tmax);
	EndS();
}
