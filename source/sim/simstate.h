


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






#ifndef SIMSTATE_H
#define SIMSTATE_H

#include "unit.h"

//Everything inside this class must:
//1. Change completely deterministically
//2. Be synchronized between clients

class SimState
{
public:
	unsigned int simframe;
	Unit unit[UNITS];
	//TO DO
};

#endif