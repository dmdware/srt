


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



#ifndef IMAGE_H
#define IMAGE_H

#include "../widget.h"

class Image : public Widget
{
public:
	Image();
	Image(Widget* parent, const char* nm, const char* filepath, bool clamp, void (*reframef)(Widget* w), float r=1, float g=1, float b=1, float a=1, float texleft=0, float textop=0, float texright=1, float texbottom=1);
	
	void draw();
};

#endif
