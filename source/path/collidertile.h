


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



#ifndef PATHCELL_H
#define PATHCELL_H

#include "../platform.h"
#include "../math/vec2i.h"


//#define FLAG_HASROAD	4
#define FLAG_HASLAND	1
#define FLAG_ABRUPT		2

#define LARGEST_UNIT_NODES		4
#define MAX_COLLIDER_UNITS		4

//Enable this option to use map tile-sized collider tiles,
//instead of pathnode-sized.
//#define TILESIZECOLLIDER

// byte-align structures
#pragma pack(push, 1)
class ColliderTile
{
public:
	//bool hasroad;
	//bool hasland;
	//bool haswater;
	//bool abrupt;	//abrupt incline?
	unsigned char flags;
	short building;
#ifdef TILESIZECOLLIDER
	std::list<short> units;
	std::list<unsigned short> foliages;
#else
	short units[MAX_COLLIDER_UNITS];
	unsigned short foliage;
#endif

	ColliderTile();
};
#pragma pack(pop)

extern ColliderTile *g_collidertile;

class Unit;
class Building;
class PathJob;

ColliderTile* ColliderAt(int nx, int ny);
Vec2i PathNodePos(int cmposx, int cmposy);
void FreePathGrid();
void AllocPathGrid(int cmwx, int cmwz);
void FillColliderGrid();
bool Standable(const PathJob* pj, const int nx, const int ny);
bool Standable2(const PathJob* pj, int cmposx, int cmposy);
bool TileStandable(const PathJob* pj, const int nx, const int ny);

#endif
