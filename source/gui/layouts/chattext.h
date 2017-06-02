


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





#ifndef CHATTEXT_H
#define CHATTEXT_H


#define CHATFONT	MAINFONT16

#define CHATTOP		(50)
#define CHATBOTTOM	(g_height-180)
#define CHATRIGHT	(g_width*2/5)

//#define CHAT_LINES		(100/8)
#define CHAT_LINES		((CHATBOTTOM-CHATTOP)/(g_font[CHATFONT].gheight)-1)

class Widget;
class ViewLayer;
class RichText;

extern short g_chat;

void AddChat(ViewLayer* cv);
void AddChat(RichText* newl);
void ClearChat();

#endif
