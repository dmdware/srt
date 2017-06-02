


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




#include "../texture.h"
#include "tile.h"
#include "../utils.h"
#include "../sim/map.h"
#include "../render/heightmap.h"

const char* INCLINENAME[] =
{
	"0000",
	"0001",
	"0010",
	"0011",
	"0100",
	"0101",
	"0110",
	"0111",
	"1000",
	"1001",
	"1010",
	"1011",
	"1100",
	"1101",
	"1110"
};

InType g_intype[INCLINES];

void DefTl(const char* sprel)
{
	for(int tti=0; tti<INCLINES; tti++)
	{
		InType* t = &g_intype[tti];
		//QueueTexture(&t->sprite.texindex, texpath, true);
		//CreateTex(t->sprite.texindex, texpath, true, false);
		char specific[DMD_MAX_PATH+1];
		sprintf(specific, "%s_inc%s", sprel, INCLINENAME[tti]);
		QueueSprite(specific, &t->sprite, false, true);
	}
#if 0
	t->sprite.offset[0] = spriteoffset.x;
	t->sprite.offset[1] = spriteoffset.y;
	t->sprite.offset[2] = t->sprite.offset[0] + spritesz.x;
	t->sprite.offset[3] = t->sprite.offset[1] + spritesz.y;
#endif
}

Tile &SurfTile(int tx, int ty)
{
	return g_hmap.m_surftile[ ty * g_mapsz.x + tx ];
}
