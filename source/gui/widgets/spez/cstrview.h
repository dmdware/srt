


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



#ifndef CONSTRUCTIONVIEW_H
#define CONSTRUCTIONVIEW_H

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

class CstrView : public Win
{
public:
	CstrView(Widget* parent, const char* n, void (*reframef)(Widget* w), void (*movefunc)(), void (*cancelfunc)(), void (*proceedfunc)(), void (*estimatefunc)());

	void (*movefunc)();
	void (*cancelfunc)();
	void (*proceedfunc)();
	void (*estimatefunc)();

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
};

void Resize_CV_Cl_2(Widget* w);

#endif
