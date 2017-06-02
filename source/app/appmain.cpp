


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



#include "appmain.h"
#include "../gui/gui.h"
#include "../gui/keymap.h"
#include "../render/shader.h"
#include "appres.h"
#include "../gui/font.h"
#include "../texture.h"
#include "../math/frustum.h"
#include "../gui/layouts/ggui.h"
#include "../gui/gui.h"
#include "../debug.h"
#include "../render/heightmap.h"
#include "../sim/map.h"
#include "../math/camera.h"
#include "../window.h"
#include "../utils.h"
#include "../sim/simdef.h"
#include "../math/hmapmath.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/build.h"
#include "../sim/bltype.h"
#include "../render/foliage.h"
#include "../render/water.h"
#include "../sim/deposit.h"
#include "../sim/selection.h"
#include "../sim/player.h"
#include "../sim/order.h"
#include "../render/transaction.h"
#include "../path/collidertile.h"
#include "../path/pathdebug.h"
#include "../gui/layouts/playgui.h"
#include "../gui/widgets/spez/botpan.h"
#include "../texture.h"
#include "../trigger/script.h"
#include "../ai/ai.h"
#include "../net/lockstep.h"
#include "../sim/simflow.h"
#include "../sim/transport.h"
#include "../render/infoov.h"
#include "../sound/soundch.h"
#include "../net/netconn.h"
#include "../econ/demand.h"
#include "../save/savemap.h"
#include "../render/graph.h"
#include "../path/tilepath.h"
#include "../render/drawqueue.h"
#include "../sys/namegen.h"
#include "../net/client.h"
#include "../sys/unicode.h"
#include "../ustring.h"
#include "../gui/layouts/edgui.h"
#include "../language.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "../save/modelholder.h"
#include "../render/shadow.h"
#include "../save/edmap.h"
#include "../gui/layouts/segui.h"
#include "../bsp/drawmap.h"
#include "../input/input.h"
#include "../sim/entity.h"
#include "../tool/procvid.h"
#include "../tool/estview.h"

//int g_mode = APPMODE_LOADING;
int g_mode = APPMODE_LOGO;

std::string g_startmap = "";
std::string g_invideo = "D:/proj/st/testfolder/download/IMG_3885.MOV";
//std::string g_invideo = "D:/proj/st/testfolder/download/path2 2015-10-11 18-47-20-88.avi";

//static long long g_lasttime = GetTicks();

#ifdef DEMO
static long long g_demostart = GetTicks();
#endif

void SkipLogo()
{
	if(g_mode != APPMODE_LOGO)
		return;

	g_mode = APPMODE_LOADING;
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("loading");
}

void UpdLogo()
{
	static int stage = 0;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(stage < 60)
	{
		float a = (float)stage / 60.0f;
		gui->get("logo")->get("logo")->m_rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		gui->get("logo")->get("logo")->m_rgba[3] = a;
	}
	else
		SkipLogo();

	stage++;
}

int g_restage = 0;
void UpdLoad()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	switch(g_restage)
	{
	case 0:
		if(!Load1Sprite()) g_restage++;
		break;
	case 1:
		if(!Load1Texture())
		{
			g_spriteload.clear();
			g_texload.clear();
			g_lastLSp = -1;
			g_lastLTex = -1;
			
			if(g_startmap.size() > 0)
			{
				std::string relative = MakeRelative(g_startmap.c_str());

				g_supresserr = true;

				if(!LoadMap(relative.c_str()))
				{
					char m[DMD_MAX_PATH+128];
					sprintf(m, "Failed to load map: %s", relative.c_str());
					ErrMess("Error", m);
					g_quit = true;
					return;
				}

				//std::string relmap = MakeRelative(g_startmap.c_str());
				//LoadMap(relmap.c_str());
				g_mode = APPMODE_PLAY;
				gui->hideall();
				gui->show("play");
			}
			else if(g_invideo.size() > 0)
			{
				char fullpath[DMD_MAX_PATH+1];
				std::string relpath = MakeRelative(g_invideo.c_str());
				FullPath(relpath.c_str(), fullpath);
				ProcVid(g_invideo.c_str());
				g_mode = APPMODE_PROCVID;
			}
			else
			{
				g_mode = APPMODE_MENU;
				//g_mode = APPMODE_EDITOR;
				gui->hideall();
				gui->show("menu");
				//gui->show("editor");
				//g_mode = APPMODE_PLAY;
				//Click_NewGame();
				//Click_OpenEd();
				Click_MapEditor();
			}
		}
		break;
	}
}

void UpdReload()
{
#if 0
	switch(g_reStage)
	{
	case 0:
		if(!Load1Texture())
		{
			g_mode = APPMODE_MENU;
		}
		break;
	}
#else
	g_restage = 0;
	g_lastLTex = -1;
	//g_lastmodelload = -1;
	g_lastLSp = -1;
	g_gui.freech();
	//FreeModels();
	FreeSprites();
	FreeTextures();
	BreakWin(TITLE.rawstr().c_str());
	//ReloadTextures();
	MakeWin(TITLE.rawstr().c_str());
#ifdef PLATFORM_MOBILE
	int width = g_width;
	int height = g_height;
	//halve for mobile small screen so it appears twice as big
	g_width >>= 1;
	g_height >>= 1;
#endif
	//ReloadModels();	//Important - VBO only possible after window GL context made.
	g_mode = APPMODE_LOADING;
	//LoadFonts();	//already called in MakeWin
	//ReloadTextures();
	LoadSysRes();
	Queue();
	FillGUI();
	char exepath[DMD_MAX_PATH+1];
	ExePath(exepath);
	Log("ExePath %s\r\n", exepath);
	
	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("loading");

	/*
	TODO
	Overhaul ui system
	Make scrollable everywhere
	Croppable pipeline 
	*/
	
#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif
#endif
}

void UpdSim()
{
	UpdIn();

    //UpdAI();
    UpdPys();
    //Animate();
	Physics();
	//CheckFuncs();
	//UpdateObjects();
	//UpdateGUI();
	//UpdateParticles();
	//UpdateDecals();
}

void UpdEd()
{
	UpdIn();

	g_cam.frameupd();
	g_cam.friction2();

#if 0
	UpdateFPS();
#endif
}

void Update()
{
	//if(g_netmode != NETM_SINGLE)
	if(g_sock)
		UpdNet();

	if(g_mode == APPMODE_LOGO)
		UpdLogo();
	//else if(g_mode == APPMODE_INTRO)
	//	UpdateIntro();
	else if(g_mode == APPMODE_LOADING)
		UpdLoad();
	else if(g_mode == APPMODE_RELOADING)
		UpdReload();
	else if(g_mode == APPMODE_PLAY)
		UpdSim();
	else if(g_mode == APPMODE_EDITOR)
		UpdEd();

#ifdef DEMO
	if(GetTicks() - g_demostart > DEMOTIME)
		g_quit = true;
#endif
}



void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif
#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 0
    if(g_mode == RENDERING)
    {
        //SwapBuffers(g_hDC);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
        g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        CheckGLError(__FILE__, __LINE__);
        glViewport(0, 0, g_width, g_height);
        CheckGLError(__FILE__, __LINE__);
        glClearColor(1.0, 1.0, 1.0, 0.0);
        CheckGLError(__FILE__, __LINE__);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGLError(__FILE__, __LINE__);
        g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
    }
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 1
	if(g_projtype == PROJ_ORTHO)
		UseShadow(SHADER_MODEL, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	else
		UseShadow(SHADER_MODELPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1i(g_shader[SHADER_OWNED].slot[SSLOT_TEXTURE1], 1);
    glActiveTexture(GL_TEXTURE0);*/
    DrawModelHolders();
	DrawEntities(false);
	DrawEntities(true);
#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
	//if(g_model2[0].m_on)
	//	g_model2[0].Render();
    EndS();
#endif

#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

#if 1
	if(g_projtype == PROJ_ORTHO)
		UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	else
		UseShadow(SHADER_MAPPERSP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
#else
	UseShadow(SHADER_LIGHTTEST, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	glUniformMatrix4fv(g_shader[SHADER_LIGHTTEST].slot[SSLOT_MVP], 1, GL_FALSE, g_lightmat.m_matrix);
#endif
    CheckGLError(__FILE__, __LINE__);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
#if 0	//no tiles for path2
	DrawTile();
	Matrix offmat;
	Vec3f offvec(-g_tilesize, 0, -g_tilesize);
	offmat.translation((float*)&offvec);
	glUniformMatrix4fv(g_shader[g_curS].slot[SSLOT_MODELMAT], 1, GL_FALSE, offmat.m_matrix);
	//DrawTile();
	offvec = Vec3f(-g_tilesize, 0, 0);
	offmat.translation((float*)&offvec);
	glUniformMatrix4fv(g_shader[g_curS].slot[SSLOT_MODELMAT], 1, GL_FALSE, offmat.m_matrix);
	//DrawTile();
#endif
    CheckGLError(__FILE__, __LINE__);
	if(g_mode == APPMODE_EDITOR)
		DrawEdMap(&g_edmap, g_showsky);
	else if(g_mode == APPMODE_PLAY)
		DrawMap(&g_map);
    CheckGLError(__FILE__, __LINE__);
    EndS();

#if 0
	glDisable(GL_DEPTH_TEST);
    UseShadow(SHADER_COLOR3D, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	Matrix mvp;
	mvp.set(projection.m_matrix);
	mvp.postmult(viewmat);
	glUniformMatrix4fv(g_shader[g_curS].slot[SSLOT_MVP], 1, GL_FALSE, mvp.m_matrix);
	glUniform4f(g_shader[g_curS].slot[SSLOT_COLOR], 0, 0, 1, 0.2f);
	VertexArray* va = &g_tileva[g_currincline];
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	//glDrawArrays(GL_LINE_LOOP, 0, va->numverts);
	glUniform4f(g_shader[g_curS].slot[SSLOT_COLOR], 1, 0, 0, 0.2f);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	glDrawArrays(GL_POINTS, 0, va->numverts);
    EndS();
	glEnable(GL_DEPTH_TEST);
#endif

#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

#if 0
    if(g_mode == RENDERING)
    {
        CheckGLError(__FILE__, __LINE__);
        glFlush();
        CheckGLError(__FILE__, __LINE__);
        //glFinish();
        CheckGLError(__FILE__, __LINE__);
        SaveRender();
        CheckGLError(__FILE__, __LINE__);
    }
#endif
}

void DrawSceneTeam(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{

#if 0
#if 1
    UseShadow(SHADER_TEAM, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1i(g_shader[SHADER_OWNED].slot[SSLOT_TEXTURE1], 1);
    glActiveTexture(GL_TEXTURE0);*/
    DrawModelHolders();
#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
    EndS();
#endif

    UseShadow(SHADER_TEAM, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    CheckGLError(__FILE__, __LINE__);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1i(g_shader[g_curS].slot[SSLOT_SHADOWMAP], 4);
	DrawTile();
    CheckGLError(__FILE__, __LINE__);
    DrawEdMap(&g_edmap, g_showsky);
    CheckGLError(__FILE__, __LINE__);
    EndS();
#endif
}

void DrawSceneDepth()
{
    //g_model[themodel].draw(0, Vec3f(0,0,0), 0);
#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    //DrawModelHoldersDepth();
    DrawModelHolders();

#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif
    //if(g_model[0].m_on)
    //	g_model[0].draw(0, Vec3f(0,0,0), 0);

    DrawEdMapDepth(&g_edmap, false);
  //  DrawEdMap(&g_edmap, false);/*
	//for(int i=0; i<10; i++)
	//	for(int j=0; j<5; j++)
		//	g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/

	//DrawTile();
#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif
#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
}

// get a power of 2 number that is big enough to hold 'lowerbound' but does not exceed 2048
int Max2Pow(int lowerbound)
{
	int twopow = 2;
	
	while( twopow < lowerbound
#if 0
		  && twopow < 2048
#endif
		  )
		twopow *= 2;
	
	return twopow;
}

int Max2Pow32(int lowerbound)
{
	int twopow = 32;
	
	while( twopow < lowerbound
#if 0
		  && twopow < 2048
#endif
		  )
		twopow *= 2;
	
	return twopow;
}

void MakeFBO(unsigned int* rendertex, unsigned int* renderrb, unsigned int* renderfb, unsigned int* renderdepthtex, int w, int h)
{
#if 0   //OpenGL 3.0 way
#if 1
	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	CHECKGLERROR();
#if 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	CheckGLError(__FILE__, __LINE__);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, renderrb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, *renderrb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
	//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	CheckGLError(__FILE__, __LINE__);

	glGenFramebuffersEXT(1, renderfb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *renderfb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
	//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CheckGLError(__FILE__, __LINE__);
#else
	//RGBA8 2D texture, 24 bit depth texture, 256x256
	glGenTextures(1, &g_rendertex);
	glBindTexture(GL_TEXTURE_2D, g_rendertex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//NULL means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	//-------------------------
	glGenFramebuffersEXT(1, &g_renderfb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
	//Attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
	//-------------------------
	glGenRenderbuffersEXT(1, &g_renderrb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
	//-------------------------
	//Attach depth buffer to FBO
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
	//-------------------------
#endif
	
#elif defined(PLATFORM_MOBILE)	//for mobile
	
	glGenFramebuffers(1, renderfb);
	glBindFramebuffer(GL_FRAMEBUFFER, *renderfb);
	
	//NSLog(@"1 glGetError() = %d", glGetError());
	
	w = Max2Pow(w);
	h = Max2Pow(h);
	
	w = 512;
	h = 512;
	
	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	//NSLog(@"2 glGetError() = %d", glGetError());
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *rendertex, 0);
	
	//NSLog(@"3 glGetError() = %d", glGetError());
	/*
	glGenRenderbuffers(1, renderrb);
	glBindRenderbuffer(GL_RENDERBUFFER, *renderrb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *renderrb);
	
	NSLog(@"4 glGetError() = %d", glGetError());
	
	glGenRenderbuffers(1, renderdepthtex);
	glBindRenderbuffer(GL_RENDERBUFFER, *renderdepthtex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *renderdepthtex);
	
	NSLog(@"5 glGetError() = %d", glGetError());
	*/
	unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		char mess[128];
		sprintf(mess, "Couldn't create framebuffer for render, error: %u.", status);
		ErrMess("Error", mess);
		return;
	}
	
	//InfoMess("suc","fbosuc");
	
#else   //OpenGL 1.4 way

	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
#ifndef PLATFORM_MOBILE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_OES, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//g_applog<<"gge1 "<<glGetError());

	glGenTextures(1, renderdepthtex);
	glBindTexture(GL_TEXTURE_2D, *renderdepthtex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#ifndef PLATFORM_MOBILE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//g_applog<<"gge2 "<<glGetError());
	//glDrawBuffer(GL_NONE); // No color buffer is drawn
	//glReadBuffer(GL_NONE);

	glGenFramebuffers(1, renderfb);
	glBindFramebuffer(GL_FRAMEBUFFER, *renderfb);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *rendertex, 0);
#ifndef PLATFORM_MOBILE
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *renderdepthtex, 0);
#endif

	//g_applog<<"gge3 "<<glGetError());

#ifndef PLATFORM_MOBILE
	//glDrawBuffers requires OpenGL ES 3.0
	//GL_DEPTH_ATTACHMENT_EXT
	GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
	glDrawBuffers(1, DrawBuffers);
#else
	GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
#endif
	
	//g_applog<<"gge4 "<<glGetError());

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		ErrMess("Error", "Couldn't create framebuffer for render.");
		return;
	}
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#ifdef APPDEBUG
	g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE));
	g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE));
#endif
}

void DelFBO(unsigned int* rendertex, unsigned int* renderrb, unsigned int* renderfb, unsigned int* renderdepthtex)
{
#if 0
	CHECKGLERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECKGLERROR();
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	CHECKGLERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECKGLERROR();
	glDeleteFramebuffers(1, &g_renderfb);
	CHECKGLERROR();
	glDeleteRenderbuffers(1, &g_renderrb);
	CHECKGLERROR();
	glDeleteTextures(1, &g_rendertex);
	CHECKGLERROR();
	
#elif defined(PLATFORM_MOBILE)
	
	glDeleteTextures(1, rendertex);
	//glDeleteRenderbuffers(1, renderrb);
	glDeleteFramebuffers(1, renderfb);
	//glDeleteRenderbuffers(1, renderdepthtex);
	CHECKGLERROR();
	
#else
	
	//Delete resources
	glDeleteTextures(1, rendertex);
#ifndef PLATFORM_MOBILE
	glDeleteTextures(1, renderdepthtex);
#endif
#ifdef PLATFORM_MOBILE
	//corpd fix sped fix
	glDeleteRenderbuffers(1, renderrb);
#endif
#ifndef PLATFORM_MOBILE
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
	glDeleteFramebuffers(1, renderfb);
	CHECKGLERROR();
#endif
}

#if 0
void Draw()
{
	if(g_quit)
		return;
	
#if 0
#ifdef PLATFORM_MOBILE
	glFlush();
	glFinish();
	
	Vec2i zoombox[2];
	
	zoombox[0].x = g_mouse.x - ZOOMBOX;
	zoombox[0].y = g_mouse.y - ZOOMBOX;
	zoombox[1].x = g_mouse.x + ZOOMBOX;
	zoombox[1].y = g_mouse.y + ZOOMBOX;
	
	zoombox[0].x = imax(0, zoombox[0].x);
	zoombox[0].y = imax(0, zoombox[0].y);
	zoombox[1].x = imin(g_width-1, zoombox[1].x);
	zoombox[1].y = imin(g_height-1, zoombox[1].y);
	
	LoadedTex zoompix;
	zoompix.sizex = zoombox[1].x - zoombox[0].x;
	zoompix.sizey = zoombox[1].y - zoombox[0].y;
	zoompix.channels = 3;
	zoompix.data = (unsigned char*)malloc( sizeof(unsigned char) * 3 * zoompix.sizex * zoompix.sizey );
	
	//glReadBuffer( GL_FRONT );
	glReadPixels(zoombox[0].x, zoombox[0].y, zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
	//CreateTex(&zoompix, &zoomtex, true, false);
	
	glBindTexture(GL_TEXTURE_2D, g_zoomtex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, zoombox[0].x - (g_mouse.x - ZOOMBOX), zoombox[0].y - (g_mouse.y - ZOOMBOX), zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
	
#endif
#endif
	
	StartTimer(TIMER_DRAWSETUP);

	CHECKGLERROR();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECKGLERROR();

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	Camera* c = &g_cam;

	StopTimer(TIMER_DRAWSETUP);

#ifdef DEBUGLOG
	Log("draw "<<__FILE__<<" "<<__LINE__);
	
#endif

#if 2
	if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
	{
		StartTimer(TIMER_DRAWSETUP);

		StopTimer(TIMER_DRAWSETUP);
		CHECKGLERROR();
		//Ortho(g_width, g_height, 1, 1, 1, 1);
#if 1

#if 0
		glClear(GL_DEPTH_BUFFER_BIT);
		unsigned int rendertex;
		unsigned int renderrb;
		unsigned int renderfb;
		unsigned int renderdepthtex;
		MakeFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex, 2048, 2048);
		glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

#ifdef PLATFORM_MOBILE
		glDisable(GL_DEPTH_TEST);
#endif
#ifndef PLATFORM_MOBILE
		glEnable(GL_DEPTH_TEST);
#endif
		DrawQueue();
#ifndef PLATFORM_MOBILE
		glDisable(GL_DEPTH_TEST);
#endif

#if 0
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECKGLERROR();
#if 1
		Ortho(g_width, g_height, 1, 1, 1, 1);
		DrawImage(rendertex, 0, 0, 512, 512, 0, 1, 1, 0, g_gui.m_crop);
		//DrawImage(rendertex, 0, 0, 2048, 2048, 0, 0, 1, 1, g_gui.m_crop);
		EndS();
#endif
		glFlush();
		DelFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex);
#endif

		Ortho(g_width, g_height, 1, 1, 1, 1);
		DrawOv();
		DrawTransx();
		UpdParts();
		SortBillboards();
		DrawBillboards();
		EndS();
#endif

#if 0
		//draw protectionism graphs
		UseS(SHADER_COLOR2D);
		Shader* s = &g_shader[g_curS];
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		float graphbounds[4];
		graphbounds[0] = (float)g_width/2;
		graphbounds[1] = (float)g_height/2 - 200;
		graphbounds[2] = (float)g_width - 30;
		graphbounds[3] = (float)g_height/2 + 130;
		float highest=0;
		//for(int i=1; i<PLAYERS; i+=(FIRMSPERSTATE+1))
		for(int i=1; i<(FIRMSPERSTATE+1)*4; i+=(FIRMSPERSTATE+1))
		{
			for(auto pit=g_protecg[i].points.begin(); pit!=g_protecg[i].points.end(); pit++)
			{
				highest = fmax(highest,  *pit);
				highest = fmax(highest,  -*pit);
			}
		}
		for(int i=1; i<(FIRMSPERSTATE+1)*4; i+=(FIRMSPERSTATE+1))
		{
			float* c = g_player[i].color;
			//glUniform4f(s->slot[SSLOT_COLOR], c[0], c[1], c[2], 1);
			DrawGraph(&g_protecg[i], graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], highest, c);
		}
		EndS();
#endif

#if 0

#define PARTDIFFS	6

		static unsigned int spheredepth;
		static unsigned int partdiffuse[6];
		static bool loaded = false;

		static unsigned int oildepth;
		static unsigned int oildiffuse;

		if(!loaded)
		{
			loaded = true;


			CreateTex(oildepth, "sprites/bl/oilref/oilref_fr000_depth.png", true, false, false);
			CreateTex(oildiffuse, "sprites/bl/oilref/oilref_fr000.png", true, false, false);

			CreateTex(spheredepth, "billboards/sphere2_depth.png", true, false, false);
			CreateTex(partdiffuse[0], "billboards/bloodpart.png", true, false, false);
			CreateTex(partdiffuse[1], "billboards/bloodsplat.png", true, false, false);
			CreateTex(partdiffuse[2], "billboards/fireball.png", true, false, false);
			CreateTex(partdiffuse[3], "billboards/fireball2.png", true, false, false);
			CreateTex(partdiffuse[4], "billboards/flame.png", true, false, false);
			CreateTex(partdiffuse[5], "billboards/flame.png", true, false, false);

#if 1
			CreateTex(partdiffuse[1], "billboards/fog.png", true, false, false);
			CreateTex(partdiffuse[2], "billboards/fog.png", true, false, false);
			CreateTex(partdiffuse[3], "billboards/exhaust2.png", true, false, false);
			CreateTex(partdiffuse[4], "billboards/exhaust.png", true, false, false);
			CreateTex(partdiffuse[5], "billboards/fog.png", true, false, false);
#endif

#if 1
			CreateTex(partdiffuse[0], "billboards/sphere.png", true, false, false);
			CreateTex(partdiffuse[1], "billboards/sphere.png", true, false, false);
			CreateTex(partdiffuse[2], "billboards/sphere.png", true, false, false);
			CreateTex(partdiffuse[3], "billboards/sphere.png", true, false, false);
			CreateTex(partdiffuse[4], "billboards/sphere.png", true, false, false);
			CreateTex(partdiffuse[5], "billboards/sphere.png", true, false, false);
#endif
		}

		static Vec2f chpos(0,0);

		unsigned int rendertex;
		unsigned int renderrb;
		unsigned int renderfb;
		unsigned int renderdepthtex;

		glEnable(GL_DEPTH_TEST);

#if 0
		glClear(GL_DEPTH_BUFFER_BIT);

		MakeFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex, 2048, 2048);
		glBindFramebuffer(GL_FRAMEBUFFER, renderfb);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

		EndS();
		UseS(SHADER_DEEPORTHO);

		//glDisable(GL_DEPTH_TEST);

		DrawDepth(
			g_texture[ oildiffuse ].texname,
			g_texture[ oildepth ].texname,
			renderdepthtex, renderfb,
			0,
			g_width/2 + chpos.x - 64,
			g_height/2 + chpos.y - 256,
			g_width/2 + chpos.x + 64,
			g_height/2 + chpos.y + 256,
			0, 0, 1, 1);

		DrawDepth(
			g_texture[ oildiffuse ].texname,
			g_texture[ oildepth ].texname,
			renderdepthtex, renderfb,
			0,
			g_width/2 - 64,
			g_height/2 - 256,
			g_width/2 + 64,
			g_height/2 + 256,
			0, 0, 1, 1);

		chpos.x += (rand()%20 - 10) * g_drawfrinterval / 1.0f;
		chpos.y += (rand()%20 - 10) * g_drawfrinterval / 1.0f;


		EndS();

		Vec3f start = Vec3f(-256,-128,-1000) / 2.0f;

		static Vec3f ppos[6] =
		{
			Vec3f(0,0,0),
			Vec3f(0,0,0),
			start,
			start + Normalize(start) * -100.0f / 20.0f,
			start + Normalize(start) * -150.0f / 20.0f,
			start + Normalize(start) * -270.0f / 20.0f
		};

#if 0
		static float angle = 0;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		for(int i=0; i<PARTDIFFS; i++)
		{
			if(i < 1)
			{
				DrawSphericalBlend(
					g_texture[ partdiffuse[i%PARTDIFFS] ].texname,
					g_texture[ spheredepth ].texname,
					renderdepthtex, renderfb,
					ppos[i].z,
					g_width/2 + ppos[i].x,
					g_height/2 + ppos[i].y,
					64, i == 0 ? angle : DEGTORAD(360.0f)-angle,
					0, 0, 1, 1 );

				ppos[i].x += (rand()%20 - 10) / 10.0f;
				ppos[i].y += (rand()%20 - 10) / 10.0f;
				ppos[i].z += (rand()%2000 - 1000) / 100.0f;
			}
			else
			{
				DrawSphericalBlend(
					g_texture[ partdiffuse[i%PARTDIFFS] ].texname,
					g_texture[ spheredepth ].texname,
					renderdepthtex, renderfb,
					ppos[i].z,
					g_width/2 + ppos[i].x,
					g_height/2 + ppos[i].y,
					64, 0.0f,
					0, 0, 1, 1 );

				ppos[i] = ppos[i] + Normalize(start) * - g_drawfrinterval * 50.0f;

				if(Magnitude(ppos[i] - start) > 3000.0)
					ppos[i] = start;
			}
		}

		angle = angle + DEGTORAD(1.0f);
		if(angle > DEGTORAD(3360.0f))
			angle = 0.0f;
#endif

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		CHECKGLERROR();
#if 0
		Ortho(g_width, g_height, 1, 1, 1, 1);
		//DrawImage(rendertex, 0, 0, 512, 512, 0, 1, 1, 0, g_gui.m_crop);
		DrawImage(rendertex, 0, 0, 2048, 2048, 0, 0, 1, 1, g_gui.m_crop);
		EndS();
#endif

		glFlush();

		//DelFBO(&rendertex, &renderrb, &renderfb, &renderdepthtex);
#endif
	}
	CHECKGLERROR();
#endif

#ifdef DEBUGLOG
	Log("draw "<<__FILE__<<" "<<__LINE__);
	
#endif

	StartTimer(TIMER_DRAWGUI);
	gui->frameupd();

#ifdef DEBUGLOG
	Log("draw "<<__FILE__<<" "<<__LINE__);
	
#endif

	CHECKGLERROR();
	//if(!(g_mode == APPMODE_PLAY && g_speed == SPEED_FAST))
	gui->draw();
	StopTimer(TIMER_DRAWGUI);

#if 0
	for(int i=0; i<30; i++)
	{
		int x = rand()%g_width;
		int y = rand()%g_height;

		Blit(blittex, &blitscreen, Vec2i(x,y));
	}

	glDrawPixels(blitscreen.sizex, blitscreen.sizey, GL_RGB, GL_BYTE, blitscreen.data);
#endif

	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
	CHECKGLERROR();
	glDisable(GL_DEPTH_TEST);
	CHECKGLERROR();

#if 0
	RichText uni;

	for(int i=16000; i<19000; i++)
		//for(int i=0; i<3000; i++)
	{
		uni.m_part.push_back(RichPart(i));
	}

	float color[] = {1,1,1,1};
	DrawBoxShadText(MAINFONT8, 0, 0, g_width, g_height, &uni, color, 0, -1);
#endif

#ifdef DEBUGLOG
	Log("draw "<<__FILE__<<" "<<__LINE__);
	
#endif

#if 1
	if(g_debuglines)
	{
		char fpsstr[256];
		sprintf(fpsstr, "drw:%lf (%lf s/frame), upd:%lf (%lf s/frame), simfr:%s, netfr:%s", 
			g_instantdrawfps, 1.0/g_instantdrawfps, 
			g_instantupdfps, 1.0/g_instantupdfps, 
			ullform(g_simframe).c_str(),
			ullform(g_netframe).c_str());
		RichText fpsrstr(fpsstr);
		//fpsrstr = ParseTags(fpsrstr, NULL);
		DrawShadowedText(MAINFONT8, 0, g_height-MINIMAP_SIZE, &fpsrstr);
		CHECKGLERROR();
	}
#endif

#ifdef DEMO
	{
		unsigned int msleft = DEMOTIME - (GetTicks() - g_demostart);
		char msg[128];
		sprintf(msg, "Demo time %d:%02d", msleft / 1000 / 60, (msleft % (1000 * 60)) / 1000);
		RichText msgrt(msg);
		float color[] = {0.5f,1.0f,0.5f,1.0f};
		DrawShadowedText(MAINFONT16, g_width - 130, g_height-16, &msgrt, color);
	}
#endif

	EndS();
	CHECKGLERROR();

#ifdef DEBUGLOG
	Log("draw "<<__FILE__<<" "<<__LINE__);
	
#endif
	
#if 0
#ifdef PLATFORM_MOBILE
	glFlush();
	glFinish();
	
	Vec2i zoombox[2];
	
	zoombox[0].x = g_mouse.x - ZOOMBOX;
	zoombox[0].y = g_mouse.y - ZOOMBOX;
	zoombox[1].x = g_mouse.x + ZOOMBOX;
	zoombox[1].y = g_mouse.y + ZOOMBOX;
	
	zoombox[0].x = imax(0, zoombox[0].x);
	zoombox[0].y = imax(0, zoombox[0].y);
	zoombox[1].x = imin(g_width, zoombox[1].x);
	zoombox[1].y = imin(g_height, zoombox[1].y);
	
	LoadedTex zoompix;
	zoompix.sizex = zoombox[1].x - zoombox[0].x;
	zoompix.sizey = zoombox[1].y - zoombox[0].y;
	zoompix.channels = 3;
	zoompix.data = (unsigned char*)malloc( sizeof(unsigned char) * 3 * zoompix.sizex * zoompix.sizey );
	
	//glReadBuffer( GL_FRONT );
	glReadPixels(zoombox[0].x, zoombox[0].y, zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
	//CreateTex(&zoompix, &zoomtex, true, false);
	
	glBindTexture(GL_TEXTURE_2D, g_zoomtex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, zoombox[0].x - (g_mouse.x - ZOOMBOX), zoombox[0].y - (g_mouse.y - ZOOMBOX), zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
	
#endif
#endif

	//SDL_GL_SwapWindow(g_window);

	//CheckNum("post draw");
}
#else

void Draw()
{
#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif
    CheckGLError(__FILE__, __LINE__);

    if(g_mode == APPMODE_LOADING)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else if(g_mode == APPMODE_EDITOR)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   // else if(g_mode == APPMODE_RENDERING)
    //    glClearColor(g_transpkey[0], g_transpkey[1], g_transpkey[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CheckGLError(__FILE__, __LINE__);
#endif

#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

	if(g_mode == APPMODE_PLAY)
	{	
		//TurnOffShader();
		//g_camera.Look();
		
		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		//Matrix projection = OrthoProj(-PROJ_RIGHT*aspect, PROJ_RIGHT*aspect, PROJ_RIGHT, -PROJ_RIGHT, MIN_DISTANCE, MAX_DISTANCE);
		
		Vec3f viewvec = g_pcam->m_view;
        Vec3f posvec = g_pcam->m_pos;
        Vec3f posvec2 = g_pcam->lookpos();
        Vec3f upvec = g_pcam->m_up;

		//char msg[128];
		//sprintf(msg, "y = %f", posvec.y);
		//Chat(msg);
        
        Matrix viewmat = gluLookAt2(posvec2.x, posvec2.y, posvec2.z,
                                    viewvec.x, viewvec.y, viewvec.z,
                                    upvec.x, upvec.y, upvec.z);
		
		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.translation(translation);
		modelmat.translation(translation);
		modelview.postmult(viewmat);

		g_frustum.construct(projection.m_matrix, modelview.m_matrix);
		
		RenderToShadowMap(projection, viewmat, modelmat, viewvec, DrawSceneDepth);
		RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
		//AfterDraw(projection, viewmat, modelmat);
	}

    g_gui.frameupd();

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CheckGLError(__FILE__, __LINE__);
#endif

#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

    g_gui.draw();
    //DrawEdMap(&g_edmap);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CheckGLError(__FILE__, __LINE__);
#endif

    if(g_mode == APPMODE_EDITOR)
    {
#ifdef DEBUG
        LastNum(__FILE__, __LINE__);
#endif
        Ortho(g_width, g_height, 1, 1, 1, 1);
        char dbgstr[128];
        sprintf(dbgstr, "b's:%d", (int)g_edmap.m_brush.size());
        RichText rdbgstr(dbgstr);
        DrawShadowedText(MAINFONT8, 0, g_height-16, &rdbgstr);
        EndS();
    }

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#ifdef DEBUG
	g_applog<<"draw "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

}
#endif

bool OverMinimap()
{
	return false;
}

void Scroll(int dx, int dy)
{
	Vec2i old = g_scroll;
	bool moved = false;
	
	if(dx != 0 || dy != 0)
	{
		moved = true;
		g_scroll.x += dx;
		g_scroll.y += dy;
	}
	
	if(!moved)
		return;
	
	Vec2i onlyx = Vec2i(g_scroll.x,old.y);
	Vec2i onlyy = Vec2i(old.x,g_scroll.y);
	Vec3i temp;
	Vec3f ray;
	Vec3f point;
	
	IsoToCart(Vec2i(g_width,g_height)/2+g_scroll, &ray, &point);
	
	Vec3f line[2];
	Vec3f fint;
	line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
	line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	
	if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		//if(!MapInter(&g_hmap, ray, point, &temp))
	{
		//g_scroll = old;
		//return;
		
		IsoToCart(Vec2i(g_width,g_height)/2+onlyx, &ray, &point);
		line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
		line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
		
		if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
			//if(!MapInter(&g_hmap, ray, point, &temp))
		{
			IsoToCart(Vec2i(g_width,g_height)/2+onlyy, &ray, &point);
			line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
			line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
			
			if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
				//if(!MapInter(&g_hmap, ray, point, &temp))
			{
				g_scroll = old;
				moved = false;
			}
			else
				g_scroll = onlyy;
		}
		else
			g_scroll = onlyx;
	}
	
	if(!moved)
		return;
	
	UpdSBl();
	
	IsoToCart(g_mouse+g_scroll, &ray, &point);
	//MapInter(&g_hmap, ray, point, &g_mouse3d);
	line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
	line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	if(FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		g_mouse3d = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
}

void Scroll()
{
	//return;	//disable for now

	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	if(g_mouseout)
		return;
	
	Vec2i scroll(0,0);

	//const Uint8 *keys = SDL_GetKeyboardState(NULL);
	//SDL_BUTTON_LEFT;
	if((!g_keyintercepted && (g_keys[SDL_SCANCODE_UP] || g_keys[SDL_SCANCODE_W])) || (g_mouse.y <= SCROLL_BORDER))
	{
		scroll.y -= CAMERA_SPEED * g_drawfrinterval;
	}

	if((!g_keyintercepted && (g_keys[SDL_SCANCODE_DOWN] || g_keys[SDL_SCANCODE_S])) || (g_mouse.y >= g_height-SCROLL_BORDER))
	{
		scroll.y += CAMERA_SPEED * g_drawfrinterval;
	}

	if((!g_keyintercepted && (g_keys[SDL_SCANCODE_LEFT] || g_keys[SDL_SCANCODE_A])) || (g_mouse.x <= SCROLL_BORDER))
	{
		scroll.x -= CAMERA_SPEED * g_drawfrinterval;
	}

	if((!g_keyintercepted && (g_keys[SDL_SCANCODE_RIGHT] || g_keys[SDL_SCANCODE_D])) || (g_mouse.x >= g_width-SCROLL_BORDER))
	{
		scroll.x += CAMERA_SPEED * g_drawfrinterval;
	}
	
	if(scroll.x == 0 && scroll.y == 0)
		return;
	
	Scroll(scroll.x, scroll.y);
}

void LoadConfig()
{
	EnumDisplay();
	
	g_selres = g_resolution[0];
	
	for(auto rit=g_resolution.begin(); rit!=g_resolution.end(); rit++)
	{
#if 0
		//below acceptable height?
		if(g_selres.height < 480)
		{
			if(rit->height > g_selres.height &&
			   rit->width > rit->height)
			{
				g_selres = *rit;
			}
		}
		//already of acceptable height?
		else
#endif
		{
			//get smallest acceptable resolution
			if(rit->height < g_selres.height &&
			   rit->width > rit->height)
			{
				g_selres = *rit;
			}
			
			break;
		}
	}
	
	//g_selres.width = 568;
	//g_selres.height = 320;
	
	//g_selres.height = 568;
	//g_selres.width = 320;
	
	g_lang = "eng";
	SwitchLang(g_lang.c_str());
	
	char cfgfull[DMD_MAX_PATH+1];
	FullWritePath(CONFIGFILE, cfgfull);

	std::ifstream f(cfgfull);

	if(!f)
		return;

	std::string line;
	char keystr[128];
	char actstr[128];

	Player* py = &g_player[g_localP];

	while(!f.eof())
	{
		strcpy(keystr, "");
		strcpy(actstr, "");

		getline(f, line);

		if(line.length() > 127)
			continue;

		actstr[0] = 0;

		sscanf(line.c_str(), "%s %s", keystr, actstr);

		float valuef = StrToFloat(actstr);
		int valuei = StrToInt(actstr);
		bool valueb = valuei ? true : false;

		if(stricmp(keystr, "fullscreen") == 0)					g_fullscreen = valueb;
		else if(stricmp(keystr, "client_width") == 0)			g_width = g_selres.width = valuei;
		else if(stricmp(keystr, "client_height") == 0)			g_height = g_selres.height = valuei;
		else if(stricmp(keystr, "screen_bpp") == 0)				g_bpp = valuei;
		else if(stricmp(keystr, "volume") == 0)					SetVol(valuei);
		else if(stricmp(keystr, "language") == 0)				SwitchLang(actstr);
		else if(stricmp(keystr, "camera_x_fov") == 0)			g_xfov = valuef;
		else if(stricmp(keystr, "camera_y_fov") == 0)			g_yfov = valuef;
	}

	f.close();

	//g_lang = LANG_ENG;
}

void LoadName()
{
	char cfgfull[DMD_MAX_PATH+1];
	FullWritePath("name.txt", cfgfull);

	std::ifstream f(cfgfull);

	if(!f)
	{
		char name[GENNAMELEN];
		GenName(name);
		g_name = RichText(name);
		return;
	}

	std::string line;

	getline(f, line);

	unsigned int* ustr = ToUTF32((unsigned char*)line.c_str());
	g_name = RichText(UStr(ustr));
	delete [] ustr;

	f.close();
}

void WriteConfig()
{
	char cfgfull[DMD_MAX_PATH+1];
	FullWritePath(CONFIGFILE, cfgfull);
	FILE* fp = fopen(cfgfull, "w");
	if(!fp)
		return;
	fprintf(fp, "fullscreen %d \r\n\r\n", g_fullscreen ? 1 : 0);
	fprintf(fp, "client_width %d \r\n\r\n", g_selres.width);
	fprintf(fp, "client_height %d \r\n\r\n", g_selres.height);
	fprintf(fp, "screen_bpp %d \r\n\r\n", g_bpp);
	fprintf(fp, "volume %d \r\n\r\n", g_volume);
	fprintf(fp, "language %s\r\n\r\n", g_lang.c_str());
	fclose(fp);
}

void WriteName()
{
	char cfgfull[DMD_MAX_PATH+1];
	FullWritePath("name.txt", cfgfull);
	FILE* fp = fopen(cfgfull, "w");
	if(!fp)
		return;
	std::string name = g_name.rawstr();
	fprintf(fp, "%s", name.c_str());
	fclose(fp);
}

int testfunc(ObjectScript::OS* os, int nparams, int closure_values, int need_ret_values, void * param)
{
	InfoMess("os", "test");
	return 1;
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void SignalCallback(int signum)
{
	//printf("Caught signal %d\n",signum);
	// Cleanup and hide up stuff here

	// Terminate program
	//g_quit = true;
	exit(0);	//force quit NOW
}

#ifdef PLATFORM_MOBILE
void CopyFile(const char* from, const char* to)
{
	FILE* fromfp = fopen(from, "rb");
	FILE* tofp = fopen(to, "wb");
	
	unsigned char byte = 0;

	int i = 0;
	int len = 0;

	fseek(fromfp, 0L, SEEK_END);
	len = ftell(fromfp);
	fseek(fromfp, 0L, SEEK_SET);
	
	//check remark about feof in CheckSum(...)
	//while(!feof(fromfp))
	for(i=0; i<len; i++)
	{
		fread(&byte, 1, 1, fromfp);
		fwrite(&byte, 1, 1, tofp);
	}
	
	fclose(fromfp);
	fclose(tofp);
}

void FirstRun()
{
	char checkfull[DMD_MAX_PATH+1];
	char checkfile[128];
	sprintf(checkfile, "first%llu.txt", VERSION);
	FullWritePath(checkfile, checkfull);
	
	FILE* fp = fopen(checkfull, "rb");
	
	if(fp)
	{
		fclose(fp);
		return;
	}
	
	//else, this is the first run for this version
	fp = fopen(checkfull, "wb");
	fwrite("1", sizeof(char), 2, fp);
	fclose(fp);
	
	std::list<std::string> maps;
	std::list<std::string> saves;
	
	char mapswritefull[DMD_MAX_PATH+1];
	char saveswritefull[DMD_MAX_PATH+1];
	char screenwritefull[DMD_MAX_PATH+1];
	FullWritePath("maps", mapswritefull);
	FullWritePath("saves", saveswritefull);
	FullWritePath("screenshot", screenwritefull);
	mkdir(mapswritefull, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir(saveswritefull, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir(screenwritefull, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	
	char mapsreadfull[DMD_MAX_PATH+1];
	char savesreadfull[DMD_MAX_PATH+1];
	FullPath("maps", mapsreadfull);
	FullPath("saves", savesreadfull);
	
	ListFiles(mapsreadfull, maps);
	ListFiles(savesreadfull, saves);
	
	for(auto fit=maps.begin(); fit!=maps.end(); fit++)
	{
		char fullfrom[DMD_MAX_PATH+1];
		char fullto[DMD_MAX_PATH+1];
		sprintf(fullfrom, "%s/%s", mapsreadfull, fit->c_str());
		sprintf(fullto, "%s/%s", mapswritefull, fit->c_str());
		CopyFile(fullfrom, fullto);
	}
	
	for(auto fit=saves.begin(); fit!=saves.end(); fit++)
	{
		char fullfrom[DMD_MAX_PATH+1];
		char fullto[DMD_MAX_PATH+1];
		sprintf(fullfrom, "%s/%s", savesreadfull, fit->c_str());
		sprintf(fullto, "%s/%s", saveswritefull, fit->c_str());
		CopyFile(fullfrom, fullto);
	}
}
#endif

void Init()
{
#ifdef PLATFORM_LINUX
	signal(SIGINT, SignalCallback);
#endif
	
#ifdef PLATFORM_IOS
	//SDL_SetEventFilter(HandleEvent, NULL);
#endif
	
#ifdef PLATFORM_MOBILE
	FirstRun();
#endif

#if 0
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		[director enableRetinaDisplay:NO];
		
		
	} else {
		
		[director enableRetinaDisplay:YES];
		
	}
#endif
	//SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	
#if 0
	if(!SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1"))
		ErrMess("failed to set dpi hint", "failed to set dpi hint");
	else
		ErrMess("set to set dpi hint", "set to set dpi hint");
#endif
	
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
	}

	if(SDLNet_Init() == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
	}

	SDL_version compile_version;
	const SDL_version *link_version=Mix_Linked_Version();
	SDL_MIXER_VERSION(&compile_version);
	printf("compiled with SDL_mixer version: %d.%d.%d\n",
		compile_version.major,
		compile_version.minor,
		compile_version.patch);
	printf("running with SDL_mixer version: %d.%d.%d\n",
		link_version->major,
		link_version->minor,
		link_version->patch);

	// load support for the OGG and MOD sample/music formats
	//int flags=MIX_INIT_OGG|MIX_INIT_MOD|MIX_INIT_MP3;
	int flags=MIX_INIT_OGG|MIX_INIT_MP3;
	int initted=Mix_Init(flags);
	if( (initted & flags) != flags)
	{
		char msg[1280];
		sprintf(msg, "Mix_Init: Failed to init required ogg and mod support!\nMix_Init: %s", Mix_GetError());
		//ErrMess("Error", msg);
		// handle error
	}

	// start SDL with audio support
	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
		char msg[1280];
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		ErrMess("Error", msg);
		// handle error
		//exit(1);
	}
	// show 44.1KHz, signed 16bit, system byte order,
	//      stereo audio, using 1024 byte chunks
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
		char msg[1280];
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		ErrMess("Error", msg);
		// handle error
		//exit(2);
	}

	Mix_AllocateChannels(SOUND_CHANNELS);

	if(!g_applog.is_open())
		OpenLog("log.txt", VERSION);

	srand((unsigned int)GetTicks());

	for(int i=0; i<BL_TYPES; i++)
	{
		BlType* bt = &g_bltype[i];
		PYGRAPHSTR[PYGRAPH_BLFIN_BEG + i] = bt->name;
	}

	//TODO c-style inits, not constructors
	FreePys();	//must be called before loading name? because cl's deinited here
	LoadConfig();
	LoadName();

	//g_os = ObjectScript::OS::create();
	//g_os->pushCFunction(testfunc);
	//g_os->setGlobal("testfunc");
	//os->eval("testfunc();");
	//os->eval("function require(){ /* if(relative == \"called.os\") */ { testfunc(); } }");
	char autoexecpath[DMD_MAX_PATH+1];
	FullPath("scripts/autoexec.os", autoexecpath);
	//g_os->require(autoexecpath);
	//g_os->release();

	//EnumerateMaps();
	//EnumerateDisplay();
	MapKeys();

	InitProfiles();
}

void Deinit()
{
	EndSess();
	FreeMap();

	g_gui.freech();

	WriteProfiles(-1, 0);
	BreakWin(TITLE.rawstr().c_str());

	for(auto cit = g_conn.begin(); cit != g_conn.end(); cit++)
	{
		Disconnect(&*cit);
	}

	const unsigned long long start = GetTicks();
	//After quit, wait to send out quit packet to make sure host/clients recieve it.
	while (GetTicks() - start < QUIT_DELAY)
	{
		if(NetQuit())
			break;
		if(g_sock)
			UpdNet();
	}

	// Clean up

	if(g_sock)
	{
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	FreeSounds();

	Mix_CloseAudio();

	// force a quit
	//while(Mix_Init(0))
	//	Mix_Quit();
	Mix_Quit();

	SDLNet_Quit();
	SDL_Quit();
}

void CaptureZoom()
{
#if 1
#ifdef PLATFORM_MOBILE
	//if(g_zoomdrawframe)
	{
		//glFlush();
		//glFinish();
		
		Vec2i zoombox[2];
		
		zoombox[0].x = g_mouse.x - ZOOMBOX;
		zoombox[0].y = g_mouse.y - ZOOMBOX;
		zoombox[1].x = g_mouse.x + ZOOMBOX;
		zoombox[1].y = g_mouse.y + ZOOMBOX;
		
		zoombox[0].x = imax(0, zoombox[0].x);
		zoombox[0].y = imax(0, zoombox[0].y);
		zoombox[1].x = imin(g_width, zoombox[1].x);
		zoombox[1].y = imin(g_height, zoombox[1].y);
		
		LoadedTex zoompix;
		zoompix.sizex = zoombox[1].x - zoombox[0].x;
		zoompix.sizey = zoombox[1].y - zoombox[0].y;
		zoompix.channels = 4;
		zoompix.data = (unsigned char*)malloc( sizeof(unsigned char) * 4 * zoompix.sizex * zoompix.sizey );
		
		//glReadBuffer( GL_FRONT );
		glReadPixels(zoombox[0].x, g_height - zoombox[1].y, zoompix.sizex, zoompix.sizey, GL_RGBA, GL_UNSIGNED_BYTE, zoompix.data);
		//CreateTex(&zoompix, &zoomtex, true, false);
		
		glBindTexture(GL_TEXTURE_2D, g_zoomtex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, zoombox[0].x - (g_mouse.x - ZOOMBOX), zoombox[0].y - (g_mouse.y - ZOOMBOX), zoompix.sizex, zoompix.sizey, GL_RGBA, GL_UNSIGNED_BYTE, zoompix.data);
	}
#endif
#endif
}

void DrawZoom()
{
	DrawImage(g_zoomtex, g_mouse.x - ZOOMBOX*2, g_mouse.y - ZOOMBOX*2, g_mouse.x + ZOOMBOX*2, g_mouse.y + ZOOMBOX*2, 0,1,1,0, g_gui.m_crop);
}

int HandleEvent(void *userdata, SDL_Event *e)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	InEv ie;
	ie.intercepted = false;
	ie.curst = CU_DEFAULT;
	Vec2i old;
	
	switch(e->type)
	{
#ifdef PLATFORM_IOS
		case SDL_APP_TERMINATING:
			/* Terminate the app.
			 Shut everything down before returning from this function.
			 */
			g_quit = true;
			g_background = true;
			exit(0);
			return 0;
		case SDL_APP_LOWMEMORY:
			/* You will get this when your app is paused and iOS wants more memory.
			 Release as much memory as possible.
			 */
			exit(0);
			return 0;
		case SDL_APP_WILLENTERBACKGROUND:
			/* Prepare your app to go into the background.  Stop loops, etc.
			 This gets called when the user hits the home button, or gets a call.
			 */
			g_quit = true;
			g_background = true;
			exit(0);
			return 0;
		case SDL_APP_DIDENTERBACKGROUND:
			/* This will get called if the user accepted whatever sent your app to the background.
			 If the user got a phone call and canceled it, you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
			 When you get this, you have 5 seconds to save all your state or the app will be terminated.
			 Your app is NOT active at this point.
			 */
			g_quit = true;
			g_background = true;
			exit(0);
			return 0;
		case SDL_APP_WILLENTERFOREGROUND:
			/* This call happens when your app is coming back to the foreground.
			 Restore all your state here.
			 */
			g_background = false;
			return 0;
		case SDL_APP_DIDENTERFOREGROUND:
			/* Restart your loops here.
			 Your app is interactive and getting CPU again.
			 */
			g_background = false;
			return 0;
#endif
		case SDL_QUIT:
			g_quit = true;
			break;
		case SDL_KEYDOWN:
			ie.type = INEV_KEYDOWN;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;
			
			//Handle copy
			if( e->key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
			{
				//SDL_SetClipboardText( inputText.c_str() );
				ie.type = INEV_COPY;
			}
			//Handle paste
			if( e->key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
			{
				//inputText = SDL_GetClipboardText();
				//renderText = true;
				ie.type = INEV_PASTE;
			}
			//Select all
			if( e->key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL )
			{
				//inputText = SDL_GetClipboardText();
				//renderText = true;
				ie.type = INEV_SELALL;
			}
			
			gui->inev(&ie);
			
			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = true;
			
			g_keyintercepted = ie.intercepted;
			break;
		case SDL_KEYUP:
			ie.type = INEV_KEYUP;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;
			
			gui->inev(&ie);
			
			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = false;
			
			g_keyintercepted = ie.intercepted;
			break;
		case SDL_TEXTINPUT:
			//g_GUI.charin(e->text.text);	//UTF8
			ie.type = INEV_TEXTIN;
			ie.text = e->text.text;
			
			gui->inev(&ie);
			break;
#if 0
		case SDL_TEXTEDITING:
			//g_GUI.charin(e->text.text);	//UTF8
			ie.type = INEV_TEXTED;
			//ie.text = e->text.text;
			ie.text = e->edit.text;
			//Log("texed: "<<e->text.text<<" [0]="<<(unsigned int)e->text.text[0]);
			//
			ie.cursor = e->edit.start;
			ie.sellen = e->edit.length;
			
			
			gui->inev(&ie);
#if 0
			ie.intercepted = false;
			ie.type = INEV_TEXTIN;
			ie.text = e->text.text;
			
			gui->inev(&ie);
#endif
			break;
#endif
#if 0
		case SDL_TEXTINPUT:
			/* Add new text onto the end of our text */
			strcat(text, event.text.text);
#if 0
			ie.type = INEV_CHARIN;
			ie.key = wParam;
			ie.scancode = 0;
			
			gui->inev(&ie);
#endif
			break;
		case SDL_TEXTEDITING:
			/*
			 Update the composition text.
			 Update the cursor position.
			 Update the selection length (if any).
			 */
			composition = event.edit.text;
			cursor = event.edit.start;
			selection_len = event.edit.length;
			break;
#endif
			//else if(e->type == SDL_BUTTONDOWN)
			//{
			//}
		case SDL_MOUSEWHEEL:
			ie.type = INEV_MOUSEWHEEL;
			ie.amount = e->wheel.y;
			
			gui->inev(&ie);
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (e->button.button)
		{
			case SDL_BUTTON_LEFT:
				g_mousekeys[MOUSE_LEFT] = true;
				g_moved = false;
				
				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_LEFT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				
#if 0	//attempt at zoombox coordinate translation
#ifdef PLATFORM_MOBILE
				if(e->button.x > g_mouse.x - ZOOMBOX &&
				   e->button.x < g_mouse.x + ZOOMBOX &&
				   e->button.y > g_mouse.y - ZOOMBOX &&
				   e->button.y < g_mouse.y + ZOOMBOX)
				{
					//ie.x = (e->button.x - g_mouse.x) * 2 + g_mouse.x;
					//ie.y = (e->button.y - g_mouse.y) * 2 + g_mouse.y;
					ie.x = (e->button.x/2 - g_mouse.x) * 2 + g_mouse.x;
					ie.y = (e->button.y/2 - g_mouse.y) * 2 + g_mouse.y;
					
					g_mouse.x = ie.x;
					g_mouse.y = ie.y;
				}
#endif
#endif
				
				gui->inev(&ie);
				
				g_keyintercepted = ie.intercepted;
				break;
			case SDL_BUTTON_RIGHT:
				g_mousekeys[MOUSE_RIGHT] = true;
				
				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_RIGHT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				
				gui->inev(&ie);
				break;
			case SDL_BUTTON_MIDDLE:
				g_mousekeys[MOUSE_MIDDLE] = true;
				
				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_MIDDLE;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				
				gui->inev(&ie);
				break;
		}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (e->button.button)
		{
				case SDL_BUTTON_LEFT:
					g_mousekeys[MOUSE_LEFT] = false;
					
					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_LEFT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					gui->inev(&ie);
					break;
				case SDL_BUTTON_RIGHT:
					g_mousekeys[MOUSE_RIGHT] = false;
					
					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_RIGHT;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					gui->inev(&ie);
					break;
				case SDL_BUTTON_MIDDLE:
					g_mousekeys[MOUSE_MIDDLE] = false;
					
					ie.type = INEV_MOUSEUP;
					ie.key = MOUSE_MIDDLE;
					ie.amount = 1;
					ie.x = g_mouse.x;
					ie.y = g_mouse.y;
					
					gui->inev(&ie);
					break;
			}
			break;
		case SDL_MOUSEMOTION:
			//g_mouse.x = e->motion.x;
			//g_mouse.y = e->motion.y;
			
			if(g_mouseout)
			{
				//TrackMouse();
				g_mouseout = false;
			}
			
			old = g_mouse;
			
			if(MousePosition())
			{
				g_moved = true;
				
				ie.type = INEV_MOUSEMOVE;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				ie.dx = g_mouse.x - old.x;
				ie.dy = g_mouse.y - old.y;
				
				gui->inev(&ie);
				
				g_curst = ie.curst;
			}
			break;
		}

	return 0;
}

void EventLoop()
{
	
#ifdef PLATFORM_MOBILE
	int width = g_width;
	int height = g_height;
	g_width >>= 1;
	g_height >>= 1;
#endif
	
#if 0
	key->keysym.scancode
		SDLMod  e->key.keysym.mod
		key->keysym.unicode

		if( mod & KMOD_NUM ) printf( "NUMLOCK " );
	if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
	if( mod & KMOD_LCTRL ) printf( "LCTRL " );
	if( mod & KMOD_RCTRL ) printf( "RCTRL " );
	if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
	if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
	if( mod & KMOD_RALT ) printf( "RALT " );
	if( mod & KMOD_LALT ) printf( "LALT " );
	if( mod & KMOD_CTRL ) printf( "CTRL " );
	if( mod & KMOD_SHIFT ) printf( "SHIFT " );
	if( mod & KMOD_ALT ) printf( "ALT " );
#endif

	//SDL_EnableUNICODE(SDL_ENABLE);

	while (!g_quit)
	{
		StartTimer(TIMER_FRAME);
		StartTimer(TIMER_EVENT);

//#ifndef PLATFORM_IOS
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			HandleEvent(NULL, &e);
		}
//#endif

		StopTimer(TIMER_EVENT);
		
		if(g_quit)
			break;
		
#if 1
		//if ((g_mode == APPMODE_LOADING || g_mode == APPMODE_RELOADING) || true /* DrawNextFrame(DRAW_FRAME_RATE) */ )
		if ( !g_background &&
			( (g_mode == APPMODE_LOADING || g_mode == APPMODE_RELOADING) || DrawNextFrame() ) )
#endif
		{
			StartTimer(TIMER_DRAW);

#ifdef DEBUGLOG
			Log("main "<<__FILE__<<" "<<__LINE__);
			
#endif
			CalcDrawRate();

			CHECKGLERROR();

#ifdef DEBUGLOG
			Log("main "<<__FILE__<<" "<<__LINE__);
			
#endif
			
#ifndef PLATFORM_MOBILE
			Draw();
			SDL_GL_SwapWindow(g_window);
#endif
			
			
#ifdef PLATFORM_MOBILE
			/*
			GLint deffbo;
			//GL_FRAMEBUFFER_BINDING_OES
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &deffbo);
			
			unsigned int offtex;
			unsigned int offrb;
			unsigned int offfb;
			unsigned int offdepth;
			
			MakeFBO(&offtex, &offrb, &offfb, &offdepth, g_width, g_height);
			glBindFramebuffer(GL_FRAMEBUFFER, offfb);
			 */

			//g_zoomdrawframe = false;
			Draw();
			SDL_GL_SwapWindow(g_window);
#if 0
			if(!g_zoomdrawframe)
			{
				SDL_GL_SwapWindow(g_window);
				CaptureZoom();
			}
			
			//glBindFramebuffer(GL_FRAMEBUFFER, deffbo);
			
			//int highw = Max2Pow(g_width);
			//int highh = Max2Pow(g_height);
			
			//DrawImage(offtex, 0, 0, g_width-1, g_height-1, 0, 1, 1, 0, g_gui.m_crop);
			//DrawImage(offtex, 0, 0, g_width-1, g_height-1, 0, (float)g_width/(float)highw, (float)g_height/(float)highh, 0, g_gui.m_crop);
			else
			{
				DrawZoom();
				SDL_GL_SwapWindow(g_window);
			}
#elif 0
			if(g_zoomdrawframe)
			{
				DrawZoom();
				SDL_GL_SwapWindow(g_window);
				CaptureZoom();
			}
#endif
			
			g_zoomdrawframe = !g_zoomdrawframe;

			//DelFBO(&offtex, &offrb, &offfb, &offdepth);
			//glBindFramebuffer(GL_FRAMEBUFFER, deffbo);
			
			
			
#endif	//PLATFORM_MOBILE
			
			CHECKGLERROR();

			if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
			{
#ifdef DEBUGLOG
				Log("main "<<__FILE__<<" "<<__LINE__);
				
#endif
				//Scroll();
#ifdef DEBUGLOG
				Log("main "<<__FILE__<<" "<<__LINE__);
				
#endif
				//UpdResTicker();
			}

			StopTimer(TIMER_DRAW);
		}

		//if((g_mode == APPMODE_LOADING || g_mode == APPMODE_RELOADING) || true /* UpdNextFrame(SIM_FRAME_RATE) */ )
		if((g_mode == APPMODE_LOADING || g_mode == APPMODE_RELOADING) || UpdNextFrame() )
		{
			//Try to avoid querying the timer, to maximize throughput
			static unsigned int lastupds = 1;
			static unsigned long long lasttick = GetTicks();

			//Trick to allow running millions of times per frame by avoiding running the check for input events query
			while(true)
			{
				bool stopall = false;

				for(int upd=0; upd<lastupds; upd++)
				{

					StartTimer(TIMER_UPDATE);

#ifdef DEBUGLOG
					Log("main "<<__FILE__<<" "<<__LINE__);
					
#endif

					//TODO make even faster by reducing calls to GetTicks() (gettimeofday()) but only calling it every 100th or 1000th time and getting fps that way
					CalcUpdRate();
					Update();

					StopTimer(TIMER_UPDATE);
				}

				unsigned long long now = GetTicks();
				unsigned long long tickspassed = now - lasttick;

				if(tickspassed < 1000 / SIM_FRAME_RATE)
				{
					lastupds = lastupds * 1.5f;
				}
				else
				{
					lastupds = imax(1, lastupds * 0.9f);
				}

				lasttick = now;

				if(!UpdNextFrame())
					break;

				if(g_speed == SPEED_FAST)
					break;
			}
		}
		
		StopTimer(TIMER_FRAME);
	}
	
#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif
}

#ifdef USESTEAM
//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
#ifdef PLATFORM_WIN
	::OutputDebugString( pchDebugText );
#endif

	if(!g_applog.is_open())
		OpenLog("log.txt", VERSION);

	Log(pchDebugText);
	

	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		x = x;
	}
}
#endif

void Main()
{
	//*((int*)0) = 0;

#ifdef USESTEAM

	if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steam_appid.txt from the game depot.

		return;
	}

	// Init Steam CEG
	if ( !Steamworks_InitCEGLibrary() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "Steamworks_InitCEGLibrary() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (InitDrmLibrary() failed).\n" );
		return;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	if ( !SteamAPI_Init() )
	{
#ifdef PLATFORM_WIN
		OutputDebugString( "SteamAPI_Init() failed\n" );
#endif
		ErrMess( "Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n" );
		return;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

#endif

	Init();
	g_applog << "MakeWin: " << std::endl;
	
	MakeWin(TITLE.rawstr().c_str());
	
#ifdef PLATFORM_MOBILE
	int width = g_width;
	int height = g_height;
	//halve for mobile small screen so it appears twice as big
	g_width >>= 1;
	g_height >>= 1;
#endif
	
	g_applog << "FillGUI: "    << std::endl;
	
	g_applog << "Queue: "      << std::endl;
	
	SDL_ShowCursor(false);
	LoadSysRes();
	Queue();
	FillGUI();
	g_applog << "EventLoop: "  << std::endl;
	
#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif
	
	EventLoop();
	g_applog << "Deinit: "     << std::endl;
	
	Deinit();
	SDL_ShowCursor(true);
}

bool RunOptions(const char* cmdline)
{
	std::string startmap;
	std::string invideo;

	if(stricmp(cmdline, "") == 0)
	{
		startmap = "";

		return false;
	}
	else
	{
		std::string cmdlinestr(cmdline);
		std::string find_devmap("+devmap ");
		std::string find_invideo("+invideo ");
		int found_devmap = cmdlinestr.rfind(find_devmap);
		int found_invideo = cmdlinestr.rfind(find_invideo);

		if(found_devmap != std::string::npos)
		{
			startmap = cmdlinestr.substr(found_devmap+find_devmap.length(), cmdlinestr.length()-found_devmap-find_devmap.length());

			g_applog<<cmdline<<std::endl;
			g_applog<<startmap<<std::endl;

			//LoadMap(startmap.c_str());
			g_startmap = startmap;
		}

		if(found_invideo != std::string::npos)
		{
			invideo = cmdlinestr.substr(found_invideo+find_invideo.length(), cmdlinestr.length()-found_invideo-find_invideo.length());

			g_applog<<cmdline<<std::endl;
			g_applog<<invideo<<std::endl;

			//LoadMap(startmap.c_str());
			g_invideo = invideo;
		}

		return true;
	}

	return false;
}

#ifdef PLATFORM_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
#ifdef PLATFORM_WIN
	g_hInstance = hInstance;
#endif

#ifdef PLATFORM_WIN
	RunOptions(lpCmdLine);
#else
	char cmdline[DMD_MAX_PATH+124];
	cmdline[0] = 0;
	for(int ai=0; ai<argc; ai++)
	{
		strcat(cmdline, argv[ai]);

		if(ai+1<argc)
			strcat(cmdline, " ");
	}
	RunOptions(cmdline);
#endif

#ifdef PLATFORM_IOS
	NSLog(@"running");
#endif
	
	//g_applog << "Log start"    << std::endl; /* TODO, include date */
	//g_applog << "Init: "       << std::endl;
	//

#ifdef PLATFORM_WIN
	if ( IsDebuggerPresent() )
	{
		// We don't want to mask exceptions (or report them to Steam!) when debugging.
		// If you would like to step through the exception handler, attach a debugger
		// after running the game outside of the debugger.	

		Main();
		return 0;
	}
#endif

#ifdef PLATFORM_WIN
#ifdef USESTEAM
	_set_se_translator( MiniDumpFunction );

	try  // this try block allows the SE translator to work
	{
#endif
#endif
		Main();
#ifdef PLATFORM_WIN
#ifdef USESTEAM
	}
	catch( ... )
	{
		return -1;
	}
#endif
#endif
	
#ifdef PLATFORM_IOS
	exit(0);
#endif

	return 0;
}
