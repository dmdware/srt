


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



#ifndef FRAME_H
#define FRAME_H

#include "../widget.h"

class Frame : public Widget
{
public:
	Frame(Widget* parent, const char* n, void (*reframef)(Widget* w));

	void draw();
	void drawover();
	void inev(InEv* ie);
	void frameupd();
};

#endif
