


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





#ifndef DRAWSORT_H
#define DRAWSORT_H

#include "../platform.h"
#include "depthable.h"

extern std::list<Depthable> g_drawlist;
extern std::list<Depthable*> g_subdrawq;
bool CompareDepth(const Depthable* a, const Depthable* b);
void DrawSort(std::list<Depthable*>& drawlist, std::list<Depthable*>& drawqueue);
void DrawSort2(std::list<Depthable*>& drawlist, std::list<Depthable*>& drawqueue);

#endif
