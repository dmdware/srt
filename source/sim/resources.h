


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




#ifndef RESOURCES_H
#define RESOURCES_H

#include "../platform.h"

class Resource
{
public:
	char icon;
	bool physical;
	bool capacity;
	bool global;
	std::string name;	//TODO change to char[]?
	float rgba[4];
	std::string depositn;	//TODO change to char[]?
	int conduit;
	std::string unit;	//measuring unit
};

//conduit
#define CD_NONE		-1
#define CD_ROAD		0
#define CD_POWL		1
#define	CD_CRPIPE		2

#define RES_NONE			-1
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_CHEMICALS		5
#define RES_IRONORE			6
#define RES_METAL			7
#define RES_STONE			8
#define RES_CEMENT			9
#define RES_COAL			10
#define RES_URANIUM			11
#define RES_PRODUCTION		12
#define RES_CRUDEOIL		13
#define RES_WSFUEL			14
#define RES_RETFUEL			15
#define RES_ENERGY			16
#define RESOURCES			17
//#define RES_ELECTRONICS		6
extern Resource g_resource[RESOURCES];

//TODO remove electronics plant to avoid crash

class Basket
{
public:
	int r[RESOURCES];

	int& operator[](const int i)
	{
		return r[i];
	}
};

class Bundle
{
public:
	unsigned char res;
	int amt;
};

//capacity supply (e.g. electricity, water pressure)
class CapSup
{
public:
	unsigned char rtype;
	int amt;
	int src;
	int dst;
};

void DefR(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a, int conduit, const char* unit);
void Zero(int *b);
bool ResB(int building, int res);
bool TrySub(const int* cost, int* universal, int* stock, int* local, int* netch, int* insufres);

#endif
