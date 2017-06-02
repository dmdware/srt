
#include "buyprop.h"
#include "selection.h"
#include "player.h"
#include "building.h"
#include "bltype.h"
#include "unit.h"
#include "utype.h"
#include "conduit.h"
#include "../gui/gui.h"
#include "../platform.h"
#include "../gui/layouts/chattext.h"
#include "../net/lockstep.h"

void Click_BuyProp()
{
	Selection* sel = &g_sel;
	Player* py = &g_player[g_localP];
	bool owned = false;	//owned by current player?
	Player* opy;
	int propi;
	int propprice;
	int proptype;

	//TODO units
	//TODO conduits

	if(sel->buildings.size())
	{
		BlType* bt = NULL;
		Building* b = NULL;

		propi = *sel->buildings.begin();
		b = &g_building[propi];
		bt = &g_bltype[b->type];

		if(b->owner == g_localP)
			owned = true;

		if(!b->forsale)
			return;

		proptype = PROP_BL_BEG + b->type;
		propprice = b->propprice;

#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
				(int)b->transporter[RES_URANIUM],
				(int)g_unit[b->transporter[RES_URANIUM]].mode,
				(int)g_unit[b->transporter[RES_URANIUM]].target,
				bi,
				(int)g_unit[b->transporter[RES_URANIUM]].targtype,
				(int)g_unit[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif

		opy = &g_player[b->owner];
	}
	if(sel->units.size())
	{
		Unit* u = NULL;

		propi = *sel->units.begin();
		u = &g_unit[propi];

		if(u->owner == g_localP)
			owned = true;

		if(!u->forsale)
			return;

		proptype = PROP_U_BEG + u->type;
		propprice = u->price;
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i tpos = *sel->roads.begin();
		RoadTile* road = RoadAt(tpos.x, tpos.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i tpos = *sel->powls.begin();
		PowlTile* powl = PowlAt(tpos.x, tpos.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		qty = sel->crpipes.size();
		conmat = g_crpipecost;
		Vec2i tpos = *sel->crpipes.begin();
		CrPipeTile* crpipe = CrPipeAt(tpos.x, tpos.y);
		maxcost = crpipe->maxcost;
	}
#endif

	if(py->global[RES_DOLLARS] < propprice)
	{
		RichText textr = RichText("You have insufficient funds to buy this property.");
		AddChat(&textr);
		return;
	}

	BuyPropPacket bpp;
	bpp.header.type = PACKET_BUYPROP;
	bpp.proptype = proptype;
	bpp.propi = propi;
	bpp.pi = g_localP;
	bpp.tx = -1;
	bpp.ty = -1;
	LockCmd((PacketHeader*)&bpp);
}