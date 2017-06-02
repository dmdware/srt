


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




#ifndef BUILDING_H
#define BUILDING_H

#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../platform.h"
#include "resources.h"
#include "bltype.h"
#include "../render/vertexarray.h"
#include "utype.h"
#include "../render/depthable.h"

class VertexArray;

//#define PROD_DEBUG	//bl's production debug output

//unit manufacturing job
class ManufJob
{
public:
	int utype;
	int owner;
};

class CycleHist
{
public:
	int prod[RESOURCES];	//earnings and production
	int cons[RESOURCES];	//expenses and consumption
	int price[RESOURCES];
	int wage;
	
	CycleHist()
	{
		reset();
	}
	
	void reset()
	{
		Zero(prod);
		Zero(cons);
		Zero(price);
		wage = -1;
	}
};

class Building
{
public:

	Depthable* depth;

	bool on;
	int type;
	int owner;

	Vec2i tpos;	//position in tiles
	Vec2f drawpos;	//drawing position in world pixels

	bool finished;

	short pownetw;
	short crpipenetw;
	std::list<short> roadnetw;

	int stocked[RESOURCES];
	int inuse[RESOURCES];

	EmitterCounter emitterco[MAX_B_EMITTERS];

	int conmat[RESOURCES];
	bool inoperation;

	int price[RESOURCES];	//price of produced goods
	int propprice;	//price of this property
	bool forsale;	//is this property for sale?
	bool demolish;	//was a demolition ordered?
	//TODO only one stocked[] and no conmat[]. set stocked[] to 0 after construction finishes, and after demolition ordered.

	std::list<int> occupier;
	std::list<int> worker;
	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	short prodlevel;	//production target level of max RATIO_DENOM
	short cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	unsigned long long lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int manufprc[UNIT_TYPES];
	std::list<ManufJob> manufjob;
	short transporter[RESOURCES];

	int hp;
	
	std::list<CycleHist> cyclehist;

	bool excin(int rtype);	//excess input resource right now?
	bool metout();	//met production target for now?
	int netreq(int rtype);	//how much of an input is still required to meet production target
	bool hasworker(int ui);
	bool tryprod();
	bool trymanuf();
	int maxprod();
	void adjcaps();
	void spawn(int utype, int uowner);
	void morecap(int rtype, int amt);
	void lesscap(int rtype, int amt);
	void getethereal();
	void getethereal(int rtype, int amt);

	void destroy();
	void fillcollider();
	void freecollider();
	void allocres();
	bool checkconstruction();
	Building();
	~Building();
};

#define BUILDINGS	256
//#define BUILDINGS	64

extern Building g_building[BUILDINGS];
class Unit;

int NewBl();
void FreeBls();
void DrawBl();
void DrawBl(Building* b, float rendz);
void UpdBls();
void StageCopyVA(VertexArray* to, VertexArray* from, float completion);
void HeightCopyVA(VertexArray* to, VertexArray* from, float completion);
void HugTerrain(VertexArray* va, Vec3f pos);
void Explode(Building* b);
float CompletPct(int* cost, int* current);
void RemWorker(Unit* w);

#endif
