


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





#ifndef PACKETS_H
#define PACKETS_H

#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../math/vec2i.h"
#include "../math/vec2uc.h"
#include "../math/camera.h"

class NetConn;

class OldPacket
{
public:
	char* buffer;
	int len;
	unsigned long long last;	//last time resent
	unsigned long long first;	//first time sent
	bool expires;
	bool acked;	//used for outgoing packets

	//sender/reciever
	IPaddress addr;
	void (*onackfunc)(OldPacket* op, NetConn* nc);

	void freemem()
	{
		if(len <= 0)
			return;

		if(buffer != NULL)
			delete [] buffer;
		buffer = NULL;
	}

	OldPacket()
	{
		len = 0;
		buffer = NULL;
		onackfunc = NULL;
		acked = false;
	}
	~OldPacket()
	{
		freemem();
	}

	OldPacket(const OldPacket& original)
	{
		len = 0;
		buffer = NULL;
		*this = original;
	}

	OldPacket& operator=(const OldPacket &original)
	{
		//corpdfix
		freemem();

		if(original.buffer && original.len > 0)
		{
			len = original.len;
			if(len > 0)
			{
				buffer = new char[len];
				memcpy((void*)buffer, (void*)original.buffer, len);
			}
			last = original.last;
			first = original.first;
			expires = original.expires;
			acked = original.acked;
			addr = original.addr;
			onackfunc = original.onackfunc;
#ifdef MATCHMAKER
			//ipaddr = original.ipaddr;
			//port = original.port;
			//memcpy((void*)&addr, (void*)&original.addr, sizeof(struct sockaddr_in));
#endif
		}
		else
		{
			buffer = NULL;
			len = 0;
			onackfunc = NULL;
		}

		return *this;
	}
};

//TODO merge some of these into multi-purpose packet types
//TODO separate protocol/control packets from user/command packets

#define	PACKET_NULL						0
#define PACKET_DISCONNECT				1
#define PACKET_CONNECT					2
#define	PACKET_ACKNOWLEDGMENT			3
#define PACKET_PLACEBL					4
#define PACKET_NETTURN					5
#define PACKET_DONETURN					6
#define PACKET_JOIN						7
#define PACKET_ADDSV					8
#define PACKET_ADDEDSV					9
#define PACKET_KEEPALIVE				10
#define PACKET_GETSVLIST				11
#define PACKET_SVADDR					12
#define PACKET_SVINFO					13
#define PACKET_GETSVINFO				14
#define PACKET_SENDNEXTHOST				15
#define PACKET_NOMOREHOSTS				16
#define PACKET_ADDCLIENT				17
#define PACKET_SELFCLIENT				18
#define PACKET_SETCLNAME				19
#define PACKET_CLIENTLEFT				20
#define PACKET_CLIENTROLE				21
#define PACKET_DONEJOIN					22
#define PACKET_TOOMANYCL				23
#define PACKET_MAPCHANGE				24
#define PACKET_CHVAL					25
#define PACKET_CLDISCONNECTED			26
#define PACKET_CLSTATE					27
#define PACKET_NOCONN					28
#define PACKET_ORDERMAN					29
#define PACKET_CHAT						30
#define PACKET_MAPSTART					31
#define PACKET_GAMESTARTED				32
#define PACKET_WRONGVERSION				33
#define PACKET_MOVEORDER				34
#define PACKET_PLACECD					35
#define PACKET_NACK						36
#define PACKET_LANCALL					37
#define PACKET_LANANSWER				38
#define PACKET_SETSALEPROP				39
#define PACKET_BUYPROP					40

// byte-align structures
#pragma pack(push, 1)

struct PacketHeader
{
	unsigned short type;
	unsigned short ack;
};

struct BasePacket
{
	PacketHeader header;
};

typedef BasePacket NoConnectionPacket;
typedef BasePacket DoneJoinPacket;
typedef BasePacket TooManyClPacket;
typedef BasePacket SendNextHostPacket;
typedef BasePacket NoMoreHostsPacket;
typedef BasePacket GetSvInfoPacket;
typedef BasePacket GetSvListPacket;
typedef BasePacket KeepAlivePacket;
typedef BasePacket AddSvPacket;
typedef BasePacket AddedSvPacket;
typedef BasePacket AckPacket;
typedef BasePacket NAckPacket;
typedef BasePacket MapStartPacket;
typedef BasePacket GameStartedPacket;
typedef BasePacket LANCallPacket;
typedef BasePacket LANAnswerPacket;

//set property selling state
//TODO set selling conduits
struct SetSalePropPacket
{
	PacketHeader header;
	int propi;
	bool selling;	//TODO custom ecbool typedef
	int price;
	int proptype;
	signed char tx;
	signed char ty;
	//int pi;	//should match owner at the time. necessary?
};

#define PROP_BL_BEG		0
#define PROP_BL_END		(BL_TYPES)
#define PROP_CD_BEG		(PROP_BL_END)
#define PROP_CD_END		(PROP_CD_BEG+CD_TYPES)
#define PROP_U_BEG		(PROP_CD_END)
#define PROP_U_END		(PROP_U_BEG+UNIT_TYPES)

//TODO sell off and set for jobs for individual trucks and individual deals apart from auto manager
//buy property
//TODO implement buying conduits
//TODO drop-down tree view for all buildings and conduits
//TODO selection box for conduits
struct BuyPropPacket
{
	PacketHeader header;
	int propi;
	int pi;
	int proptype;
	signed char tx;
	signed char ty;
};

//TODO order demolish property and conduit

//variable length depending on number of tiles involved
struct PlaceCdPacket
{
	PacketHeader header;
	short player;
	unsigned char cdtype;
	short ntiles;
	Vec2uc place[0];
};

//variable length depending on number of units ordered
struct MoveOrderPacket
{
	PacketHeader header;
	Vec2i mapgoal;
	unsigned short nunits;
	unsigned short units[0];
};

//order unit manufacture at building
struct OrderManPacket
{
	PacketHeader header;
	int utype;
	int player;
	int bi;
};

#define MAX_CHAT			711

struct ChatPacket
{
	PacketHeader header;
	int client;
	char msg[MAX_CHAT+1];
};

#define CLCH_UNRESP			0	//client became unresponsive
#define CLCH_RESP			1	//became responsive again
#define CLCH_PING			2
#define CLCH_READY			3	//client became ready to start
#define CLCH_NOTREADY		4	//client became not ready
#define CLCH_PAUSE			5	//pause speed
#define CLCH_PLAY			6	//normal play speed
#define CLCH_FAST			7	//fast forward speed

struct ClStatePacket
{
	PacketHeader header;
	unsigned char chtype;
	short client;
	float ping;
};

/*
The difference between client left
and disconnected is that ClDisc*
means it was done by server (kicked? timed out?)
*/

struct ClientLeftPacket
{
	PacketHeader header;
	short client;
};

struct ClDisconnectedPacket
{
	PacketHeader header;
	short client;
	bool timeout;
};

#define CHVAL_BLPRICE					0
#define CHVAL_BLWAGE					1
#define CHVAL_TRWAGE					2
#define CHVAL_TRPRICE					3
#define CHVAL_CSTWAGE					4
#define CHVAL_PRODLEV					5
#define CHVAL_CDWAGE					6
#define CHVAL_MANPRICE					7

struct ChValPacket
{
	PacketHeader header;
	unsigned char chtype;
	int value;
	unsigned char player;
	unsigned char res;
	unsigned short bi;
	unsigned char x;
	unsigned char y;
	unsigned char cdtype;
	unsigned char utype;
};

//not counting null terminator
#define MAPNAME_LEN		127
#define SVNAME_LEN		63
#define PYNAME_LEN		63

struct MapChangePacket
{
	PacketHeader header;
	unsigned int checksum;
	char map[MAPNAME_LEN+1];
};

struct JoinPacket
{
	PacketHeader header;
	unsigned int version;
	char name[PYNAME_LEN+1];
};

struct WrongVersionPacket
{
	PacketHeader header;
	unsigned int correct;
};

struct AddClientPacket
{
	PacketHeader header;
	signed char client;
	signed char player;
	char name[PYNAME_LEN+1];
	bool ishost;
	bool ready;
};

struct SelfClientPacket
{
	PacketHeader header;
	int client;
};

struct SetClNamePacket
{
	PacketHeader header;
	int client;
	char name[PYNAME_LEN+1];
};

struct ClientRolePacket
{
	PacketHeader header;
	signed char client;
	signed char player;
};

struct SvAddrPacket
{
	PacketHeader header;
	IPaddress addr;
};

class SendSvInfo	//sendable
{
public:
	IPaddress addr;
	char svname[SVNAME_LEN+1];
	short nplayers;
	short maxpys;
	char mapname[MAPNAME_LEN+1];
	bool started;
};

struct SvInfoPacket
{
	PacketHeader header;
	SendSvInfo svinfo;
};

struct NetTurnPacket
{
	PacketHeader header;
	/*
	Is this the net frame upon which it is delivered?
	Or the frame when this will be executed?
	Let's make it the frame that will be executed.
	*/
	unsigned long long fornetfr;	//for net fr #..
	unsigned short loadsz;
	//commands go after
};

struct DoneTurnPacket
{
	PacketHeader header;
	unsigned long long curnetfr;	//current net fr #.. (start of cl's current turn)
	short client;	//should match sender
};

struct PlaceBlPacket
{
	PacketHeader header;
	int btype;
	Vec2i tpos;
	int player;
};

struct ConnectPacket
{
	PacketHeader header;
	bool reconnect;
	unsigned short yourlastrecvack;
	unsigned short yournextrecvack;
	unsigned short yourlastsendack;
};

struct DisconnectPacket
{
	PacketHeader header;
	bool reply;
};

// Default alignment
#pragma pack(pop)

#endif



