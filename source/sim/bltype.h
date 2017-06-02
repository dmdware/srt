


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




#ifndef BLYPE_H
#define BLYPE_H

#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../math/vec2uc.h"
#include "resources.h"
#include "../render/billboard.h"
#include "../render/particle.h"
#include "conduit.h"

#define MAX_B_EMITTERS	10

#define BLSND_PROD	0	//production sound
#define BLSND_FINI	1	//finished construction sound
#define BLSND_PLACE	2	//placed new construction project sound
#define BLSND_SEL	3	//selected
#define BLSND_CSEL	4	//selected construction
#define BL_SOUNDS	5

#define FLAG_BLOWN_STATE	1
#define FLAG_BLOWN_FIRM		2

class BlType
{
public:
	// width in tiles
	Vec2uc width;

	int nframes;
	int cnframes;
	/*
	The sprites are pointers to arrays of frames for
	non-terrain hugging building sprites, or the set
	of inclination combinations for terrain hugging.
	In the future the terrain hugging ones might
	have frames.
	*/
	unsigned int* sprite;
	unsigned int* csprite;

	char name[256];
	std::string desc;

	int foundation;

	int input[RESOURCES];
	int output[RESOURCES];

	int conmat[RESOURCES];

	int reqdeposit;

	EmitterPlace emitterpl[MAX_B_EMITTERS];

	bool hugterr;

	std::list<unsigned char> manuf;

	short sound[BL_SOUNDS];
	int maxhp;

	short visrange;

	int price[RESOURCES];
	int opwage;

	unsigned char flags;
	int prop;	//building proportion/rate number
	int wprop;	//proportion to workers ratio divisible by RATIO_DENOM

	void free();
	BlType();
	~BlType();
};

//foundation
#define FD_LAND			0
#define FD_COAST		1
#define FD_SEA			2

//building
#define BL_NONE				-1
#define BL_HOUSE1			0
#define BL_STORE			1
#define BL_TRFAC			2
#define BL_FARM				3
#define BL_SHMINE			4
#define BL_IRONSM			5
#define BL_OILWELL			6
#define BL_OILREF			7
#define BL_NUCPOW			8
#define BL_COALPOW			9
#define BL_CEMPL			10
#define BL_CHEMPL			11
//#define BL_ELECPL			12
#define BL_HOUSE2			12
#define BL_GASSTN			13
#define BL_BRK				14
#define BL_TYPES			15

#define BL_ROAD				(BL_TYPES+CD_ROAD)
#define BL_POWL				(BL_TYPES+CD_POWL)
#define BL_CRPIPE			(BL_TYPES+CD_CRPIPE)
//#define BL_WATERPIPE		(BL_TYPES+4)

#define TOTAL_BUILDABLES	(BL_TYPES+CD_TYPES)

extern BlType g_bltype[BL_TYPES];

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
		  int wprop);
void BMat(int type, int res, int amt);
void BIn(int type, int res, int amt);
void BOut(int type, int res, int amt, int defl);
void BEmitter(int type, int emitterindex, int ptype, Vec3f offset);
void BDes(int type, const char* desc);
void BSon(int type, int stype, const char* relative);
void BMan(int type, unsigned char utype);

#endif
