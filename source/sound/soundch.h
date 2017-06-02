


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






#ifndef SOUNDCH_H
#define SOUNDCH_H

#include "../platform.h"
#include "../utils.h"

#define SOUND_CHANNELS	16

class SoundCh
{
public:
	unsigned long long last;

	SoundCh(){ last = GetTicks(); }
};

extern SoundCh g_soundch[SOUND_CHANNELS];

int NewChan();

#endif
