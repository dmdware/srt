


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




#include "unit.h"
#include "../render/shader.h"
#include "utype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "umove.h"
#include "simdef.h"
#include "simflow.h"
#include "labourer.h"
#include "../debug.h"
#include "../math/frustum.h"
#include "building.h"
#include "labourer.h"
#include "truck.h"
#include "../econ/demand.h"
#include "../math/vec4f.h"
#include "../path/pathjob.h"
#include "../render/anim.h"
#include "map.h"
#include "../render/drawqueue.h"
#include "../render/drawsort.h"
#include "../path/collidertile.h"
#include "../path/pathjob.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/chattext.h"

#ifdef RANDOM8DEBUG
int thatunit = -1;
#endif

Unit g_unit[UNITS];


int CountU(int utype)
{
	int c = 0;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;
		
		if(u->type != utype)
			continue;

		c++;
	}

	return c;
}

int CountU(int utype, int owner)
{
	int c = 0;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;
		
		if(u->type != utype)
			continue;

		if(u->owner != owner)
			continue;

		c++;
	}

	return c;
}

Unit::Unit()
{
	on = false;
	threadwait = false;
	depth = NULL;
}

Unit::~Unit()
{
	destroy();
}

void Unit::destroy()
{
	//corpc fix
	if(on)
		ResetMode(this);	//make sure to disembark, reset truck driver, etc.

	//corpc fix TODO
	if(g_collidertile && on)
		freecollider();

	on = false;
	threadwait = false;

	if(home >= 0)
		Evict(this, true);

	if(mode == UMODE_GOSUP
		//|| mode == GOINGTOREFUEL
			//|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
				)
	{
		if(supplier >= 0)
		{
			//Building* b = &g_building[supplier];
			//b->transporter[cargotype] = -1;
		}
	}

	if(type == UNIT_TRUCK)
	{
		bool targbl = false;
		bool targcd = false;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_BL )
			targbl = true;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_CD )
			targbl = true;

		if(mode == UMODE_GODEMB)
			targbl = true;

		if(mode == UMODE_ATDEMB)
			targbl = true;

		if(mode == UMODE_GODEMCD)
			targcd = true;

		if(mode == UMODE_ATDEMCD)
			targcd = true;

		if(targtype == TARG_BL)
			targbl = true;

		if(targtype == TARG_CD)
			targcd = true;

		if( targbl )
		{
			if(target >= 0)
			{
				Building* b = &g_building[target];
				b->transporter[cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(target >= 0 && target2 >= 0 && cdtype >= 0)
			{
				CdTile* ctile = GetCd(cdtype, target, target2, false);
				ctile->transporter[cargotype] = -1;
			}
		}
	}

	auto qit=g_drawlist.begin();
	while(qit!=g_drawlist.end())
	{
		if(&*qit != depth)
		{
			qit++;
			continue;
		}

		qit = g_drawlist.erase(qit);
	}
	depth = NULL;
}

/*
How is this diff from ResetPath?
Find out and fix this.
*/
void Unit::resetpath()
{
	path.clear();
	tpath.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = false;
}

void DrawUnits()
{
	Shader* s = &g_shader[g_curS];

	for(int i=0; i<UNITS; i++)
	{
		StartTimer(TIMER_DRAWUMAT);

		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hidden())
			continue;

		UType* t = &g_utype[u->type];
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
		StopTimer(TIMER_DRAWUMAT);

		//m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.y);
		Sprite* sp = &g_sprite[t->sprite[0][0]];
		Texture* difftex = &g_texture[sp->difftexi];
		Texture* depthtex = &g_texture[sp->depthtexi];

		Vec3i cm3pos;
		cm3pos.x = u->cmpos.x;
		cm3pos.y = u->cmpos.y;
		cm3pos.z = (int)( Bilerp(&g_hmap, u->cmpos.x, u->cmpos.y) * TILE_RISE );
		Vec2i isopos = CartToIso(cm3pos);
		Vec2i screenpos = isopos - g_scroll;

#if 0
		DrawImage(difftex->texname,
			(float)screenpos.x + sp->offset[0], (float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2], (float)screenpos.y + sp->offset[3], 
			0,0,1,1, g_gui.m_crop);
#else
		DrawDeep(difftex->texname, depthtex->texname, 0,
			(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
			(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
			sp->crop[0], sp->crop[1],
			sp->crop[2], sp->crop[3]);
#endif
	}
}

/*
TODO
Unit selection based on drawpos x,y.
Check if sprite clip rect + drawpos x,y
is in g_mouse + g_scroll.
Same for bl, cd.
*/

void DrawUnit(Unit* u, float rendz)
{
	StartTimer(TIMER_DRAWUNITS);

	short tx = u->cmpos.x / TILE_SIZE;
	short ty = u->cmpos.y / TILE_SIZE;

	Shader* s = &g_shader[g_curS];
	
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

#if 0
	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, 1.0f);
	else
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}
#endif

	if(!IsTileVis(g_localP, tx, ty))
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}

	if(u->hidden())
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}

	if(USel(u - g_unit))
	{
		EndS();
		//UseS(SHADER_COLOR2D);
		UseS(SHADER_DEEPCOLOR);
		s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		UType* t = &g_utype[ u->type ];

		Depthable* d = u->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z + TILE_SIZE/200);

		Vec2i top = CartToIso(top3) - g_scroll;
		Vec2i bot = CartToIso(bot3) - g_scroll;
		Vec2i lef = CartToIso(lef3) - g_scroll;
		Vec2i rig = CartToIso(rig3) - g_scroll;

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		int topd, botd, lefd, rigd;
		
#if 1
		CartToDepth(top3, &topd);
		CartToDepth(bot3, &botd);
		CartToDepth(lef3, &lefd);
		CartToDepth(rig3, &rigd);

#if 0
		top = Vec2i(50, 50);
		bot = Vec2i(50, 100);
		lef = Vec2i(0, 75);
		rig = Vec2i(100, 75);
#endif

#if 0
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)topd,
			(float)rig.x, (float)rig.y, (float)rigd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)lef.x, (float)lef.y, (float)lefd,
			(float)top.x, (float)top.y, (float)topd
		};
#else
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)topd,
			(float)rig.x, (float)rig.y, (float)rigd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)lef.x, (float)lef.y, (float)lefd,
			(float)top.x, (float)top.y, (float)topd
		};
#endif
#else
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)d->rendz,
			(float)rig.x, (float)rig.y, (float)d->rendz,
			(float)bot.x, (float)bot.y, (float)d->rendz,
			(float)lef.x, (float)lef.y, (float)d->rendz,
			(float)top.x, (float)top.y, (float)d->rendz
		};
#endif

#if 0
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		//glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);

		glVertexPointer(3, GL_FLOAT, sizeof(float)*0, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_LOOP, 0, 4);
#else
		//DrawDeepColor(0, 1, 0, 1, vertices, 5, GL_LINE_STRIP);
		DrawDeepColor(0, 1, 0, 1, vertices, 6, GL_TRIANGLES);
#endif

		EndS();

		UseS(SHADER_DEEPORTHO);
		s = &g_shader[g_curS];
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	}

	UType* t = &g_utype[u->type];
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

	unsigned char dir = (360 - ((int)u->rotation.y + 360*4) % 360) / (360/DIRS);

	//assert( dir < 8 );

	int spi = t->sprite[ dir % DIRS ][(int)(u->frame[BODY_LOWER]) % t->nframes ];

	//assert( (int)u->frame[BODY_LOWER] < t->nframes );

	Sprite* sp = &g_sprite[spi];
	
	Texture* difftex = &g_texture[sp->difftexi];
	Texture* depthtex = &g_texture[sp->depthtexi];

	Vec3i cm3pos;
	cm3pos.x = u->cmpos.x;
	cm3pos.y = u->cmpos.y;
	cm3pos.z = (int)( Bilerp(&g_hmap, u->cmpos.x, u->cmpos.y) * TILE_RISE );
	Vec2i isopos = CartToIso(cm3pos);
	Vec2i screenpos = isopos - g_scroll;

#if 0
	DrawImage(difftex->texname,
		screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
		screenpos.x + sp->offset[2], screenpos.y + sp->offset[3]);
#elif 0
	DrawDeep(difftex->texname, depthtex->texname, rendz,
		screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
		screenpos.x + sp->offset[2], screenpos.y + sp->offset[3],
		0, 0, 1, 1);
#else
	DrawDeep(difftex->texname, depthtex->texname, rendz,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	StopTimer(TIMER_DRAWUNITS);
}

int NewUnit()
{
	for(int i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

// starting belongings for labourer
void StartBel(Unit* u)
{
	Zero(u->belongings);
	u->car = -1;
	u->home = -1;

	if(u->type == UNIT_LABOURER)
	{
		//if(u->owner >= 0)
		{
			//u->belongings[ RES_DOLLARS ] = 100;
			//u->belongings[ RES_DOLLARS ] = CYCLE_FRAMES * LABOURER_FOODCONSUM * 30;
			u->belongings[ RES_DOLLARS ] = CYCLE_FRAMES/SIM_FRAME_RATE * LABOURER_FOODCONSUM * 10;
		}

		u->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		u->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
	else if(u->type == UNIT_TRUCK)
	{
		u->belongings[ RES_RETFUEL ] = STARTING_FUEL;
	}
}

bool PlaceUnit(int type, Vec2i cmpos, int owner, int *reti)
{
	int i = NewUnit();

	if(i < 0)
		return false;

	if(reti)
		*reti = i;

#if 0
	bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int step;
	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int jobframes;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//std::vector<TransportJob> bids;

	float frame[2];
#endif

	Unit* u = &g_unit[i];
	UType* t = &g_utype[type];

	u->on = true;
	u->type = type;
	u->cmpos = cmpos;
	//u->drawpos = Vec3f(cmpos.x, cmpos.y, Bilerp(&g_hmap, (float)cmpos.x, (float)cmpos.y)*TILE_RISE);
	u->owner = owner;
	u->path.clear();
	u->goal = cmpos;
	u->target = -1;
	u->target2 = -1;
	u->targetu = false;
	u->underorder = false;
	u->fuelstation = -1;
	u->targtype = TARG_NONE;
	//u->home = -1;
	StartBel(u);
	u->hp = t->starthp;
	u->passive = false;
	u->prevpos = u->cmpos;
	u->taskframe = 0;
	u->pathblocked = false;
	u->jobframes = 0;
	u->supplier = -1;
	u->exputil = 0;
	u->reqamt = 0;
	u->targtype = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;
	u->subgoal = u->goal;

	u->mode = UMODE_NONE;
	u->pathdelay = 0;
	u->lastpath = g_simframe;

	u->cdtype = CD_NONE;
	u->driver = -1;
	//u->framesleft = 0;
	u->cyframes = WORK_DELAY-1;
	u->cargoamt = 0;
	u->cargotype = -1;

	u->rotation.y = 0;
	u->frame[BODY_LOWER] = 0;

	u->forsale = false;
	u->price = 1;

	u->fillcollider();
	g_drawlist.push_back(Depthable());
	Depthable* d = &*g_drawlist.rbegin();
	d->dtype = DEPTH_U;
	d->index = i;
	u->depth = d;
	UpDraw(u);
	AddVis(u);
	Explore(u);

	return true;
}

void FreeUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		g_unit[i].destroy();
		g_unit[i].on = false;
	}
}

bool Unit::hidden() const
{
	switch(mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		return true;
	default:break;
	}

	return false;
}

void AnimUnit(Unit* u)
{
	UType* t = &g_utype[u->type];

	if(u->type == UNIT_BATTLECOMP || u->type == UNIT_LABOURER)
	{
		if(u->prevpos == u->cmpos)
		{
			u->frame[BODY_LOWER] = 0;
			return;
		}

		PlayAni(u->frame[BODY_LOWER], 0, 29, true, 1.0f);
	}
}

void UpdAI(Unit* u)
{
	//return;	//do nothing for now?

	if(u->type == UNIT_LABOURER)
		UpdLab(u);
	else if(u->type == UNIT_TRUCK)
		UpdTruck(u);
}

void UpdCheck(Unit* u)
{
	if(u->type == UNIT_LABOURER)
		UpdLab2(u);
}

void UpdUnits()
{
#ifdef TSDEBUG
	if(tracku)
	{
		Log("tracku t"<<tracku->type<<" mode"<<(int)tracku->mode<<" tpathsz"<<tracku->tpath.size()<<" pathsz"<<tracku->path.size());
	}
#endif

	for(int i = 0; i < UNITS; i++)
	{
		StartTimer(TIMER_UPDUONCHECK);

		Unit* u = &g_unit[i];

		if(!u->on)
		{
			StopTimer(TIMER_UPDUONCHECK);
			continue;
		}

		StopTimer(TIMER_UPDUONCHECK);

#if 1
		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
#endif

#if 1
		/*
		Second check, because unit might be destroyed in last call,
		which might remove Depthable ->depth, which would cause a
		crash in MoveUnit.
		*/
		if(!u->on)
			continue;
#endif

		StartTimer(TIMER_MOVEUNIT);
		MoveUnit(u);
		StopTimer(TIMER_MOVEUNIT);
		//must be called after Move... labs without paths are stuck
		UpdCheck(u);
		StartTimer(TIMER_ANIMUNIT);
		AnimUnit(u);
		StopTimer(TIMER_ANIMUNIT);

#if 0
		/*
		Last call, because unit might be destroyed in this call,
		which might remove Depthable ->depth, which would cause a
		crash in MoveUnit.
		*/
		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
#endif
	}
}

void ResetPath(Unit* u)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("path reset");
		InfoMess("pr", "pr");
	}
#endif

	u->path.clear();
	u->tpath.clear();

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("ResetPath u=thatunit");
	}
#endif
}

void ResetGoal(Unit* u)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("g reset");
		InfoMess("rg", "rg");
	}
#endif

	u->exputil = 0;
	u->goal = u->subgoal = u->cmpos;
	ResetPath(u);

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset path track u 3", "reset path track u 3");
	}
#endif
}

void ResetMode(Unit* u)
{
#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("\tResetMode u=thatunit");
	}
#endif

#ifdef HIERDEBUG
	if(u - g_unit == 5 && u->mode == UMODE_GOBLJOB && u->target == 5)
	{
		InfoMess("rsu5", "rsu5");
	}
#endif

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("mode reset");
		char msg[128];
		sprintf(msg, "rm %s prevm=%d", g_utype[u->type].name, (int)u->mode);
		InfoMess("rm", msg);
	}
#endif

#if 0
	switch(u->mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		u->freecollider();
		PlaceUAb(u->type, u->cmpos, &u->cmpos);
		u->fillcollider();
		UpDraw(u);
	default:break;
	}
#else
	if(u->mode == UMODE_DRIVE &&
		u->type == UNIT_LABOURER)	//corpc fix
	{
		Unit* tr = &g_unit[u->target];
		tr->driver = -1;
		
		RemVis(u);
		u->freecollider();
		PlaceUAb(u->type, tr->cmpos, &u->cmpos);
#if 0
		//TODO
		u->drawpos.x = u->cmpos.x;
		u->drawpos.y = u->cmpos.y;
		u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
#endif
		u->fillcollider();
		AddVis(u);
		UpDraw(u);
	}
	else if(u->hidden())
	{
		RemVis(u);
		u->freecollider();
		PlaceUAb(u->type, u->cmpos, &u->cmpos);
#if 0
		//TODO
		u->drawpos.x = u->cmpos.x;
		u->drawpos.y = u->cmpos.y;
		u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
#endif
		u->fillcollider();
		AddVis(u);
		UpDraw(u);
	}
#endif

#if 0
	//URAN_DEBUG
	if(u-g_unit == 19)
	{
		Building* b = &g_building[5];
		char msg[1280];
		sprintf(msg, "ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
			(int)b->transporter[RES_URANIUM],
			(int)g_unit[b->transporter[RES_URANIUM]].mode,
			(int)g_unit[b->transporter[RES_URANIUM]].target,
			5,
			(int)g_unit[b->transporter[RES_URANIUM]].targtype,
			(int)g_unit[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 1");
	if(u->type == UNIT_LABOURER)
	{
		//LastNum("resetmode 1a");
		if(u->mode == UMODE_BLJOB)
			RemWorker(u);

		//if(hidden())
		//	relocate();
	}
	else if(u->type == UNIT_TRUCK)
	{
#if 1
		if(u->mode == UMODE_GOSUP
		                //|| mode == GOINGTOREFUEL
		                //|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
		  )
		{
			if(u->supplier >= 0)
			{
				//necessary?
				Building* b = &g_building[u->supplier];
				b->transporter[u->cargotype] = -1;
			}
		}

		bool targbl = false;
		bool targcd = false;

		if( (u->mode == UMODE_GOSUP || u->mode == UMODE_ATSUP || u->mode == UMODE_GOREFUEL || u->mode == UMODE_REFUELING) && u->targtype == TARG_BL )
			targbl = true;

		if( (u->mode == UMODE_GOSUP || u->mode == UMODE_ATSUP || u->mode == UMODE_GOREFUEL || u->mode == UMODE_REFUELING) && u->targtype == TARG_CD )
			targbl = true;

		if(u->mode == UMODE_GODEMB)
			targbl = true;

		if(u->mode == UMODE_ATDEMB)
			targbl = true;

		if(u->mode == UMODE_GODEMCD)
			targcd = true;

		if(u->mode == UMODE_ATDEMCD)
			targcd = true;

		if(u->targtype == TARG_BL)
			targbl = true;

		if(u->targtype == TARG_CD)
			targcd = true;

		if( targbl )
		{
			if(u->target >= 0)
			{
				Building* b = &g_building[u->target];
				b->transporter[u->cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(u->target >= 0 && u->target2 >= 0 && u->cdtype >= 0)
			{
				CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
				ctile->transporter[u->cargotype] = -1;
			}
		}
#endif
		u->targtype = TARG_NONE;

		if(u->driver >= 0)
		{
			Unit* op = &g_unit[u->driver];
			//LastNum("resetmode 1b");
			//g_unit[u->driver].Disembark();

			//corpc fix
			//important to reset driver first and then tell him to disembark so we don't get stuck in a resetting loop
			u->driver = -1;

			if(op->mode == UMODE_DRIVE)
				Disembark(op);
		}
	}

	//LastNum("resetmode 2");

	//transportAmt = 0;
	u->targtype = TARG_NONE;
	u->target = u->target2 = -1;
	u->supplier = -1;
	u->mode = UMODE_NONE;
	ResetGoal(u);

#if 0
	//URAN_DEBUG
	if(u-g_unit == 19)
	{
		Building* b = &g_building[5];
		char msg[1280];
		sprintf(msg, "/ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n cargty%d",
			(int)b->transporter[RES_URANIUM],
			(int)g_unit[b->transporter[RES_URANIUM]].mode,
			(int)g_unit[b->transporter[RES_URANIUM]].target,
			5,
			(int)g_unit[b->transporter[RES_URANIUM]].targtype,
			(int)g_unit[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 3");
}

void ResetTarget(Unit* u)
{

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 3", "reset mode track u 3");
	}
#endif

	ResetMode(u);
	u->target = -1;
}
