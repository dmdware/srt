

#include "undo.h"
#include "../../bsp/sortb.h"
#include "../../math/camera.h"
#include "../../utils.h"
#include "../../save/edmap.h"
#include "sesim.h"

std::list<UndoH> g_undoh;
int g_currundo = -1;	//the index which we will revert to when pressing undo next time
bool g_savedlatest = false;
//bool g_doubleredo = false;
//bool g_doubleundo = false;
//bool g_oncurrh = false;

//#define UNDO_DEBUG

UndoH::~UndoH()
{
#if 0
	g_applog<<"~UndoH()"<<std::endl;
#endif
}

//write undo history
void WriteH(UndoH* towrite)
{
	//Important. For some reason there's a memory leak here
	//if we don't call clear(). Bug in std::list<>?
	towrite->brushes.clear();
	towrite->brushes = g_edmap.m_brush;
	towrite->modelholders.clear();
	towrite->modelholders = g_modelholder;

	for(int ei=0; ei<ENTITIES; ei++)
		towrite->entities[ei] = g_entity[ei];

#ifdef UNDO_DEBUG
	g_applog<<"save h "<<towrite->brushes.size()<<" brushes"<<std::endl;
	g_applog.flush();
#endif
}

void LinkPrevUndo(UndoH* tosave)
{
	UndoH h;

	if(tosave != NULL)
		h = *tosave;
	else
	{
		WriteH(&h);
	}

#ifdef UNDO_DEBUG
	g_applog<<"linkpr gcurrun="<<g_currundo<<std::endl;
	g_applog.flush();
#endif

	g_savedlatest = false;
	//g_doubleredo = false;
	//g_doubleundo = true;

	int j=0;
	auto i=g_undoh.begin();
	while(i!=g_undoh.end())
	{
#ifdef UNDO_DEBUG
		g_applog<<"erase? "<<j<<std::endl;
		g_applog.flush();
#endif

		if(j > g_currundo)
		{
#ifdef UNDO_DEBUG
			g_applog<<"erase. "<<j<<std::endl;
			g_applog.flush();
#endif
			i = g_undoh.erase(i);
			j++;
			continue;
		}

		i++;
		j++;
	}

	g_currundo++;
	//if(g_currundo >= MAX_UNDO)
	//	g_currundo = MAX_UNDO-1;
	if(g_currundo > MAX_UNDO)
		g_currundo = MAX_UNDO;

#ifdef UNDO_DEBUG
	g_applog<<"linkpr gcurrun="<<g_currundo<<std::endl;
	g_applog.flush();
#endif

	g_undoh.push_back(h);

#ifdef UNDO_DEBUG
	g_applog<<"linkpr undoh.size="<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif

	int overl = (int)g_undoh.size() - MAX_UNDO;
	if(overl > 0)
	{
		j=0;
		i=g_undoh.begin();
		while(i!=g_undoh.end() && overl > j)
		{
			i = g_undoh.erase(i);
			j++;
		}
	}

#ifdef UNDO_DEBUG
	g_applog<<"linkpr undoh.size2="<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif
}

void LinkLatestUndo()
{
#ifdef UNDO_DEBUG
	g_applog<<"linklate "<<g_currundo<<" == "<<((int)g_undoh.size()-1)<<std::endl;
	g_applog.flush();
#endif

	if(g_currundo >= (int)g_undoh.size()-1 && !g_savedlatest)	//only save if we're at the latest undo
	{
		LinkPrevUndo();
		g_currundo--;
		g_savedlatest = true;
		//g_doubleredo = true;
		//g_doubleundo = false;
	}
}

void Undo()
{
	//g_doubleredo = true;

#ifdef UNDO_DEBUG
	g_applog<<"undo? g_curu="<<g_currundo<<std::endl;
	g_applog.flush();
#endif

	if(g_currundo <= -1)
	{
		//g_doubleredo = false;
		return;
	}

#ifdef UNDO_DEBUG
	g_applog<<"undo from1 "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif

	LinkLatestUndo();
	/*
	if(g_doubleundo)
	{
		//g_doubleundo = false;
		g_currundo --;

		if(g_currundo <= -1)
		{
			//g_doubleredo = false;
			return;
		}
	}*/

#ifdef UNDO_DEBUG
	g_applog<<"undoh.soze="<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif

	int j=0;
	for(auto i=g_undoh.begin(); i!=g_undoh.end(); i++, j++)
	{
#ifdef UNDO_DEBUG
		g_applog<<"undoh #"<<j<<std::endl;
		g_applog.flush();
#endif

		if(j == g_currundo)
		{
			UndoH* h = &*i;
			g_edmap.m_brush = h->brushes;
			g_modelholder = h->modelholders;

			for(int ei=0; ei<ENTITIES; ei++)
				g_entity[ei] = h->entities[ei];

#ifdef UNDO_DEBUG
			g_applog<<"undid now "<<g_edmap.m_brush.size()<<" brushes"<<std::endl;
			g_applog.flush();
#endif

			break;
		}
	}

#ifdef UNDO_DEBUG
	g_applog<<"undo from2 "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif

	g_currundo--;
	if(g_currundo < 0)
	{
		g_currundo = 0;
		//return;
	}

	//if(g_currundo <= 0)
	//	g_doubleredo = false;

	g_sel1b = NULL;
	g_selB.clear();
	g_selE = NULL;
	g_dragW = -1;
	g_dragS = -1;
	g_dragV = -1;
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);

#ifdef UNDO_DEBUG
	g_applog<<"undo to "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif

	//g_oncurrh = true;
	//g_doubleredo = true;
	//g_doubleundo = false;
}

void Redo()
{
	//g_doubleundo = true;
	g_currundo++;	//moves to current state
	/*
	if(g_doubleredo)
	{
		g_currundo++;	//moves to next state
	}*/

	if(g_currundo >= g_undoh.size())
	{
		g_currundo = g_undoh.size()-1;
	//if(g_currundo > g_undoh.size())
	//{
	//	g_currundo = g_undoh.size();
	//if(g_currundo > g_undoh.size()-2)
	//{
	//	g_currundo = g_undoh.size()-2;
		return;
	}

	int j=0;
	for(auto i=g_undoh.begin(); i!=g_undoh.end(); i++, j++)
	{
		if(j == g_currundo)
		{
			UndoH* h = &*i;
			g_edmap.m_brush = h->brushes;
			g_modelholder = h->modelholders;

			for(int ei=0; ei<ENTITIES; ei++)
				g_entity[ei] = h->entities[ei];

#ifdef UNDO_DEBUG
			g_applog<<"redid to "<<g_edmap.m_brush.size()<<" brushes"<<std::endl;
			g_applog.flush();
#endif
			break;
		}
	}
	/*
	if(g_doubleredo)
	{
		//g_doubleredo = false;
		g_currundo--;	//move back to new current state
	}*/

	g_sel1b = NULL;
	g_selB.clear();
	g_selE = NULL;
	g_dragW = -1;
	g_dragS = -1;
	g_dragV = -1;
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);

#ifdef UNDO_DEBUG
	g_applog<<"redo to "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	g_applog.flush();
#endif

	if(g_currundo > g_undoh.size()-2)
	{
		g_currundo = g_undoh.size()-2;
	}

	//g_doubleredo = false;
	//g_doubleundo = true;
}

void ClearUndo()
{
#ifdef UNDO_DEBUG
	g_applog<<"clear undo"<<std::endl;
	g_applog.flush();
#endif

	g_currundo = -1;
	g_undoh.clear();
	g_savedlatest = false;
	//g_doubleundo = true;
}
