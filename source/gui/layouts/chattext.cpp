


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




#include "chattext.h"
#include "../gui.h"
#include "../../sim/player.h"
#include "../../net/packets.h"
#include "../font.h"
#include "../../net/sendpackets.h"
#include "../../net/client.h"
#include "../icon.h"

short g_chat = -1;

void Resize_ChatLine(Widget* w)
{
	Font* f = &g_font[w->m_font];
	Player* py = &g_player[g_localP];
	int i = 0;
	sscanf(w->m_name.c_str(), "%d", &i);
	float topy = (float)CHATTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->m_pos[0] = 0;
	w->m_pos[1] = topy + f->gheight * i;
	w->m_pos[2] = (float)CHATRIGHT;	//f->gheight * 20;
	w->m_pos[3] = topy + f->gheight * (i+1);
}

void Resize_ChatPrompt(Widget* w)
{
	Font* f = &g_font[w->m_font];
	Player* py = &g_player[g_localP];
	int i = CHAT_LINES+1;
	float topy = (float)CHATTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->m_pos[0] = 0;
	w->m_pos[1] = topy + f->gheight * i;
	w->m_pos[2] = (float)CHATRIGHT;	//f->gheight * 20;
	w->m_pos[3] = topy + f->gheight * (i+1);
}

void Resize_ChatBG(Widget* w)
{
	Font* f = &g_font[CHATFONT];
	Player* py = &g_player[g_localP];
	int i = 0;
	float topy = (float)CHATTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->m_pos[0] = 0;
	w->m_pos[1] = topy + f->gheight * i;
	w->m_pos[2] = (float)CHATRIGHT;	//f->gheight * 20;
	i = CHAT_LINES+1;
	w->m_pos[3] = (float)CHATBOTTOM;	//topy + f->gheight * (i+1);
}

void Submit_Chat()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* cv = (ViewLayer*)gui->get("chat");
	EditBox* enter = (EditBox*)cv->get("enter");
	RichText localmsg = g_name + RichText(": ") + enter->m_value;
	AddChat(&localmsg);
	//char msg[128];
	//sprintf(msg, "%d texlen", enter->m_value.texlen());
	//InfoMess("entermva len", msg);
	//InfoMess("entermva", enter->m_value.rawstr().c_str());
	//InfoMess("local", localmsg.rawstr().c_str());

	ChatPacket cp;
	cp.header.type = PACKET_CHAT;
	cp.client = g_localC;
	strncpy(cp.msg, enter->m_value.rawstr().c_str(), MAX_CHAT);
	cp.msg[MAX_CHAT] = 0;

	if(g_netmode == NETM_HOST)
		SendAll((char*)&cp, sizeof(ChatPacket), true, false, NULL);
	else if(g_netmode == NETM_CLIENT && g_svconn)
		SendData((char*)&cp, sizeof(ChatPacket), &g_svconn->addr, true, false, g_svconn, &g_sock, 0, NULL);

	RichText blank;
	enter->changevalue(&blank);
	enter->m_caret = 0;
	enter->m_scroll[0] = 0;
	enter->m_scroll[1] = 0;
}

void ClearChat()
{
#if 0
	for(int i=0; i<CHAT_LINES+1; i++)
	{
		RichText blank;
		AddChat(&blank);
	}
#endif
}

//add the chat text layout
void AddChat(ViewLayer* cv)
{
	cv->add(new Image(cv, "", "gui/backg/white.jpg", true, Resize_ChatBG, 1.0f, 1.0f, 1.0f, 0.3f));

#if 0
	for(int i=0; i<CHAT_LINES; i++)
	{
		char name[32];
		sprintf(name, "%d", i);
		cv->add(new Text(cv, name, RichText(), CHATFONT, Resize_ChatLine, true, 1.0f, 1.0f, 1.0f, 1.0f));
		//TODO get rid of warnings
	}
#else
	cv->add(new TextBlock(cv, "block", RichText(), CHATFONT, Resize_ChatBG, 0.2f, 0.9f, 0.9f, 1.0f));
#endif

	cv->add(new EditBox(cv, "enter", RichText(), CHATFONT, Resize_ChatPrompt, false, MAX_CHAT, NULL, Submit_Chat, -1));
}

//add a single chat message
void AddChat(RichText* newl)
{
	//Log(newl->rawstr());

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* cv = (ViewLayer*)gui->get("chat");

#if 0
	for(int i=0; i<CHAT_LINES-1; i++)
	{
		char name[32];
		sprintf(name, "%d", i);

		char name2[32];
		sprintf(name2, "%d", i+1);

		Text* text = (Text*)cv->get(name);
		Text* text2 = (Text*)cv->get(name2);

		text->m_text = text2->m_text;
	}

	char name[32];
	sprintf(name, "%d", CHAT_LINES-1);
	Text* text = (Text*)cv->get(name);
	std::string datetime = DateTime();
	//text->m_text = RichText(UStr("[")) + RichText(UStr(datetime.c_str())) + RichText(UStr("] ")) + *newl;
	text->m_text = *newl;
#else
	TextBlock* block = (TextBlock*)cv->get("block");
	//RichText old = block->m_text;
	/*
	necessary to wrap block in RichText() copy constructor because otherwise
	there will be pointer problems as it copies itself while erasing itself.
	TODO add check to see if itself is passed as param
	*/
	block->m_text = RichText(block->m_text) + RichText("\n") + *newl;
	//block->m_text = RichText(block->m_text) + ParseTags(RichText("\\labour\\labour\\labour"),NULL);
	//block->m_text = *newl;

	//InfoMess("newl", newl->rawstr().c_str());
	//InfoMess("block", block->m_text.rawstr().c_str());

	//block->m_scroll[1] += 1.0f;

	int lines = CountLines(&block->m_text, CHATFONT, block->m_pos[0], block->m_pos[1], block->m_pos[2]-block->m_pos[0], block->m_pos[3]-block->m_pos[1]);
	int cutlines = imax(0, lines - CHAT_LINES);

	if(cutlines > 0)
	{
		int glyphi = GetLineStart(&block->m_text, CHATFONT, block->m_pos[0], block->m_pos[1], block->m_pos[2]-block->m_pos[0], block->m_pos[3]-block->m_pos[1], cutlines);
		int len = block->m_text.texlen();
		block->m_text = block->m_text.substr(glyphi, len-glyphi);
	}
#endif
}
