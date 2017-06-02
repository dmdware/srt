

#ifndef EDMAP_H
#define EDMAP_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../bsp/brush.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../bsp/door.h"
#include "modelholder.h"
#include "../sim/entity.h"

class TraceWork;
class TraceJob;

class EdMap
{
public:
	std::list<Brush> m_brush;
	Vec3f m_min;
	Vec3f m_max;

	void checkbrush(TraceWork* tw, TraceJob* tj, Brush* b);
	void trace(TraceWork* tw, TraceJob* tj);
	void tracebox(TraceWork *tw, Vec3f start, Vec3f end, Vec3f vmin, Vec3f vmax, float maxstep);
	void traceray(TraceWork *tw, Vec3f start, Vec3f end);
	void trytostep(TraceWork* tw, TraceJob* tj);
};

extern EdMap g_edmap;
extern std::vector<Brush*> g_selB;
extern Ent* g_selE;
extern Brush* g_sel1b;	//drag selected brush
extern int g_dragV;	//drag vertex of selected brush
extern int g_dragS;	//drag side of selected brush
extern bool g_dragW;
extern int g_dragD;
extern int g_dragM;	//drag model holder
extern std::vector<ModelHolder*> g_selM;
extern bool g_dragE_r;	//dragging entity rotation box?
extern bool g_dragE_p;	//dragging entity position box?
extern ModelHolder* g_sel1m;	//drag selected model (model being dragged or manipulated currently)

#define DRAG_DOOR_POINT		1
#define DRAG_DOOR_AXIS		2

void DrawEdMap(EdMap* map, bool showsky);
void DrawEdMapDepth(EdMap* map, bool showsky);

#endif
