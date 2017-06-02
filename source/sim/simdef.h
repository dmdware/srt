


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




#ifndef SIMDEF_H
#define SIMDEF_H

#ifndef MATCHMAKER
#include "../render/heightmap.h"
#include "../platform.h"
#include "../texture.h"
#include "../sim/resources.h"
#include "../gui/icon.h"
#include "../sim/bltype.h"
#include "../sim/utype.h"
#include "../render/foliage.h"
#include "../render/water.h"
#include "../sim/selection.h"
#include "../sound/sound.h"
#include "../render/particle.h"
#include "../render/sprite.h"
#include "../gui/cursor.h"
#include "../sim/player.h"
#endif

#define AVG_DIST		(TILE_SIZE*6)
//#define CYCLE_FRAMES	(SIM_FRAME_RATE*60*60*24*30)	//1 month / 30 days
#define CYCLE_FRAMES	(SIM_FRAME_RATE*60)	//1 month / 30 days

#define MAXJOBDIST		(TILE_SIZE*MAX_MAP*2)
//#define MAXJOBDIST		(TILE_SIZE*64*2)
//#define MAXJOBDIST		(TILE_SIZE*MAX_MAP*TILE_SIZE*MAX_MAP)
#define MAXJOBDISTNODES	(MAXJOBDIST/PATHNODE_SIZE*TILE_SIZE/PATHNODE_SIZE)	//1 tile width x 40 tiles width
//#define MAXJOBDISTNODES	(MAXJOBDIST/PATHNODE_SIZE/PATHNODE_SIZE)	//1 tile x 40 tiles
//#define MAXJOBDISTNODES	(MAXJOBDIST/PATHNODE_SIZE*10)	//ten deviation from straight path

//labourer unit starting items
//#define STARTING_LABOUR		1000	//lasts for 33.333 seconds
#define STARTING_LABOUR		10		//lasts for 33.333 seconds
//#define STARTING_LABOUR			9		//8am-5pm

/*
 wheat whole flour:
 339 kcal / 100 g =
 3.39 kcal / g =
 0.00339 kcal / mg ->
 1900 kcal / 24 hours =
 79.16666666666667 kcal / hour ->
 (79.16666666666667 kcal / hour) / (0.00339 kcal / mg) =
 23352.99901671583186 mg / hour =
 23.35299901671583 g / hour
 */

#define LABOURER_FOODCONSUM			1
//#define LABOURER_FOODCONSUM		23		//grams per WORK_DELAY (hour)
#define LABOURER_ENERGYCONSUM	1
//#define STARTING_RETFOOD		9000	//lasts for 5 minutes
#define STARTING_RETFOOD		(5 * CYCLE_FRAMES/SIM_FRAME_RATE * LABOURER_FOODCONSUM)	//lasts for 5 minutes

/*
4 months = 4 * 30 days = 120 days ->
120 days * 1900 kcal / day = 228000 kcal ->
228000 kcal / (3.39 kcal / g) = 67256.63716814159292 g
 */

//#define STARTING_RETFOOD		67257	//lasts for 4 months
//#define STARTING_RETFOOD		67257	//lasts for 4 months

#define MUL_RETFOOD				(STARTING_RETFOOD*2)
//#define DRIVE_WORK_DELAY		(SIM_FRAME_RATE*60*60)	//in sim frames, 1 hour
#define DRIVE_WORK_DELAY		(SIM_FRAME_RATE)	//in sim frames
//#define WORK_DELAY				(SIM_FRAME_RATE*60*60)	//in sim frames, 1 hour
#define WORK_DELAY				(SIM_FRAME_RATE)	//in sim frames
//#define LOOKJOB_DELAY_MAX		(SIM_FRAME_RATE*60*30)	//30 minutes
#define LOOKJOB_DELAY_MAX		(SIM_FRAME_RATE*5)	//30 minutes
#define FUEL_DISTANCE			(TILE_SIZE)		//(TILE_SIZE*2.0f)


#define	SHOP_RATE				200

/*
 (300 shop g / hour) /  (23.35299901671583 consum g / hour) =
 
 about 1 hour of shopping per 23.8 hours of existance ;)
 */

//#define	SHOP_RATE				300
#define TRUCK_CONSUMPRATE		1		//galons per SIM_FRAME_RATE (second)
#define TBID_DELAY				200	//truck job bid delay
#define STARTING_FUEL			(10 * CYCLE_FRAMES/SIM_FRAME_RATE * TRUCK_CONSUMPRATE)	//lasts for 10 minute(s)

//the delay between which the building buys inputs, checks if it can complete the input-to-output transform, etc.
//#define BL_AI_DELAY				(SIM_FRAME_RATE*60*30)	//in simframes, 30 minutes

//the delay between which the labourers and trucks switch modes
//#define U_AI_DELAY				(SIM_FRAME_RATE*60*30)	//in simframes, 30 minutes

#define DEFL_DRWAGE			9000
#define DEFL_TRFEE			10000
#define DEFL_CSWAGE			DEFL_DRWAGE

//starting institution funds
#define INST_FUNDS			(40 * 1000 * 1000)

//protectionist state player
//#define PROTEC_PY	0	//main firm = dark green
//#define PROTEC_PY	4	//main firm = red
#define PROTEC_PY	8	//main firm = light green

void LoadSysRes();
void Queue();

#endif
