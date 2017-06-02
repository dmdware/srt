


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






#include "../platform.h"
#include "shader.h"
#include "../gui/font.h"
#include "../math/matrix.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "../sim/player.h"
#include "../math/vec4f.h"
#include "../sim/simflow.h"
#include "../sim/simdef.h"
#include "../math/frustum.h"
#include "infoov.h"
#include "../sim/map.h"
#include "transaction.h"
#include "../math/hmapmath.h"
#include "fogofwar.h"
#include "../language.h"

//draw unit/bl info overlay
void DrawOv()
{
	//glDisable(GL_DEPTH_TEST);
	const float color[] = {1,1,1,1};

	Shader* s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], color[3]);

	Player* py = &g_player[g_localP];


	//if(g_zoom < 0.6f)
	//	goto end;
	//if(g_drawtransx)
	//show hide up unit stats
	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER && u->type != UNIT_TRUCK)
			continue;

		if(u->hidden())
			continue;
		
		//show only stats for units hide to mouse position
#if 0
		if(iabs(g_mouse3d.x - u->cmpos.x) > TILE_SIZE ||
			iabs(g_mouse3d.y - u->cmpos.y) > TILE_SIZE)
			continue;
#else
		Depthable* d = u->depth;

		Vec2i pixcenter = (d->pixmin + d->pixmax) / 2;

		//20 pixel square
		if(iabs(g_mouse.x + g_scroll.x - pixcenter.x) > 10 ||
			iabs(g_mouse.y + g_scroll.y - pixcenter.y) > 10)
			continue;
#endif

		Vec2i tpos = u->cmpos / TILE_SIZE;

		if(!IsTileVis(g_localP, tpos.x, tpos.y))
			continue;

		//if(!g_frustum.pointin(u->drawpos.x, u->drawpos.y, u->drawpos.y))
		//	continue;

		RichText rt;

#if 1
		//char pathinfo[182];
		//sprintf(pathinfo, "\n\n\n\n\n#%d\ntpathsz=%d pathsz=%d\n", (int)(u-g_unit), (int)u->tpath.size(), (int)u->path.size());
		//rt.m_part.push_back(pathinfo);

		for(auto pit=u->tpath.begin(); pit!=u->tpath.end(); pit++)
		{
			Vec2i cmp = Vec2i(pit->x, pit->y) * TILE_SIZE + Vec2i(1,1)*TILE_SIZE/2;
			Vec3i cmp3;
			cmp3.x = cmp.x;
			cmp3.y = cmp.y;
			cmp3.z = Bilerp(&g_hmap, cmp.x, cmp.y) * TILE_RISE;
			Vec2i screenpos = CartToIso(cmp3) - g_scroll;
			char pc[32];
			sprintf(pc, "*");
			RichText prt = RichText(pc);
			DrawShadowedText(MAINFONT32, screenpos.x, screenpos.y, &prt);
		}
		
		{
			Vec2i cmp = u->subgoal;
			Vec3i cmp3;
			cmp3.x = cmp.x;
			cmp3.y = cmp.y;
			cmp3.z = Bilerp(&g_hmap, cmp.x, cmp.y) * TILE_RISE;
			Vec2i screenpos = CartToIso(cmp3) - g_scroll;
			char pc[32];
			sprintf(pc, "*");
			RichText prt = RichText(pc);
			float subgoalc[4] = {0.0f, 1, 0, 1};
			//DrawShadowedText(MAINFONT32, screenpos.x, screenpos.y, &prt, subgoalc);

			unsigned int spi = g_cursor[g_curst];
			Sprite* sp = &g_sprite[spi];

			float crop[] = {0,0,(float)g_width-1,(float)g_height-1}; 
			glUniform4f(g_shader[g_curS].slot[SSLOT_COLOR], subgoalc[0], subgoalc[1], subgoalc[2], subgoalc[3]);
			DrawImage(g_texture[sp->difftexi].texname, screenpos.x+sp->offset[0], screenpos.y+sp->offset[1], screenpos.x+sp->offset[2], screenpos.y+sp->offset[3], 0,0,1,1, crop);
		}

		{
			Vec2i cmp = u->goal;
			Vec3i cmp3;
			cmp3.x = cmp.x;
			cmp3.y = cmp.y;
			cmp3.z = Bilerp(&g_hmap, cmp.x, cmp.y) * TILE_RISE;
			Vec2i screenpos = CartToIso(cmp3) - g_scroll;
			char pc[32];
			sprintf(pc, "*");
			RichText prt = RichText(pc);
			float subgoalc[4] = {1.0f, 0, 0, 1};
			//DrawShadowedText(MAINFONT32, screenpos.x, screenpos.y, &prt, subgoalc);

			unsigned int spi = g_cursor[g_curst];
			Sprite* sp = &g_sprite[spi];

			float crop[] = {0,0,(float)g_width-1,(float)g_height-1}; 
			glUniform4f(g_shader[g_curS].slot[SSLOT_COLOR], subgoalc[0], subgoalc[1], subgoalc[2], subgoalc[3]);
			DrawImage(g_texture[sp->difftexi].texname, screenpos.x+sp->offset[0], screenpos.y+sp->offset[1], screenpos.x+sp->offset[2], screenpos.y+sp->offset[3], 0,0,1,1, crop);
		}
	
#endif

		std::string mode;
		Building* b;
		BlType* bt;
		CdType* ct;

		switch(u->mode)
		{
		case UMODE_NONE:
			mode = STRTABLE[STR_IDLE].rawstr();
			break;
		case UMODE_GOBLJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode =  STRTABLE[STR_TOJOB].rawstr() + " (" + std::string(bt->name) + ")";
			break;
		case UMODE_BLJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_ATJOB].rawstr() + " (" + std::string(bt->name) + ")";
			break;
		case UMODE_GOCSTJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_TOJOB].rawstr() + " (" + std::string(bt->name) + " " + STRTABLE[STR_CSTR].rawstr() + ")";
			break;
		case UMODE_CSTJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_ATJOB].rawstr() + " (" + std::string(bt->name) + " " + STRTABLE[STR_CSTR].rawstr() + ")";
			break;
		case UMODE_GOCDJOB:
			ct = &g_cdtype[u->cdtype];
			mode = STRTABLE[STR_TOJOB].rawstr() + " (" + std::string(ct->name) + " " + STRTABLE[STR_CSTR].rawstr() + ")";
			break;
		case UMODE_CDJOB:
			ct = &g_cdtype[u->cdtype];
			mode = STRTABLE[STR_ATJOB].rawstr() + " (" + std::string(ct->name) + " " + STRTABLE[STR_CSTR].rawstr() + ")";
			break;
		case UMODE_GOSHOP:
			mode = STRTABLE[STR_TOSTORE].rawstr();
			break;
		case UMODE_SHOPPING:
			mode = STRTABLE[STR_ATSTORE].rawstr();
			break;
		case UMODE_GOREST:
			mode = STRTABLE[STR_TOHOME].rawstr();
			break;
		case UMODE_RESTING:
			mode = STRTABLE[STR_ATHOME].rawstr();
			break;
		case UMODE_GODRIVE:
			mode = STRTABLE[STR_TOJOB].rawstr() + " (" + STRTABLE[STR_TRUCKING].rawstr() + ")";
			break;
		case UMODE_DRIVE:
			mode = STRTABLE[STR_ATJOB].rawstr() + " (" + STRTABLE[STR_TRUCKING].rawstr() + ")";
			break;
		case UMODE_GOSUP:
			b = &g_building[u->supplier];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_TOSUP].rawstr() + " (" + std::string(bt->name) + ")";
			break;
		case UMODE_GODEMB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_TODEM].rawstr() + " (" + std::string(bt->name) + ")";
			break;
		case UMODE_GOREFUEL:
			mode = STRTABLE[STR_TOREF].rawstr();
			break;
		case UMODE_REFUELING:
			mode = STRTABLE[STR_REFUEL].rawstr();
			break;
		case UMODE_ATDEMB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_OFFLOAD].rawstr() + " (" + std::string(bt->name) + ")";
			break;
		case UMODE_ATSUP:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = STRTABLE[STR_LOADUP].rawstr() + " (" + std::string(bt->name) + ")";
			break;
		case UMODE_GODEMCD:
			ct = &g_cdtype[u->cdtype];
			mode = STRTABLE[STR_TODEM].rawstr() + " (" + std::string(ct->name) + ")";
			break;
		case UMODE_ATDEMCD:
			ct = &g_cdtype[u->cdtype];
			mode = STRTABLE[STR_OFFLOAD].rawstr() + " (" + std::string(ct->name) + ")";
			break;
		default:
			mode = "?";
			break;
		}

		//mode += "\n";

		Vec3i cmpos = Vec3i( u->cmpos.x, u->cmpos.y,
			SurfTile(u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE).elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		rt.m_part.push_back(RichPart(UStr(mode.c_str())));
		DrawCenterShadText(MAINFONT16, screenpos.x, screenpos.y, &rt);
		//DrawBoxShadText(MAINFONT8, screenpos.x, screenpos.y, g_width, g_height, &rt, color, 0, -1);

		RichText rt2;

		if(u->type == UNIT_LABOURER)
		{
			char food[16];
			char labour[16];
			char funds[16];

			Resource* foodr = &g_resource[RES_RETFOOD];
			Resource* labourr = &g_resource[RES_LABOUR];
			Resource* fundsr = &g_resource[RES_DOLLARS];

			sprintf(food, "%d \n", u->belongings[RES_RETFOOD]);
			sprintf(labour, "%d \n", u->belongings[RES_LABOUR]);
			sprintf(funds, "%d \n", u->belongings[RES_DOLLARS]);

			rt2.m_part.push_back(RichPart(RICH_ICON, foodr->icon));
			rt2.m_part.push_back(RichPart(UStr(food)));
			rt2.m_part.push_back(RichPart(RICH_ICON, labourr->icon));
			rt2.m_part.push_back(RichPart(UStr(labour)));
			rt2.m_part.push_back(RichPart(RICH_ICON, fundsr->icon));
			rt2.m_part.push_back(RichPart(UStr(funds)));

#if 0
			char add[32];
			sprintf(add, "t%d,t%d fr%d,%d\n", (int)(u->target), u->target2, u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE);
			rt2.m_part.push_back(RichPart(UStr(add)));
#endif
		}
		else if(u->type == UNIT_TRUCK)
		{
			char fuel[16];

			Resource* fuelr = &g_resource[RES_RETFUEL];

			sprintf(fuel, "%d \n", u->belongings[RES_RETFUEL]);

			rt2.m_part.push_back(RichPart(RICH_ICON, fuelr->icon));
			rt2.m_part.push_back(RichPart(UStr(fuel)));

			if(u->cargotype >= 0 && u->cargoamt > 0)
			{
				char carry[16];

				Resource* carryr = &g_resource[u->cargotype];

				sprintf(carry, "%d \n", u->cargoamt);

				rt2.m_part.push_back(RichPart(RICH_ICON, carryr->icon));
				rt2.m_part.push_back(RichPart(UStr(carry)));
			}

#if 0
			char add[16];
			sprintf(add, "this%d\n", (int)(u-g_unit));
			rt2.m_part.push_back(RichPart(UStr(add)));
#endif
		}

		//Vec3i cmpos = Vec3i( u->cmpos.x, u->cmpos.y,
		//	SurfTile(u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE).elev * TILE_RISE );
		//Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		Font* f = &g_font[MAINFONT8];
		//DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y + f->gheight, &rt2);
		DrawBoxShadText(MAINFONT8, screenpos.x - f->gheight*2, screenpos.y + f->gheight, g_width, g_height, &rt2, color, 0, -1);
	}

	Vec2i pixmin = g_scroll;
	Vec2i pixmax = g_scroll + Vec2i(g_width,g_height);

	//show lack-of-conduit-access hovering icons
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;
		
		if(!IsTileVis(g_localP, b->tpos.x, b->tpos.y))
			continue;

		//if(!b->finished)
		//	continue;

		//if(!g_frustum.pointin(b->drawpos.x, b->drawpos.y, b->drawpos.y))
		//	continue;

		//corpd fix check frustum
		Depthable* d = b->depth;

		if(d->pixmin.x > pixmax.x ||
			d->pixmin.y > pixmax.y ||
			d->pixmax.x < pixmin.x ||
			d->pixmax.y < pixmin.y)
			continue;

		bool showcd[CD_TYPES];
		memset(showcd, -1, sizeof(showcd));
		int showcnt = CD_TYPES;
		BlType* bt = &g_bltype[b->type];

		//now generic and moddable

		for(int ci=0; ci<CD_TYPES; ci++)
		{
			//does this bltype use this cdtype?
			bool usescd = false;

			for(int ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(r->conduit != ci)
					continue;

				if(b->finished &&
				bt->input[ri] <= 0)
					continue;

				if(!b->finished &&
				bt->conmat[ri] <= 0)
					continue;

				usescd = true;
				break;
			}

			if(!usescd)
			{
				showcnt--;
				showcd[ci] = false;
				continue;
			}

			CdType* ct = &g_cdtype[ci];

			if(ct->blconduct)
			{
				short& netw = *(short*)(((char*)b)+ct->netwoff);
				if(netw >= 0)
				{
					showcnt--;
					showcd[ci] = false;
				}
			}
			else
			{
				std::list<short>* netw = (std::list<short>*)(((char*)b)+ct->netwoff);
				if(netw->size() > 0)
				{
					showcnt--;
					showcd[ci] = false;
				}
			}
		}

		if(showcnt <= 0)
			continue;

		Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0),
							 b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0),
							 SurfTile(b->tpos.x, b->tpos.y).elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//int x = b->drawpos.x - (25 * showcnt)/2;
		//int y = b->drawpos.y + 25;
		int x = screenpos.x;
		int y = screenpos.y;

		for(int ci=0; ci<CD_TYPES; ci++)
		{
			if(!showcd[ci])
				continue;

			CdType* ct = &g_cdtype[ci];
			Texture* tex = &g_texture[ct->lacktex];
			DrawImage(tex->texname, x, y, x+25, y+25, 0,0,1,1, g_gui.m_crop);

			x += 25;
		}
	}

	EndS();

	UseS(SHADER_COLOR2D);
	s = &g_shader[g_curS];
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

	//show production met % and next cycle count down timer and HP
	//and owner color box
	//and for sale price
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;
		
		if(!IsTileVis(g_localP, b->tpos.x, b->tpos.y))
			continue;
		
		float pos[4];

		BlType* bt = &g_bltype[b->type];

		Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0) - bt->width.x*TILE_SIZE/2,
							 b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0) + bt->width.y*TILE_SIZE/2,
							 0 );
		cmpos.z = (int)Bilerp(&g_hmap, (float)cmpos.x, (float)cmpos.y);
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;
		
		//owner color box
		float* owncolor = g_player[b->owner].color;
		pos[0] = screenpos.x + 8 + 1;
		pos[2] = pos[0] + 8;
		pos[3] = screenpos.y;
		pos[1] = pos[3] - 8;
		DrawSquare(0, 0, 0, 1, pos[0]-1, pos[1]-1, pos[2]+1, pos[3]+1, g_gui.m_crop);
		DrawSquare(owncolor[0], owncolor[1], owncolor[2], owncolor[3], pos[0], pos[1], pos[2], pos[3], g_gui.m_crop);

		if(!b->finished)
			continue;

		//if(!g_frustum.pointin(b->drawpos.x, b->drawpos.y, b->drawpos.y))
		//	continue;
		
		//corpd fix check frustum
		Depthable* d = b->depth;

		if(d->pixmin.x > pixmax.x ||
			d->pixmin.y > pixmax.y ||
			d->pixmax.x < pixmin.x ||
			d->pixmax.y < pixmin.y)
			continue;

#if 0
		std::string t;
		char add[64];
		sprintf(add, "%%%d met\n", b->cymet);
		t += add;
		sprintf(add, "%0.1f next\n", (CYCLE_FRAMES - (g_simframe - b->lastcy))/(float)SIM_FRAME_RATE);
		t += add;
		RichText rt(UStr(t.c_str()));
		//DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, &rt);

		if(g_zoom > 0.1f)
			DrawBoxShadText(MAINFONT16, screenpos.x - 16, screenpos.y - 16, 128, 128, &rt, color, 0, -1);
		else if(g_zoom > 0.04f)
			DrawBoxShadText(MAINFONT8, screenpos.x - 16, screenpos.y - 8, 128, 128, &rt, color, 0, -1);
#else

		//HP
		pos[0] = screenpos.x;
		pos[3] = screenpos.y;
		pos[2] = pos[0] + 2;
		pos[1] = pos[3] - bt->maxhp / 100;
		//DrawSquare(0, 0, 0, 1.0f, pos[0], pos[1], pos[2], pos[3]);
		pos[1] = pos[3] - b->hp / 100;
		//DrawSquare(1.0f, 0.2f, 0.2f, 1.0f, pos[0], pos[1], pos[2], pos[3]);

		//production met %
		pos[0] = pos[2] + 1;
		pos[2] = pos[0] + 2;
		//pos[0] = screenpos.y - b->prodlevel / 2 / 5;
		pos[1] = pos[3] - b->prodlevel / 30;
		DrawSquare(0, 0, 0, 1.0f, pos[0], pos[1], pos[2], pos[3], g_gui.m_crop);
		pos[1] = pos[3] - b->cymet / 30;
		DrawSquare(0.2f, 1.0f, 0.2f, 1.0f,  pos[0], pos[1], pos[2], pos[3], g_gui.m_crop);

		//cycle count down timer
		pos[0] = pos[2] + 1;
		pos[2] = pos[0] + 2;
		//pos[0] = screenpos.x - 30;
		pos[1] = pos[3] - 30;
		DrawSquare(0, 0, 0, 1.0f, pos[0], pos[1], pos[2], pos[3], g_gui.m_crop);
		//pos[1] = pos[3] - (g_simframe-b->lastcy)/(WORK_DELAY*24);	//for hour work
		pos[1] = pos[3] - (g_simframe-b->lastcy)/(CYCLE_FRAMES/30);	//for second-labour
		DrawSquare(0.2f, 0.2f, 1.0f, 1.0f, pos[0], pos[1], pos[2], pos[3], g_gui.m_crop);

		pos[0] = pos[2] + 1;
		pos[2] = pos[0];
#endif

		if(b->forsale)
		{
			EndS();

			UseS(SHADER_ORTHO);
			s = &g_shader[g_curS];
			glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

			char salec[32];
			strcpy(salec, iform(b->propprice).c_str());
			Resource* r = &g_resource[RES_DOLLARS];
			RichText saler = RichText(RichPart(RICH_ICON, r->icon)) + RichText(salec);
			Font* f = &g_font[MAINFONT8];
			DrawShadowedText(MAINFONT8, pos[2] + 2, pos[3] - f->gheight, &saler);

			EndS();

			UseS(SHADER_COLOR2D);
			s = &g_shader[g_curS];
			glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		}
	}

	EndS();
	CHECKGLERROR();
	Ortho(g_width, g_height, color[0], color[1], color[2], color[3]);

	//RichText rt("lkajslkdlads");
	//DrawCenterShadText(MAINFONT32, 100,100, &rt);

end:

	return;
	//glEnable(GL_DEPTH_TEST);
}
