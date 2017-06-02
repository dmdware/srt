


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



#ifndef RECONSTRUCTPATH_H
#define RECONSTRUCTPATH_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "pathjob.h"

class PathNode;

void ReconstructPath(std::list<Vec2i> &path, PathNode* bestS, Vec2i &subgoal, int cmgoalx, int cmgoalz);
void ReconstructPathJPS(std::list<Vec2i> &path, PathNode* bestS, Vec2i &subgoal, int cmgoalx, int cmgoalz);
void ReconstructPath(PathJob* pj, PathNode* endnode);

#endif
