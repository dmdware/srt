


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



#ifndef LISTBOX_H
#define LISTBOX_H

#include "../widget.h"

class ListBox : public Widget
{
public:

	int m_mousedown[2];

	ListBox(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)());

	void draw();
	void inev(InEv* ie);
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
