


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



#include "icon.h"
#include "../texture.h"

Icon g_icon[ICONS];

void DefI(int type, const char* relative, const UStr tag)
{
	Icon* i = &g_icon[type];

	i->m_tag = tag;
	//QueueTexture(&i->m_tex, relative, true);
	CreateTex(i->m_tex, relative, true, false);
	Texture* t = &g_texture[i->m_tex];
	i->m_width = t->width;
	i->m_height = t->height;
}
