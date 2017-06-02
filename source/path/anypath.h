


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



#ifndef ANYPATH_H
#define ANYPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../platform.h"

class Unit;
class Building;
class PathJob;
class PathNode;

bool AnyPath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, int cdtype,
                 Unit* thisu, Unit* ignoreu, Building* ignoreb,
                 int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
				 int nminx, int nminy, int nmaxx, int nmaxy);

void Expand_AP(PathJob* pj, PathNode* node);

#endif
