


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
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../bsp/trace.h"
#include "../algo/binheap.h"
#include "jpspath.h"
#include "pathjob.h"
#include "reconstructpath.h"
#include "../gui/layouts/chattext.h"

void ReconstructPath(PathJob* pj, PathNode* endnode)
{
	pj->path->clear();

#if 1
	// Reconstruct the path, following the path steps
	for(PathNode* n = endnode; n; n = n->previous)
	{
		Vec2i npos = PathNodePos(n);

#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		Log("Reconstruct "<<npos.x<<","<<npos.y<<" pjtype="<<(int)pj->pjtype);
		
	}
#endif

		Vec2i cmpos( npos.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );
		pj->path->push_front(cmpos);
	}
#endif


#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		Log("Reconstruct start "<<(pj->cmstartx/PATHNODE_SIZE)<<","<<(pj->cmstarty/PATHNODE_SIZE)<<" pjtype="<<(int)pj->pjtype);
		
	}
#endif

#if 1	//necessary for exact point
	//pj->path->push_back(Vec2i(pj->goalx, pj->goaly)*PATHNODE_SIZE + PATHNODE_SIZE/2);
	if(pj->capend)
		pj->path->push_back(pj->cmgoal);
#endif

	if(pj->path->size() > 0)
		*pj->subgoal = *pj->path->begin();

#ifdef TSDEBUG
	if(&g_unit[pj->thisu] == tracku && tracku)
	{
		auto lastp = pj->path->begin();
		int i=0;
		for(auto p=pj->path->begin(); p!=pj->path->end() && i<3; i++, p++)
		{
			char msg[128];

			sprintf(msg, "path[%d] n(%d,%d) cm(%d,%d) d(%d,%d)", i, (int)p->x/PATHNODE_SIZE, (int)p->y/PATHNODE_SIZE,
				(int)p->x, (int)p->y, (int)(p->x-lastp->x), (int)(p->y-lastp->y));

			InfoMess(msg,msg);

			lastp = p;
		}
	}
#endif
}
