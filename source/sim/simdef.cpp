


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




#include "simdef.h"
#include "../trigger/console.h"
#include "conduit.h"
#include "building.h"
#include "truck.h"
#include "labourer.h"
#include "../sound/sound.h"
#include "../language.h"
#include "entity.h"

#ifndef MATCHMAKER
#include "../gui/layouts/chattext.h"
#endif

void LoadSysRes()
{
	// Cursor types

#if 0
#define CU_NONE		0	//cursor off?
#define CU_DEFAULT	1
#define CU_MOVE		2	//move window
#define CU_RESZL	3	//resize width (horizontal) from left side
#define CU_RESZR	4	//resize width (horizontal) from right side
#define CU_RESZT	5	//resize height (vertical) from top side
#define CU_RESZB	6	//resize height (vertical) from bottom side
#define CU_RESZTL	7	//resize top left corner
#define CU_RESZTR	8	//resize top right corner
#define CU_RESZBL	9	//resize bottom left corner
#define CU_RESZBR	10	//resize bottom right corner
#define CU_WAIT		11	//shows a hourglass?
#define CU_DRAG		12	//drag some object between widgets?
#define CU_STATES	13
#endif

	LoadSprite("gui/transp", &g_cursor[CU_NONE], false, false);
	LoadSprite("gui/cursors/default", &g_cursor[CU_DEFAULT], false, false);
	LoadSprite("gui/cursors/move", &g_cursor[CU_MOVE], false, false);
	LoadSprite("gui/cursors/reszh", &g_cursor[CU_RESZL], false, false);
	LoadSprite("gui/cursors/reszh", &g_cursor[CU_RESZR], false, false);
	LoadSprite("gui/cursors/reszv", &g_cursor[CU_RESZT], false, false);
	LoadSprite("gui/cursors/reszv", &g_cursor[CU_RESZB], false, false);
	LoadSprite("gui/cursors/reszd2", &g_cursor[CU_RESZTL], false, false);
	LoadSprite("gui/cursors/reszd1", &g_cursor[CU_RESZTR], false, false);
	LoadSprite("gui/cursors/reszd1", &g_cursor[CU_RESZBL], false, false);
	LoadSprite("gui/cursors/reszd2", &g_cursor[CU_RESZBR], false, false);
	LoadSprite("gui/cursors/default", &g_cursor[CU_WAIT], false, false);
	LoadSprite("gui/cursors/default", &g_cursor[CU_DRAG], false, false);


	// Icons

	DefI(ICON_DOLLARS, "gui/icons/dollars.png", "\\$");
	DefI(ICON_PESOS, "gui/icons/pesos.png", "\\peso");
	DefI(ICON_EUROS, "gui/icons/euros.png", "\\euro");
	DefI(ICON_POUNDS, "gui/icons/pounds.png", "\\pound");
	DefI(ICON_FRANCS, "gui/icons/francs.png", "\\franc");
	DefI(ICON_YENS, "gui/icons/yens.png", "\\yen");
	DefI(ICON_RUPEES, "gui/icons/rupees.png", "\\rupee");
	DefI(ICON_ROUBLES, "gui/icons/roubles.png", "\\ruble");
	DefI(ICON_LABOUR, "gui/icons/labour.png", "\\labour");
	DefI(ICON_HOUSING, "gui/icons/housing.png", "\\housing");
	DefI(ICON_FARMPRODUCT, "gui/icons/farmproducts.png", "\\farmprod");
	DefI(ICON_WSFOOD, "gui/icons/wsfood.png", "\\wsfood");
	DefI(ICON_RETFOOD, "gui/icons/retfood.png", "\\retfood");
	DefI(ICON_CHEMICALS, "gui/icons/chemicals.png", "\\chemicals");
	DefI(ICON_ELECTRONICS, "gui/icons/electronics.png", "\\electronics");
	DefI(ICON_RESEARCH, "gui/icons/research.png", "\\research");
	DefI(ICON_PRODUCTION, "gui/icons/production.png", "\\production");
	DefI(ICON_IRONORE, "gui/icons/ironore.png", "\\ironore");
	DefI(ICON_URANIUMORE, "gui/icons/uraniumore.png", "\\uraniumore");
	DefI(ICON_STEEL, "gui/icons/steel.png", "\\steel");
	DefI(ICON_CRUDEOIL, "gui/icons/crudeoil.png", "\\crudeoil");
	DefI(ICON_WSFUEL, "gui/icons/fuelwholesale.png", "\\wsfuel");
	DefI(ICON_STONE, "gui/icons/stone.png", "\\stone");
	DefI(ICON_CEMENT, "gui/icons/cement.png", "\\cement");
	DefI(ICON_ENERGY, "gui/icons/energy.png", "\\energy");
	DefI(ICON_ENUR, "gui/icons/uranium.png", "\\enur");
	DefI(ICON_COAL, "gui/icons/coal.png", "\\coal");
	DefI(ICON_TIME, "gui/icons/time.png", "\\time");
	DefI(ICON_RETFUEL, "gui/icons/fuelretail.png", "\\retfuel");
	DefI(ICON_LOGS, "gui/icons/logs.png", "\\logs");
	DefI(ICON_LUMBER, "gui/icons/lumber.png", "\\lumber");
	DefI(ICON_WATER, "gui/icons/water.png", "\\water");
	DefI(ICON_EXCLAMATION, "gui/icons/exclamation.png", "\\exclam");
	DefI(ICON_CENTS, "gui/icons/cents.png", "\\cent");
	DefI(ICON_SMILEY, "gui/icons/smiley.png", ":)");
	DefI(ICON_FIRM, "gui/icons/firm.png", "\\firm");
	DefI(ICON_GOV, "gui/icons/gov.png", "\\gov");
}

// Queue all the game resources and define objects
void Queue()
{
	//return;
	DefEnt(ETYPE_HUMAN, "Human 1", ECAT_HUMAN, "models/human1/human1.ms3d", Vec3f(2.4267f, 2.4267f, 2.4267f), 1.0f, -1);
}
