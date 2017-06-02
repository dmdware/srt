


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





#ifndef FILLBODIES_H
#define FILLBODIES_H

#include "pathnode.h"

void FillBodies();

class TileRegs
{
public:

	class TilePass
	{
	public:
		unsigned short from;
		unsigned short to;
	};

	std::list<TilePass> tregs[SDIRS];

	void add(unsigned char sdir, unsigned short from, unsigned short to);
};

extern TileRegs* g_tilepass;
extern unsigned char* g_tregs;

// byte-align structures
#pragma pack(push, 1)

class TileNode
{
public:
	unsigned short score;
	//short nx;
	//short ny;
	unsigned short totalD;
	//unsigned char expansion;
	TileNode* previous;
	//bool tried;
	std::list<unsigned short> tregs;
	bool opened;
	bool closed;
	//unsigned char jams[SDIRS];
	/*
	"jams" now doesn't indicate the number of jams on the tile 
	(as an indicator of severity), but the inverse of the minimum
	size unit that can't pass. So the smaller the unit that can't
	pass, the greater the severity, and the greater the "jams" value.
	255 is thus the max unit width.
	*/
	unsigned char jams;
	TileNode()
	{
		//tried = false;
		previous = NULL;
		opened = false;
		closed = false;
		jams = 0;
	};
	TileNode(int startx, int startz, int endx, int endz, int nx, int ny, TileNode* prev, int totalD, int stepD, unsigned short treg);
	//PathNode(int startx, int startz, int endx, int endz, int nx, int ny, PathNode* prev, int totalD, int stepD, unsigned char expan);
};

#pragma pack(pop)

extern TileNode* g_tilenode;

#endif
