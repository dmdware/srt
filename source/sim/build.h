


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




#ifndef BUILD_H
#define BUILD_H

#include "../math/vec3f.h"
#include "../math/vec2i.h"

class Building;
class BlType;
class Matrix;

void DrawSBl();
void UpdSBl();
bool CheckCanPlace(int type, Vec2i tpos, int ignoreb);
void RecheckStand();
void DrawBReason(Matrix* mvp, float width, float height, bool persp);
bool PlaceBl(int type, Vec2i pos, bool finished, int owner, int* bid);
bool PlaceBAb(int btype, Vec2i tabout, Vec2i* tplace);
bool PlaceUAb(int utype, Vec2i cmabout, Vec2i* cmplace);
bool BlCollides(int type, Vec2i tpos, int ignoreb);
bool Collides(Vec2i cmmin, Vec2i cmmax, int ignoreb);

#endif
