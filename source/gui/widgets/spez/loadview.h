


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



#ifndef LOADVIEW_H
#define LOADVIEW_H

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


#define SLLISTBOT	70 //save/load view bottom space

class LoadView : public Win
{
public:
	LoadView(Widget* parent, const char* n, void (*reframef)(Widget* w));

	//when persistance of internal widget states is important between calls to "regen()",
	//then children widgets must be stored as members, rather than items in the m_subwidg list,
	//because the list is destroyed and regenerated.
	//in this case we need to keep the state of the scroll bar. the other widgets like the join button
	//probably don't need to be member variables.
	Image m_svlistbg;
	std::list<RichText> m_files;
	VScroll m_vscroll;
	RichText* m_selfile;	//selected file
	float m_listbot;	//bottom y screen coord of list items
	float m_listtop;
	Button m_loadbut;
	Button m_delbut;
	Text m_curname;
	unsigned char m_savemode;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	//void regen(Selection* sel);
	void regen(unsigned char savemode);
	virtual void draw();
	virtual void drawover();
	virtual void reframe();
	virtual void inev(InEv* ie);
	virtual void frameupd();
};

#endif
