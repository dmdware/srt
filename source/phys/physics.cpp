

#include "physics.h"
#include "../math/camera.h"
#include "../window.h"
#include "../bsp/trace.h"
#include "../bsp/tracework.h"
#include "../sim/entity.h"
#include "../save/savemap.h"

#define MAX_CHAR_STEP	25

void Physics()
{
	for(int ei=0; ei<ENTITIES; ei++)
	{
		//if(!g_pcam->m_grounded)
			//g_pcam->accelrise(- g_drawfrinterval * GRAVITY);
		
		Ent* e = &g_entity[ei];

		if(!e->on)
			continue;

		EType* et = &g_etype[e->type];
		Camera* c = &e->camera;

		Vec3f old = c->m_pos;
		Vec3f next = c->m_pos + c->m_vel;

		c->friction();

		TraceWork tw;
		TraceBox(&g_map.m_brush, &tw, old, next, et->vMin, et->vMax, MAX_CHAR_STEP);

		c->moveto(old + (next-old) * tw.traceratio);
	}
}