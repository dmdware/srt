


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






#include "manuf.h"
#include "building.h"
#include "bltype.h"
#include "unit.h"
#include "utype.h"
#include "player.h"
#include "../net/client.h"

void OrderMan(int utype, int bi, int player)
{
	ManufJob mj;
	Building* b = &g_building[bi];
	mj.owner = player;
	mj.utype = utype;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();
}
