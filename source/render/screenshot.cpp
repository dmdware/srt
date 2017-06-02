


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




#include "../platform.h"
#include "../window.h"
#include "../utils.h"
#include "../texture.h"
#include "../sim/player.h"

void SaveScreenshot()
{
	Player* py = &g_player[g_localP];

	LoadedTex screenshot;
	screenshot.channels = 3;
	screenshot.sizex = g_width;
	screenshot.sizey = g_height;
	screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );

	if(!screenshot.data)
	{
		OUTOFMEM();
		return;
	}

	//memset(screenshot.data, 0, g_width * g_height * 3);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	CHECKGLERROR();
	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);
	CHECKGLERROR();

	FlipImage(&screenshot);

	char relative[256];
	std::string datetime = FileDateTime();
	//sprintf(relative, "screenshots/%s.jpg", datetime.c_str());
	sprintf(relative, "screenshots/%s.png", datetime.c_str());
	char fullpath[DMD_MAX_PATH+1];
	FullWritePath(relative, fullpath);

	Log("Writing screenshot %s\r\n", fullpath);
	

	//SaveJPEG(fullpath, &screenshot, 0.9f);
	SavePNG(fullpath, &screenshot);

	//free(screenshot.data);
}
