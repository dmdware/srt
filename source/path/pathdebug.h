


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



#ifndef PATHDEBUG_H
#define PATHDEBUG_H

class Unit;

extern Unit* g_pathunit;
extern std::vector<Vec3f> g_gridvecs;

void DrawSteps();
void DrawGrid();
void DrawUnitSquares();
void DrawPaths();
void DrawVelocities();
void LogPathDebug();

#endif
