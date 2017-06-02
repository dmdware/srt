


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





#include "client.h"

#ifndef MATCHMAKER
#include "../gui/layouts/chattext.h"
#include "../language.h"
#endif

Client g_client[CLIENTS];
int g_localC;
RichText g_name = RichText("Player");

void ResetCls()
{
	g_localC = -1;
	g_localP = -1;
	g_speed = SPEED_PLAY;

	for(int i=0; i<CLIENTS; i++)
	{
		Client* c = &g_client[i];

		c->on = false;
		c->name = RichText("Player");
		//c->color = 0;

		if(c->nc)
		{
			Disconnect(c->nc);
			c->nc->client = -1;
			c->nc = NULL;
		}

		//c->nc = NULL;
		c->ready = false;
		c->unresp = false;
		c->speed = SPEED_PLAY;
	}

	for(int i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

		py->client = -1;
	}
}

void ResetCl(Client* c)
{
	c->on = false;
	c->name = RichText("Player");
	c->ready = false;
	c->unresp = false;
	c->speed = SPEED_PLAY;

	if(c->nc)
	{
		c->nc->client = -1;
		c->nc = NULL;
	}

	if(c->player >= 0)
	{
		Player* py = &g_player[c->player];
		py->client = -1;
		py->on = false;
		c->player = -1;
	}

	if(c - g_client == g_localC)
		g_speed = SPEED_PLAY;

	UpdSpeed();
}

int NewClient()
{
	for(int i=0; i<CLIENTS; i++)
		if(!g_client[i].on)
			return i;

	return -1;
}

//version where we already have a client index
void AddClient(NetConn* nc, RichText name, int ci)
{
	if(nc)
		nc->client = ci;

	Client* c = &g_client[ci];

	c->on = true;
	c->player = -1;
	c->name = name;
	c->nc = nc;
	c->speed = SPEED_PLAY;

	if(nc)
		nc->client = ci;

#ifndef MATCHMAKER
	RichText msg = name + RichText(" ") + STRTABLE[STR_JOINEDGAME];
	AddChat(&msg);
#endif
}

//version that gets a new client index from the slots
bool AddClient(NetConn* nc, RichText name, int* retci)
{
	int ci = NewClient();

	if(ci < 0)
		return false;

	//calls the other version here
	AddClient(nc, name, ci);

	if(retci)
		*retci = ci;

	return true;
}
