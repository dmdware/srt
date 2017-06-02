


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




#ifndef SELECTION_H
#define SELECTION_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec3f.h"

class Selection
{
public:
	std::list<int> units;
	std::list<int> buildings;
	std::list<Vec2i> roads;
	std::list<Vec2i> powls;
	std::list<Vec2i> crpipes;
	std::list<unsigned short> fol;

	void clear();
};

extern unsigned int g_circle;

Selection DoSel();
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void DrawMarquee();
void ClearSel(Selection* s);
void AfterSel(Selection* s);
bool USel(short ui);
bool BSel(short bi);

void DoSelection(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir);

#endif
