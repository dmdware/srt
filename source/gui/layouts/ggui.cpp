


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



#include "../../app/appmain.h"
#include "../../gui/gui.h"
#include "../keymap.h"
#include "../../render/heightmap.h"
#include "../../math/hmapmath.h"
#include "../../math/camera.h"
#include "../../render/screenshot.h"
#include "../../save/savemap.h"
#include "../../sim/unit.h"
#include "../../sim/utype.h"
#include "../../sim/building.h"
#include "../../sim/bltype.h"
#include "../../sim/selection.h"
#include "../../render/water.h"
#include "../../sim/order.h"
#include "../../path/pathdebug.h"
#include "../../path/pathnode.h"
#include "../../sim/build.h"
#include "../../sim/player.h"
#include "../../sim/unit.h"
#include "../../sim/utype.h"
#include "../../gui/widgets/winw.h"
#include "../../debug.h"
#include "../../trigger/console.h"
#include "../../window.h"
#include "../../net/lockstep.h"
#include "../../net/sendpackets.h"
#include "../../gui/widgets/spez/svlist.h"
#include "../../gui/widgets/spez/newhost.h"
#include "../../gui/widgets/spez/loadview.h"
#include "../../gui/widgets/spez/lobby.h"
#include "../../sim/simdef.h"
#include "../../path/collidertile.h"
#include "../../sim/map.h"
#include "../../sim/bltype.h"
#include "../../net/client.h"
#include "../../algo/random.h"
#include "../../sim/umove.h"
#include "../widgets/spez/saveview.h"
#include "../widgets/spez/loadview.h"
#include "../../language.h"
#include "../../sim/border.h"
#include "segui.h"
#include "seviewport.h"

//not engine
#include "edgui.h"
#include "playgui.h"
#include "ggui.h"
#include "messbox.h"
#include "../../app/appmain.h"

//bool g_canselect = true;

//char g_lastsave[DMD_MAX_PATH+1];

void Resize_Fullscreen(Widget* w)
{
	w->m_pos[0] = 0;
	w->m_pos[1] = 0;
	w->m_pos[2] = (float)g_width-1;
	w->m_pos[3] = (float)g_height-1;

#if 0
	if(w->m_type == WIDGET_IMAGE)
	{
		Texture* tex = &g_texture[w->m_tex];
	
		w->m_texc[0] = w->m_pos[0] / (float)tex->width;
		w->m_texc[1] = w->m_pos[1] / (float)tex->height;
		w->m_texc[2] = w->m_pos[2] / (float)tex->width;
		w->m_texc[3] = w->m_pos[3] / (float)tex->height;
	}
#endif
}

void Resize_FullscreenSq(Widget* w)
{
	float minsz = fmin((float)g_width-1, (float)g_height-1);

	w->m_pos[0] = g_width/2.0f - minsz/2.0f;
	w->m_pos[1] = g_height/2.0f - minsz/2.0f;
	w->m_pos[2] = g_width/2.0f + minsz/2.0f;
	w->m_pos[3] = g_height/2.0f + minsz/2.0f;

#if 0
	if(w->m_type == WIDGET_IMAGE)
	{
		Texture* tex = &g_texture[w->m_tex];
	
		w->m_texc[0] = w->m_pos[0] / (float)tex->width;
		w->m_texc[1] = w->m_pos[1] / (float)tex->height;
		w->m_texc[2] = w->m_pos[2] / (float)tex->width;
		w->m_texc[3] = w->m_pos[3] / (float)tex->height;
	}
#endif
}

void Resize_AppLogo(Widget* w)
{
	w->m_pos[0] = 30;
	w->m_pos[1] = 30;
	w->m_pos[2] = 200;
	w->m_pos[3] = 200;
}

void Resize_AppTitle(Widget* w)
{
	w->m_pos[0] = 30;
	w->m_pos[1] = 30;
	w->m_pos[2] = (float)g_width-1;
	w->m_pos[3] = 100;
}

void Click_LoadMapButton()
{
	GUI *gui = &g_gui;
	LoadView *v = (LoadView*)gui->get("load");
	v->regen(SAVEMODE_MAPS);
	v->show();
}

void Click_SaveMapButton()
{
	GUI *gui = &g_gui;
	SaveView *v = (SaveView*)gui->get("save");
	v->regen(SAVEMODE_MAPS);
	v->show();
}

void Click_QSaveMapButton()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveMapButton();
		return;
	}

	SaveMap(g_lastsave);
}

void Resize_MenuItem(Widget* w)
{
	int row;
	sscanf(w->m_name.c_str(), "%d", &row);
	Font* f = &g_font[w->m_font];

	Player* py = &g_player[g_localP];

	//w->m_pos[0] = g_width/2 - f->gheight*4;
	w->m_pos[0] = f->gheight*4;
	w->m_pos[1] = g_height/2 - f->gheight*4 + f->gheight*1.5f*row;
	w->m_pos[2] = w->m_pos[0] + f->gheight * 10;
	w->m_pos[3] = w->m_pos[1] + f->gheight;
}

void Resize_LoadingStatus(Widget* w)
{
	Player* py = &g_player[g_localP];

	w->m_pos[0] = g_width/2 - 64;
	w->m_pos[1] = g_height/2;
	w->m_pos[2] = g_width;
	w->m_pos[3] = g_height;
	w->m_tpos[0] = g_width/2;
	w->m_tpos[1] = g_height/2;
}

void Resize_WinText(Widget* w)
{
	Widget* parent = w->m_parent;

	w->m_pos[0] = parent->m_pos[0];
	w->m_pos[1] = parent->m_pos[1];
	w->m_pos[2] = w->m_pos[0] + 400;
	w->m_pos[3] = w->m_pos[1] + 2000;
}

void Click_HostGame()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->show("new host");
	NewHost* newhost = (NewHost*)gui->get("new host");
	newhost->regen();
}

void Click_ListHosts()
{
	//g_mode = APPMODE_PLAY;

	//g_netmode = NETM_CLIENT;
	//g_needsvlist = true;
	//g_reqdsvlist = false;

	//Connect("localhost", PORT, false, true, false, false);
	//Connect(SV_ADDR, PORT, true, false, false, false);

	//BegSess();

	//g_canturn = true;	//temp, assuming no clients were handshook to server when netfr0 turn happened and server didn't have any cl's to send NetTurnPacket to

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->show("sv list");
}

void Click_EndGame()
{
	EndSess();
}

#if 0
void Click_NewGame()
{
	//return;

	CHECKGLERROR();

	FreeMap();

	g_hmap.alloc(MAX_MAP, MAX_MAP);

	for(int x=0; x<=MAX_MAP; x++)
	{
		for(int y=0; y<=MAX_MAP; y++)
		{
			g_hmap.setheight(x, y, rand3()%7);
		//	g_hmap.setheight(x, y, 0);
		}
	}
    
    g_hmap.lowereven();
	g_hmap.remesh();

	for(int x=0; x<=MAX_MAP; x++)
	{
		for(int y=0; y<=MAX_MAP; y++)
		{
			g_hmap.setheight(x, y, g_hmap.getheight(x, y) / 2 + 1);
		//	g_hmap.setheight(x, y, 0);
		}
	}
    
    g_hmap.lowereven();
	g_hmap.remesh();

	AllocPathGrid(g_mapsz.x*TILE_SIZE, g_mapsz.y*TILE_SIZE);
	AllocGrid((int)g_mapsz.x, (int)g_mapsz.y);
	FillColliderGrid();

	FillForest(rand3());

	for(int i=0; i<10; i++)
		//for(int j=0; j<10; j++)
	//for(int i=0; i<15; i++)
		for(int j=0; j<2; j++)
	//for(int i=0; i<1; i++)
		//for(int j=0; j<15; j++)
		{

			//Vec3i cmpos((g_mapsz.x+4)*TILE_SIZE/2 + (i+2)*PATHNODE_SIZE, 0, g_mapsz.y*TILE_SIZE/2 + (j+2)*PATHNODE_SIZE);
			//cmpos.y = g_hmap.accheight(cmpos.x, cmpos.z);
			Vec2i cmpos(4*TILE_SIZE + (i+2)*PATHNODE_SIZE*3, 4*TILE_SIZE + (j+2)*PATHNODE_SIZE*3);

			//if(rand()%2 == 1)
			//	PlaceUnit(UNIT_BATTLECOMP, cmpos, 0);
			//else
			PlaceUnit(UNIT_LABOURER, cmpos, 0, NULL);
			//PlaceUnit(UNIT_TRUCK, cmpos, rand()%PLAYERS, NULL);

			ClearFol(cmpos.x - TILE_SIZE, cmpos.y - TILE_SIZE, cmpos.x + TILE_SIZE, cmpos.y + TILE_SIZE);
		}

	for(int x=0; x<g_mapsz.x; x++)
		for(int y=0; y<g_mapsz.y; y++)
		{
			g_border[x + y*g_mapsz.x] = (signed char)-1;
			//g_border[x + y*g_mapsz.x] = rand()%6;
		}

	Vec3i cmscroll;
	cmscroll.x = 4 * TILE_SIZE;
	cmscroll.y = 4 * TILE_SIZE;
	cmscroll.z = Bilerp(&g_hmap, cmscroll.x, cmscroll.y);
	g_scroll = CartToIso(cmscroll) - Vec2i(g_width,g_height)/2;

	//g_localP = NewPlayer();
	g_localP = 0;
	Player* py = &g_player[g_localP];
	py->on = true;
	py->ai = false;
	AddClient(NULL, g_name, &g_localC);
	AssocPy(g_localP, g_localC);

	//return;

	CHECKGLERROR();

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		//p->on = true;
		p->ai = (i == g_localP) ? false : true;
		//p->ai = true;

		p->global[RES_DOLLARS] = INST_FUNDS;
		p->global[RES_FARMPRODUCTS] = 4000;
		p->global[RES_RETFOOD] = 4000;
		p->global[RES_CEMENT] = 150;
		p->global[RES_STONE] = 4000;
		p->global[RES_RETFUEL] = 4000;
		p->global[RES_URANIUM] = 4000;
		//p->global[RES_ELECTRONICS] = 4000;
		p->global[RES_METAL] = 150;
		p->global[RES_PRODUCTION] = 40;
	}

	for(int i=1; i<imin(PLAYERS,8); i++)
	{
		Player* p = &g_player[i];
		p->on = true;
	}

	CHECKGLERROR();

	g_mode = APPMODE_PLAY;

	GUI* gui = &g_gui;

	//return;

	gui->hideall();
	gui->show("play");
	gui->show("chat");
	gui->show("play right opener");

	CHECKGLERROR();

#if 0
	gui->add(new Win(gui, "window", Resize_Window));
	//gui->show("window");

	Win* win = (Win*)gui->get("window");

	RichText bigtext;

	for(int i=0; i<2000; i++)
	{
		bigtext.m_part.push_back(RichPart((unsigned int)(rand()%3000+16000)));

		if(rand()%10 == 1)
			bigtext.m_part.push_back(RichPart((unsigned int)'\n'));
	}

	win->add(new TextBlock(win, "text block", bigtext, MAINFONT16, Resize_WinText));
#endif

	g_lastsave[0] = '\0';
	BegSess();
	g_netmode = NETM_SINGLE;
}
#endif

void Click_OpenTut()
{
	RichText mess = RichText("Coming soon.");
	Mess(&mess);
}

void Click_OpenMod()
{
	RichText mess = RichText("Coming soon.");
	Mess(&mess);
}

void Click_OpenEd()
{
#ifdef PLATFORM_MOBILE
	RichText mess = RichText("Only available in desktop version.");
	Mess(&mess);
	return;
#endif

	CHECKGLERROR();

	FreeMap();
	FreePys();

	g_hmap.alloc(MAX_MAP, MAX_MAP);

	for(int x=0; x<MAX_MAP; x++)
	{
		for(int y=0; y<MAX_MAP; y++)
		{
			//g_hmap.setheight(x, y, rand()%7);
			g_hmap.setheight(x, y, 1);
		}
	}
    
    g_hmap.lowereven();
	g_hmap.remesh();

	AllocPathGrid(g_mapsz.x*TILE_SIZE, g_mapsz.y*TILE_SIZE);
	AllocGrid((int)g_mapsz.x, (int)g_mapsz.y);
	FillColliderGrid();

	Vec3i cmscroll;
	cmscroll.x = 4 * TILE_SIZE;
	cmscroll.y = 4 * TILE_SIZE;
	cmscroll.z = Bilerp(&g_hmap, cmscroll.x, cmscroll.y);
	g_scroll = CartToIso(cmscroll) - Vec2i(g_width,g_height)/2;

	g_mode = APPMODE_EDITOR;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->hideall();
	gui->show("ed");
	Change_ToolCat();

	g_lastsave[0] = '\0';
}

void Resize_CenterWin(Widget* w)
{
	Player* py = &g_player[g_localP];

	w->m_pos[0] = 0;
	w->m_pos[1] = g_height - 200;
	w->m_pos[2] = 400;
	w->m_pos[3] = g_height;
}

void Resize_SvList(Widget* w)
{
	Player* py = &g_player[g_localP];

	w->m_pos[0] = 0;
	w->m_pos[1] = g_height - 300;
	w->m_pos[2] = 550;
	w->m_pos[3] = g_height;
}

//TODO make window resize functions fit window to screen, no matter how small it is, but be more centered on larger screens
//TODO scroll bars on smaller screens
void Resize_CenterWin2(Widget* w)
{
	Player* py = &g_player[g_localP];

	w->m_pos[0] = g_width/2 - 250 + 60;
	w->m_pos[1] = g_height - 300;
	w->m_pos[2] = g_width/2 + 200 + 60;
	w->m_pos[3] = g_height;
}

void Click_LoadGame()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->show("load");
	((LoadView*)gui->get("load"))->regen(SAVEMODE_SAVES);
}

void Click_Options()
{
	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("options");
}

#if 0
void Click_Quit()
{
	EndSess();
	FreeMap();
	g_quit = true;
}
#endif

void Resize_JoinCancel(Widget* w)
{
	w->m_pos[0] = g_width/2 - 75;
	w->m_pos[1] = g_height/2 + 100 - 30;
	w->m_pos[2] = g_width/2 + 75;
	w->m_pos[3] = g_height/2 + 100 - 0;
	CenterLabel(w);
}

void Click_SetOptions()
{
	GUI* gui = &g_gui;
	ViewLayer* opview = (ViewLayer*)gui->get("options");
#ifndef PLATFORM_MOBILE
	DropList* winmodes = (DropList*)opview->get("0");
	DropList* reslist = (DropList*)opview->get("1");
#endif
	EditBox* namebox = (EditBox*)opview->get("2");
	DropList* vols = (DropList*)opview->get("3");
	DropList* langs = (DropList*)opview->get("4");

	bool restart = false;
	bool write = false;

#ifndef PLATFORM_MOBILE
	int w;
	int h;
	bool fs;
#endif
	std::string lang;
	int vol;
#ifndef PLATFORM_MOBILE
	std::string resname = reslist->m_options[ reslist->m_selected ].rawstr();
	fs = winmodes->m_selected == 1 ? true : false;
	sscanf(resname.c_str(), "%dx%d", &w, &h);
#endif

	char langfull[DMD_MAX_PATH+1];
	FullPath("lang/", langfull);
	std::list<std::string> langdirs;
	ListDirs(langfull, langdirs);

	int lin=0;
	for(auto lit=langdirs.begin(); lit!=langdirs.end(); lit++, lin++)
	{
		if(lin != langs->m_selected)
			continue;

		lang = *lit;
		break;
	}

	vol = vols->m_selected * 10;

#ifndef PLATFORM_MOBILE
	if(w != g_selres.width)
		restart = true;

	if(h != g_selres.height)
		restart = true;

	if(fs != g_fullscreen)
		restart = true;
#endif
	
	if(vol != g_volume)
		write = true;
	
	if(stricmp(lang.c_str(), g_lang.c_str()) != 0)
	{
		SwitchLang(lang.c_str());
		write = true;
		restart = true;
	}

	gui->hideall();
	gui->show("menu");

	std::string oldname = g_name.rawstr();
	std::string newname = namebox->m_value.rawstr();

	//name changed?
	if(strcmp(oldname.c_str(), newname.c_str()) != 0)
	{
		g_name = namebox->m_value;
		WriteName();
	}

	if(restart)
		write = true;

	if(!write)
		return;
	
#ifdef PLATFORM_MOBILE
#else
	g_selres.width = w;
	g_selres.height = h;
	g_width = w;
	g_height = h;
	g_fullscreen = fs;
#endif
	g_lang = lang;
	//g_lang = LANG_ENG;
	g_volume = vol;
	SetVol(vol);

	WriteConfig();

	if(!restart)
		return;

	g_mode = APPMODE_RELOADING;
}

void Resize_CopyInfo(Widget* w)
{
	w->m_pos[0] = g_width - 260;
	w->m_pos[1] = g_height - 60;
	w->m_pos[2] = g_width;
	w->m_pos[3] = g_height;
}

void EnumDisplay()
{
	g_resolution.clear();
	
	for(int i=0; i<SDL_GetNumDisplayModes(0); i++)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, i, &mode);
		
		bool found = false;
		
		for(auto rit=g_resolution.begin(); rit!=g_resolution.end(); rit++)
		{
			if(rit->width == mode.w &&
			   rit->height == mode.h)
			{
				found = true;
				break;
			}
		}
		
		if(found)
			continue;
		
#ifdef PLATFORM_MOBILE
		//landscape only
		if(mode.h > mode.w)
			continue;
#endif
		
		Resolution res;
		res.width = mode.w;
		res.height = mode.h;
		g_resolution.push_back(res);
	}
}

void FillMenu()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	// Main ViewLayer
	gui->add(new ViewLayer(gui, "main"));
	ViewLayer* mainview = (ViewLayer*)gui->get("main");
	mainview->add(new Image(mainview, "", "gui/mmbg.jpg", true, Resize_Fullscreen));
	mainview->add(new Image(mainview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	mainview->add(new Text(mainview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
#if 1
	//mainview->add(new Link(mainview, "0", (STRTABLE[STR_TUTORIAL]), MAINFONT16, Resize_MenuItem, Click_OpenTut));
	mainview->add(new Link(mainview, "1", (STRTABLE[STR_NEWGAME]), MAINFONT16, Resize_MenuItem, Click_NewGame));
	mainview->add(new Link(mainview, "2", (STRTABLE[STR_LOADGAME]), MAINFONT16, Resize_MenuItem, Click_LoadGame));
	mainview->add(new Link(mainview, "3", (STRTABLE[STR_HOSTGAME]), MAINFONT16, Resize_MenuItem, Click_HostGame));
	mainview->add(new Link(mainview, "4", (STRTABLE[STR_JOINGAME]), MAINFONT16, Resize_MenuItem, Click_ListHosts));
#ifndef PLATFORM_MOBILE
	mainview->add(new Link(mainview, "5", (STRTABLE[STR_MAPED]), MAINFONT16, Resize_MenuItem, Click_OpenEd));
#endif
	//mainview->add(new Link(mainview, "6", RichText("Load Mod"), MAINFONT16, Resize_MenuItem, Click_OpenMod));
	mainview->add(new Link(mainview, "6", (STRTABLE[STR_OPTIONS]), MAINFONT16, Resize_MenuItem, Click_Options));
#ifndef PLATFORM_MOBILE
	mainview->add(new Link(mainview, "7", (STRTABLE[STR_QUIT]), MAINFONT16, Resize_MenuItem, Click_Quit));
#endif
#else
	mainview->add(new Link(mainview, "0", RichText("New Game"), MAINFONT16, Resize_MenuItem, Click_NewGame));
	mainview->add(new Link(mainview, "1", RichText("Load Game"), MAINFONT16, Resize_MenuItem, Click_LoadGame));
	mainview->add(new Link(mainview, "2", RichText("Options"), MAINFONT16, Resize_MenuItem, Click_Options));
	mainview->add(new Link(mainview, "3", RichText("Quit"), MAINFONT16, Resize_MenuItem, Click_Quit));
#endif

	char infostr[512];
	char verstr[32];
	VerStr(VERSION, verstr);
	sprintf(infostr, "%s (c) DMD 'Ware 2015\n%s %s", 
		STRTABLE[STR_COPYRIGHT].rawstr().c_str(), 
		STRTABLE[STR_VERSION].rawstr().c_str(), 
		verstr);
	mainview->add(new TextBlock(mainview, "copy", RichText(infostr), MAINFONT16, Resize_CopyInfo, 0.2f, 1.0f, 0.2f, 1.0f));

	//Options ViewLayer
	gui->add(new ViewLayer(gui, "options"));
	ViewLayer* opview = (ViewLayer*)gui->get("options");
	opview->add(new Image(opview, "", "gui/mmbg.jpg", true, Resize_Fullscreen));
	opview->add(new Image(opview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	opview->add(new Text(opview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	opview->add(new DropList(opview, "0", MAINFONT16, Resize_MenuItem, NULL));
	opview->add(new DropList(opview, "1", MAINFONT16, Resize_MenuItem, NULL));
	opview->add(new EditBox(opview, "2", g_name, MAINFONT16, Resize_MenuItem, false, PYNAME_LEN, NULL, NULL, -1));
	opview->add(new DropList(opview, "3", MAINFONT16, Resize_MenuItem, NULL));
	opview->add(new DropList(opview, "4", MAINFONT16, Resize_MenuItem, NULL));
	opview->add(new Link(opview, "6", (STRTABLE[STR_DONE]), MAINFONT16, Resize_MenuItem, Click_SetOptions));
	opview->add(new Text(opview, "7", (STRTABLE[STR_LOWERRESREC]), MAINFONT16, Resize_MenuItem));

	DropList* vols = (DropList*)opview->get("3");
	for(int i=0; i<=100; i+=10)
	{
		char volstr[32];
		sprintf(volstr, "%d", i);
		vols->m_options.push_back((STRTABLE[STR_VOLUMEPCT]) + RichText(volstr));
	}
	vols->m_selected = g_volume / 10;

#if 1
	DropList* langs = (DropList*)opview->get("4");
	std::list<std::string> langdirs;
	char langpath[DMD_MAX_PATH+1];
	FullPath("lang/", langpath);
	ListDirs(langpath, langdirs);

	int lin = 0;
	for(auto lit=langdirs.begin(); lit!=langdirs.end(); lit++, lin++)
	{
		std::string selflang = LoadLangStr(lit->c_str(), STRFILE[STR_SELFLANG]);

		if(stricmp(lit->c_str(), g_lang.c_str()) == 0)
			langs->m_selected = lin;

		langs->m_options.push_back(RichText(selflang.c_str()));
	}
#endif
	
	DropList* winmodes = (DropList*)opview->get("0");
	winmodes->m_options.push_back((STRTABLE[STR_WINDOWED]));
	winmodes->m_options.push_back((STRTABLE[STR_FULLSCREEN]));
	winmodes->m_selected = g_fullscreen ? 1 : 0;
	DropList* reslist = (DropList*)opview->get("1");
	EnumDisplay();
	int uniquei = 0;
	for(auto rit=g_resolution.begin(); rit!=g_resolution.end(); rit++, uniquei++)
	{
		if(rit->width == g_selres.width &&
		   rit->height == g_selres.height)
			reslist->m_selected = uniquei;

		char resname[32];
		sprintf(resname, "%dx%d", rit->width, rit->height);
		reslist->m_options.push_back(RichText(resname));
	}
	
	gui->add(new SaveView(gui, "save", Resize_BlPreview));
	gui->add(new LoadView(gui, "load", Resize_BlPreview));
	gui->add(new SvList(gui, "sv list", Resize_SvList));
	gui->add(new NewHost(gui, "new host", Resize_CenterWin2));
	FillLobby();

	gui->add(new ViewLayer(gui, "join"));
	ViewLayer* joinview = (ViewLayer*)gui->get("join");
	joinview->add(new Image(joinview, "", "gui/mmbg.jpg", true, Resize_Fullscreen));
	joinview->add(new Image(joinview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	joinview->add(new Text(joinview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	joinview->add(new Text(joinview, "status", (STRTABLE[STR_JOINING]), MAINFONT16, Resize_LoadingStatus));
	joinview->add(new Button(joinview, "cancel", "gui/transp.png", (STRTABLE[STR_CANCEL]), RichText(), MAINFONT16, BUST_LINEBASED, Resize_JoinCancel, Click_JoinCancel, NULL, NULL, NULL, NULL, -1, NULL));
}

void MouseLDown()
{
	if(g_mode == APPMODE_PLAY ||
		g_mode == APPMODE_EDITOR)
	{
		Player* py = &g_player[g_localP];
		g_mousestart = g_mouse;
        
        if(g_mode == APPMODE_EDITOR)
            g_vdrag[0] = g_mouse3d;

		if(g_mode == APPMODE_EDITOR &&
			GetEdTool() == TOOLCAT_BORDERS)
			EdPlaceBords();
	}
}

void EdPlaceFol()
{
	Vec2i pixpos = g_mouse + g_scroll;
	Vec3f ray;
	Vec3f point;
	IsoToCart(pixpos, &ray, &point);
	Vec3i cmpos;
	//if(!MapInter(&g_hmap, ray, point, &cmpos))
	//	return;
    
    Vec3f line[2];
    Vec3f fint;
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
        return;
    cmpos = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);

	int ftype = rand()%FL_TYPES;
	FlType *ft = &g_fltype[ftype];

	Vec2i cmmin;
	Vec2i cmmax;

	cmmin.x = cmpos.x - ft->size.x / 2;
	cmmin.y = cmpos.y - ft->size.x / 2;
	cmmax.x = cmmin.x + ft->size.x;
	cmmax.y = cmmin.y + ft->size.y;

	if(Collides(cmmin, cmmax, -1))
		return;

	PlaceFol(ftype, cmpos);
}

void EdPlaceUnit()
{	
	int type = GetEdUType();

	if(type < 0)
		return;

	int owner = GetPlaceOwner();

	if(owner < 0)
		return;

	Vec2i cmpos = Vec2i(g_mouse3d.x, g_mouse3d.y);

	if(!UnitCollides(NULL, cmpos, type))
		PlaceUnit(type, cmpos, owner, NULL);
}

void EdPlaceBl()
{
#if 0
	Vec2i pixpos = g_mouse + g_scroll;
	Vec3f ray;
	Vec3f point;
	IsoToCart(pixpos, &ray, &point);
	Vec3i cmpos;
	//if(!MapInter(&g_hmap, ray, point, &cmpos))
    //   return;
    Vec3f line[2];
    Vec3f fint;
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
        return;
    cmpos = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
#endif

	if(!g_canplace)
		return;

	int type = GetEdBlType();

	Vec2i tpos (g_mouse3d.x/TILE_SIZE, g_mouse3d.y/TILE_SIZE);
    
    int owner = GetPlaceOwner();

	if(CheckCanPlace(type, tpos, -1))
		PlaceBl(type, tpos, true, owner, NULL);
#if 0
    g_hmap.setheight( intersect.x / TILE_SIZE, intersect.z / TILE_SIZE, 0);
    g_hmap.lowereven();
	g_hmap.remesh();
#endif
}

void EdDel()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Selection sel = DoSel();

	for(auto uit = sel.units.begin(); uit != sel.units.end(); uit++)
	{
		g_unit[ *uit ].destroy();
		return;
	}

	for(auto bit = sel.buildings.begin(); bit != sel.buildings.end(); bit++)
	{
		g_building[ *bit ].destroy();
		return;
	}

	for(int cdtype = 0; cdtype < CD_TYPES; cdtype++)
	{
		CdType* ct = &g_cdtype[cdtype];
		short seloff = ct->seloff;
		std::list<Vec2i> *list = (std::list<Vec2i>*)(((char*)&sel)+seloff);

		for(auto cit = list->begin(); cit != list->end(); cit++)
		{
			CdTile* ctile = GetCd(cdtype, cit->x, cit->y, false);
			ctile->destroy();
			ConnectCdAround(cdtype, cit->x, cit->y, false);
			return;
		}
	}

	for(auto fit = sel.fol.begin(); fit != sel.fol.end(); fit++)
	{
		g_foliage[ *fit ].destroy();
		return;
	}
}

void MouseLUp()
{
	if(g_mode == APPMODE_EDITOR)
	{
		int edtool = GetEdTool();
		int edact = GetEdAct();
        int elevact = GetElevAct();

        if(edtool == TOOLCAT_ELEVATION)
        {
            Vec2i from = Vec2i(g_vdrag[0].x, g_vdrag[0].y)/TILE_SIZE;
            Vec2i to = Vec2i(g_mouse3d.x, g_mouse3d.y)/TILE_SIZE;
            
            if(elevact == ELEVACT_LOWER)
                LowerTerr(from, to);
            else if(elevact == ELEVACT_RAISE)
                RaiseTerr(from, to);
            else if(elevact == ELEVACT_SPREAD)
                SpreadTerr(from, to);
        }
		else if(edact == TOOLACT_PLACE)
		{
            int ctype = GetEdCdType();
            int owner = GetPlaceOwner();
            
			if(edtool == TOOLCAT_UNITS)
				EdPlaceUnit();
			else if(edtool == TOOLCAT_BUILDINGS)
				EdPlaceBl();
			else if(edtool == TOOLCAT_CONDUITS)
				PlaceCd(ctype, owner);
			else if(edtool == TOOLCAT_FOLIAGE)
				EdPlaceFol();
		}
		else if(edact == TOOLACT_DELETE)
		{
			EdDel();
		}
	}
	else if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_localP];
		Camera* c = &g_cam;
		GUI* gui = &g_gui;

		if(g_build == BL_NONE)
		{
			gui->hide("cstr view");
			gui->hide("bl view");
			gui->hide("truck mgr");
			gui->hide("save");
			gui->hide("load");
			g_sel = DoSel();
			AfterSel(&g_sel);
		}
		else if(g_build < BL_TYPES)
		{
			if(g_canplace)
			{
				//int bid;
				//PlaceBl(g_build, Vec2i(g_vdrag[0].x/TILE_SIZE, g_vdrag[0].z/TILE_SIZE), false, g_localP, &bid);

				//InfoMess("pl", "pl");

				//if(g_netmode != NETM_SINGLE)
				{
					PlaceBlPacket pbp;
					pbp.header.type = PACKET_PLACEBL;
					pbp.player = g_localP;
					pbp.btype = g_build;
					pbp.tpos = Vec2i(g_vdrag[0].x/TILE_SIZE, g_vdrag[0].y/TILE_SIZE);

					LockCmd((PacketHeader*)&pbp);
				}
			}
			else
			{
			}

			g_build = -1;
		}
		else if(g_build >= BL_TYPES && g_build < BL_TYPES+CD_TYPES)
		{
			//Log("place r");
			PlaceCd(g_build - BL_TYPES, g_localP);
			g_build = -1;
		}
	}
}

void MouseMove(InEv* ie)
{
	if(g_mode != APPMODE_PLAY)
		return;

#if 0
	if(g_mousekeys[MOUSE_MIDDLE])
	{
		Vec2i d = g_mouse - g_mousestart;

		ViewportRotate(3, d.x, d.y);

		g_mousestart = g_mouse;
	}
#else

	Vec2i d = Vec2i(ie->x, ie->y) - Vec2i(g_width, g_height)/2;

	if(!g_arrest)
		g_pcam->rotatebymouse(d.x, d.y);

	//TODO send rotation packet

	CenterMouse();
#endif

	if(g_mode == APPMODE_PLAY || 
		g_mode == APPMODE_EDITOR)
	{
#if 0
#ifdef PLATFORM_MOBILE
		if(g_mousekeys[MOUSE_LEFT] &&
		   g_build == BL_NONE)
		{
			Scroll(-ie->dx, -ie->dy);
		}
#endif
#endif
		
#if 0
		Player* py = &g_player[g_localP];

		if(g_mousekeys[MOUSE_MIDDLE])
		{
			CenterMouse();
		}

		Vec3f ray;
		Vec3f point;
		IsoToCart(g_mouse+g_scroll, &ray, &point);
        //MapInter(&g_hmap, ray, point, &g_mouse3d);
        Vec3f line[2];
        Vec3f fint;
        line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
        line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
        if(FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		{
            g_mouse3d = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);

			if(g_mousekeys[MOUSE_LEFT])
			{
				g_vdrag[1] = g_mouse3d;

				if(g_mode == APPMODE_EDITOR &&
					GetEdTool() == TOOLCAT_BORDERS)
					EdPlaceBords();
			}
			else
			{
				g_vdrag[0] = g_mouse3d;
				g_vdrag[1] = g_mouse3d;
			}
		}

		UpdSBl();
        
        if(g_mode == APPMODE_EDITOR)
        {
            int edtool = GetEdTool();
            int edact = GetEdAct();
            int elevact = GetElevAct();
            int ctype = GetEdCdType();
            int owner = GetPlaceOwner();
            
            if(edtool == TOOLCAT_CONDUITS)
            {
                if(g_mousekeys[MOUSE_LEFT])
                {
                    UpdCdPlans(ctype, owner, g_vdrag[0], g_mouse3d);
                }
            }
        }
#endif
	}
}

void MouseRDown()
{
	if(g_mode == APPMODE_PLAY)
	{
		if(g_build != BL_NONE)
		{
			if(g_build > BL_TYPES && g_build < BL_TYPES+CD_TYPES)
				ClearCdPlans(g_build - BL_TYPES);

			g_build = BL_NONE;
		}
	}
}

void MouseRUp()
{
	if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_localP];
		Camera* c = &g_cam;

		//if(!g_keyintercepted)
		{
			Order(g_mouse.x, g_mouse.y, g_width, g_height);
		}
	}
}

void Resize_Logo(Widget *thisw)
{
	//float minsz = (float)fmin(g_width, g_height);
	float minsz = (float)120;

	thisw->m_pos[0] = g_width / 2 - minsz / 2;
	thisw->m_pos[1] = g_height / 2 - minsz / 2;
	thisw->m_pos[2] = g_width / 2 + minsz / 2;
	thisw->m_pos[3] = g_height / 2 + minsz / 2;
}

void Draw_LoadBar()
{
	float pct = (float)(g_lastLSp + g_lastLTex) / (float)(g_spriteload.size() + g_texload.size());

	EndS();
	UseS(SHADER_COLOR2D);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	
	float bar[4] =
	{
		(float)g_width/2 - 50,
		(float)g_height/2 + 30,
		(float)g_width/2 + 100,
		(float)g_height/2 + 30 + 3
	};
	
	DrawSquare(0, 0, 0, 1, bar[0]-1, bar[1]-1, bar[2]+1, bar[3]+1, g_gui.m_crop);
	DrawSquare(0, 1, 0, 1, bar[0], bar[1], bar[0] + pct * (bar[2]-bar[0]), bar[3], g_gui.m_crop);


	EndS();
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
}

void AnyKey(int k)
{
	if(g_mode == APPMODE_LOGO)
		SkipLogo();
}

#if 0
void FillGUI()
{
	//DrawSceneFunc = DrawScene;
	//DrawSceneDepthFunc = DrawSceneDepth;

	Log("2.1\r\n");
	

	for(int i=0; i<PLAYERS; i++)
	{
		Log("2.1.1");

		Player* py = &g_player[i];
		GUI* gui = &g_gui;

		Log("2.1.2");
		
		//TODO get rid of unnecessary assign funcs and just assign vars manually
		gui->assignkey(SDL_SCANCODE_F1, SaveScreenshot, NULL);
		gui->assignkey(SDL_SCANCODE_F2, LogPathDebug, NULL);
		gui->assignlbutton(MouseLDown, MouseLUp);
		gui->assignrbutton(MouseRDown, MouseRUp);
		gui->assignmousemove(MouseMove);
		gui->assignanykey(AnyKey, NULL);

		Log("2.1.3");
		
	}

	Log("2.2");
	

	// Loading ViewLayer
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	Log("2.2");
	

	
	gui->add(new ViewLayer(gui, "logo"));
	ViewLayer* logoview = (ViewLayer*)gui->get("logo");
	logoview->add(new Image(logoview, "", "gui/mmbg.jpg", true, Resize_Fullscreen));
	//logoview->add(new Image(logoview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	//logoview->add(new Text(logoview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	logoview->add(new Image(logoview, "logo", "gui/centerp/dmd.png", true, Resize_Logo, 1, 1, 1, 0));

	gui->add(new ViewLayer(gui, "loading"));
	ViewLayer* loadview = (ViewLayer*)gui->get("loading");

	Log("2.3");
	

	loadview->add(new Image(loadview, "loadbg", "gui/mmbg.jpg", true, Resize_Fullscreen));
	//loadview->add(new Image(loadview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	//loadview->add(new Text(loadview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	loadview->add(new Text(loadview, "status", RichText("Loading..."), MAINFONT8, Resize_LoadingStatus));
	loadview->add(new InsDraw(loadview, Draw_LoadBar));

	Log("2.4");
	

	FillMenu();

	Log("2.5");
	

	FillEd();	//must be done after Queue simdef's

	Log("2.6");
	

	FillPlay();

	Log("2.7");
	

	FillConsole();
	FillMess();
	
	gui->hideall();
	//gui->show("loading");
	gui->show("logo");
}
#endif