


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



#ifndef BUTTON_H
#define BUTTON_H

#include "../widget.h"

//styles
#define BUST_CORRODE			0
#define BUST_LINEBASED		1
#define BUST_LEFTIMAGE		2

class Button : public Widget
{
public:

	int m_style;

	Button();
	Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip, int f, int style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm, void (*click3)(Widget* w));

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
};

#endif
