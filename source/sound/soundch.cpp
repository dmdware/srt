


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






#include "soundch.h"

SoundCh g_soundch[SOUND_CHANNELS];

int NewChan()
{
	int best = 0;
	unsigned long long bestd = 0;
	unsigned long long now = GetTicks();

	for(unsigned char i=0; i<SOUND_CHANNELS; i++)
	{
		SoundCh* c = &g_soundch[i];
		unsigned long long d = now - c->last;

		if(d < bestd)
			continue;

		bestd = d;
		best = i;
	}

	return best;
}