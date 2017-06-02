


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






#include "job.h"
#include "building.h"
#include "conduit.h"
#include "utype.h"
#include "simdef.h"
#include "simflow.h"
#include "../econ/utility.h"
#include "../path/jpspath.h"
#include "../path/partialpath.h"
#include "../path/tilepath.h"
#include "../path/anypath.h"
#include "../path/pathnode.h"
#include "../path/pathjob.h"
#include "../algo/binheap.h"
#include "../path/collidertile.h"
#include "../path/fillbodies.h"
#include "../gui/layouts/chattext.h"

//#define PREPATHJOB		//check if path to job exists before considering job
//#define PATHTOJOB	//must we pathfind to job to see if there is a path to it before we consider the job?
#define PATHTOJOB2	//optimized

// comparison, not case sensitive.
bool CompareJobs(const JobOpp& a, const JobOpp& b)
{
#if 0
  unsigned int i=0;
  while ( (i<first.length()) && (i<second.length()) )
  {
    if (tolower(first[i])<tolower(second[i])) return true;
    else if (tolower(first[i])>tolower(second[i])) return false;
    ++i;
  }
  return ( first.length() < second.length() );
#else
	return a.jobutil > b.jobutil;
#endif
}

bool FindJob(Unit* u)
{
	if(u->jobframes < LOOKJOB_DELAY_MAX)
		return false;

	StartTimer(TIMER_FINDJOB);

	u->jobframes = 0;

	if(Trapped(u, NULL))
	{
		//RichText rt = RichText("findjob trapped");
		//AddChat(&rt);
		short tin = (u->cmpos.x/TILE_SIZE) + (u->cmpos.y/TILE_SIZE)*g_mapsz.x;
		TileNode* tn = &g_tilenode[tin];
		//for(unsigned char d=0; d<SDIRS; d++)
		//tn->jams = imin(tn->jams + 3, MAX_JAM_VAL);
		UType* ut = &g_utype[u->type];
		tn->jams = imax(tn->jams, MAX_JAM_VAL - ut->size.x);
		StopTimer(TIMER_FINDJOB);
		return false;
	}

	//bool pathed = false;

	int bestjobtype = UMODE_NONE;
	int besttarget = -1;
	int besttarget2 = -1;
	//float bestDistWage = -1;
	//float distWage;
	int bestutil = -1;
	//bool fullquota;
	int bestctype = CD_NONE;
	Vec2i bestgoal;

	//Vec3f pos = camera.Position();
	//CResource* res;
	UType* ut = &g_utype[u->type];

	//LastNum("before truck job");
	std::list<JobOpp> jobs;

	StartTimer(TIMER_JOBLIST);

	//Truck jobs
	for(int i=0; i<UNITS; i++)
	{
		Unit* u2 = &g_unit[i];

		if(!u2->on)
			continue;

		if(u2->hp <= 0)
			continue;

		if(u2->type != UNIT_TRUCK)
			continue;

		//only allow firm or state belonging to same state
		if(u2->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
			continue;

		//Chat("tj0");

		//if(u->mode != AWAITINGDRIVER)
		//	continue;

#ifdef RANDOM8DEBUG
		if(i == 12)
			thatunit = u - g_unit;
#endif


		if(u2->mode != UMODE_GOSUP &&
			u2->mode != UMODE_GODEMB &&
			u2->mode != UMODE_GODEMCD &&
			u2->mode != UMODE_GOREFUEL)
		{


#ifdef RANDOM8DEBUG
		if(i == 12)
			Log("not one of truck modes");
#endif

			continue;
		}

		//Chat("tj1");

		if(u2->driver >= 0 && &g_unit[u2->driver] != u)
		{

#ifdef RANDOM8DEBUG
		if(i == 12)
			Log("driver>=0 && driver!=u");
#endif

			//AddChat(&RichText("Has driver"));

			continue;
		}

		//Chat("tj2");

		Player* py = &g_player[u2->owner];


		//Chat("tj3");

		if(py->global[RES_DOLLARS] < u2->opwage)
		{
			Bankrupt(u2->owner, "truck expenses");
			continue;
		}

		//Chat("tj4");

		//int cmdist = Magnitude(u->cmpos - u2->cmpos);
		int cmdist = JOBHEUR(u->cmpos - u2->cmpos);

		if(cmdist > MAXJOBDIST)
		{
			//AddChat(&RichText(">jobdist"));
			continue;
		}

		//int jobutil = JobUtil(u2->opwage, cmdist, DRIVE_WORK_DELAY);
		int jobutil = JobUtil(py->truckwage, cmdist, DRIVE_WORK_DELAY);

#ifndef PATHTOJOB2
		if(jobutil <= bestutil)
			continue;
#endif

#ifdef RANDOM8DEBUG
		if(i == 12)
			Log("better util");
#endif

#ifdef PATHTOJOB
		Vec2i subgoal;
		std::list<Vec2i> path;

#ifdef RANDOM8DEBUG
		if(i == 12)
			thatunit = u - g_unit;
#endif

#if 1
		JPSPath(u->type, UMODE_GODRIVE, u->cmpos.x, u->cmpos.y,
			i, -1, TARG_U, CD_NONE,
			&path, &subgoal,
			u, u2, NULL,
			u2->cmpos.x, u2->cmpos.y,
			u2->cmpos.x, u2->cmpos.y, u2->cmpos.x, u2->cmpos.y);
#else
		PartialPath(u->type, UMODE_GODRIVE, u->cmpos.x, u->cmpos.y,
			i, -1, TARG_U, CD_NONE,
			&path, &subgoal,
			u, u2, NULL,
			u2->cmpos.x, u2->cmpos.y,
			u2->cmpos.x, u2->cmpos.y, u2->cmpos.x, u2->cmpos.y,
			TILE_SIZE*2/PATHNODE_SIZE);
#endif

#ifdef RANDOM8DEBUG
		thatunit = -1;
#endif

		if(path.size() <= 0)
			continue;
#endif

#ifndef PATHTOJOB2
		bestutil = jobutil;
		besttarget = i;
		bestjobtype = UMODE_GODRIVE;
		bestgoal = u2->cmpos;
#else
		jobs.push_back(JobOpp());
		JobOpp* j = &*jobs.rbegin();
		j->jobutil = jobutil;
		j->target = i;
		j->jobtype = UMODE_GODRIVE;
		j->goal = u2->cmpos;
		j->targtype = TARG_U;
		j->ignoreu = u2;
		j->ignoreb = NULL;

		//AddChat(&RichText("have truck job"));
#endif
	}

#ifdef RANDOM8DEBUG
	thatunit = -1;
#endif

	// Construction jobs
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->finished)
			continue;
		
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
			continue;

		BlType* bt = &g_bltype[b->type];

		if(b->conmat[RES_LABOUR] >= bt->conmat[RES_LABOUR])
			continue;

		Player* py = &g_player[b->owner];

		if(py->global[RES_DOLLARS] < b->conwage)
		{
			char reason[32];
			sprintf(reason, "%s construction", bt->name);
			Bankrupt(b->owner, reason);
			continue;
		}

		Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//int cmdist = Magnitude(u->cmpos - bcmpos);
		int cmdist = JOBHEUR(u->cmpos - bcmpos);

		if(cmdist > MAXJOBDIST)
			continue;

		int jobutil = JobUtil(b->conwage, cmdist, WORK_DELAY);

#if 0
		char msg[128];
		sprintf(msg, "%u job util %d", g_simframe, jobutil);
		RichText rt(msg);
		AddChat(&rt);
#endif

#ifndef PATHTOJOB2
		//if(distWage < bestDistWage)
		if(jobutil <= bestutil)
			continue;
#endif

#ifdef PATHTOJOB
		Vec2i subgoal;
		std::list<Vec2i> path;

		JPSPath(u->type, UMODE_GOCSTJOB, u->cmpos.x, u->cmpos.y,
			i, -1, TARG_BL, CD_NONE,
			&path, &subgoal,
			u, NULL, b,
			bcmpos.x, bcmpos.y,
			bcmpos.x, bcmpos.y, bcmpos.x, bcmpos.y);

		if(path.size() <= 0)
			continue;
#endif

#ifndef PATHTOJOB2
		besttarget = i;
		bestjobtype = UMODE_GOCSTJOB;
		//bestDistWage = distWage;
		bestutil = jobutil;
		bestgoal = bcmpos;
#else
		jobs.push_back(JobOpp());
		JobOpp* j = &*jobs.rbegin();
		j->jobutil = jobutil;
		j->target = i;
		j->jobtype = UMODE_GOCSTJOB;
		j->goal = bcmpos;
		j->targtype = TARG_BL;
		j->ignoreu = NULL;
		j->ignoreb = b;
#endif
	}

	// Normal/building jobs
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;
		
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
			continue;

		//if(!b->inoperation)
		//	continue;

		BlType* bt = &g_bltype[b->type];

		if(b->metout())
			continue;

		if(b->excin(RES_LABOUR))
			continue;

		if(b->opwage <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(py->global[RES_DOLLARS] < b->opwage)
		{
			char reason[32];
			sprintf(reason, "%s expenses", bt->name);
			Bankrupt(b->owner, reason);
			continue;
		}

		Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//int cmdist = Magnitude(u->cmpos - bcmpos);
		int cmdist = JOBHEUR(u->cmpos - bcmpos);

		if(cmdist > MAXJOBDIST)
			continue;

		int jobutil = JobUtil(b->conwage, cmdist, WORK_DELAY);

#ifndef PATHTOJOB2
		//if(distWage < bestDistWage)
		if(jobutil <= bestutil)
			continue;
#endif

#ifdef PATHTOJOB
		Vec2i subgoal;
		std::list<Vec2i> path;

		JPSPath(u->type, UMODE_GOBLJOB, u->cmpos.x, u->cmpos.y,
			i, -1, TARG_BL, CD_NONE,
			&path, &subgoal,
			u, NULL, b,
			bcmpos.x, bcmpos.y,
			bcmpos.x, bcmpos.y, bcmpos.x, bcmpos.y);

		if(path.size() <= 0)
			continue;
#endif

#ifndef PATHTOJOB2
		besttarget = i;
		bestjobtype = UMODE_GOBLJOB;
		//bestDistWage = distWage;
		bestutil = jobutil;
		bestgoal = bcmpos;
#else
		jobs.push_back(JobOpp());
		JobOpp* j = &*jobs.rbegin();
		j->jobutil = jobutil;
		j->target = i;
		j->jobtype = UMODE_GOBLJOB;
		j->goal = bcmpos;
		j->targtype = TARG_BL;
		j->ignoreu = NULL;
		j->ignoreb = b;
#endif
	}

	//LastNum("after truck job 2");

	//Infrastructure/conduits construction jobs
	for(int ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int x=0; x<g_mapsz.x; x++)
		{
			for(int y=0; y<g_mapsz.y; y++)
			{
				CdTile* ctile = GetCd(ctype, x, y, false);

				if(!ctile->on)
					continue;

				if(ctile->finished)
					continue;
				
				//only allow firm or state belonging to same state
				if(ctile->owner / (FIRMSPERSTATE+1) != u->owner / (FIRMSPERSTATE+1))
					continue;

				if(ctile->conmat[RES_LABOUR] >= ct->conmat[RES_LABOUR])
					continue;

				Player* py = &g_player[ctile->owner];

				if(py->global[RES_DOLLARS] < ctile->conwage)
				{
					Bankrupt(ctile->owner, "infrastructure construction");
					continue;
				}

				Vec2i ccmpos = Vec2i(x,y) * TILE_SIZE + ct->physoff;
				//int cmdist = Magnitude(u->cmpos - ccmpos);
				int cmdist = JOBHEUR(u->cmpos - ccmpos);

				if(cmdist > MAXJOBDIST)
					continue;

				int jobutil = JobUtil(ctile->conwage, cmdist, WORK_DELAY);

#ifndef PATHTOJOB2
				if(jobutil <= bestutil)
					continue;
#endif

#ifdef PATHTOJOB
				Vec2i subgoal;
				std::list<Vec2i> path;

				JPSPath(u->type, UMODE_GOCDJOB, u->cmpos.x, u->cmpos.y,
					x, y, TARG_CD, ctype,
					&path, &subgoal,
					u, NULL, NULL,
					ccmpos.x, ccmpos.y,
					ccmpos.x, ccmpos.y, ccmpos.x, ccmpos.y);

				if(path.size() <= 0)
					continue;
#endif

#ifndef PATHTOJOB2
				besttarget = x;
				besttarget2 = z;
				bestjobtype = UMODE_GOCDJOB;
				bestctype = ctype;
				bestutil = jobutil;
				bestgoal = ccmpos;
#else
				jobs.push_back(JobOpp());
				JobOpp* j = &*jobs.rbegin();
				j->jobutil = jobutil;
				j->target = x;
				j->target2 = y;
				j->ctype = ctype;
				j->jobtype = UMODE_GOCDJOB;
				j->goal = ccmpos;
				j->targtype = TARG_CD;
				j->ignoreu = NULL;
				j->ignoreb = NULL;

				//RichText msg("cd job");
				//AddChat(&msg);
#endif
			}
		}
	}


	StopTimer(TIMER_JOBLIST);
	StartTimer(TIMER_JOBSORT);

	//u->jobframes = 0;

#ifndef PATHTOJOB2
	if(bestutil <= 0 || bestjobtype == UMODE_NONE)
	{
		ResetGoal(u);
		StopTimer(TIMER_FINDJOB);
		return false;
	}
#else
	jobs.sort(CompareJobs);
#endif

	StopTimer(TIMER_JOBSORT);

#ifndef PATHTOJOB2
	ResetMode(u);

	u->mode = bestjobtype;
	u->goal = bestgoal;
	u->target = besttarget;
	u->target2 = besttarget2;
	u->cdtype = bestctype;

	StopTimer(TIMER_FINDJOB);
#else

	StartTimer(TIMER_JOBPATH);

	int nminx = (u->cmpos.x-MAXJOBDIST/2)/PATHNODE_SIZE;
	int nminy = (u->cmpos.y-MAXJOBDIST/2)/PATHNODE_SIZE;
	int nmaxx = (u->cmpos.x+MAXJOBDIST/2)/PATHNODE_SIZE;
	int nmaxy = (u->cmpos.y+MAXJOBDIST/2)/PATHNODE_SIZE;

	//corpd fix xp
	nminx = imax(0,nminx);
	nminy = imax(0,nminy);
	nmaxx = imin(g_mapsz.x*TILE_SIZE-1,nmaxx);
	nmaxy = imin(g_mapsz.y*TILE_SIZE-1,nmaxy);

	//int numpaths = 0;

	for(auto jit=jobs.begin(); jit!=jobs.end(); jit++)
	{
		JobOpp* j = &*jit;

		if(j->jobutil <= 0)
		{
			//AddChat(&RichText("jobutil 0"));
			goto fail;
		}

#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check");
			AddChat(&m);
		}
#endif

		//numpaths++;

		//first pathable job
#if 1	//doesn't work because checks corner pathnode of tile for abruptness only, which invalidates whole tile at map edge
		//high level check first
		//edit: doesn't cause any problems?
		if(Trapped( u, j->ignoreu ))
		{
			short tin = (u->cmpos.x/TILE_SIZE) + (u->cmpos.y/TILE_SIZE)*g_mapsz.x;
			TileNode* tn = &g_tilenode[tin];
			//tn->jams = imin(tn->jams + 3, MAX_JAM_VAL);
			UType* ut = &g_utype[u->type];
			tn->jams = imax(tn->jams, MAX_JAM_VAL - ut->size.x);

#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check trap fail");
			AddChat(&m);
		}
#endif

			//AddChat(&RichText("trapped"));
			continue;
		}

		std::list<Vec2s> tpath;

		TilePath(u->type, j->jobtype, u->cmpos.x, u->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype,
			&tpath,
			u, j->ignoreu, j->ignoreb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			g_mapsz.x*g_mapsz.y);

		if(tpath.size() <= 0)
		{
			
#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check tpath fail");
			AddChat(&m);
		}
#endif
			continue;
		}
#endif

		std::list<Vec2i> path;
		Vec2i subgoal;
		UType* ut = &g_utype[u->type];

		PartialPath(u->type, j->jobtype, u->cmpos.x, u->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype,
			&path, &subgoal,
			u, j->ignoreu, j->ignoreb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			MAXJOBDISTNODES * ut->cmspeed, 
			true, false);

		/*
		for some reason i get to a dead end situation where all the trucks can't be jobbed due to this part
		using hierpath. so just ignore jams for going to drive trucks, since only 1 unit needs to get to it,
		unlike buildings, which usually (stores) have a whole crowd of people lined up. and really, the whole
		reason for jams checking was because of worker clumps around stores.
		edit: maybe it's better for a truck to abandon its job if it's trapped and let another one take it up.
		*/
#ifndef HIERPATH	//this is the fix! but trucks can find paths despite being said trapped (otherwise), why?
		//edit: i see, maybe it's the "random" movement when no cm-path is found to next tile
		if(j->jobtype == UMODE_GODRIVE &&
			Trapped( &g_unit[j->target], u ))
		{
			//if(j->target == 15)
			//	InfoMess("15t", "15t");

			Unit* tu = &g_unit[j->target];
			short tin = (tu->cmpos.x/TILE_SIZE) + (tu->cmpos.y/TILE_SIZE)*g_mapsz.x;
			TileNode* tn = &g_tilenode[tin];
			//for(unsigned char d=0; d<SDIRS; d++)
			//tn->jams = imin(tn->jams + 3, MAX_JAM_VAL);
			UType* ut = &g_utype[u->type];
			tn->jams = imax(tn->jams, MAX_JAM_VAL - ut->size.x);

			ResetMode(tu);

			
#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check trap fail 2");
			AddChat(&m);
		}
#endif

			continue;
		}
#endif

		ResetMode(u);

		u->mode = j->jobtype;
		u->goal = j->goal;
		u->target = j->target;
		u->target2 = j->target2;
		u->cdtype = j->ctype;
		u->exputil = j->jobutil;
		u->path = path;
		u->subgoal = subgoal;

		StopTimer(TIMER_JOBPATH);
		StopTimer(TIMER_FINDJOB);

		return true;
	}

fail:
	StopTimer(TIMER_JOBPATH);
	StopTimer(TIMER_FINDJOB);
	ResetGoal(u);

	return false;

#endif

#if 0

	//ResetGoal();
	//LastNum("after truck job 5");

	if(bestjobtype == NONE)
	{
		//LastNum("after truck job 5a");
		//char msg[128];
		//sprintf(msg, "none j %d", UnitID(this));
		//Chat(msg);
		if(pathed)
			jobframes = 0;
		ResetMode();
		return false;
	}
	else
	{
		//LastNum("after truck job 5b");
		ResetGoal();
	}
#endif
	return true;
}

//see if this new job is better than the one any labourer is currently going to
void NewJob(int jobtype, int target, int target2, int cdtype)
{
	//return;

	//std::list<Vec2i> prevpath;
	//int prevstep;

	Vec2i newgoal;
	int newpay = 0;
	signed char targtype = -1;
	Unit* ignoreu = NULL;
	Building* ignoreb = NULL;

	//string jobname = "unknown";

	//float newreqlab = 0;
	//float newworkdelay = 0;

	if(jobtype == UMODE_GOBLJOB)
	{
		targtype = TARG_BL;
		newgoal = g_building[target].tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		Building* b = &g_building[target];
		ignoreb = b;
		Player* p = &g_player[b->owner];
		//newpay = p->wage[b->type];
		newpay = b->opwage;
		//jobname = "normal job";
		//newreqlab = b->netreq(LABOUR);
		//newworkdelay = WORK_DELAY;
	}
	else if(jobtype == UMODE_GOCSTJOB)
	{
		targtype = TARG_BL;
		newgoal = g_building[target].tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		Building* b = &g_building[target];
		ignoreb = b;
		Player* p = &g_player[b->owner];
		//newpay = p->conwage;
		newpay = b->conwage;
		//jobname = "construction job";
		//newreqlab = b->netreq(LABOUR);
		//newworkdelay = WORK_DELAY;
	}
	else if(jobtype == UMODE_GOCDJOB)
	{
		targtype = TARG_CD;
		CdType* ct = &g_cdtype[cdtype];
		newgoal = Vec2i(target, target2) * TILE_SIZE + ct->physoff;
		CdTile* ctile = GetCd(cdtype, target, target2, false);
		//CPlayer* p = &g_player[road->owner];
		//newpay = p->conwage;
		newpay = ctile->conwage;
		//jobname = "road job";
		//newreqlab = road->netreq(LABOUR);
		//newworkdelay = WORK_DELAY;
	}
	else if(jobtype == UMODE_GODRIVE)
	{
		targtype = TARG_U;
		Unit* u = &g_unit[target];
		ignoreu = u;
		newgoal = u->cmpos;
		Player* p = &g_player[u->owner];
		newpay = p->truckwage;
		//jobname = "drive job";

		//float trucklen = u->pathlength();
		//CUnitType* truckt = &g_unitType[u->type];
		//float truckseconds = trucklen / truckt->speed / (float)FRAME_RATE;
		//newworkdelay = DRIVE_WORK_DELAY;
		//newreqlab = LABOURER_LABOUR;	//max(1, truckseconds / newworkdelay * 1000.0f);
	}

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER)
			continue;

		int prevjobtype = u->mode;

		if(prevjobtype == jobtype && u->target == target)
		{
			if(jobtype == UMODE_GOBLJOB || jobtype == UMODE_GOCSTJOB || jobtype == UMODE_GODRIVE)
			{
				//u->mode = UMODE_NONE;
				//ResetGoal(u);
				continue;
			}
			else if(jobtype == UMODE_GOCDJOB)
			{
				if(u->target2 == target2 && u->cdtype == cdtype)
				{
					//u->mode = UMODE_NONE;
					//ResetGoal(u);
					continue;
				}
			}
		}

		if(prevjobtype == UMODE_GOBLJOB ||
			prevjobtype == UMODE_GOCSTJOB ||
			prevjobtype == UMODE_GOCDJOB ||
			prevjobtype == UMODE_GODRIVE)
		{
			//u->NewJob();
			//prevgoal = u->goal;
			//prevsubgoal = u->subgoal;

			//int newpathlen = Magnitude(newgoal - u->cmpos);
			//int currpathlen = Magnitude(u->goal - u->cmpos);
			int newpathlen = JOBHEUR(newgoal - u->cmpos);
			int currpathlen = JOBHEUR(u->goal - u->cmpos);
			int currpay = 0;
			//int currreqlab = 0;
			//int currworkdelay = 0;

			if(newpathlen > MAXJOBDIST)
				continue;

			if(prevjobtype == UMODE_GOCSTJOB)
			{
				Building* b = &g_building[u->target];
				//CPlayer* p = &g_player[b->owner];
				currpay = b->conwage;
				//currreqlab = min(u->labour, b->netreq(LABOUR));
				//currworkdelay = WORK_DELAY;
			}
			else if(prevjobtype == UMODE_GOBLJOB)
			{
				Building* b = &g_building[u->target];
				//CPlayer* p = &g_player[b->owner];
				//currpay = p->wage[b->type];
				currpay = b->opwage;
				//currreqlab = min(u->labour, b->netreq(LABOUR));
				//currworkdelay = WORK_DELAY;
			}
			else if(prevjobtype == UMODE_GOCDJOB)
			{
				CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
				//CPlayer* p = &g_player[road->owner];
				//currpay = p->conwage;
				currpay = ctile->conwage;
				//currreqlab = min(u->labour, road->netreq(LABOUR));
				//currworkdelay = WORK_DELAY;
			}
			else if(prevjobtype == UMODE_GODRIVE)
			{
				Unit* truck = &g_unit[u->target];
				Player* p = &g_player[truck->owner];
				currpay = p->truckwage;
				//float trucklen = truck->pathlength();
				//CUnitType* truckt = &g_unitType[truck->type];
				//float truckseconds = trucklen / truckt->speed / (float)FRAME_RATE;
				//currworkdelay = DRIVE_WORK_DELAY;
				//currreqlab = u->labour;	//min(u->labour, max(1, truckseconds / currworkdelay * 1000.0f));
			}

			int currutil = JobUtil(currpay, currpathlen, WORK_DELAY);
			int newutil = JobUtil(newpay, newpathlen, WORK_DELAY);

			if(newutil < currutil)
				continue;

#if 0
			int jobtype, int target, int target2, int cdtype)
{
	//return;

	//std::list<Vec2i> prevpath;
	//int prevstep;

	Vec2i newgoal;
#endif

	/*
	Important: the pathfinding method here, including its search limit and whether
	it finds a path or doesn't in all cases, must match what is in FindJob and NewJob and 
	MoveUnit, because the unit might get stuck in loop as a new job is found for it, but 
	it's unable to adjust its path accordingly.
	*/
#ifdef HIERPATH
			std::list<Vec2s> tpath;

			TilePath(u->type, jobtype, u->cmpos.x, u->cmpos.y,
				target, target2, targtype, cdtype,
				&tpath,
				u, ignoreu, ignoreb,
				newgoal.x, newgoal.y,
				newgoal.x, newgoal.y, newgoal.x, newgoal.y,
				g_mapsz.x*g_mapsz.y);

			if(tpath.size() <= 0)
				continue;
#elif 0
			//TO DO: make us specify goal bounds outside of pathing functions,
			//so they don't have to do it for us.
			if(!AnyPath(u->type, jobtype, u->cmpos.x, u->cmpos.y,
				target, target2, targtype, cdtype,
				u, ignoreu, ignoreb,
				newgoal.x, newgoal.y,
				newgoal.x, newgoal.y, newgoal.x, newgoal.y,
				newgoal.x/PATHNODE_SIZE, newgoal.y/PATHNODE_SIZE, newgoal.x/PATHNODE_SIZE, newgoal.y/PATHNODE_SIZE))
				continue;
#elif 1

#if 1
			//high level check first
			std::list<Vec2s> tpath;

			TilePath(u->type, jobtype, u->cmpos.x, u->cmpos.y,
				target, target2, targtype, cdtype,
				&tpath,
				u, ignoreu, ignoreb,
				newgoal.x, newgoal.y,
				newgoal.x, newgoal.y, newgoal.x, newgoal.y,
				g_mapsz.x*g_mapsz.y);

			if(tpath.size() <= 0)
				continue;
#endif

			std::list<Vec2i> path;
			Vec2i subgoal;
			UType* ut = &g_utype[u->type];

			PartialPath(u->type, jobtype, u->cmpos.x, u->cmpos.y,
					target, target2, targtype, cdtype,
					&path, &subgoal,
					u, ignoreu, ignoreb,
					newgoal.x, newgoal.y,
					newgoal.x, newgoal.y, newgoal.x, newgoal.y,
					MAXJOBDISTNODES * ut->cmspeed, 
					true, false);

			if(path.size() <= 0)
				continue;
#endif

			if(jobtype == UMODE_GODRIVE &&
				Trapped( &g_unit[target], u ))
			{
				Unit* tu = &g_unit[target];
				short tin = (tu->cmpos.x/TILE_SIZE) + (tu->cmpos.y/TILE_SIZE)*g_mapsz.x;
				TileNode* tn = &g_tilenode[tin];
				//for(unsigned char d=0; d<SDIRS; d++)
				//tn->jams = imin(tn->jams + 3, MAX_JAM_VAL);
				UType* ut = &g_utype[tu->type];
				tn->jams = imax(tn->jams, MAX_JAM_VAL - ut->size.x);
				continue;
			}

			//otherwise, switch to new job
			u->resetpath();
			u->goal = newgoal;
			u->mode = jobtype;
			u->target = target;
			u->target2 = target2;
			u->cdtype = cdtype;
			u->exputil = newutil;
			//isn't necessary for labourers, and might not be reset,
			//being passed to a pathing function and causing wrong behaviour.
			//u->targtype = targtype;

#if 0
			CUnitType* t = &g_unitType[u->type];

			float currtimeearn = currreqlab * currpay / (currpathlen / t->speed / (float)FRAME_RATE + currreqlab * currworkdelay / 1000.0f);

			u->target = target;
			u->target2 = target2;
			u->mode = jobtype;
			u->goal = newgoal;

			float newpathlen;
			float newreqlab2;
			float newtimeearn;

			if(!u->Pathfind())
			{
				goto worsejob;
			}

			newpathlen = u->pathlength();
			newreqlab2 = min(u->labour, newreqlab);
			newtimeearn = newreqlab2 * newpay / (newpathlen / t->speed / (float)FRAME_RATE + newreqlab2 * newworkdelay / 1000.0f);

			//if(newpay / newpathlen > currpay / currpathlen)
			if(newtimeearn > currtimeearn)
			{
				//Log("found better job ["<<jobname<<"] path length = "<<newpathlen<<" ("<<(newpathlen/TILE_SIZE)<<" tiles)");
				//

				continue;
			}

worsejob:
			u->target = prevtarget;
			u->target2 = prevtarget2;
			u->mode = prevjobtype;
			u->goal = prevgoal;
			u->subgoal = prevsubgoal;
			u->path.clear();
			for(int j=0; j<prevpath.size(); j++)
				u->path.push_back(prevpath[j]);
			u->step = prevstep;
#endif
		}
	}
}
