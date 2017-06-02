


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



#ifndef PATHNODE_H
#define PATHNODE_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"
#include "../math/fixmath.h"
#include "../render/heightmap.h"

//must divide tiles evenly into integers,
//so they line up.
//#define PATHNODE_SIZE	(TILE_SIZE/8)
#define PATHNODE_SIZE	(TILE_SIZE/20)
#define PATHNODE_DIAG	(isqrt(PATHNODE_SIZE*PATHNODE_SIZE*2))

//straight dirs
#define SDIR_E	0
#define SDIR_W	1
#define SDIR_S	2
#define SDIR_N	3
#define SDIRS	4

// Offsets for straights moves
const Vec2s STRAIGHTOFFSETS[4] =
{
	Vec2s(1, 0), //E
	Vec2s(-1, 0), //W
	Vec2s(0, 1), //S
	Vec2s(0, -1) //N
};

// Offsets for diagonal moves
const Vec2s DIAGONALOFFSETS[4] =
{
	Vec2s(-1, -1), //NW
	Vec2s(1, -1), //NE
	Vec2s(-1, 1), //SW
	Vec2s(1, 1) //SE
};

//TODO check for memleak because app is taking too long to exit.

#define DIR_S       0
#define DIR_SW      1
#define DIR_W       2
#define DIR_NW      3
#define DIR_N       4
#define DIR_NE      5
#define DIR_E       6
#define DIR_SE      7
#define DIRS        8

const Vec2s OFFSETS[DIRS] =
{
	Vec2s(0, 1), //S
	Vec2s(-1, 1), //SW
	Vec2s(-1, 0), //W
	Vec2s(-1, -1), //NW
	Vec2s(0, -1), //N
	Vec2s(1, -1), //NE
	Vec2s(1, 0), //E
	Vec2s(1, 1) //SE
};

//vertical dirs
#define VDIR_UP		0
#define VDIR_MID	1
#define VDIR_DOWN	2
#define VDIRS		3

const signed char VOFFSETS[VDIRS] =
{
	1, 0, -1
};

#if 0
//big error: pathnode score and G factors count pathnodes, not centimeters
const int STEPDIST[DIRS] =
{
	(int)PATHNODE_DIAG, //NW
	(int)PATHNODE_SIZE, //N
	(int)PATHNODE_DIAG, //NE
	(int)PATHNODE_SIZE, //E
	(int)PATHNODE_DIAG, //SE
	(int)PATHNODE_SIZE, //S
	(int)PATHNODE_DIAG, //SW
	(int)PATHNODE_SIZE //W
};
#else
//now totalD and stepD should be multiplied by two
//(each PATHNODE_SIZE will count as two).
//multiply by 2 to give granularity
//(needed for diagonal moves).
//e.g., int H = Manhattan(Vec2i(endx-nx,endz-ny)) << 1;
//must be in the same order as OFFSETS.
//corpc fix
const unsigned char STEPDIST[DIRS] =
{
	(unsigned char)2, //S
	(unsigned char)3, //SW
	(unsigned char)2, //W
	(unsigned char)3, //NW
	(unsigned char)2, //N
	(unsigned char)3, //NE
	(unsigned char)2, //E
	(unsigned char)3 //SE
};
#endif

// byte-align structures
#pragma pack(push, 1)

class PathNode
{
public:
	unsigned short score;
	//short nx;
	//short ny;
	unsigned short totalD;
	//unsigned char expansion;
	PathNode* previous;
	//bool tried;
	bool opened;
	bool closed;
	PathNode()
	{
		//tried = false;
		previous = NULL;
		opened = false;
		closed = false;
	};
	PathNode(int startx, int startz, int endx, int endz, int nx, int ny, PathNode* prev, int totalD, int stepD);
	//PathNode(int startx, int startz, int endx, int endz, int nx, int ny, PathNode* prev, int totalD, int stepD, unsigned char expan);
};

#pragma pack(pop)

class BinHeap;
class PathJob;

extern Vec2i g_pathdim;
extern PathNode* g_pathnode;
extern BinHeap g_openlist;

bool CompareNodes(void* a, void* b);
Vec2i PathNodePos(PathNode* node);
PathNode* PathNodeAt(int nx, int ny);
int PathNodeIndex(int nx, int ny);
bool AtGoal(PathJob* pj, PathNode* node);
void SnapToNode(PathJob* pj);
void ResetPathNodes();

#endif
