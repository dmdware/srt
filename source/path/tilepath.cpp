


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




#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../render/transaction.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../bsp/trace.h"
#include "../algo/binheap.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "tilepath.h"
#include "fillbodies.h"
#include "../sim/simflow.h"

//not engine
#include "../gui/layouts/chattext.h"

unsigned int pathnum = 0;


void ClearNodes(std::list<TileNode*>& toclear)
{
	for(auto niter = toclear.begin(); niter != toclear.end(); niter++)
	{
		TileNode* n = *niter;
		n->opened = false;
		n->closed = false;
		n->previous = NULL;
		n->tregs.clear();
	}
}

bool CompareTiles(void* a, void* b)
{
	return ((TileNode*)a)->score > ((TileNode*)b)->score;
}

Vec2s TileNodePos(TileNode* node)
{
	if(!node)
		return Vec2s(-1,-1);

	const int i = node - g_tilenode;
	const int ny = i / g_mapsz.x;
	const int nx = i % g_mapsz.x;
	return Vec2s(nx, ny);
}

void UpdJams()
{
	//TODO too heavy on the CPU
	//return;

	//only decrement jam every minute
	if(g_simframe % UPD_JAMS_DELAY != 0)
		return;

	for(short y=0; y<g_mapsz.y; y++)
		for(short x=0; x<g_mapsz.x; x++)
		{
			short index = x + y * g_mapsz.x;
			TileNode* tn = &g_tilenode[index];
			//for(unsigned char d=0; d<SDIRS; ++d)
			{
				if(tn->jams <= 0)
					continue;
				tn->jams --;
			}
		}
}

/*
TilePath + sub-pathing (hierarchical pathfinding)
won't work if you have to end up in certain tile
regions. If you automatically get access to a tile
region by being in the current tile,
it will work. So if you have a sub-tile-level
zig-zagging maze, this won't work.
If you have forests that might separate
a tile into two, this will work.
*/

void TilePath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, signed char cdtype,
			  std::list<Vec2s> *tpath, Unit* thisu, Unit* ignoreu, Building* ignoreb,
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
			  int maxsearch, bool ignorejams)
{
	UType* ut = &g_utype[utype];

#if 0
	if(thisu)
	{
		RichText rt("pathf");
		NewTransx(thisu->cmpos, &rt);
	}
#endif

	if(umode == UMODE_GODRIVE)
		ignorejams = true;

	if(!ignoreb)
	{
		switch(umode)
		{
		case UMODE_GODEMB:
		case UMODE_ATDEMB:
		case UMODE_GOBLJOB:
		case UMODE_BLJOB:
		case UMODE_GOCSTJOB:
		case UMODE_CSTJOB:
		case UMODE_GOSHOP:
		case UMODE_SHOPPING:
		case UMODE_GOREST:
		case UMODE_RESTING:
			ignoreb = &g_building[target];
			break;
		case UMODE_GOSUP:
		case UMODE_ATSUP:
			if(thisu)
				ignoreb = &g_building[thisu->supplier];
			break;
		case UMODE_GOREFUEL:
		case UMODE_REFUELING:
			if(thisu)
				ignoreb = &g_building[thisu->fuelstation];
			break;
		case UMODE_NONE:
		case UMODE_GOCDJOB:
		case UMODE_CDJOB:
		case UMODE_GODRIVE:
		case UMODE_DRIVE:
		case UMODE_GODEMCD:
		case UMODE_ATDEMCD:
			break;
		}
	}

	if(!ignoreu)
	{
		switch(umode)
		{
		case UMODE_GOSUP:
		case UMODE_GODEMB:
		case UMODE_GOREFUEL:
		case UMODE_REFUELING:
		case UMODE_ATDEMB:
		case UMODE_ATSUP:
		case UMODE_GOBLJOB:
		case UMODE_BLJOB:
		case UMODE_GOCSTJOB:
		case UMODE_CSTJOB:
		case UMODE_GOSHOP:
		case UMODE_SHOPPING:
		case UMODE_GOREST:
		case UMODE_RESTING:
		case UMODE_NONE:
		case UMODE_GOCDJOB:
		case UMODE_CDJOB:
		case UMODE_GODEMCD:
		case UMODE_ATDEMCD:
			break;
		case UMODE_GODRIVE:
		case UMODE_DRIVE:
			ignoreu = &g_unit[target];
			break;
		}
	}

	//could be cleaner, not rely on thisu->cdtype
	//if(targtype == TARG_CD && thisu)
	// TO DO
	if((umode == UMODE_GODEMCD ||
		umode == UMODE_ATDEMCD ||
		umode == UMODE_GOCDJOB) &&
		thisu)
	{
		CdType* ct = &g_cdtype[cdtype];
		//...and not muck around with cmgoalmin/max
		CdTile* ctile = GetCd(cdtype, target, target2, false);
		Vec2i ccmpos = Vec2i(target, target2)*TILE_SIZE + ct->physoff;
		cmgoalminx = ccmpos.x - TILE_SIZE/2;
		cmgoalminy = ccmpos.y - TILE_SIZE/2;
		cmgoalmaxx = cmgoalminx + TILE_SIZE;
		cmgoalmaxy = cmgoalminy + TILE_SIZE;
	}

#if 0
	if((umode == UMODE_GOSUP ||
		umode == UMODE_GODEMB ||
		umode == UMODE_GOREFUEL) &&
		thisu)
	{
		Building* b;

		if(umode == UMODE_GOSUP)
			b = &g_building[thisu->supplier];
		else if(umode == UMODE_GODEMB)
			b = &g_building[thisu->target];
		else if(umode == UMODE_GOREFUEL)
			b = &g_building[thisu->fuelstation];

		BlType* bt = &g_bltype[b->type];

		int btminx = b->tpos.x - bt->width.x/2;
		int btminz = b->tpos.y - bt->width.y/2;
		int btmaxx = btminx + bt->width.x - 1;
		int btmaxz = btminz + bt->width.y - 1;

		cmgoalminx = btminx * TILE_SIZE;
		cmgoalminy = btminz * TILE_SIZE;
		cmgoalmaxx = cmgoalminx + bt->width.x*TILE_SIZE - 1;
		cmgoalmaxy = cmgoalminy + bt->width.y*TILE_SIZE - 1;
	}
#endif

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstarty = cmstarty;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->tpath = tpath;
	//pj->path = path;
	//pj->subgoal = subgoal;
	pj->thisu = thisu ? thisu - g_unit : -1;
	pj->ignoreu = ignoreu ? ignoreu - g_unit : -1;
	pj->ignoreb = ignoreb ? ignoreb - g_building : -1;
	pj->goalx = cmgoalx / TILE_SIZE;
	pj->goaly = cmgoalz / TILE_SIZE;
	pj->goalminx = cmgoalminx;
	pj->goalminy = cmgoalminy;
	pj->goalmaxx = cmgoalmaxx;
	pj->goalmaxy = cmgoalmaxy;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath; //call TODO
	pj->pjtype = PATHJOB_TILE;
	pj->maxsearch = maxsearch;
	pj->nminx = 0;
	pj->nminy = 0;
	pj->nmaxx = g_pathdim.x-1;
	pj->nmaxy = g_pathdim.y-1;

	pj->tpath->clear();

	int nx = cmstartx / PATHNODE_SIZE;
	int ny = cmstarty / PATHNODE_SIZE;
	int nin = nx + ny * g_pathdim.x;
	unsigned char treg = g_tregs[nin];
	int tx = cmstartx / TILE_SIZE;
	int ty = cmstarty / TILE_SIZE;

	int maxallowjam = 0;
	int nextmax = 0;

	//do
	{
		BinHeap openlist(CompareTiles);
		std::list<TileNode*> toclear;

		int tin = tx + ty * g_mapsz.x;
		TileNode* tnode = &g_tilenode[tin];
		tnode->opened = true;
		tnode->totalD = 0;
		tnode->score = 1;
		tnode->previous = NULL;
		tnode->tregs.push_back(treg);
		openlist.insert(tnode);
		toclear.push_back(tnode);

#ifdef HIERDEBUG
		pathnum ++;

		//if(pathnum == 73)
		if(thisu - g_unit == 19)
		{
			Log("the 13th unit:");
			g_speed = SPEED_PAUSE;
			Unit* u = thisu;
			//g_cam.move( u->drawpos - g_cam.m_view );
			g_zoom = MAX_ZOOM;
		}

		Log("tpath----"<<" #"<<pathnum);
#endif

		while(openlist.hasmore())
		{
			tnode = (TileNode*)openlist.deletemin();

			tnode->closed = true;
			int runningD = tnode->totalD;

			Vec2s tpos = TileNodePos(tnode);
			
#if 0
		if(pj->thisu == 36)
		{
			int dtx = (int)(tpos.x-pj->goalx);
			int dty = (int)(tpos.y-pj->goaly);
			char cm[128];
			sprintf(cm, "36 at tpos dt%d,%d", dtx, dty);

			//if(dtx == 0 && dty < 2)
				AddChat(&RichText(cm));
		}
#endif

#ifdef HIERDEBUG
			Log("tpos "<<tpos.x<<","<<tpos.y<<" d "<<(tpos.x-pj->goalx)<<","<<(tpos.y-pj->goaly)<<" #"<<pathnum);
#endif

			//If at goal...
			unsigned int cmminx = tpos.x * TILE_SIZE;
			unsigned int cmminy = tpos.y * TILE_SIZE;
			unsigned int cmmaxx = (tpos.x+1) * TILE_SIZE - 1;
			unsigned int cmmaxy = (tpos.y+1) * TILE_SIZE - 1;

			if(cmminx <= pj->goalmaxx &&
				cmminy <= pj->goalmaxy &&
				cmmaxx >= pj->goalminx &&
				cmmaxy >= pj->goalminy)
			{
				//Return path

#if 0
				
	{
		if(pj->thisu == 36)
		{
			char cm[128];
			sprintf(cm, "36 at GOAL");

			AddChat(&RichText(cm));
		}
	}
#endif

#ifdef HIERDEBUG
				Log("rp.----- #"<<pathnum);
#endif

				//Don't include very first tile because we're already in it.
				//But what if the tpath size is only 1 tile?
				//for(TileNode* n = tnode; n->previous; n = n->previous)
				//Edit: Now just check if we're in the last tpath node,
				//then full-path to goal. Or if we start in first tpath node,
				//then pop it and path to next.
				for(TileNode* n = tnode; n; n = n->previous)
					//for(TileNode* n = tnode; n->previous || (n && pj->tpath->size() == 0); n = n->previous)
				{
					tpos = TileNodePos(n);
					pj->tpath->push_front(tpos);
				}


				//cap end? necessary? corpd fix xp
				tpos = Vec2s(pj->goalx,pj->goaly);
				pj->tpath->push_back(tpos);


#ifdef TSDEBUG
				if(tracku == thisu)
				{
					char msg[128];
					sprintf(msg, "tpath tracku success sz=%d", (int)pj->tpath->size());
					InfoMess(msg,msg);
				}
#endif

				break;
			}

			for(unsigned char i=0; i<SDIRS; i++)
			{
#ifdef HIERDEBUG
				Log("sdir"<<(int)i);
				Log("\t0");
#endif

				if(tpos.x + STRAIGHTOFFSETS[i].x < 0)
					continue;

#ifdef HIERDEBUG
				Log("\t1");
#endif

				if(tpos.x + STRAIGHTOFFSETS[i].x >= g_mapsz.x)
					continue;

#ifdef HIERDEBUG
				Log("\t2");
#endif

				if(tpos.y + STRAIGHTOFFSETS[i].y < 0)
					continue;

#ifdef HIERDEBUG
				Log("\t3");
#endif

				if(tpos.y + STRAIGHTOFFSETS[i].y >= g_mapsz.y)
					continue;

#ifdef HIERDEBUG
				Log("\t4");

				Log("ib = "<<(pj->ignoreb));
#endif

				Vec2s tpos2(tpos.x + STRAIGHTOFFSETS[i].x, tpos.y + STRAIGHTOFFSETS[i].y);

				//pass[i] = Standable(pj, tpos.x + STRAIGHTOFFSETS[i].x * TILE_SIZE/PATHNODE_SIZE, tpos.y + STRAIGHTOFFSETS[i].y * TILE_SIZE/PATHNODE_SIZE);
				if(!TileStandable(pj, (tpos2.x * TILE_SIZE + TILE_SIZE/2)/PATHNODE_SIZE, (tpos2.y * TILE_SIZE + TILE_SIZE/2)/PATHNODE_SIZE))
					continue;

#ifdef HIERDEBUG
				Log("\t5");
#endif

				tin = tpos2.x + tpos2.y * g_mapsz.x;
				TileNode* tnode2 = &g_tilenode[ tin ];
				TileRegs* tregs2 = &g_tilepass[ tin ];

				//For each region we've accessed on this tile,
				//check if it leads to this other tile.

				//Vec2s tpos2(tpos.x + STRAIGHTOFFSETS[i].x, tpos.y + STRAIGHTOFFSETS[i].y);
				//int tin2 = (tpos2.x) + (tpos2.y) * g_mapsz.x;

				unsigned char i2 = 0;

				//Straight direction
				if(i == SDIR_E)
					i2 = SDIR_W;
				else if(i == SDIR_W)
					i2 = SDIR_E;
				else if(i == SDIR_N)
					i2 = SDIR_S;
				else
					i2 = SDIR_N;

				//For each opened region in starting tile...
				for(auto rit=tnode->tregs.begin(); rit!=tnode->tregs.end(); rit++)
				{
					bool visited = false;
					bool found = false;

#ifdef HIERDEBUG
					Log("\t6 rit="<<*rit);
#endif

					//Check for passage to that region from second tile
					//(if there is a region in backwards direction there leading back to first tile region)...
					for(auto rit2=tregs2->tregs[i2].begin(); rit2!=tregs2->tregs[i2].end(); rit2++)
					{
#ifdef HIERDEBUG
						Log("\t6.1 rit2->to="<<rit2->to);
#endif

						//If leads to that first region...
						if(rit2->to != *rit)
							continue;

#ifdef HIERDEBUG
						Log("\t6.2");
#endif

#if 0
						//if(tnode2->jams > 0)
						if(!ignorejams &&
							tnode2->jams > 0 &&
							tnode2->jams > tnode->jams)
							continue;
#else
						if(//!ignorejams &&
							tnode2->jams > 0 &&
							MAX_JAM_VAL - tnode2->jams < ut->size.x &&
							tnode2->jams > tnode->jams)
						{
#if 0
							if(pj->thisu == 36)
							{
								int dtx = tpos2.x - pj->goalx;
								int dty = tpos2.y - pj->goaly;

								char cm[129];
								sprintf(cm, "JAM 36 at dt%d,%d", dtx, dty);
								AddChat(&RichText(cm));
							}
#endif

							continue;
						}

						//special spreading rule for humanoids
						if(tnode2->jams > 0 &&
							ut->size.x < 60	&& //humanoid?
							tnode2->jams > tnode->jams)
						{
							continue;
						}

#endif

						if(tnode2->closed)
						{
#ifdef HIERDEBUG
							Log("\t10");
#endif

							//Visited and expanded this tile.

							int newD = runningD + 2;

							if(newD > tnode2->totalD)
								continue;

							found = true;
							//toclear.push_back(tnode2);
							tnode2->previous = tnode;
							int H = PATHHEUR( Vec2i(tpos2.x - pj->goalx, tpos2.y - pj->goaly) ) << 1;
							//if(pj->roaded)
							//H += tnode2->jams;
							//else if(pj->utype == UNIT_LABOURER)
							//	H += tnode2->jams / 16;
							tnode2->score = runningD + 2 + H;
							tnode2->totalD = newD;
							tnode2->closed = false;
							tnode2->opened = true;
							tnode2->tregs.push_back(rit2->from);
							//openlist.heapify(tnode2);
							openlist.insert(tnode2);
						}
						//If we are already waiting for this tile to queue up...
						else if(tnode2->opened)
						{
#ifdef HIERDEBUG
							Log("\t7");
#endif

							//Check visited regions in this second tile...
							for(auto vrit2=tnode2->tregs.begin(); vrit2!=tnode2->tregs.end(); vrit2++)
							{
#ifdef HIERDEBUG
								Log("\t8");
#endif

								//Have we already visited this region?
								if(*vrit2 == rit2->from)
								{
									visited = true;
									break;
								}
							}

							//If we've already expanded to this region...
							if(visited)
								continue;

							int newD = runningD + 2;

							if(newD > tnode2->totalD)
								continue;

#ifdef HIERDEBUG
							Log("\t9");
#endif

							//Else, not visited this region.
							found = true;
							//toclear.push_back(tnode2);
							tnode2->previous = tnode;
							int H = PATHHEUR( Vec2i(tpos2.x - pj->goalx, tpos2.y - pj->goaly) ) << 1;
							//if(pj->roaded)
							//H += tnode2->jams;
							//else if(pj->utype == UNIT_LABOURER)
							//	H += tnode2->jams / 16;
							tnode2->score = newD + H;
							tnode2->totalD = newD;
							tnode2->closed = false;
							tnode2->opened = true;
							tnode2->tregs.push_back(rit2->from);
							openlist.heapify(tnode2);
						}
						//not opened or closed
						else
						//else if(tnode2->jams == 0)
						{
#ifdef HIERDEBUG
							Log("\t11");
#endif

							//Not visited this tile at all.
							found = true;
							toclear.push_back(tnode2);
							tnode2->previous = tnode;
							int H = PATHHEUR( Vec2i(tpos2.x - pj->goalx, tpos2.y - pj->goaly) ) << 1;
							//if(pj->roaded)
							//H += tnode2->jams;
							//else if(pj->utype == UNIT_LABOURER)
							//	H += tnode2->jams / 16;
							tnode2->score = runningD + 2 + H;
							tnode2->totalD = runningD + 2;
							tnode2->closed = false;
							tnode2->opened = true;
							tnode2->tregs.push_back(rit2->from);
							openlist.insert(tnode2);
						}
					}

#ifdef HIERDEBUG
					Log("\t11.1");

					if(found)
						Log("found");
					Log("\t12");
#endif

					//Already found path from desired region, stop looking.
					//if(found)
					//	break;
					//Edit: no, there might be other regions in this tile.
				}
			}
		}


#ifdef TSDEBUG
		if(tracku == thisu)
		{
			char msg[128];
			sprintf(msg, "tpath tracku fail nodes=%d", (int)toclear.size());
			InfoMess(msg,msg);
		}
#endif

		ClearNodes(toclear);

#ifdef HIERDEBUG
		if(pj->tpath->size() == 0)
			Log("not found tpath #"<<pathnum);
#endif

		delete pj;
	}
	//while();
}

void Expand_T(PathJob* pj, PathNode* node)
{
	Vec2i npos = PathNodePos(node);

	int thisdistance = PATHHEUR(Vec2i(npos.x - pj->goalx, npos.y - pj->goaly)) << 1;

	if( !pj->closestnode || thisdistance < pj->closest )
	{
		pj->closestnode = node;
		pj->closest = thisdistance;
	}

	//int runningD = 0;

	//if(node->previous)
	//	runningD = node->previous->totalD;

	int runningD = node->totalD;

#if 0
	bool stand[DIRS];

	for(unsigned char i=0; i<DIRS; i++)
		stand[i] = Standable(pj, npos.x + OFFSETS[i].x, npos.y + OFFSETS[i].y);

	bool pass[DIRS];

	pass[DIR_NW] = stand[DIR_NW] && stand[DIR_N] && stand[DIR_W];
	pass[DIR_N] = stand[DIR_N];
	pass[DIR_NE] = stand[DIR_NE] && stand[DIR_N] && stand[DIR_E];
	pass[DIR_E] = stand[DIR_E];
	pass[DIR_SE] = stand[DIR_SE] && stand[DIR_S] && stand[DIR_E];
	pass[DIR_S] = stand[DIR_S];
	pass[DIR_SW] = stand[DIR_SW] && stand[DIR_S] && stand[DIR_W];
	pass[DIR_W] = stand[DIR_W];

	for(unsigned char i=0; i<DIRS; i++)
	{
#else
	bool pass[SDIRS];

	for(unsigned char i=0; i<SDIRS; i++)
		//pass[i] = Standable(pj, npos.x + STRAIGHTOFFSETS[i].x * TILE_SIZE/PATHNODE_SIZE, npos.y + STRAIGHTOFFSETS[i].y * TILE_SIZE/PATHNODE_SIZE);
		pass[i] = TileStandable(pj, npos.x + STRAIGHTOFFSETS[i].x * TILE_SIZE/PATHNODE_SIZE, npos.y + STRAIGHTOFFSETS[i].y * TILE_SIZE/PATHNODE_SIZE);

	for(unsigned char i=0; i<SDIRS; i++)
	{
#endif
		if(!pass[i])
			continue;

		//int newD = runningD + STEPDIST[i];
		int newD = runningD + 2;

		Vec2i nextnpos(npos.x + STRAIGHTOFFSETS[i].x * TILE_SIZE/PATHNODE_SIZE, npos.y + STRAIGHTOFFSETS[i].y * TILE_SIZE/PATHNODE_SIZE);
		PathNode* nextn = PathNodeAt(nextnpos.x, nextnpos.y);

		if(!nextn->closed && (!nextn->opened || newD < nextn->totalD))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->totalD = newD;
			int H = PATHHEUR( nextnpos - Vec2i(pj->goalx, pj->goaly) ) << 1;
			nextn->score = nextn->totalD + H;
			nextn->previous = node;

			if( !nextn->opened )
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
				g_openlist.heapify(nextn);
		}
	}
}
