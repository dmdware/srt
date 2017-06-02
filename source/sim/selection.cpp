


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




#include "selection.h"
#include "../math/matrix.h"
#include "../window.h"
#include "../math/plane3f.h"
#include "../math/frustum.h"
#include "../bsp/brush.h"
#include "utype.h"
#include "unit.h"
#include "bltype.h"
#include "building.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../math/hmapmath.h"
#include "../utils.h"
#include "../app/appmain.h"
#include "build.h"
#include "player.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../gui/widgets/spez/blview.h"
#include "../gui/widgets/spez/truckmgr.h"
#include "../gui/gui.h"
#include "map.h"
#include "../render/drawsort.h"
#include "../sound/sound.h"
#include "../path/pathjob.h"
#include "../render/foliage.h"
#include "entity.h"

// Selection circle texture index
unsigned int g_circle = 0;
// Selected entity list
std::list<int> g_selection;

void Selection::clear()
{
	units.clear();
	buildings.clear();
	roads.clear();
	powls.clear();
	crpipes.clear();
}

// Selection frustum for drag/area-selection
static Vec3f normalLeft;
static Vec3f normalTop;
static Vec3f normalRight;
static Vec3f normalBottom;
static float distLeft;
static float distTop;
static float distRight;
static float distBottom;

static Frustum g_selfrust;	//selection frustum

//is unit selected?
bool USel(short ui)
{
	for(auto sit=g_sel.units.begin(); sit!=g_sel.units.end(); sit++)
		if(*sit == ui)
			return true;

	return false;
}

//is bl selected?
bool BSel(short bi)
{
	for(auto sit=g_sel.buildings.begin(); sit!=g_sel.buildings.end(); sit++)
		if(*sit == bi)
			return true;

	return false;
}

void DrawMarquee()
{
	Player* py = &g_player[g_localP];

	if(!g_mousekeys[0] || g_keyintercepted || g_mode != APPMODE_PLAY || g_build != BL_NONE)
		return;

#ifdef PLATFORM_MOBILE
	return;
#endif

#if 0
	EndS();
	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_shader[SHADER_COLOR2D].slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_shader[SHADER_COLOR2D].slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].slot[SSLOT_TEXCOORD0]);
#endif

	float vertices[] =
	{
		//posx, posy    texx, texy
		(float)g_mousestart.x,	(float)g_mousestart.y, 0,			0, 0,
		(float)g_mousestart.x,	(float)g_mouse.y,0,				1, 0,
		(float)g_mouse.x,			(float)g_mouse.y,0,				1, 1,

		(float)g_mouse.x,			(float)g_mousestart.y,0,			1, 1,
		(float)g_mousestart.x,	(float)g_mousestart.y,0,			0, 1
	};
	
	Shader *s = &g_shader[g_curS];

#ifdef PLATFORM_GL14
	//glVertexAttribPointer(g_shader[SHADER_COLOR2D].slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
	glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
#endif
	
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

#if 0
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat)
{
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	UseS(SHADER_COLOR3D);
	Shader* s = &g_shader[g_curS];
	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(projection->m_matrix);
	mvp.postmult(*viewmat);
	mvp.postmult(*modelmat);
#endif
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	float* color = g_player[g_localP].color;
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 0.5f);

	Player* py = &g_player[g_localP];

	glLineWidth(3);

	for(auto seliter = g_sel.buildings.begin(); seliter != g_sel.buildings.end(); seliter++)
	{
		const int bi = *seliter;
		const Building* b = &g_building[bi];
		const BlType* t = &g_bltype[b->type];

		const int tminx = b->tpos.x - t->width.x/2;
		const int tminz = b->tpos.y - t->width.y/2;
		const int tmaxx = tminx + t->width.x;
		const int tmaxz = tminz + t->width.y;

		const int cmminx = tminx*TILE_SIZE;
		const int cmminy = tminz*TILE_SIZE;
		const int cmmaxx = tmaxx*TILE_SIZE;
		const int cmmaxy = tmaxz*TILE_SIZE;

		const int off = TILE_SIZE/100;

		const float y1 = g_hmap.accheight(cmmaxx + off, cmminy - off) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(cmmaxx + off, cmmaxy + off) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(cmminx - off, cmmaxy + off) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(cmminx - off, cmminy - off) + TILE_SIZE/20;

		const float vertices[] =
		{
			//posx, posy posz
			(float)(cmmaxx + off), y1, (float)(cmminy - off),
			(float)(cmmaxx + off), y2, (float)(cmmaxy + off),
			(float)(cmminx - off), y3, (float)(cmmaxy + off),

			(float)(cmminx - off), y4, (float)(cmminy - off),
			(float)(cmmaxx + off), y1, (float)(cmminy - off)
		};

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}


	glLineWidth(1);
	EndS();

	//if(g_projtype == PROJ_PERSPECTIVE)
	UseS(SHADER_BILLBOARD);
	//else
	//	UseS(SHADER_BILLBOAR);

	s = &g_shader[g_curS];

	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);

	color = g_player[g_localP].color;
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 1.0f);

	//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(s->slot[SSLOT_NORMAL]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

	for(auto seliter = g_sel.units.begin(); seliter != g_sel.units.end(); seliter++)
	{
		Unit* u = &g_unit[ *seliter ];
		//Ent* e = g_entity[ 0 ];
		Vec2f p = u->drawpos;
		UType* t = &g_utype[ u->type ];

		//Vec3f p = c->m_pos + Vec3f(0, t->vmin.y, 0) + Vec3f(0, 1.0f, 0);

		const float r = t->size.x * 1.0f;

#if 0
		float y1 = Bilerp(&g_hmap, p.x + r, p.z - r);
		float y2 = Bilerp(&g_hmap, p.x + r, p.z + r);
		float y3 = Bilerp(&g_hmap, p.x - r, p.z + r);
		float y4 = Bilerp(&g_hmap, p.x - r, p.z - r);
#elif 1
		const float y1 = g_hmap.accheight(p.x + r, p.y - r) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(p.x + r, p.y + r) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(p.x - r, p.y + r) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(p.x - r, p.y - r) + TILE_SIZE/20;
#else
		float y1 = p.y;
		float y2 = p.y;
		float y3 = p.y;
		float y4 = p.y;
#endif

		const float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, y1, p.y - r,          1, 0,
			p.x + r, y2, p.y + r,          1, 1,
			p.x - r, y3, p.y + r,          0, 1,

			p.x - r, y3, p.y + r,          0, 1,
			p.x - r, y4, p.y - r,          0, 0,
			p.x + r, y1, p.y - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	EndS();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#endif

void SelPtCd(Vec2i pt, Selection *sel)
{
	//for(int i=0; i<BUILDINGS; i++)
	for(auto dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_CD)
			continue;

		short tx = d->index % g_mapsz.x;
		short ty = d->index / g_mapsz.x;

		CdTile* ctile = GetCd(d->cdtype, tx, ty, d->plan);
		int i = d->index;

		CdType* ct = &g_cdtype[ d->cdtype ];
		Tile& tile = SurfTile(tx, ty);

		int spi = ct->sprite[ctile->conntype][(int)ctile->finished][tile.incltype];
		Sprite* sp = &g_sprite[ spi ];

		Vec3i cmpos = Vec3i( tx * TILE_SIZE + TILE_SIZE/2, ty * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );

		Vec2i screenpos = CartToIso(cmpos) - g_scroll;
		
		//Shouldn't happen unless user mod
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		Texture* tex = &g_texture[ sp->difftexi ];
		
		int pixx = pt.x - (screenpos.x + (int)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - (screenpos.y + (int)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;
		
		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		std::list<Vec2i> *sellist = (std::list<Vec2i>*)(((char*)sel)+ct->seloff);
		sellist->push_back(Vec2i(tx,ty));
	}
}

/*
TODO
Go over all the code
remove commented unecessary code
remove if-0'd code
rewrite in C90
*/

int SelPtBl(Vec2i pt)
{
	int sel = -1;

	//for(int i=0; i<BUILDINGS; i++)
	for(auto dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_BL)
			continue;

		Building* b = &g_building[d->index];
		int i = d->index;

		//if(!b->on)
		//	continue;

		BlType* t = &g_bltype[ b->type ];

#if 0
		const int tminx = b->tpos.x - t->width.x/2;
		const int tminz = b->tpos.y - t->width.y/2;
		const int tmaxx = tminx + t->width.x;
		const int tmaxz = tminz + t->width.y;

		const int cmminx = tminx*TILE_SIZE;
		const int cmminy = tminz*TILE_SIZE;
		const int cmmaxx = tmaxx*TILE_SIZE;
		const int cmmaxy = tmaxz*TILE_SIZE;

		const int cmx = (cmminx+cmmaxx)/2;
		const int cmz = (cmminy+cmmaxy)/2;
#endif

#if 0
		Depthable* d = b;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmax.z);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z);

		Vec2i top = CartToIso(top3);
		Vec2i bot = CartToIso(bot3);
		Vec2i lef = CartToIso(lef3);
		Vec2i rig = CartToIso(rig3);

		if(minx > rig.x)
			continue;

		if(miny > bot.y)
			continue;

		if(maxx < lef.x)
			continue;

		if(maxy < top.y)
			continue;
#endif

		Sprite* sp;

		unsigned int* spi = NULL;

		if(b->finished)
			spi = t->sprite;
		else
			spi = t->csprite;

		if(t->hugterr)
		{
			Tile& surf = SurfTile(b->tpos.x, b->tpos.y);
			unsigned char inc = surf.incltype;
			sp = &g_sprite[ spi[inc] ];
		}
		else
		{
			//TODO frame
			sp = &g_sprite[ spi[0] ];
		}

		Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0),
			b->tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0),
			SurfTile(b->tpos.x, b->tpos.y).elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Texture* tex = &g_texture[ sp->difftexi ];

#if 0
		DrawImage(tex->texname,
			(float)screenpos.x + sp->offset[0],
			(float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2],
			(float)screenpos.y + sp->offset[3]);
#endif

#if 0
		if(!sp->pixels)
			InfoMess("!","!1");
		if(!sp->pixels->data)
			InfoMess("!","!2");
#endif

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		int pixx = pt.x - (screenpos.x + (int)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - (screenpos.y + (int)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

unsigned short SelPtFol(Vec2i pt)
{
	unsigned short sel = USHRT_MAX;

	//for(int i=0; i<BUILDINGS; i++)
	for(auto dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_FOL)
			continue;

		Foliage* f = &g_foliage[d->index];
		int i = d->index;

		//if(!b->on)
		//	continue;

		FlType* t = &g_fltype[ f->type ];
		
		Sprite* sp;

		unsigned int spi = 0;

		spi = t->sprite;
		sp = &g_sprite[ spi ];

		Vec2i screenmin = d->pixmin - g_scroll;
		Vec2i screenmax = d->pixmax - g_scroll;
	
		if(screenmin.x >= g_width)
			continue;

		if(screenmin.y >= g_height)
			continue;

		if(screenmax.x < 0)
			continue;

		if(screenmax.y < 0)
			continue;

		Vec3i cmpos = Vec3i( f->cmpos.x, f->cmpos.y, Bilerp(&g_hmap, (float)f->cmpos.x, (float)f->cmpos.y) * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		int pixx = pt.x - screenmin.x;

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - screenmin.y;

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

//select unit at point
int SelPtU(Vec2i pt)
{
	int sel = -1;
	
	for(auto dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	{
		Depthable* d = *dit;

		if(d->dtype != DEPTH_U)
			continue;

		Unit* u = &g_unit[d->index];
		int i = d->index;

		//if(!b->on)
		//	continue;

		UType* t = &g_utype[ u->type ];
		
		Sprite* sp;

		unsigned int spi = 0;
		unsigned char dir = (360 - ((int)u->rotation.y + 360*4) % 360) / (360/DIRS);

		spi = t->sprite[ dir % DIRS ][ (int)(u->frame[BODY_LOWER]) % t->nframes ];
		sp = &g_sprite[ spi ];

		Vec3i cmpos = Vec3i( u->cmpos.x, u->cmpos.y, Bilerp(&g_hmap, (float)u->cmpos.x, (float)u->cmpos.y) * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;
		
		int pixx = pt.x - (screenpos.x + (int)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - (screenpos.y + (int)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

//select a point
Selection SelPt()
{
	int selu = SelPtU(g_mouse);

	Selection sel;

#if 0
	InfoMess("sel one", "a");
#endif

	if(selu >= 0)
	{
#if 0
		InfoMess("sel one", "b");
#endif
		sel.units.push_back( selu );
	}

	int selb = SelPtBl(g_mouse);

	if(selb >= 0)
	{
		sel.buildings.push_back( selb );
	}

	SelPtCd(g_mouse, &sel);
	
	unsigned short self = SelPtFol(g_mouse);

	if(self != USHRT_MAX)
	{
		sel.fol.push_back(self);
	}

	return sel;
}

//select area units
std::list<int> SelArU(int minx, int miny, int maxx, int maxy)
{
	std::list<int> unitsel;

	bool haveowned = false;
	bool haveowmili = false;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UType* t = &g_utype[ u->type ];

		Depthable* d = u->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmax.z);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z);

		Vec2i top = CartToIso(top3);
		Vec2i bot = CartToIso(bot3);
		Vec2i lef = CartToIso(lef3);
		Vec2i rig = CartToIso(rig3);

		if(minx > rig.x)
			continue;

		if(miny > bot.y)
			continue;

		if(maxx < lef.x)
			continue;

		if(maxy < top.y)
			continue;

		unitsel.push_back(i);

		if(u->owner == g_localP && u->type != UNIT_LABOURER)
		{
			haveowned = true;

			if(t->military)
				haveowmili = true;
		}
	}

	//filter units....

	if(haveowmili)
	{
		//only owned military

		auto uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Unit* u = &g_unit[*uit];

			if(u->type == UNIT_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(u->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			UType* t = &g_utype[u->type];

			if(!t->military)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}
	else if(haveowned)
	{
		//only owned (no labourers)

		auto uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Unit* u = &g_unit[*uit];

			if(u->type == UNIT_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(u->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}

	return unitsel;
}

//select area
Selection SelAr()
{
	Player* py = &g_player[g_localP];

	int minx = imin(g_mousestart.x, g_mouse.x) + g_scroll.x;
	int maxx = imax(g_mousestart.x, g_mouse.x) + g_scroll.x;
	int miny = imin(g_mousestart.y, g_mouse.y) + g_scroll.y;
	int maxy = imax(g_mousestart.y, g_mouse.y) + g_scroll.y;

	Selection selection;

	selection.units = SelArU(minx, miny, maxx, maxy);

	return selection;
}

Selection DoSel()
{
	Selection sel;

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		sel = SelPt();
#ifndef PLATFORM_MOBILE
	//no annoying area select each time touch is dragged to scroll
	else
		sel = SelAr();
#endif

	return sel;
}

void ClearSel(Selection* s)
{
	s->buildings.clear();
	s->crpipes.clear();
	s->powls.clear();
	s->roads.clear();
	s->units.clear();
}

void AfterSel(Selection* s)
{
	bool haveconstr = false;
	bool havefini = false;
	bool havetruck = false;

	BlType* bt = NULL;
	CdType* ct = NULL;
	UType* ut = NULL;

	for(auto seliter = s->buildings.begin(); seliter != s->buildings.end(); seliter++)
	{
		int bi = *seliter;
		Building* b = &g_building[bi];
		bt = &g_bltype[b->type];

		if(!b->finished)
		{
			haveconstr = true;
			break;
		}
		else
		{
			havefini = true;
			break;
		}
	}

	if(!havefini && !haveconstr && s->units.size() > 0)
	{
		auto seliter = s->units.begin();
		int ui = *seliter;
		Unit* u = &g_unit[ui];
		if(u->type == UNIT_TRUCK)
			havetruck = true;
		ut = &g_utype[u->type];
	}

#if 1//TODO make abstract generic conduits use
	for(auto seliter = s->roads.begin(); seliter != s->roads.end(); seliter++)
	{
		ct = &g_cdtype[CD_ROAD];
		CdTile* cdtile = GetCd(CD_ROAD, seliter->x, seliter->y, false);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->buildings.clear();
			haveconstr = true;
			break;
		}
	}

	for(auto seliter = s->powls.begin(); seliter != s->powls.end(); seliter++)
	{
		ct = &g_cdtype[CD_POWL];
		CdTile* cdtile = GetCd(CD_POWL, seliter->x, seliter->y, false);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->buildings.clear();
			s->roads.clear();
			haveconstr = true;
			break;
		}
	}

	for(auto seliter = s->crpipes.begin(); seliter != s->crpipes.end(); seliter++)
	{
		ct = &g_cdtype[CD_CRPIPE];
		CdTile* cdtile = GetCd(CD_CRPIPE, seliter->x, seliter->y, false);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->buildings.clear();
			s->roads.clear();
			s->powls.clear();
			haveconstr = true;
			break;
		}
	}

#endif

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(haveconstr)
	{
		CstrView* cv = (CstrView*)gui->get("cstr view");
		cv->regen(s);
		gui->show("cstr view");

		if(bt)
			PlayClip(bt->sound[BLSND_CSEL]);
		//else if(ct)
		//	PlayClip(ct->sound[BLSND_CSEL]);
	}
	else if(havefini)
	{
		BlView* bv = (BlView*)gui->get("bl view");
		bv->regen(s);
		gui->show("bl view");
		PlayClip(bt->sound[BLSND_SEL]);
	}
	else if(havetruck)
	{
		TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
		tm->regen(s);
		gui->show("truck mgr");
		PlayClip(ut->sound[USND_SEL]);

#ifdef TSDEBUG
		tracku = &g_unit[ *s->units.begin() ];
#endif
	}

#if 0
	//35
	//2
	//15
	//39
	//12
	if(s->units.size() > 0)
	{
		char msg[128];
		sprintf(msg, "sel %d u \n target=%d \n dgoal=%d,%d",
			*s->units.begin(),
			g_unit[*s->units.begin()].target,
			g_unit[*s->units.begin()].goal.x - g_unit[*s->units.begin()].cmpos.x,
			g_unit[*s->units.begin()].goal.y - g_unit[*s->units.begin()].cmpos.y);
		InfoMess(msg, msg);
	}
#endif
}

//
void DrawSelectionCircles()
{
	//if(g_projtype == PROJ_PERSPECTIVE)
	//UseS(SHADER_BILLBOARDPERSP);
	//else
	//	UseS(SHADER_BILLBOAR);

	Shader* s = &g_shader[g_curS];

	glUniform4f(s->slot[SSLOT_COLOR], 0, 1, 0, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

	//glDisable(GL_CULL_FACE);

	for(auto selectioniterator = g_selection.begin(); selectioniterator != g_selection.end(); selectioniterator++)
	{
#if 0
		Ent* e = g_entity[ *selectioniterator ];
		//Ent* e = g_entity[ 0 ];
		Camera* c = &e->camera;
		EType* t = &g_etype[ e->type ];

		Vec3f p = c->m_pos + Vec3f(0, t->vmin.y, 0) + Vec3f(0, 1.0f, 0);

		const float r = t->vmax.x * 3.0f;

		float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, p.y + 1, p.z - r,          1, 0,
			p.x + r, p.y + 1, p.z + r,          1, 1,
			p.x - r, p.y + 1, p.z + r,          0, 1,

			p.x - r, p.y + 1, p.z + r,          0, 1,
			p.x - r, p.y + 1, p.z - r,          0, 0,
			p.x + r, p.y + 1, p.z - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glVertexPointer(3, GL_FLOAT, sizeof(float) * 5, &vertices[0]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
	}

	//EndS();
}

void AfterSelection()
{
}

void SelectOne(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f line[2];
	line[0] = campos;
	line[1] = campos + (ray * 10000.0f);

	float closestd = -1;

	for(int i=0; i<ENTITIES; i++)
	{
#if 0
		Ent* e = g_entity[i];

		if(!e)
			continue;

		Camera* c = &e->camera;
		EType* t = &g_etype[ e->type ];

		/*
		Vec3f p1 = c->m_pos;
		Vec3f p2 = c->m_pos + Vec3f(0, t->vmin.y, 0);
		Vec3f p3 = c->m_pos + Vec3f(0, t->vmax.y, 0);
		Vec3f p4 = c->m_pos + Vec3f(0, (t->vmin.y + t->vmax.y) / 2.0f, 0);

		//Not within marquee?
		//if(!PointInsidePlanes( c->m_pos + Vec3f(0,t->vmin.y,0) ))
		//if(!PointInsidePlanes( c->m_pos ))
		if(!PointInsidePlanes( p1 )
			&& !PointInsidePlanes( p2 )
			&& !PointInsidePlanes( p3 )
			&& !PointInsidePlanes( p4 ))
			continue;*/
		
		//Vec3f vmin = c->m_pos + t->vmin;
		//Vec3f vmax = c->m_pos + t->vmax;

		//if(!g_selfrust.BoxInFrustum2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
		//	continue;

		Vec3f normals[6];
		float dists[6];

		MakeHull(normals, dists, c->m_pos, t->vmin, t->vmax);

		Vec3f intersection;

		if(!LineInterHull(line, normals, dists, 6, &intersection))
			continue;

		float thisd = Magnitude(line[0] - intersection);

		if(thisd < closestd || closestd < 0)
		{
			g_selection.clear();
			g_selection.push_back(i);
			closestd = thisd;
		}
#endif
	}
}

bool PointBehindPlane(Vec3f p, Vec3f normal, float dist)
{
        float result = p.x*normal.x + p.y*normal.y + p.z*normal.z + dist;

        if(result <= EPSILON)
                return true;

        return false;
}

bool PointInsidePlanes(Vec3f p)
{
        if(!PointBehindPlane(p, normalLeft, distLeft))
                return false;

        if(!PointBehindPlane(p, normalTop, distTop))
                return false;

        if(!PointBehindPlane(p, normalRight, distRight))
                return false;

        if(!PointBehindPlane(p, normalBottom, distBottom))
                return false;

        return true;
}

#if 0
void SelectAreaOrtho()
{
	int minx = min(g_mousestart.x, g_mouse.x);
	int maxx = max(g_mousestart.x, g_mouse.x);
	int miny = min(g_mousestart.y, g_mouse.y);
	int maxy = max(g_mousestart.y, g_mouse.y);

	Vec3f ray = g_camera.View() - g_camera.Position();
	Vec3f onnear = OnNear(minx, miny);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = onnear;
	lineTopLeft[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(maxx, miny);
	Vec3f lineTopRight[2];
	lineTopRight[0] = onnear;
	lineTopRight[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(minx, maxy);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = onnear;
	lineBottomLeft[1] = onnear + (ray * 100000.0f);

	onnear = OnNear(maxx, maxy);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = onnear;
	lineBottomRight[1] = onnear + (ray * 100000.0f);

	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	GetMapIntersection2(lineTopLeft, &interTopLeft);
	GetMapIntersection2(lineTopRight, &interTopRight);
	GetMapIntersection2(lineBottomLeft, &interBottomLeft);
	GetMapIntersection2(lineBottomRight, &interBottomRight);

	Vec3f leftPoly[3];
	Vec3f topPoly[3];
	Vec3f rightPoly[3];
	Vec3f bottomPoly[3];

	leftPoly[0] = g_camera.m_pos;
	leftPoly[1] = interBottomLeft;
	leftPoly[2] = interTopLeft;

	topPoly[0] = g_camera.m_pos;
	topPoly[1] = interTopLeft;
	topPoly[2] = interTopRight;

	rightPoly[0] = g_camera.m_pos;
	rightPoly[1] = interTopRight;
	rightPoly[2] = interBottomRight;

	bottomPoly[0] = g_camera.Position();
	bottomPoly[1] = interBottomRight;
	bottomPoly[2] = interBottomLeft;

	g_normalLeft = Normal(leftPoly);
	g_normalTop = Normal(topPoly);
	g_normalRight = Normal(rightPoly);
	g_normalBottom = Normal(bottomPoly);

	g_distLeft = PlaneDistance(g_normalLeft, leftPoly[0]);
	g_distTop = PlaneDistance(g_normalTop, topPoly[0]);
	g_distRight = PlaneDistance(g_normalRight, rightPoly[0]);
	g_distBottom = PlaneDistance(g_normalBottom, bottomPoly[0]);
}
#endif

#if 0
// This determines if a BOX is in or around our frustum by it's min and max points
bool BoxInsidePlanes( float x, float y, float z, float x2, float y2, float z2)
{
	// Go through all of the corners of the box and check then again each plane
	// in the frustum.  If all of them are behind one of the planes, then it most
	// like is not in the frustum.
	for(int i = 0; i < 6; i++ )
	{
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y  + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y  + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z  + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y  + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y  + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x  + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;
		if(m_Frustum[i][A] * x2 + m_Frustum[i][B] * y2 + m_Frustum[i][C] * z2 + m_Frustum[i][D] > 0)  continue;

		// If we get here, it isn't in the frustum
		return false;
	}

	// Return a true for the box being inside of the frustum
	return true;
}
#endif

void SelectAreaPersp(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	int minx = imin(g_mousestart.x, g_mouse.x);
	int maxx = imax(g_mousestart.x, g_mouse.x);
	int miny = imin(g_mousestart.y, g_mouse.y);
	int maxy = imax(g_mousestart.y, g_mouse.y);

	//Vec3f campos = g_camera.m_pos;
	//Vec3f camside = g_camera.m_strafe;
	//Vec3f camup2 = g_camera.up2();
	//Vec3f viewdir = Normalize( g_camera.m_view - g_camera.m_pos );

	Vec3f topLeftRay = ScreenPerspRay(minx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = campos;
	lineTopLeft[1] = campos + (topLeftRay * 10000.0f);

	Vec3f topRightRay = ScreenPerspRay(maxx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopRight[2];
	lineTopRight[0] = campos;
	lineTopRight[1] = campos + (topRightRay * 10000.0f);

	Vec3f bottomLeftRay = ScreenPerspRay(minx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = campos;
	lineBottomLeft[1] = campos + (bottomLeftRay * 10000.0f);

	Vec3f bottomRightRay = ScreenPerspRay(maxx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = campos;
	lineBottomRight[1] = campos + (bottomRightRay * 10000.0f);
	
#if 0
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	GetMapIntersection2(lineTopLeft, &interTopLeft);
	GetMapIntersection2(lineTopRight, &interTopRight);
	GetMapIntersection2(lineBottomLeft, &interBottomLeft);
	GetMapIntersection2(lineBottomRight, &interBottomRight);
#endif

	Vec3f leftPoly[3];
	Vec3f topPoly[3];
	Vec3f rightPoly[3];
	Vec3f bottomPoly[3];
	Vec3f frontPoly[3];
	Vec3f backPoly[3];

	//Assemble polys in clockwise order so that their normals face outward

	leftPoly[0] = campos;
#if 0
	leftPoly[1] = interBottomLeft;
	leftPoly[2] = interTopLeft;
#endif
	leftPoly[1] = lineBottomLeft[1];
	leftPoly[2] = lineTopLeft[1];

	topPoly[0] = campos;
#if 0
	topPoly[1] = interTopLeft;
	topPoly[2] = interTopRight;
#endif
	topPoly[1] = lineTopLeft[1];
	topPoly[2] = lineTopRight[1];

	rightPoly[0] = campos;
#if 0
	rightPoly[1] = interTopRight;
	rightPoly[2] = interBottomRight;
#endif
	rightPoly[1] = lineTopRight[1];
	rightPoly[2] = lineBottomRight[1];

	bottomPoly[0] = campos;
#if 0
	bottomPoly[1] = interBottomRight;
	bottomPoly[2] = interBottomLeft;
#endif
	bottomPoly[1] = lineBottomRight[1];
	bottomPoly[2] = lineBottomLeft[1];
	
	frontPoly[0] = lineBottomLeft[1];
	frontPoly[1] = lineBottomRight[1];
	frontPoly[2] = lineTopRight[1];

	backPoly[0] = lineTopRight[0] + topLeftRay;
	backPoly[1] = lineBottomRight[0] + bottomRightRay;
	backPoly[2] = lineBottomLeft[0] + bottomLeftRay;

	// Normals will actually face inward
	// (using counter-clockwise function Normal2)

	normalLeft = Normal2(leftPoly);
	normalTop = Normal2(topPoly);
	normalRight = Normal2(rightPoly);
	normalBottom = Normal2(bottomPoly);
	Vec3f normalFront = Normal(frontPoly);
	Vec3f normalBack = Normal(backPoly);

	distLeft = PlaneDistance(normalLeft, leftPoly[0]);
	distTop = PlaneDistance(normalTop, topPoly[0]);
	distRight = PlaneDistance(normalRight, rightPoly[0]);
	distBottom = PlaneDistance(normalBottom, bottomPoly[0]);
	float distFront = PlaneDistance(normalFront, frontPoly[0]);
	float distBack = PlaneDistance(normalBack, backPoly[0]);

	g_selfrust.construct(
		Plane3f(normalLeft.x, normalLeft.y, normalLeft.z, distLeft),
		Plane3f(normalRight.x, normalRight.y, normalRight.z, distRight),
		Plane3f(normalTop.x, normalTop.y, normalTop.z, distTop),
		Plane3f(normalBottom.x, normalBottom.y, normalBottom.z, distBottom),
		Plane3f(normalFront.x, normalFront.y, normalFront.z, distFront),
		Plane3f(normalBack.x, normalBack.y, normalBack.z, distBack));

#if 0
	for(int i=0; i<ENTITIES; i++)
	{
		Ent* e = g_entity[i];

		if(!e)
			continue;

		Camera* c = &e->camera;
		EType* t = &g_etype[ e->type ];

		/*
		Vec3f p1 = c->m_pos;
		Vec3f p2 = c->m_pos + Vec3f(0, t->vmin.y, 0);
		Vec3f p3 = c->m_pos + Vec3f(0, t->vmax.y, 0);
		Vec3f p4 = c->m_pos + Vec3f(0, (t->vmin.y + t->vmax.y) / 2.0f, 0);

		//Not within marquee?
		//if(!PointInsidePlanes( c->m_pos + Vec3f(0,t->vmin.y,0) ))
		//if(!PointInsidePlanes( c->m_pos ))
		if(!PointInsidePlanes( p1 )
			&& !PointInsidePlanes( p2 )
			&& !PointInsidePlanes( p3 )
			&& !PointInsidePlanes( p4 ))
			continue;*/
		
		Vec3f vmin = c->m_pos + t->vmin;
		Vec3f vmax = c->m_pos + t->vmax;

		if(!g_selfrust.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		g_selection.push_back(i);
	}
#endif
}

void DoSelection(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	g_selection.clear();

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		SelectOne(campos, camside, camup2, viewdir);
	else
		SelectAreaPersp(campos, camside, camup2, viewdir);

	AfterSelection();
}
