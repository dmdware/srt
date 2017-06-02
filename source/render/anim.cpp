


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





#include "anim.h"

bool PlayAni(float& frame, int first, int last, bool loop, float rate)
{
	if(frame < first || frame > last+1)
	{
		frame = first;
		return false;
	}

	frame += rate;

	if(frame > last)
	{
		if(loop)
			frame = first;
		else
			frame = last;

		return true;
	}

	return false;
}

//Play animation backwards
bool PlayAniB(float& frame, int first, int last, bool loop, float rate)
{
	if(frame < first-1 || frame > last)
	{
		frame = last;
		return false;
	}

	frame -= rate;

	if(frame < first)
	{
		if(loop)
			frame = last;
		else
			frame = first;

		return true;
	}

	return false;
}