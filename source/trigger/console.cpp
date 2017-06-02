#include "console.h"
#include "../gui/gui.h"
#include "../sim/player.h"

void Resize_ConsoleLine(Widget* w)
{
	Font* f = &g_font[w->m_font];
	Player* py = &g_player[g_localP];

	int i = 0;
	sscanf(w->m_name.c_str(), "%d", &i);

	w->m_pos[0] = 0;
	w->m_pos[1] = 30 + f->gheight * i;
	w->m_pos[2] = g_width;
	w->m_pos[3] = 30 + f->gheight * (i+1);
}

void Resize_Console(Widget* w)
{
	Font* f = &g_font[w->m_font];
	Player* py = &g_player[g_localP];
	int i = CONSOLE_LINES;
	w->m_pos[0] = 0;
	w->m_pos[1] = 30 + f->gheight * i;
	w->m_pos[2] = g_width;
	w->m_pos[3] = 30 + f->gheight * (i+1);
}

void Change_Console(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* conview = (ViewLayer*)gui->get("console");
	EditBox* con = (EditBox*)conview->get("console");

#if 0
	int caret = con->m_caret;

	if(caret <= 0)
		return;

	unsigned int enter = con->m_value.substr(caret-1, 1).m_part.begin()->m_text.m_data[0];

	if(enter == '\n' || enter == '\r')
		InfoMess("console", "enter");
#endif

	if(scancode == SDL_SCANCODE_ESCAPE && !down)
		ToggleConsole();
}

void SubmitConsole(RichText* rt)
{
	//return;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "console"));
	ViewLayer* con = (ViewLayer*)gui->get("console");

	auto witer = con->m_subwidg.begin();
	witer++;	//skip bg image widget
	for(int i=0; i<CONSOLE_LINES-1; i++)
	{
		auto witer2 = witer;
		witer2++;
		(*witer)->m_text = (*witer2)->m_text;
		witer = witer2;
	}

	auto witer2 = witer;
	witer2++;
	(*witer)->m_text = ParseTags(*rt, NULL);
}

void Submit_Console()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "console"));
	ViewLayer* con = (ViewLayer*)gui->get("console");

	EditBox* coned = (EditBox*)con->get("console");
	SubmitConsole(&coned->m_value);
	RichText blank("");
	coned->changevalue(&blank);
}

void Resize_Con_BG(Widget* w)
{
	Player* py = &g_player[g_localP];
	Font* f = &g_font[MAINFONT16];

	w->m_pos[0] = 0;
	w->m_pos[1] = 0;
	w->m_pos[2] = g_width;
	w->m_pos[3] = CONSOLE_LINES * f->gheight + 30;

	Texture* bg = &g_texture[ w->m_tex ];

	w->m_texc[0] = 0;
	w->m_texc[1] = 0;
	w->m_texc[2] = (w->m_pos[2] - w->m_pos[0]) / (float)bg->width;
	w->m_texc[3] = (w->m_pos[3] - w->m_pos[1]) / (float)bg->height;
}

void FillConsole()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->add(new ViewLayer(gui, "console"));
	ViewLayer* con = (ViewLayer*)gui->get("console");

	con->add(new Image(gui, "", "gui/backg/svlistbg.jpg", true, Resize_Con_BG, 1, 1, 1, 0.7f));

	int y = 30;
	for(int i=0; i<CONSOLE_LINES; i++)
	{
		char name[32];
		sprintf(name, "%d", i);
		con->add(new Text(con, name, RichText(name), MAINFONT16, Resize_ConsoleLine));
	}

	con->add(new EditBox(con, "console", RichText("console"), MAINFONT16, Resize_Console, false, 128, Change_Console, Submit_Console, -1));
}

void ToggleConsole()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* con = (ViewLayer*)gui->get("console");
	con->get("console")->m_opened = true;
	con->m_opened =! con->m_opened;
}