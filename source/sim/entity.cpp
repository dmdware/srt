


#include "entity.h"
#include "../math/frustum.h"
#include "../sim/player.h"
//#include "../phys/q3bsp.h"
#include "../render/billboard.h"
#include "../render/model.h"
//#include "../render/skybox.h"
//#include "item.h"
//#include "animations.h"
//#include "physics.h"
//#include "gui.h"
//#include "script.h"
#include "../render/vertexarray.h"
#include "../save/edmap.h"

EType g_etype[ETYPES];
Ent g_entity[ENTITIES];
std::vector<Spawn> g_spawn;
std::vector<Spawn> g_zspawn;

Vec3f Ent::traceray(Vec3f vLine[])
{
	Vec3f vTrace[2];
	vTrace[0] = vLine[0];
	vTrace[1] = vLine[1];

	EType* t = &g_etype[type];
	Model* m = &g_model[t->model[BODY_LOWER]];

	if(t->collider >= 0)
		m = &g_model[t->collider];

	VertexArray va;
	m->m_ms3d.genva(&va, frame[BODY_LOWER], Vec3f(1,1,1), Vec3f(0,0,0), false);
	Vec3f vTri[3];
	int i, j;

	for(i=0; i<va.numverts; i+=3)
	{
		for(j=0; j<3; j++)
			vTri[j] = camera.m_pos + Rotate(va.vertices[i+j], camera.m_orientv.y, 0, 1, 0);
		
		InterPoly(vTri, vTrace, 3, &vTrace[1]);
	}

#if 0	//if aimer
	if(t->model[BODY_UPPER] < 0 || t->collider >= 0)
		return vTrace[1];

	m = &g_model[t->model[BODY_UPPER]];
	va.free();
	m->m_ms3d.genva(&va, frame[BODY_UPPER], Vec3f(1,1,1), Vec3f(0,0,0), false);
	
	for(i=0; i<va->numverts; i+=3)
	{
		for(j=0; j<3; j++)
		{
			vTri[j] = RotateAround(va.vertices[i+j], Vec3f(0, MID_HEIGHT_OFFSET, 0), -camera.m_orientv.x, 1, 0, 0);
			vTri[j] = camera.m_pos + Rotate(vTri[j], camera.m_orientv.y, 0, 1, 0);
		}
		
		InterPoly(vTri, vTrace, 3, &vTrace[1]);
	}
#endif

	return vTrace[1];
}


#if 0

//bool TriBoxOverlap(Vec3f vPos, Vec3f vMin, Vec3f vMax, Vec3f* vTri);
bool Ent::Collision(Vec3f vScaleDown, Vec3f vCenter, Vec3f vFirstCenter)
{
	EType* t = &g_entityType[type];
	CModel* m = &g_model[t->model[BODY_LOWER]];
	
	if(t->collider >= 0)
		m = &g_model[t->collider];

	CVertexArray* va = &m->vertexArrays[(int)frame[BODY_LOWER]];
	CTriangle tri;
	int i;

	int nearest = -1;
	float nearestD = 0.0f;
	Vec3f vTriCenter;
	float thisD;
	CVertexArray* nearva;

	for(i=0; i<va->numverts; i+=3)
	{
		tri.a = camera.Position() + Rotate(va->vertices[i+0], camera.Yaw(), 0, 1, 0);
		tri.b = camera.Position() + Rotate(va->vertices[i+2], camera.Yaw(), 0, 1, 0);
		tri.c = camera.Position() + Rotate(va->vertices[i+1], camera.Yaw(), 0, 1, 0);
		
		if(TriBoxOverlap2(vScaleDown, vCenter, tri))
		{
			
			vTriCenter = (tri.a + tri.b + tri.c)/3.0f;
			thisD = Magnitude2( vTriCenter - vFirstCenter );

			if(thisD < nearestD || nearest < 0)
			{
				nearva = va;
				nearestD = thisD;
				nearest = i;
			}
			
			/*
			Vec3f vTri[] = {tri.a, tri.b, tri.c};
			g_vCollisionNormal = Normal2(vTri);

			return true;*/
		}
	}

	if(t->model[BODY_UPPER] < 0 || t->collider >= 0)
	{
		if(nearest >= 0)
		{
			tri.a = camera.Position() + Rotate(va->vertices[nearest+0], camera.Yaw(), 0, 1, 0);
			tri.b = camera.Position() + Rotate(va->vertices[nearest+1], camera.Yaw(), 0, 1, 0);
			tri.c = camera.Position() + Rotate(va->vertices[nearest+2], camera.Yaw(), 0, 1, 0);
		
			Vec3f vTri[] = {tri.a, tri.b, tri.c};
			g_vCollisionNormal = Normal2(vTri);

			return true;
		}

		return false;
	}

	m = &g_model[t->model[BODY_UPPER]];
	va = &m->vertexArrays[(int)frame[BODY_UPPER]];
	
	for(i=0; i<va->numverts; i+=3)
	{
		tri.a = RotateAround(va->vertices[i+0], Vec3f(0, MID_HEIGHT_OFFSET, 0), -camera.Pitch(), 1, 0, 0);
		tri.a = camera.Position() + Rotate(tri.a, camera.Yaw(), 0, 1, 0);
		
		tri.b = RotateAround(va->vertices[i+2], Vec3f(0, MID_HEIGHT_OFFSET, 0), -camera.Pitch(), 1, 0, 0);
		tri.b = camera.Position() + Rotate(tri.b, camera.Yaw(), 0, 1, 0);
		
		tri.c = RotateAround(va->vertices[i+1], Vec3f(0, MID_HEIGHT_OFFSET, 0), -camera.Pitch(), 1, 0, 0);
		tri.c = camera.Position() + Rotate(tri.c, camera.Yaw(), 0, 1, 0);
		
		
		if(TriBoxOverlap2(vScaleDown, vCenter, tri))
		{
			
			vTriCenter = (tri.a + tri.b + tri.c)/3.0f;
			thisD = Magnitude2( vTriCenter - vFirstCenter );

			if(thisD < nearestD || nearest < 0)
			{
				nearva = va;
				nearestD = thisD;
				nearest = i;
			}
			
			/*
			Vec3f vTri[] = {tri.a, tri.b, tri.c};
			g_vCollisionNormal = Normal2(vTri);

			return true;*/
		}
	}

	if(nearest >= 0)
	{
		tri.a = camera.Position() + Rotate(nearva->vertices[nearest+0], camera.Yaw(), 0, 1, 0);
		tri.b = camera.Position() + Rotate(nearva->vertices[nearest+1], camera.Yaw(), 0, 1, 0);
		tri.c = camera.Position() + Rotate(nearva->vertices[nearest+2], camera.Yaw(), 0, 1, 0);
		
		Vec3f vTri[] = {tri.a, tri.b, tri.c};
		g_vCollisionNormal = Normal2(vTri);

		return true;
	}

	return false;
}

void DrawHands()
{
	if(g_viewmode == THIRDPERSON)
		return;

	glClear(GL_DEPTH_BUFFER_BIT);
	
    Ent* e;
    EType* t;
    CCamera* c;
    CModel* m;
    Vec3f bounds[2];
	
	CPlayer* p = &g_player[g_localP];
	CHold* h;
	int item;
	CItemType* iT;
	CModel* iM;
	
	float precolor[] = {1,1,1,1};
        
    if(g_reddening > 0.0f)
    {
		precolor[1] = 1.0f - g_reddening;
		precolor[2] = 1.0f - g_reddening;
	}

	float colorf[] = {1, 1, 1, 1};
	Vec3f colorv;

	int i = p->entity;

	e = &g_entity[i];
    c = &e->camera;
    t = &g_entityType[e->type];

	/*
#ifdef DEBUG_3
	if(g_entity[95].type != 0)
	{
		g_applog<<"g_entity[95].type != 0"<<std::endl;
	}

		g_applog<<"drawing hands... 2"<<std::endl;
		g_applog<<"i = "<<i<<std::endl;
		g_applog<<"e->type = "<<e->type<<std::endl;
		g_applog.flush();
#endif*/

#ifdef DEBUG_2
		strcpy(g_debug2str, g_model[t->model[BODY_UPPER]].name);
		//Chat("drawhands");
		g_debug2v = c->Position();
		g_debug2v2 = t->vCenterOff;
		//g_debug2v = c->Position() + t->vCenterOff;
		g_debug2val = DEBUG2DRAWHANDS;
#endif
		
	colorv = g_map.LightVol((c->Position()) + (t->vCenterOff));
	colorf[0] = colorv.x;
	colorf[1] = precolor[1] * colorv.y;
	colorf[2] = precolor[2] * colorv.z;
	glColor4fv(colorf);
	
	/*
#if defined(DEBUG_3) || defined(DEBUG_1)
		g_applog<<"drawing hands... 3"<<std::endl;
		g_applog<<"t->model[BODY_UPPER] = "<<t->model[BODY_UPPER]<<std::endl;
		g_applog<<"g_model[t->model[BODY_LOWER]].name = "<<g_model[t->model[BODY_LOWER]].name<<std::endl;
		g_applog<<"g_model[t->model[BODY_UPPER]].name = "<<g_model[t->model[BODY_UPPER]].name<<std::endl;
		g_applog<<"e->frame[BODY_UPPER] = "<<e->frame[BODY_UPPER]<<std::endl;
		g_applog.flush();
#endif*/

    m = &g_model[t->model[BODY_UPPER]];
    m->Draw(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), c->Yaw());
    //m->Draw(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), 0);
	
	/*
#ifdef DEBUG_3
		g_applog<<"drawing hands... 4"<<std::endl;
		g_applog.flush();
#endif
		*/

	if(p->equipped < 0)
		return;

	/*
#ifdef DEBUG_3
		g_applog<<"drawing hands... 5"<<std::endl;
		g_applog.flush();
#endif
		*/

	h = &p->items[p->equipped];
	item = h->type;
	iT = &g_itemType[item];
	iM = &g_model[iT->model];
	iM->Draw(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), c->Yaw());
}

void SortEntities()
{
	static int sort = -1;
	sort++;

	Vec3f pos = g_cam->Position();
	int cluster = g_map.FindCluster(pos);

    Ent* e;
	CCamera* c;
    EType* t;
    CModel* m;
	bool foundt;

	vector<int> is;

	for(int i=0; i<ENTITIES; i++)
	{
		e = &g_entity[i];

		if(!e->on)
			continue;

		t = &g_entityType[e->type];

		foundt = false;

		if(t->model[BODY_LOWER] >= 0)
		{
			m = &g_model[t->model[BODY_LOWER]];
			if(m->transp)
				foundt = true;
		}
		
        if(t->model[BODY_UPPER] >= 0)
        {
            m = &g_model[t->model[BODY_UPPER]];
			if(m->transp)
				foundt = true;
		}

		if(!foundt)
			continue;

		c = &e->camera;

		/*
		e->cluster = g_map.FindCluster(c->Position());
		if(!g_map.IsClusterVisible(cluster, e->cluster))
			continue;*/

		e->dist = Magnitude2(pos - c->Position());

		is.push_back(i);
	}
    
	Ent temp;
	int leftoff = 0;
	int last = -1;
	bool backtracking = false;
	Ent* e2 = NULL;
	int earliest = 0;
    
	for(int i=0; i<ENTITIES; i++)
	{
		if(i >= earliest)
		{
			e = &g_entity[i];

			if(!e->on)
				continue;
		
			t = &g_entityType[e->type];

			foundt = false;

			if(t->model[BODY_LOWER] >= 0)
			{
				m = &g_model[t->model[BODY_LOWER]];
				if(m->transp)
					foundt = true;
			}
		
			if(t->model[BODY_UPPER] >= 0)
			{
				m = &g_model[t->model[BODY_UPPER]];
				if(m->transp)
					foundt = true;
			}

			if(!foundt)
			{
				if(backtracking)
					i-=2;

				continue;
			}
		
			c = &e->camera;

			/*
			if(!g_map.IsClusterVisible(cluster, e->cluster))
			{
				if(backtracking)
					i-=2;

				continue;
			}*/
        
			/*
			g_applog<<"?";
			g_applog.flush();
			
			g_applog<<"1 @ i="<<i<<std::endl;
			g_applog.flush();*/

			if(e2 == NULL)
			{
				e2 = e;
				last = i;
				earliest = i;
				continue;
			}

			if(e == e2)
				continue;

			if((!backtracking && e->dist > e2->dist) || (backtracking && e2->dist > e->dist))
			{
				/*
				//g_applog<<"SORT["<<sort<<"] ["<<i<<"].dist("<<e->dist<<") > ["<<last<<"].dist("<<e2->dist<<")"<<std::endl;
				
				for(int j=0; j<is.size(); j++)
				{
					if(is[j] == i)
					{
						g_applog<<"[#"<<sort<<"] g_entity["<<is[j]<<"].dist = "<<g_entity[is[j]].dist<<" <<<<<< i"<<std::endl;
					}
					else if(is[j] == last)
					{
						g_applog<<"[#"<<sort<<"] g_entity["<<is[j]<<"].dist = "<<g_entity[is[j]].dist<<" <<<<<< last"<<std::endl;
					}
					else
					{
						g_applog<<"[#"<<sort<<"] g_entity["<<is[j]<<"].dist = "<<g_entity[is[j]].dist<<std::endl;
					}
				}

				g_applog<<"============================================="<<std::endl;

				g_applog.flush();*/

				if(!backtracking)
				{
					leftoff = i;
					backtracking = true;
				}

				temp = *e;
				(*e) = *e2;
				(*e2) = temp;

				if(e->controller >= 0)
				{
					g_player[e->controller].entity = i;

					if(e->controller == g_localP)
						g_cam = &e->camera;
				}
				
				if(e2->controller >= 0)
				{
					g_player[e2->controller].entity = last;
					
					if(e2->controller == g_localP)
						g_cam = &e2->camera;
				}

				//temp = g_entity[i];
				//g_entity[i] = g_entity[last];
				//g_entity[last] = temp;

				if(last < i)
				{
					i-=3;
				}
				else
				{
					e2 = e;
					last = i;
					i-=2;
				}

				//i = leftoff-1;
			}
			else
			{
				//g_applog<<"else"<<std::endl;
				//g_applog.flush();

				if(backtracking)
				{
					
				//g_applog<<"back to a"<<std::endl;
				//g_applog.flush();

					backtracking = false;
					i = leftoff;
					e2 = &g_entity[i];
					last = i;
				}
				else
				{
				//g_applog<<"else 2"<<std::endl;
				//g_applog.flush();

					e2 = e;
					last = i;
				}
			}
		}
		else
		{
		//g_applog<<"2"<<std::endl;
		//g_applog.flush();

			backtracking = false;
			i = leftoff;
			e2 = &g_entity[i];
			last = i;
		}
	}

	/*
	float lastd = 999999999999999.0f;

	for(int i=0; i<ENTITIES; i++)
	{
		e = &g_entity[i];

		if(!e->on)
			continue;
		
		t = &g_entityType[e->type];

		foundt = false;

		if(t->model[BODY_LOWER] >= 0)
		{
			m = &g_model[t->model[BODY_LOWER]];
			if(m->transp)
				foundt = true;
		}
		
        if(t->model[BODY_UPPER] >= 0)
        {
            m = &g_model[t->model[BODY_UPPER]];
			if(m->transp)
				foundt = true;
		}

		if(!foundt)
			continue;
		
		c = &e->camera;
		cluster2 = g_map.FindCluster(c->Position());
		if(!g_map.IsClusterVisible(cluster, cluster2))
			continue;

		if(e->dist > lastd)
		{
			g_applog<<"SORT["<<sort<<"] out of order ["<<i<<"].dist("<<e->dist<<") > ["<<last<<"].dist("<<g_entity[last].dist<<")"<<std::endl;
			g_applog.flush();
			break;
		}

		lastd = e->dist;
		last = i;
	}*/
}

#endif

void DrawEntities(bool transp)
{
	//int cluster = g_map.FindCluster(g_cam->Position());

    Ent* e;
    EType* t;
    Camera* c;
    Model* m;
    Vec3f bounds[2];

	Shader* shader = &g_shader[g_curS];

	int localE = g_player[g_localP].entity;
	Player* p;
	//Hold* h;
	int item;
	//ItemType* iT;
	Model* iM;
	
	float precolor[] = {1,1,1,1};
        
    if(g_reddening > 0.0f)
    {
		precolor[1] = 1.0f - g_reddening;
		precolor[2] = 1.0f - g_reddening;
	}

	float colorf[] = {1, 1, 1, 1};
	Vec3f colorv;

	bool foundt;	// found transparency?

    for(int i=0; i<ENTITIES; i++)
    {
        e = &g_entity[i];
        if(!e->on)
            continue;
        
        c = &e->camera;
		
		//e->cluster = g_map.FindCluster(c->Position());
		//if(!g_map.IsClusterVisible(cluster, e->cluster) && !g_map.IsClusterVisible(e->cluster, cluster))
		//	continue;

        t = &g_etype[e->type];
		
		/*
        bounds[0] = c->Position() + t->vMin;
        bounds[1] = c->Position() + t->vMax;
        
		if(!g_frustum.BoxInFrustum(bounds[0].x, bounds[0].y, bounds[0].z, bounds[1].x, bounds[1].y, bounds[1].z))
			continue;*/

		if(e->controller >= 0)
			p = &g_player[e->controller];
        
        //if(i == localE && g_viewmode == FIRSTPERSON)
        //    continue;

		foundt = false;

#if 0
		if(t->model[BODY_LOWER] >= 0)
		{
			m = &g_model[t->model[BODY_LOWER]];
			if(m->transp)
				foundt = true;
		}
		
        if(t->model[BODY_UPPER] >= 0)
        {
            m = &g_model[t->model[BODY_UPPER]];
			if(m->transp)
				foundt = true;
		}
#endif
		
		if(foundt != transp)
			continue;

		//g_applog<<"entity["<<i<<"] type="<<t->lmodel<<" @ "<<(c->Position() + t->vCenterOff).x<<","<<(c->Position() + t->vCenterOff).y<<","<<(c->Position() + t->vCenterOff).z<<std::endl;
		//g_applog.flush();
		
		
#ifdef DEBUG_3
		if(g_debug3)
		{
			g_applog<<"get lightvol...";
			g_applog.flush();
		}
#endif
		
		
#ifdef DEBUG_2
		//Chat("drawentities");
		g_debug2val = DEBUG2DRAWENT;
		strcpy(g_debug2str, g_model[t->model[BODY_UPPER]].name);
		//Chat("drawhands");
		g_debug2v = c->Position();
		g_debug2v2 = t->vCenterOff;
#endif

#if 0
		if(e->nolightvol)
			colorv = Vec3f(1, 1, 1);
		else
			colorv = g_map.LightVol(c->Position() + t->vCenterOff);
#else
		colorv = Vec3f(1,1,1);
#endif
		
		if(g_selE == e)
		{
			colorv.x = 0;
			colorv.y = 1.5f;
			colorv.z = 1.5f;
		}

		colorf[0] = colorv.x;
		colorf[1] = precolor[1] * colorv.y;
		colorf[2] = precolor[2] * colorv.z;

		
		
#ifdef DEBUG_3
		if(g_debug3)
		{
			g_applog<<"got it"<<std::endl;
			g_applog.flush();
		}
#endif


		//g_applog<<"color = "<<colorf[0]<<","<<colorf[1]<<","<<colorf[2]<<","<<colorf[3]<<std::endl;
		//g_applog.flush();

		////glColor4fv(colorf);
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colorf);
        
        if(t->model[BODY_LOWER] >= 0)
        {
			//g_applog<<"t->model[BODY_LOWER] >= 0"<<std::endl;
			//g_applog.flush();

            m = &g_model[t->model[BODY_LOWER]];

			//g_applog<<"t->model[BODY_LOWER] = "<<t->model[BODY_LOWER]<<std::endl;
			//g_applog<<"e->frame[BODY_LOWER] = "<<e->frame[BODY_LOWER]<<std::endl;
			//g_applog.flush();

			
#ifdef DEBUG_3
			if(g_debug3)
			{
				g_applog<<"lmodel "<<m->name<<" t->model[BODY_LOWER]="<<t->model[BODY_LOWER]<<std::endl;
				g_applog.flush();
			}
#endif
		
			VertexArray va;
			m->m_ms3d.genva(&va, e->frame[BODY_LOWER], Vec3f(1,1,1), Vec3f(0,0,0), true);
			//m->genvbo();
            m->draw(e->frame[BODY_LOWER], c->m_pos, c->m_orientv.y, &va);
            //m->Draw(e->frame[BODY_LOWER], c->Position(), 0, 0);
			va.free();
        }
#if 0
        if(t->model[BODY_UPPER] >= 0)
        {
			//g_applog<<"t->model[BODY_UPPER] >= 0"<<std::endl;
			//g_applog.flush();

            m = &g_model[t->model[BODY_UPPER]];
            m->Draw2(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), c->Yaw());
            //m->Draw2(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), 0);
        }

		if(e->controller >= 0 && p->equipped >= 0)
		{
			//g_applog<<"e->controller >= 0 && p->equipped >= 0"<<std::endl;
			//g_applog.flush();

			h = &p->items[p->equipped];
			item = h->type;
			iT = &g_itemType[item];
			iM = &g_model[iT->model];
			iM->Draw2(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), c->Yaw());
		}
#endif
    }

	////glColor4fv(precolor);
	glUniform4f(shader->slot[SSLOT_COLOR], 1, 1, 1, 1);
}

#if 0

void UpdateDoor(Ent* e, int i)
{
	if(e->state == STATE_OPENING)
	{
		if(e->frame[BODY_LOWER] >= ANIM_OPENING_E)
		{
			if(e->script > 0)
			{
				DoScriptFunc(e->script);
				e->script = -1;
			}

			return;
		}

		e->frame[BODY_LOWER] += 1;

		if(Collides2(i) >= 0)
			e->frame[BODY_LOWER] -= 1;
	}
	else if(e->state == STATE_CLOSING)
	{
		if(e->frame[BODY_LOWER] <= ANIM_OPENING_S)
			return;

		e->frame[BODY_LOWER] -= 1;

		if(Collides2(i) >= 0)
			e->frame[BODY_LOWER] += 1;
	}
}

void UpdateObjects()
{
	Ent* e;
	EType* t;

	for(int i=0; i<ENTITIES; i++)
	{
		e = &g_entity[i];
		if(!e->on)
			continue;

		t = &g_entityType[e->type];

		if(t->category == ENTITY::DOOR)
			UpdateDoor(e, i);
	}
}

#endif

int NewEntity()
{
	for(int i=0; i<ENTITIES; i++)
		if(!g_entity[i].on)
			return i;

	return -1;
}

bool PlaceEntity(int type, int controller, float amount, float clip, Vec3f pos, float yaw, int* id, bool nolightvol, int script)
{
	int i = NewEntity();
	if(i < 0)
		return false;

	Ent* e = &g_entity[i];

	/*
#ifdef DEBUG_3
	g_applog<<"PlaceEntity #"<<i<<" type="<<type<<" g_model[g_entityType[type].model[BODY_LOWER]].name="<<g_model[g_entityType[type].model[BODY_LOWER]].name<<std::endl;
	if(g_entity[95].type != 0)
	{
		g_applog<<"g_entity[95].type != "<<std::endl;
	}
	g_applog.flush();
#endif*/

	e->on = true;
	e->frame[BODY_LOWER] = 0;
	e->frame[BODY_UPPER] = 0;
	e->type = type;
	e->controller = controller;
	e->amount = amount;
	e->clip = clip;
	e->state = STATE_NONE;
	e->nolightvol = nolightvol;
	e->script = script;

	Camera* c = &e->camera;

	c->position(pos.x, pos.y, pos.z, pos.x+1, pos.y, pos.z, 0, 1, 0);
	c->rotateview(DEGTORAD(yaw), 0, 1, 0);

	EType* t = &g_etype[type];
	//e->cluster = g_map.FindCluster(c->Position() + t->vCenterOff);

	Player* p = NULL;
	if(controller >= 0)
	{
		g_player[controller].entity = i;
		p = &g_player[controller];
	}

	if(id)
		(*id) = i;

	/*
	EType* t = &g_entityType[type];
	Vec3f vMin = t->vMin;

	if(controller >= 0)
		if(p->crouched)
			vMin.y += t->crouch;

	//pos = g_map.TraceBox(pos, pos, vMin, t->vMax, t->maxStep);
	//g_entity[i].camera.MoveTo(pos);

	/*
	if(t->category != ENTITY::FIXEDENT && t->category != ENTITY::DOOR)
		while(g_map.Collided())
		{
			pos.y += t->maxStep;
			pos = g_map.TraceBox(pos, pos, vMin, t->vMax, t->maxStep);
			g_entity[i].camera.MoveTo(pos);
		}
		*/
		
	//if(controller >= 0)
	//	p->goal = pos;

	//return g_map.Collided();

	return true;
}

void FreeEnt(Ent* e)
{
}

void DefEnt(int etype, const char* name, int category, const char* lowermodel, Vec3f scale, float animrate, int collider)
{

	/*
#ifdef DEBUG_3
	g_applog<<"New entity "<<lowermodel<<" (category "<<category<<")"<<std::endl;
	g_applog.flush();
#endif*/

	EType* t = &g_etype[etype];

	t->name = name;

	/*
#ifdef DEBUG_3
	g_applog<<"Loading model "<<mdl<<std::endl;
	g_applog.flush();
#endif*/

	t->model[BODY_LOWER] = LoadModel(lowermodel, scale, Vec3f(0,0,0), true, true);
	t->model[BODY_UPPER] = -1;

	t->category = category;
	//ModelMinMax(t->model[BODY_LOWER], &t->vMin, &t->vMax);
	
	t->vMin = Vec3f(-25, -162, -25);
	t->vMax = Vec3f(25, 20, 25);

	/*
	if(category == FIXEDENT)
	{
		t->vMin = t->vMin * 2.0f;
		t->vMax = t->vMax * 2.0f;
	}*/

	t->maxStep = 15;
	t->speed = 200;
	t->jump = 0;
	t->crouch = 0;
	t->animrate = animrate;
	t->vCenterOff = (t->vMin + t->vMax)/2.0f;
	t->collider = collider;
}


#if 0

bool CheckEntitySound(vector<CSound>* vec, const char* filepath)
{
	for(int i=0; i<(*vec).size(); i++)
		if(!stricmp((*vec)[i].filepath, filepath))
			return true;

	return false;
}

void EntitySound(int category, const char* filepath)
{
	EType* t = &g_entityType[g_lastEnt];
	vector<CSound>* vec;
	
	if(category == ENTITYSND::CLOSESND)
		vec = &t->closeSound;
	else if(category == ENTITYSND::OPENSND)
		vec = &t->openSound;

	char fullpath[64];
	SoundPath(filepath, fullpath);

	if(CheckEntitySound(vec, fullpath))
		return;

	(*vec).push_back(CSound(fullpath));
}

int DefEnt(int category, int item, const char* lowermodel, const char* uppermodel, Vec3f scale, Vec3f translate, Vec3f vMin, Vec3f vMax, float maxStep, float speed, float jump, float crouch, float animrate)
{
	g_entityType.push_back(EType());
	int i = g_entityType.size() - 1;
	EType* t = &(g_entityType[i]);

	char raw[32];
	StripPathExt(lowermodel, raw);
	strcpy(t->lmodel, raw);
    
	if(stricmp(lowermodel, "") == 0)
		t->model[BODY_LOWER] = -1;
	else
	{
		char mdl[128];
		StripPathExt(lowermodel, raw);
		sprintf(mdl, "models\\%s.md2", raw);
		t->model[BODY_LOWER] = LoadModel(mdl, scale, translate);
	}
    
	if(stricmp(uppermodel, "") == 0)
		t->model[BODY_UPPER] = -1;
	else
	{
		char mdl[128];
		StripPathExt(uppermodel, raw);
		sprintf(mdl, "models\\%s.md2", raw);
		t->model[BODY_UPPER] = LoadModel(mdl, scale, translate);
	}

	t->category = category;
	t->item = item;
	t->vMin = vMin;
	t->vMax = vMax;
	t->maxStep = maxStep;
	t->speed = speed;
	t->jump = jump;
	t->crouch = crouch;
	t->animrate = animrate;
	t->vCenterOff = (t->vMin + t->vMax)/2.0f;
	t->collider = -1;
	
	return i;
}

int EntityID(const char* lmodel)
{
	char raw[32];
	StripPathExt(lmodel, raw);

    for(int i=0; i<g_entityType.size(); i++)
        if(stricmp(g_entityType[i].lmodel, raw) == 0)
		{

            return i;
		}

    return -1;
}

void Entities()
{
	Vec3f charMin = Vec3f(-10, -50, -10);
	Vec3f charMax = Vec3f(10, 5, 10);
    float crouch = 14.29f;

	//Ent(int type, const char* lowermodel, const char* uppermodel, Vec3f scale, Vec3f translate, Vec3f vMin, Vec3f vMax, float maxStep, float speed, float jump, float crouch)
    
	Ent(ENTITY::HUMAN, -1, "human2lower", "human2upper", Vec3f(1, 1, 1), Vec3f(0, 0, 0), charMin, charMax, 15, 100, 100, crouch, 3.0f);
	Ent(ENTITY::ZOMBIE, -1, "zombie2lower", "zombie2upper", Vec3f(1, 1, 1), Vec3f(0, 0, 0), charMin, charMax, 15, 50, 100, crouch, 1.5f);
	Ent(ENTITY::ZOMBIE, -1, "zombie3lower", "zombie3upper", Vec3f(1, 1, 1), Vec3f(0, 0, 0), charMin, charMax, 15, 50, 100, crouch, 1.5f);
	
	Ent(ENTITY::ITEM, ITEM::MP5, "mp5", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-10, 0, -10), Vec3f(10, 10, 10), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::MOSSBERG500, "mossberg500", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-10, 0, -10), Vec3f(10, 10, 10), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::M1911, "m1911", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-10, 0, -10), Vec3f(10, 10, 10), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::PRIMARYAMMO, "ammo1", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-4.2f, 0, -4.2f), Vec3f(4.2f, 2.3f, 4.2f), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::SECONDARYAMMO, "ammo2", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-4.2f, 0, -4.2f), Vec3f(4.2f, 2.3f, 4.2f), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::TERTAMMO, "ammo3", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-4.2f, 0, -4.2f), Vec3f(4.2f, 2.3f, 4.2f), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::BBAT, "bbat", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-4.2f, 0, -4.2f), Vec3f(4.2f, 2.3f, 4.2f), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::KNIFE, "knife", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-4.2f, 0, -4.2f), Vec3f(4.2f, 2.3f, 4.2f), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::ITEM, ITEM::WTALKIE, "wtalkie", "", Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(-4.2f, 0, -4.2f), Vec3f(4.2f, 2.3f, 4.2f), 15, 200, 100, crouch, 1.0f);
    
	Ent(ENTITY::NOCAT, -1, "washmchn", "", Vec3f(1, 1, 1), Vec3f(0,0,0), Vec3f(-17.24f, -17.24f, -17.24f), Vec3f(17.24f, 17.24f, 17.24f), 15, 200, 100, crouch, 1.0f);
	Ent(ENTITY::NOCAT, -1, "dryer", "", Vec3f(1, 1, 1), Vec3f(0,0,0), Vec3f(-17.24f, -17.24f, -17.24f), Vec3f(17.24f, 17.24f, 17.24f), 15, 200, 100, crouch, 1.0f);

}

#endif

void Swizzle(Vec3f &v)
{
	float temp = v.z;
	v.z = -v.y;
	v.y = temp;
}

void Swizzle(Vec3f &vmin, Vec3f &vmax)
{
	float temp = vmin.z;
	vmin.z = -vmin.y;
	vmin.y = temp;

	temp = vmax.z;
	vmax.z = -vmax.y;
	vmax.y = temp;

	float tempmin = min(vmin.z, vmax.z);
	float tempmax = max(vmin.z, vmax.z);
	vmin.z = tempmin;
	vmax.z = tempmax;
}

void ReadEntity(std::string classname, std::string origin, std::string angle, std::string model, std::string size, std::string type, std::string sky, std::string count, std::string clip, std::string collider, 
				std::string opensound, std::string closesound, std::string activity, std::string nolightvol, std::string bbmin, std::string bbmax, std::string map, std::string script)
{
	g_applog<<"classname = "<<classname<<", origin = "<<origin<<", angle = "<<angle<<", model = "<<model<<", size = "<<size<<", type = "<<type<<", sky = "<<sky<<", count = "<<count<<", clip = "<<clip
		<<", colllider = "<<collider<<", map = "<<map<<" min="<<bbmin<<", max="<<bbmax<<std::endl;
	
		
	bool nolvol = false;

	if(stricmp(nolightvol.c_str(), "true") == 0)
		nolvol = true;
	
	int scrpt = -1;
	if(stricmp(script.c_str(), "") != 0)
		sscanf(script.c_str(), "%d", &scrpt);

	if(stricmp(classname.c_str(), "info_player_start") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		pos.y += 50.0f - 27.5f + 4.0f;
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		Spawn spawn(pos, ang, -1, -1);
		g_spawn.push_back(spawn);

		g_cam.position(pos.x, pos.y, pos.z, pos.x, pos.y, pos.z + 100, 0, 1, 0);
	}
#if 0
    else if(stricmp(classname.c_str(), "info_player_zombie") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		pos.y += 50.0f - 27.5f + 4.0f;
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		//ang += DEGTORAD(180);
		int act = ACTIVITY::ONSIGHT;

		if(stricmp(activity.c_str(), "none") == 0)
			act = ACTIVITY::NOACT;
		else if(stricmp(activity.c_str(), "onsight") == 0)
			act = ACTIVITY::ONSIGHT;

		Spawn spawn(pos, ang, act, scrpt);
		g_zspawn.push_back(spawn);
	}
	else if(stricmp(classname.c_str(), "func_map") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);

		Vec3f vmin, vmax;
		sscanf(bbmin.c_str(), "%f %f %f", &vmin.x, &vmin.y, &vmin.z);
		sscanf(bbmax.c_str(), "%f %f %f", &vmax.x, &vmax.y, &vmax.z);
		Swizzle(vmin, vmax);

		CFuncMap funcmap(pos, vmin, vmax, map.c_str());
		g_funcmap.push_back(funcmap);
	}
	else if(stricmp(classname.c_str(), "func_proxy") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);

		Vec3f vmin, vmax;
		sscanf(bbmin.c_str(), "%f %f %f", &vmin.x, &vmin.y, &vmin.z);
		sscanf(bbmax.c_str(), "%f %f %f", &vmax.x, &vmax.y, &vmax.z);
		Swizzle(vmin, vmax);

		CFuncProxy funcproxy(pos, vmin, vmax, scrpt);
		g_funcproxy.push_back(funcproxy);
	}
	else if(stricmp(classname.c_str(), "weapon_mp5") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		
		float clp = 30;
		if(stricmp(clip.c_str(), "") != 0)
			sscanf(clip.c_str(), "%f", &clp);

		int eid = EntityID("mp5");
        
		PlaceEntity(eid, -1, 1, clp, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "weapon_mossberg500") == 0 || stricmp(classname.c_str(), "weapon_shotgun") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		
		float clp = 6;
		if(stricmp(clip.c_str(), "") != 0)
			sscanf(clip.c_str(), "%f", &clp);

		int eid = EntityID("mossberg500");
        
		PlaceEntity(eid, -1, 1, clp, pos, ang, NULL, nolvol, scrpt);
	}
	
	else if(stricmp(classname.c_str(), "weapon_m1911") == 0 || stricmp(classname.c_str(), "weapon_grapplinghook") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);

		float clp = 7;
		if(stricmp(clip.c_str(), "") != 0)
			sscanf(clip.c_str(), "%f", &clp);

		int eid = EntityID("m1911");
        
		PlaceEntity(eid, -1, 1, clp, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "weapon_bat") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		float clp = 0;

		int eid = EntityID("bbat");
        
		PlaceEntity(eid, -1, 1, clp, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "weapon_knife") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		float clp = 0;

		int eid = EntityID("knife");
        
		PlaceEntity(eid, -1, 1, clp, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "ammo_primary") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		float amt;
		sscanf(count.c_str(), "%f", &amt);

		int eid = EntityID("ammo1");
        
		//if(eid < 0)
		//	eid = Ent(ENTITY::ITEM, "mp5", 1);
        
		PlaceEntity(eid, -1, amt, -1, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "ammo_secondary") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		float amt;
		sscanf(count.c_str(), "%f", &amt);

		int eid = EntityID("ammo2");
        
		//if(eid < 0)
		//	eid = Ent(ENTITY::ITEM, "mp5", 1);
        
		PlaceEntity(eid, -1, amt, -1, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "ammo_tertiary") == 0 || stricmp(classname.c_str(), "ammo_bullets") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);
		float amt;
		sscanf(count.c_str(), "%f", &amt);

		int eid = EntityID("ammo3");
        
		//if(eid < 0)
		//	eid = Ent(ENTITY::ITEM, "mp5", 1);
        
		PlaceEntity(eid, -1, amt, -1, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "_entity") == 0 || stricmp(classname.c_str(), "item_enviro") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);

		int eid = EntityID(model.c_str());
        
		if(eid < 0)
			eid = Ent(ENTITY::NOCAT, model.c_str(), 1, -1);

		float amt = 1;
        
		PlaceEntity(eid, -1, amt, -1, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "fixed_entity") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);

		int eid = EntityID(model.c_str());
        
		if(eid < 0)
		{
			int coli = -1;

			if(stricmp(collider.c_str(), "") != 0)
			{
				char colmdl[128];
				char rawcol[64];
				StripPathExt(collider.c_str(), rawcol);
				sprintf(colmdl, "models\\%s.md2", rawcol);
				coli = LoadModel(colmdl, Vec3f(1,1,1), Vec3f(0,0,0));
			}

			eid = Ent(ENTITY::FIXEDENT, model.c_str(), 1, coli);
		}
        
		PlaceEntity(eid, -1, -1, -1, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "func_door") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);
		float ang;
		sscanf(angle.c_str(), "%f", &ang);

		int eid = EntityID(model.c_str());
        
		if(eid < 0)
			eid = Ent(ENTITY::DOOR, model.c_str(), 1, -1);

		g_lastEnt = eid;

		if(stricmp(opensound.c_str(), ""))
			EntitySound(ENTITYSND::OPENSND, opensound.c_str());
		
		if(stricmp(closesound.c_str(), ""))
			EntitySound(ENTITYSND::CLOSESND, closesound.c_str());

		PlaceEntity(eid, -1, -1, -1, pos, ang, NULL, nolvol, scrpt);
	}
	else if(stricmp(classname.c_str(), "_billboard") == 0 || stricmp(classname.c_str(), "misc_portal_surface") == 0)
	{
		Vec3f pos;
		sscanf(origin.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
		Swizzle(pos);

		float sizef;
		sscanf(size.c_str(), "%f", &sizef);
        
		PlaceBillboard(type.c_str(), pos, sizef, -1, nolvol);
	}
	else if(stricmp(classname.c_str(), "worldspawn") == 0)
	{
		if(stricmp(sky.c_str(), "") != 0)
			LoadSkyBox(sky.c_str());
	}
#endif
}

void ReadEntities(char* str)
{
	g_spawn.clear();

	std::string classname;
	std::string origin;
	std::string angle;
	std::string model;
	std::string size;
	std::string type;
	std::string sky;
	std::string count;
	std::string clip;
	std::string collider;
	std::string opensound;
	std::string closesound;
	std::string activity;
	std::string nolightvol;
	std::string bbmin;
	std::string bbmax;
	std::string map;
	std::string script;
	std::string var;
	std::string val;

	int len = strlen(str);

	for(int i=0; i<len; i++)
	{
		classname = "";
		angle = "";
		origin = "";
		model = "";
		size = "";
		type = "";
		sky = "";
		count = "";
		clip = "";
		collider = "";
		opensound = "";
		closesound = "";
		activity = "";
		nolightvol = "";
		bbmin = "";
		bbmax = "";
		script = "";
		map = "";

		for(; i<len; i++)
			if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
				break;
		
		for(; i<len; i++)
			if(str[i] == '{')
			{
				i++;
				break;
			}

		for(; i<len; i++)
			if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
				break;

		while(str[i] != '}' && i < len)
		{
			var = "";
			val = "";

			for(; i<len; i++)
				if(str[i] == '"')
				{
					i++;
					break;
				}

			for(; i<len; i++)
			{
				if(str[i] == '"')
				{
					i++;
					break;
				}

				var = var + str[i];
			}

			for(; i<len; i++)
				if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
					break;

			for(; i<len; i++)
				if(str[i] == '"')
				{
					i++;
					break;
				}

			for(; i<len; i++)
			{
				if(str[i] == '"')
				{
					i++;
					break;
				}

				val = val + str[i];
			}

			if(stricmp(var.c_str(), "classname") == 0)
				classname = val;
			else if(stricmp(var.c_str(), "origin") == 0)
				origin = val;
			else if(stricmp(var.c_str(), "angle") == 0)
				angle = val;
			else if(stricmp(var.c_str(), "model") == 0)
				model = val;
			else if(stricmp(var.c_str(), "size") == 0)
				size = val;
			else if(stricmp(var.c_str(), "type") == 0)
				type = val;
			else if(stricmp(var.c_str(), "sky") == 0)
				sky = val;
			else if(stricmp(var.c_str(), "count") == 0)
				count = val;
			else if(stricmp(var.c_str(), "clip") == 0)
				clip = val;
			else if(stricmp(var.c_str(), "collider") == 0)
				collider = val;
			else if(stricmp(var.c_str(), "opensound") == 0)
				opensound = val;
			else if(stricmp(var.c_str(), "closesound") == 0)
				closesound = val;
			else if(stricmp(var.c_str(), "activity") == 0)
				activity = val;
			else if(stricmp(var.c_str(), "nolightvol") == 0)
				nolightvol = val;
			else if(stricmp(var.c_str(), "min") == 0)
				bbmin = val;
			else if(stricmp(var.c_str(), "max") == 0)
				bbmax = val;
			else if(stricmp(var.c_str(), "map") == 0)
				map = val;
			else if(stricmp(var.c_str(), "script") == 0)
				script = val;

			g_applog<<var<<" = "<<val<<std::endl;
			
			for(; i<len; i++)
				if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
					break;
		}

		ReadEntity(classname, origin, angle, model, size, type, sky, count, clip, collider, opensound, closesound, activity, nolightvol, bbmin, bbmax, map, script);
	}
}

#if 0

bool IsHuman(int type)
{
	EType* t = &g_entityType[type];

	if(t->category == ENTITY::HUMAN)
		return true;

    return false;
}

bool IsZombie(int type)
{
	EType* t = &g_entityType[type];

	if(t->category == ENTITY::ZOMBIE)
		return true;
    
    return false;
}

int RandomHuman()
{
	std::vector<int> h;
	EType* t;

	for(int i=0; i<g_entityType.size(); i++)
	{
		t = &g_entityType[i];

		if(t->category != ENTITY::HUMAN)
			continue;

		h.push_back(i);
	}

	return h[ rand()%h.size() ];
}

int RandomZombie()
{
	std::vector<int> z;
	EType* t;

	for(int i=0; i<g_entityType.size(); i++)
	{
		t = &g_entityType[i];

		if(t->category != ENTITY::ZOMBIE)
			continue;

		z.push_back(i);
	}

	return z[ rand()%z.size() ];
}

#endif