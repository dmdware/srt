


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




#ifndef CONDUIT_H
#define CONDUIT_H

#include "connectable.h"
#include "resources.h"
#include "../render/vertexarray.h"
#include "../math/vec3i.h"
#include "../render/heightmap.h"
#include "../render/depthable.h"

#define CD_ROAD		0
#define CD_POWL		1
#define CD_CRPIPE	2
#define CD_TYPES	3

class CdTile
{
public:

	Depthable* depth;

	bool on;
	unsigned char conntype;
	bool finished;
	unsigned char owner;
	int conmat[RESOURCES];
	short netw;	//network
	//bool inaccessible;
	short transporter[RESOURCES];
	Vec2f drawpos;
	//int maxcost[RESOURCES];
	int conwage;

	CdTile();
	~CdTile();

	//virtual unsigned char cdtype();
	int netreq(int res, unsigned char cdtype);
	void destroy();
	void allocate(unsigned char cdtype);
	bool checkconstruction(unsigned char cdtype);
	virtual void fillcollider();
	virtual void freecollider();
};

class CdType
{
public:
	int conmat[RESOURCES];
	unsigned short netwoff;	//offset to network list in Building class
	unsigned short seloff;	//offset to selection list in Selection class
	//TO DO elevation inclines
	unsigned int sprite[CONNECTION_TYPES][2][INCLINES];	//0 = not finished, 1 = finished/constructed
	unsigned short maxforwincl;
	unsigned short maxsideincl;
	bool blconduct;	//do buildings conduct this resource (also act as conduit in a network?)
	Vec2i physoff;	//offset in cm. physoff from tile(tx,ty) corner, where workers go to build it (collider).
	Vec3i drawoff;	//offset in cm. drawoff from tile(tx,ty) corner, where the depthable sorted box begins.
	CdTile* cdtiles[2];	//0 = actual placed, 1 = plan proposed
	bool cornerpl;	//is the conduit centered on corners or tile centers?
	char name[256];
	std::string desc;
	unsigned int lacktex;
	//reqsource indicates if being connected to the grid 
	//requires connection to a source building, or just the network.
	bool reqsource;
	unsigned char flags;

	CdType()
	{
		Zero(conmat);
		blconduct = false;
		cdtiles[0] = NULL;
		cdtiles[1] = NULL;
		cornerpl = false;
		name[0] = '\0';
		lacktex = 0;
		reqsource = false;
	}

	~CdType()
	{
		for(int i=0; i<2; i++)
			if(cdtiles[i])
			{
				delete [] cdtiles[i];
				cdtiles[i] = NULL;
			}
	}
};

extern CdType g_cdtype[CD_TYPES];

inline CdTile* GetCd(unsigned char ctype, int tx, int ty, bool plan)
{
	CdType* ct = &g_cdtype[ctype];
	CdTile* tilesarr = ct->cdtiles[(int)plan];
	return &tilesarr[ tx + ty*g_mapsz.x ];
}

class Building;
struct PlaceCdPacket;

void DefCd(unsigned char ctype,
			const char* name,
           unsigned short netwoff,
           unsigned short seloff,
           bool blconduct,
           bool cornerpl,
           Vec2i physoff,
           Vec3i drawoff,
		   const char* lacktex,
		   unsigned short maxsideincl,
		   unsigned short maxforwincl,
		   bool reqsource,
		  unsigned char flags);
void CdDes(unsigned char ctype, const char* desc);
void CdMat(unsigned char ctype, unsigned char rtype, short ramt);
void UpdCdPlans(unsigned char ctype, char owner, Vec3i start, Vec3i end);
void ClearCdPlans(unsigned char ctype);
void ReNetw(unsigned char ctype);
void ResetNetw(unsigned char ctype);
bool ReNetwB(unsigned char ctype);
void MergeNetw(unsigned char ctype, int A, int B);
bool ReNetwTl(unsigned char ctype);
bool CompareCo(unsigned char ctype, CdTile* ctile, int tx, int ty);
bool BAdj(unsigned char ctype, int i, int tx, int ty);
bool CompareB(unsigned char ctype, Building* b, CdTile* ctile);
bool CdLevel(unsigned char ctype, float iterx, float iterz, float testx, float testz, float dx, float dz, int i, float d, bool plantoo);
void PlaceCd(unsigned char ctype, int ownerpy);
void PlaceCd(PlaceCdPacket* pcp);
void PlaceCd(unsigned char ctype, int tx, int ty, int owner, bool plan);
void Repossess(unsigned char ctype, int tx, int ty, int owner);
void DrawCd(CdTile* ctile, unsigned char x, unsigned char y, unsigned char cdtype, bool plan, float rendz);
void DrawCd(unsigned char ctype);
void CdXY(unsigned char ctype, CdTile* ctile, bool plan, int& tx, int& ty);
void DefConn(unsigned char conduittype,
	unsigned char connectiontype,
	bool finished,
	const char* sprel);
void PruneCd(unsigned char ctype);
void ConnectCdAround(unsigned char ctype, int x, int y, bool plan);

#endif
