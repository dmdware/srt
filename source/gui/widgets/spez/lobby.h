


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





#ifndef LOBBY_H
#define LOBBY_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../viewlayer.h"
#include "../../../sim/selection.h"
#include "../winw.h"
#include "../../layouts/chattext.h"

//TODO move lobby to layouts

void Lobby_Regen();
void FillLobby();
void Lobby_DrawPyL();
void Lobby_DrawState();
void Click_JoinCancel();

#endif
