


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



#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "../widget.h"

class CheckBox : public Widget
{
public:
	CheckBox();
	CheckBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), int sel=0, float r=1, float g=1, float b=1, float a=1, void (*change)()=NULL);

	void draw();
	void inev(InEv* ie);
	int square();
};

#endif
