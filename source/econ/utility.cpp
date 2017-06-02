


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





#include "utility.h"

// food/housing/physical res utility evaluation
int PhUtil(int price, int cmdist)
{
	if(price <= 0)
		return MAX_UTIL;

	if(cmdist <= 0)
		return MAX_UTIL;

	return MAX_UTIL / price / cmdist;
}

// electricity utility evaluation
int GlUtil(int price)
{
	if(price <= 0)
		return MAX_UTIL;

	return MAX_UTIL / price;
}

// inverse phys utility - solve for distance based on utility and price
int InvPhUtilD(int util, int price)
{
	//util = 100000000 / price / cmdist;
	//util / 100000000 = 1 / price / cmdist;
	//util * price / 100000000 = 1 / cmdist;
	//100000000 / (util * price) = cmdist;

	if(util <= 0)
		return MAX_UTIL;

	if(price <= 0)
		return MAX_UTIL;

	return MAX_UTIL / (util * price);
}

// inverse phys utility - solve for price based on utility and distance
int InvPhUtilP(int util, int cmdist)
{
	if(util <= 0)
		return MAX_UTIL;

	if(cmdist <= 0)
		return MAX_UTIL;

	return MAX_UTIL / (util * cmdist);
}

int InvGlUtilP(int util)
{
	if(util <= 0)
		return MAX_UTIL;

	return MAX_UTIL / util;
}

int JobUtil(int wage, int cmdist, int workdelay)
{
	if(wage <= 0)
		return 0;

	if(cmdist <= 0)
		return MAX_UTIL;

	if(workdelay <= 0)
		return MAX_UTIL;

	return MAX_UTIL / cmdist / workdelay * wage;
}