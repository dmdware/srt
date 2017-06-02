


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



#ifndef BLGRAPHS_H
#define BLGRAPHS_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../winw.h"

class BlGraphs : public Win
{
public:
	BlGraphs(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Button m_close;
	InsDraw m_graphs;
	DropList m_rsel;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
	void regvals(Selection* sel);
	virtual void reframe();
	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
};


void Resize_BG_Close(Widget *thisw);
void Click_BG_Close();
void Resize_BG_RSel(Widget *thisw);

#endif
