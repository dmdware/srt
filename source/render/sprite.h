


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




#ifndef SPRITE_H
#define SPRITE_H

#include "../texture.h"

class Sprite
{
public:

	Sprite();
	~Sprite();
	void free();

	bool on;
	unsigned int difftexi;
	unsigned int teamtexi;
	unsigned int depthtexi;
	float offset[4];	//the pixel texture coordinates centered around a certain point, for use for vertex positions when not cropping (old)
	float crop[4];	//the texture coordinates [0..1] of the cropped sprite, used for the texture coords
	float cropoff[4];	//the pixel texture coordinates of the cropped sprite, used for vertex positions
	LoadedTex* pixels;
	std::string fullpath;
};

#define SPRITES	4096
extern Sprite g_sprite[SPRITES];

class SpriteToLoad
{
public:
	std::string relative;
	unsigned int* spindex;
	bool loadteam;
	bool loaddepth;
};

extern std::vector<SpriteToLoad> g_spriteload;

extern int g_lastLSp;

bool Load1Sprite();
void FreeSprites();
void LoadSprite(const char* relative, unsigned int* spindex, bool loadteam, bool loaddepth);
void QueueSprite(const char* relative, unsigned int* spindex, bool loadteam, bool loaddepth);
void ParseSprite(const char* relative, Sprite* s);
bool PlayAnim(float& frame, int first, int last, bool loop, float rate);
bool PlayAnimB(float& frame, int first, int last, bool loop, float rate);	//Play animation backwards

#endif
