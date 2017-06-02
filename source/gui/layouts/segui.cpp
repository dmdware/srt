


#include "../../app/appmain.h"
#include "../../math/3dmath.h"
#include "../../gui/gui.h"
#include "../../texture.h"
#include "segui.h"
#include "../../window.h"
#include "seviewport.h"
#include "sesim.h"
#include "../../save/save.h"
#include "../../save/saveedm.h"
#include "../../sim/simdef.h"
#include "../../bsp/sortb.h"
#include "undo.h"
#include "../../save/compilemap.h"
#include "../../render/shadow.h"
#include "../../save/modelholder.h"
#include "../../save/savesprite.h"
#include "../../render/screenshot.h"
#include "../../sim/entity.h"
#include "../../bsp/tracework.h"
#include "../../sim/selection.h"
#include "../../sim/order.h"
#include "../../utils.h"
#include "ggui.h"
#include "../../save/savemap.h"
#include "../widgets/spez/loadview.h"
#include "../widgets/spez/saveview.h"
#include "../widgets/spez/svlist.h"
#include "../widgets/spez/newhost.h"
#include "../../input/input.h"
#include "../../sim/entity.h"
#include "messbox.h"

int g_projtype = PROJ_PERSP;
bool g_showsky = false;
float g_snapgrid = 25;
char g_lastsave[DMD_MAX_PATH+1];
bool g_supresserr = false;
float g_maxelev = 10000;

void AnyKeyDown(int k)
{
	SkipLogo();
}

void AnyKeyUp(int k)
{
}

void Escape()
{
	if(g_mode != APPMODE_EDITOR)
		return;
}

void Delete()
{
	if(g_mode != APPMODE_EDITOR)
		return;

	if(g_selB.size() > 0 || g_selM.size() > 0)
		LinkPrevUndo();

	for(int i=0; i<g_selB.size(); i++)
	{
		for(auto j=g_edmap.m_brush.begin(); j!=g_edmap.m_brush.end(); j++)
		{
			if(g_selB[i] == &*j)
			{
				g_edmap.m_brush.erase(j);
				break;
			}
		}
	}

	g_selB.clear();
	g_sel1b = NULL;

	CloseSideView();

	for(int i=0; i<g_selM.size(); i++)
	{
		for(auto j=g_modelholder.begin(); j!=g_modelholder.end(); j++)
		{
			if(g_selM[i] == &*j)
			{
				g_modelholder.erase(j);
				break;
			}
		}
	}

	g_selM.clear();
	g_sel1m = NULL;

	//LinkLatestUndo();
	//g_applog<<"delete b"<<std::endl;
	//g_applog.flush();
}

#if 0
void MouseMove(InEv* ie)
{
	if(g_mode != APPMODE_PLAY)
		return;

	if(g_mousekeys[MOUSE_MIDDLE])
	{
		Vec2i d = g_mouse - g_mousestart;

		ViewportRotate(3, d.x, d.y);

		g_mousestart = g_mouse;
	}
}
#endif

void MouseMidButtonDown()
{
	if(g_mode != APPMODE_PLAY)
		return;

	g_mousestart = g_mouse;
}

void MouseRightButtonDown()
{
}

void MouseRightButtonUp()
{
	if(g_mode != APPMODE_PLAY)
		return;

	EdPane* v = &g_edpane[3];
	
	//Order(g_mouse.x, g_mouse.y, g_width, g_height, v->pos(), v->focus(), v->viewdir(), v->strafe(), v->up2());
}

void MouseLeftButtonDown()
{
	if(g_mode == APPMODE_LOGO)
		SkipLogo();
	else if(g_mode == APPMODE_EDITOR)
	{

	}
	else if(g_mode == APPMODE_PLAY)
	{
		g_mousestart = g_mouse;
	}
}

void MouseLeftButtonUp()
{
	if(g_mode != APPMODE_PLAY)
		return;

	EdPane* v = &g_edpane[3];
	
	DoSelection(v->pos(), v->strafe(), v->up2(), v->viewdir());
}

void Change_RotDeg(unsigned int key, unsigned int scancode, bool down, int parm)
{
}

void Change_Zoom(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Widget* zoombox = g_gui.get("editor")->get("top panel")->get("zoom");

	//if(zoombox->m_value.rawstr().c_str()[0] == '\0')
	//	return;

	g_zoom = StrToFloat(zoombox->m_value.rawstr().c_str());

	if(g_zoom <= 0.0f)
		g_zoom = 1.0f;
}

//void Change_SnapGrid(int dummy)
void Change_SnapGrid()
{
	Widget* snapgbox = g_gui.get("editor")->get("top panel")->get("snapgrid");

	//if(snapgbox->m_value.rawstr().c_str()[0] == '\0')
	//	return;

	//g_snapgrid = StrToFloat(snapgbox->m_value.rawstr().c_str());
	//int power = snapgbox->m_selected;
	//g_snapgrid = 400 / pow(2, power);

	float cm_scales[] = CM_SCALES;

	g_snapgrid = cm_scales[ snapgbox->m_selected ];

	if(g_snapgrid <= 0.0f)
		g_snapgrid = 1;
}

void Change_MaxElev(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Widget* maxelevbox = g_gui.get("editor")->get("top panel")->get("maxelev");

	g_maxelev = StrToFloat(maxelevbox->m_value.rawstr().c_str());
}

void Click_NewBrush()
{
	LinkPrevUndo();
	//Vec3f pos = g_focus;
	Vec3f pos = g_cam.m_view;
	pos.x = Snap(g_snapgrid, pos.x);
	pos.y = Snap(g_snapgrid, pos.y);
	pos.z = Snap(g_snapgrid, pos.z);
	Brush b;
#if 1
	BrushSide top(Vec3f(0,1,0), pos + Vec3f(0,STOREY_HEIGHT,0));
	BrushSide bottom(Vec3f(0,-1,0), pos + Vec3f(0,0,0));
	BrushSide left(Vec3f(-1,0,0), pos + Vec3f(-TILE_SIZE/2.0f,0,0));
	BrushSide right(Vec3f(1,0,0), pos + Vec3f(TILE_SIZE/2.0f,0,0));
	BrushSide front(Vec3f(0,0,1), pos + Vec3f(0,0,TILE_SIZE/2.0f));
	BrushSide back(Vec3f(0,0,-1), pos + Vec3f(0,0,-TILE_SIZE/2.0f));
#else
	BrushSide top(Vec3f(0,1,0), pos + Vec3f(0,1000,0));
	BrushSide bottom(Vec3f(0,-1,0), pos + Vec3f(0,0,0));
	BrushSide left(Vec3f(-1,0,0), pos + Vec3f(-1000/2.0f,0,0));
	BrushSide right(Vec3f(1,0,0), pos + Vec3f(1000/2.0f,0,0));
	BrushSide front(Vec3f(0,0,1), pos + Vec3f(0,0,1000/2.0f));
	BrushSide back(Vec3f(0,0,-1), pos + Vec3f(0,0,-1000/2.0f));
#endif
	b.add(top);		//0
	b.add(bottom);	//1
	b.add(left);	//2
	b.add(right);	//3
	b.add(front);	//4
	b.add(back);	//5
	b.collapse();
	b.remaptex();

	//g_applog<<"---------push back brush-----------"<<std::endl;
	g_edmap.m_brush.push_back(b);
	//g_selB.clear();
	//auto i = g_edmap.m_brush.rbegin();
	//g_selB.push_back(&*i);

	EdPaneType* t = &g_edpanetype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
	//LinkLatestUndo();
}

void Click_LoadEdMap()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("projects\\project", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	FreeEdMap(&g_edmap);

	if(LoadEdMap(filepath, &g_edmap))
		strcpy(g_lastsave, filepath);

	EdPaneType* t = &g_edpanetype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
	ClearUndo();
}

void Click_SaveEdMap()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("projects\\project", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	SaveEdMap(filepath, &g_edmap);
	strcpy(g_lastsave, filepath);
}

void Click_QSaveEdMap()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveEdMap();
		return;
	}

	SaveEdMap(g_lastsave, &g_edmap);
}

void Click_CompileMap()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	CompileMap(filepath, &g_edmap);
}

void Click_ExportBuildingSprite()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\building base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	////PrepareRender(filepath, RENDER_BUILDING);
#endif
}

void Click_ExportUnitSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\unit base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	////PrepareRender(filepath, RENDER_UNIT);
#endif
}

void Click_ExportTileSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\tile base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	////PrepareRender(filepath, RENDER_TERRTILE);
#endif
}

void Click_ExportRoadSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\road base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	////PrepareRender(filepath, RENDER_ROAD);
#endif
}

void RunMap(const char* fullmap)
{
	// TO DO: launch game
#if 1
	char relativeexe[] = "path2";
	char fullexe[1024];
	FullPath(relativeexe, fullexe);
	//char relativemap[] = "temp/tempmap";
	//char relativemap[DMD_MAX_PATH];
	//char fullmap[1024];
	//FullPath(relativemap, fullmap);

	char cmd[DMD_MAX_PATH+2048];
	sprintf(cmd, "%s +devmap %s", fullexe, fullmap);
	system(cmd);
#endif

#if 0
	FreeEdMap(&g_edmap);
	LoadEdMap(full, &g_edmap);
	g_mode = APPMODE_PLAY;
	g_gui.hideall();
	g_gui.show("play");
#endif

#if 0
	//float height = 188;
			float height = 70 * 2.57f;

			PlaceEntity(ENTITY_BATTLECOMPUTER, -1, 1, 0, Vec3f(0, height, 0), rand()%360, NULL);
			PlaceEntity(ENTITY_HUMAN1, -1, 1, 0, Vec3f(-78, height, 0), rand()%360, NULL);
			PlaceEntity(ENTITY_ZOMBIE1, -1, 1, 0, Vec3f(-78, height, 150), rand()%360, NULL);
			PlaceEntity(ENTITY_HUMAN2, -1, 1, 0, Vec3f(-78, height, 200), rand()%360, NULL);
			PlaceEntity(ENTITY_ZOMBIE2, -1, 1, 0, Vec3f(0, height, 120), rand()%360, NULL);
			PlaceEntity(ENTITY_HUMAN3, -1, 1, 0, Vec3f(-78, height, 300), rand()%360, NULL);
			PlaceEntity(ENTITY_ZOMBIE3, -1, 1, 0, Vec3f(0, height, 220), rand()%360, NULL);
			PlaceEntity(ENTITY_BATTLECOMPUTER, -1, 1, 0, Vec3f(100, height, 90), rand()%360, NULL);
			
			//for(int i=0; i<5; i++)
			for(int i=0; i<0; i++)
			{
				int entid;
				PlaceEntity(ENTITY_BATTLECOMPUTER, -1, 1, 0, Vec3f(0, height, 0), rand()%360, &entid);

				Ent* pent = g_entity[entid];
				
				Camera* cam = &pent->camera;
				EType* t = &g_etype[pent->type];

				Vec3f bodymin = pent->bodymin();
				Vec3f bodymax = t->vmax;
				
				cam->grounded(false);

				
				for(int j=0; j<5; j++)
				{
					TraceWork tw;
					Vec3f old = cam->m_pos;

					cam->grounded(false);

					g_edmap.tracebox(&tw, old, old + Vec3f(rand()%200-100, rand()%100-50, rand()%200-100), bodymin, bodymax, t->maxStep);

					cam->moveto(tw.clip);

					if(tw.onground)
						cam->grounded(true);
				}
			}
#endif
}

void Click_CompileRunMap()
{
	/*
	OPENFILENAME ofn;

	char filepath[DMD_MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[DMD_MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = DMD_MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	CompileMap(filepath, &g_edmap);
	*/

	char filepath[DMD_MAX_PATH+1];
	FullPath("temp/tempmap", filepath);
	char initdir[DMD_MAX_PATH+1];
	FullPath("maps/", initdir);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//SaveEdMap(filepath, &g_edmap);
	CompileMap(filepath, &g_edmap);
	RunMap(filepath);
}

void Click_BChooseTex()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("textures/", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("textures/texture", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	LinkPrevUndo();

	unsigned int diffuseindex;
	std::string relativepath = MakeRelative(filepath);
	CreateTex(diffuseindex, relativepath.c_str(), false, true);
	unsigned int texname = g_texture[diffuseindex].texname;

	if(diffuseindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load diffuse texture %s", relativepath.c_str());

		ErrMess("Error", msg);
	}

	char specpath[DMD_MAX_PATH+1];
	strcpy(specpath, relativepath.c_str());
	StripExt(specpath);
	strcat(specpath, ".spec.jpg");

	unsigned int specindex;
	CreateTex(specindex, specpath, false, true);

	if(specindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load specular texture %s", specpath);

		ErrMess("Error", msg);
	}

	char normpath[DMD_MAX_PATH+1];
	strcpy(normpath, relativepath.c_str());
	StripExt(normpath);
	strcat(normpath, ".norm.jpg");

	unsigned int normindex;
	CreateTex(normindex, normpath, false, true);

	if(normindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load normal texture %s", normpath);

		ErrMess("Error", msg);
	}

	char ownpath[DMD_MAX_PATH+1];
	strcpy(ownpath, relativepath.c_str());
	StripExt(ownpath);
	strcat(ownpath, ".team.png");

#if 0
	unsigned int ownindex;
	CreateTex(ownindex, ownpath, false, true);

	if(ownindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load team color texture %s", normpath);

		ErrMess("Error", msg);
	}
#endif

	if(g_sel1b == NULL)
	{
		for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
		{
			Brush* b = *i;

			b->m_texture = diffuseindex;

			for(int j=0; j<b->m_nsides; j++)
			{
				BrushSide* s = &b->m_sides[j];
				s->m_diffusem = diffuseindex;
				s->m_specularm = specindex;
				s->m_normalm = normindex;
				//s->m_ownerm = ownindex;
			}
		}
	}
	else
	{
		Brush* b = g_sel1b;

		b->m_texture = diffuseindex;

		if(g_dragS >= 0)
		{
			BrushSide* s = &b->m_sides[g_dragS];
			s->m_diffusem = diffuseindex;
			s->m_specularm = specindex;
			s->m_normalm = normindex;
			//s->m_ownerm = ownindex;
		}
		else
		{
			for(int i=0; i<b->m_nsides; i++)
			{
				BrushSide* s = &b->m_sides[i];
				s->m_diffusem = diffuseindex;
				s->m_specularm = specindex;
				s->m_normalm = normindex;
				//s->m_ownerm = ownindex;
			}
		}
	}
}

void Click_FitToFace()
{
	LinkPrevUndo();

	if(g_sel1b == NULL)
	{
		for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
		{
			Brush* b = *i;

			for(int j=0; j<b->m_nsides; j++)
				b->m_sides[j].fittex();
		}
	}
	else
	{
		Brush* b = g_sel1b;

		if(g_dragS >= 0)
			b->m_sides[g_dragS].fittex();
		else
		{
			for(int i=0; i<b->m_nsides; i++)
				b->m_sides[i].fittex();
		}
	}

	RedoBSideGUI();
}

void CloseSideView()
{
	g_gui.hide("brush side edit");
	g_gui.hide("brush edit");
	g_gui.hide("door edit");
}

void Click_DoorView()
{
	CloseSideView();
	g_gui.show("door edit");
}

void CopyBrush()
{
	if(g_selB.size() <= 0 && g_selM.size() <= 0)
		return;

	if(g_selB.size() > 0)
	{
		g_copyB = *g_selB[0];
		g_copyM.model = -1;
	}
	else if(g_selM.size() > 0)
	{
		g_copyM = *g_selM[0];
		g_copyB.m_nsides = 0;
	}

	//g_applog<<"copy brush"<<std::endl;
	//g_applog.flush();
}

void PasteBrush()
{
	//g_applog<<"paste brush?"<<std::endl;
	//g_applog.flush();

	if(g_copyB.m_nsides > 0)
	{
		LinkPrevUndo();

		//Vec3f pos = g_focus;
		Vec3f pos = g_cam.m_view;
		pos.x = Snap(g_snapgrid, pos.x);
		pos.y = Snap(g_snapgrid, pos.y);
		pos.z = Snap(g_snapgrid, pos.z);
		Brush b = g_copyB;
		//b.moveto(pos);
	/*
		for(int i=0; i<b.m_nsides; i++)
		{
			BrushSide* s = &b.m_sides[i];

			g_applog<<"side"<<i<<" plane="<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<","<<s->m_plane.m_d<<std::endl;
		}
		*/
		g_edmap.m_brush.push_back(b);

		//g_applog<<"paste brush"<<std::endl;
		//g_applog.flush();
	}
	else if(g_copyM.model >= 0)
	{
		LinkPrevUndo();

		g_modelholder.push_back(g_copyM);
	}
}

void Down_C()
{
	if(g_mode != APPMODE_EDITOR)
		return;

	if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		CopyBrush();
}

void Down_V()
{
	if(g_mode != APPMODE_EDITOR)
		return;

	if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		PasteBrush();
}

void Click_CutBrush()
{
	g_edtool = EDTOOL_CUT;
}

bool OpenFileDialog(char* initdir, char* filepath)
{
#ifdef PLATFORM_WIN

#if 0
	filepath[0] = '\0';
	OPENFILENAME opf={0};
	opf.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrInitialDir = initdir;
	opf.lpstrFile = filepath;
	opf.nMaxFile = DMD_MAX_PATH;
	GetOpenFileName(&opf);
#endif

	memset(filepath, 0, sizeof(char)*DMD_MAX_PATH+1);

	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));

	//strcpy(filepath, initdir);
	filepath[0] = '\0';

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	//ofn.hInstance = g_hInstance;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = DMD_MAX_PATH;
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = DMD_MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return false;

	return true;
#elif defined( PLATFORM_LINUX )
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Open File",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    NULL);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initdir);

    char initfile[DMD_MAX_PATH+1];
    strcpy(initfile, filepath);
    StripPath(initfile);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initfile);

    //gtk_widget_show_all (dialog);
    //gtk_dialog_run (GTK_DIALOG (dialog));

#if 1
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //open_file (filename);

        strcpy(filepath, filename);

        g_free (filename);

		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

        return true;
    }
#endif
    //gtk_widget_hide(dialog);
    gtk_widget_destroy (dialog);

    while (gtk_events_pending ())
        gtk_main_iteration ();

	return false;
#endif
}

bool SaveFileDialog(char* initdir, char* filepath)
{
#ifdef PLATFORM_WIN
	memset(filepath, 0, sizeof(char)*DMD_MAX_PATH+1);

	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));

	//strcpy(filepath, initdir);
	filepath[0] = '\0';

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL  ;
	ofn.hInstance = g_hInstance;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = DMD_MAX_PATH;
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = DMD_MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return false;
	
	return true;
#elif defined( PLATFORM_LINUX )
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Save File",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
    NULL);

	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initdir);

    char initfile[DMD_MAX_PATH+1];
    strcpy(initfile, filepath);
    StripPath(initfile);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initfile);

    //gtk_widget_show_all (dialog);
    //gtk_dialog_run (GTK_DIALOG (dialog));

#if 1
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //open_file (filename);

        strcpy(filepath, filename);

        g_free (filename);

		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

        return true;
    }
#endif
    //gtk_widget_hide(dialog);
    gtk_widget_destroy (dialog);

    while (gtk_events_pending ())
        gtk_main_iteration ();

	return false;
#endif
}

void Click_NewEnt()
{
	//InfoMess("ne","n");
	g_gui.show("place entity");
}

void Click_AddMS3D()
{
	char filepath[DMD_MAX_PATH+1];

	char initdir[DMD_MAX_PATH+1];
	FullPath("models\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("models\\model", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	std::string relative = MakeRelative(filepath);

	g_applog<<"relative "<<relative<<std::endl;

	int modelid = LoadModel(relative.c_str(), Vec3f(1,1,1), Vec3f(0,0,0), true, true);

	if(modelid < 0)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load model %s", relative.c_str());

		ErrMess("Error", msg);

		return;
	}

	LinkPrevUndo();

	Vec3f focus = g_cam.m_view;
	ModelHolder mh(modelid, focus);
	g_modelholder.push_back(mh);
}

void RotateModels(float degrees, float xaxis, float yaxis, float zaxis)
{
	LinkPrevUndo();

	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		pmh->rotdegrees = pmh->rotdegrees + Vec3f( degrees*xaxis, degrees*yaxis, degrees*zaxis );
		pmh->retransform();
	}
}

void RotateBrushes(float radians, Vec3f axis)
{
	LinkPrevUndo();
	//EdMap* m = &g_edmap;

	//for(auto i=map->m_brush.begin(); i!=map->m_brush.end(); i++)
	for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;
		Vec3f centroid(0,0,0);

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* s = &b->m_sides[j];

			centroid = centroid + s->m_centroid;
		}

		centroid = centroid / (float)b->m_nsides;

		std::list<float> oldus;
		std::list<float> oldvs;

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* s = &b->m_sides[j];

			RotatePlane(s->m_plane, centroid, radians, axis);
			//RotatePlane(s->m_tceq[0], centroid, radians, axis);
			//RotatePlane(s->m_tceq[1], centroid, radians, axis);
			//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
			//RotatePlane(s->m_tceq[0], s->m_centroid, radians, axis);
			//RotatePlane(s->m_tceq[1], s->m_centroid, radians, axis);
			//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			//s->m_tceq[0].m_normal = RotateAround(s->m_tceq[0].m_normal, sharedv, radians, axis.x, axis.y, axis.z);
			//s->m_tceq[1].m_normal = RotateAround(s->m_tceq[1].m_normal, sharedv, radians, axis.x, axis.y, axis.z);
			//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			//Vec3f offcenter = sharedv - centroid;
			//Vec3f newoffcenter = Rotate(offcenter, radians, axis.x, axis.y, axis.z);
			//float uminusd = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z;
			//float vminusd = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z;
			//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal - offcenter, radians, axis.x, axis.y, axis.z) + newoffcenter;
			//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal - offcenter, radians, axis.x, axis.y, axis.z) + newoffcenter;
			//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
			//float newuminusd = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z;
			//float newvminusd = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z;
			//float uscale = uminusd / newuminusd;
			//float vscale = vminusd / newvminusd;
			//s->m_tceq[0].m_normal = s->m_tceq[0].m_normal * uscale;
			//s->m_tceq[1].m_normal = s->m_tceq[1].m_normal * vscale;
			/*
			Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			float oldu = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float oldv = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
			float newu = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - oldu;
			float changev = newv - oldv;
			s->m_tceq[0].m_d += changeu;
			s->m_tceq[1].m_d += changev;
			*/
			Vec3f sharedv = *s->m_sideverts.begin();
			float u = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float v = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
		}

		b->collapse();

		auto oldu = oldus.begin();
		auto oldv = oldvs.begin();

		for(int j=0; j<b->m_nsides; j++, oldu++, oldv++)
		{
			BrushSide* s = &b->m_sides[j];

			Vec3f newsharedv = *s->m_sideverts.begin();

			float newu = newsharedv.x*s->m_tceq[0].m_normal.x + newsharedv.y*s->m_tceq[0].m_normal.y + newsharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = newsharedv.x*s->m_tceq[1].m_normal.x + newsharedv.y*s->m_tceq[1].m_normal.y + newsharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->m_tceq[0].m_d -= changeu;
			s->m_tceq[1].m_d -= changev;
		}

		b->remaptex();
	}
}

void Click_RotXCCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(1, 0, 0));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 1, 0, 0);
}

void Click_RotXCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(1, 0, 0));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 1, 0, 0);
}

void Click_RotYCCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(0, 1, 0));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 0, 1, 0);
}

void Click_RotYCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(0, 1, 0));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 0, 1, 0);
}

void Click_RotZCCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(0, 0, 1));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 0, 0, 1);
}

void Click_RotZCW()
{
	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(0, 0, 1));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 0, 0, 1);
}

void Click_RotateTex()
{
	LinkPrevUndo();

	float degrees = StrToFloat(g_gui.get("editor")->get("top panel")->get("rotdeg")->m_value.rawstr().c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->m_sides[g_dragS];

	//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
	float oldu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
	float oldv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
	Vec3f axis = s->m_plane.m_normal;
	float radians = DEGTORAD(degrees);
	s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
	s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);

	//Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];
	float newu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
	float newv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
	float changeu = newu - oldu;
	float changev = newv - oldv;
	s->m_tceq[0].m_d -= changeu;
	s->m_tceq[1].m_d -= changev;

	s->remaptex();
}

void RedoBSideGUI()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;
	
	if(!g_gui.get("brush side edit"))	InfoMess("e", "no bse");
	if(!g_gui.get("brush side edit")->get("left panel"))	InfoMess("e", "no lp");
	if(!g_gui.get("brush side edit")->get("left panel")->get("u equation"))	InfoMess("e", "no ue");
	if(!g_gui.get("brush side edit")->get("left panel")->get("v equation"))	InfoMess("e", "no ve");

	EditBox* uwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("u equation");
	EditBox* vwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("v equation");

	Plane3f* tceq = g_sel1b->m_sides[g_dragS].m_tceq;

	char tceqstr[256];
	sprintf(tceqstr, "%f %f %f %f", tceq[0].m_normal.x, tceq[0].m_normal.y, tceq[0].m_normal.z, tceq[0].m_d);
	RichText tceqrstr = RichText(tceqstr);
	uwidg->changevalue(&tceqrstr);
	sprintf(tceqstr, "%f %f %f %f", tceq[1].m_normal.x, tceq[1].m_normal.y, tceq[1].m_normal.z, tceq[1].m_d);
	tceqrstr = RichText(tceqstr);
	vwidg->changevalue(&tceqrstr);

	uwidg->m_scroll[0] = 0;
	vwidg->m_scroll[0] = 0;
}

void Change_TexEq(unsigned int c, unsigned int scancode, bool down, int parm)
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	EditBox* uwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("u equation");
	EditBox* vwidg = (EditBox*)g_gui.get("brush side edit")->get("left panel")->get("v equation");

	float A = 0;
	float B = 0;
	float C = 0;
	float D = 0;

	sscanf(uwidg->m_value.rawstr().c_str(), "%f %f %f %f", &A, &B, &C, &D);

	Plane3f* tceq = g_sel1b->m_sides[g_dragS].m_tceq;
	tceq[0].m_normal.x = A;
	tceq[0].m_normal.y = B;
	tceq[0].m_normal.z = C;
	tceq[0].m_d = D;

	A = 0;
	B = 0;
	C = 0;
	D = 0;

	sscanf(vwidg->m_value.rawstr().c_str(), "%f %f %f %f", &A, &B, &C, &D);

	tceq[1].m_normal.x = A;
	tceq[1].m_normal.y = B;
	tceq[1].m_normal.z = C;
	tceq[1].m_d = D;

	g_sel1b->m_sides[g_dragS].remaptex();
}

void Change_SelComp()
{
	//RedoBSideGUI();
}

int GetComponent()
{
	int c = g_gui.get("brush side edit")->get("left panel")->get("select component")->m_selected;

	if(c != 0 && c != 1)
		c = 0;

	return c;
}

void Click_ScaleTex()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	int c = GetComponent();

	float scale = 1.0f/StrToFloat(g_gui.get("brush side edit")->get("left panel")->get("texture scale")->m_value.rawstr().c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->m_sides[g_dragS];

	float oldcomp = s->m_centroid.x*s->m_tceq[c].m_normal.x + s->m_centroid.y*s->m_tceq[c].m_normal.y + s->m_centroid.z*s->m_tceq[c].m_normal.z + s->m_tceq[c].m_d;
	s->m_tceq[c].m_normal = s->m_tceq[c].m_normal * scale;

	//Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];
	float newcomp = s->m_centroid.x*s->m_tceq[c].m_normal.x + s->m_centroid.y*s->m_tceq[c].m_normal.y + s->m_centroid.z*s->m_tceq[c].m_normal.z + s->m_tceq[c].m_d;
	float changecomp = newcomp - oldcomp;
	s->m_tceq[c].m_d -= changecomp;

	s->remaptex();

	RedoBSideGUI();
}

void Click_ShiftTex()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	int c = GetComponent();

	float shift = StrToFloat(g_gui.get("brush side edit")->get("left panel")->get("texture shift")->m_value.rawstr().c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->m_sides[g_dragS];

	s->m_tceq[c].m_d += shift;

	s->remaptex();

	RedoBSideGUI();
}

void Change_ShowSky()
{
	Widget* showskchbox = g_gui.get("editor")->get("top panel")->get("showsky");

	g_showsky = (bool)showskchbox->m_selected;
}

void Click_ProjPersp()
{
	g_projtype = PROJ_PERSP;
}

void Click_ProjOrtho()
{
	g_projtype = PROJ_ORTHO;
}

void Click_ResetView()
{
	ResetView();
}

void Click_Explode()
{
	g_edtool = EDTOOL_EXPLOSION;
}

void Click_SetDoor()
{
	if(g_selB.size() <= 0)
	{
		//MessageBox(g_hWnd, "NULL 1b", "asdasd", NULL);
		return;
	}

	LinkPrevUndo();

	Brush* b = *g_selB.begin();
	EdDoor* door = b->m_door;

	if(!b->m_door)
	{
		b->m_door = new EdDoor();
		door = b->m_door;

		door->axis = Vec3f(0,200,0);
		door->point = b->m_sides[2].m_centroid;
		door->startopen = false;
	}

	float opendeg = 90;

	ViewLayer* dooredview = (ViewLayer*)g_gui.get("door edit");
	EditBox* opendegedit = (EditBox*)dooredview->get("opendeg");
	opendeg = StrToFloat(opendegedit->m_value.rawstr().c_str());

	door->opendeg = opendeg;

	b->getsides(&door->m_nsides, &door->m_sides);

#if 0
	g_applog<<"set ed door"<<std::endl;
	for(int i=0; i<door->m_nsides; i++)
	{
		g_applog<<"side "<<i<<std::endl;
		Plane3f* p = &door->m_sides[i].m_plane;

		g_applog<<"plane = "<<p->m_normal.x<<","<<p->m_normal.y<<","<<p->m_normal.z<<",d="<<p->m_d<<std::endl;
	}
#endif
}

void Click_UnmakeDoor()
{
	if(g_selB.size() <= 0)
	{
		//MessageBox(g_hWnd, "NULL 1b", "asdasd", NULL);
		return;
	}

	Brush* b = *g_selB.begin();

	if(b->m_door)
	{
		LinkPrevUndo();
		delete [] b->m_door;
		b->m_door = NULL;
	}
}

void Click_OpenCloseDoor()
{
	if(g_selB.size() <= 0)
		return;

	Brush* b = *g_selB.begin();
	EdDoor* door = b->m_door;

	if(!b->m_door)
		return;

	LinkPrevUndo();

	Brush transformed;
	transformed.setsides(door->m_nsides, door->m_sides);
	transformed.collapse();
	transformed.remaptex();

	door->startopen = !door->startopen;

	if(door->startopen)
	{
		//transform

		Vec3f centroid = door->point;
		Vec3f axis = Normalize(door->axis);
		float radians = DEGTORAD(door->opendeg);

		//char msg[128];
		//sprintf(msg, "point(%f,%f,%f) axis(%f,%f,%f)

		std::list<float> oldus;
		std::list<float> oldvs;

		for(int j=0; j<transformed.m_nsides; j++)
		{
			BrushSide* s = &transformed.m_sides[j];
			RotatePlane(s->m_plane, centroid, radians, axis);

			Vec3f sharedv = *s->m_sideverts.begin();
			float u = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float v = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
		}

		//transformed.collapse();

		auto oldu = oldus.begin();
		auto oldv = oldvs.begin();

		for(int j=0; j<transformed.m_nsides; j++, oldu++, oldv++)
		{
			BrushSide* s = &transformed.m_sides[j];

			Vec3f newsharedv = *s->m_sideverts.begin();

			float newu = newsharedv.x*s->m_tceq[0].m_normal.x + newsharedv.y*s->m_tceq[0].m_normal.y + newsharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = newsharedv.x*s->m_tceq[1].m_normal.x + newsharedv.y*s->m_tceq[1].m_normal.y + newsharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->m_tceq[0].m_d -= changeu;
			s->m_tceq[1].m_d -= changev;
		}

		//transformed.remaptex();
	}

	b->setsides(transformed.m_nsides, transformed.m_sides);
	b->collapse();
	b->remaptex();
}

void Resize_LoadingText(Widget* thisw)
{
	thisw->m_pos[0] = g_width * 0.4f;
	thisw->m_pos[1] = g_height * 0.5f;
}

void Resize_TopPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT;
}

void Resize_LeftPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_LoadButton(Widget* thisw)
{
	int i = 0;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;

#if 0
	char msg[128];
	sprintf(msg, "pos:%f,%f,%f,%f", thisw->m_pos[0], thisw->m_pos[1], thisw->m_pos[2], thisw->m_pos[3]);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif
}

void Resize_SaveButton(Widget* thisw)
{
	int i = 1;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_QSaveButton(Widget* thisw)
{
	int i = 2;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_CompileRunButton(Widget* thisw)
{
	int i = 3;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_CompileMapButton(Widget* thisw)
{
	int i = 3;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportBldgButton(Widget* thisw)
{
	int i = 4;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportUnitButton(Widget* thisw)
{
	int i = 5;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportTileButton(Widget* thisw)
{
	int i = 6;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportRoadButton(Widget* thisw)
{
	int i = 7;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportRidgeButton(Widget* thisw)
{
	int i = 8;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportWaterButton(Widget* thisw)
{
	int i = 9;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_UndoButton(Widget* thisw)
{
	int i = 10;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RedoButton(Widget* thisw)
{
	int i = 11;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_NewBrushButton(Widget* thisw)
{
	int i = 12;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_CutBrushButton(Widget* thisw)
{
	int i = 13;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_NewEntityButton(Widget* thisw)
{
	int i = 14;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}


void Resize_NewModelButton(Widget* thisw)
{
	int i = 15;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_SelEntButton(Widget* thisw)
{
	int i = 16;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_SelBrushButton(Widget* thisw)
{
	int i = 17;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotXCCWButton(Widget* thisw)
{
	int i = 18;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotXCWButton(Widget* thisw)
{
	int i = 19;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotYCCWButton(Widget* thisw)
{
	int i = 20;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotYCWButton(Widget* thisw)
{
	int i = 21;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotZCCWButton(Widget* thisw)
{
	int i = 22;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotZCWButton(Widget* thisw)
{
	int i = 23;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ViewportsFrame(Widget* thisw)
{
	thisw->m_pos[0] = LEFT_PANEL_WIDTH;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_TopLeftViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = parentw->m_pos[0];
	thisw->m_pos[1] = parentw->m_pos[1];
	thisw->m_pos[2] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[3] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
}

void Resize_BottomLeftViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = parentw->m_pos[0];
	thisw->m_pos[1] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
	thisw->m_pos[2] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[3] = parentw->m_pos[3];
}

void Resize_TopRightViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[1] = parentw->m_pos[1];
	thisw->m_pos[2] = parentw->m_pos[2];
	thisw->m_pos[3] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
}

void Resize_BottomRightViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[1] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
	thisw->m_pos[2] = parentw->m_pos[2];
	thisw->m_pos[3] = parentw->m_pos[3];
}

void Resize_FullViewport(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_HDivider(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = parentw->m_pos[0];
	thisw->m_pos[1] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
	thisw->m_pos[2] = parentw->m_pos[2];
	thisw->m_pos[3] = (parentw->m_pos[3]+parentw->m_pos[1])/2 + 1;
}

void Resize_VDivider(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[1] = parentw->m_pos[1];
	thisw->m_pos[2] = (parentw->m_pos[2]+parentw->m_pos[0])/2 + 1;
	thisw->m_pos[3] = parentw->m_pos[3];
}

void Resize_RotDegEditBox(Widget* thisw)
{
	int i = 0;
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_RotDegText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 2;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
	thisw->m_pos[2] = thisw->m_pos[0] + 164;
	thisw->m_pos[3] = thisw->m_pos[1] + 164;
}

void Resize_ZoomEditBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 2;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_ZoomText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 4;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
	thisw->m_pos[2] = thisw->m_pos[0] + 164;
	thisw->m_pos[3] = thisw->m_pos[1] + 164;
}

void Resize_SnapGridEditBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(50+32*i++), Margin(32+16)
	int i = 4;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 50+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_SnapGridText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 6;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
	thisw->m_pos[2] = thisw->m_pos[0] + 164;
	thisw->m_pos[3] = thisw->m_pos[1] + 164;
}

void Resize_MaxElevEditBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 6;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_MaxElevText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 8;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
	thisw->m_pos[2] = thisw->m_pos[0] + 164;
	thisw->m_pos[3] = thisw->m_pos[1] + 164;
}

void Resize_ShowSkyCheckBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 8;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_PerspProjButton(Widget* thisw)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 10;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
	CenterLabel(thisw);
}

void Resize_OrthoProjButton(Widget* thisw)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 11;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
	CenterLabel(thisw);
}

void Resize_ResetViewButton(Widget* thisw)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 12;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
	CenterLabel(thisw);
}

void Resize_FramesText(Widget* thisw)
{
	int i = 13;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32 + 16;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
}

void Resize_FramesEditBox(Widget* thisw)
{
	int i = 13;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 16;
}

void Resize_ExplodeButton(Widget* thisw)
{
	int i = 14;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 32;
	CenterLabel(thisw);
}

void Resize_AddTileButton(Widget* thisw)
{
	int i = 15;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 32;
	CenterLabel(thisw);
}

void Resize_LeadsNECheckBox(Widget* thisw)
{
	int i = 16;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 16;
	CenterLabel(thisw);
}

void Resize_LeadsSECheckBox(Widget* thisw)
{
	int i = 16;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32 + 16;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 32;
	CenterLabel(thisw);
}

void Resize_LeadsSWCheckBox(Widget* thisw)
{
	int i = 18;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 16;
	CenterLabel(thisw);
}

void Resize_LeadsNWCheckBox(Widget* thisw)
{
	int i = 18;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32 + 16;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 32;
	CenterLabel(thisw);
}

void Change_Leads()
{
}

void Click_AddTile()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("textures/", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("textures/texture", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	LinkPrevUndo();

	unsigned int diffuseindex;
	std::string relativepath = MakeRelative(filepath);
	CreateTex(diffuseindex, relativepath.c_str(), false, true);
	unsigned int texname = g_texture[diffuseindex].texname;

	if(diffuseindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load diffuse texture %s", relativepath.c_str());

		ErrMess("Error", msg);
	}

	char specpath[DMD_MAX_PATH+1];
	strcpy(specpath, relativepath.c_str());
	StripExt(specpath);
	strcat(specpath, ".spec.jpg");

	unsigned int specindex;
	CreateTex(specindex, specpath, false, true);

	if(specindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load specular texture %s", specpath);

		ErrMess("Error", msg);
	}

	char normpath[DMD_MAX_PATH+1];
	strcpy(normpath, relativepath.c_str());
	StripExt(normpath);
	strcat(normpath, ".norm.jpg");

	unsigned int normindex;
	CreateTex(normindex, normpath, false, true);

	if(normindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load normal texture %s", normpath);

		ErrMess("Error", msg);
	}

#if 0
	char ownpath[DMD_MAX_PATH+1];
	strcpy(ownpath, relativepath.c_str());
	StripExt(ownpath);
	strcat(ownpath, ".team.png");

	unsigned int ownindex;
	CreateTex(ownindex, ownpath, false, true);

	if(ownindex == 0 && !g_supresserr)
	{
		char msg[DMD_MAX_PATH+1];
		sprintf(msg, "Couldn't load team color texture %s", normpath);

		ErrMess("Error", msg);
	}
#endif

#if 0	//no tiles for path2
	g_tiletexs[TEX_DIFF] = diffuseindex;
	g_tiletexs[TEX_SPEC] = specindex;
	g_tiletexs[TEX_NORM] = normindex;
	g_tiletexs[TEX_TEAM] = ownindex;
#endif
}

void Change_Frames(unsigned int dummy, unsigned int scancode, bool down, int parm)
{
}

void Resize_BrushEditFrame(Widget* thisw)
{
	//Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_SetDoor(Widget* thisw)
{
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = 32;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT + 32;
	CenterLabel(thisw);
}

void Resize_UnmakeDoor(Widget* thisw)
{
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 32;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = 64;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT + 32;
	CenterLabel(thisw);
}
void Resize_OpenCloseDoor(Widget* thisw)
{
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1))
	int i=0;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT + 32 + f->gheight*i;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT + 32 + f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_OpenDoorDegrees(Widget* thisw)
{
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1))
	int i=1;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT + 32 + f->gheight*i;
	thisw->m_pos[2] = 70;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT + 32 + f->gheight*(i+1);
	CenterLabel(thisw);
}
void Resize_OpenDoorDegreesEd(Widget* thisw)
{
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1))
	int i=1;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 40;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT + 32 + f->gheight*i;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT + 32 + f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_ChooseTexButton(Widget* thisw)
{
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1))
	int i=1;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = parentw->m_pos[1] + f->gheight*i;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_FitToFaceButton(Widget* thisw)
{
	//Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32)
	int i=2;
	int j=0;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = parentw->m_pos[1] + f->gheight*i;
	thisw->m_pos[2] = 32+32*j;
	thisw->m_pos[3] = parentw->m_pos[1] + f->gheight*i+32;
	CenterLabel(thisw);
}

void Resize_DoorView(Widget* thisw)
{
	//Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32)
	int i=2;
	int j=1;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 32;
	thisw->m_pos[1] = parentw->m_pos[1] + f->gheight*i;
	thisw->m_pos[2] = 32+32*j;
	thisw->m_pos[3] = parentw->m_pos[1] + f->gheight*i+32;
	CenterLabel(thisw);
}

void Resize_BrushSideEditFrame(Widget* thisw)
{
	int j=1;
	//leftpanel->add(new Button(leftpanel, "door view", "gui/door.png", "", "Door view",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_DoorView, NULL, NULL));
	int i=4;
	//Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_RotateTexButton(Widget* thisw)
{
	int i = 4;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_TexEqEditBox1(Widget* thisw)
{
	int i = 6;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexEqEditBox2(Widget* thisw)
{
	int i = 7;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_SelComponentDropList(Widget* thisw)
{
	int i = 8;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexScaleEditBox(Widget* thisw)
{
	int i = 9;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_ScaleTexButton(Widget* thisw)
{
	int i = 9;
	//MAINFONT8, Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_TexShiftEditBox(Widget* thisw)
{
	int i = 10;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexShiftButton(Widget* thisw)
{
	int i = 10;
	//Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
	CenterLabel(thisw);
}

void Click_CompileModel()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
#ifdef PLATFORM_WIN
        OPENFILENAME ofn;

        char filepath[DMD_MAX_PATH+1];

        ZeroMemory( &ofn , sizeof( ofn));

        char initdir[DMD_MAX_PATH+1];
        FullPath("export models\\", initdir);
        CorrectSlashes(initdir);
        //strcpy(filepath, initdir);
        FullPath("export models\\export", filepath);
        CorrectSlashes(filepath);

        ofn.lStructSize = sizeof ( ofn );
        ofn.hwndOwner = NULL  ;
        ofn.lpstrInitialDir = initdir;
        ofn.lpstrFile = filepath;
        //ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof( filepath );
        //ofn.lpstrFilter = "Save\0*.bl\0All\0*.*\0";
        ofn.lpstrFilter = "All\0*.*\0";
        ofn.nFilterIndex =1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = DMD_MAX_PATH;        //0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if(!GetSaveFileName(&ofn))
                return;

        //CorrectSlashes(filepath);
        //SaveEdBuilding(filepath, &g_edbldg);
		////CompileModel(filepath, &g_edmap, g_modelholder);
#elif defined( PLATFORM_LINUX )
	GtkWidget *dialog;

     dialog = gtk_file_chooser_dialog_new ("Open File",
     				      NULL,
     				      GTK_FILE_CHOOSER_ACTION_OPEN,
     				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
     				      NULL);

     if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
       {
         char *filename;

         filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         //open_file (filename);
         g_free (filename);
       }

     gtk_widget_destroy (dialog);
#endif
#endif
}

void Resize_ChooseFile(Widget *thisw)
{
	thisw->m_pos[0] = g_width/2 - 200;
	thisw->m_pos[1] = g_height/2 - 200;
	thisw->m_pos[2] = g_width/2 + 200;
	thisw->m_pos[3] = g_height/2 + 200;
}

void Callback_ChooseFile(const char* fullpath)
{
}

void Resize_PlayText(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
}

void MouseWheel(int delta)
{
#if 0
	if(g_zoom <= MIN_ZOOM && delta < 0)
		return;

	if(g_zoom >= MAX_ZOOM && delta > 0)
		return;
#endif
	g_zoom *= 1.0f + (float)delta / 10.0f;
}

void Click_NewGame()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	FreeEdMap(&g_edmap);

	RunMap(filepath);

	EdPaneType* t = &g_edpanetype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
	ClearUndo();
}

void Click_LoadSaved()
{
	char filepath[DMD_MAX_PATH+1];
	char initdir[DMD_MAX_PATH+1];
	FullPath("saves\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("saves\\save", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	FreeEdMap(&g_edmap);

	RunMap(filepath);

	EdPaneType* t = &g_edpanetype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
	ClearUndo();
}

void Click_MapEditor()
{
	FreeEdMap(&g_edmap);

	g_mode = APPMODE_EDITOR;
	g_gui.hideall();
	g_gui.show("editor");
}

void Click_Quit()
{
	EndSess();
	FreeMap();

	FreeEdMap(&g_edmap);
	g_quit = true;
}

void Resize_Splat(Widget* thisw)
{
#if 0
	Font* f = &g_font[MAINFONT64];
	thisw->m_pos[0] = g_width/2 - f->gheight * 5 - 64;
	thisw->m_pos[1] = g_height/2 - f->gheight * 3 + f->gheight*0*1.5f - 64;
	thisw->m_pos[2] = thisw->m_pos[0] + 256;
	thisw->m_pos[3] = thisw->m_pos[1] + 256;
#else
	Font* f = &g_font[MAINFONT64];
	thisw->m_pos[0] = 10;
	thisw->m_pos[1] = g_height/2 - f->gheight * 3 + f->gheight*0*1.5f - 64;
	thisw->m_pos[2] = thisw->m_pos[0] + 256;
	thisw->m_pos[3] = thisw->m_pos[1] + 256;
#endif
}

#if 0
void Resize_MenuItem(Widget* thisw)
{
	int i =0;

	sscanf(thisw->m_name.c_str(), "%d", &i);

	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = g_width/2 - f->gheight * 5;
	thisw->m_pos[1] = g_height/2 - f->gheight * 2 + f->gheight*i*1.5f;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;

	if(i == - 1)
	{
	thisw->m_pos[0] = g_width/2 - f->gheight * 5;
	thisw->m_pos[1] = g_height/2 - f->gheight * 3 + f->gheight*0*1.5f;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
	}
	else if(i == -3)
	{
		f = &g_font[MAINFONT32];
		thisw->m_pos[0] = g_width/2 + f->gheight * 1;
		thisw->m_pos[1] = g_height/2 - f->gheight * 2 + f->gheight*0*1.5f;
		thisw->m_pos[2] = g_width - f->gheight*1;
		thisw->m_pos[3] = g_height;

		for(int j=0; j<4; j++)
			thisw->m_crop[j] = thisw->m_pos[j];
	}
}
#endif

void Resize_PlaceEntBG(Widget* thisw)
{
	thisw->m_pos[0] = g_width/2 - 200;
	thisw->m_pos[1] = g_height/2 - 200;
	thisw->m_pos[2] = g_width/2 + 200;
	thisw->m_pos[3] = g_height/2 + 200;
}

void Resize_PlaceEntWhat(Widget* thisw)
{
	Font* f = &g_font[thisw->m_font];
	Widget* parw = thisw->m_parent;
	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 10;
	thisw->m_pos[2] = thisw->m_pos[0] + 150;
	thisw->m_pos[3] = thisw->m_pos[1] + 16;
}

void Resize_PlaceEntCancel(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	thisw->m_pos[0] = parw->m_pos[0] + 100;
	thisw->m_pos[1] = parw->m_pos[1] + 200 + 200 - 32;
	thisw->m_pos[2] = thisw->m_pos[0] + 100;
	thisw->m_pos[3] = thisw->m_pos[1] + 32;
	CenterLabel(thisw);
}

void Resize_PlaceEntOK(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[1] + 200 + 200 - 32;
	thisw->m_pos[2] = thisw->m_pos[0] + 100;
	thisw->m_pos[3] = thisw->m_pos[1] + 32;
	CenterLabel(thisw);
}

void Click_PlaceEntCancel()
{
	g_gui.hide("place entity");
}

void Click_PlaceEntDelete()
{
	g_gui.hide("place entity");

	if(g_selE)
	{
		FreeEnt(g_selE);
		g_selE = NULL;
	}
	else
	{
		RichText mess = RichText("No entity selected.");
		Mess(&mess);
	}
}

void Click_PlaceEntOK()
{
	int which = -1;
	int item = -1;

	ViewLayer* peview =(ViewLayer*)g_gui.get("place entity");
	DropList* pwhat = (DropList*)peview->get("what");

	int etype = pwhat->m_selected;

	//drop new entity
	if(!g_selE)
	{
		int eid;
		Vec3f focus = g_cam.m_view;
		if(PlaceEntity(etype, -1, 1, 0, focus, 0, &eid, false, -1))
		{
			g_selE = &g_entity[eid];
		}
	}
	//modify selected entity's properties
	else
	{
	}

#if 0
	switch(pwhat->m_selected)
	{
	case 0:
		which = ENTITY_BATTLECOMPUTER;
		break;
	case 1:
		which = ENTITY_BATTLECOMPUTER;
		item = ENTITY_MP5;
		break;
	case 2:
		which = ENTITY_BATTLECOMPUTER;
		item = ENTITY_UMP;
		break;
	case 3:
		which = ENTITY_BATTLECOMPUTER;
		item = ENTITY_MOSSBERG500;
		break;
	case 4:
		which = ENTITY_BATTLECOMPUTER;
		item = ENTITY_AK47;
		break;
	case 5:
		which = ENTITY_BATTLECOMPUTER;
		item = ENTITY_M1911;
		break;
	case 6:
		which = ENTITY_HUMAN1;
		break;
	case 7:
		which = ENTITY_HUMAN2;
		break;
	case 8:
		which = ENTITY_HUMAN3;
		break;
	case 9:
		which = ENTITY_ZOMBIE1;
		break;
	case 10:
		which = ENTITY_ZOMBIE2;
		break;
	case 11:
		which = ENTITY_ZOMBIE3;
		break;
	}

	g_gui.hide("place entity");

	if(which < 0)
		return;

	int hID;
	EType* t = &g_etype[which];
	PlaceEntity(which, -1, 0, 0, g_cam.m_view - Vec3f(0,t->vmin.y,0) + Vec3f(0,2,0), rand()%360, &hID);

	if(item < 0)
		return;

	int iID;
	t = &g_etype[item];
	PlaceEntity(item, -1, 0, 0, g_cam.m_view - Vec3f(0,t->vmin.y,0) + Vec3f(0,2,0), rand()%360, &iID);

	Equip(hID, iID);
#endif
}

void Resize_Win(Widget* thisw)
{
	thisw->m_pos[0] = 30;
	thisw->m_pos[1] = 30;
	thisw->m_pos[2] = 150;
	thisw->m_pos[3] = 200;
}

void FillGUI()
{
	g_applog<<"assign keys"<<std::endl;
	g_applog.flush();

	g_gui.assignanykey(&AnyKeyDown, &AnyKeyUp);

	g_applog<<"1,";
	g_applog.flush();

	g_gui.assignkey(SDL_SCANCODE_W, Down_Forward, Up_Forward);
	g_gui.assignkey(SDL_SCANCODE_A, Down_Left, Up_Left);
	g_gui.assignkey(SDL_SCANCODE_S, Down_Backward, Up_Backward);
	g_gui.assignkey(SDL_SCANCODE_D, Down_Right, Up_Right);
	g_gui.assignkey(SDL_SCANCODE_R, Down_Rise, Up_Rise);
	g_gui.assignkey(SDL_SCANCODE_F, Down_Fall, Up_Fall);

	g_gui.assignkey(SDL_SCANCODE_F1, SaveScreenshot, NULL);
	g_applog<<"2,";
	g_applog.flush();
	g_gui.assignkey(SDL_SCANCODE_ESCAPE, &Escape, NULL);
	g_applog<<"3,";
	g_applog.flush();
	g_gui.assignkey(SDL_SCANCODE_DELETE, &Delete, NULL);
	g_applog<<"4,";
	g_applog.flush();
	g_gui.assignkey(SDL_SCANCODE_C, Down_C, NULL);
	g_applog<<"5,";
	g_applog.flush();
	g_gui.assignkey(SDL_SCANCODE_V, Down_V, NULL);
	g_applog<<"5,";
	g_applog.flush();
	g_gui.assignmousewheel(MouseWheel);
	//AssignMouseWheel(&MouseWheel);
	//AssignMouseMove(&MouseMove);
	g_gui.assignlbutton(&MouseLeftButtonDown, &MouseLeftButtonUp);
	g_gui.assignrbutton(&MouseRightButtonDown, &MouseRightButtonUp);
	g_gui.assignmbutton(MouseMidButtonDown, NULL);
	g_gui.assignmousemove(MouseMove);
	//AssignRButton(NULL, &MouseRightButtonUp);
	g_applog<<"6,"<<std::endl;
	g_applog.flush();

	Font* f = &g_font[MAINFONT8];

	g_applog<<"logo..."<<std::endl;
	g_applog.flush();

	
	Log("2.2");
	
	GUI* gui = &g_gui;

	
	gui->add(new ViewLayer(gui, "logo"));
	ViewLayer* logoview = (ViewLayer*)gui->get("logo");
	logoview->add(new Image(logoview, "", "gui/mmbg.jpg", true, Resize_Fullscreen));
	//logoview->add(new Image(logoview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	//logoview->add(new Text(logoview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	logoview->add(new Image(logoview, "logo", "gui/centerp/dmd.png", true, Resize_Logo, 1, 1, 1, 0));

	gui->add(new ViewLayer(gui, "loading"));
	ViewLayer* loadview = (ViewLayer*)gui->get("loading");

	Log("2.3");
	

	loadview->add(new Image(loadview, "loadbg", "gui/mmbg.jpg", true, Resize_Fullscreen));
	//loadview->add(new Image(loadview, "", "gui/centerp/3d.png", true, Resize_AppLogo));
	//loadview->add(new Text(loadview, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	loadview->add(new Text(loadview, "status", RichText("Loading..."), MAINFONT8, Resize_LoadingStatus));
	loadview->add(new InsDraw(loadview, Draw_LoadBar));

	Log("2.4");
	
	FillMenu();

	g_applog<<"loading..."<<std::endl;
	g_applog.flush();

	g_gui.add(new ViewLayer(&g_gui, "editor"));
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	edview->add(new Frame(edview, "top panel", Resize_TopPanel));
	edview->add(new Frame(edview, "left panel", Resize_LeftPanel));

	Widget* toppanel = edview->get("top panel");
	Widget* leftpanel = edview->get("left panel");

	toppanel->add(new Image(toppanel, "top panel bg", "gui/filled.jpg", true, Resize_TopPanel));
	leftpanel->add(new Image(leftpanel, "left panel bg", "gui/filled.jpg", true, Resize_LeftPanel));

//Button(Widget* parent, const char* name, const char* filepath, const std::string label, const std::string tooltip, int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm) : Widget()


	toppanel->add(new Button(toppanel, "load", "gui/load.png", "", "Load",												MAINFONT8, BUST_LEFTIMAGE, Resize_LoadButton, Click_LoadEdMap, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "save", "gui/save.png", "", "Save",												MAINFONT8, BUST_LEFTIMAGE, Resize_SaveButton, Click_SaveEdMap, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "qsave", "gui/qsave.png", "", "Quick save",										MAINFONT8, BUST_LEFTIMAGE, Resize_QSaveButton, Click_QSaveEdMap, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "run", "gui/run.png", "", "Compile",										MAINFONT8, BUST_LEFTIMAGE, Resize_CompileRunButton, Click_CompileRunMap, NULL, NULL, NULL, NULL, -1, NULL));
#if 0
#if 1
	toppanel->add(new Button(toppanel, "build", "gui/build.png", "", "Export model",									MAINFONT8, BUST_LEFTIMAGE, Resize_CompileMapButton, Click_CompileModel, NULL, NULL, NULL, NULL, -1));
#endif
	toppanel->add(new Button(toppanel, "build", "gui/buildbuilding.png", "", "Export building/tree/animation sprites",	MAINFONT8, BUST_LEFTIMAGE, Resize_ExportBldgButton, Click_ExportBuildingSprite, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "build", "gui/buildunit.png", "", "Export unit/animation sprites from 8 sides",	MAINFONT8, BUST_LEFTIMAGE, Resize_ExportUnitButton, Click_ExportUnitSprites, NULL, NULL, NULL, NULL, -1));
#if 1
	toppanel->add(new Button(toppanel, "build", "gui/buildtile.png", "", "Export tile with inclines",					MAINFONT8, BUST_LEFTIMAGE, Resize_ExportTileButton, Click_ExportTileSprites, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "build", "gui/buildroad.png", "", "Export road tiles with applicable inclines and rotations",	MAINFONT8, BUST_LEFTIMAGE, Resize_ExportRoadButton, Click_ExportRoadSprites, NULL, NULL, NULL, NULL, -1));
#if 0
	toppanel->add(new Button(toppanel, "build", "gui/buildridge.png", "", "Export ridge with inclines from 4 sides",	MAINFONT8, BUST_LEFTIMAGE, Resize_ExportRidgeButton, Click_CompileMap, NULL, NULL));
	toppanel->add(new Button(toppanel, "build", "gui/buildwater.png", "", "Export water tile with inclines",			MAINFONT8, BUST_LEFTIMAGE, Resize_ExportWaterButton, Click_CompileMap, NULL, NULL));
#endif
#endif
#endif
	toppanel->add(new Button(toppanel, "undo", "gui/undo.png", "", "Undo",												MAINFONT8, BUST_LEFTIMAGE, Resize_UndoButton, Undo, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "redo", "gui/redo.png", "", "Redo",												MAINFONT8, BUST_LEFTIMAGE, Resize_RedoButton, Redo, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "newbrush", "gui/newbrush.png", "", "New brush",									MAINFONT8, BUST_LEFTIMAGE, Resize_NewBrushButton, &Click_NewBrush, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "cutbrush", "gui/cutbrush.png", "", "Cut brush",									MAINFONT8, BUST_LEFTIMAGE, Resize_CutBrushButton, Click_CutBrush, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "newent", "gui/newent.png", "", "Add entity",									MAINFONT8, BUST_LEFTIMAGE, Resize_NewEntityButton, Click_NewEnt, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "newent", "gui/newmodel.png", "", "Add milkshape3d model",						MAINFONT8, BUST_LEFTIMAGE, Resize_NewModelButton, Click_AddMS3D, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "selent", "gui/selent.png", "", "Select entities only",							MAINFONT8, BUST_LEFTIMAGE, Resize_SelEntButton, NULL, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "selbrush", "gui/selbrush.png", "", "Select brushes only",						MAINFONT8, BUST_LEFTIMAGE, Resize_SelBrushButton, NULL, NULL, NULL, NULL, NULL, -1, NULL));
	//toppanel->add(new Button(toppanel, "selentbrush", "gui/selentbrush.png", "", "Select entities and brushes",			MAINFONT8, BUST_LEFTIMAGE, Resize_SelEntBrushButton, NULL, NULL, NULL));
	toppanel->add(new Button(toppanel, "rotxccw", "gui/rotxccw.png", "", "Rotate counter-clockwise on x axis",			MAINFONT8, BUST_LEFTIMAGE, Resize_RotXCCWButton, Click_RotXCCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotxcw", "gui/rotxcw.png", "", "Rotate clockwise on x axis",					MAINFONT8, BUST_LEFTIMAGE, Resize_RotXCWButton, Click_RotXCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotyccw", "gui/rotyccw.png", "", "Rotate counter-clockwise on y axis",			MAINFONT8, BUST_LEFTIMAGE, Resize_RotYCCWButton, Click_RotYCCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotycw", "gui/rotycw.png", "", "Rotate clockwise on y axis",					MAINFONT8, BUST_LEFTIMAGE, Resize_RotYCWButton, Click_RotYCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotzccw", "gui/rotzccw.png", "", "Rotate counter-clockwise on z axis",			MAINFONT8, BUST_LEFTIMAGE, Resize_RotZCCWButton, Click_RotZCCW, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "rotzcw", "gui/rotzcw.png", "", "Rotate clockwise on z axis",					MAINFONT8, BUST_LEFTIMAGE, Resize_RotZCWButton, Click_RotZCW, NULL, NULL, NULL, NULL, -1, NULL));

//EditBox::EditBox(Widget* parent, const char* n, const std::string t, int f, void (*reframef)(Widget* thisw), bool pw, int maxl, void (*change2)(int p), int parm) : Widget()

	toppanel->add(new EditBox(toppanel, "rotdeg", RichText("15"),															MAINFONT8, Resize_RotDegEditBox, false, 6, &Change_RotDeg, NULL, 0));
	toppanel->add(new Text(toppanel, "rotdegtext", "degrees",													MAINFONT8, Resize_RotDegText));
	toppanel->add(new EditBox(toppanel, "zoom", RichText("1"),															MAINFONT8, Resize_ZoomEditBox, false, 6, &Change_Zoom, NULL, 0));
	toppanel->add(new Text(toppanel, "zoomtext", "zoom",														MAINFONT8, Resize_ZoomText));
	toppanel->add(new DropList(toppanel, "snapgrid",															MAINFONT8, Resize_SnapGridEditBox, Change_SnapGrid));
	DropList* snapgs = (DropList*)toppanel->get("snapgrid");
	//snapgs->m_options.push_back("4 m");	//0
	//snapgs->m_options.push_back("2 m");	//1
	//snapgs->m_options.push_back("1 m");	//2
	//snapgs->m_options.push_back("50 cm");	//3
	//snapgs->m_options.push_back("25 cm");	//4
	//snapgs->m_options.push_back("12.5 cm");	//5
	//snapgs->m_options.push_back("6.25 cm");	//6
	//snapgs->m_options.push_back("3.125 cm");	//7
	//snapgs->m_options.push_back("1.5625 cm");	//8
	//snapgs->select(4);

	float cm_scales[] = CM_SCALES;
	std::string cm_scales_txt[] = CM_SCALES_TXT;

	for(int j=0; j<sizeof(cm_scales)/sizeof(float); j++)
	{
		snapgs->m_options.push_back(cm_scales_txt[j].c_str());
	}

	//snapgs->select(6);
	snapgs->m_selected = 6;

	//toppanel->add(new EditBox(toppanel, "snapgrid", "25",														MAINFONT8, Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16), false, 6, &Change_SnapGrid, 0));
	toppanel->add(new Text(toppanel, "snapgrid text", "snap grid",													MAINFONT8, Resize_SnapGridText));
	toppanel->add(new EditBox(toppanel, "maxelev", RichText("10000"),														MAINFONT8, Resize_MaxElevEditBox, false, 6, &Change_MaxElev, NULL, 0));
	toppanel->add(new Text(toppanel, "maxelev text", "max elev.",													MAINFONT8, Resize_MaxElevText));
	toppanel->add(new CheckBox(toppanel, "showsky", "show sky",													MAINFONT8, Resize_ShowSkyCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_ShowSky));

	toppanel->add(new Button(toppanel, "persp", "gui/projpersp.png", "", "Perspective projection",				MAINFONT8, BUST_LEFTIMAGE, Resize_PerspProjButton, Click_ProjPersp, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "ortho", "gui/projortho.png", "", "Orthographic projection",				MAINFONT8, BUST_LEFTIMAGE, Resize_OrthoProjButton, Click_ProjOrtho, NULL, NULL, NULL, NULL, -1, NULL));
	toppanel->add(new Button(toppanel, "resetview", "gui/resetview.png", "", "Reset view",						MAINFONT8, BUST_LEFTIMAGE, Resize_ResetViewButton, Click_ResetView, NULL, NULL, NULL, NULL, -1, NULL));

	toppanel->add(new Text(toppanel, "frames text", "frames",														MAINFONT8, Resize_FramesText));
	toppanel->add(new EditBox(toppanel, "frames", RichText("1"),															MAINFONT8, Resize_FramesEditBox, false, 6, &Change_Frames, NULL, 0));

#if 1
	toppanel->add(new Button(toppanel, "explosion", "gui/explosion.png", "", "Explode crater",					MAINFONT8, BUST_LEFTIMAGE, Resize_ExplodeButton, Click_Explode, NULL, NULL, NULL, NULL, -1, NULL));
#endif

#if 0
	toppanel->add(new Button(toppanel, "addtile", "gui/addtile.png", "", "Add tile texture",					MAINFONT8, BUST_LEFTIMAGE, Resize_AddTileButton, Click_AddTile, NULL, NULL, NULL, NULL, -1));
	
	toppanel->add(new CheckBox(toppanel, "NE", "Leads NE",													MAINFONT8, Resize_LeadsNECheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
	toppanel->add(new CheckBox(toppanel, "SE", "Leads SE",													MAINFONT8, Resize_LeadsSECheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
	toppanel->add(new CheckBox(toppanel, "SW", "Leads SW",													MAINFONT8, Resize_LeadsSWCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
	toppanel->add(new CheckBox(toppanel, "NW", "Leads NW",													MAINFONT8, Resize_LeadsNWCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_Leads));
#endif


	//toppanel->add(new Text(toppanel, "fps", "fps: 0", MAINFONT8, Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 10), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 70), true));

	edview->add(new Frame(edview, "viewports frame", Resize_ViewportsFrame));
	Widget* viewportsframe = edview->get("viewports frame");

#if 0
Viewport::Viewport(Widget* parent, const char* n, void (*reframef)(Widget* thisw),
					 void (*drawf)(int p, int x, int y, int w, int h),
					 bool (*ldownf)(int p, int x, int y, int w, int h),
					 bool (*lupf)(int p, int x, int y, int w, int h),
					 bool (*mousemovef)(int p, int x, int y, int w, int h),
					 bool (*rdownf)(int p, int relx, int rely, int w, int h),
					 bool (*rupf)(int p, int relx, int rely, int w, int h),
					 bool (*mousewf)(int p, int d),
					 int parm)
#endif

	viewportsframe->add(new Viewport(viewportsframe, "bottom left viewport",	Resize_BottomLeftViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 0));
	viewportsframe->add(new Viewport(viewportsframe, "top left viewport",		Resize_TopLeftViewport,		&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 1));
	viewportsframe->add(new Viewport(viewportsframe, "bottom right viewport",	Resize_BottomRightViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 2));
	viewportsframe->add(new Viewport(viewportsframe, "top right viewport",		Resize_TopRightViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 3));

	g_edpanetype[VIEWPORT_FRONT] = EdPaneType(Vec3f(0, 0, MAX_DISTANCE/3), Vec3f(0, 1, 0), "Front");
	g_edpanetype[VIEWPORT_TOP] = EdPaneType(Vec3f(0, MAX_DISTANCE/3, 0), Vec3f(0, 0, -1), "Top");
	g_edpanetype[VIEWPORT_LEFT] = EdPaneType(Vec3f(MAX_DISTANCE/3, 0, 0), Vec3f(0, 1, 0), "Left");
	//g_edpanetype[VIEWPORT_ANGLE45O] = EdPaneType(Vec3f(MAX_DISTANCE/3, MAX_DISTANCE/3, MAX_DISTANCE/3), Vec3f(0, 1, 0), "Angle");
	g_edpanetype[VIEWPORT_ANGLE45O] = EdPaneType(Vec3f(1000.0f/3, 1000.0f/3, 1000.0f/3), Vec3f(0, 1, 0), "");
	//g_cam.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);
	ResetView();

	g_edpane[0] = EdPane(VIEWPORT_FRONT);
	g_edpane[1] = EdPane(VIEWPORT_TOP);
	g_edpane[2] = EdPane(VIEWPORT_LEFT);
	g_edpane[3] = EdPane(VIEWPORT_ANGLE45O);

	viewportsframe->add(new Image(viewportsframe, "h divider", "gui/filled.jpg", true, Resize_HDivider));
	viewportsframe->add(new Image(viewportsframe, "v divider", "gui/filled.jpg", true, Resize_VDivider));

	g_gui.add(new ViewLayer(&g_gui, "brush edit"));
	ViewLayer* brusheditview = (ViewLayer*)g_gui.get("brush edit");
	brusheditview->add(new Frame(NULL, "left panel", Resize_BrushEditFrame));

	leftpanel = brusheditview->get("left panel");
	//leftpanel->add(new EditBox(leftpanel, "texture path", "no texture", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 0), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), false, 64, NULL, -1));
	//i++;
	leftpanel->add(new Button(leftpanel, "choose texture", "gui/transp.png", "Choose Texture", "Choose texture", MAINFONT8, BUST_LEFTIMAGE, Resize_ChooseTexButton, Click_BChooseTex, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new Button(leftpanel, "fit to face", "gui/fittoface.png", "", "Fit to face",	MAINFONT8, BUST_LEFTIMAGE, Resize_FitToFaceButton, Click_FitToFace, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new Button(leftpanel, "door view", "gui/door.png", "", "Door view",	MAINFONT8, BUST_LEFTIMAGE, Resize_DoorView, Click_DoorView, NULL, NULL, NULL, NULL, -1, NULL));

	g_gui.add(new ViewLayer(&g_gui, "brush side edit"));
	ViewLayer* brushsideeditview = (ViewLayer*)g_gui.get("brush side edit");
	brushsideeditview->add(new Frame(brushsideeditview, "left panel", Resize_BrushSideEditFrame));
	
	leftpanel = brushsideeditview->get("left panel");
	leftpanel->add(new Button(leftpanel, "rotate texture", "gui/transp.png", "Rotate Texture", "Rotate texture", MAINFONT8, BUST_LEFTIMAGE, Resize_RotateTexButton, Click_RotateTex, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new EditBox(leftpanel, "u equation", RichText("A B C D"), MAINFONT8, Resize_TexEqEditBox1, false, 256, Change_TexEq, NULL, 0));
	leftpanel->add(new EditBox(leftpanel, "v equation", RichText("A B C D"), MAINFONT8, Resize_TexEqEditBox2, false, 256, Change_TexEq, NULL, 1));
	leftpanel->add(new DropList(leftpanel, "select component", MAINFONT8, Resize_SelComponentDropList, Change_SelComp));
	Widget* selcompwidg = leftpanel->get("select component");
	selcompwidg->m_options.push_back("u component");
	selcompwidg->m_options.push_back("v component");
	leftpanel->add(new EditBox(leftpanel, "texture scale", RichText("1"), MAINFONT8, Resize_TexScaleEditBox, false, 10, NULL, NULL, 0));
	leftpanel->add(new Button(leftpanel, "texture scale button", "gui/transp.png", RichText("Scale"), RichText("Scale texture component"), MAINFONT8, BUST_LEFTIMAGE, Resize_ScaleTexButton, Click_ScaleTex, NULL, NULL, NULL, NULL, -1, NULL));
	leftpanel->add(new EditBox(leftpanel, "texture shift", RichText("0.05"), MAINFONT8, Resize_TexShiftEditBox, false, 10, NULL, NULL, 0));
	leftpanel->add(new Button(leftpanel, "texture shift button", "gui/transp.png", RichText("Shift"), RichText("Shift texture component"), MAINFONT8, BUST_LEFTIMAGE, Resize_TexShiftButton, Click_ShiftTex, NULL, NULL, NULL, NULL, -1, NULL));

	g_gui.add(new ViewLayer(&g_gui, "choose file"));
	ViewLayer* choosefileview = (ViewLayer*)g_gui.get("choose file");
	//choosefileview->add(new ChooseFile(choosefileview, "choose file", Resize_ChooseFile, Callback_ChooseFile));

#if 1
	g_gui.add(new ViewLayer(&g_gui, "door edit"));
	ViewLayer* dooreditview = (ViewLayer*)g_gui.get("door edit");
#if 1
	dooreditview->add(new Button(dooreditview, "set door", "gui/yesdoor.png", "", "Make door and set properties", MAINFONT8, BUST_LEFTIMAGE, Resize_SetDoor, Click_SetDoor, NULL, NULL, NULL, NULL, -1, NULL));
	dooreditview->add(new Button(dooreditview, "unmake door", "gui/nodoor.png", "", "Unmake door",	MAINFONT8, BUST_LEFTIMAGE, Resize_UnmakeDoor, Click_UnmakeDoor, NULL, NULL, NULL, NULL, -1, NULL));
	dooreditview->add(new Button(dooreditview, "open/close", "gui/transp.png", "Open / Close", "Open / Close door", MAINFONT8, BUST_LEFTIMAGE, Resize_OpenCloseDoor, Click_OpenCloseDoor, NULL, NULL, NULL, NULL, -1, NULL));
	dooreditview->add(new Text(dooreditview, "opendeg label", "Open degrees:", MAINFONT8, Resize_OpenDoorDegrees));
	dooreditview->add(new EditBox(dooreditview, "opendeg", RichText("90"), MAINFONT8, Resize_OpenDoorDegreesEd, false, 10, NULL, NULL, 0));
#endif
#endif

	g_gui.add(new Win(&g_gui, "place entity", Resize_PlaceEntBG));
	Win* peview = (Win*)g_gui.get("place entity");
	peview->add(new DropList(peview, "what",															MAINFONT8, Resize_PlaceEntWhat, NULL));
	DropList* pewhat = (DropList*)peview->get("what");
	for(int eti=0; eti<ETYPES; eti++)
	{
		EType* et = &g_etype[eti];
		pewhat->m_options.push_back(et->name.c_str());
	}
	peview->add(new Button(peview, "ok", "gui/transp.png", "Place Ent", "OK", MAINFONT8, BUST_LINEBASED, Resize_PlaceEntOK, Click_PlaceEntOK, NULL, NULL, NULL, NULL, -1, NULL));
	peview->add(new Button(peview, "delete", "gui/transp.png", "Delete", "Delete", MAINFONT8, BUST_LINEBASED, Resize_PlaceEntCancel, Click_PlaceEntDelete, NULL, NULL, NULL, NULL, -1, NULL));

	g_gui.add(new ViewLayer(&g_gui, "render"));
	ViewLayer* renderview = (ViewLayer*)g_gui.get("render");

	renderview->add(new Viewport(NULL, "render viewport",	Resize_FullViewport, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3));

	g_gui.hideall();
	g_gui.show("logo");
	//OpenAnotherView("brush edit view");

#define WALLOFTEXT "CONTROLS\n"\
					"========\n"\
					"\n"\
					"GAME\n"\
					"----\n"\
					"\n"\
					"WASD    scroll horizontally\n"\
					"RF    scroll vertically\n"\
					"middle mouse button+drag    rotate angle view\n"\
					"left mouse button+drag    drag-select units\n"\
					"\n"\
					"\n"\
					"WORLD APPMODE_EDITOR\n"\
					"------------\n"\
					"\n"\
					"WASD    scroll horizontally\n"\
					"RF    scroll vertically\n"\
					"right mouse button+drag    rotate angle view\n"\
					"CTRL+mouse drag    move camera\n"\
					"mouse wheel    zoom in/out\n"\
					"CTRL+C    copy selected brush\n"\
					"CTRL+V    paste copied brush\n"\
					"DEL    delete selected brush\n"
	
	g_gui.add(new ViewLayer(&g_gui, "menu"));
	ViewLayer* menuview = (ViewLayer*)g_gui.get("menu");
	//playview->add(new Text(playview, "text", "Play", MAINFONT16, Resize_PlayText));
	menuview->add(new Image(menuview, "loadbg", "gui/mmbg.jpg", true, Resize_Fullscreen));
	
	//menuview->add(new Viewport(menuview, "full viewport",		Resize_FullViewport,	&DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3));
	menuview->add(new Image(menuview, "-2", "billboards/bloodsplat.png", true, Resize_Splat));
	menuview->add(new Text(menuview, "-1", TITLE, MAINFONT64, Resize_MenuItem, true, 0.8f, 0.6f, 0.3f, 1.0f));
	//menuview->add(new TextBlock(menuview, "-3", WALLOFTEXT, MAINFONT8, Resize_MenuItem, 0, 1, 0, 0.8f));
	menuview->add(new Link(menuview, "0", STRTABLE[STR_NEWGAME], MAINFONT32, Resize_MenuItem, Click_NewGame));
	//menuview->add(new Link(menuview, "1", "Load Saved", MAINFONT32, Resize_MenuItem, Click_LoadSaved));
	menuview->add(new Link(menuview, "1", STRTABLE[STR_LOADGAME], MAINFONT32, Resize_MenuItem, Click_LoadGame));
	menuview->add(new Link(menuview, "2", STRTABLE[STR_HOSTGAME], MAINFONT32, Resize_MenuItem, Click_HostGame));
	menuview->add(new Link(menuview, "3", STRTABLE[STR_JOINGAME], MAINFONT32, Resize_MenuItem, Click_ListHosts));
	menuview->add(new Link(menuview, "4", STRTABLE[STR_MAPED], MAINFONT32, Resize_MenuItem, Click_MapEditor));
	menuview->add(new Link(menuview, "5", STRTABLE[STR_OPTIONS], MAINFONT32, Resize_MenuItem, Click_Options));
	menuview->add(new Link(menuview, "6", STRTABLE[STR_QUIT], MAINFONT32, Resize_MenuItem, Click_Quit));

	char infostr[512];
	char verstr[32];
	VerStr(VERSION, verstr);
	sprintf(infostr, "%s (c) DMD 'Ware 2015\n%s %s", 
		STRTABLE[STR_COPYRIGHT].rawstr().c_str(), 
		STRTABLE[STR_VERSION].rawstr().c_str(), 
		verstr);
	menuview->add(new TextBlock(menuview, "copy", RichText(infostr), MAINFONT16, Resize_CopyInfo, 0.2f, 1.0f, 0.2f, 1.0f));

	g_gui.add(new ViewLayer(&g_gui, "play"));
	ViewLayer* playview = (ViewLayer*)g_gui.get("play");
	//playview->add(new Text(playview, "text", "Play", MAINFONT16, Resize_PlayText));
	//playview->add(new Viewport(playview, "full viewport",		Resize_FullViewport,	&DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3));

	FillMess();
}

int GetNumFrames()
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* frameseditbox = toppanel->get("frames");

	int nframes = StrToInt(frameseditbox->m_value.rawstr().c_str());

	return nframes;
}

void SetNumFrames(int nframes)
{
	ViewLayer* edview = (ViewLayer*)g_gui.get("editor");

	if(!edview)  ErrMess("Error", "edview not found");

	Widget* toppanel = edview->get("top panel");

	if(!toppanel)  ErrMess("Error", "toppanel not found");

	EditBox* frameseditbox = (EditBox*)toppanel->get("frames");

	if(!frameseditbox)  ErrMess("Error", "frameseditbox not found");

	char nframesstr[128];
	sprintf(nframesstr, "%d", nframes);
	RichText nframesrstr = RichText(nframesstr);

	frameseditbox->changevalue(&nframesrstr);
}
