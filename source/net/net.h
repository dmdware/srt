


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





#ifndef NET_H
#define NET_H

#include "../platform.h"
#include "packets.h"

#define PORT		50420
#define SV_ADDR		"polyfrag.com"	//live server
//#define SV_ADDR			"23.226.224.175"		//vps
//#define SV_ADDR		"54.221.229.124"	//corp1 aws
//#define SV_ADDR			"192.168.1.100"		//home local server ip
//#define SV_ADDR			"192.168.1.103"		//home local server ip
//#define SV_ADDR			"174.6.61.178"		//home public server ip

#define RESEND_DELAY	30
#define RESEND_EXPIRE	(5000)
#define NETCONN_TIMEOUT	(30*1000)
#define NETCONN_UNRESP	(NETCONN_TIMEOUT/3)
#define QUIT_DELAY		(10*1000)
//#define SLIDING_WIN		3	//max resend outgoing packets ahead
#define SLIDING_WIN		10	//max resend outgoing packets ahead	//corpd fix
#define RECV_BUFFER		10	//max packets ahead to buffer recieved out of sequence

unsigned short NextAck(unsigned short ack);
unsigned short PrevAck(unsigned short ack);
bool PastAck(unsigned short test, unsigned short current);
bool PastFr(unsigned long long test, unsigned long long current);

//extern unsigned long long g_lastS;  //last sent
//extern unsigned long long g_lastR;  //last recieved

extern int g_netmode;
extern bool g_lanonly;

extern char g_mapname[MAPNAME_LEN+1];
extern char g_svname[SVNAME_LEN+1];
extern unsigned int g_mapcheck;

#ifdef MATCHMAKER
extern unsigned int g_transmitted;
#endif

#define NETM_SINGLE			0	//single player
#define NETM_HOST			1	//hosting
#define NETM_CLIENT			2	//client
//#define NET_SPECT			3	//spectator client

//#define NET_DEBUG	//debug messages for packets recvd

void UpdNet();
void ClearPackets();
void CheckAddSv();
bool Same(IPaddress* a, IPaddress* b);
bool NetQuit();

#ifndef MATCHMAKER
extern unsigned int g_transmitted;
#endif

#endif

