


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






#ifndef JOB_H
#define JOB_H

#include "labourer.h"

class Unit;
class Building;

//job opportunity
class JobOpp
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
	Unit* ignoreu;
	Building* ignoreb;
};

bool FindJob(Unit* u);
void NewJob(int jobtype, int target, int target2, int cdtype);


#endif