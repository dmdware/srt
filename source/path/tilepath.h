


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





#ifndef TILEPATH_H
#define TILEPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2s.h"
#include "../platform.h"

class PathJob;
class PathNode;
class Unit;
class Building;

extern unsigned int pathnum;

#define MAX_JAM_VAL		255	//max of unsigned char
//#define MAX_JAM_VAL					1	//max of unsigned char
//#define MAX_JAM_VAL_DONTDRIVE		2	//max of unsigned char

//#define UPD_JAMS_DELAY		(SIM_FRAME_RATE*60)	//minute
//#define UPD_JAMS_DELAY		(SIM_FRAME_RATE*10)	//10 seconds
#define UPD_JAMS_DELAY			(SIM_FRAME_RATE)	//1 second

//can't be greater than (or equal to?): 255 (MAX_JAM_VAL) - biggest unit size = 5
//#define	HUMANOID_JAM	4
#define	HUMANOID_JAM	1

void TilePath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, signed char cdtype,
               std::list<Vec2s> *tpath, Unit* thisu, Unit* ignoreu, Building* ignoreb,
               int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
               int maxsearch, bool ignorejams=false);

void Expand_T(PathJob* pj, PathNode* node);

void UpdJams();

/*
hierarchical works well when PATHNODE_SIZE is TILE_SIZE/20
(smaller units), but causes clumps around stores when TILE_SIZE/8.
TILE_SIZE/8 has less total pathnodes, so is an easier load,
possibly not as good as hierarchical though.
edit: HIERPATH causes clumps around stores in both cases.
also, check the "causes clumps" comment in MoveUnit();
*/
//#define HIERPATH	//hierarchical pathfinding?

#endif