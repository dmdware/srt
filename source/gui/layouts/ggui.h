


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



#ifndef GGUI_H
#define GGUI_H

#include "../../math/vec3f.h"

class ViewLayer;
class Widget;

#if 0
extern bool g_canselect;
#endif

//extern char g_lastsave[DMD_MAX_PATH+1];

void Resize_Fullscreen(Widget* w);
void Resize_FullscreenSq(Widget* w);
void Resize_AppLogo(Widget* w);
void Resize_AppTitle(Widget* w);
void Click_NewGame();
void Click_OpenEd();
void FillGUI();
void Click_LoadMapButton();
void Click_SaveMapButton();
void Click_QSaveMapButton();
void Resize_MenuItem(Widget* w);
void Resize_CenterWin(Widget* w);
void EnumDisplay();
void Resize_Logo(Widget *thisw);
void MouseMove(InEv* ie);
void Resize_LoadingStatus(Widget* w);
void Draw_LoadBar();

void FillMenu();
void Click_LoadGame();
void Click_ListHosts();
void Click_HostGame();
void Click_Options();
void Resize_CopyInfo(Widget* w);

#endif	//GGUI_H
