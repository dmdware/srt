


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




#ifndef TILE_H
#define TILE_H

#include "sprite.h"
#include "../phys/collidable.h"
#include "../math/vec2i.h"

#include "../platform.h"
#include "../render/vertexarray.h"


//incline
class InType
{
public:
	unsigned int sprite;
};

#define IN_0000		0
#define IN_0001		1
#define IN_0010		2
#define IN_0011		3
#define IN_0100		4
#define IN_0101		5
#define IN_0110		6
#define IN_0111		7
#define IN_1000		8
#define IN_1001		9
#define IN_1010		10
#define IN_1011		11
#define IN_1100		12
#define IN_1101		13
#define IN_1110		14
#define INCLINES	15

extern VertexArray g_tileva[INCLINES];
extern int g_currincline;
extern bool g_cornerinc[INCLINES][4];
//extern int g_tilesize;

//#define g_tilesize		(10*100)	//10 meters = 1,000 centimeters
//extern int g_tilesize;
//#define TILE_RISE		(g_tilesize/3)
#define TILE_DIAG		(sqrt(TILE_SIZE*TILE_SIZE*2))
//#define TILE_RISE		(tan(DEGTORAD(30))*TILE_DIAG/2)
#define TILE_RISE		(tan(DEGTORAD(30))*TILE_DIAG/4)

#define TEX_DIFF		0
#define TEX_SPEC		1
#define TEX_NORM		2
#define TEX_TEAM		3
#define TEX_TYPES		4

//extern unsigned int g_tiletexs[TEX_TYPES];

void DrawTile();
void MakeTiles();

extern const char* INCLINENAME[INCLINES];

extern InType g_intype[INCLINES];

class Tile
{
public:
	unsigned char incltype;
	unsigned char elev;
};

void DefTl(const char* sprel);
Tile &SurfTile(int tx, int ty);

#endif
