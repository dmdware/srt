


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




#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"

class OrderMarker
{
public:
	Vec2i pos;
	long tick;
	float radius;

	OrderMarker(Vec2i p, long t, float r)
	{
		pos = p;
		tick = t;
		radius = r;
	}
};

extern std::list<OrderMarker> g_order;

#define ORDER_EXPIRE		2000

struct MoveOrderPacket;

void DrawOrders(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void Order(int mousex, int mousey, int viewwidth, int viewheight);
void MoveOrder(MoveOrderPacket* mop);
