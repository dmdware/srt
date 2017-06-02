


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



#ifndef TEXTAREA_H
#define TEXTAREA_H

#include "../widget.h"

class TextArea : public Widget
{
public:
	int m_highl[2];	// highlighted (selected) text
	UStr m_compos;	//composition for unicode text

	TextArea(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), float r=1, float g=1, float b=1, float a=1, void (*change)()=NULL);

	void draw();
	int rowsshown();
	int square();

	float topratio()
	{
		return m_scroll[1] / (float)m_lines;
	}

	float bottomratio()
	{
		return (m_scroll[1]+rowsshown()) / (float)m_lines;
	}

	float scrollspace();
	void changevalue(const char* newv);
	bool delnext();
	bool delprev();
	void copyval();
	void pasteval();
	void selectall();
	//void placestr(const char* str);
	void placestr(const RichText* str);
	void placechar(unsigned int k);
	void inev(InEv* ie);
	void hide();
	void gainfocus();
	void losefocus();
};

#endif
