


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






#ifndef TRUCK_H
#define TRUCK_H

class Unit;

void UpdTruck(Unit* u);

#define TRSND_NEWJOB	0
#define TRSND_DONEJOB	1
#define TRSND_WORK		2
#define TR_SOUNDS		3

extern short g_trsnd[TR_SOUNDS];

#endif