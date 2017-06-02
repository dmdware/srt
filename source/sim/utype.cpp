


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




#include "utype.h"
#include "../texture.h"
#include "resources.h"
#include "../path/pathnode.h"
#include "../utils.h"

UType g_utype[UNIT_TYPES];

void DefU(int type, const char* sprel, int nframes,
	Vec2s size, const char* name,
	int starthp,
	bool landborne, bool walker, bool roaded, bool seaborne, bool airborne,
	int cmspeed, bool military,
	int visrange,
	int prop)
{
	UType* t = &g_utype[type];

	//QueueTexture(
	//QueueModel(&t->model, sprel);

	t->free();

	for(int s=0; s<DIRS; s++)
	{
		t->sprite[s] = new unsigned int [ nframes ];

		for(int f=0; f<nframes; f++)
		{
			char frrel[MAX_PATH+1];
			sprintf(frrel, "%s_si%d_fr%03d", sprel, s, f);
			QueueSprite(frrel, &t->sprite[s][f], true, true);
			//Log("q "<<frrel);
		}
	}

	t->nframes = nframes;

	t->size = size;
	strcpy(t->name, name);
	t->starthp = starthp;
	Zero(t->cost);
	t->landborne = landborne;
	t->walker = walker;
	t->roaded = roaded;
	t->seaborne = seaborne;
	t->airborne = airborne;
	t->cmspeed = cmspeed;
	t->military = military;
	t->visrange = visrange;
	t->prop = prop;

	for(int i=0; i<U_SOUNDS; i++)
		t->sound[i] = -1;
}

void UCost(int type, int res, int amt)
{
	UType* t = &g_utype[type];
	t->cost[res] = amt;
}
