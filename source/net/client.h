


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





#ifndef CLIENT_H
#define CLIENT_H

#include "../gui/richtext.h"
#include "../sim/player.h"
#include "netconn.h"
#include "../sim/simflow.h"

//a client is like a player, but concerns networking.
//a client must be a human player.
//a client controls a player slot.
class Client
{
public:
	bool on;
	int player;
	RichText name;
	//unsigned char color;
	NetConn* nc;
	bool unresp;	//unresponsive?
	bool ready;
	short ping;	//for client use; server keeps it in NetConn's
	unsigned char speed;
	unsigned long long curnetfr;	//not so much the frame as the net turn

	Client()
	{
		nc = NULL;
		speed = SPEED_PLAY;
	}
};

#define CLIENTS	PLAYERS

extern Client g_client[CLIENTS];
extern int g_localC;
extern RichText g_name;

void ResetCls();
void ResetCl(Client* c);
bool AddClient(NetConn* nc, RichText name, int* retci);
void AddClient(NetConn* nc, RichText name, int ci);

#endif
