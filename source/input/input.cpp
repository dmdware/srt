

#include "input.h"
#include "../math/camera.h"
#include "../app/appmain.h"
#include "../utils.h"

Vec3f g_throttle;

#define ED_CAM_SPEED			20.0f

void UpdIn()
{
	if(g_mode == APPMODE_EDITOR)
	{
		Camera* c = &g_cam;
		Vec3f t;

		t.x += 1.0 * g_keys[SDL_SCANCODE_D];
		t.x -= 1.0 * g_keys[SDL_SCANCODE_A];
		
		t.z += 1.0 * g_keys[SDL_SCANCODE_W];
		t.z -= 1.0 * g_keys[SDL_SCANCODE_S];
		
		t.y += 1.0 * g_keys[SDL_SCANCODE_R];
		t.y -= 1.0 * g_keys[SDL_SCANCODE_F];
		
		c->accelerate(t.z * ED_CAM_SPEED/g_zoom);
		c->accelstrafe(t.x * ED_CAM_SPEED/g_zoom);
		c->accelrise(t.y * ED_CAM_SPEED/g_zoom);
		c->limithvel(ED_CAM_SPEED/g_zoom);
	}
}

void Down_Forward()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Up_Forward()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Down_Backward()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Up_Backward()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Down_Left()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Up_Left()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Down_Right()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Up_Right()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Down_Rise()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Up_Rise()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Down_Fall()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}

void Up_Fall()
{
	if(g_mode == APPMODE_EDITOR)
	{
	}
}