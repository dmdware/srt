


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



#include "gui.h"
#include "../texture.h"
#include "../sim/player.h"

void ViewLayer::reframe()
{
	Player* py = &g_player[g_localP];

	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = g_width-(float)1;
	m_pos[3] = g_height-(float)1;

	Widget::reframe();
}

void ViewLayer::show()
{
	Widget::show();
	
	//necessary for window widgets:
	tofront();	//can't break list iterator, might shift
}