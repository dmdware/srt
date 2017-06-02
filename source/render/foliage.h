


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




#ifndef FOLIAGE_H
#define FOLIAGE_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2s.h"
#include "../math/vec3i.h"
#include "../math/matrix.h"
#include "vertexarray.h"
#include "sprite.h"
#include "depthable.h"

class FlType
{
public:
	char name[64];
	Vec2s size;
	unsigned int sprite;
};


#define FL_SPRUCE1			0
#define FL_SPRUCE2			1
#define FL_SPRUCE3			2
#define FL_TYPES			3

extern FlType g_fltype[FL_TYPES];

// byte-align structures
#pragma pack(push, 1)
class Foliage
{
public:
	Depthable* depth;
	bool on;
	unsigned char type;
	Vec2i cmpos;
	Vec3f drawpos;
	float yaw;
	unsigned char lastdraw;	//used for preventing repeats

	Foliage();
	void fillcollider();
	void freecollider();
	void destroy();
};
#pragma pack(pop)

//#define FOLIAGES	128
//#define FOLIAGES	1024
//#define FOLIAGES	2048
//#define FOLIAGES	6000
//#define FOLIAGES	10000
//#define FOLIAGES	30000
#define FOLIAGES	60000	//ushort limit
//#define FOLIAGES	240000

extern Foliage g_foliage[FOLIAGES];

void DefF(int type, const char* sprel, Vec3f scale, Vec3f translate, Vec2s size);
bool PlaceFol(int type, Vec3i cmpos);
void DrawFol(Foliage* f, float rendz);
void ClearFol(int cmminx, int cmminy, int cmmaxx, int cmmaxy);
void FreeFol();
void FillForest(unsigned int r);
#endif
