


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




#include "platform.h"

#ifdef PLATFORM_WIN
HINSTANCE g_hInstance = NULL;
#endif

SDL_Window *g_window;
SDL_Renderer* g_renderer;
SDL_GLContext g_glcontext;

#ifdef PLATFORM_WIN

#if 0
crash_rpt::CrashRpt g_crashRpt(
	"1a119d97-a82d-4284-b3ae-ad5f20d18586", // GUID assigned to this application.
	L"States, Firms, and Households v 0.0.10a0", // Application name that will be used in message box.
	L"DMD 'Ware"       // Company name that will be used in message box.
	);
#endif

#endif