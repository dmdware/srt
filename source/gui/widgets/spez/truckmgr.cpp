


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



#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../layouts/gviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "truckmgr.h"
#include "blpreview.h"
#include "blview.h"
#include "cstrview.h"
#include "../../icon.h"
#include "../../../math/fixmath.h"
#include "../../../net/lockstep.h"
#include "../../../net/client.h"
#include "../../../language.h"
#include "../../../sim/buyprop.h"

void Click_TM_Close()
{
	GUI* gui = &g_gui;
	TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
	tm->hide();
}

void Click_TM_Set()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
	Selection* sel = &g_sel;

	int row = 0;

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* dw = (EditBox*)tm->get(wn);
	std::string sval = dw->m_value.rawstr();

	if(sval.length())
	{
		int ival;
		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		//ival = imin(ival, RATIO_DENOM);
		ival = imax(ival, 0);
		//py->truckwage = ival;
		if(ival != py->truckwage)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_TRWAGE;
			cvp.player = g_localP;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp);
		}
	}
	
	row++;

	sprintf(wn, "%d 1", row);
	dw = (EditBox*)tm->get(wn);
	sval = dw->m_value.rawstr();

	if(sval.length())
	{
		int ival;
		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		//ival = imin(ival, RATIO_DENOM);
		ival = imax(ival, 0);
		//py->transpcost = ival;
		if(ival != py->transpcost)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_TRPRICE;
			cvp.player = g_localP;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp);
		}
	}

	row++;
	
	sprintf(wn, "%d 0", row);

	CheckBox* cb = (CheckBox*)tm->get(wn);
	bool forsale = cb->m_selected;

	sprintf(wn, "%d 1", row);
	dw = (EditBox*)tm->get(wn);
	sval = dw->m_value.rawstr();
	
	if(sval.length())
	{
		int ival;
		sscanf(sval.c_str(), "%d", &ival);

		ival = imax(ival, 0);
		//py->transpcost = ival;
		if(ival != py->transpcost)
		{
			SetSalePropPacket sspp;
			sspp.header.type = PACKET_SETSALEPROP;
			sspp.price = ival;
			sspp.selling = forsale;
			sspp.proptype = PROP_U_BEG + UNIT_TRUCK;
			
			for(int ui=0; ui<UNITS; ui++)
			{
				Unit* u = &g_unit[ui];

				if(!u->on)
					continue;

				if(u->type != UNIT_TRUCK)
					continue;

				if(u->owner != g_localP)
					continue;

				sspp.propi = ui;
				LockCmd((PacketHeader*)&sspp);
			}
		}
	}

}

TruckMgr::TruckMgr(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_TRUCKMGR;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void TruckMgr::regen(Selection* sel)
{
	RichText uname;
	int wage;
	int cost;

	Player* py = &g_player[g_localP];
	UType* ut;
	Unit* u;
	bool owned = true;	//owned by current player?
	Player* opy;
	int ntruck = CountU(UNIT_TRUCK, g_localP);
	bool forsale = false;
	int price = 0;

	if(sel->units.size() > 0)
	{
		int ui = *sel->units.begin();
		u = &g_unit[ui];
		ut = &g_utype[u->type];

		if(u->owner == g_localP)
			owned = true;
		else
			owned = false;

		forsale = u->forsale;
		price = u->price;

		//g_bptype = b->type;
		opy = &g_player[u->owner];
		wage = opy->truckwage;
		cost = opy->transpcost;

		uname = RichText(UStr(ut->name));
	}
	else
	{
		owned = true;
		opy = &g_player[g_localP];
		wage = opy->truckwage;
		cost = opy->transpcost;
		forsale = false;
	}

	freech();
    
    RichText ownname = opy->name;
    if(opy->client >= 0)
    {
        Client* c = &g_client[opy->client];
        ownname = c->name;
    }

	//add(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	add(new Text(this, "owner", ownname, MAINFONT16, Resize_BP_Ow, true, opy->color[0], opy->color[1], opy->color[2], opy->color[3]));
	add(new Text(this, "title", STRTABLE[STR_TRUCKMGR], MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int row = 0;

	if(owned && !ntruck)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);

		add(new TextBlock(this, rowname, STRTABLE[STR_DONTHAVETR], MAINFONT16, Resize_BV_Cl_w, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		goto endbut;
		return;
	}

	{
		Resource* r = &g_resource[RES_DOLLARS];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		add(new Text(this, rowname, STRTABLE[STR_DRWAGE] + RichText(UStr(" ")) + RichText(RichPart(RICH_ICON, r->icon)) + RichText(UStr("/")) + STRTABLE[STR_SECOND] + RichText(UStr(":")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", opy->truckwage);

		if(owned)
			add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 6, NULL /*Change_TM_DW*/, NULL, -1));
		else
			add(new Text(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	{
		Resource* r = &g_resource[RES_DOLLARS];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		add(new Text(this, rowname, STRTABLE[STR_TRANSPCOST] + RichText(UStr(" ")) + RichText(RichPart(RICH_ICON, r->icon)) + RichText(UStr("/")) + STRTABLE[STR_SECOND] + RichText(UStr(":")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", opy->transpcost);

		if(owned)
			add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 6, NULL /*Change_TM_TC*/, NULL, -1));
		else
			add(new Text(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	{
		Resource* r = &g_resource[RES_DOLLARS];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		if(owned && !ntruck)
		{
			price = 1;

			for(int ui=0; ui<UNITS; ui++)
			{
				Unit* u = &g_unit[ui];

				if(!u->on)
					continue;

				if(u->owner != g_localP)
					continue;

				if(u->type != UNIT_TRUCK)
					continue;

				forsale = u->forsale;
				price = u->price;
				break;
			}
		}

		if(owned)
			add(new CheckBox(this, rowname, STRTABLE[STR_FORSALE] + RichText(UStr(" ")) + RichText(RichPart(RICH_ICON, r->icon)), MAINFONT16, Resize_BV_Cl, forsale, 1.0f,1.0f,1.0f,1.0f )); 
		else if(forsale)
			add(new Text(this, rowname, STRTABLE[STR_FORSALE] + RichText(UStr(" ")) + RichText(RichPart(RICH_ICON, r->icon)), MAINFONT16, Resize_BV_Cl, 1.0f,1.0f,1.0f,1.0f ));
		
		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", price);

		//TODO pop out for sale price editbox when clicking on checkbox, and don't show checkbox if no trucks owned, and just say no trucks owned

		if(owned)
			add(new EditBox(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl, false, 6, NULL /*Change_TM_TC*/, NULL, -1));
		else if(forsale)
			add(new Text(this, rowname, RichText(UStr(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	//TO DO
	endbut:

	char rowname[32];
	
	{
		if(!ntruck && owned)
			row+=2;

		sprintf(rowname, "%d %d", row, 0);
		add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_CLOSE], RichText(), MAINFONT16, BUST_LINEBASED, Resize_CV_Cl_2, Click_TM_Close, NULL, NULL, NULL, NULL, -1, NULL));
	}

	if(ntruck && owned)
	{
		sprintf(rowname, "%d %d", row, 1);
		add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_SET], RichText(), MAINFONT16, BUST_LINEBASED, Resize_CV_Cl_2, Click_TM_Set, NULL, NULL, NULL, NULL, -1, NULL));
	}
	else if(!owned && forsale)
	{
		sprintf(rowname, "%d %d", row, 0);
		add(new Button(this, rowname, "gui/transp.png", STRTABLE[STR_BUYPROP], RichText(), MAINFONT16, BUST_LINEBASED, Resize_CV_Cl_2, Click_BuyProp, NULL, NULL, NULL, NULL, -1, NULL));
	}

	row++;

	reframe();
}
