


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




#include "resources.h"
#include "building.h"
#include "bltype.h"

Resource g_resource[RESOURCES];
int g_resEd;

void DefR(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a, int conduit, const char* unit)
{
	Resource* res = &g_resource[resi];
	res->name = n;
	res->depositn = depn;
	res->icon = iconindex;
	res->physical = phys;
	res->capacity = cap;
	res->global = glob;
	res->rgba[0] = r;
	res->rgba[1] = g;
	res->rgba[2] = b;
	res->rgba[3] = a;
	res->conduit = conduit;
	res->unit = unit;
}

/*
Set resource array to 0.
*/
void Zero(int *r)
{
	//for(int i=0; i<RESOURCES; i++)
	//	r[i] = 0;

	memset(r, 0, sizeof(int)*RESOURCES);
}

/*
Players have a global cache of resources, which are available at any point on the map
like in any other RTS game, and they also have localized caches, which are located at
buildings and must be transported.

Players start off with a certain amount of global resources which can be used anywhere on
the map because they don't have any buildings yet to supply localized resources or trucks
to transport them. Global resources can't be replenished though. After a certain point,
all the players' resources will be localized.

When deducting payments/expenditures of resources, preference is given to using up the global
cache first, and only then using up the localized cache of whatever building is in question.

Parameters
	cost:	the array of resources to be deducted
	universal:	pointer to the player's global cache
	stock:	pointer to the building's local cache
	local:	pointer to the player's local cache counter (just a sum of all his/her building caches)
	netch:	a pointer a resource array indicating the net change of resources (I don't remember how this was any different from cost)
	insufres:	a pointer to an int that will indicate the resource type that was insufficient if false is returned (transaction failure)
*/
bool TrySub(const int* cost, int* universal, int* stock, int* local, int* netch, int* insufres)
{
	int combined[RESOURCES];

	if(netch != NULL)
		Zero(netch);

	for(int i=0; i<RESOURCES; i++)
	{
		if(i != RES_LABOUR && !g_resource[i].capacity)
			combined[i] = universal[i];
		else
			combined[i] = 0;

		if(stock != NULL)
			combined[i] += stock[i];

		if(cost[i] > combined[i])
		{
			if(insufres != NULL)
				*insufres = i;
			return false;
		}
	}

	for(int i=0; i<RESOURCES; i++)
	{
		if(i == RES_LABOUR)
		{
			stock[i] -= cost[i];

			if(netch != NULL)
				netch[i] += cost[i];
		}
		else if(cost[i] > universal[i])
		{
			int deducted = universal[i];
			universal[i] = 0;
			stock[i] -= (cost[i] - deducted);

			if(!g_resource[i].capacity)
				local[i] -= (cost[i] - deducted);

			if(netch != NULL)
				netch[i] += cost[i];
		}
		else
		{
			universal[i] -= cost[i];

			if(netch != NULL)
				netch[i] += cost[i];
		}
	}

	return true;
}

/*
Does this building consume or produce res?
*/
bool ResB(int building, int res)
{
	Building* b = &g_building[building];
	BlType* t = &g_bltype[b->type];
	if(t->input[res] > 0)
		return true;
	if(t->output[res] > 0)
		return true;
	return false;
}
