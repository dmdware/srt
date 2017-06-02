


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



#ifndef PANE_H
#define PANE_H

#include "../widget.h"
#include "image.h"
#include "../cursor.h"
#include "vscrollbar.h"

class Pane : public Widget
{
public:

	VScroll m_vscroll;

	float m_minsz[2];
	int m_mousedown[2];

	Pane();
	Pane(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	void inev(InEv* ie);
	void draw();
	void drawover();
	void reframe();
	void chcall(Widget* ch, int type, void* data);
	void subframe(float* fr);
};

#endif
