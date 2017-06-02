


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





#include "graph.h"
#include "../sim/simflow.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/conduit.h"
#include "../sim/resources.h"
#include "../sim/simdef.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "../gui/widgets/spez/blgraphs.h"
#include "../gui/widgets/spez/gengraphs.h"

Graph g_graph[GRAPHS];
const char* GRAPHNAME[GRAPHS] =
{"Average personal satiety",
"Average personal funds",
"Total personal satiety",
"Total personal funds"};

Graph g_protecg[PLAYERS];

void DrawGraph(Graph* g, float left, float top, float right, float bottom, float highest, float* color)
{
	EndS();
	UseS(SHADER_COLOR2D);
	Shader* s = &g_shader[g_curS];

	//float highest = 0;
	float lowest = 0;

	for(auto pit=g->points.begin(); pit!=g->points.end(); pit++)
	{
		if(*pit < lowest)
			lowest = *pit;

		if(*pit > highest)
			highest = *pit;
	}

	float xadv = (right-left) / (float)(g->points.size()-1);
	float yadv = (bottom-top) / (highest-lowest);

	std::vector<Vec3f> ps;
	ps.reserve( g->points.size() );
	
	int x = 0;
	for(auto pit=g->points.begin(); pit!=g->points.end(); pit++)
	{
		ps.push_back( Vec3f( left + xadv*x, top + (highest - *pit)*yadv, 0.0f ) );
		x++;
	}

	Player* py = &g_player[g_localP];
	if(!color)
		glUniform4f(s->slot[SSLOT_COLOR], 0.1f, 0.9f, 0.1f, 1.0f);
	else
		glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 1.0f);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

	if(ps.size() <= 0)
		return;

#ifdef PLATFORM_GL14
	glVertexPointer(3, GL_FLOAT, sizeof(float)*0, &ps[0]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &ps[0]);
#endif
	//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[2]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*0, &vertices[2]);
	glDrawArrays(GL_LINE_STRIP, 0, ps.size());
	CHECKGLERROR();

	EndS();	//corpd fix

	CHECKGLERROR();
	Ortho(g_width, g_height, 1.0f, 1.0f, 1.0f, 1.0f);
}

//tally graph data
void Tally()
{
	RecStats();
	RecPyStats();

	unsigned long long froff = g_simframe % CYCLE_FRAMES;

	//do this every 1 frame in CYCLE_FRAMES (once a minute)
	if(froff != 0)
		return;

	//Tally protectionism stats
	for(int i=0; i<PLAYERS; i++)
	{
		g_protecg[i].points.push_back((float)g_player[i].global[RES_DOLLARS]);
	}

	//Tally average personal satiety
	{
		Graph* g = &g_graph[GR_AVGSAT];
		float p = 0;
		int pcnt = 0;

		for(int i=0; i<UNITS; i++)
		{
			Unit* u = &g_unit[i];

			if(!u->on)
				continue;

			if(u->type != UNIT_LABOURER)
				continue;

			p = ( p * pcnt + (float)u->belongings[RES_RETFOOD] ) / (pcnt+1);
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back(p);
	}

	//Tally average personal funds
	{
		Graph* g = &g_graph[GR_AVGFUN];
		float p = 0;
		int pcnt = 0;

		for(int i=0; i<UNITS; i++)
		{
			Unit* u = &g_unit[i];

			if(!u->on)
				continue;

			if(u->type != UNIT_LABOURER)
				continue;

			p = ( p * pcnt + (float)u->belongings[RES_DOLLARS] ) / (pcnt+1);
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back(p);
	}

	//Tally total personal satiety
	{
		Graph* g = &g_graph[GR_TOTSAT];
		float p = 0;
		int pcnt = 0;

		for(int i=0; i<UNITS; i++)
		{
			Unit* u = &g_unit[i];

			if(!u->on)
				continue;

			if(u->type != UNIT_LABOURER)
				continue;

			p += (float)u->belongings[RES_RETFOOD];
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back(p);
	}
	
	//Tally total personal funds
	{
		Graph* g = &g_graph[GR_TOTFUN];
		float p = 0;
		int pcnt = 0;

		for(int i=0; i<UNITS; i++)
		{
			Unit* u = &g_unit[i];

			if(!u->on)
				continue;

			if(u->type != UNIT_LABOURER)
				continue;

			p += (float)u->belongings[RES_DOLLARS];
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back(p);
	}
}

void FreeGraphs()
{
	for(int i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];
		g->points.clear();
		g->cycles = 0;
		g->startframe = 0;
	}
}