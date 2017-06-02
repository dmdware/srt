


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
#include "../render/transaction.h"
#include "simdef.h"
#include "simflow.h"
#include "../bsp/trace.h"
#include "../phys/collision.h"
#include "../path/collidertile.h"
#include "../path/pathdebug.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "../path/jpspath.h"
#include "../path/jpspartpath.h"
#include "../path/pathnode.h"
#include "../path/partialpath.h"
#include "labourer.h"
#include "../math/fixmath.h"
#include "build.h"
#include "../path/pathjob.h"
#include "../path/tilepath.h"
#include "../path/astarpath.h"
#include "../path/anypath.h"
#include "../path/fillbodies.h"
#include "../render/fogofwar.h"

//not engine
#include "../gui/layouts/chattext.h"

//TODO circular unit radius
bool UnitCollides(Unit* u, Vec2i cmpos, int utype)
{
	UType* t = &g_utype[utype];
	int minx = cmpos.x - t->size.x/2;
	int miny = cmpos.y - t->size.x/2;
	int maxx = minx + t->size.x - 1;
	int maxy = miny + t->size.x - 1;

	int cx = cmpos.x / PATHNODE_SIZE;
	int cy = cmpos.y / PATHNODE_SIZE;

	ColliderTile* cell = ColliderAt(cx, cy);

	if(!t->seaborne && !(cell->flags & FLAG_HASLAND))
		return true;

#if 0
	if(t->roaded && !(cell->flags & FLAG_HASROAD))
		return true;
#else
	if(t->roaded)
	{
		CdTile* cdtile = GetCd(CD_ROAD, cmpos.x / TILE_SIZE, cmpos.y / TILE_SIZE, false);

		if(!cdtile->on || !cdtile->finished)
			return true;
	}
#endif

	if(cell->flags & FLAG_ABRUPT)
		return true;

#if 0
	//complete collision check - slow
	for(int i=0; i<UNITS; i++)
	{
		Unit* u2 = &g_unit[i];

		if(!u2->on)
			continue;

		if(u2 == u)
			continue;

		t = &g_utype[u2->type];
		int minx2 = u2->cmpos.x - t->size.x/2;
		int miny2 = u2->cmpos.y - t->size.x/2;
		int maxx2 = minx2 + t->size.x;
		int maxy2 = miny2 + t->size.x;

		/*
		It's important to test for
		equality as being passable
		because the units might be
		right beside each other.
		*/
		if(minx >= maxx2)
			continue;

		if(miny >= maxy2)
			continue;

		if(maxx <= minx2)
			continue;

		if(maxy <= miny2)
			continue;

		return true;
	}
#else

	int nminx = minx / PATHNODE_SIZE;
	int nminy = miny / PATHNODE_SIZE;
	int nmaxx = maxx / PATHNODE_SIZE;
	int nmaxy = maxy / PATHNODE_SIZE;

	for(int x=nminx; x<=nmaxx; x++)
		for(int y=nminy; y<=nmaxy; y++)
		{
			cell = ColliderAt(x, y);

			if(cell->flags & FLAG_ABRUPT)
				return true;	//corpc fix

			if(cell->foliage != USHRT_MAX)
				return true;

			if(cell->building >= 0)
			{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("ucb reset bi="<<cell->building<<" bt="<<g_bltype[g_building[cell->building].type].name<<" uprevpos="<<u->prevpos.x<<","<<u->prevpos.y<<" ucmpos="<<u->cmpos.x<<","<<u->cmpos.y<<" usubgoal="<<u->subgoal.x<<","<<u->subgoal.y<<" ");
		InfoMess("ucb", "ucb");
	}
#endif

#if 1
				Building* b = &g_building[cell->building];
				BlType* t2 = &g_bltype[b->type];

				int tminx = b->tpos.x - t2->width.x/2;
				int tminy = b->tpos.y - t2->width.y/2;
				int tmaxx = tminx + t2->width.x;
				int tmaxz = tminy + t2->width.y;

				int minx2 = tminx*TILE_SIZE;
				int miny2 = tminy*TILE_SIZE;
				int maxx2 = tmaxx*TILE_SIZE - 1;
				int maxy2 = tmaxz*TILE_SIZE - 1;

				if(minx <= maxx2 && miny <= maxy2 && maxx >= minx2 && maxy >= miny2)
				{
#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("\tUnitCollides=true withb="<<cell->building<<" type="<<g_bltype[b->type].name<<" ");
	}
#endif
#if 0
					Log("collides at cell ("<<(minx/PATHNODE_SIZE)<<","<<(miny/PATHNODE_SIZE)<<")->("<<(maxx/PATHNODE_SIZE)<<","<<(maxy/PATHNODE_SIZE)<<")");
					Log("subgoal = "<<(u->subgoal.x/PATHNODE_SIZE)<<","<<(u->subgoal.y/PATHNODE_SIZE));
					
#endif


#ifdef TSDEBUG
	if(u == tracku && tracku)
	{
		char msg[128];
		sprintf(msg, "collides track u bl %s id=%d targ=%d", t2->name, (int)cell->building, (int)u->target);
		InfoMess(msg, msg);
	}
#endif

					return true;
				}
#else
				return true;
#endif
			}

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter ++)
			{
				short uindex = cell->units[uiter];

				if(uindex < 0)
					continue;

				Unit* u2 = &g_unit[uindex];

				if(u2 == u)
					continue;

				if(u2->hidden())
					continue;

#if 1
				UType* t2 = &g_utype[u2->type];
				int minx2 = u2->cmpos.x - t2->size.x/2;
				int miny2 = u2->cmpos.y - t2->size.x/2;
				int maxx2 = minx2 + t2->size.x - 1;
				int maxy2 = miny2 + t2->size.x - 1;

				if(minx <= maxx2 && miny <= maxy2 && maxx >= minx2 && maxy >= miny2)
				{
#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("\tUnitCollides=true withu="<<uindex<<" type="<<g_utype[u2->type].name<<" dpos="<<(u->cmpos.x-u2->cmpos.x)<<","<<(u->cmpos.y-u2->cmpos.y)<<" u2->on="<<u2->on<<" u2->mode="<<u2->mode<<" ");
		//return false;
	}
#endif

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("ucu reset");
		InfoMess("ucu", "ucu");
	}
#endif

#ifdef TSDEBUG
	if(u == tracku && tracku)
	{
		char msg[128];
		sprintf(msg, "collides track u u %s", t2->name);
		InfoMess(msg, msg);
	}
#endif

					return true;
				}
#else
				return true;
#endif
			}
		}
#endif

	return false;
}

void MoveUnit(Unit* u)
{
	UType* t = &g_utype[u->type];
	u->prevpos = u->cmpos;
	u->collided = false;

#ifdef HIERDEBUG
	if(u - g_unit == 19)
	{
		if(UnitCollides(u, u->cmpos, u->type))
			InfoMess("prec", "prec");
	}
#endif

#if 0
	if(UnitCollides(u, u->cmpos, u->type))
	{
		char msg[128];
		sprintf(msg, "prec %s", g_utype[u->type].name);
		InfoMess("prec", msg);
	}
#endif

	if(u->threadwait)
		return;

	if(u->type == UNIT_TRUCK && u->driver < 0)
		return;

	//bug for trucks and probably labourers, but not military?
	//only when another bug has already caused goal to be not right?
	//if(Magnitude(u->goal - u->cmpos) <= t->cmspeed)
	//	return;
#if 0
	if(Magnitude(u->goal - u->cmpos) <= t->cmspeed)
	{
		if(CheckIfArrived(u))
			OnArrived(u);
		else
			//there should be something to reestablish goal to target, 
			//but I only needed this because of another bug that caused goal to be wrong
			ResetMode(u);
		return;
	}
#elif 1
	//Needed so that with large MAXJOBDISTNODES, we don't spend a lot of time
	//on unreachable paths in a crowd for labourers at the beginning of new game
	if( /* u->mode == UMODE_NONE && */
	   u->cmpos == u->goal )	//corpd fix
	{
		//RichText mess = RichText("goal-cmpos < speed");
		//AddChat(&mess);
		return;
	}
#endif

	if(u->underorder && u->target < 0 && Magnitude(u->goal - u->cmpos) <= PATHNODE_SIZE)
		return;

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("unitmove u=thatunit dgoal="<<(u->goal.x-u->cmpos.x)<<","<<(u->goal.y-u->cmpos.y)<<" ");
		Log("unitmove u=thatunit dsubgoal="<<(u->subgoal.x-u->cmpos.x)<<","<<(u->subgoal.y-u->cmpos.y)<<" ");
	}
#endif

	//Signs that we need a new path
	if( /*u->cmpos != u->goal && */	//corpd fix
	   ( u->path.size() <= 0 ||
		(*u->path.rbegin() != u->goal
#ifdef HIERPATH
		&&
			(u->tpath.size() == 0 //||
			//*u->tpath.rbegin() != Vec2s(u->goal.x/TILE_SIZE,u->goal.y/TILE_SIZE)	//doesn't work for corner-placed conduits
			//let's hope tpath will always be reset when new path is needed.
			)
#endif
			) ) )
	{


#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("unitmove u=thatunit repath");
		Log("unitmove u=thatunit repath (u->path.size() <= 0) = "<<(u->path.size() <= 0));
		Log("unitmove u=thatunit repath (*u->path.rbegin() != u->goal) = "<<(*u->path.rbegin() != u->goal));
	}
#endif

#if 1
		if(t->military)
		//if(1)
		{
			if(g_simframe - u->lastpath < u->pathdelay)
			{
				return;
			}

			//u->pathdelay += 1;
			//u->pathdelay *= 2;
			//u->pathdelay += 50;
			u->pathdelay = (u->pathdelay + 50) % PATH_DELAY;
			u->lastpath = g_simframe;

#if 1
			int nodesdist = PATHHEUR( (u->goal - u->cmpos) / PATHNODE_SIZE );

			PartialPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
						&u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y,
			            //nodesdist*nodesdist*1);
			TILE_SIZE*TILE_SIZE*4/PATHNODE_SIZE/PATHNODE_SIZE, true, true);	//enough to move 3 tiles around a corner, filled with obstacles

#if 1
			if(u->path.size() <= 0)
			{
				u->path.push_back( u->goal );
				u->subgoal = u->goal;
			}
#endif
#elif 1
			u->path.clear();
			u->path.push_back( u->goal );
			u->subgoal = u->goal;
#elif 0
			JPSPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, &u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y);
#else
			JPSPartPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, &u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y,
			            nodesdist*4);
#endif

#if 0
			RichText rtext("ppathf");
			NewTransx(u->cmpos + Vec3f(0,t->size.y,0), &rtext);
#endif
		}
		//else if not military
		else //if(!u->pathblocked)
#endif
		{

			if(g_simframe - u->lastpath < u->pathdelay)
			{
				return;
			}

			//u->pathdelay += 1;
			//u->pathdelay *= 2;
			//u->pathdelay += 10;
			u->pathdelay = (u->pathdelay + PATH_DELAY/3) % PATH_DELAY;
			u->lastpath = g_simframe;
			
			//if(Trapped()

			UType* ut = &g_utype[u->type];
			//int nodesdist = PATHHEUR( (u->goal - u->cmpos) / PATHNODE_SIZE );
			int nodesdist = MAXJOBDISTNODES * ut->cmspeed;

			//To avoid slow-downs.
			PartialPath(u->type, u->mode,
			            u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
						&u->path, &u->subgoal,
			            u, NULL, NULL,
			            u->goal.x, u->goal.y,
			            u->goal.x, u->goal.y, u->goal.x, u->goal.y,
			            //nodesdist*nodesdist*1,
			//TILE_SIZE*TILE_SIZE*20/PATHNODE_SIZE/PATHNODE_SIZE, 
			nodesdist,
			true, /*true*/ false);
			//Enough to move 19 tiles around a corner, filled with obstacles.

#if 0
			//causes clumps, as units gather around a bl that might have all entrances blocked off, 
			//but unit will move closer to goal in the meanwhile while there might be a blockage
			if(u->path.size() <= 0)
			{
				int nodesdist = PATHHEUR( (u->goal - u->cmpos) / PATHNODE_SIZE );

				PartialPath(u->type, u->mode,
							u->cmpos.x, u->cmpos.y, u->target, u->target2, u->targtype, u->cdtype,
							&u->path, &u->subgoal,
							u, NULL, NULL,
							u->goal.x, u->goal.y,
							u->goal.x, u->goal.y, u->goal.x, u->goal.y,
							nodesdist*nodesdist*10, true, true);

				u->pathblocked = true;
			}
			else
				u->pathblocked = false;
#endif

#if 1	//trucks don't work for some reason
			//induce random walking to unclump from the clumped area
			//and just to make it look like the unit doesn't know where to go
			if(u->path.size() <= 0 && g_utype[u->type].walker )
			{
				//u->path.push_back( u->goal );
				//PlaceUAb(u->type, u->cmpos, &u->subgoal);
				//u->path.push_back( u->subgoal );
				//u->subgoal = u->goal;

				//set it in a new direction (out of 8 directions) every 3 simframes
				//unsigned int timeoff = (g_simframe % (8 * 3)) / 3;
				unsigned int timeoff = ( (g_simframe + (u-g_unit)) % (8 * 3)) / 3;
				//don't set goal because that would throw off the real target goal
				u->subgoal /*= u->goal*/ = u->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 2;
				//follow the goal after subgoal so that we bump into something and repath automatically
				u->path.push_back( u->goal );

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("unitmove u=thatunit random8dir");
	}
#endif
			}
#endif
		}

		//goto endcheck;

		return;
	}

	//return;	//temp

	u->freecollider();
	RemVis(u);

	Vec2i dir = u->subgoal - u->cmpos;

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("unitmove u=thatunit dmove<speed? u->path.size()="<<u->path.size());
	}
#endif

	//Arrived at subgoal?
	//if(Magnitude2(u->subgoal - u->cmpos) <= t->cmspeed * t->cmspeed)
	if(iabs(dir.x) <= t->cmspeed &&	//necesssary extra checks to make sure dir doesn't overflow when computing Magnitude2 (dot product).
		iabs(dir.y) <= t->cmspeed &&
		Magnitude2(dir) <= t->cmspeed * t->cmspeed)
	{
		//TODO put this and below copy in UpDraw?
		u->rotation.y = GetYaw(dir.x, dir.y);
		PlayAnim(u->frame[BODY_LOWER], 0, t->nframes, true, 1.0f);

		u->cmpos = u->subgoal;
		dir = Vec2i(0,0);	//fixed; no overreach collision now.

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		Log("unitmove u=thatunit dmove<speed? yes");
	}
#endif

		if(u->path.size() >= 2)
		{
			u->path.erase( u->path.begin() );
			u->subgoal = *u->path.begin();
			dir = u->subgoal - u->cmpos;
		}
		else
		{
#ifdef HIERPATH
			//Did we finish the local path? Have tpath?
			if(u->tpath.size() > 0)
			{

				Vec2s tpos = *u->tpath.begin();

#ifdef HIERDEBUG
				Log("tpos pop "<<tpos.x<<","<<tpos.y);
#endif

				unsigned int cmtminx = tpos.x * TILE_SIZE;
				unsigned int cmtminy = tpos.y * TILE_SIZE;
				unsigned int cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
				unsigned int cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

				UType* ut = &g_utype[ u->type ];

				unsigned int ucmminx = u->cmpos.x - ut->size.x/2;
				unsigned int ucmminy = u->cmpos.y - ut->size.x/2;
				unsigned int ucmmaxx = ucmminx + ut->size.x - 1;
				unsigned int ucmmaxy = ucmminy + ut->size.x - 1;

#ifdef HIERDEBUG
				Log("fini local");
				Log("\tucmminx <= cmtmaxx = "<<(ucmminx <= cmtmaxx));
				Log("\tucmminy <= cmtmaxy = "<<(ucmminy <= cmtmaxy));
				Log("\tucmmaxx >= cmtminx = "<<(ucmmaxx >= cmtminx));
				Log("\tucmmaxy >= cmtminy = "<<(ucmmaxy >= cmtminy));
#endif

				if(/* u->cmpos/TILE_SIZE == Vec2i(tpos.x,tpos.y) */
					ucmminx <= cmtmaxx &&
					ucmminy <= cmtmaxy &&
					ucmmaxx >= cmtminx &&
					ucmmaxy >= cmtminy &&
					u->tpath.size() > 1)
				{
					u->path.clear();

					if(*u->tpath.begin() == Vec2s(u->cmpos.x/TILE_SIZE,u->cmpos.y/TILE_SIZE))
						u->tpath.erase( u->tpath.begin() );
					//Advance
				}
				else
				{
					ResetPath(u);

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset path track u 1", "reset path track u 1");
	}
#endif
				}
			}
			//necessary so units can get a real path if
			//this was just a temp partial or random 8 move
			else
#endif
			{
				ResetPath(u);
				//ResetMode(u);	//temp, change it back
#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset path track u 2", "reset path track u 2");
	}
#endif
			}
#if 0
			else
			{
				u->fillcollider();
				UpDraw(u);
				u->drawpos.x = u->cmpos.x;
				u->drawpos.y = u->cmpos.y;
				u->drawpos.y = g_hmap.accheight(u->drawpos.x, u->drawpos.y);
				u->rotation.y = GetYaw(dir.x, dir.y);
				return;
			}
#endif
		}
	}

#if 0
	//the following demonstrates a problem of large distances (on maps 255x255 tile maps definitely).
	//the problem only occurs with jump point search path algo, which can have long distances between individual diagonal or straight path waypoint moves.
	//the problem has been alleviated by making sure that "dir" is within reasonable bounds. this loses unnecessary accuracy, which is not needed by paths
	returned by any path-node/tile-based algorithms that can only have straight or diagonal moves.
	if(dir.x >= isqrt(UINT_MAX))
		InfoMess("a", "dir.x > isqrt(UINT_MAX)");

	if(dir.y >= isqrt(UINT_MAX))
		InfoMess("a", "dir.y > isqrt(UINT_MAX)");
#endif

	if(dir.x != 0 || dir.y != 0)
	//if(mag > t->cmspeed)
	{
		u->rotation.y = GetYaw(dir.x, dir.y);
		PlayAnim(u->frame[BODY_LOWER], 0, t->nframes, true, 1.0f);

#if 0
		//is absolute value of dir.x or dir.y greater than sqrt of int max?
		//in other words, if dir.x or dir.y is squared/multiplied by itself, will it overflow?
		//if so, bring it down. int max is about 2 billion, and 46340.9500011 is the square root of it.
		//dividing by this will be enough, since we know the number is at least this. 46340.9500011 is about 2^15.4999999997.
		//bit shifting to the right is equivalent to dividing by powers of 2.
		//when we bit shift instead of dividing, we have to take into account that the number we bit shift might be negative.
		//so we hide the front bit, bit shift the result, then put back that bit.
		unsigned int intmaxrt = isqrt(INT_MAX);
		if(dir.x & INT_MAX >= intmaxrt ||
			dir.y & INT_MAX >= intmaxrt)
		{
			//dir.x = ((dir.x & INT_MAX) >> 15) | (dir.x & ((unsigned int)INT_MAX+1));
			//dir.y = ((dir.y & INT_MAX) >> 15) | (dir.y & ((unsigned int)INT_MAX+1));

#if 0
			//edit: temporary. until i fix the crashing bug.
			dir.x /= 46341;
			dir.y /= 46341;
#else
			//this is better
			int dirmax = imax(abs(dir.x), abs(dir.y));
			//sign(dir.x);
			//int hmax = dirmax >> 1;	//half of max element
			int qmax = dirmax >> 2;	//quarter of max element
			dir.x /= qmax;
			dir.x /= qmax;
#endif
		}

		//we can still end up overflowing a signed int if the two squares are greater than int max...
		unsigned int sqrx = (unsigned int)dir.x * (unsigned int)dir.x;
		unsigned int sqry = (unsigned int)dir.y * (unsigned int)dir.y;

#if 1
		//the minimum number we need for either of them to overflow int max is INT_MAX/2.
		if(sqrx > INT_MAX/2 || sqry > INT_MAX/2)
		{
			//2,1 will be enough. bit shifting by 2 is division by 4. and bit shift by 1 is division by 2.
			//the squares are effected doubly by anything in dir.x/y because they are the result of the
			//multiplication of that number by itself.
			sqrx >>= 2;
			sqry >>= 2;
			dir.x >>= 1;
			dir.y >>= 1;
		}
		//edit: this is no longer necessary because previous part brings it down enough.
		//edit: or is it?
#endif

		unsigned int insum = sqrx + sqry;

		//if(insum >
		//but let's not worry about that :)

		//now we can safely plug in the formula dir.x^2 + dir.y^2 without overflowing a signed int.
		//int mag = Magnitude(dir);
		int mag = isqrt( insum );
#else
		//actually, instead of all this bit shifting nonsense, why not do this?
		int maxdelta = 1 << 12;
		//get absolute value and see if it exceeds max value
		//we're assuming dir.x can't equal INT_MIN, because that can't give a positive signed int value.
		if(abs(dir.x) >= maxdelta ||
			abs(dir.y) >= maxdelta)
		{
			//pick the max element and divide by it (or something something a bit smaller than it.
			//some multiple smaller so we can still have something when we divide the coordinates by it.)
			int dirmax = imax(abs(dir.x), abs(dir.y));
			//sign(dir.x);
			//int hmax = dirmax >> 1;	//half of max element
			//int qmax = dirmax >> 2;	//quarter of max element
			int dmax = dirmax >> 8;	//max divided by
			dir.x /= dmax;
			dir.y /= dmax;
			//dividing a vector's components by the same number gives a vector pointing in the same direction but smaller.
			//dividing by (max/256) gives us a maximum vector distance of 256 in the case that the line is straight/axial.
		}

		int mag = Magnitude(dir);
#endif

#if 1
		if(mag <= 0)
		{
			mag = 1;
		}
#if 0
		//we need to make sure that it isn't too small, otherwise
		//rounding off errors can make the units visibly move faster.
		//EDIT: maybe that was something else?
		else if(mag < 10)
		{
			dir = dir * 10;
			mag = Magnitude(dir);
		}
#endif

#endif

#if 0
		//one final precaution against overflow of signed int.
		if(mag > 64)
		{
#if 0
			dir = dir * 30 / mag;	//multiplying by 30 before division: danger of overflow again.
			mag = 30;
#else
			int magd32 = mag >> 5;	//mag divided by 32
			dir = dir / magd32;
			mag = magd32;

			//as explained, we don't need much precision in direction, because it is only used for an angle,
			//and tile-based path algo's only generate diagonal or straight moves.
#endif
		}
#endif

#if 1
		Vec2i speeddir = dir * t->cmspeed / mag;
#elif 0
		long long sdirx = dir.x * t->cmspeed / mag;
		long long sdiry = dir.y * t->cmspeed / mag;
		Vec2i speeddir(sdirx,sdiry);
#endif

		//Can result in overshooting the target otherwise,
		//and being a few centimeters in collision with building,
		//leading to forever being unable to follow the path.
		if(t->cmspeed >= mag)
			speeddir = dir;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("u cmpos="<<u->cmpos.x<<","<<u->cmpos.y<<" mag="<<mag<<" speeddir="<<speeddir.x<<","<<speeddir.y<<" t->cmspeed="<<t->cmspeed);
		//InfoMess("uc", "uc");
	}
#endif

#if 0
		if(Magnitude(scaleddir) > TILE_SIZE)
		{
			InfoMess("j", "Jump");
		}
#endif

		//if(speeddir == Vec2i(0,0))
		//	InfoMess("z","z");

		u->cmpos = u->cmpos + speeddir;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		Log("the 13th unit:");
		Log("newpos="<<u->cmpos.x<<","<<u->cmpos.y);
		//InfoMess("uc", "uc");
	}
#endif
	}

	//corpc fix, no more sublevel braces; jumping to subgoal and dir!=0 are both moves that can have collisions.
#if 1
	if(UnitCollides(u, u->cmpos, u->type))
#else
	if(Trace(u->type, u->mode, u->prevpos, u->cmpos, u, NULL, NULL) != COLLIDER_NONE)
#endif
	{
		bool ar = CheckIfArrived(u);


#ifdef RANDOM8DEBUG
		if(u - g_unit == thatunit)
		{
			Log("\tUnitCollides=true ResetPath u=thatunit");
		}
#endif

#ifdef HIERDEBUG
		//if(pathnum == 73)
		if(u - g_unit == 19)
		{
			Log("the 13th unit:");
			Log("uc oihoih");
			InfoMess("uc", "uc");
		}
#endif

		u->collided = true;
		u->cmpos = u->prevpos;
		u->path.clear();
		u->tpath.clear();
		u->subgoal = u->cmpos;
		u->fillcollider();
		AddVis(u);
		Explore(u);
		UpDraw(u);
		ResetPath(u);


#ifdef TSDEBUG
		if(u == tracku)
		{
			char msg[128];
			sprintf(msg, "reset path track u mu arr=%d", (int)ar);
			InfoMess(msg, msg);
		}
#endif

		if(ar)
			OnArrived(u);
#if 0
		else
		{
			//Sometimes, you get a row of trucks along a building
			//and none of them can start moving except that ones
			//at the ends.
			if(u->type == UNIT_TRUCK &&
				u->cargoamt <= 0 &&
				u->driver >= 0 &&
				g_simframe - u->lastpath > 0)
				ResetMode(u);
		}
#endif

		//goto endcheck;

		return;
	}
#if 0
	u->collided = false;
#endif

	//if(UnitCollides(u, u->cmpos, u->type))
	//	u->collided = true;

	if(!u->hidden())
	{
		u->fillcollider();
		AddVis(u);
		Explore(u);
		UpDraw(u);
	}

	u->drawpos.x = u->cmpos.x;
	u->drawpos.y = u->cmpos.y;
	u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);

#if 0
endcheck:

	if(abs(u->cmpos.x - u->prevpos.x) > TILE_SIZE ||
		abs(u->cmpos.y - u->prevpos.y) > TILE_SIZE)
		InfoMess("skip", "skip");
#endif
}

bool CheckIfArrived(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int ui = (int)(u-g_unit);

	int ucmminx = u->cmpos.x - ut->size.x/2;
	int ucmminy = u->cmpos.y - ut->size.x/2;
	int ucmmaxx = ucmminx + ut->size.x - 1;
	int ucmmaxy = ucmminy + ut->size.x - 1;

	//switched from fine-grained collisions to pathnode check
#if 1
	Building* b;
	BlType* bt;
	CdType* ct;
	CdTile* ctile;
	Unit* u2;
	UType* u2t;

	int btminx;
	int btminz;
	int btmaxx;
	int btmaxz;
	int bcmminx;
	int bcmminy;
	int bcmmaxx;
	int bcmmaxy;
	int ccmposx;
	int ccmposz;
	int ccmminx;
	int ccmminy;
	int ccmmaxx;
	int ccmmaxy;
	int u2cmminx;
	int u2cmminy;
	int u2cmmaxx;
	int u2cmmaxy;

	switch(u->mode)
	{
#if 1
	case UMODE_GOBLJOB:
	case UMODE_GOCSTJOB:
	case UMODE_GOSHOP:
	case UMODE_GOREST:
	case UMODE_GODEMB:
		b = &g_building[u->target];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminz = b->tpos.y - bt->width.y/2;
		btmaxx = btminx + bt->width.x - 1;
		btmaxz = btminz + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminz * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
		{
			//char msg[1280];
			//sprintf(msg, "arr ui=%d u(%d,%d) b(%d,%d) %d<=%d && %d<=%d && %d<=%d && %d<=%d", ui, u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE, b->tpos.x, b->tpos.y, ucmminx, bcmmaxx, ucmminy, bcmmaxy, bcmminx, ucmmaxx, bcmminy, ucmmaxy);
			//RichText at("arr");
			//RichText at(msg);
			//AddChat(&at);
			return true;
		}
		break;
#endif
	case UMODE_GOCDJOB:
	case UMODE_GODEMCD:
		ct = &g_cdtype[u->cdtype];
		ctile = GetCd(u->cdtype, u->target, u->target2, false);

		ccmposx = u->target*TILE_SIZE + ct->physoff.x;
		ccmposz = u->target2*TILE_SIZE + ct->physoff.y;

		ccmminx = ccmposx - TILE_SIZE/2;
		ccmminy = ccmposz - TILE_SIZE/2;
		ccmmaxx = ccmminx + TILE_SIZE;
		ccmmaxy = ccmminy + TILE_SIZE;

		if(ucmminx <= ccmmaxx && ucmminy <= ccmmaxy && ccmminx <= ucmmaxx && ccmminy <= ucmmaxy)
			return true;
		break;
#if 1
	case UMODE_GOSUP:
		b = &g_building[u->supplier];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminz = b->tpos.y - bt->width.y/2;
		btmaxx = btminx + bt->width.x - 1;
		btmaxz = btminz + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminz * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
			return true;
		break;

	case UMODE_GOREFUEL:
		b = &g_building[u->fuelstation];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminz = b->tpos.y - bt->width.y/2;
		btmaxx = btminx + bt->width.x - 1;
		btmaxz = btminz + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminz * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
			return true;
		break;

	case UMODE_GODRIVE:
		u2 = &g_unit[u->target];
		u2t = &g_utype[u2->type];

		u2cmminx = u2->cmpos.x - u2t->size.x/2;
		u2cmminy = u2->cmpos.y - u2t->size.x/2;
		u2cmmaxx = u2cmminx + u2t->size.x - 1;
		u2cmmaxy = u2cmminy + u2t->size.x - 1;

		//InfoMess("dr","d?");

		if(ucmminx <= u2cmmaxx && ucmminy <= u2cmmaxy && u2cmminx <= ucmmaxx && u2cmminy <= ucmmaxy)
			return true;
		break;
#endif
	default:
		break;
	};

#else

	const int unminx = ucmminx/PATHNODE_SIZE;
	const int unminz = ucmminy/PATHNODE_SIZE;
	const int unmaxx = ucmmaxx/PATHNODE_SIZE;
	const int unmaxz = ucmmaxy/PATHNODE_SIZE;

	for(int z=unminz; z<=unmaxz; z++)
		for(int x=unminx; x<=unmaxx; x++)
		{
			ColliderTile* cell = ColliderAt(x, y);

			if(cell->building >= 0)
			{
				switch(u->mode)
				{
					case UMODE_GOBLJOB:
					case UMODE_GOCSTJOB:
					case UMODE_GOSHOP:
					case UMODE_GOREST:
					case UMODE_GODEMB:
						if(u->target == cell->building)
							return true;
						break;
					case UMODE_GOSUP:
						if(u->supplier == cell->building)
							return true;
						break;
					case UMODE_GOREFUEL:
						if(u->fuelstation == cell->building)
							return true;
						break;
					default:
						break;
				}
			}


			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u2 = &g_unit[uindex];

				if(!u2->hidden())
				{
					switch(u->mode)
					{
						case UMODE_GODRIVE:
							if(u->target == uindex)
								return true;
							break;
						default:
							break;
					}
				}
			}
		}

#endif

	return false;
}

//arrived at transport vehicle
void ArAtTra(Unit* u)
{
	Unit* u2 = &g_unit[u->target];
	//u->freecollider();
	u->mode = UMODE_DRIVE;
	u2->driver = u - g_unit;
}

void OnArrived(Unit* u)
{
	u->pathdelay = 0;
	
	if(u->mode != UMODE_NONE)
		u->cyframes = 1;	//TODO check if it doesn't interfere with anything. needed to charge minimum of one man-hour.

	Player* utilpy = NULL;
	Building* b = NULL;
	CdTile* ctile = NULL;
	Unit* u2 = NULL;

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset goal track u 2", "reset goal track u 2");
	}
#endif

	switch(u->mode)
	{
	case UMODE_GOBLJOB:
		u->mode = UMODE_BLJOB;
		u->freecollider();
		b = &g_building[u->target];
		b->worker.push_back(u-g_unit);
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOCSTJOB:
		u->mode = UMODE_CSTJOB;
		u->freecollider();
		b = &g_building[u->target];
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOCDJOB:
		u->mode = UMODE_CDJOB;
		u->freecollider();
		ctile = GetCd(u->cdtype, u->target, u->target2, false);
		utilpy = &g_player[ctile->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOSHOP:
		u->mode = UMODE_SHOPPING;
		u->freecollider();
		b = &g_building[u->target];
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOREST:
		u->mode = UMODE_RESTING;
		u->freecollider();
		b = &g_building[u->home];
		utilpy = &g_player[b->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GODRIVE:
		ArAtTra(u);
		u->freecollider();
		u2 = &g_unit[u->target];
		utilpy = &g_player[u2->owner];
		utilpy->util += u->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GODEMB:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_ATDEMB;
		ResetGoal(u);
		break;
	case UMODE_GODEMCD:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_ATDEMCD;
		ResetGoal(u);
		break;
	case UMODE_GOSUP:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_ATSUP;
		ResetGoal(u);
		break;
	case UMODE_GOREFUEL:
		if(u->driver >= 0)
			Disembark(&g_unit[u->driver]);
		u->driver = -1;
		u->mode = UMODE_REFUELING;
		ResetGoal(u);
		break;
	default:
		break;
	};

	//if(type == TRUCK && UnitSelected(this))
	//	RedoLeftPanel();

	///RecheckSelection();
}
