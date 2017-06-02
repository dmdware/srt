


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




#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../math/vec2uc.h"
#include "../texture.h"
#include "vertexarray.h"
#include "tile.h"
#include "../math/isomath.h"

#define MAPMINZ				(0)
#define MAPMAXZ				(g_mapsz.y*TILE_SIZE)

#define MAX_MAP				63
//#define MAX_MAP				25
//#define MAX_MAP				14

class Vec2f;
class Vec3f;
class Shader;
class Matrix;
class Plane3f;

extern Vec2i g_mapview[2];

/*
Number of tiles, not heightpoints/corners.
Number of height points/corners is +1.
*/
extern Vec2uc g_mapsz;

class Heightmap
{
public:
	unsigned char *m_heightpoints;
	Vec3f *m_3dverts;
	Vec2f *m_texcoords0;
	Vec3f *m_normals;
	int *m_countryowner;
	bool *m_triconfig;
	Plane3f *m_tridivider;
	Tile *m_surftile;

	void alloc(int wx, int wy);
	void remesh();
	void draw();

	inline unsigned char getheight(int tx, int ty)
	{
		return m_heightpoints[ (ty)*(g_mapsz.x+1) + tx ];
	}

	float accheight(int x, int y);
	void adjheight(int x, int y, signed char change);
	void setheight(int x, int y, unsigned char height);
	void destroy();
	Vec3f getnormal(int x, int y);
    void lowereven();
    void highereven();

    Heightmap()
    {
        m_heightpoints = NULL;
        m_3dverts = NULL;
        m_texcoords0 = NULL;
        m_normals = NULL;
        m_countryowner = NULL;
        m_triconfig = NULL;
        m_tridivider = NULL;
        m_surftile = NULL;
    }
	~Heightmap()
	{
		destroy();
	}
};

extern Heightmap g_hmap;

void AllocGrid(int wx, int wy);
void FreeGrid();

#endif
