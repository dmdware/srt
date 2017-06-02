


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






#ifndef SIMFLOW_H
#define SIMFLOW_H

extern unsigned long long g_simframe;
extern unsigned long long g_netframe;	//net frames follow sim frames except when there's a pause, simframes will stop but netframes will continue

#define SPEED_PAUSE		1
#define SPEED_PLAY		2
#define SPEED_FAST		3

extern unsigned char g_speed;
extern bool g_gameover;

extern bool g_arrest;

void UpdSpeed();

#endif