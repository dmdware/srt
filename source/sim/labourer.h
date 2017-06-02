


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




#ifndef LABOURER_H
#define LABOURER_H

#include "unit.h"

#define LABSND_WORK		0
#define LABSND_SHOP		1
#define LABSND_REST		2
#define LAB_SOUNDS		3

extern short g_labsnd[LAB_SOUNDS];


void UpdLab(Unit* u);
void UpdLab2(Unit* u);
void Evict(Unit* u, bool silent);
void Disembark(Unit* op);

#endif
