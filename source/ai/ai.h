


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



#ifndef AI_H
#define AI_H

#include "../sim/simdef.h"

class Player;
class Building;

#define AI_FRAMES	(CYCLE_FRAMES/30+1)
//#define AI_FRAMES	CYCLE_FRAMES
//#define AI_FRAMES	1

void UpdAI();
void UpdAI(Player* p);
void AdjProd(Player* p);
void AdjPrWg(Player* p, Building* b);
void BuyProps(Player* p);
void AIBuild(Player* p);
void PlotCd(int pi, int ctype, Vec2i from, Vec2i to);
void AIManuf(Player* p);
void ConnectCd(Player* p, int ctype, Vec2i tplace);

#endif