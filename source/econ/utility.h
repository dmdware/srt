


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





#ifndef UTILITY_H
#define UTILITY_H

//#define MAX_UTIL	1000000000	//possible overflow?
//make sure this value is bigger than any combined obtainable revenue of building from labourer spending
//edit: and multiplied by distance?
#define MAX_UTIL	200000000	//200mill
//#define MAX_UTIL	2000000

int PhUtil(int price, int cmdist);
int GlUtil(int price);
int InvPhUtilD(int util, int price);
int InvPhUtilP(int util, int cmdist);
int InvGlUtilP(int util);
int JobUtil(int wage, int cmdist, int workdelay);

#endif
