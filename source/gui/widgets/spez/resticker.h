


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



#ifndef RESTICKER_H
#define RESTICKER_H

#include "../../widget.h"

class ResTicker : public Widget
{
public:
	ResTicker(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;

	void draw();
	void drawover();
	void reframe();
	void inev(InEv* ie);
	void frameupd();
};

#endif
