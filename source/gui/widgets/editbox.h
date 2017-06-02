


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



#ifndef EDITBOX_H
#define EDITBOX_H

#include "../widget.h"
#include "../../ustring.h"

class EditBox : public Widget
{
public:
	int m_highl[2];	// highlighted (selected) text
	UStr m_compos;	//composition for unicode text
	void (*submitfunc)();
	void (*changefunc3)(unsigned int key, unsigned int scancode, bool down, int parm);

	EditBox();
	EditBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), bool pw, int maxl, void (*change3)(unsigned int key, unsigned int scancode, bool down, int parm), void (*submitf)(), int parm);

	void draw();
	RichText drawvalue();
	void frameupd();
	void inev(InEv* ie);
	void placestr(const RichText* str);
	void changevalue(const RichText* str);
	bool delnext();
	bool delprev();
	void copyval();
	void pasteval();
	void selectall();
	void hide();
	void gainfocus();
	void losefocus();
};

#endif
