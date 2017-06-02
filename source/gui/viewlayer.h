


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



#ifndef VIEWLAYER_H
#define VIEWLAYER_H

#include "widget.h"

class ViewLayer : public Widget
{
public:

	ViewLayer()
	{
		m_name = "";
		m_opened = false;
		m_type = WIDGET_VIEWLAYER;
		m_parent = NULL;
		m_hidden = true;
	}

	ViewLayer(Widget* parent, const char* n) : Widget()
	{
		m_name = n;
		m_opened = false;
		m_type = WIDGET_VIEWLAYER;
		m_parent = parent;
		m_hidden = true;
		
		reframe();
	}

	virtual void reframe();
	virtual void show();
};

#endif
