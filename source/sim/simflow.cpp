


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






#include "simflow.h"
#include "../net/client.h"

//TODO does g_netframe need to be unsigned long long?

bool g_arrest = false;

unsigned long long g_simframe = 0;
unsigned long long g_netframe = 0;	//net frames follow sim frames except when there's a pause, simframes will stop but netframes will continue
unsigned char g_speed = SPEED_PLAY;
bool g_gameover = false;

#ifndef MATCHMAKER
void UpdSpeed()
{
	short fast = 0;
	short play = 0;
	short pause = 0;

	/*
	We need a separate counter for net frames and sim frames,
	because net frames continue while sim frames are paused.
	*/

	//If anybody's paused, we can't continue.
	for(int ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_client[ci];

		if(!c->on)
			continue;

		switch(c->speed)
		{
		case SPEED_FAST:
			fast++;
			break;
		case SPEED_PLAY:
			play++;
			break;
		case SPEED_PAUSE:
			pause++;
			break;
		default:
			break;
		}
	}

	if(pause)
		g_speed = SPEED_PAUSE;
	else if(play)
		g_speed = SPEED_PLAY;
	else if(fast)
		g_speed = SPEED_FAST;
}
#endif