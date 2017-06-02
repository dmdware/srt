


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





#ifndef SENDPACKETS_H
#define SENDPACKETS_H

#include "../platform.h"

class NetConn;

void SendAll(char* data, int size, bool reliable, bool expires, IPaddress* exception);
void SendData(char* data, int size, IPaddress * paddr, bool reliable, bool expires, NetConn* nc, UDPsocket* sock, int msdelay, void (*onackfunc)(OldPacket* p, NetConn* nc));
void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, char* buffer, int bytes);
void ResendPacks();
void Register(char* username, char* password, char* email);
void Login(char* username, char* password);

#endif	//SENDPACKETS_H