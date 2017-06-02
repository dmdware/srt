


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




#ifndef DEPOSIT_H
#define DEPOSIT_H

#include "../math/vec2i.h"
#include "../math/matrix.h"
#include "../math/vec3f.h"

class Deposit
{
public:
	bool on;
	bool occupied;
	int restype;
	int amount;
	Vec2i tpos;
	Vec3f drawpos;

	Deposit();
};

#define DEPOSITS	128

extern Deposit g_deposit[DEPOSITS];

void FreeDeposits();
void DrawDeposits(const Matrix projection, const Matrix viewmat);

#endif
