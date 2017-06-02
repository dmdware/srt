


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




#include "../window.h"
#include "map.h"
#include "../utils.h"
#include "../math/vec3i.h"

Vec2i g_scroll(0,100);
Vec3i g_mouse3d;
//unsigned char* g_hmap = NULL;
Tile* g_surftile = NULL;

void ScrollTo(int x, int y)
{
	g_scroll.x = x;
	g_scroll.y = y;
}
