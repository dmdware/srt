


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



#ifndef DROPDOWNS_H
#define DROPDOWNS_H

#include "../widget.h"

class DropList : public Widget
{
public:

	int m_mousedown[2];

	DropList();
	DropList(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)());

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);

	int rowsshown();
	int square();
	void erase(int which);

	float topratio()
	{
		return m_scroll[1] / (float)(m_options.size());
	}

	float bottomratio()
	{
		return (m_scroll[1]+rowsshown()) / (float)(m_options.size());
	}

	float scrollspace();
};

#endif
