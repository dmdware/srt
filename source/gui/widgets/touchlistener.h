


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



#ifndef TOUCHLISTENER_H
#define TOUCHLISTENER_H

#include "../widget.h"

class TouchListener : public Widget
{
public:
	TouchListener();
	TouchListener(Widget* parent, const char* name, void (*reframef)(Widget* w), void (*click2)(int p), void (*overf)(int p), void (*out)(), int parm);

	void inev(InEv* ie);
};

#endif
