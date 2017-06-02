


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



#ifndef BUILDPREVIEW_H
#define BUILDPREVIEW_H

#include "../../widget.h"
#include "../viewportw.h"
#include "../winw.h"

class BlPreview : public Win
{
public:
	BlPreview(Widget* parent, const char* n, void (*reframef)(Widget* w));

	virtual void inev(InEv* ie);
};


void Resize_BP_VP(Widget* w);
void Resize_BP_Tl(Widget* w);
void Resize_BP_Ow(Widget* w);

#endif
