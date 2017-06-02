


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




#ifndef SOUND_H
#define SOUND_H

#include "../platform.h"

class Sound
{
public:
	Sound() 
	{ 
		on = false;
		sample = NULL;
	}
	Sound(const char* fp);
	~Sound();

	bool on;
	Mix_Chunk *sample;
	char filepath[DMD_MAX_PATH+1];
	void play();
};

#define SOUNDS	256

extern Sound g_sound[SOUNDS];

class SoundLoad
{
	char fullpath[DMD_MAX_PATH+1];
	int* retindex;
};

//TO DO queue sound

extern int g_volume;

void SoundPath(const char* from, char* to);
bool QueueSound(const char* relative, short* index);
bool LoadSound(const char* relative, short* index);
void FreeSounds();
void ReloadSounds();
void PlayClip(short si);
void SetVol(int percent);

void Sound_Order();

#endif
