


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




#ifndef PLATFORM_H
#define PLATFORM_H

//#define MATCHMAKER		//uncomment this line for matchmaker server
//#define USESTEAM			//uncomment this line for steam version

#ifdef _WIN32
#define PLATFORM_GL14
#define PLATFORM_WIN
#endif

#if __APPLE__

#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define PLATFORM_GL14
#define PLATFORM_MAC
#endif
#if TARGET_OS_IPHONE
#define PLATFORM_IOS
#define PLATFORM_IPHONE
#define PLATFORM_MOBILE
#define PLATFORM_GLES20
#undef PLATFORM_GL14
#endif
#if TARGET_OS_IPAD
#define PLATFORM_IOS
#define PLATFORM_IPAD
#define PLATFORM_MOBILE
#define PLATFORM_GLES20
#undef PLATFORM_GL14
#endif

#endif

#if defined( __GNUC__ )
//#define PLATFORM_LINUX
#endif
#if defined( __linux__ )
#define PLATFORM_LINUX
#define PLATFORM_GL14
#endif
#if defined ( __linux )
#define PLATFORM_LINUX
#define PLATFORM_GL14
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#ifdef PLATFORM_WIN
#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
//#include <dirent.h>
#include "../libs/win/dirent-1.20.1/include/dirent.h"
#endif

#ifdef PLATFORM_LINUX
/* POSIX! getpid(), readlink() */
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
//file listing dirent
#include <dirent.h>
//htonl
#include <arpa/inet.h>
#include <sys/time.h>
#endif

#if defined(PLATFORM_MAC) && !defined(PLATFORM_IOS)
#include <sys/types.h>
#include <sys/dir.h>
//htonl
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#if defined(PLATFORM_IOS)
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>	//mkdir
#endif

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <fstream>
#include <iostream>
#include <math.h>
#include <limits.h>

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

#ifdef PLATFORM_WIN
#include <jpeglib.h>
#include <png.h>
#include <zip.h>
#endif

#ifdef PLATFORM_LINUX
#include <jpeglib.h>
#include <png.h>
#endif
//#define NO_SDL_GLEXT

#ifdef PLATFORM_MAC

#if 0
// https://trac.macports.org/ticket/42710
#ifndef FALSE            /* in case these macros already exist */
#define FALSE   0        /* values of boolean */
#endif
#ifndef TRUE
#define TRUE    1
#endif
#define HAVE_BOOLEAN

#endif

#ifdef PLATFORM_IOS
/*
 Use User Header Search Paths !
(Or else jpeglib.h from system folders will be used, version mismatch)
*/
#include "jpeglib.h"
#include "png.h"

#else

#include <jpeglib.h>
#include <png.h>
//#include <zip.h>
#endif

#endif

#ifndef MATCHMAKER
#ifdef PLATFORM_WIN
#include <GL/glew.h>
#endif
#endif

#ifndef MATCHMAKER
#ifdef PLATFORM_LINUX
//#include <GL/xglew.h>
#include <GL/glew.h>
#endif
#endif

//#define GL_GLEXT_PROTOTYPES

#if 1

#ifdef PLATFORM_LINUX
#include <SDL2/SDL.h>
#ifndef MATCHMAKER
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_mixer.h>
//#include <GL/glut.h>
#endif
#include <SDL2/SDL_net.h>
#endif

#if defined(PLATFORM_MAC) && !defined(PLATFORM_IOS)
#ifndef MATCHMAKER
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif
#endif

#ifdef PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#endif

#if defined(PLATFORM_MAC) && !defined(PLATFORM_IOS)
//#include <GL/xglew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
//#include <SDL2/SDL_net.h>
//#include <SDL2/SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#endif

#if defined(PLATFORM_IOS)
//#include <GL/xglew.h>
#include "SDL.h"
#include "SDL_opengles2.h"
//#include <SDL2/SDL_net.h>
//#include <SDL2/SDL_mixer.h>
#include "SDL_net.h"
#include "SDL_mixer.h"
#endif

#ifdef PLATFORM_WIN
#include <GL/wglew.h>
#include <SDL.h>
#ifndef MATCHMAKER
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#endif
#include <SDL_net.h>
#endif

#endif

#ifdef PLATFORM_WIN
#ifndef MATCHMAKER
#include <gl/glaux.h>
#endif
#endif

extern "C" {
#define __STDC_CONSTANT_MACROS
//#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
	
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
};

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#ifdef USESTEAM
#include <steam_api.h>
#include <isteamuserstats.h>
#include <isteamremotestorage.h>
#include <isteammatchmaking.h>
#include <steam_gameserver.h>
#endif

#if 0
#ifdef PLATFORM_WIN
#include "drdump/CrashRpt.h"

extern crash_rpt::CrashRpt g_crashRpt;
#endif
#endif

#ifdef PLATFORM_WIN
#pragma comment(lib, "x86/SDL2.lib")
#pragma comment(lib, "x86/SDL2main.lib")
//#pragma comment(lib, "SDL.lib")
//#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "x86/SDL2_net.lib")
#pragma comment(lib, "x86/SDL2_mixer.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "libpng15.lib")
#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "zipstatic.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "sdkencryptedappticket.lib")
#endif

#ifndef DMD_MAX_PATH
#define DMD_MAX_PATH 1024
#endif

#ifndef PLATFORM_WIN
#define SOCKET int
typedef unsigned char byte;
typedef unsigned int UINT;
typedef int16_t WORD;
#define _isnan isnan
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define ERROR 0
#define APIENTRY
#endif

#ifdef PLATFORM_MAC
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif

#ifdef PLATFORM_WIN
#define stricmp _stricmp
#endif

/*
 #ifndef _isnan
 int _isnan(double x) { return x != x; }
 #endif
 */
#ifdef PLATFORM_WIN
extern HINSTANCE g_hInstance;
#endif

//#ifndef MATCHMAKER
extern SDL_Window *g_window;
extern SDL_Renderer* g_renderer;
extern SDL_GLContext g_glcontext;
//#endif

#ifndef MATCHMAKER
//#include "../objectscript/objectscript.h"
#endif

#define SPECBUMPSHADOW

#define GLDEBUG
//#define DEBUGLOG

#define CHECKGLERROR() CheckGLError(__FILE__,__LINE__)

#ifndef GLDEBUG
#define CheckGLError(a,b); (void)0;
#endif

//#define FREEZE_DEBUG
//#define RANDOM8DEBUG

//#define DEMO		//is this a time-restricted version?
#define DEMOTIME		(5*60*1000)

// ???
#ifdef STEAM_CEG
// Steam DRM header file
#include "cegclient.h"
#else
#define Steamworks_InitCEGLibrary() (true)
#define Steamworks_TermCEGLibrary()
#define Steamworks_TestSecret()
#define Steamworks_SelfCheck()
#endif

#endif // #define LIBRARY_H
