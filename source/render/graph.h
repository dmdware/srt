


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






#ifndef GRAPH_H
#define GRAPH_H

#include "../platform.h"
#include "../sim/player.h"

class Graph
{
public:
	std::list<float> points;
	unsigned long long startframe;
	unsigned int cycles;

	Graph()
	{
		startframe = 0;
		cycles = 0;
	}
};

#define GR_AVGSAT		0	//average personal satiety
#define GR_AVGFUN		1	//average personal funds
#define GR_TOTSAT		2	//total personal satiety
#define GR_TOTFUN		3	//total personal funds
#define GRAPHS			4

extern const char* GRAPHNAME[GRAPHS];

extern Graph g_graph[GRAPHS];

extern Graph g_protecg[PLAYERS];

void Tally();
void FreeGraphs();
void DrawGraph(Graph* g, float left, float top, float right, float bottom, float highest=0, float* color=NULL);

#endif