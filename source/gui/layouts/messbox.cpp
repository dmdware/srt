


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





#include "../gui.h"
#include "../viewlayer.h"
#include "../widgets/textblock.h"
#include "../widgets/button.h"
#include "../widgets/winw.h"
#include "../widgets/touchlistener.h"
#include "ggui.h"
#include "../../language.h"

void Resize_Fill(Widget* w)
{
	Widget* parw = w->m_parent;
	
	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1];
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];
}

void Resize_MB_Continue(Widget* w)
{
	Widget* parw = w->m_parent;
	
	w->m_pos[0] = (parw->m_pos[0] + parw->m_pos[2]) / 2.0f - 50;
	w->m_pos[1] = parw->m_pos[3] - 50;
	w->m_pos[2] = (parw->m_pos[0] + parw->m_pos[2]) / 2.0f + 50;
	w->m_pos[3] = parw->m_pos[3];

	CenterLabel(w);
}

void Click_MB_Continue()
{	
	GUI* gui = &g_gui;
	gui->hide("message");
}

void FillMess()
{
	GUI* gui = &g_gui;
	gui->add(new ViewLayer(gui, "message"));
	ViewLayer* messlayer = (ViewLayer*)gui->get("message");
	messlayer->add(new TouchListener(messlayer, "", Resize_Fullscreen, NULL, NULL, NULL, -1));
	messlayer->add(new Win(messlayer, "win", Resize_CenterWin));
	Win* win = (Win*)messlayer->get("win");
	win->add(new TextBlock(win, "block", RichText("Message"), MAINFONT16, Resize_Fill, 0.8f, 0.8f, 0.2f, 1.0f));
	win->add(new Button(win, "continue", "gui/transp.png", STRTABLE[STR_CONTINUE], RichText(), MAINFONT16, BUST_LINEBASED, Resize_MB_Continue, Click_MB_Continue, NULL, NULL, NULL, NULL, -1, NULL));
}

void Mess(RichText* mess)
{
	GUI* gui = &g_gui;
	gui->show("message");
	ViewLayer* messlayer = (ViewLayer*)gui->get("message");
	Win* win = (Win*)messlayer->get("win");
	win->show();
	TextBlock* block = (TextBlock*)win->get("block");
	block->m_text = *mess;
}