


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




#include "../texture.h"
#include "foliage.h"
#include "water.h"
#include "shader.h"
#include "../math/camera.h"
#include "../math/frustum.h"
#include "vertexarray.h"
#include "heightmap.h"
#include "../utils.h"
#include "../window.h"
#include "../sim/player.h"
#include "../path/pathnode.h"
#include "../path/collidertile.h"
#include "../phys/collision.h"
#include "../math/hmapmath.h"
#include "../sim/simflow.h"
#include "../path/fillbodies.h"
#include "drawqueue.h"
#include "drawsort.h"
#include "../sim/map.h"
#include "fogofwar.h"
#include "../algo/random.h"

FlType g_fltype[FL_TYPES];
Foliage g_foliage[FOLIAGES];

Foliage::Foliage()
{
	on = false;
	lastdraw = 0;
	depth = NULL;
}

void DefF(int type, const char* sprelative, Vec3f scale, Vec3f translate, Vec2s size)
{
	FlType* t = &g_fltype[type];
	//QueueTexture(&t->texindex, texrelative, true);
	//QueueModel(&t->model, sprel, scale, translate);
	QueueSprite(sprelative, &t->sprite, false, true);
	t->size = size;
}

int NewFoliage()
{
	for(int i=0; i<FOLIAGES; i++)
	{
		if(!g_foliage[i].on)
			return i;
	}

	return -1;
}

#if 0
void PlaceFol()
{
	if(g_scT < 0)
		return;

	int i = NewFoliage();

	if(i < 0)
		return;

	Foliage* s = &g_foliage[i];
	s->on = true;
	s->pos = g_vMouse;
	s->type = g_scT;
	s->yaw = DEGTORAD((rand()%360));

	if(s->pos.y <= WATER_LEVEL)
		s->on = false;
}
#endif

void DrawFol(Foliage* f, float rendz)
{
	//return;

	StartTimer(TIMER_DRAWFOLIAGE);

	Shader* s = &g_shader[g_curS];

	FlType* t = &g_fltype[f->type];
#if 0
	Vec3f vmin(u->drawpos.x - t->size.x/2, u->drawpos.y, u->drawpos.y - t->size.x/2);
	Vec3f vmax(u->drawpos.x + t->size.x/2, u->drawpos.y + t->size.y, u->drawpos.y + t->size.x/2);

	if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
		continue;

	Player* py = &g_player[u->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

	Model* m = &g_model[t->model];
#endif

	//m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.y);

	int spi = t->sprite;

	//assert( (int)u->frame[BODY_LOWER] < t->nframes );

	Sprite* sp = &g_sprite[spi];
	
	Texture* difftex = &g_texture[sp->difftexi];
	Texture* depthtex = &g_texture[sp->depthtexi];

#if 0
	Vec3i cm3pos;
	cm3pos.x = f->cmpos.x;
	cm3pos.y = f->cmpos.y;
	cm3pos.z = (int)( Bilerp(&g_hmap, f->cmpos.x, f->cmpos.y) * TILE_RISE );
	Vec2i isopos = CartToIso(cm3pos);
	Vec2i screenpos = isopos - g_scroll;

	//Depthable* d = f->depth;

#if 0
	if(g_mouse3d.x + TILE_SIZE > d->cmmin.x &&
		g_mouse3d.x - TILE_SIZE < d->cmmax.x &&
		g_mouse3d.y + TILE_SIZE > d->cmmin.y &&
		g_mouse3d.y - TILE_SIZE < d->cmmax.y)
	{
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 0.5f);
	}
#endif

	DrawImage(difftex->texname,
		screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
		screenpos.x + sp->offset[2], screenpos.y + sp->offset[3]);
#else
	Depthable* d = f->depth;

	short tx = f->cmpos.x / TILE_SIZE;
	short ty = f->cmpos.y / TILE_SIZE;

	float alpha = 1.0f;

	Vec2i screenmin = d->pixmin - g_scroll;
	Vec2i screenmax = d->pixmax - g_scroll;
	
	Vec3i cmpos;
	Vec2i screenpos;

	if(screenmin.x >= g_width)
		goto end;

	if(screenmin.y >= g_height)
		goto end;

	if(screenmax.x < 0)
		goto end;

	if(screenmax.y < 0)
		goto end;

#if 1
	if(g_mouse3d.x + TILE_SIZE > d->cmmin.x &&
		g_mouse3d.x - TILE_SIZE < d->cmmax.x &&
		g_mouse3d.y + TILE_SIZE > d->cmmin.y &&
		g_mouse3d.y - TILE_SIZE < d->cmmax.y)
	{
		//glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 0.5f);
		alpha = 0.5f;
	}
#endif

	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, alpha);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, alpha);
	else
		goto end;

	cmpos.x = f->cmpos.x;
	cmpos.y = f->cmpos.y;
	//cmpos.z = g_hmap.accheight(cmpos.x, cmpos.y) * TILE_RISE;
	cmpos.z = Bilerp(&g_hmap, cmpos.x, cmpos.y) * TILE_RISE;
	screenpos = CartToIso(cmpos) - g_scroll;

#if 0
	DrawImage(difftex->texname,
		(float)(d->pixmin.x - g_scroll.x), (float)(d->pixmin.y - g_scroll.y),
		(float)(d->pixmax.x - g_scroll.x), (float)(d->pixmax.y - g_scroll.y));
#elif 0

	DrawImage(difftex->texname,
		(float)screenmin.x, (float)screenmin.y,
		(float)screenmax.x, (float)screenmax.y);
#else
	DrawDeep(difftex->texname, depthtex->texname, rendz,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	rendtrees++;
#endif

end:
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

	StopTimer(TIMER_DRAWFOLIAGE);
}

bool PlaceFol(int type, Vec3i cmpos)
{
	int i = NewFoliage();

	if(i < 0)
		return false;

	int nx = cmpos.x / PATHNODE_SIZE;
	int ny = cmpos.y / PATHNODE_SIZE;
	ColliderTile* c = ColliderAt(nx, ny);

	if(c->foliage != USHRT_MAX)
		return false;

	Foliage* f = &g_foliage[i];
	f->on = true;
	f->type = type;
	f->cmpos = Vec2i(cmpos.x, cmpos.y);
	f->drawpos = Vec3f(cmpos.x, cmpos.y, cmpos.z);
	//f->yaw = rand()%360;
	f->fillcollider();

	g_drawlist.push_back(Depthable());
	Depthable* d = &*g_drawlist.rbegin();
	d->dtype = DEPTH_FOL;
	d->index = i;
	f->depth = d;
	UpDraw(f);

	return true;
}

void Foliage::destroy()
{
	on = false;
	freecollider();

	for(auto qit=g_drawlist.begin(); qit!=g_drawlist.end(); qit++)
	{
		if(&*qit == depth)
		{
			g_drawlist.erase(qit);
			depth = NULL;
			break;
		}
	}
}

void ClearFol(int cmminx, int cmminy, int cmmaxx, int cmmaxy)
{
#if 0
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		if(f->pos.x >= minx && f->pos.x <= maxx && f->pos.z >= miny && f->pos.z <= maxy)
		{
			f->on = false;
			f->reinstance();
		}
	}
#endif

	//c = cell position
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	//corpc fix
	nminx = imax(0, nminx);
	nminy = imax(0, nminy);
	nmaxx = imin(g_pathdim.x-1, nmaxx);
	nmaxy = imin(g_pathdim.y-1, nmaxy);

	for(int ny = nminy; ny <= nmaxy; ny++)
		for(int nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);

			//if(c->foliage < 0)
			//	continue;

			//corpc fix
			if(c->foliage == USHRT_MAX)
				continue;

			Foliage* f = &g_foliage[c->foliage];
			FlType* ft = &g_fltype[f->type];

			int cmminx2 = f->cmpos.x - ft->size.x/2;
			int cmminy2 = f->cmpos.y - ft->size.x/2;
			int cmmaxx2 = cmminx2 + ft->size.x - 1;
			int cmmaxy2 = cmminy2 + ft->size.x - 1;

			//if(f->cmpos.x >= cmminx && f->cmpos.x <= cmmaxx && f->cmpos.y >= cmminy && f->cmpos.y <= cmmaxy)
			if(cmminx2 <= cmmaxx && cmminy2 <= cmmaxy && cmmaxx2 >= cmminx && cmmaxy2 >= cmminy)
			{
				f->destroy();
			}
		}

	FillBodies();
}

void FillForest(unsigned int r)
{
	//return;	//no trees

	int maxfoliage = FOLIAGES*g_mapsz.x*g_mapsz.y/MAX_MAP/MAX_MAP;
	maxfoliage = imin(FOLIAGES, maxfoliage);

	unsigned int tx = RAND() % g_mapsz.x;
	unsigned int ty = RAND() % g_mapsz.y;
	unsigned int cmx = RAND() % TILE_SIZE;
	unsigned int cmy = RAND() % TILE_SIZE;

	Vec2i last = Vec2i(tx*TILE_SIZE + cmx, ty*TILE_SIZE + cmy);

	for(int i=0; i<maxfoliage; i++)
	{
		//break;
		//number of attempts to place this tree
		for(int j=0; j<300; j++)
		{
			//this function uses so many random numbers that we need to reset to avoid periodicity/pattern
			//srand4(rand2(),rand2());

			//RAND() += 30029;
			int type = RAND()%FL_TYPES;
			FlType* ft = &g_fltype[type];

			//new forest?
			if(RAND()%10 == 0)
			{
				tx = RAND() % g_mapsz.x;
				ty = RAND() % g_mapsz.x;
				cmx = RAND() % TILE_SIZE;
				cmy = RAND() % TILE_SIZE;
				last = Vec2i(tx*TILE_SIZE + cmx, ty*TILE_SIZE + cmy);
			}

			cmx = RAND() % (TILE_SIZE*2);
			cmy = RAND() % (TILE_SIZE*2);

			//move from last seedling
			int x = last.x + cmx - TILE_SIZE/2*2;
			int y = last.y + cmy - TILE_SIZE/2*2;

			//int x = last.x + (r-=(r%(TILE_SIZE-1)))%(TILE_SIZE*2) - TILE_SIZE/2*2;
			//int y = last.y + (r-=(r%(TILE_SIZE-1)))%(TILE_SIZE*2) - TILE_SIZE/2*2;

			if(OffMap(x, y, x, y))
				continue;
			last = Vec2i(x,y);
			//int tx = x / TILE_SIZE;
			//int ty = y / TILE_SIZE;
			//float y = g_hmap.accheight2(x, y);
			float z = Bilerp(&g_hmap, x, y);
			//Vec3f norm = g_hmap.getnormal(x/TILE_SIZE, z/TILE_SIZE);
			//float offequator = fabs( (float)g_mapsz.y*TILE_SIZE/2.0f - y );
			//if(y > WATER_LEVEL && 1.0f - norm.y <= 0.3f)
			if(z > WATER_LEVEL)
			{
				if(PlaceFol(type, Vec3i(x, y, z*TILE_RISE)))
					break;
			}
		}
	}
}

void FreeFol()
{
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		f->on = false;

		for(auto qit=g_drawlist.begin(); qit!=g_drawlist.end(); qit++)
		{
			if(&*qit == f->depth)
			{
				g_drawlist.erase(qit);
				f->depth = NULL;
				break;
			}
		}
	}
}
