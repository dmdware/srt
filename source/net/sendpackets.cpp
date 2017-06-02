


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






#include "net.h"
#include "sendpackets.h"
#include "../sim/player.h"
#include "packets.h"
#include "../sim/simdef.h"
#include "lockstep.h"
#include "parity.h"

void SendAll(char* data, int size, bool reliable, bool expires, IPaddress* exception)
{
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
	{
		//if(ci->ctype != CONN_CLIENT)
		//	continue;

		if(!ci->isclient)
			continue;

		if(exception &&
			//memcmp(&ci->addr, exception, sizeof(IPaddress)) == 0
				Same(&ci->addr, exception))
			continue;

		//InfoMess("sa", "sa");

		SendData(data, size, &ci->addr, reliable, expires, &*ci, &g_sock, 0, NULL);
	}
}

void SendData(char* data, int size, IPaddress * paddr, bool reliable, bool expires, NetConn* nc, UDPsocket* sock, int msdelay, void (*onackfunc)(OldPacket* p, NetConn* nc))
{
	//g_lastS = GetTicks();

#if 0
	if(((PacketHeader*)data)->type == PACKET_NETTURN)
	{
		char msg[128];
		sprintf(msg, "send netturn for netfr%u load=%u", ((NetTurnPacket*)data)->fornetfr, (unsigned int)((NetTurnPacket*)data)->loadsz);
		//InfoMess("s", msg);
		Log(msg);
		FILE* fp = fopen("sntp.txt", "wb");
		fwrite(msg, strlen(msg)+1, 1, fp);
		fclose(fp);
	}
#endif


	if(reliable)
	{
		//InfoMess("sa", "s2");
		((PacketHeader*)data)->ack = nc->nextsendack;
		OldPacket p;
		p.buffer = new char[ size ];
		p.len = size;
		memcpy(p.buffer, data, size);
		memcpy((void*)&p.addr, (void*)paddr, sizeof(IPaddress));
		//in msdelay milliseconds, p.last will be RESEND_DELAY millisecs behind GetTicks()
		p.last = GetTicks() + msdelay - RESEND_DELAY;
		p.first = p.last;
		p.expires = expires;
		p.onackfunc = onackfunc;
		g_outgo.push_back(p);
		nc->nextsendack = NextAck(nc->nextsendack);
	}

	
			//Log("s a"<<((PacketHeader*)data)->ack<<" t"<<((PacketHeader*)data)->type);

#ifdef NET_DEBUG

	//unsigned int ipaddr = SDL_SwapBE32(ip.host);
	//unsigned short port = SDL_SwapBE16(ip.port);
	if(paddr)
	{
		Log("send to "<<SDL_SwapBE32(paddr->host)<<":"<<SDL_SwapBE16(paddr->port)<<" at tick "<<GetTicks()<<" ack"<<((PacketHeader*)data)->ack<<" t"<<((PacketHeader*)data)->type);
		
	}

	int nhs = 0;
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(ci->handshook)
			nhs++;

	Log("g_conn.sz = "<<g_conn.size()<<" numhandshook="<<nhs);
	
#endif

	if(reliable && msdelay > 0)
		return;

	PacketHeader* ph = (PacketHeader*)data;

	if(reliable && 
		(!nc || !nc->handshook) && 
		(ph->type != PACKET_CONNECT && ph->type != PACKET_DISCONNECT && ph->type != PACKET_ACKNOWLEDGMENT && ph->type != PACKET_NOCONN) )
	{
		Connect(paddr, false, false, false, false);
		return;
	}

	//UDPpacket *out = SDLNet_AllocPacket(65535);
	
#if 1
	unsigned char *data2;
	int size2 = Parify((unsigned char*)data, &data2, size);
	
	UDPpacket *out = SDLNet_AllocPacket(size2+1);	//corpd fix
#endif

#if 0
	unsigned char *tempout;

	int outsz = Unparify(data2, &tempout, size2);

	if(outsz)
	{
		Log("self parity success");
		
		free(tempout);
	}
	else
	{
		Log("self parity fail");
		
	}
#endif

	memcpy(out->data, data2, size2);
	out->len = size2;
	out->data[size2] = 0;	//?? needed?
	
	free(data2);

	//memcpy(out->data, data, size);
	//out->len = size;
	//out->data[size] = 0;


	//if(bindaddr)
	{
		SDLNet_UDP_Unbind(*sock, 0);
		if(SDLNet_UDP_Bind(*sock, 0, (const IPaddress*)paddr) == -1)
		{
			char msg[1280];
			sprintf(msg, "SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			ErrMess("Error", msg);
			//printf("SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			//exit(7);
		}
	}

#if 0
	Log("send t"<<((PacketHeader*)data)->type);
	

			char msg[128];
			sprintf(msg, "send ack%u t%d", (unsigned int)((PacketHeader*)out->data)->ack, ((PacketHeader*)out->data)->type);
			InfoMess("send", msg);
#endif

	//sendto(g_socket, data, size, 0, (struct addr *)paddr, sizeof(struct sockaddr_in));
	SDLNet_UDP_Send(*sock, 0, out);

	//Log("sent");

//#ifdef MATCHMAKER
#if 1
	g_transmitted += size;
#endif

	SDLNet_FreePacket(out);
}

void ResendPacks()
{
	OldPacket* p;
	unsigned long long now = GetTicks();
	//unsigned long long due = now - RESEND_DELAY;
	//unsigned long long expire = now - RESEND_EXPIRE;

	//remove expired ack'd packets
	auto i=g_outgo.begin();
	while(i!=g_outgo.end())
	{
		p = &*i;

		if(!p->acked)
		{
			i++;
			continue;
		}

        //p->last and first might be in the future due to delayed sends,
        //which would cause an overflow for unsigned long long.
        unsigned long long safelast = enmin(p->last, now);
        unsigned long long passed = now - safelast;
        unsigned long long safefirst = enmin(p->first, now);

		if(passed < RESEND_EXPIRE)
		{
			i++;
			continue;
		}

		i = g_outgo.erase(i);
	}

	//resend due packets within sliding window
	i=g_outgo.begin();
	while(i!=g_outgo.end())
	{
		p = &*i;

		//kept just in case it needs to be recalled by other side
		if(p->acked)
		{
			i++;
			continue;
		}
        
        unsigned long long safelast = enmin(p->last, now);
        unsigned long long passed = now - safelast;
        unsigned long long safefirst = enmin(p->first, now);

		NetConn* nc = Match(&p->addr);

#if 1
		//increasing resend delay for the same outgoing packet

		unsigned int nextdelay = RESEND_DELAY;
        unsigned long long firstpassed = now - safefirst;

		if(nc && firstpassed >= RESEND_DELAY)
		{
			//fixed
			unsigned long long sincelast = safelast - safefirst;
			//old: 30, 60, 120, 240, +?
			//new: 30, 60, 90, 120, 150, 180, 210, 240, 270
			nextdelay = ((sincelast / RESEND_DELAY) + 1) * RESEND_DELAY;
		}
#endif

		//if(p->last > due)
		//if((nc && passed > (unsigned int)(0.8f * nc->ping)) || (!nc && passed > RESEND_DELAY))
		//if(passed > RESEND_DELAY)
		//if(passed > nextdelay)
		if(passed < nextdelay)
		{
			i++;
			continue;
		}

		PacketHeader* ph = (PacketHeader*)p->buffer;

		/*
		If we don't have a connection to them
		and it's not a control packet, we
		need to connect to them to send reliably.
		Send it when we get a handshake back.
		*/
		if((!nc || !nc->handshook) &&
			ph->type != PACKET_CONNECT &&
			ph->type != PACKET_DISCONNECT &&
			ph->type != PACKET_ACKNOWLEDGMENT &&
			ph->type != PACKET_NOCONN)
		{
			Connect(&p->addr, false, false, false, false);
			i++;
			continue;
		}

#if 1
		if(nc)
		{
			unsigned short lastack = nc->nextsendack + SLIDING_WIN - 1;

			if(PastAck(lastack, ph->ack) && ph->ack != lastack)
			{
				i++;
				continue;
				//don't resend more than SLIDING_WIN packets ahead
			}
		}
#endif

		//if(p->expires && p->first < expire)
		if(p->expires && now - safefirst > RESEND_EXPIRE)
		{
			//i->freemem();
			i = g_outgo.erase(i);

#if 0
			Log("expire at "<<DateTime()<<" dt="<<expire<<"-"<<p->first<<"="<<(expire-p->first)<<"(>"<<RESEND_EXPIRE<<") left = "<<g_outgo.size()<<endl;
			
#endif

			continue;
		}

#ifdef NET_DEBUG
		Log("\t resend...";
		
#endif

		SendData(p->buffer, p->len, &p->addr, false, p->expires, nc, &g_sock, 0, NULL);

		p->last = now;
#ifdef _IOS
		NSLog(@"Resent at %lld", now);
#endif

#if 0
		Log("resent at "<<DateTime()<<" left = "<<g_outgo.size()<<endl;
		
#endif

		i++;
	}
}

void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, char* buffer, int bytes)
{
#if 0	//actually, don't mess with RUDP protocol; build it on top of packets
	//if it's a NetTurnPacket, we have to do something special...
	if(((PacketHeader*)buffer)->type == PACKET_NETTURN)
	{
		NetTurnPacket* ntp = (NetTurnPacket*)buffer;

		//if it's not for the next turn, drop the ack so the server waits for us to complete up to the necessary turn
		if(ntp->fornetfr != (g_netframe / NETTURN + 1) * NETTURN)
		{
			char msg[128];
			sprintf(msg, "faulty turn fr%u", ntp->fornetfr);
			InfoMess("Er", msg);
			return;
		}
	}
#endif

	AckPacket p;
	p.header.type = PACKET_ACKNOWLEDGMENT;
	p.header.ack = ack;

#if 0
	Log("ack "<<ack);
	
#endif
	
#if 0
	Log("pack acknowledgement ack"<<ack<<" to"<<htonl(addr->host)<<":"<<htons(addr->port));
	
#endif

	SendData((char*)&p, sizeof(AckPacket), addr, false, true, nc, sock, 0, NULL);
}




