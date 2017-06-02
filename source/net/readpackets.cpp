


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





#include "readpackets.h"
#include "sendpackets.h"
#include "packets.h"
#include "net.h"
#include "netconn.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../utils.h"
#include "lockstep.h"
#include "../sim/build.h"
#include "../sim/simdef.h"
#include "../sim/simflow.h"
#include "../sys/unicode.h"
#include "client.h"
#include "../save/savemap.h"

#ifndef MATCHMAKER
#include "../gui/widgets/spez/svlist.h"
#include "../gui/widgets/spez/lobby.h"
#include "../gui/layouts/ggui.h"
#include "../gui/layouts/chattext.h"
#include "../sim/manuf.h"
#include "../gui/layouts/messbox.h"
#include "../app/appmain.h"
#include "../sim/order.h"
#include "../gui/widgets/spez/roleview.h"
#endif

/*
What this function does is take a range of packet ack's (acknowledgment number for reliable UDP transmission)
and executes that range of buffered received packets. This is needed because packets might arrive out of order,
be missing some in between, and I execute them only after a whole range up to the latest ack has been received.

The out-of-order packets are stored in the g_recv vector.

Notice that there is preprocessor check if we are compiling this for the master server MATCHMAKER (because I'm
making this for a persistent, online world) or client. If server, there's extra parameters to match the packets
to the right client; we're only interested in processing the packet range for a certain client.

Each packet goes to the PacketSwitch function, that is like a switch-table that executes the right
packet-execution function based on the packet type ID. The switch-table could probably be turned into
an array of function pointers to improve performance, probably only slightly.

The function takes a time of log(O) to execute, because it has to search through all the buffered packets
several times to execute them in the right order. And before that, there's a check to see if we even have
the whole range of packets from the last "lastrecvack" before calling this function.

I keep a "nextsendack" and "lastrecvack" for each client, for sent packets and received packets. I only update the
lastrecvack up to the latest one once a continuous range has been received, with no missing packets. Recvack
is thus the last executed received packet.
*/

unsigned short ParseRecieved(unsigned short first, unsigned short last, NetConn* nc)
{
	OldPacket* p;
	PacketHeader* header;
	unsigned short current = first;
	unsigned short afterlast = NextAck(last);

	do
	{
		bool execd = false;

		for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		{
			p = &*i;
			header = (PacketHeader*)&p->buffer;

			if(header->ack != current)
				continue;

			//if(memcmp((void*)&p->addr, (void*)&nc->addr, sizeof(IPaddress)) != 0)
			if(!Same(&p->addr, &nc->addr))
				continue;

			PacketSwitch(header->type, p->buffer, p->len, nc, &p->addr, &g_sock);
			execd = true;
			current = NextAck(current);

			//p->freemem();
			i = g_recv.erase(i);
			break;
		}

		//corpd fix
		if(execd)
			continue;

		break;
	} while(current != afterlast);

	return PrevAck(current);
}

//do what needs to be done when we've recieved a packet range [first,last]
bool Recieved(unsigned short first, unsigned short last, NetConn* nc)
{
	OldPacket* p;
	PacketHeader* header;
	unsigned short current = first;
	unsigned short afterlast = NextAck(last);
	bool missed;

	do
	{
		missed = true;
		for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		{
			p = &*i;
			header = (PacketHeader*)&p->buffer;

			if(header->ack != current)
				continue;

			//if(memcmp((void*)&p->addr, (void*)&nc->addr, sizeof(IPaddress)) != 0)
			if(!Same(&p->addr, &nc->addr))
				continue;

			current = NextAck(current);
			missed = false;
			break;
		}

		if(missed)
			return false;
	} while(current != afterlast);

	return true;
}

void AddRecieved(char* buffer, int len, NetConn* nc)
{
	OldPacket p;
	//corpd fix
	p.addr = nc->addr;
	//p.acked
	p.buffer = new char[ len ];
	p.len = len;
	memcpy((void*)p.buffer, (void*)buffer, len);
	memcpy((void*)&p.addr, (void*)&nc->addr, sizeof(IPaddress));

	//char cm[1280];
	//PacketHeader *ph = (PacketHeader*)buffer;
	//sprintf(cm, "[sys] add recvd queue ack=%hu cur_recv_ack=%hu type=%hu", ph->ack, nc->lastrecvack, ph->type);
	//RichText msg = RichText(cm);
	//AddChat(&msg);

#if 0
	if(ph->type == PACKET_CLSTATE)
	{
#if 0

		struct ClStatePacket
		{
			PacketHeader header;
			unsigned char chtype;
			short client;
			float ping;
		};
#endif
		ClStatePacket *csp = (ClStatePacket*)buffer;
		//sprintf(cm, "[sys]clstate chtype=%hhu cli=%hd ping=%f", csp->chtype, csp->client, csp->ping);
		//msg = RichText(cm);
		//AddChat(&msg);
	}

#endif

	g_recv.push_back(p);
}

void LastUnrecvd(IPaddress* from, unsigned short lastack)
{

}

void TranslatePacket(char* buffer, int bytes, bool checkprev, UDPsocket* sock, IPaddress* from)
{
	PacketHeader* header = (PacketHeader*)buffer;

#if 0
	Log("pack translate t"<<header->type<<" ack"<<header->ack<<" from"<<htonl(from->host)<<":"<<htons(from->port));

#endif

	NetConn* nc = Match(from);
	if(nc)
	{
#ifdef NET_DEBUG
		//#if 1
		//unsigned int ipaddr = SDL_SwapBE32(ip.host);
		//unsigned short port = SDL_SwapBE16(ip.port);

		Log("upd last "<<SDL_SwapBE32(nc->addr.host)<<":"<<SDL_SwapBE16(nc->addr.port)<<" "<<DateTime()<<" msec"<<GetTicks());

#endif
		nc->lastrecv = GetTicks();

#ifndef MATCHMAKER
		//check if was previously unresponsive
		//and if (s)he was, tell others that (s)he
		//is now responsive.
		if(nc->client >= 0)
		{
			Client* c = &g_client[nc->client];

			if(c->unresp)
			{
				c->unresp = false;

				if(g_netmode == NETM_HOST)
				{
					//inform others
					ClStatePacket csp;
					csp.header.type = PACKET_CLSTATE;
					csp.chtype = CLCH_RESP;
					csp.client = nc->client;
					SendAll((char*)&csp, sizeof(ClStatePacket), true, false, &nc->addr);
				}
			}
		}
#endif
	}

	//bool bindaddr = true;

	//control packets
	switch(header->type)
	{
	case PACKET_ACKNOWLEDGMENT:
	case PACKET_CONNECT:
	case PACKET_DISCONNECT:	//need to send back ack
	case PACKET_NOCONN:
	case PACKET_NACK:
	case PACKET_LANCALL:
	case PACKET_LANANSWER:
		checkprev = false;
		break;
	default:
		break;
	}

#ifndef MATCHMAKER
	//if(g_loadbytes > 0)
	{
		//char msg[128];
		//sprintf(msg, DateTime().c_str());
		//MessageBlock(msg, true);
	}
#endif

	//Log("pack ack"<<header->ack<<" t"<<header->type<<" ::"<<SDL_SwapBE32(from->host)<<":"<<SDL_SwapBE16(from->port)<<" "<<DateTime());

	//InfoMess("ra","ra");

	unsigned short next;	//next expected packet ack
	unsigned short last = PrevAck(header->ack);	//last packet ack to be executed

	if(checkprev && nc != NULL)
	{
		next = NextAck(nc->lastrecvack);	//next expected packet ack
		last = next;	//last packet ack to be executed

		//CASE #1: “old” packet
		if(PastAck(header->ack, nc->lastrecvack) || Recieved(header->ack, header->ack, nc))
		{
			Acknowledge(header->ack, nc, from, sock, buffer, bytes);
			//InfoMess("a", "pa");
			//Log("past ack "<<header->ack<<" pa"<<PastAck(header->ack, nc->lastrecvack)<<",r"<<Recieved(header->ack, header->ack, nc));
#ifdef NET_DEBUG
			//#if 1
			char msg[128];
			sprintf(msg, "\tpast ack%u t%d nc->recack=%u", (unsigned int)header->ack, header->type, (unsigned int)nc->lastrecvack);
			Log(msg);
#endif
			//InfoMess("pa", msg);
			return;
		}

		//CASE #2: current packet (the next expected packet)
		if(header->ack == next) 
		{
			// Translate packet
			last = next;
			//last = PrevAck(header->ack);
		} 

		//CASE #3: an unbuffered, future packet
		else  // More than +1 after lastrecvack?
		{
			/*
			last will be updated to the last executed packet at the end.
			for now it will hold the last buffered packet to be executed.
			*/
			unsigned short checklast = PrevAck(header->ack);

			if(Recieved(next, checklast, nc))
			{
				// Translate in order
				last = checklast;
				goto procpack;
			}
			else
			{
				AddRecieved(buffer, bytes, nc);

				if(Recieved(next, checklast, nc))
				{
					// Translate in order
					last = checklast;
					goto procpack;
				}
				else
				{
					//TODO
					//how to find which ack was missed, have to go through all buffered
					//this is something somebody smart can do in the future
					//NAckPacket nap;
					//nap.header.type = PACKET_NACK;
					//nap.header.ack =
				}
			}
		}
	}

	procpack:

#if 1
	//ack SvInfoPacket's before disconnect...
	nc = Match(from);
	//bool acked = false;
	//ack Connect packets after new NetConn added...
	//Don't acknowledge NoConn packets as they are non-reliable,
	//and ack'ing them would cause a non-ending ack loop.
	if(header->type != PACKET_ACKNOWLEDGMENT &&
		header->type != PACKET_NOCONN &&
		sock && nc)
	{
		//corpd fix
		//Is this an after-connection packet? Update ack / sequence number.
		//if( (header->type != PACKET_CONNECT || !((ConnectPacket*)buffer)->reconnect) )
		//	nc->lastrecvack = header->ack;
		//Already done in ReadConnectPacket ahead

		Acknowledge(header->ack, nc, from, sock, buffer, bytes);
	}
	//corpd fix
#if 1
	//corpd fix: ack even if we already have nc
	else if( ( header->type == PACKET_CONNECT /* && !((ConnectPacket*)buffer)->reconnect */ ) &&
		sock /* && !nc */ )
	{
		Acknowledge(header->ack, NULL, from, sock, buffer, bytes);
	}
#endif
	//corpd fix
	//nc might still point to deallocated NetConn
	//edit: only gets dealloc'd further down
	else if(header->type == PACKET_DISCONNECT /* && !nc */ && sock)
	{
		Acknowledge(header->ack, NULL, from, sock, buffer, bytes);
	}
#endif

	//We're getting an anonymous packet.
	//Maybe we've timed out and they still have a connection.
	//Tell them we don't have a connection.
	//We check if sock is set to make sure this isn't a local
	//command packet being executed.
	if(!nc &&
		header->type != PACKET_CONNECT &&
		header->type != PACKET_NOCONN &&
		header->type != PACKET_LANCALL &&
		header->type != PACKET_LANANSWER &&
		sock)
	{
		NoConnectionPacket ncp;
		ncp.header.type = PACKET_NOCONN;
		SendData((char*)&ncp, sizeof(NoConnectionPacket), from, false, true, NULL, &g_sock, 0, NULL);
		return;
	}

updinack:

	// Translate in order
	if(checkprev && nc)
	{
		last = header->ack;
		last = ParseRecieved(next, last, nc);
	}

	// Translate in order
	if(NextAck(last) == header->ack ||
		!checkprev)
	{
		PacketSwitch(header->type, buffer, bytes, nc, from, sock);
		last = header->ack;
	}

	// Translate in order
	if(checkprev && nc)
	{
		while(true)
		{
			if(!Recieved(last+1, last+1, nc))
				break;

			last++;
			ParseRecieved(last, last, nc);
		}
	}

#if 1
	//have to do this again because PacketSwitch might
	//read a ConnectPacket, which adds new connections.
	//have to comment this out because connection might have
	//been Disconnected(); and erased.
	//if(!nc)
	nc = Match(from);

	//ack Connect packets after new NetConn added...
	//Don't acknowledge NoConn packets as they are non-reliable
	if(header->type != PACKET_ACKNOWLEDGMENT &&
		header->type != PACKET_NOCONN &&
		sock && nc /* && !acked */ && checkprev)
	{
#if 0
		if(!nc)
		{
			NetConn newnc;
			newnc.addr = *from;
			newnc.handshook = false;
			newnc.nextsendack = 0;
			newnc.lastrecvack = GetTicks();
			g_conn.push_back(newnc);
			nc = &*g_conn.rbegin();
		}
#endif

		if(header->type != PACKET_CONNECT &&
			header->type != PACKET_DISCONNECT)
			//nc->lastrecvack = header->ack;
			nc->lastrecvack = last;

		//corpd fix
		//Acknowledge(header->ack, nc, from, sock, buffer, bytes);
	}
#endif
}

void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef NET_DEBUG
	//unsigned int ipaddr = SDL_SwapBE32(ip.host);
	//unsigned short port = SDL_SwapBE16(ip.port);
	//warning: "from" might be NULL
	Log("psw "<<((PacketHeader*)buffer)->type<<" ack"<<((PacketHeader*)buffer)->ack<<" from "<<(from ? SDL_SwapBE32(from->host) : 0)<<":"<<(from ? SDL_SwapBE16(from->port) : 0));

	int nhs = 0;
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(ci->handshook)
			nhs++;

	Log("g_conn.sz = "<<g_conn.size()<<" numhandshook="<<nhs);

#endif

	switch(type)
	{
	case PACKET_ACKNOWLEDGMENT:
		ReadAckPacket((AckPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CONNECT:
		ReadConnectPacket((ConnectPacket*)buffer, nc, from, sock);
		break;
	case PACKET_DISCONNECT:
		ReadDisconnectPacket((DisconnectPacket*)buffer, nc, from, sock);
		break;
	case PACKET_NOCONN:
		ReadNoConnPacket((NoConnectionPacket*)buffer, nc, from, sock);
		break;
	case PACKET_PLACEBL:
		ReadPlaceBlPacket((PlaceBlPacket*)buffer, nc, from, sock);
		break;
	case PACKET_MOVEORDER:
		ReadMoveOrderPacket((MoveOrderPacket*)buffer, nc, from, sock);
		break;
	case PACKET_PLACECD:
		ReadPlaceCdPacket((PlaceCdPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ORDERMAN:
		ReadOrderManPacket((OrderManPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SETSALEPROP:
		ReadSetSalePropPacket((SetSalePropPacket*)buffer, nc, from, sock);
		break;
	case PACKET_BUYPROP:
		ReadBuyPropPacket((BuyPropPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CHAT:
		ReadChatPacket((ChatPacket*)buffer, nc, from, sock);
		break;
	case PACKET_NETTURN:
		ReadNetTurnPacket((NetTurnPacket*)buffer, nc, from, sock);
		break;
	case PACKET_DONETURN:
		ReadDoneTurnPacket((DoneTurnPacket*)buffer, nc, from, sock);
		break;
	case PACKET_JOIN:
		ReadJoinPacket((JoinPacket*)buffer, nc, from, sock);
		break;
	case PACKET_WRONGVERSION:
		ReadWrongVersionPacket((WrongVersionPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ADDSV:
		ReadAddSvPacket((AddSvPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ADDEDSV:
		ReadAddedSvPacket((AddedSvPacket*)buffer, nc, from, sock);
		break;
	case PACKET_KEEPALIVE:
		//Log("recv kap");
		//don't need to do anything here. TranslatePacket already upped the nc->lastrecvack.
		//ReadKeepAlivePacket((KeepAlivePacket*)buffer, nc, from, sock);
		break;
	case PACKET_GETSVLIST:
		ReadGetSvListPacket((GetSvListPacket*)buffer, nc, from, sock);
		break;
		//TODO check packet size to be at least expected size for packet type to avoid memory violation
	case PACKET_LANCALL:
		ReadLANCallPacket((LANCallPacket*)buffer, nc, from, sock);
		break;
	case PACKET_LANANSWER:
		ReadLANAnswerPacket((LANAnswerPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SVADDR:
		ReadSvAddrPacket((SvAddrPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SENDNEXTHOST:
		ReadSendNextHostPacket((SendNextHostPacket*)buffer, nc, from, sock);
		break;
	case PACKET_NOMOREHOSTS:
		ReadNoMoreHostsPacket((NoMoreHostsPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SVINFO:
		ReadSvInfoPacket((SvInfoPacket*)buffer, nc, from, sock);
		break;
	case PACKET_GETSVINFO:
		ReadGetSvInfoPacket((GetSvInfoPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ADDCLIENT:
		ReadAddClPacket((AddClientPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SELFCLIENT:
		ReadSelfClPacket((SelfClientPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SETCLNAME:
		ReadSetClNamePacket((SetClNamePacket*)buffer, nc, from, sock);
		break;
	case PACKET_CLIENTLEFT:
		ReadClientLeftPacket((ClientLeftPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CLIENTROLE:
		ReadClientRolePacket((ClientRolePacket*)buffer, nc, from, sock);
		break;
	case PACKET_DONEJOIN:
		ReadDoneJoinPacket((DoneJoinPacket*)buffer, nc, from, sock);
		break;
	case PACKET_TOOMANYCL:
		ReadTooManyClPacket((TooManyClPacket*)buffer, nc, from, sock);
		break;
	case PACKET_MAPCHANGE:
		ReadMapChPacket((MapChangePacket*)buffer, nc, from, sock);
		break;
	case PACKET_MAPSTART:
		ReadMapStartPacket((MapStartPacket*)buffer, nc, from, sock);
		break;
	case PACKET_GAMESTARTED:
		ReadGameStartedPacket((GameStartedPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CHVAL:
		ReadChValPacket((ChValPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CLSTATE:
		ReadClStatePacket((ClStatePacket*)buffer, nc, from, sock);
		break;
	default:
		break;
	}
}

void ReadAckPacket(AckPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	OldPacket* p;
	PacketHeader* header;

#ifndef MATCHMAKER
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
#endif

	for(auto i=g_outgo.begin(); i!=g_outgo.end(); i++)
	{
		p = &*i;
		header = (PacketHeader*)p->buffer;
		if(header->ack == ap->header.ack &&
			//memcmp((void*)&p->addr, (void*)from, sizeof(IPaddress)) == 0
				Same(&p->addr, from))
		{
			if(p->acked)
				return;	//already dealt with, will expire

			if(!nc)
				nc = Match(from);

			if(nc)
			{
				//nc->ping = ((float)(GetTicks() - i->first) + nc->ping) / 2.0f;
				nc->ping = (float)(GetTicks() - i->first);

#ifndef MATCHMAKER
				//update sv listing info
				if(nc->ishostinfo)
				{
					for(auto sit=v->m_svlist.begin(); sit!=v->m_svlist.end(); sit++)
					{
						if(!Same(&sit->addr, from))
							continue;

						sit->ping = (int)nc->ping;
						char pingstr[16];
						sprintf(pingstr, "%d", (int)nc->ping);
						sit->pingrt = RichText(pingstr);
					}
				}
#endif

#ifdef NET_DEBUG
				Log("new ping for "<<nc->addr.host<<": "<<nc->ping);

#endif
			}

			if(p->onackfunc)
				p->onackfunc(p, nc);

			//p->freemem();
			i = g_outgo.erase(i);
			//p->acked = true;
#if 0
			Log("left to ack "<<g_outgo.size());

#endif
			return;
		}
	}
}

void ReadClStatePacket(ClStatePacket* csp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#if 0
#define CLCH_UNRESP			0	//client became unresponsive
#define CLCH_RESP			1	//became responsive again
#define CLCH_PING			2
#define CLCH_READY			3	//client became ready to start
#define CLCH_NOTREADY		4	//client became not ready

	struct ClStatePacket
	{
		PacketHeader header;
		unsigned char chtype;
		short client;
		float ping;
	};
#endif

#ifndef MATCHMAKER
	//TODO
	//InfoMess("ss", "cs");
	//Log("cl p "<<csp->ping);

	Client* c = &g_client[csp->client];

	//Is it from an established connection?
	if(nc && (nc->isourhost || nc->isclient))
	{
		switch(csp->chtype)
		{
		case CLCH_UNRESP:
			c->unresp = true;
			break;
		case CLCH_RESP:
			c->unresp = false;
			break;
		case CLCH_READY:
			c->ready = true;
			break;
		case CLCH_NOTREADY:
			c->ready = false;
			break;
		case CLCH_PING:
			c->ping = (short)csp->ping;
			break;
		default:
			break;
		};

		//Is it from a client? Should resend to others?
		if(nc->isclient)
		{
			switch(csp->chtype)
			{
			case CLCH_UNRESP:
			case CLCH_RESP:
			case CLCH_READY:
			case CLCH_NOTREADY:
			case CLCH_PING:
				SendAll((char*)csp, sizeof(ClStatePacket), true, false, &nc->addr);
				break;
			case CLCH_PAUSE:
			case CLCH_PLAY:
			case CLCH_FAST:
				LockCmd((PacketHeader*)csp);
				break;
			default:
				break;
			};
		}
	}
	//Is it from us? (Lockstep command queue?)
	else if(!nc && !sock)
	{
		RichText mess;
		mess = c->name;
		char add[128];

		//InfoMess("sp","sp");

		switch(csp->chtype)
		{
		case CLCH_PAUSE:
			c->speed = SPEED_PAUSE;
			//sprintf(add, " paused. simfr:%u netfr:%u", g_simframe, g_netframe);
			mess = mess + RichText(" ") + STRTABLE[STR_PAUSED];
			AddChat(&mess);
			break;
		case CLCH_PLAY:
			c->speed = SPEED_PLAY;
			//sprintf(add, " pressed play.");
			mess = mess + RichText(" ") + STRTABLE[STR_PRESSEDPLAY];
			AddChat(&mess);
			break;
		case CLCH_FAST:
			c->speed = SPEED_FAST;
			//sprintf(add, " pressed fast forward.");
			mess = mess + RichText(" ") + STRTABLE[STR_PRESSEDFAST];
			AddChat(&mess);
			break;
		default:
			break;
		};

		UpdSpeed();
	}
#endif
}

void ReadLANCallPacket(LANCallPacket* lcp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(!nc)
		nc = Match(from);

	if(nc)
		return;

	if(g_netmode != NETM_HOST)
		return;

	LANAnswerPacket lap;
	lap.header.type = PACKET_LANANSWER;

	SendData((char*)&lap, sizeof(LANAnswerPacket), from, false, true, nc, &g_sock, 0, NULL);
}

void ReadLANAnswerPacket(LANAnswerPacket* lap, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(!nc)
		nc = Match(from);

	if(nc)
		return;

	if(g_netmode == NETM_HOST)
		return;

#if 0
	unsigned int host = htonl(from->host);
	unsigned short port = htons(from->port);

	char ipstr[128];
	sprintf(ipstr, "%d.%d.%d.%d:%hu",
		( host >> 24 ) & 0xff,
		( host >> 16 ) & 0xff,
		( host >> 8 ) & 0xff,
		( host >> 0 ) & 0xff,
		port );

	InfoMess("r lan aw", ipstr );
#endif

	Connect(from, false, false, false, true);

#if 1
	GetSvInfoPacket gsip;
	gsip.header.type = PACKET_GETSVINFO;
	SendData((char*)&gsip, sizeof(GetSvInfoPacket), from, true, false, Match(from), &g_sock, 1, NULL);
#endif
}

void ReadDoneTurnPacket(DoneTurnPacket* dtp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	//TO DO check for player==nc->player

	if(!nc)
		nc = Match(from);

	if(!nc)
		return;

	//if(nc->ctype != CONN_CLIENT)
	//	return;
	if(!nc->isclient)
		return;

	//if(dtp->player != nc->player)
	//	return;

	if(dtp->client < 0 || dtp->client >= CLIENTS)
		return;

	Client* c = &g_client[dtp->client];

	//we added 1 because the client is on the verge of the next turn
	//adding 1 gives the next turn
	//py->fornetfr = dtp->fornetfr + 1;
	//edit: I now found that when pausing the sv is ahead of cl by 1 turn.
	//This is probably because the pause command is executed 1 turn later
	//on the sv. Let's try not adding 1:
	//Actually, it was ahead by 33 sim frames, not sure about net frames.
	c->curnetfr = dtp->curnetfr;

#if 0
	char msg[128];
	sprintf(msg, "read done turn packet netf%u", py->fornetfr);
	InfoMess("r", msg);
#endif
#endif
}

void ReadNetTurnPacket(NetTurnPacket* ntp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	//InfoMess("a", "rntp");

	if(g_netmode == NETM_CLIENT && nc->isourhost)
	{
#if 0
		char msg[128];
		sprintf(msg, "recv netturn for netfr%u load=%u", ntp->fornetfr, (unsigned int)ntp->loadsz);
		//InfoMess("r", msg);
		//if(ntp->loadsz > 0)
		{
			FILE* fp = fopen("rntp.txt", "wb");
			fwrite(msg, strlen(msg)+1, 1, fp);
			fclose(fp);
		}
#endif

#if 0
		unsigned long long turnframes = TurnFrames();
		unsigned long long nextturn = g_netframe / turnframes + 1;

		//not for next turn?
		//NetTurnPacket for a turn must be executed at the beginning (netframe % frames == 0) frame
		if(ntp->fornetfr != nextturn * turnframes)
		{
			//something wrong, did we miss a turn?
			if(PastFr(g_netframe, ntp->fornetfr))
				//ErrMess("Error", "Turn missed?");
					AddChat(&RichText("ERROR: Lockstep turn missed?"));
			else
				//ErrMess("Error", "Future turn?");
				AddChat(&RichText("ERROR: Future lockstep turn?"));

			//return;	//if not, discard
			//edit: how can we discard a NetTurnPacket if we won't get it again?
		}
#endif

#if 0
		//Cl_StepTurn(ntp);
		AppendCmds(&g_nextcmd, ntp);
		g_canturn = true;
#endif

		unsigned long long turnframes = TurnFrames();
		unsigned long long nextturn = g_netframe / turnframes + 1;
		unsigned long long nextnextturn = nextturn + 1;

		//if(ntp->fornetfr == nextturn * turnframes &&
		//	!g_canturn)
		//If we're ahead of the server by 1 turn...
		if(!g_next.canturn)
		{
			AppendCmds(&g_next.cmds, ntp);
			g_next.canturn = true;
			g_next.startnetfr = ntp->fornetfr;
		}
		//We don't know turn length here, we just know it comes after
		//Hrm...
		//else if(ntp->fornetfr == nextnextturn * turnframes)
		//if(ntp->fornetfr > nextturn * turnframes)
		//If we're at the same turn as the server...
		else if(!g_next2.canturn)
		{
			AppendCmds(&g_next2.cmds, ntp);
			g_next2.canturn = true;
			g_next2.startnetfr = ntp->fornetfr;
			//g_canturn2 = true;
		}
		else
		{
			NetTurn nt;
			AppendCmds(&nt.cmds, ntp);
			nt.canturn = true;
			nt.startnetfr = ntp->fornetfr;
			g_next3.push_back(nt);
		}
#if 0
		else
		{
			//something wrong, did we miss a turn?
			if(PastFr(g_netframe, ntp->fornetfr))
			{
				//ErrMess("Error", "Turn missed?");
				RichText mess = RichText("ERROR: Lockstep turn missed?");
				AddChat(&mess);
			}
			else
			{
				//ErrMess("Error", "Future turn?");
				RichText mess = RichText("ERROR: Future lockstep turn?");
				AddChat(&mess);
			}
		}
#endif

#if 0
		//char msg[128];
		sprintf(msg, "passed for netfr%u", ntp->fornetfr);
		InfoMess("Error", msg);
#endif
	}
	else if(g_netmode == NETM_HOST)	//cl can't send batch of commands packet
	{
		//AppendCmds(&g_nextnextcmd, ntp);
	}
#endif
}

void ReadOrderManPacket(OrderManPacket* omp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(omp->bi < 0)
		return;

	if(omp->bi >= BUILDINGS)
		return;

	if(omp->utype < 0)
		return;

	if(omp->utype >= UNIT_TYPES)
		return;

	if(omp->player < 0)
		return;

	if(omp->player >= PLAYERS)
		return;

	if(!from)
	{
		OrderMan(omp->utype, omp->bi, omp->player);
		return;
	}

	else if(g_netmode == NETM_HOST)
	{
#if 0
		//AppendCmd(&g_nextnextcmd, (PacketHeader*)omp, sizeof(OrderManPacket));

		std::list<PacketHeader*>* q;

		//if(!g_next.canturn)
		//	q = &g_next.cmds;
		//else 
		if(!g_next2.canturn)
			q = &g_next2.cmds;
		else
		{
			NetTurn nt;
			g_next3.push_back(nt);
			q = &g_next3.rbegin()->cmds;
		}

		AppendCmd(q, (PacketHeader*)omp, sizeof(OrderManPacket));
#else
		LockCmd((PacketHeader*)omp);
#endif
	}
	else if(g_netmode == NETM_SINGLE)
	{
		//OrderMan(omp->utype, omp->bi, omp->player);
		LockCmd((PacketHeader*)omp);
	}
#endif
}

void ReadSetSalePropPacket(SetSalePropPacket* sspp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{	
#ifndef MATCHMAKER
	//if(sspp->bi < 0)
	//	return;

	//if(sspp->bi >= BUILDINGS)
	//	return;

	//if(sspp->pi < 0)
	//	return;

	//if(sspp->pi >= PLAYERS)
	//	return;

	if(!from)
	{
		bool act = false;
		bool act2 = false;
		int owner;
		bool* forsale;
		int* propprice;
		char* tname;

		if(sspp->proptype >= PROP_BL_BEG &&
			sspp->proptype < PROP_BL_END)
		{
			if(sspp->propi < 0)
				return;

			if(sspp->propi >= BUILDINGS)
				return;
			
			Building* b;
			BlType* bt;
			b = &g_building[sspp->propi];
			owner = b->owner;
			forsale = &b->forsale;
			propprice = &b->propprice;
			bt = &g_bltype[b->type];
			tname = bt->name;
		}
		else if(sspp->proptype >= PROP_U_BEG &&
			sspp->proptype < PROP_U_END)
		{
			if(sspp->propi < 0)
				return;

			if(sspp->propi >= UNITS)
				return;
			
			Unit* u;
			UType* ut;
			u = &g_unit[sspp->propi];
			owner = u->owner;
			forsale = &u->forsale;
			propprice = &u->price;
			ut = &g_utype[u->type];
			tname = ut->name;
		}
		//TODO conduits
		//TODO units

		if(*forsale != sspp->selling)
			act = true;

		*forsale = sspp->selling;

		if(*propprice != sspp->price)
			act2 = true;

		*propprice = sspp->price;

		Player* py = &g_player[owner];
		RichText* name = &py->name;

		if(py->client >= 0)
		{
			Client* c = &g_client[py->client];
			name = &c->name;
		}

		Resource* r = &g_resource[RES_DOLLARS];

		//put up or down from sale?
		if(act)
		{
			char text[256];
			RichText rich;

			sprintf(text, "%s.", iform(sspp->price).c_str());

			if(sspp->selling)
				rich = *name + 
				RichText(" ") + 
				STRTABLE[STR_PUTUP] + 
				RichText(" ") + 
				RichText(tname) + 
				RichText(" ") + 
				STRTABLE[STR_FORSALEFOR] + 
				RichText(" ") + 
				RichText(text);
			else
				rich = *name + 
				RichText(" ") + 
				STRTABLE[STR_TOOKDOWN] + 
				RichText(" ") + 
				RichText(tname) + 
				RichText(" ") + 
				STRTABLE[STR_FROMSALE] + 
				RichText(".");

			AddChat(&rich);
		}
		//changed price?
		else if(act2 && *forsale)
		{
			char text[256];

			sprintf(text, "%s.", iform(sspp->price).c_str());

			RichText rich = *name + 
				RichText(" ") + 
				STRTABLE[STR_CHANGEDPROPPRICEOF] + 
				RichText(" ") + 
				RichText(tname) + 
				RichText(" ") + 
				STRTABLE[STR_TO] + 
				RichText(" ") + 
				RichText(text);

			AddChat(&rich);
		}

		return;
	}

	else if(g_netmode == NETM_HOST)
	{
		LockCmd((PacketHeader*)sspp);
	}
	else if(g_netmode == NETM_SINGLE)
	{
		LockCmd((PacketHeader*)sspp);
	}
#endif
}

void ReadBuyPropPacket(BuyPropPacket* bpp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
#if 0
	if(bpp->bi < 0)
		return;

	if(bpp->bi >= BUILDINGS)
		return;
#endif

	if(bpp->pi < 0)
		return;

	if(bpp->pi >= PLAYERS)
		return;

	//TODO check buy only from right inst type: firm or state, depending on building type

	if(!from)
	{
		int* owner;
		bool* forsale;
		int* propprice;
		BlType* bt;
		char* tname;
		char hideview[16] = "";

		if(bpp->proptype >= PROP_BL_BEG &&
			bpp->proptype < PROP_BL_END)
		{
			if(bpp->propi < 0)
				return;

			if(bpp->propi >= BUILDINGS)
				return;
			
			Building* b;
			BlType* bt;
			b = &g_building[bpp->propi];
			owner = &b->owner;
			forsale = &b->forsale;
			propprice = &b->propprice;
			bt = &g_bltype[b->type];
			tname = bt->name;
			strcpy(hideview, "bl view");
		}
		else if(bpp->proptype >= PROP_U_BEG &&
			bpp->proptype < PROP_U_END)
		{
			if(bpp->propi < 0)
				return;

			if(bpp->propi >= UNITS)
				return;
			
			Unit* u;
			UType* ut;
			u = &g_unit[bpp->propi];
			owner = &u->owner;
			forsale = &u->forsale;
			propprice = &u->price;
			ut = &g_utype[u->type];
			tname = ut->name;
			strcpy(hideview, "truck mgr");
		}
		//TODO units
		//TODO conduits

		if(bpp->pi == *owner)
			return;

		Player* sellpy = &g_player[*owner];
		Player* buypy = &g_player[bpp->pi];

		RichText* sellname = &sellpy->name;
		RichText* buyname = &buypy->name;

		if(sellpy->client >= 0)
		{
			Client* c = &g_client[sellpy->client];
			sellname = &c->name;
		}

		if(buypy->client >= 0)
		{
			Client* c = &g_client[buypy->client];
			buyname = &c->name;
		}

		if(*propprice > buypy->global[RES_DOLLARS])
			return;

		*owner = bpp->pi;
		*forsale = false;

		buypy->global[RES_DOLLARS] -= *propprice;
		sellpy->global[RES_DOLLARS] += *propprice;

		Resource* r = &g_resource[RES_DOLLARS];

		char text[256];
		sprintf(text, "%s.", iform(*propprice).c_str());
		RichText rich = *buyname + 
			RichText(" ") + 
			STRTABLE[STR_BOUGHT] + 
			RichText(" ") + 
			RichText(tname) + 
			RichText(" ") + 
			STRTABLE[STR_FROM] + 
			RichText(" ") + 
			*sellname + 
			RichText(" ") + 
			STRTABLE[STR_FOR] + 
			RichText(" ") + 
			RichText(text);
		AddChat(&rich);

		GUI* gui = &g_gui;
		gui->hide(hideview);

		return;
	}

	else if(g_netmode == NETM_HOST)
	{
		LockCmd((PacketHeader*)bpp);
	}
	else if(g_netmode == NETM_SINGLE)
	{
		LockCmd((PacketHeader*)bpp);
	}
#endif
}

void ReadChatPacket(ChatPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER

	//InfoMess("chat", cp->msg);

	if(!nc)
		return;

	//if(cp->player >= PLAYERS)
	{
		//InfoMess("21","1");
		//char msg[128];
		//sprintf(msg, "%d", cp->player);
		//InfoMess("21", msg);
		//	return;
	}

	if(g_netmode == NETM_HOST)
	{
		if(cp->client < 0)
		{
			//InfoMess("21","2");
			return;
		}

		SendAll((char*)cp, sizeof(ChatPacket), true, false, &nc->addr);
	}
	else if(g_netmode == NETM_SINGLE)
	{
	}

	RichText name;

	if(cp->client >= 0)
	{
		Client* c = &g_client[cp->client];
		name = c->name + RichText(": ");
		//name.m_part.push_back(RichPart(": "));	//incorrect! can cause problems when appending to textblocks
	}

	unsigned int* utf32 = ToUTF32((unsigned char*)cp->msg);
	RichText chat = name + ParseTags(RichText(RichPart(utf32)), NULL);
	delete [] utf32;
	//InfoMess("chat2", chat.rawstr().c_str());
	AddChat(&chat);

	PlayClip(g_chat);

	//InfoMess("21","3");
#endif
}

void ReadPlaceBlPacket(PlaceBlPacket* pbp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!from)
	{
		if(CheckCanPlace(pbp->btype, pbp->tpos, -1))
		{
			PlaceBl(pbp->btype, pbp->tpos, false, pbp->player, NULL);

			BlType* bt = &g_bltype[pbp->btype];
			Player* py = &g_player[pbp->player];
			RichText name = py->name;
			if(py->client >= 0)
			{
				Client* c = &g_client[py->client];
				name = c->name;
			}
			RichText mess = name + RichText(" ") + STRTABLE[STR_PLACEDA] + RichText(" ") + RichText(bt->name);
			AddChat(&mess);
		}

		return;
	}

	if(nc && nc->isclient && g_netmode == NETM_HOST)
	{
#if 0
		PlaceBlPacket* pbp2 = (PlaceBlPacket*)malloc(sizeof(PlaceBlPacket));
		memcpy(pbp2, pbp, sizeof(PlaceBlPacket));
		g_nextnextcmd.push_back((PacketHeader*)pbp2);
#elif 0
		AppendCmd(&g_nextnextcmd, (PacketHeader*)pbp, sizeof(PlaceBlPacket));
#else
		LockCmd((PacketHeader*)pbp);
#endif
	}
#if 0	//no longer required, exec'd with NULL "from" addr
#if 1	//cl can only exec command batch packets, but it will then call this func
	else if(g_netmode == NETM_CLIENT)
	{
		//InfoMess("polk", "p");
#ifndef MATCHMAKER
		if(CheckCanPlace(pbp->btype, pbp->tpos))
			PlaceBl(pbp->btype, pbp->tpos, false, pbp->player, NULL);
#endif
	}
#endif
	else if(g_netmode == NETM_SINGLE)
	{
#if 0
		char msg[128];
		sprintf(msg, "%d at %d,%d", pbp->btype, pbp->tpos.x, pbp->tpos.y);
		InfoMess("polk", msg);
#endif
#ifndef MATCHMAKER
		if(CheckCanPlace(pbp->btype, pbp->tpos))
			PlaceBl(pbp->btype, pbp->tpos, false, pbp->player, NULL);
#endif
	}
#endif
#endif
}

void ReadMoveOrderPacket(MoveOrderPacket* mop, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER

	if(!from)
	{
		MoveOrder(mop);

		return;
	}

	if(nc && nc->isclient && g_netmode == NETM_HOST)
	{
		LockCmd((PacketHeader*)mop);
	}
#endif
}

void ReadPlaceCdPacket(PlaceCdPacket* pcp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER

	if(!from)
	{
		PlaceCd(pcp);

		return;
	}

	if(nc && nc->isclient && g_netmode == NETM_HOST)
	{
		LockCmd((PacketHeader*)pcp);
	}
#endif
}

//If we got a "no connection" packet while attempting to send
//data to a connection we have, reconnect to them, setting their
//lastrecvack to the one before our current nextsendack (?)
//Will that work? If we have outgoing packets. It should be the earliest
//outgoing packet nextsendack. But what if one ahead has been ack'd?
//Recvack will still be at the first one. If they have a connection (or buffered packet).
void ReadNoConnPacket(NoConnectionPacket* ncp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(!nc)
		nc = Match(from);

	if(!nc)
		return;	//Not our problem; some random stray packet from an intruder?

	//Otherwise, other side timed out and we reconnect

	//Reconnect(from);	//Might want to encapsulate following in this function later

#ifndef MATCHMAKER
	if(nc->ismatch)
	{
		g_sentsvinfo = false;
	}

	if(nc->isourhost)
	{
		Disconnect(nc);
		EndSess();
		RichText mess(STRTABLE[STR_HOSTDISC]);
		Mess(&mess);
		return;
	}
#endif

	if(nc->closed)
		return;

	//corpd fix
	//nc->closed = true;

	//also same thing as above...
	//check if we already called Disconnect on this connection
	//and have an outgoing DisconnectPacket
	for(auto pit=g_outgo.begin(); pit!=g_outgo.end(); pit++)
	{
		if(!Same(&pit->addr, &nc->addr))
			continue;

		PacketHeader* ph = (PacketHeader*)pit->buffer;

		if(ph->type != PACKET_DISCONNECT)
			continue;

		return;
	}

	//TODO get rid of faulty reconnect
	//Reconnect(from, nc);
	//FlushPrev(from);
	nc->closed = true;
}

void ReadClientLeftPacket(ClientLeftPacket* clp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		ResetCl(&g_client[clp->client]);
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		ResetCl(&g_client[clp->client]);

		ClientLeftPacket clp2;
		memcpy(&clp2, clp, sizeof(ClientLeftPacket));
		SendAll((char*)&clp2, sizeof(ClientLeftPacket), true, false, &nc->addr);
	}
	else
		return;

	Client* c = &g_client[clp->client];
	RichText msg = c->name + RichText(" ") + STRTABLE[STR_LEFTGAME];
	AddChat(&msg);
#endif
}

//cl disconnected by server
void ReadClDisconnectedPacket(ClDisconnectedPacket* cdp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#if 0
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		ResetCl(&g_client[cdp->client]);
	}
#endif

#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		ResetCl(&g_client[cdp->client]);
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		ResetCl(&g_client[cdp->client]);

		ClDisconnectedPacket cdp2;
		memcpy(&cdp2, cdp, sizeof(ClDisconnectedPacket));
		SendAll((char*)&cdp2, sizeof(ClDisconnectedPacket), true, false, &nc->addr);
	}
	else
		return;

	Client* c = &g_client[cdp->client];
	RichText msg;
	if(cdp->timeout)
		msg = c->name + RichText(" ") + STRTABLE[STR_TIMEDOUT];
	else
		msg = c->name + RichText(" ") + STRTABLE[STR_WASKICKED];
	AddChat(&msg);
#endif
}

void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	//char msg[128];
	//sprintf(msg, "dis %u:%u", from->host, (unsigned int)from->port);
	//InfoMess("d", msg);

	if(!nc)
		nc = Match(from);

	if(!nc)
		return;

	//if(nc->ctype == CONN_HOST)
	if(nc->isourhost)
	{
		g_svconn = NULL;
		//nc->isourhost = false;
#ifndef MATCHMAKER
		EndSess();
		RichText mess = STRTABLE[STR_HOSTDISC];
		Mess(&mess);
#endif
		//TODO message box to inform that host left the game and that game is over
	}
	//else if(nc->ctype == CONN_MATCHER)
	if(nc->ismatch)
	{
		g_mmconn = NULL;
		//nc->ismatch = false;
		g_sentsvinfo = false;
	}

#if 0
	//done further down
	if(g_netmode == NETM_HOST && nc->isclient)
	{
		//disconnect inform others? on the occasion of connection being closed by cl side
		ClientLeftPacket clp;
		clp.header.type = PACKET_CLIENTLEFT;
		clp.client = cli;
		//TODO processing for client left packet
		SendAll((char*)&clp, sizeof(ClientLeftPacket), true, false, &nc->addr);

		RichText msg = c->name + RichText(" left.");
		AddChat(&msg);

		//nc->isclient = false;
	}
#endif

	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(&*ci == nc)
		{
			ci->closed = true;
			FlushPrev(&ci->addr);

			if(dp->reply)
			{
				//FlushPrev(&ci->addr);
				//g_conn.erase(ci);
			}

			break;
		}

#ifndef MATCHMAKER
		//TODO get rid of client
		if(nc->client >= 0)
		{
			Client* c = &g_client[nc->client];

			if(g_netmode == NETM_HOST)
			{
				//inform other clients
				ClientLeftPacket clp;
				clp.header.type = PACKET_CLIENTLEFT;
				clp.client = nc->client;
				SendAll((char*)&clp, sizeof(ClientLeftPacket), true, false, &nc->addr);

				RichText msg = c->name + RichText(" left.");
				AddChat(&msg);
			}

			ResetCl(c);
		}
#endif

		nc->client = -1;

#if 1	//necessary, at least for the reason that
		//a game host fails to resend SvInfoPacket
		//the second time a client queries info.

#if 0
		for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
			if(&*ci == nc)
			{
				//erase
			}
#else
		//should be automatically erased from list,
		//without worry of being reused after this function
		//returns.
		//nc->closed = true;
		//already done above
#endif
#endif
}

void ReadChValPacket(ChValPacket* cvp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!from && !sock)
	{
		Building* b = NULL;
		Player* py = &g_player[cvp->player];
		CdTile* cdtile = NULL;
		Resource* r = NULL;
		BlType* bt = NULL;
		CdType* ct = NULL;
		UType* ut = NULL;
		CycleHist* lastch = NULL;

		RichText chat;
		RichText name = py->name;

		if(py->client >= 0)
		{
			Client* c = &g_client[py->client];
			name = c->name;
		}

		chat = name;
		char add[512];

		switch(cvp->chtype)
		{
			//TODO verify that player owns this
		case CHVAL_BLPRICE:
			b = &g_building[cvp->bi];
			b->price[cvp->res] = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			//sprintf(add, " ");
			//chat = chat + RichText(add);
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETPRICE];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " %s %s %s ", 
				STRTABLE[STR_AT].rawstr().c_str(),
				bt->name,
				STRTABLE[STR_TO].rawstr().c_str());
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			if(b->cyclehist.size())
			{
				lastch = &*b->cyclehist.rbegin();
				lastch->price[cvp->res] = cvp->value;
			}
			//TODO messages for the rest
			break;
		case CHVAL_BLWAGE:
			b = &g_building[cvp->bi];
			b->opwage = cvp->value;
#if 1
			//r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			//sprintf(add, " set wage");
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETWAGE];
			//chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " %s %s %s ", 
				STRTABLE[STR_AT].rawstr().c_str(),
				bt->name,
				STRTABLE[STR_TO].rawstr().c_str());
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			if(b->cyclehist.size())
			{
				lastch = &*b->cyclehist.rbegin();
				lastch->wage = cvp->value;
			}
			break;
		case CHVAL_CSTWAGE:
			b = &g_building[cvp->bi];
			b->conwage = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETCONWAGE];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_AT];
			chat = chat + RichText(" ");
			chat = chat + RichText(bt->name);
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_TO];
			chat = chat + RichText(" ");
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_TRPRICE:
			py->transpcost = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			//bt = &g_bltype[b->type];
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETTRANSPPR];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_TO];
			chat = chat + RichText(" ");
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_TRWAGE:
			py->truckwage = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			//bt = &g_bltype[b->type];
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETDRWAGE];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_TO];
			chat = chat + RichText(" ");
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_PRODLEV:
			b = &g_building[cvp->bi];
			b->prodlevel = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETPRODLEV];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_AT];
			chat = chat + RichText(" ");
			chat = chat + RichText(bt->name);
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_TO];
			chat = chat + RichText(" ");
			//r = &g_resource[RES_DOLLARS];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_CDWAGE:
			//TODO verify that player owns this
			cdtile = GetCd(cvp->cdtype, cvp->x, cvp->y, false);
			cdtile->conwage = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			ct = &g_cdtype[cvp->cdtype];
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETCONWAGE];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_AT];
			chat = chat + RichText(" ");
			chat = chat + RichText(ct->name);
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_TO];
			chat = chat + RichText(" ");
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_MANPRICE:
			b = &g_building[cvp->bi];
			b->manufprc[cvp->utype] = cvp->value;
			ut = &g_utype[cvp->utype];
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_SETMANPR];
			//chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_OF];
			chat = chat + RichText(" ");
			chat = chat + RichText(ut->name);
			chat = chat + RichText(" ");
			chat = chat + STRTABLE[STR_TO];
			chat = chat + RichText(" ");
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICH_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		default:
			break;
		};

		AddChat(&chat);

		return;
	}

	if(g_netmode == NETM_HOST)
	{
		if(!nc)
			return;
		if(!nc->isclient)
			return;
		if(nc->client < 0)
			return;

		Client* c = &g_client[nc->client];

		if(cvp->player != c->player)
			return;

		//AppendCmd(&g_nextnextcmd, (PacketHeader*)cvp, sizeof(ChValPacket));
		//TODO change to LockCmd?
		LockCmd((PacketHeader*)cvp);
	}
#endif
}

void ReadAddSvPacket(AddSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef MATCHMAKER
	if(!nc)
		return;

	nc->ishostinfo = true;
	//nc->svinfo = asp->svinfo;
	//nc->svinfo.addr = *from;

#if 0
	Log("addsv "<<nc->addr.port);

#endif

	AddedSvPacket asp2;
	asp2.header.type = PACKET_ADDEDSV;
	SendData((char*)&asp2, sizeof(AddedSvPacket), from, true, false, nc, &g_sock, 0, NULL);
#else
#endif
}

//cl reads a sv addr of game host
void ReadSvAddrPacket(SvAddrPacket* sap, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
#if 0
	//temp
	if(g_netmode == NETM_HOST)
		return;

	//InfoMess("g", "gsl");

	//temp
	if(g_svconn)
		return;

	Click_NewGame();

	//Connect(&sap->addr, false, true, false, true);
	Connect("localhost", PORT, false, true, false, true);
	BegSess();
#else

#if 0
	char msg[128];
	sprintf(msg, "rsap %u:%u", sap->addr.host, (unsigned int)sap->addr.port);
	InfoMess(msg, msg);
#endif

	if(!nc->ismatch)
		return;

	//we can get next host now
	g_reqdnexthost = false;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	auto sl = &v->m_svlist;

	bool found = false;

	for(auto sit=sl->begin(); sit!=sl->end(); sit++)
	{
		//if have, update info
		//if(!Same(&sit->addr, &sap->addr))
		//corpd fix
		if(Same(&sit->addr, &sap->addr))
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		//InfoMess("rsap", "rsap");

		SvList::SvInfo svinfo;
		svinfo.addr = sap->addr;
		svinfo.mapnamert = RichText("???");
		char name[128];
		//corpd fix
		sprintf(name, "ack%hu %u:%hu", sap->header.ack, htonl(sap->addr.host), htons(sap->addr.port));
		//sprintf(name, "%u:%hu", htonl(sap->addr.host), htons(sap->addr.port));
		svinfo.name = RichText(name);
		svinfo.pingrt = RichText("???");
		sl->push_back(svinfo);
		NetConn* havenc = Match(&sap->addr);
		Connect(&sap->addr, false, false, false, true);

		//InfoMess("gsv","gsvi?");

		if(havenc && havenc->handshook)
		{
			GetSvInfoPacket gsip;
			gsip.header.type = PACKET_GETSVINFO;
			SendData((char*)&gsip, sizeof(GetSvInfoPacket), &havenc->addr, true, false, nc, &g_sock, 0, NULL);
			//InfoMess("gsv","gsvi1");
		}
	}
#endif
#endif
}

void ReadGetSvInfoPacket(GetSvInfoPacket* gsip, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode != NETM_HOST)
		return;

	SvInfoPacket sip;
	sip.header.type = PACKET_SVINFO;
	//sip.svinfo.addr = {0};	//?
	strcpy(sip.svinfo.mapname, g_mapname);
	strcpy(sip.svinfo.svname, g_svname);
	sip.svinfo.mapname[MAPNAME_LEN] = 0;
	sip.svinfo.svname[SVNAME_LEN] = 0;
	//sip.svinfo.nplayers = g_nplayers;	//TO DO

	int npys = 0;
	int nmaxpys = PLAYERS;	//TODO make adjustable

	for(int ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_client[ci];

		if(!c->on)
			continue;

		npys++;
	}

	sip.svinfo.nplayers = npys;
	sip.svinfo.maxpys = nmaxpys;
	sip.svinfo.started = (bool)(g_mode == APPMODE_PLAY);

	SendData((char*)&sip, sizeof(SvInfoPacket), from, true, false, nc, &g_sock, 0, NULL);
#endif
}

#ifndef MATCHMAKER
bool ComparePings(SvList::SvInfo& a, SvList::SvInfo& b)
{
	return a.ping < b.ping;
}
#endif

void ReadSvInfoPacket(SvInfoPacket* sip, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!nc)
		return;

	if(!nc->ishostinfo)
		return;

	//InfoMess("rgsip", "rgsip");
	//InfoMess("rgsip", sip->svinfo.svname);
	//Log("rgsip ack"<<sip->header.ack);

	//check if we already have this addr
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	auto sl = &v->m_svlist;

	bool found = false;

	SvList::SvInfo* sinfo;

	for(auto sit=sl->begin(); sit!=sl->end(); sit++)
	{
		//if have, update info
		if(!Same(&sit->addr, from))	//self-address might be different on LAN
			//if(!Same(&sit->addr, &sip->svinfo.addr))	//how do we know self-address?
				continue;

		found = true;
		sinfo = &*sit;
		break;
	}

	//might happen if we're on LAN since we don't get an SvAddrPacket from matchmaker
	if(!found)
	{
		SvList::SvInfo svinfo;
		svinfo.addr = *from;
		svinfo.mapnamert = RichText("???");
		svinfo.name = RichText("???");
		svinfo.pingrt = RichText("???");
		sl->push_back(svinfo);

		sinfo = &*sl->rbegin();

		found = true;
	}

	if(found)
	{
		sinfo->replied = true;

		sip->svinfo.mapname[MAPNAME_LEN] = 0;
		sip->svinfo.svname[SVNAME_LEN] = 0;

		sinfo->nplayers = sip->svinfo.nplayers;
		sinfo->nmaxpys = sip->svinfo.maxpys;

		char pysstr[32];
		sprintf(pysstr, "%d/%d", sip->svinfo.nplayers, sip->svinfo.maxpys);
		sinfo->nplayersrt = RichText(pysstr);

		char pingstr[16];
		sprintf(pingstr, "%d", (int)nc->ping);
		sinfo->pingrt = RichText(pingstr);

#if 1
		//yes unicode?
		//unsigned int* mapnameuni = ToUTF32((const unsigned char*)sip->svinfo.mapname, strlen(sip->svinfo.mapname));
		//unsigned int* svnameuni = ToUTF32((const unsigned char*)sip->svinfo.svname, strlen(sip->svinfo.svname));
		unsigned int* mapnameuni = ToUTF32((const unsigned char*)sip->svinfo.mapname);
		unsigned int* svnameuni = ToUTF32((const unsigned char*)sip->svinfo.svname);
		sinfo->mapnamert = RichText(UStr(mapnameuni));
		sinfo->name = RichText(UStr(svnameuni));
		sinfo->name = ParseTags(sinfo->name, NULL);
		delete [] mapnameuni;
		delete [] svnameuni;
#else
		//no unicode?
		//sit->mapnamert = sip->svinfo.mapname;
		//sit->name = sip->svinfo.svname;
#endif

		//break;	//multiple copies?

		sinfo->started = sip->svinfo.started;

		if(sip->svinfo.started)
		{
			//sinfo->name = STRTABLE[STR_STARTED] + RichText(" ") + sinfo->name;
		}

	}

	//return;	//temp

	sl->sort(ComparePings);

	//if it's only a hostinfo and nc isn't closed
	if(!nc->closed && !nc->isourhost && !nc->isclient && !nc->ismatch)
		Disconnect(nc);
#endif
}

void ReadGetSvListPacket(GetSvListPacket* gslp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef MATCHMAKER
	if(!nc)
		return;

	//if(!nc->isclient)
	//	return;

	nc->svlistoff = 0;

	//Log("req sv l");
	//

	ReadSendNextHostPacket(NULL, nc, from, sock);

	//Log("/req sv l");
	//
#endif
}

void ReadSendNextHostPacket(SendNextHostPacket* snhp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef MATCHMAKER
	//Log("sendnexthost1 g_conn.size()="<<g_conn.size());
	//

	if(!nc)
		nc = Match(from);

	//Log("sendnexthost2");
	//

	if(!nc)
		return;

	//Log("sendnexthost3");
	//

	int hin = -1;
	auto hit = g_conn.begin();
	while(hit != g_conn.end())
	{
		if(hit->ishostinfo)
		{
			hin++;

			//Log("hin "<<hin<<" svlistoff "<<nc->svlistoff);

			if(hin == nc->svlistoff)
			{
				SvAddrPacket sap;
				sap.header.type = PACKET_SVADDR;
				sap.addr = hit->addr;
				SendData((char*)&sap, sizeof(SvAddrPacket), from, true, false, nc, &g_sock, 0, NULL);
				nc->svlistoff++;
				return;
			}
		}

		hit++;
	}

	NoMoreHostsPacket nmhp;
	nmhp.header.type = PACKET_NOMOREHOSTS;
	SendData((char*)&nmhp, sizeof(NoMoreHostsPacket), from, true, false, nc, &g_sock, 0, NULL);
#endif
}

void ReadNoMoreHostsPacket(NoMoreHostsPacket* nmhp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	g_reqsvlist = false;

	if(g_netmode != NETM_HOST &&
		g_mmconn)
		Disconnect(g_mmconn);

#if 0
	char msg[128];
	sprintf(msg, "nmh");
	InfoMess(msg, msg);
#endif
#endif
}

void ReadAddedSvPacket(AddedSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	//InfoMess("nmh", "nmh sv");

	if(!nc)
		return;

	if(!nc->ismatch)
		return;

	if(g_netmode != NETM_HOST)
		return;

	g_sentsvinfo = true;
	//InfoMess("added", "added sv");
#endif
}

void ReadWrongVersionPacket(WrongVersionPacket* wvp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		Disconnect(g_svconn);
		EndSess();
		char cmess[256];
		char v1[32], v2[32];
		VerStr(VERSION, v1);
		VerStr(wvp->correct, v2);
		sprintf(cmess, " (%s, %s).", v1, v2);
		RichText mess = STRTABLE[STR_ERVERMATCH] + RichText(cmess);
		Mess(&mess);
	}
#endif
}

void ReadJoinPacket(JoinPacket* jp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_HOST)
	{
		if(!nc)
			return;

		if(jp->version != VERSION)
		{
			WrongVersionPacket wvp;
			wvp.header.type = PACKET_WRONGVERSION;
			wvp.correct = VERSION;
			SendData((char*)&wvp, sizeof(WrongVersionPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
			return;
		}

		//is it too late to join? (has the game already started?)
		if(g_mode == APPMODE_PLAY)
		{
			GameStartedPacket gsp;
			gsp.header.type = PACKET_GAMESTARTED;
			SendData((char*)&gsp, sizeof(GameStartedPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
			return;
		}

		//we set this to indicate to include this NetConn in SendAll's
		nc->isclient = true;

		//InfoMess("conn", "read join");
		//TO DO send join info map etc.

		RichText name;
		unsigned int* uname = ToUTF32((unsigned char*)jp->name);
		name.m_part.push_back(UStr(uname));
		delete [] uname;
		int joinci;

		//InfoMess(" ? mcp", " ? mcp");

		//unsigned int ipaddr = SDL_SwapBE32(ip.host);
		//unsigned short port = SDL_SwapBE16(ip.port);
#if 0
		char ipname[128];
		sprintf(ipname, "%u:%u", SDL_SwapBE32(nc->addr.host), (unsigned int)SDL_SwapBE16(nc->addr.port));
		name = RichText(ipname);
#endif

		if(!AddClient(nc, name, &joinci))
		{
			TooManyClPacket tmcp;
			tmcp.header.type = PACKET_TOOMANYCL;
			SendData((char*)&tmcp, sizeof(TooManyClPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
			return;
		}

		int msdelay = RESEND_DELAY;

		MapChangePacket mcp;
		mcp.header.type = PACKET_MAPCHANGE;
		strcpy(mcp.map, g_mapname);
		mcp.checksum = g_mapcheck;
		SendData((char*)&mcp, sizeof(MapChangePacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
		msdelay += RESEND_DELAY;

		AddClientPacket acp;
		acp.header.type = PACKET_ADDCLIENT;
		//acp.client = joinci;
		//strcpy(acp.name, jp->name);
		//acp.player = -1;
		for(int i=0; i<CLIENTS; i++)
		{
			Client* c = &g_client[i];

			if(!c->on)
				continue;

			if(i == g_localC)
				acp.ishost = true;
			else
				acp.ishost = false;

			RichText* cname = &c->name;
			acp.client = i;

			if(cname->m_part.size() > 0)
			{
				//unsigned char* name8 = ToUTF8(cname->m_part.begin()->m_text.m_data);
				//name8[PYNAME_LEN] = 0;
				//strcpy(acp.name, (char*)name8);
				//delete [] name8;
				std::string name8 = cname->rawstr();
				strcpy(acp.name, name8.c_str());
			}
			else
				strcpy(acp.name, "");

			acp.player = c->player;
			acp.ready = c->ready;

			SendData((char*)&acp, sizeof(AddClientPacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
			msdelay += RESEND_DELAY;

			//is this the joined client? send his info to everyone else
			if(i == joinci)
			{
				SendAll((char*)&acp, sizeof(AddClientPacket), true, false, &nc->addr);
			}
		}

		SelfClientPacket scp;
		scp.header.type = PACKET_SELFCLIENT;
		scp.client = joinci;
		SendData((char*)&scp, sizeof(SelfClientPacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
		msdelay += RESEND_DELAY;

		DoneJoinPacket djp;
		djp.header.type = PACKET_DONEJOIN;
		SendData((char*)&djp, sizeof(DoneJoinPacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
		msdelay += RESEND_DELAY;
	}
#endif
}

void ReadAddClPacket(AddClientPacket* acp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		NetConn* cnc = NULL;
		if(acp->ishost)
			cnc = nc;

		acp->name[PYNAME_LEN] = 0;
		//Log("acp");
		unsigned int* uname = ToUTF32((unsigned char*)acp->name);
		//int addci;
		//AddClient(cnc, RichText(UStr(uname)), &addci);
		AddClient(cnc, RichText(UStr(uname)), acp->client);
		//Client* c = &g_client[addci];
		Client* c = &g_client[acp->client];
		c->player = acp->player;
		c->ready = acp->ready;

		if(acp->player >= 0)
		{
			Player* py = &g_player[acp->player];
			//py->client = addci;
			py->client = acp->client;
		}

		GUI* gui = &g_gui;
		ViewLayer* join = (ViewLayer*)gui->get("join");
		if(join->m_opened)
		{
			Text* status = (Text*)join->get("status");
			char state[MAX_PATH+1];
			sprintf(state, "Got client #%d, player #%d, ", acp->client, acp->player);
			//status->m_text = RichText(RichPart(state)) + RichText(RichPart(UStr(uname)));
			status->m_text = RichText(state) + RichText(UStr(uname));
		}

		delete [] uname;
	}
	else if(g_netmode == NETM_HOST)
	{
	}
#endif
}

void ReadSelfClPacket(SelfClientPacket* scp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
#ifndef MATCHMAKER
		//Log("scp");
		g_localC = scp->client;
		Client* c = &g_client[scp->client];
		g_localP = c->player;
		c->name = g_name;

		GUI* gui = &g_gui;
		ViewLayer* join = (ViewLayer*)gui->get("join");
		if(join->m_opened)
		{
			Text* status = (Text*)join->get("status");
			char state[MAX_PATH+1];
			sprintf(state, "Got self client #%d, player #%d", scp->client, g_localP);
			status->m_text = RichText(state);
		}
#endif
	}
}

void ReadSetClNamePacket(SetClNamePacket* scnp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		Client* c = &g_client[scnp->client];
		unsigned int* uname = ToUTF32((unsigned char*)scnp->name);
		c->name = RichText(UStr(uname));
		delete [] uname;
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		Client* c = &g_client[scnp->client];
		unsigned int* uname = ToUTF32((unsigned char*)scnp->name);
		c->name = RichText(UStr(uname));
		delete [] uname;

		SetClNamePacket scnp2;
		memcpy(&scnp2, scnp, sizeof(SetClNamePacket));
		SendAll((char*)&scnp2, sizeof(SetClNamePacket), true, false, &nc->addr);
	}
#endif
}

void ReadClientRolePacket(ClientRolePacket* crp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		//Log("crp");
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		//TO DO reject if another client controls crp->player

		ClientRolePacket crp2;
		memcpy(&crp2, crp, sizeof(ClientRolePacket));
		//SendAll((char*)&crp2, sizeof(ClientRolePacket), true, false, &nc->addr);
		SendAll((char*)&crp2, sizeof(ClientRolePacket), true, false, NULL);
	}
	else
		return;

	Client* c = &g_client[crp->client];
	c->on = true;	//corpc fix

	if(c->player >= 0)
	{
		Player* py = &g_player[c->player];
		py->client = -1;
		c->player = -1;
	}

	c->player = crp->player;
	Player* py = &g_player[crp->player];
	py->client = crp->client;
	py->on = true;

	if(crp->client == g_localC)
		g_localP = crp->player;
#endif
}

void ReadDoneJoinPacket(DoneJoinPacket* djp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
#ifndef MATCHMAKER
		//Log("djp");
		GUI* gui = &g_gui;
		gui->hideall();
		gui->show("lobby");
		gui->show("chat");
		//gui->show("role");
		((RoleView*)gui->get("role"))->regen();
		Lobby_Regen();

		//return;

		int pi = NewClPlayer();

		if(pi < 0)
		{
			//InfoMess("Error", "No player slots left.");
			EndSess();
		}

		ClientRolePacket crp;
		crp.header.type = PACKET_CLIENTROLE;
		crp.client = g_localC;
		crp.player = pi;
		SendData((char*)&crp, sizeof(ClientRolePacket), &g_svconn->addr, true, false, g_svconn, &g_sock, 0, NULL);
#endif
	}
}

void ReadTooManyClPacket(TooManyClPacket* tmcp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
#ifndef MATCHMAKER

		Disconnect(nc);
		GUI* gui = &g_gui;
#if 0
		gui->hideall();
		gui->show("menu");
#else
		ViewLayer* v = (ViewLayer*)gui->get("join");
		Text* status = (Text*)v->get("status");
		status->m_text = STRTABLE[STR_ERSVFULL];
#endif

		//TO DO info message

#endif
	}
}

void ReadMapChPacket(MapChangePacket* mcp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		//Log("mcp");
		//
		strcpy(g_mapname, mcp->map);
		FreeMap();
		//TO DO load, download etc. check sum

		GUI* gui = &g_gui;
		ViewLayer* join = (ViewLayer*)gui->get("join");
		if(join->m_opened)
		{
			Text* status = (Text*)join->get("status");
			char state[MAX_PATH+1];
			sprintf(state, "Got map: %s", mcp->map);
			status->m_text = RichText(state);
		}

		char maprelative[MAX_PATH+1];
		sprintf(maprelative, "maps/%s", mcp->map);
		bool status = LoadMap(maprelative);

#if 0
			char cmess[1024];
			sprintf(cmess, "mapch %s", maprelative);
			RichText mess = RichText(cmess);
			Mess(&mess);
#endif

		if(!status)
		{
			EndSess();
			char cmess[1024];
			sprintf(cmess, "%s %s", STRTABLE[STR_ERLOADMAP].rawstr().c_str(), maprelative);
			RichText mess = RichText(cmess);
			Mess(&mess);
			return;
		}

		if(g_mapcheck != mcp->checksum)
		{
			EndSess();
			char cmess[1024];
			sprintf(cmess, "%s %s (%u,%u)", STRTABLE[STR_ERMAPMATCH].rawstr().c_str(), maprelative, g_mapcheck, mcp->checksum);
			RichText mess = RichText(cmess);
			Mess(&mess);
			return;
		}
	}
#endif
}

void ReadMapStartPacket(MapStartPacket* msp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		BegSess();
		g_mode = APPMODE_PLAY;
		GUI* gui = &g_gui;
		gui->hideall();
		gui->show("play");
		gui->show("chat");
	}
#endif
}

void ReadGameStartedPacket(GameStartedPacket* gsp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		Disconnect(nc);
		EndSess();
		RichText mess = STRTABLE[STR_ERGAMEST];
		Mess(&mess);
		return;
	}
#endif
}

//connect packet won't be discarded if it's a (reply's or otherwise) copy, so this function needs to be durable.
//i.e., no repeat action if cp->header.ack is PastAck(...);.
void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#if 0
	char msg[128];
	sprintf(msg, "\tcon %u:%u reply=%d", from->host, (unsigned int)from->port, (int)cp->reply);
	Log(msg);
	//InfoMess("d", msg);
#endif

	//InfoMess("d", "rc");

#if 1	//flush all previous incoming and outgoing packets from this addr
	//FlushPrev(from);
	//actually, that might be bad, if we've got a game host we're playing in,
	//and we request a sv list and get connected to this same host to get its
	//game info. actually, just need to be make sure Connect(); doesn't send
	//another ConnectPacket if we've already handshook.
#endif

	bool isnew = false;

	if(!nc)
	{
		nc = Match(from);

#if 0	//corpd fix
		if( ( nc && ( nc->ismatch || nc == g_mmconn ) )
			&& g_netmode == NETM_HOST )
		{
			nc->ismatch = true;
			g_mmconn = nc;
			g_sentsvinfo = false;
		}
#endif

		if(!nc)
		{

			//char msg[128];
			//sprintf(msg, "con %u:%u", from->host, (unsigned int)from->port);
			//InfoMess("d", msg);

			isnew = true;
			NetConn newnc;
			//newnc.ctype = CONN_CLIENT;
			//temporary - must get some packet telling us this is client that wants to join TO DO
			//newnc.isclient = true;
			newnc.addr = *from;
			newnc.handshook = true;
			newnc.lastrecvack = cp->header.ack;
			newnc.nextsendack = 0;
			newnc.lastrecv = GetTicks();
			newnc.closed = false;
			g_conn.push_back(newnc);
			nc = &*g_conn.rbegin();

#if 0
			{
				auto ci1 = g_conn.begin();
				auto ci2 = g_conn.rbegin();

				if(g_conn.size() > 1 &&
					ci1->addr.host == ci2->addr.host &&
					ci1->addr.port == ci2->addr.port)
				{
					char msg[128];
					sprintf(msg, "mult c same at f%s, l%d", __FILE__, __LINE__);
					InfoMess("e", msg);
				}
			}
#endif

#if 0	//now done by ack
			ConnectPacket replycp;
			replycp.header.type = PACKET_CONNECT;
			//replycp.header.ack = 0;
			//nc->nextsendack = 0;
			replycp.reply = true;
			SendData((char*)&replycp, sizeof(ConnectPacket), from, true, false, nc, &g_sock, 0);

			//temp
			//g_canturn = true;
#endif

			//return;
		}
	}

	//corpd fix
	if( nc && ( nc->ismatch || nc == g_mmconn ) )
	{
		nc->ismatch = true;
		g_mmconn = nc;
		g_sentsvinfo = false;
	}

	nc->handshook = true;
	nc->closed = false;
	//nc->nextsendack = 0;

#if 0	//for ack to work
	nc->lastrecvack = cp->header.ack;
	nc->nextsendack = 0;
#else
	//nc->lastrecvack = cp->header.ack;
	if(isnew)
	{
#if 0
		if(cp->reconnect)
		{
			//nc->lastrecvack = PrevAck(cp->yournextrecvack);
			nc->lastrecvack = cp->yourlastrecvack;
			nc->nextsendack = NextAck(cp->yourlastsendack);
			//cp.yournextrecvack = nc->nextsendack;
			//cp.yourlastsendack = nc->lastrecvack;
		}
		else
#endif
		{
			nc->lastrecvack = cp->header.ack;
			nc->nextsendack = 0;
		}
	}
	else
	{
		FlushPrev(&nc->addr);

#if 0
		if(cp->reconnect)
		{
			//if(!PastAck(PrevAck(cp->yournextrecvack), nc->lastrecvack))
			//	nc->lastrecvack = PrevAck(cp->yournextrecvack);
			if(!PastAck(cp->yourlastrecvack, nc->lastrecvack))
				nc->lastrecvack = cp->yourlastrecvack;
		}
		else
#endif
		{
			FlushPrev(&nc->addr);
			nc->lastrecvack = cp->header.ack;
			nc->nextsendack = 0;
		}
	}
#endif

#if 0	//now done by ack
	//we already have a connection to them,
	//so they must have lost theirs if this isn't a reply to ours.
	if(!cp->reply)
	{
		//this is probably a copy since we already have a connection
		//(or else they might have closed their connection and reconnected).
		//we need to check if we already have an outgoing reply ConnectPacket.

		bool outgoing = false;

		for(auto pit=g_outgo.begin(); pit!=g_outgo.end(); pit++)
		{
			if(!Same(&pit->addr, from))
				continue;

			PacketHeader* ph = (PacketHeader*)pit->buffer;

			if(ph->type != PACKET_CONNECT)
				continue;

#if 1	//necessary to know if it's a reply=true?
			ConnectPacket* oldcp = (ConnectPacket*)pit->buffer;

			if(!oldcp->reply)
				continue;
#endif

			outgoing = true;
			break;
		}

		if(!outgoing)
		{
			FlushPrev(from);

			nc->lastrecvack = cp->header.ack;

			ConnectPacket replycp;
			replycp.header.type = PACKET_CONNECT;
			replycp.header.ack = 0;
			//nc->nextsendack = 1;
			replycp.reply = true;
			SendData((char*)&replycp, sizeof(ConnectPacket), from, true, false, nc, &g_sock, 0);
		}
	}

	//temp
	//g_canturn = true;
#endif

	//we got this in reply to a ConnectPacket sent?

#ifndef MATCHMAKER
#if 0	//now done in OnAck_Connect
	else
	{
		//is this a reply copy?
		if(PastAck(cp->header.ack, nc->lastrecvack))
			return;	//if so, discard, because we've already dealt with a previous copy

		//update lastrecvack since TranslatePacket won't do it for a ConnectPacket
		nc->lastrecvack = cp->header.ack;

		//if(nc->ctype == CONN_HOST)
		if(nc->isourhost)
		{
			g_svconn = nc;

			//InfoMess("conn", "conn to our host");

			//TO DO request data, get ping, whatever, server info

			//g_canturn = true;
			//
			//char msg[128];
			//sprintf(msg, "send join to %u:%u aka %u:%u", from->host, (unsigned int)from->port, nc->addr.host, (unsigned int)nc->addr.port);
			//InfoMess("j", msg);

			JoinPacket jp;
			jp.header.type = PACKET_JOIN;
			std::string name = g_name.rawstr();
			if(name.length() >= PYNAME_LEN)
				name[PYNAME_LEN] = 0;
			strcpy(jp.name, name.c_str());
			SendData((char*)&jp, sizeof(JoinPacket), from, true, false, nc, &g_sock, 0);
		}
		//else if(nc->ctype == CONN_MATCHER)

		if(nc->ishostinfo)
		{
			//TO DO request data, get ping, whatever, server info
			GetSvInfoPacket gsip;
			gsip.header.type = PACKET_GETSVINFO;
			SendData((char*)&gsip, sizeof(GetSvInfoPacket), from, true, false, nc, &g_sock, 0);
		}

		if(nc->ismatch)
		{
			//InfoMess("got mm", "got mm");
			//Log("got mm");
			//
			g_mmconn = nc;
			g_sentsvinfo = false;

			if(g_reqsvlist && !g_reqdnexthost)
			{
				//Log("got mm send f svl");
				//

				//g_reqdsvlist = true;
				//g_needsvlist = false;
				g_reqdnexthost = true;

				GetSvListPacket gslp;
				gslp.header.type = PACKET_GETSVLIST;
				SendData((char*)&gslp, sizeof(GetSvListPacket), &nc->addr, true, false, nc, &g_sock, 0);
				//InfoMess("sglp", "sglp");
			}
		}
	}
#endif
#else
#endif
}

void OnAck_Disconnect(OldPacket* p, NetConn* nc)
{
	if(!nc)
		return;

	nc->closed = true;	//to be cleaned up this or next frame

#if 0	//done in CheckConns
	if(nc == g_svconn)
		g_svconn = NULL;
	if(nc == g_)
#endif
}

//on connect packed ack'd
void OnAck_Connect(OldPacket* p, NetConn* nc)
{
	if(!nc)
		nc = Match(&p->addr);

	if(!nc)
		return;

	nc->handshook = true;

	ConnectPacket* scp = (ConnectPacket*)p->buffer;

	//if(!scp->reconnect)
	{
#ifndef MATCHMAKER
		GUI* gui = &g_gui;

		//if(nc->ctype == CONN_HOST)
		if(nc->isourhost /* && g_mode == APPMODE_JOINING */)
		{
			g_svconn = nc;

			//InfoMess("conn", "conn to our host");

			//TO DO request data, get ping, whatever, server info

			//g_canturn = true;
			//
			//char msg[128];
			//sprintf(msg, "send join to %u:%u aka %u:%u", from->host, (unsigned int)from->port, nc->addr.host, (unsigned int)nc->addr.port);
			//InfoMess("j", msg);

			JoinPacket jp;
			jp.header.type = PACKET_JOIN;
			std::string name = g_name.rawstr();
			if(name.length() >= PYNAME_LEN)
				name[PYNAME_LEN] = 0;
			strcpy(jp.name, name.c_str());
			jp.version = VERSION;
			SendData((char*)&jp, sizeof(JoinPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
		}
		//else if(nc->ctype == CONN_MATCHER)
#endif

#ifdef MATCHMAKER
		if(nc->isclient)
		{

		}
#endif

		if(nc->ishostinfo)
		{
			//TO DO request data, get ping, whatever, server info
			GetSvInfoPacket gsip;
			gsip.header.type = PACKET_GETSVINFO;
			SendData((char*)&gsip, sizeof(GetSvInfoPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
			//InfoMess("gsv","gsvi2");
		}

#ifndef MATCHMAKER
		if(nc->ismatch)
		{
			//InfoMess("got mm", "got mm");
			//Log("got mm");
			//
			g_mmconn = nc;
			g_sentsvinfo = false;

#if 0
			char msg[128];
			sprintf(msg, "onackmm");
			InfoMess(msg, msg);
#endif

			if(g_reqsvlist && !g_reqdnexthost)
			{

#if 0
				char msg[128];
				sprintf(msg, "reqnexth");
				InfoMess(msg, msg);
#endif
				//Log("got mm send f svl");
				//

				//g_reqdsvlist = true;
				//g_needsvlist = false;
				g_reqdnexthost = true;

				GetSvListPacket gslp;
				gslp.header.type = PACKET_GETSVLIST;
				SendData((char*)&gslp, sizeof(GetSvListPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
				//InfoMess("sglp", "sglp");
			}
		}
#endif
	}
}
