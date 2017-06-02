


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



#ifndef INSDRAW_H
#define INSDRAW_H

#include "../widget.h"

class InsDraw : public Widget
{
public:
	InsDraw();
	InsDraw(Widget* parent, void (*inst)());

	void draw();
};

#endif
