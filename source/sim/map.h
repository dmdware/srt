


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




#ifndef MAP_H
#define MAP_H

#include "../math/vec2i.h"
#include "../math/vec3i.h"
#include "../math/vec2f.h"
#include "../render/tile.h"
#include "../math/fixmath.h"

extern Vec2i g_scroll;
extern Vec3i g_mouse3d;

void ScrollTo(int x, int y);

//extern unsigned char* g_hmap;
extern Tile* g_surftile;

#endif
