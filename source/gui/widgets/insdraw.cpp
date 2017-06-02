


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



#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "droplist.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"

InsDraw::InsDraw() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_INSDRAW;
	clickfunc = NULL;
	m_ldown = false;
}

InsDraw::InsDraw(Widget* parent, void (*inst)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_INSDRAW;
	clickfunc = inst;
	m_ldown = false;
}

void InsDraw::draw()
{
	if(clickfunc != NULL)
		clickfunc();
}

