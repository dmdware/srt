


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







#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "simdef.h"

class Unit;
class Building;

#if 0
class TransportJob
{
public:
	int targtype;
	int target;
	int target2;
	std::list<Vec2i> path;
	int pathlen;
	int restype;
	int resamt;
	int fuelcost;
	int driverwagepaid;
	int netprofit;	// truck profit
	int transporter;
	int totalcosttoclient;	// transport fee + resource cost
	int clientcharge;	// transport fee
	int supplier;
    
	TransportJob()
	{
		pathlen = 0;
		totalcosttoclient = 0;
		clientcharge = 0;
		driverwagepaid = 0;
		netprofit = 0;
		fuelcost = 0;
		resamt = 0;
	}
};
#else

//job opportunity
class TransportJob
{
public:
	int jobutil;
	int jobtype;
	int target;
	int target2;
	//float bestDistWage = -1;
	//float distWage;
	//bool fullquota;
	int ctype;	//conduit type
	Vec2i goal;
	int targtype;
	Unit* thisu;
	Unit* ignoreu;
	Building* ignoreb;
	int suputil;
	int supbi;
	Vec2i supcmpos;
	short truckui;
	Building* potsupb;
};

#endif

class ResSource
{
public:
	int supplier;
	int amt;
};

//extern vector<int> g_freetrucks;

////#define TRANSPORT_DEBUG	//chat output debug messages

void ManageTrips();

#endif
