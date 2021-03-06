


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



#ifndef NEWHOSTVIEW_H
#define NEWHOSTVIEW_H

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

class NewHost : public Win
{
public:
	NewHost(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Button m_create;
	EditBox m_gamename;
	Text m_gnlab;	//game name label
	Text m_maplabel;
	DropList m_map;
	CheckBox m_lanonly;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	//void regen(Selection* sel);
	void regen();
	virtual void draw();
	virtual void drawover();
	virtual void reframe();
	virtual void inev(InEv* ie);
};

#endif
