


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



#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../sim/build.h"
#include "../sim/simdef.h"
#include "../econ/utility.h"
#include "../sim/unit.h"
#include "../sim/simflow.h"
#include "../net/lockstep.h"
#include "../gui/layouts/chattext.h"
#include "../path/tilepath.h"
#include "../sim/conduit.h"
#include "../net/packets.h"
#include "../gui/widgets/spez/gengraphs.h"

void UpdAI()
{
#if 0
	//protectionism
	//for each state
	for(int i=0; i<PLAYERS; i+=(FIRMSPERSTATE+1))
	{
		Player* st = &g_player[i];
		if(!st->protectionism)
			continue;
		int mainfirm = i+1;
		Player* firm = &g_player[mainfirm];
		int excessfunds = st->global[RES_DOLLARS] - INST_FUNDS;
		excessfunds = imax(0, excessfunds);
		//subsidies
		firm->global[RES_DOLLARS] += excessfunds;
		st->global[RES_DOLLARS] -= excessfunds;
	}
#endif

	//return;	//do nothing for now

	//only host updates AI and issues commands
	if(g_netmode != NETM_HOST &&
		g_netmode != NETM_SINGLE)
		return;

	bool caldem1 = false;

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;

		if(g_simframe - p->lastthink < AI_FRAMES)
			continue;

		p->lastthink = g_simframe;

		UpdAI(p);
	}
}

/*
Building proportion number ai build rate

Houses 2 other 1 part

Trial and error wages and prices adjust

Rule if population decreasing then lower prices and increase wages
*/

void UpdAI(Player* p)
{
#define BUILD_FRAMES		(SIM_FRAME_RATE*60*1);
	unsigned long long blphase = g_simframe / BUILD_FRAMES;
	int pi = p - g_player;

	if((blphase+pi+g_simframe) % PLAYERS == 1)
		AIBuild(p);

	if((blphase+pi+g_simframe) % PLAYERS == 2)
		AIManuf(p);
	
	AdjProd(p);
	
	if((blphase+pi+g_simframe) % PLAYERS == 3)
		BuyProps(p);

#if 0
	for(int ui=0; ui<UNITS; ui++)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->owner != p-g_player)
			continue;

		if(u->forsale)
			continue;

		SetSalePropPacket sspp;
		sspp.header.type = PACKET_SETSALEPROP;
		sspp.propi = ui;
		sspp.proptype = PROP_U_BEG + u->type;
		sspp.selling = true;
		sspp.price = 0;
		LockCmd((PacketHeader*)&sspp);
	}
#endif

	//done in tryprod:
	//AdjPrWg(p);

	//RichText msg = RichText("UpdAI");
	//AddChat(&msg);
}

void AIBuyProp(Player* py, int propi, int proptype)
{
	int pyi = py - g_player;

	BuyPropPacket bpp;
	bpp.header.type = PACKET_BUYPROP;
	bpp.pi = pyi;
	bpp.proptype = proptype;
	bpp.propi = propi;
	LockCmd((PacketHeader*)&bpp);
}

void BuyProps(Player* p)
{
	int pi = p - g_player;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		//cse fix
		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		if(!b->forsale)
			continue;

		if(b->owner == pi)
			continue;

		if(b->propprice <= 0)
		{
			AIBuyProp(p, bi, PROP_BL_BEG + b->type);
			continue;
		}

		if(b->cyclehist.size() < 2)
			continue;
		
		CycleHist* lastch = &*(b->cyclehist.rbegin());
		CycleHist* lastch2 = &*(b->cyclehist.rbegin()--);

		int profit = lastch->prod[RES_DOLLARS] + 
			lastch2->prod[RES_DOLLARS] - 
			lastch->cons[RES_DOLLARS] -
			lastch2->cons[RES_DOLLARS];

		if((10 + profit) * 2 < b->propprice)
			continue;
		
		AIBuyProp(p, bi, PROP_BL_BEG + b->type);
	}

	for(int ui=0; ui<UNITS; ui++)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(!u->forsale)
			continue;

		if(u->owner == pi)
			continue;

		if(u->price <= 0 ||
			u->price <= (p->transpcost - p->truckwage) * 12)
		{
			AIBuyProp(p, ui, PROP_U_BEG + u->type);
		}
	}
}

int CountB()
{
	int c = 0;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		c ++;
	}

	return c;
}

void AdjPrWg(Player* p, Building* b)
{
	int bi = b - g_building;
	int pi = p - g_player;

	if(b->cyclehist.size() < 3)
		return;
	
	//the completed cycle is [end-1], because tryprod() that calls this
	//func pushes a new one for the beginning of the new cycle.
	CycleHist* lastch = &*(b->cyclehist.rbegin()--);
	CycleHist* lastch2 = &*(b->cyclehist.rbegin()----);
	
	//total profits (might be negative)
	int profit = lastch->prod[RES_DOLLARS] - lastch->cons[RES_DOLLARS];
	int profit2 = lastch2->prod[RES_DOLLARS] - lastch2->cons[RES_DOLLARS];
	int totprof = profit + profit2;
	//earnings by resource
	int rearn[RESOURCES];
	int rearn2[RESOURCES];
	Zero(rearn);
	Zero(rearn2);
	//price changes by resource
	int rprch[RESOURCES];
	Zero(rprch);
	int haverch = -1;
	//wage change
	int wch;
	//list of resources this bltype produces
	std::list<int> prr;

	wch = lastch->wage - lastch2->wage;

#if 0
	GUI* gui = &g_gui;
	GenGraphs* gg = (GenGraphs*)gui->get("gen graphs");
	Graph* g = &gg->
#endif

	int nlab = CountU(UNIT_LABOURER);
	int nbl = CountB();
	int nlabreq = nbl * 30 / BL_TYPES;
	bool subreqlab = nlab < nlabreq;

	BlType* bt = &g_bltype[b->type];

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] > 0)
			prr.push_back(ri);

		rearn[ri] = lastch->cons[ri] * lastch->price[ri];
		rearn2[ri] = lastch2->cons[ri] * lastch2->price[ri];
		rprch[ri] = lastch->price[ri] - lastch2->price[ri];

		if(rprch[ri] != 0)
			haverch = ri;
	}

#if 0
	/*
	The general idea is to try adjusting price up,
	then the next cycle down, for the same resource,
	and then move on to the next resource if that fails.
	Then move on to the wage.
	*/

	//did we change wage?
	if(wch != 0)
	{
		//need more money?
		if(totprof < 0)
		{

		}
		//can spend more money?
		else if(totprof > 0)
		{
		}
	}


	//did we change resource price?
	if(haverch >= 0)
	{
	}
#endif

	/*
	Too complicated
	Plan 2
	Just decrease wage and increase all the prices if need more money
	And opposite if can spend
	*/

	//positive reinforcement?
	bool positive = false;
	
	//if(totprof > 0)
	//if(profit > 0)
	//did we make more profit this cycle than the cycle before?
	if(profit > profit2)
	{
		if(p->global[RES_DOLLARS] > INST_FUNDS*3/4)
		//was the net profit of the last two cycle positive?
		//if(totprof > 0)
			positive = false;
		else if(p->global[RES_DOLLARS] < INST_FUNDS*3/4)
		//was the net profit of the last two cycle negative?
		//if(totprof < 0)
			positive = true;
	}
	//else if(totprof < 0)
	//else if(profit < 0)
	//did we make less profit this cycle than the cycle before?
	else if(profit < profit2)
	{
		if(p->global[RES_DOLLARS] > INST_FUNDS*3/4)
		//if(totprof > 0)
			positive = true;
		else if(p->global[RES_DOLLARS] < INST_FUNDS*3/4)
		//if(totprof < 0)
			positive = false;
	}

	/*
	If positive, do the same change but more.
	And if negative, try in the opposite direction, but half.
	*/
	int sign = 1;

	if(!positive)
		sign = -1;

	unsigned int simrand = g_simframe * 1103515245 + 12345;
	simrand ^= bi;

	//wage adjustment...
	bool dowch = false;
	int nextw;

	//if there was a wage change
	if(wch != 0)
	{
		nextw = b->opwage + sign * wch * 10 / 9;
		dowch = true;
	}

	//extra rule for negative reinforcement: try changing one of the prices or 
	//the wage in the opposite direction, because there are many factors
	//if(!positive && (g_simframe+pi+bi) % (1+0) == 0)
	if(!positive && simrand % (1+(int)prr.size()) == 0)
	{
		if(iabs(wch) > 4)
			nextw = b->opwage - sign * b->opwage / 2;
		else if(wch != 0)
			nextw = b->opwage - sign * b->opwage * 4;
		else
			nextw = b->opwage - sign * 4;

		dowch = true;
	}

	if(dowch)
	{
		//a rule so the workers don't die off
		if(subreqlab)
		{
			int chamt = iabs( b->opwage - nextw );
			nextw = b->opwage + chamt * 3;
		}

		if(nextw < 10)
			nextw = 10;
		else if(nextw > INST_FUNDS / 100)
			nextw = INST_FUNDS / 100;

		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_BLWAGE;
		cvp.player = pi;
		cvp.bi = bi;
		cvp.value = nextw;
		LockCmd((PacketHeader*)&cvp);
	}

#if 0
		char m[128];
		sprintf(m, "prr %d,%d", simrand % (1+(int)prr.size()), (int)prr.size());
		RichText r = RichText(m);
		AddChat(&r);
#endif

	//price adjustments...
	int riti = 0;
	for(auto rit=prr.begin(); rit!=prr.end(); rit++, riti++)
	{
		int ri = *rit;

#if 0
		char m[128];
		sprintf(m, "r %d,%d", simrand % (1+(int)prr.size()), riti+1);
		RichText r = RichText(m);
		AddChat(&r);
#endif

		bool doprch = false;
		int nextrpr;

		if(rprch[ri] != 0)
		{
			nextrpr = b->price[ri] + sign * rprch[ri] * 10 / 9;
			doprch = true;
		}

		//extra rule for negative reinforcement: try changing one of the prices or 
		//the wage in the opposite direction, because there are many factors
		//if(!positive && (g_simframe+pi+bi) % (1+(int)prr.size()) == riti)
		if(!positive && simrand % (2+(int)prr.size()) == riti+1)
		{
			if(iabs(rprch[ri]) > 4)
				nextrpr = b->price[ri] - sign * b->price[ri] / 2;
			else if(rprch[ri] != 0)
				nextrpr = b->price[ri] - sign * b->price[ri] * 4;
			else
				nextrpr = b->price[ri] - sign * 4;

			doprch = true;
		}

		if(doprch)
		{
			//a rule so the workers don't die off
			if(subreqlab)
			{
				int chamt = iabs( b->price[ri] - nextrpr );
				nextrpr = b->price[ri] - chamt * 3;
			}

			if(nextrpr < 10)
				nextrpr = 10;
			else if(nextrpr > INST_FUNDS / 100)
				nextrpr = INST_FUNDS / 100;

			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLPRICE;
			cvp.player = pi;
			cvp.bi = bi;
			cvp.value = nextrpr;
			cvp.res = ri;
			LockCmd((PacketHeader*)&cvp);
		}
	}

	//INST_FUNDS;
}

void AdjProd(Player* p)
{
	int pi = p - g_player;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		BlType* bt = &g_bltype[b->type];
		bool toomuch = true;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			if(b->stocked[ri] < bt->output[ri] * 2)
			{
				toomuch = false;
				break;
			}
		}

		if( (toomuch && b->prodlevel == 0) ||
			(!toomuch && b->prodlevel == RATIO_DENOM) )
			continue;

		int value = 0;

		if(!toomuch)
			value = RATIO_DENOM;

		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_PRODLEV;
		cvp.bi = bi;
		cvp.value = value;
		cvp.player = pi;
		LockCmd((PacketHeader*)&cvp);
	}
}

void Manuf(int pi, int uti)
{
	std::list<int> manufers;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		//TODO enemey check
		//TODO visibility check

		BlType* bt = &g_bltype[b->type];

		for(auto mt=bt->manuf.begin(); mt!=bt->manuf.end(); mt++)
		{
			if(*mt == uti)
			{
				manufers.push_back(bi);
				break;
			}
		}
	}

	if(!manufers.size())
		return;

	//chosen manuf index
	int cmi = g_simframe % manufers.size();
	int mi = 0;
	auto mit = manufers.begin();

	for(; mit!=manufers.end(), mi<cmi; mit++, mi++)
	{
	}

	OrderManPacket omp;
	omp.header.type = PACKET_ORDERMAN;
	omp.player = pi;
	omp.bi = *mit;
	omp.utype = uti;
	LockCmd((PacketHeader*)&omp);
}

void AIManuf(Player* p)
{
	int bcount = 0;
	int pi = p - g_player;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		bcount++;
	}

	int ucount[UNIT_TYPES];
	memset(ucount, 0, sizeof(int) * UNIT_TYPES);

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->owner != pi)
			continue;

		ucount[u->type]++;
	}

	for(int uti=0; uti<UNIT_TYPES; uti++)
	{
		UType* ut = &g_utype[uti];

		int needed = ut->prop * bcount / RATIO_DENOM;
		int tomanuf = needed - ucount[uti];

		if(tomanuf < 0)
			continue;

		for(int j=0; j<tomanuf; j++)
		{
			Manuf(pi, uti);
		}
	}
}

/*
To calc what bl next and where

For each bl on map multiply by 15 and decrease by radius from spot of bl

Spot with greatest num of all bl types is densest
Will be next spot where no bl placed

Then multiply bl proportion rate num by 15 also and subtract dense tile's num for all bl's of that type, and choose the bltype with greatest
For eg house proportion is 3 and theres only 1 gives 2

15 is some distance num for max city clustering distance that should effect interaction
*/
void AIBuild(Player* p)
{
	//RichText msg = RichText("AIBuild");
	//AddChat(&msg);

	int pi = p - g_player;

	//check for any unfinished bl's
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		//if(b->owner != pi)
		//	continue;

		if(!b->finished)
			return;
	}

#define DIST_MULT		30

	int* bldensity = (int*)malloc(sizeof(int) * g_mapsz.x * g_mapsz.y);
	memset(bldensity, 0, sizeof(int) * g_mapsz.x * g_mapsz.y);

	//TODO only get density for visible tiles, so that each player builds own base

	//calc density
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		BlType* bt = &g_bltype[b->type];
		Vec2i tstart = b->tpos;
		Vec2i tmin, tmax;

		tmin.x = tstart.x - DIST_MULT;
		tmin.y = tstart.y - DIST_MULT;
		tmax.x = tstart.x + DIST_MULT;
		tmax.y = tstart.y + DIST_MULT;
		tmin.x = imax(0, tmin.x);
		tmin.y = imax(0, tmin.y);
		tmax.x = imin(g_mapsz.x-1, tmax.x);
		tmax.y = imin(g_mapsz.y-1, tmax.y);

		for(int tx=tmin.x; tx<=tmax.x; tx++)
			for(int ty=tmin.y; ty<=tmax.y; ty++)
			{
				Vec2i tspot;
				tspot.x = tx;
				tspot.y = ty;
				int tdist = Magnitude(tspot - tstart);
				int dens = DIST_MULT - tdist;

				if(dens <= 0)
					continue;

				int* spoti = &bldensity[ tx + ty * g_mapsz.x ];
				(*spoti) += dens;
			}
	}

	//add unit's density
	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		Vec2i tstart = u->cmpos / TILE_SIZE;

		int* spoti = &bldensity[ tstart.x + tstart.y * g_mapsz.x ];
		(*spoti) += 1;
	}

	Vec2i highspot;
	int high = -1;

	//get highest density spot
	for(int tx=0; tx<g_mapsz.x; tx++)
	{
		for(int ty=0; ty<g_mapsz.y; ty++)
		{	
			int* spoti = &bldensity[ tx + ty * g_mapsz.x ];

			if(*spoti < high)
				continue;

			high = *spoti;
			highspot.x = tx;
			highspot.y = ty;
		}
	}

	free(bldensity);

	int highscore = 0;
	int highbti = -1;

	//calc bltype to build
	for(int bti=0; bti<BL_TYPES; bti++)
	{
		BlType* bt = &g_bltype[bti];
		int score = 0;
		//int maxcontrib = DIST_MULT * bt->prop;

		//go through each bl of that type
		for(int bi=0; bi<BUILDINGS; bi++)
		{
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(b->type != bti)
				continue;

			Vec2i tstart = b->tpos;

			int tdist = Magnitude(highspot - tstart);
			tdist = imin(tdist, DIST_MULT);
			int dens = DIST_MULT - tdist;
			dens = imax(0, dens);

			//add contribution of that bl for this tile
			score -= dens;
		}

		if(bt->prop == 0)
			continue;

		score /= bt->prop;

		if(score < highscore &&
			highbti >= 0)
			continue;

		highscore = score;
		highbti = bti;
	}

	if(highbti < 0)
		return;

	//check if proportional to workers

	int bcount = 0;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		//if(b->owner != pi)
		//	continue;

		if(b->type != highbti)
			continue;

		bcount++;
	}

	int wcnt = CountU(UNIT_LABOURER);
	BlType* bt = &g_bltype[highbti];

	//RATIO_DENOM * 45
	if(wcnt < bcount * bt->wprop / RATIO_DENOM)
		return;

	//char msg[128];
	//sprintf(msg, "w b %d, %d", wcnt, bcount);
	//RichText rm = RichText(msg);
	//AddChat(&rm);

	//find a place to build
	Vec2i tplace;
	if(!PlaceBAb(highbti, highspot, &tplace))
	{
		p->lastthink += CYCLE_FRAMES*4;
		return;
	}

	PlaceBlPacket pbp;
	pbp.header.type = PACKET_PLACEBL;
	pbp.btype = highbti;
	pbp.player = pi;
	pbp.tpos = tplace;
	LockCmd((PacketHeader*)&pbp);

	//connect conduits

	for(int ctype=0; ctype<CD_TYPES; ctype++)
	{
		bool needed = false;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			Resource* r = &g_resource[ri];

			if(r->conduit != ctype)
				continue;

			if(bt->input[ri] > 0)
			{
				needed = true;
				break;
			}

			if(bt->output[ri] > 0)
			{
				needed = true;
				break;
			}
		}

		if(!needed)
			continue;

		ConnectCd(p, ctype, tplace);
#if 1
		PlotCd(pi, ctype, tplace, tplace + Vec2i(bt->width.x,0));
		PlotCd(pi, ctype, tplace, tplace - Vec2i(bt->width.x,0));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(0,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace - Vec2i(0,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(bt->width.x,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace - Vec2i(bt->width.x,bt->width.y));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(bt->width.x,-bt->width.y));
		PlotCd(pi, ctype, tplace, tplace + Vec2i(-bt->width.x,bt->width.y));
#endif
	}
}

void PlotCd(int pi, int ctype, Vec2i from, Vec2i to)
{
	from.x = imin(from.x, g_mapsz.x-1);
	from.y = imin(from.y, g_mapsz.y-1);
	to.x = imin(to.x, g_mapsz.x-1);
	to.y = imin(to.y, g_mapsz.y-1);
	from.x = imax(from.x, 0);
	from.y = imax(from.y, 0);
	to.x = imax(to.x, 0);
	to.y = imax(to.y, 0);

	std::list<Vec2s> places;

#if 1
	TilePath(UNIT_LABOURER, UMODE_NONE, from.x*TILE_SIZE + TILE_SIZE/2, from.y*TILE_SIZE + TILE_SIZE/2,
		-1, -1, TARG_NONE,
		ctype, &places, NULL, NULL, NULL,
		to.x*TILE_SIZE + TILE_SIZE/2, to.y*TILE_SIZE + TILE_SIZE/2, 
		to.x*TILE_SIZE, to.y*TILE_SIZE, to.x*TILE_SIZE + TILE_SIZE - 1, to.y*TILE_SIZE + TILE_SIZE - 1,
		g_mapsz.x * g_mapsz.y / 2, true);
#endif

#if 0
	int d = Magnitude(from - to);

	for(int di=0; di<d; di++)
	{
		Vec2s cur = Vec2s(from.x,from.y) + Vec2s(to.x,to.y) * 10 * di / d;
		places.push_back(cur);
	}

	places.push_back(Vec2s(to.x,to.y));
#endif

	if(!places.size())
		return;

	PlaceCdPacket* pcp = (PlaceCdPacket*)malloc(
		sizeof(PlaceCdPacket) + 
		sizeof(Vec2uc)*(short)places.size() );
	pcp->header.type = PACKET_PLACECD;
	pcp->cdtype = ctype;
	pcp->player = pi;
	pcp->ntiles = (short)places.size();

	auto pit = places.begin();
	for(short pin = 0; pit != places.end(); pit++, pin++)
		pcp->place[pin] = Vec2uc(pit->x, pit->y);

	LockCmd((PacketHeader*)pcp);

	free(pcp);
}

void ConnectCd(Player* p, int ctype, Vec2i tplace)
{
	Vec2i nearest;
	int neard = -1;

	for(int tx=0; tx<g_mapsz.x; tx++)
	{
		for(int ty=0; ty<g_mapsz.y; ty++)
		{
			CdTile* ctile = GetCd(ctype, tx, ty, false);

			if(!ctile->on)
				continue;

			Vec2i to = Vec2i(tx, ty);
			int d = Magnitude(to - tplace);

			if(d < neard ||
				neard < 0)
			{
				neard = d;
				nearest = to;
			}
		}
	}

	int pi = p - g_player;
	Vec2i to = nearest;

	//not found a target?
	if(neard < 0)
	{
		to = tplace;

		unsigned int dx = g_simframe % 5;
		unsigned int dy = (g_simframe + 11) % 5;

		to.x += dx - 2;
		to.y += dy - 2;

		to.x = imax(0, to.x);
		to.y = imax(0, to.y);
		to.x = imin(g_mapsz.x-1, to.x);
		to.y = imin(g_mapsz.y-1, to.y);
	}

	PlotCd(pi, ctype, tplace, to);
}