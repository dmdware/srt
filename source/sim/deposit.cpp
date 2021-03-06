


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




#include "deposit.h"
#include "resources.h"
#include "../math/3dmath.h"
#include "../render/shader.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../gui/icon.h"
#include "../math/hmapmath.h"
#include "../gui/gui.h"
#include "../math/vec4f.h"
#include "player.h"

Deposit g_deposit[DEPOSITS];

Deposit::Deposit()
{
	on = false;
	occupied = false;
}

void FreeDeposits()
{
	for(int i=0; i<DEPOSITS; i++)
	{
		g_deposit[i].on = false;
		g_deposit[i].occupied = false;
	}
}

void DrawDeposits(const Matrix projection, const Matrix viewmat)
{
	if(g_mapsz.x <= 0 || g_mapsz.y <= 0)
		return;

	Shader* s = &g_shader[g_curS];

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[IN_SAND] ].texname);
	//glUniform1i(s->slot[SSLOT_SANDTEX], 0);

	Matrix mvp;
	mvp.set(projection.m_matrix);
	mvp.postmult(viewmat);

	Player* py = &g_player[g_localP];

	for(int i=0; i<DEPOSITS; i++)
	{
		Deposit* d = &g_deposit[i];

		if(!d->on)
			continue;

		if(d->occupied)
			continue;

		Vec3f pos = d->drawpos;
#if 0
		pos.x = d->tpos.x*TILE_SIZE + TILE_SIZE/2.0f;
		pos.z = d->tpos.y*TILE_SIZE + TILE_SIZE/2.0f;
		pos.y = Bilerp(&g_hmap, pos.x, pos.z);
#endif
		//Vec4f ScreenPos(Matrix* mvp, Vec3f vec, float width, float height)
		Vec4f spos = ScreenPos(&mvp, pos, g_currw, g_currh, true);

		Resource* res = &g_resource[d->restype];
		Icon* ic = &g_icon[res->icon];

		DrawImage(g_texture[ ic->m_tex ].texname, spos.x - 25, spos.y - 25, spos.x + 25, spos.y + 25, 0,0,1,1, g_gui.m_crop);
		//DrawShadowedText(MAINFONT8, spos.x, spos.y, &dep->m_label);
		//DrawCenterShadText(MAINFONT8, spos.x, spos.y, &dep->m_label);
	}
}
