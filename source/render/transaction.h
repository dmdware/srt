


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




#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "../platform.h"
#include "../gui/richtext.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../math/vec2i.h"

class Transaction
{
public:
	RichText rtext;
	Vec3f drawpos;
	float life;
	float halfwidth;
};

#define TRANSACTION_RISE		(15.0f*30.0f)
#define TRANSACTION_DECAY		(0.015f*30.0f)

extern std::list<Transaction> g_transx;
extern bool g_drawtransx;

class Matrix;

void DrawTransx();
void NewTransx(Vec2i cmpos, const RichText* rtext);
void FreeTransx();

#endif
