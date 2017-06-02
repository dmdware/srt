


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


#include "simflow.h"
#include "evqueue.h"
#include "../net/client.h"
#include "unit.h"

std::list<SimEv> g_simev;

void PlanEvent(unsigned short type, unsigned long long execframe, Unit* u)
{
	SimEv se;

	se.evtype = type;
	se.planframe = g_simframe;
	se.execframe = execframe;

	auto sit=g_simev.begin();
	bool place = false;

	while(true)
	{
		if(sit == g_simev.end())
		{
			place = true;
		}
		else
		{
			SimEv* spotse = &*sit;

			if(spotse->execframe > execframe)
				place = true;
		}

		if(!place)
			sit++;

		g_simev.insert(sit, se);
		break;
	}
}

void ExecEvent(SimEv* se)
{
}

void ExecEvents()
{
}