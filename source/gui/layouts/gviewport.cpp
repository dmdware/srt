


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





#include "../../render/shader.h"
#include "../../gui/gui.h"
#include "../../math/3dmath.h"
#include "../../window.h"
#include "../../platform.h"
#include "../../gui/font.h"
#include "../../math/camera.h"
#include "../../math/matrix.h"
#include "../../render/heightmap.h"
#include "../../math/vec4f.h"
#include "../../bsp/brush.h"
#include "../../math/frustum.h"
#include "../../sim/simdef.h"
#include "gviewport.h"
#include "../../math/hmapmath.h"
#include "../../render/water.h"
#include "../../save/savemap.h"
#include "../../gui/widgets/spez/botpan.h"
#include "../../sim/bltype.h"
#include "../../sim/utype.h"
#include "../../sim/player.h"
#include "../../debug.h"
#include "../../sim/conduit.h"

VpType g_vptype[VIEWPORT_TYPES];
Viewport g_viewport[4];
//Vec3f g_focus;

VpType::VpType(Vec3f offset, Vec3f up, const char* label, bool axial)
{
	m_offset = offset;
	m_up = up;
	strcpy(m_label, label);
	m_axial = axial;
}

#if 0
Viewport::Viewport()
{
	m_drag = false;
	m_ldown = false;
	m_rdown = false;
}

Viewport::Viewport(int type)
{
	m_drag = false;
	m_ldown = false;
	m_rdown = false;
	m_mdown = false;
	m_type = type;
}

Vec3f Viewport::up()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f upvec = c->m_up;
	VpType* t = &g_vptype[m_type];

	if(t->m_axial)
		upvec = t->m_up;

	return upvec;
}

Vec3f Viewport::up2()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f upvec = c->up2();
	VpType* t = &g_vptype[m_type];

	if(t->m_axial)
		upvec = t->m_up;

	return upvec;
}

Vec3f Viewport::strafe()
{
	Vec3f upvec = up();
	VpType* t = &g_vptype[m_type];
	Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));

	//if(!t->m_axial)
	//	sidevec = c->m_strafe;

	return sidevec;
}

Vec3f Viewport::focus()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f viewvec = c->m_view;
	return viewvec;
}

Vec3f Viewport::viewdir()
{
	Vec3f focusvec = focus();
	Vec3f posvec = pos();
	//Vec3f viewvec = posvec + Normalize(focusvec-posvec);
	//return viewvec;
	return focusvec-posvec;
}

Vec3f Viewport::pos()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f posvec = c->m_pos;

#if 0
	if(g_projtype == PROJ_PERSP && !t->m_axial)
	{
		Vec3f dir = Normalize( c->m_view - c->m_pos );
		posvec = c->m_view - dir * 1000.0f / g_zoom;
	}
#endif

	VpType* t = &g_vptype[m_type];

	if(t->m_axial)
		posvec = c->m_view + t->m_offset;

	return posvec;
}
#endif

void DrawMinimap()
{
	//g_frustum.construct(projection.m_matrix, viewmat.m_matrix);

	CHECKGLERROR();

}

void DrawPreview()
{
	Shader* s = &g_shader[g_curS];
	Player* py = &g_player[g_localP];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);
	//glUniform4f(s->slot[SSLOT_OWNCOLOR], 1, 0, 0, 0);

	Sprite* sp = NULL;

	if(g_bptype >= 0 && g_bptype < BL_TYPES)
	{
		BlType* t = &g_bltype[g_bptype];
		sp = &g_sprite[t->sprite[0]];
	}
	else if(g_bptype == BL_ROAD)
	{
		CdType* ct = &g_cdtype[CD_ROAD];
		sp = &g_sprite[ct->sprite[CONNECTION_EASTWEST][1][IN_0000]];
	}
	else if(g_bptype == BL_POWL)
	{
		CdType* ct = &g_cdtype[CD_POWL];
		sp = &g_sprite[ct->sprite[CONNECTION_EASTWEST][1][IN_0000]];
	}
	else if(g_bptype == BL_CRPIPE)
	{
		CdType* ct = &g_cdtype[CD_CRPIPE];
		sp = &g_sprite[ct->sprite[CONNECTION_EASTWEST][1][IN_0000]];
	}

	if(!sp)
		return;

}
