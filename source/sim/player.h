


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




#ifndef PLAYER_H
#define PLAYER_H

#include "../platform.h"
#include "../net/netconn.h"
#include "resources.h"
#include "../gui/richtext.h"
#include "../econ/institution.h"

#ifndef MATCHMAKER
#include "../gui/gui.h"
#include "../math/camera.h"
#include "selection.h"
//#include "../../script/objectscript.h"
#endif

struct PlayerColor
{
	unsigned char color[3];
	char name[32];
};

#define PLAYER_COLORS	48

extern PlayerColor g_pycols[PLAYER_COLORS];

class Player
{
public:
	bool on;
	bool ai;

	int local[RESOURCES];	// used just for counting; cannot be used
	int global[RESOURCES];
	int resch[RESOURCES];	//resource changes/deltas
	int truckwage;	//truck driver wage per second
	int transpcost;	//transport cost per second
	unsigned long long util;
	unsigned long long gnp;

	float color[4];
	RichText name;
	int client;	//for server
	int entity;

	signed char insttype;	//institution type
	signed char instin;	//institution index
	int parentst;	//parent state player index

	bool protectionism;
	int imtariffratio;	//import tariff ratio
	int extariffratio;	//export tariff ratio

	unsigned long long lastthink;	//AI simframe timer

#ifndef MATCHMAKER
	Player();
	~Player();
#endif
};

//#define PLAYERS 32
#define PLAYERS ARRSZ(g_pycols)
//#define PLAYERS	6	//small number of AI players so it doesn't freeze (as much)

extern Player g_player[PLAYERS];
extern int g_localP;
extern int g_playerm;
extern bool g_diplomacy[PLAYERS][PLAYERS];

extern float g_reddening;

void FreePys();
void AssocPy(int player, int client);
int NewPlayer();
int NewClPlayer();
void DefP(int ID, float red, float green, float blue, float alpha, RichText name);
void DrawPy();
void Bankrupt(int player, const char* reason);
void UpdPys();

#endif
