


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



#ifndef PLAYGUI_H
#define PLAYGUI_H

class Widget;

void FillPlay();
void BuildMenu_OpenPage1();
void BuildMenu_OpenPage2();
void BuildMenu_OpenPage3();
void Click_RightMenu_BackToOpener();
void UpdResTicker();
void ShowMessage(const RichText& msg);
void Resize_Window(Widget* w);
void Resize_BlPreview(Widget* w);
void Click_Pause();

#endif
