


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



#ifndef BOTTOMPANEL_H
#define BOTTOMPANEL_H

#include "../../widget.h"
#include "../viewportw.h"
#include "../droplist.h"

#define MINIMAP_SIZE				120
#define BOTTOM_MID_PANEL_HEIGHT		200
#define MINIMAP_OFF					5

//bottom panel
class BotPan : public Widget
{
public:
	BotPan(Widget* parent, const char* n, void (*reframef)(Widget* w));

	//Image left_outer_toprightcorner;
	//Image left_outer_top;
	//Viewport left_minimap;

	//Image right_outer_topleftcorner;
	//Image right_outer_top;

	
	//Image right_inner.draw();
	//Image right_left.draw();

	//Image middle_outer_top;

	//Image white_bg;

	Button bottomright_button[9];
	bool bottomright_button_on[9];

	//DropList graph_sel;	//graph selector

	void draw();
	void drawover();
	void reframe();
	void inev(InEv* ie);
	void frameupd();
};

#endif
