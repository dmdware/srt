


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





#include "bltype.h"
#include "../sound/sound.h"

BlType g_bltype[BL_TYPES];

BlType::BlType()
{
	sprite = NULL;
	csprite = NULL;
	nframes = 0;
	cnframes = 0;
}

BlType::~BlType()
{
	free();
}

void BlType::free()
{
	if(sprite)
	{
		delete [] sprite;
		sprite = NULL;
	}

	if(csprite)
	{
		delete [] csprite;
		csprite = NULL;
	}
}

void DefB(int type,
		  const char* name,
		  Vec2i size,
		  bool hugterr,
		  const char* sprel,
		  int nframes,
		  const char* csprel,
		  int cnframes,
		  int foundation,
		  int reqdeposit,
		  int maxhp,
		  int visrange,
		  int opwage,
		  unsigned char flags,
		  int prop,
		  int wprop)
{
	BlType* t = &g_bltype[type];

	t->free();

	t->width.x = size.x;
	t->width.y = size.y;
	sprintf(t->name, name);
	//QueueModel(&t->model, sprel, scale, translate);
	//QueueModel(&t->cmodel, csprel, cscale, ctranslate);

	/*
	TODO
	Neater way to do this by adding up a string together
	from pieces.
	*/
	if(hugterr)
	{
		t->sprite = new unsigned int [INCLINES];
		t->csprite = new unsigned int [INCLINES];

		for(int i=0; i<INCLINES; i++)
		{
			char specific[MAX_PATH+1];
			sprintf(specific, "%s_inc%s", sprel, INCLINENAME[i]);
			QueueSprite(specific, &t->sprite[i], true, true);
		}
		
		for(int i=0; i<INCLINES; i++)
		{
			char specific[MAX_PATH+1];
			sprintf(specific, "%s_inc%s", csprel, INCLINENAME[i]);
			QueueSprite(specific, &t->csprite[i], true, true);
		}
	}
	else
	{
		t->sprite = new unsigned int [nframes];
		t->csprite = new unsigned int [cnframes];

		for(int f=0; f<nframes; f++)
		{
			char specific[MAX_PATH+1];
			sprintf(specific, "%s_fr%03d", sprel, f);
			QueueSprite(specific, &t->sprite[f], true, true);
		}

		for(int f=0; f<cnframes; f++)
		{
			char specific[MAX_PATH+1];
			sprintf(specific, "%s_fr%03d", csprel, f);
			QueueSprite(specific, &t->csprite[f], true, true);
		}
	}

	t->foundation = foundation;
	t->hugterr = hugterr;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;

	for(int i=0; i<BL_SOUNDS; i++)
		t->sound[i] = -1;

	t->maxhp = maxhp;
	t->manuf.clear();
	t->visrange = visrange;

	Zero(t->price);

	t->opwage = opwage;
	t->flags = flags;
	t->prop = prop;
	t->wprop = wprop;
}

void BMan(int type, unsigned char utype)
{
	BlType* t = &g_bltype[type];
	t->manuf.push_back(utype);
}

void BSon(int type, int stype, const char* relative)
{
	BlType* t = &g_bltype[type];
	LoadSound(relative, &t->sound[stype]);
}

void BDes(int type, const char* desc)
{
	BlType* t = &g_bltype[type];
	t->desc = desc;
}

void BMat(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->conmat[res] = amt;
}

void BIn(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->input[res] = amt;
}

//defl = default price
void BOut(int type, int res, int amt, int defl)
{
	BlType* t = &g_bltype[type];
	t->output[res] = amt;
	t->price[res] = defl;
}

void BEmitter(int type, int emitterindex, int ptype, Vec3f offset)
{
	BlType* t = &g_bltype[type];
	EmitterPlace* e = &t->emitterpl[emitterindex];
	*e = EmitterPlace(ptype, offset);
}
