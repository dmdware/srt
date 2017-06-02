

#include "sesim.h"
#include "../../render/shader.h"
#include "../../platform.h"
#include "../../utils.h"
#include "../../math/3dmath.h"
#include "../../math/vec4f.h"
#include "seviewport.h"
#include "../../gui/gui.h"
#include "../../sim/simdef.h"
#include "segui.h"
#include "../../bsp/door.h"
#include "../../save/modelholder.h"
#include "../../save/compilemap.h"
#include "../../render/shadow.h"

Brush g_copyB;
ModelHolder g_copyM;
int g_edtool = EDTOOL_NONE;
bool g_leads[LEADS_DIRS];

// draw selected brushes filled bg
void DrawFilled(EdMap* map, std::list<ModelHolder>& modelholder)
{
	//UseS(SHADER_COLOR3D);
	Shader* shader = &g_shader[g_curS];
	glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.3f, 0.9f, 0.5f);

	for(int i=0; i<g_selB.size(); i++)
	{
		Brush* b = g_selB[i];

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* side = &b->m_sides[j];

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->m_drawva.vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif
			
#ifdef PLATFORM_GL14
			//glDrawArrays(GL_TRIANGLES, 0, va->numverts);

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->m_drawva.vertices);
            glVertexPointer(3, GL_FLOAT, 0, side->m_drawva.vertices);
#endif
			
			glDrawArrays(GL_TRIANGLES, 0, side->m_drawva.numverts);
		}
	}

#if 1
	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;
		Model* m = &g_model[pmh->model];
		int maxframes = m->m_ms3d.m_totalFrames;

		DrawVA(&pmh->frames[ g_renderframe % maxframes ], pmh->translation);
	}
#endif
}

// draw brush outlines
void DrawOutlines(EdMap* map, std::list<ModelHolder>& modelholder)
{
	//UseS(SHADER_COLOR3D);
	Shader* shader = &g_shader[g_curS];

	//glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, 0.75f);

	for(int i=0; i<ENTITIES; i++)
	{
		Ent* e = &g_entity[i];

		if(!e->on)
			continue;

		EType* et = &g_etype[e->type];
		Camera* c = &e->camera;

		VertexArray va;

		Model* m = &g_model[et->model[BODY_LOWER]];
		m->m_ms3d.genva(&va, e->frame[BODY_LOWER], Vec3f(1,1,1), Vec3f(0,0,0), true);
		float alpha = 1.0f - Magnitude2(c->m_pos - g_cam.m_view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);
		
		Matrix transmat, rotmat;
		transmat.translation((float*)&c->m_pos);
		float rotrad[3] = {DEGTORAD(c->m_orientv.x), DEGTORAD(c->m_orientv.y), 0};
		rotmat.rotrad(rotrad);

		Matrix modelmat;
		modelmat.reset();
		glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

		Matrix mvpmat;
		mvpmat.set(g_camproj.m_matrix);
		mvpmat.postmult(g_camview);
		mvpmat.postmult(transmat);
		mvpmat.postmult(rotmat);
		glUniformMatrix4fv(shader->slot[SSLOT_MVP], 1, 0, mvpmat.m_matrix);

		glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);
			
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->m_drawva.vertices);
#endif
			
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, va.vertices);
#endif
		//glDrawArrays(GL_LINE_STRIP, 0, side->m_outline.m_edv.size());
		glDrawArrays(GL_LINE_LOOP, 0, va.numverts);
	}

	
#if 1	//ditto as drawselout...
	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvpmat;
	mvpmat.set(g_camproj.m_matrix);
	mvpmat.postmult(g_camview);
	glUniformMatrix4fv(shader->slot[SSLOT_MVP], 1, 0, mvpmat.m_matrix);
#endif

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			//glDrawArrays(GL_TRIANGLES, 0, va->numverts);

			float alpha = 1.0f - Magnitude2(side->m_centroid - g_cam.m_view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);

			glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);
			
#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->m_drawva.vertices);
#endif
			
#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, side->m_drawva.vertices);
#endif
			
			//glDrawArrays(GL_LINE_STRIP, 0, side->m_outline.m_edv.size());
			glDrawArrays(GL_LINE_LOOP, 0, side->m_drawva.numverts);
		}
	}

#if 1
	for(auto mhiter = modelholder.begin(); mhiter != modelholder.end(); mhiter++)
	{
		ModelHolder* pmh = &*mhiter;

		float topverts[] =
		{
			//top
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//top far right
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//top near right
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//top near left
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
		};

		float bottomverts[] =
		{
			//bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//bottom far right
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//bottom near right
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//bottom near left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
		};

		float leftverts[] =
		{
			//left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//left top far
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//left top near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//left bottom near
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
		};

		float rightverts[] =
		{
			//right
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//right top far
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//right top near
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//right bottom near
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
		};

		float nearverts[] =
		{
			//near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//near left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//near right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//near right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
		};

		float farverts[] =
		{
			//far
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//far left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//far right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//far right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
		};

		float alpha = 1.0f - Magnitude2((pmh->absmin+pmh->absmax)/2.0f - g_cam.m_view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);

		glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, topverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, farverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
#endif
}

// draw selected brush outlines
void DrawSelOutlines(EdMap* map, std::list<ModelHolder>& modelholder)
{
	//UseS(SHADER_COLOR3D);
	Shader* shader = &g_shader[g_curS];
	glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, 0.75f);

#if 1	//not necessary? breaks ortho projection because of postmult type
	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvpmat;
	mvpmat.set(g_camproj.m_matrix);
	mvpmat.postmult(g_camview);
    glUniformMatrix4fv(shader->slot[SSLOT_MVP], 1, 0, mvpmat.m_matrix);
#endif

	for(auto bi=g_selB.begin(); bi!=g_selB.end(); bi++)
	{
		Brush* b = *bi;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];

			//glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			//glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			//glDrawArrays(GL_TRIANGLES, 0, va->numverts);
			
#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, side->m_drawva.vertices);
#endif
#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, side->m_drawva.vertices);
#endif
			
			//glDrawArrays(GL_LINE_STRIP, 0, side->m_outline.m_edv.size());
			glDrawArrays(GL_LINE_STRIP, 0, side->m_drawva.numverts);
		}
	}

#if 1
	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		float topverts[] =
		{
			//top
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//top far right
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//top near right
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//top near left
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//top far left
		};

		float bottomverts[] =
		{
			//bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//bottom far right
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//bottom near right
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//bottom near left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//bottom far left
		};

		float leftverts[] =
		{
			//left
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//left top far
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//left top near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//left bottom near
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//left bottom far
		};

		float rightverts[] =
		{
			//right
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//right top far
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//right top near
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//right bottom near
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//right bottom far
		};

		float nearverts[] =
		{
			//near
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmax.z,	//near left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmax.z,	//near right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmax.z,	//near right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmax.z,	//near left bottom
		};

		float farverts[] =
		{
			//far
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
			pmh->absmin.x, pmh->absmax.y, pmh->absmin.z,	//far left top
			pmh->absmax.x, pmh->absmax.y, pmh->absmin.z,	//far right top
			pmh->absmax.x, pmh->absmin.y, pmh->absmin.z,	//far right bottom
			pmh->absmin.x, pmh->absmin.y, pmh->absmin.z,	//far left bottom
		};

		float alpha = 1.0f - Magnitude2((pmh->absmin+pmh->absmax)/2.0f - g_cam.m_view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);

		glUniform4f(shader->slot[SSLOT_COLOR], 0.2f, 0.9f, 0.3f, alpha);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, topverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, farverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
#endif
}

void DrawDrag_Door(EdMap* map, Matrix* mvp, int w, int h, bool persp)
{
	Shader* shader = &g_shader[g_curS];

	Brush* b = *g_selB.begin();

	EdDoor* door = b->m_door;
	//InfoMess("dd1","dd1");

	if(!door)
		return;

	//InfoMess("dd","dd");

	Vec3f startpoint = door->point;
	Vec3f axispoint = door->point + door->axis;

	Vec4f startscreenpos = ScreenPos(mvp, startpoint, w, h, persp);
	Vec4f axisscreenpos = ScreenPos(mvp, axispoint, w, h, persp);

	{
		float verts[] =
		{
			startscreenpos.x, startscreenpos.y, 0,
			axisscreenpos.x, axisscreenpos.y, 0
		};

		float colour2[] = DOOR_POINT_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
		
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
		
		glDrawArrays(GL_LINE_STRIP, 0, 2);

	}

	{
		float triverts[] =
		{
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0
		};
		
		float verts[] =
		{
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x + DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y + DOOR_POINT_DRAG_HSIZE, 0,
			startscreenpos.x - DOOR_POINT_DRAG_HSIZE, startscreenpos.y - DOOR_POINT_DRAG_HSIZE, 0
		};

		float colour[] = DOOR_POINT_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, triverts);
#endif
		
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, triverts);
#endif
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		float colour2[] = DOOR_POINT_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
		
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
		
		
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}

	{
		float triverts[] =
		{
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0
		};
		
		float verts[] =
		{
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE, 0,
			axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE, axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE, 0
		};

		float colour[] = DOOR_AXIS_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, triverts);
#endif
		
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, triverts);
#endif
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		float colour2[] = DOOR_AXIS_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
		
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
		
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
}


void DrawDrag_Clip(EdMap* map, Matrix* mvp, int w, int h, bool persp)
{
	return;

#if 0
	Vec2i vmin;
	Vec2i vmax;

	AllScreenMinMax(&vmin, &vmax, w, h);

	Shader* shader = &g_shader[g_curS];

	float verts[] =
	{
		vmin.x, vmin.y, 0,
		vmax.x, vmin.y, 0,
		vmax.x, vmax.y, 0,
		vmin.x, vmax.y, 0,
		vmin.x, vmin.y, 0
	};

	float colour[] = VERT_DRAG_FILLRGBA;
	glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

	glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	glDrawArrays(GL_QUADS, 0, 4);

	float colour2[] = VERT_DRAG_OUTLRGBA;
	glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

	glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
#endif
}

void DrawDrag_Ents(EdMap* map, Matrix* mvp, int w, int h, bool persp)
{
	Shader* shader = &g_shader[g_curS];

	if(g_selE)
	{
		Ent* e = g_selE;

		EType* et = &g_etype[e->type];
		Camera* c = &e->camera;

		//VertexArray va;

		//Model* m = &g_model[et->model[BODY_LOWER]];
		//m->m_ms3d.genva(&va, e->frame[BODY_LOWER], Vec3f(1,1,1), Vec3f(0,0,0), true);
		float alpha = 1.0f - Magnitude2(c->m_pos - g_cam.m_view) / (PROJ_RIGHT*2.0f/g_zoom) / (PROJ_RIGHT*2.0f/g_zoom);
		
		Matrix transmat, rotmat;
		transmat.translation((float*)&c->m_pos);
		float rotrad[3] = {DEGTORAD(c->m_orientv.x), DEGTORAD(c->m_orientv.y), 0};
		rotmat.rotrad(rotrad);

		Vec4f boxcen = ScreenPos(mvp, c->m_pos, w, h, persp);
		Vec3f viewbox3 = Vec3f(0,0,VIEWVEC_SIZE);
		viewbox3.transform(rotmat);
		viewbox3 = viewbox3 + c->m_pos;
		Vec4f viewbox = ScreenPos(mvp, viewbox3, w, h, persp);

		{
			float verts[] =
			{
				boxcen.x - DOOR_AXIS_DRAG_HSIZE, boxcen.y - DOOR_AXIS_DRAG_HSIZE, 0,
				boxcen.x + DOOR_AXIS_DRAG_HSIZE, boxcen.y - DOOR_AXIS_DRAG_HSIZE, 0,
				boxcen.x + DOOR_AXIS_DRAG_HSIZE, boxcen.y + DOOR_AXIS_DRAG_HSIZE, 0,
				boxcen.x + DOOR_AXIS_DRAG_HSIZE, boxcen.y + DOOR_AXIS_DRAG_HSIZE, 0,
				boxcen.x - DOOR_AXIS_DRAG_HSIZE, boxcen.y + DOOR_AXIS_DRAG_HSIZE, 0,
				boxcen.x - DOOR_AXIS_DRAG_HSIZE, boxcen.y - DOOR_AXIS_DRAG_HSIZE, 0
			};

			float colour[] = VERT_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

	#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	#endif
		
	#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, verts);
	#endif
		
			glDrawArrays(GL_TRIANGLES, 0, 6);

			float colour2[] = VERT_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

	#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	#endif
		
	#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, verts);
	#endif
		
			glDrawArrays(GL_LINE_STRIP, 0, 5);
		}

		{
			float line[] = 
			{
				boxcen.x, boxcen.y, 0,
				viewbox.x, viewbox.y, 0
			};
			
			float lcolour[] = VERT_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, lcolour);
			
	#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, line);
	#endif
		
	#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, line);
	#endif

			glDrawArrays(GL_LINES, 0, 2);

			float verts[] =
			{
				viewbox.x - DOOR_AXIS_DRAG_HSIZE, viewbox.y - DOOR_AXIS_DRAG_HSIZE, 0,
				viewbox.x + DOOR_AXIS_DRAG_HSIZE, viewbox.y - DOOR_AXIS_DRAG_HSIZE, 0,
				viewbox.x + DOOR_AXIS_DRAG_HSIZE, viewbox.y + DOOR_AXIS_DRAG_HSIZE, 0,
				viewbox.x + DOOR_AXIS_DRAG_HSIZE, viewbox.y + DOOR_AXIS_DRAG_HSIZE, 0,
				viewbox.x - DOOR_AXIS_DRAG_HSIZE, viewbox.y + DOOR_AXIS_DRAG_HSIZE, 0,
				viewbox.x - DOOR_AXIS_DRAG_HSIZE, viewbox.y - DOOR_AXIS_DRAG_HSIZE, 0
			};

			float colour[] = DOOR_AXIS_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

	#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	#endif
		
	#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, verts);
	#endif
		
			glDrawArrays(GL_TRIANGLES, 0, 6);

			float colour2[] = VERT_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

	#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
	#endif
		
	#ifdef PLATFORM_GL14
			glVertexPointer(3, GL_FLOAT, 0, verts);
	#endif
		
			glDrawArrays(GL_LINE_STRIP, 0, 5);
		}
	}
}

void DrawDrag_VertFaceBrush(EdMap* map, Matrix* mvp, int w, int h, bool persp)
{
	Shader* shader = &g_shader[g_curS];

	for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;

		for(int j=0; j<b->m_nsharedv; j++)
		{
			Vec3f sharedv = b->m_sharedv[j];
			Vec4f screenpos = ScreenPos(mvp, sharedv, w, h, persp);

			float triverts[] =
			{
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0
			};
			
			float verts[] =
			{
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0,
				screenpos.x + VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y + VERT_DRAG_HSIZE, 0,
				screenpos.x - VERT_DRAG_HSIZE, screenpos.y - VERT_DRAG_HSIZE, 0
			};

			float colour[] = VERT_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, triverts);
#endif
#ifdef PLATFORM_GL14
            glVertexPointer(3, GL_FLOAT, 0, triverts);
#endif
			glDrawArrays(GL_TRIANGLES, 0, 6);

			float colour2[] = VERT_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
#ifdef PLATFORM_GL14
            glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
			glDrawArrays(GL_LINE_STRIP, 0, 5);
		}
	}

	for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* side = &b->m_sides[j];
			Vec4f screenpos = ScreenPos(mvp, side->m_centroid, w, h, persp);

			float triverts[] =
			{
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0
			};
			
			float verts[] =
			{
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0,
				screenpos.x + FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y + FACE_DRAG_HSIZE, 0,
				screenpos.x - FACE_DRAG_HSIZE, screenpos.y - FACE_DRAG_HSIZE, 0
			};

			if(g_sel1b == b && g_dragS == j)
			{
				float colour[] = FACE_DRAG_SFILLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
			}
			else
			{
				float colour[] = FACE_DRAG_FILLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
			}

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, triverts);
#endif
#ifdef PLATFORM_GL14
            glVertexPointer(3, GL_FLOAT, 0, triverts);
#endif
			glDrawArrays(GL_TRIANGLES, 0, 6);

			if(g_sel1b == b && g_dragS == j)
			{
				float colour2[] = FACE_DRAG_SOUTLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
			}
			else
			{
				float colour2[] = FACE_DRAG_OUTLRGBA;
				glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
			}

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
#ifdef PLATFORM_GL14
            glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
			glDrawArrays(GL_LINE_STRIP, 0, 5);
		}
	}


	for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;
		Vec3f centroid = Vec3f(0,0,0);

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* s = &b->m_sides[j];

			centroid = centroid + s->m_centroid / (float)b->m_nsides;
		}

		Vec4f screenpos = ScreenPos(mvp, centroid, w, h, persp);

		float triverts[] =
		{
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0
		};
		
		float verts[] =
		{
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0
		};

		float coluor[] = BRUSH_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, coluor);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, triverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, triverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);

		float colour2[] = BRUSH_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}
}


void DrawDrag_ModelHolder(EdMap* map, Matrix* mvp, int w, int h, bool persp)
{
	Shader* shader = &g_shader[g_curS];

	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		// Top side
		Vec3f topcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmax.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f topscreenpos = ScreenPos(mvp, topcentroid, w, h, persp);
		float toptriverts[] =
		{
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0
		};
		float topverts[] =
		{
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Bottom side
		Vec3f bottomcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmin.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f bottomscreenpos = ScreenPos(mvp, bottomcentroid, w, h, persp);
		float bottomtriverts[] =
		{
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0
		};
		float bottomverts[] =
		{
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Left side
		Vec3f leftcentroid = Vec3f( pmh->absmin.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f leftscreenpos = ScreenPos(mvp, leftcentroid, w, h, persp);
		float lefttriverts[] =
		{
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0
		};
		float leftverts[] =
		{
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Right side
		Vec3f rightcentroid = Vec3f( pmh->absmax.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f rightscreenpos = ScreenPos(mvp, rightcentroid, w, h, persp);
		float righttriverts[] =
		{
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0
		};
		float rightverts[] =
		{
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Near side
		Vec3f nearcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmax.z );
		Vec4f nearscreenpos = ScreenPos(mvp, nearcentroid, w, h, persp);
		float neartriverts[] =
		{
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0
		};
		float nearverts[] =
		{
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Far side
		Vec3f farcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmin.z );
		Vec4f farscreenpos = ScreenPos(mvp, farcentroid, w, h, persp);
		float fartriverts[] =
		{
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0
		};
		float farverts[] =
		{
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Top side
		if(g_sel1m == pmh && g_dragS == DRAG_TOP)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, toptriverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, toptriverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		if(g_sel1m == pmh && g_dragS == DRAG_TOP)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, topverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, topverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Bottom side
		if(g_sel1m == pmh && g_dragS == DRAG_BOTTOM)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomtriverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, bottomtriverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		if(g_sel1m == pmh && g_dragS == DRAG_BOTTOM)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, bottomverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, bottomverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Left side
		if(g_sel1m == pmh && g_dragS == DRAG_LEFT)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, lefttriverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, lefttriverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		if(g_sel1m == pmh && g_dragS == DRAG_LEFT)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, leftverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, leftverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Right side
		if(g_sel1m == pmh && g_dragS == DRAG_RIGHT)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, righttriverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, righttriverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		if(g_sel1m == pmh && g_dragS == DRAG_RIGHT)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, rightverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, rightverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Near side
		if(g_sel1m == pmh && g_dragS == DRAG_NEAR)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, neartriverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, neartriverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		if(g_sel1m == pmh && g_dragS == DRAG_NEAR)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, nearverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, nearverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);

		// Far side
		if(g_sel1m == pmh && g_dragS == DRAG_FAR)
		{
			float colour[] = FACE_DRAG_SFILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
		else
		{
			float colour[] = FACE_DRAG_FILLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, fartriverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, farverts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		if(g_sel1m == pmh && g_dragS == DRAG_FAR)
		{
			float colour2[] = FACE_DRAG_SOUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
		else
		{
			float colour2[] = FACE_DRAG_OUTLRGBA;
			glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);
		}
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, farverts);
#endif
#ifdef PLATFORM_GL14
        glVertexPointer(3, GL_FLOAT, 0, farverts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 5);


	}


	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		Vec3f centroid = (pmh->absmin + pmh->absmax) / 2.0f;

		Vec4f screenpos = ScreenPos(mvp, centroid, w, h, persp);

		float verts[] =
		{
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x + BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y + BRUSH_DRAG_HSIZE, 0,
			screenpos.x - BRUSH_DRAG_HSIZE, screenpos.y - BRUSH_DRAG_HSIZE, 0
		};

		float colorr[] = BRUSH_DRAG_FILLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colorr);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);

		float colour2[] = BRUSH_DRAG_OUTLRGBA;
		glUniform4fv(shader->slot[SSLOT_COLOR], 1, colour2);

#ifdef PLATFORM_GLES20
		glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, verts);
#endif
#ifdef PLATFOMR_GL14
		glVertexPointer(3, GL_FLOAT, 0, verts);
#endif
		glDrawArrays(GL_LINE_STRIP, 0, 6);
	}
}

//draw drag handles
void DrawDrag(EdMap* map, Matrix* mvp, int w, int h, bool persp)
{
	Shader* shader = &g_shader[g_curS];

	if(!g_gui.get("door edit")->m_hidden)
	{
		//InfoMess("dd3","dd3");

		if(g_selB.size() <= 0)
			return;

		//return;
		
		//InfoMess("dd2","dd2");

		DrawDrag_Door(map, mvp, w, h, persp);
	}
	else if(g_selB.size() > 0)
	{
		DrawDrag_VertFaceBrush(map, mvp, w, h, persp);
	}
	else if(g_selM.size() > 0)
	{
		DrawDrag_ModelHolder(map, mvp, w, h, persp);
	}
	
	DrawDrag_Clip(map, mvp, w, h, persp);
	
	DrawDrag_Ents(map, mvp, w, h, persp);
}

void SelectBrush(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax)
{
	g_dragV = -1;
	g_dragS = -1;
	CloseSideView();
	//CloseView("brush edit");

	//g_applog<<"select brush ("<<line[0].x<<","<<line[0].y<<","<<line[0].z<<")->("<<line[1].x<<","<<line[1].y<<","<<line[1].z<<")"<<std::endl;
	std::list<Brush*> selB;
	std::list<ModelHolder*> selM;
	std::list<Ent*> selE;

	//distances from user camera
	std::list<float> selBd;
	std::list<float> selMd;
	std::list<float> selEd;

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Vec3f trace = b->traceray(line);
		if(trace != line[1] && trace.y <= g_maxelev
#if 1
			&&
			trace.x >= vmin.x && trace.x <= vmax.x &&
			trace.y >= vmin.y && trace.y <= vmax.y &&
			trace.z >= vmin.z && trace.z <= vmax.z
#endif
			)
		{
			//g_selB.clear();
			//g_selB.push_back(&*b);
			selB.push_back(&*b);
			//OpenAnotherView("brush edit");
			//return;

			float d = Magnitude(trace - line[0]);
			selBd.push_back(d);
		}
	}

	for(auto mh=g_modelholder.begin(); mh!=g_modelholder.end(); mh++)
	{
		Vec3f trace = mh->traceray(line);
		if(trace != line[1] && trace.y <= g_maxelev
#if 1
			&&
			trace.x >= vmin.x && trace.x <= vmax.x &&
			trace.y >= vmin.y && trace.y <= vmax.y &&
			trace.z >= vmin.z && trace.z <= vmax.z
#endif
			)
		{
			//g_selB.clear();
			//g_selB.push_back(&*b);
			selM.push_back(&*mh);
			//OpenAnotherView("brush edit");
			//return;
			
			float d = Magnitude(trace - line[0]);
			selMd.push_back(d);
		}
	}

	for(int ei=0; ei<ENTITIES; ei++)
	{
		Ent* e = &g_entity[ei];

		if(!e->on)
			continue;

		Vec3f trace = e->traceray(line);
		
		if(trace != line[1] && trace.y <= g_maxelev
#if 1
			&&
			trace.x >= vmin.x && trace.x <= vmax.x &&
			trace.y >= vmin.y && trace.y <= vmax.y &&
			trace.z >= vmin.z && trace.z <= vmax.z
#endif
			)
		{
			selE.push_back(e);	
			
			float d = Magnitude(trace - line[0]);
			selEd.push_back(d);
		}
	}

#define CLOSE_NONE	-1
#define CLOSE_ENT	0
#define CLOSE_BR	1
#define CLOSE_MH	2

	//closest object
	float closestd = -1;
	int closetype = CLOSE_NONE;

	//closest object AFTER last selected object
	float closestafterd = -1;
	int closeaftertype = CLOSE_NONE;
	
	//selected objected start distance, if selected
	float startd = -1;

	//get starting distance
	if(g_selB.size())
	{
		Brush* b = *g_selB.begin();
		Vec3f trace = b->traceray(line);
		startd = Magnitude(line[0] - trace);
	}
	else if(g_selM.size())
	{
		ModelHolder* mh = *g_selM.begin();
		Vec3f trace = mh->traceray(line);
		startd = Magnitude(line[0] - trace);
	}
	else if(g_selE)
	{
		Ent* e = g_selE;
		Vec3f trace = e->traceray(line);
		startd = Magnitude(line[0] - trace);
	}

	//find next object after last selected one, and closest object to camera, if that fails

	auto sbit = selB.begin();
	auto sbitd = selBd.begin();

	auto smit = selM.begin();
	auto smitd = selMd.begin();

	auto seit = selE.begin();
	auto seitd = selEd.begin();
	
	void* closestobj = NULL;
	void* closestafterobj = NULL;

	//intersected brushes
	for(; sbit!=selB.end(); sbit++, sbitd++)
	{
		//check closest
		if(closestd < 0 || *sbitd < closestd)
		{
			closestd = *sbitd;
			closetype = CLOSE_BR;
			closestobj = (void*)*sbit;
		}

		//check closest after last selected object
		if(startd >= 0 && *sbitd < closestafterd && *sbitd > startd)
		{
			closestafterd = *sbitd;
			closeaftertype = CLOSE_BR;
			closestafterobj = (void*)*sbit;
		}
	}

	//intersected model holders
	for(; smit!=selM.end(); smit++, smitd++)
	{
		//check closest
		if(closestd < 0 || *smitd < closestd)
		{
			closestd = *smitd;
			closetype = CLOSE_MH;
			closestobj = (void*)*smit;
		}

		//check closest after last selected object
		if(startd >= 0 && *smitd < closestafterd && *smitd > startd)
		{
			closestafterd = *smitd;
			closeaftertype = CLOSE_MH;
			closestafterobj = (void*)*smit;
		}
	}

	//intersected entities
	for(; seit!=selE.end(); seit++, seitd++)
	{
		//check closest
		if(closestd < 0 || *seitd < closestd)
		{
			closestd = *seitd;
			closetype = CLOSE_ENT;
			closestobj = (void*)*seit;
		}

		//check closest after last selected object
		if(startd >= 0 && *seitd < closestafterd && *seitd > startd)
		{
			closestafterd = *seitd;
			closeaftertype = CLOSE_ENT;
			closestafterobj = (void*)*seit;
		}
	}

	g_selB.clear();
	g_selM.clear();
	g_selE = NULL;

	void* finalobj = NULL;
	int finaltype = CLOSE_NONE;

	//if we had a previously selected object, try selecting the one after
	if(startd >= 0)
	{
		if(closestafterd >= 0)
		{
			finaltype = closeaftertype;
			finalobj = closestafterobj;
		}
		//if no object after, select first to camera
		else if(closestd >= 0)
		{
			finaltype = closetype;
			finalobj = closestobj;
		}
	}
	//otherwise, select first to camera
	else
	{
		if(closestd >= 0)
		{
			finaltype = closetype;
			finalobj = closestobj;
		}
	}

	switch(finaltype)
	{
	case CLOSE_ENT:
		g_selE = (Ent*)finalobj;
		break;
	case CLOSE_BR:
		g_selB.push_back((Brush*)finalobj);
		break;
	case CLOSE_MH:
		g_selM.push_back((ModelHolder*)finalobj);
		break;
	default:
		break;
	}

#if 0
	//If we already have a selected brush (globally),
	//choose the one after it in our selection array
	//(selB) as the next selected brush.
	if(g_selB.size() == 1)
	{
		bool found = false;
		for(auto i=selB.begin(); i!=selB.end(); i++)
		{
			if(found)
			{
				g_selB.clear();
				g_selB.push_back(*i);
				CloseSideView();
				g_gui.show("brush edit");
				return;
			}

			auto j = g_selB.begin();

			if(*i == *j)
				found = true;
		}
	}

	int prevnB = g_selB.size();
	g_selB.clear();

	//If we've reached the end of the selection array,
	//(selB), select through the models,
	if(g_selM.size() == 1)
	{
		bool found = false;
		for(auto i=selM.begin(); i!=selM.end(); i++)
		{
			if(found)
			{
				g_selM.clear();
				g_selM.push_back(*i);
				//OpenAnotherView("model edit");
				return;
			}

			auto j = g_selM.begin();

			if(*i == *j)
				found = true;
		}
	}

	int prevnM = g_selM.size();
	g_selM.clear();

	//and then restart at model at the front
	//if we previously selected a brush or if
	//there aren't any brushes at all
	if(selM.size() > 0
		&& (prevnB > 0 || map->m_brush.size() == 0))
	{
		g_selM.push_back( *selM.begin() );
	}
	//or else the brush at the front.
	else if(selB.size() > 0)
	{
		g_selB.push_back( *selB.begin() );
		g_gui.show("brush edit");
	}
#endif
}

bool SelectDrag_Door(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp, Brush* b, EdDoor* door)
{
	Vec3f startvec = door->point;
	Vec3f axisvec = door->point + door->axis;

	Vec4f startscreenpos = ScreenPos(mvp, startvec, w, h, persp);
	Vec4f axisscreenpos = ScreenPos(mvp, axisvec, w, h, persp);

	if(x >= axisscreenpos.x - DOOR_AXIS_DRAG_HSIZE && x <= axisscreenpos.x + DOOR_AXIS_DRAG_HSIZE && y >= axisscreenpos.y - DOOR_AXIS_DRAG_HSIZE && y <= axisscreenpos.y + DOOR_AXIS_DRAG_HSIZE)
	{
		g_sel1b = b;
		g_dragV = -1;
		g_dragS = -1;
		g_dragW = false;
		g_dragD = DRAG_DOOR_AXIS;
		return true;
	}

	if(x >= startscreenpos.x - DOOR_POINT_DRAG_HSIZE && x <= startscreenpos.x + DOOR_POINT_DRAG_HSIZE && y >= startscreenpos.y - DOOR_POINT_DRAG_HSIZE && y <= startscreenpos.y + DOOR_POINT_DRAG_HSIZE)
	{
		g_sel1b = b;
		g_dragV = -1;
		g_dragS = -1;
		g_dragW = false;
		g_dragD = DRAG_DOOR_POINT;
		return true;
	}

	return false;
}

void SelectDrag_VertFaceBrush(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp)
{
	float nearest = -1;

	for(int i=0; i<g_selB.size(); i++)
	{
		Brush* b = g_selB[i];

		for(int j=0; j<b->m_nsharedv; j++)
		{
			Vec3f sharedv = b->m_sharedv[j];
			Vec4f screenpos = ScreenPos(mvp, sharedv, w, h, persp);

			if(x >= screenpos.x - VERT_DRAG_HSIZE && x <= screenpos.x + VERT_DRAG_HSIZE && y >= screenpos.y - VERT_DRAG_HSIZE && y <= screenpos.y + VERT_DRAG_HSIZE)
			{
				float thisD = Magnitude(b->m_sharedv[j] - eye);

				if(thisD < nearest || nearest < 0)
				{
					g_sel1b = b;
					g_dragV = j;
					g_dragS = -1;
					g_dragW = false;
					nearest = thisD;
				}
			}
		}

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* side = &b->m_sides[j];
			//g_applog<<"centroid "<<side->m_centroid.x<<","<<side->m_centroid.y<<","<<side->m_centroid.z<<std::endl;
			//g_applog.flush();
			Vec4f screenpos = ScreenPos(mvp, side->m_centroid, w, h, persp);

			if(x >= screenpos.x - FACE_DRAG_HSIZE && x <= screenpos.x + FACE_DRAG_HSIZE && y >= screenpos.y - FACE_DRAG_HSIZE && y <= screenpos.y + FACE_DRAG_HSIZE)
			{
				float thisD = Magnitude(side->m_centroid - eye);

				if(thisD <= nearest || nearest < 0 || g_dragS < 0)
				{
					g_sel1b = b;
					g_dragV = -1;
					g_dragS = j;
					g_dragW = false;
					nearest = thisD;
				}
			}
		}

		Vec3f centroid(0,0,0);

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* side = &b->m_sides[j];
			centroid = centroid + side->m_centroid;
		}

		centroid = centroid / (float)b->m_nsides;
		Vec4f screenpos = ScreenPos(mvp, centroid, w, h, persp);

		if(x >= screenpos.x - BRUSH_DRAG_HSIZE && x <= screenpos.x + BRUSH_DRAG_HSIZE && y >= screenpos.y - BRUSH_DRAG_HSIZE && y <= screenpos.y + BRUSH_DRAG_HSIZE)
		{
			float thisD = Magnitude(centroid - eye);

			g_sel1b = b;
			g_dragV = -1;
			g_dragS = -1;
			g_dragW = true;
			nearest = thisD;
		}
	}
}

void SelectDrag_ModelHolder(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp)
{
	float nearest = -1;
	Shader* shader = &g_shader[g_curS];

	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		// Whole model centroid
		Vec3f modelcentroid =(pmh->absmin + pmh->absmax)/2.0f;
		Vec4f modelscreenpos = ScreenPos(mvp, modelcentroid, w, h, persp);
		float centerverts[] =
		{
			modelscreenpos.x - FACE_DRAG_HSIZE, modelscreenpos.y - FACE_DRAG_HSIZE, 0,
			modelscreenpos.x + FACE_DRAG_HSIZE, modelscreenpos.y - FACE_DRAG_HSIZE, 0,
			modelscreenpos.x + FACE_DRAG_HSIZE, modelscreenpos.y + FACE_DRAG_HSIZE, 0,
			modelscreenpos.x - FACE_DRAG_HSIZE, modelscreenpos.y + FACE_DRAG_HSIZE, 0,
			modelscreenpos.x - FACE_DRAG_HSIZE, modelscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Top side
		Vec3f topcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmax.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f topscreenpos = ScreenPos(mvp, topcentroid, w, h, persp);
		float topverts[] =
		{
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0,
			topscreenpos.x + FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y + FACE_DRAG_HSIZE, 0,
			topscreenpos.x - FACE_DRAG_HSIZE, topscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Bottom side
		Vec3f bottomcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, pmh->absmin.y, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f bottomscreenpos = ScreenPos(mvp, bottomcentroid, w, h, persp);
		float bottomverts[] =
		{
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x + FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y + FACE_DRAG_HSIZE, 0,
			bottomscreenpos.x - FACE_DRAG_HSIZE, bottomscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Left side
		Vec3f leftcentroid = Vec3f( pmh->absmin.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f leftscreenpos = ScreenPos(mvp, leftcentroid, w, h, persp);
		float leftverts[] =
		{
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0,
			leftscreenpos.x + FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y + FACE_DRAG_HSIZE, 0,
			leftscreenpos.x - FACE_DRAG_HSIZE, leftscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Right side
		Vec3f rightcentroid = Vec3f( pmh->absmax.x, (pmh->absmin.y + pmh->absmax.y)/2.0f, (pmh->absmin.z + pmh->absmax.z)/2.0f );
		Vec4f rightscreenpos = ScreenPos(mvp, rightcentroid, w, h, persp);
		float rightverts[] =
		{
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0,
			rightscreenpos.x + FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y + FACE_DRAG_HSIZE, 0,
			rightscreenpos.x - FACE_DRAG_HSIZE, rightscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Near side
		Vec3f nearcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmax.z );
		Vec4f nearscreenpos = ScreenPos(mvp, nearcentroid, w, h, persp);
		float nearverts[] =
		{
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0,
			nearscreenpos.x + FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y + FACE_DRAG_HSIZE, 0,
			nearscreenpos.x - FACE_DRAG_HSIZE, nearscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Far side
		Vec3f farcentroid = Vec3f( (pmh->absmin.x + pmh->absmax.x)/2.0f, (pmh->absmin.y + pmh->absmax.y)/2.0f, pmh->absmin.z );
		Vec4f farscreenpos = ScreenPos(mvp, farcentroid, w, h, persp);
		float farverts[] =
		{
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0,
			farscreenpos.x + FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y + FACE_DRAG_HSIZE, 0,
			farscreenpos.x - FACE_DRAG_HSIZE, farscreenpos.y - FACE_DRAG_HSIZE, 0
		};

		// Top side
		if(x >= topscreenpos.x - FACE_DRAG_HSIZE && x <= topscreenpos.x + FACE_DRAG_HSIZE && y >= topscreenpos.y - FACE_DRAG_HSIZE && y <= topscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(topcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_TOP;
				g_dragW = false;
				nearest = thisD;
			}
		}

		// Bottom side
		if(x >= bottomscreenpos.x - FACE_DRAG_HSIZE && x <= bottomscreenpos.x + FACE_DRAG_HSIZE && y >= bottomscreenpos.y - FACE_DRAG_HSIZE && y <= bottomscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(bottomcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_BOTTOM;
				g_dragW = false;
				nearest = thisD;
			}
		}

		// Left side
		if(x >= leftscreenpos.x - FACE_DRAG_HSIZE && x <= leftscreenpos.x + FACE_DRAG_HSIZE && y >= leftscreenpos.y - FACE_DRAG_HSIZE && y <= leftscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(leftcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_LEFT;
				g_dragW = false;
				nearest = thisD;
			}
		}

		// Right side
		if(x >= rightscreenpos.x - FACE_DRAG_HSIZE && x <= rightscreenpos.x + FACE_DRAG_HSIZE && y >= rightscreenpos.y - FACE_DRAG_HSIZE && y <= rightscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(rightcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_RIGHT;
				g_dragW = false;
				nearest = thisD;
			}
		}

		// Near side
		if(x >= nearscreenpos.x - FACE_DRAG_HSIZE && x <= nearscreenpos.x + FACE_DRAG_HSIZE && y >= nearscreenpos.y - FACE_DRAG_HSIZE && y <= nearscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(nearcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_NEAR;
				g_dragW = false;
				nearest = thisD;
			}
		}

		// Far side
		if(x >= farscreenpos.x - FACE_DRAG_HSIZE && x <= farscreenpos.x + FACE_DRAG_HSIZE && y >= farscreenpos.y - FACE_DRAG_HSIZE && y <= farscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(farcentroid - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = DRAG_FAR;
				g_dragW = false;
				nearest = thisD;
			}
		}

		// Whole model drag
		if(x >= modelscreenpos.x - FACE_DRAG_HSIZE && x <= modelscreenpos.x + FACE_DRAG_HSIZE && y >= modelscreenpos.y - FACE_DRAG_HSIZE && y <= modelscreenpos.y + FACE_DRAG_HSIZE)
		{
			float thisD = Magnitude(modelcentroid - eye);

			// Prefer selecting the whole model drag handle over selecting a side drag handle
			if(thisD < nearest || nearest < 0 || g_sel1m == pmh || g_sel1m == NULL)
			{
				g_sel1m = pmh;
				g_dragV = -1;
				g_dragS = -1;
				g_dragW = true;
				nearest = thisD;
			}
		}
	}
}

void SelectDrag_Ent(Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp)
{
	float nearest = -1;
	Shader* shader = &g_shader[g_curS];
	Ent* e = g_selE;

	{
		EType* et = &g_etype[e->type];
		Model* m = &g_model[et->model[BODY_LOWER]];
		Camera* c = &e->camera;
		
		Matrix transmat, rotmat;
		transmat.translation((float*)&c->m_pos);
		float rotrad[3] = {DEGTORAD(c->m_orientv.x), DEGTORAD(c->m_orientv.y), 0};
		rotmat.rotrad(rotrad);

		Vec4f boxcen = ScreenPos(mvp, c->m_pos, w, h, persp);
		Vec3f viewbox3 = Vec3f(0,0,100);
		viewbox3.transform(rotmat);
		viewbox3 = viewbox3 + c->m_pos;
		Vec4f viewbox = ScreenPos(mvp, viewbox3, w, h, persp);

		if(x >= boxcen.x - DOOR_AXIS_DRAG_HSIZE && x <= boxcen.x + DOOR_AXIS_DRAG_HSIZE && y >= boxcen.y - DOOR_AXIS_DRAG_HSIZE && y <= boxcen.y + DOOR_AXIS_DRAG_HSIZE)
		{
			float thisD = Magnitude(c->m_pos - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_dragE_r = false;
				g_dragE_p = true;
				nearest = thisD;
			}
		}
		
		if(x >= viewbox.x - DOOR_AXIS_DRAG_HSIZE && x <= viewbox.x + DOOR_AXIS_DRAG_HSIZE && y >= viewbox.y - DOOR_AXIS_DRAG_HSIZE && y <= viewbox.y + DOOR_AXIS_DRAG_HSIZE)
		{
			float thisD = Magnitude(c->m_pos - eye);

			if(thisD < nearest || nearest < 0)
			{
				g_dragE_r = true;
				g_dragE_p = false;
				nearest = thisD;
			}
		}
	}
}

bool SelectDrag(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp)
{
	g_sel1b = NULL;
	g_sel1m = NULL;
	g_dragV = -1;
	g_dragS = -1;
	g_dragW = false;
	g_dragD = -1;
	g_dragE_r = false;
	g_dragE_p = false;

#if 1
	if(!g_gui.get("door edit")->m_hidden)
	{
		if(g_selB.size() <= 0)
			goto nodoor;

		Brush* b = *g_selB.begin();
		EdDoor* door = b->m_door;

		if(door == NULL)
			goto nodoor;

		if(SelectDrag_Door(map, mvp, w, h, x, y, eye, persp, b, door))
			return true;
	}

	nodoor:
#endif

	CloseSideView();
	//CloseView("brush side edit");

	if(g_selB.size())
	{
		SelectDrag_VertFaceBrush(map, mvp, w, h, x, y, eye, persp);

		if(g_sel1b != NULL)
		{
			if(g_dragS >= 0)
			{
				CloseSideView();
				g_gui.show("brush edit");
				g_gui.show("brush side edit");
				RedoBSideGUI();
			}

			return true;
		}
	}

	if(g_selM.size())
	{
		SelectDrag_ModelHolder(map, mvp, w, h, x, y, eye, persp);

		if(g_sel1m != NULL)
		{
			//MessageBox(g_hWnd, "sle m", "asd", NULL);
			return true;
		}
	}

	if(g_selE)
	{
		SelectDrag_Ent(mvp, w, h, x, y, eye, persp);

		if(g_dragE_p || g_dragE_r)
			return true;
	}

	return false;
}


//if a brush vert is behind the plane, we can use it to add a brush side
//otherwise, if no verts are behind the plane, the brush must be pruned/destroyed.
bool PruneB2(Brush* b, Plane3f* p, float epsilon)
{
	for(int svi=0; svi<b->m_nsharedv; svi++)
	{
		Vec3f sv = b->m_sharedv[svi];

		if(PointOnOrBehindPlane(sv, p->m_normal, p->m_d, epsilon))
			return false;
	}

	return true;
}

//all brush verts behind plane?
bool AllBeh(Brush* b, Plane3f* p, float epsilon)
{
	for(int svi=0; svi<b->m_nsharedv; svi++)
	{
		Vec3f sv = b->m_sharedv[svi];

		if(!PointOnOrBehindPlane(sv, p->m_normal, p->m_d, epsilon))
			return false;
	}

	return true;
}

//#define PRUNEB_DEBUG

bool PruneB(EdMap* map, Brush* b)
{
#ifdef PRUNEB_DEBUG
	g_applog<<"-------------"<<std::endl;
	g_applog.flush();
#endif
	
#if 1
	//remove sides that don't have triangles
	for(int i=0; i<b->m_nsides; i++)
	{
#ifdef PRUNEB_DEBUG
		g_applog<<"\t side"<<i<<std::endl;
#endif
		BrushSide* s1 = &b->m_sides[i];

		//if(s1->m_ntris <= 0)
		if(s1->m_sideverts.size() < 3)
		{
#ifdef PRUNEB_DEBUG
		g_applog<<"\t rem side"<<i<<std::endl;
#endif
			b->removeside(i);
			i--;
			continue;
		}
	}
#endif

#if 0
	//remove sides that share the exact same set of vertices
	for(int i=0; i<b->m_nsides; i++)
	{
#ifdef PRUNEB_DEBUG
		g_applog<<"\t side"<<i<<std::endl;
#endif
		BrushSide* s1 = &b->m_sides[i];

		bool allclose = true;
		Vec3f matchv = b->m_sharedv[ s1->m_vindices[0] ];
		
#ifdef PRUNEB_DEBUG
		g_applog<<"s1->m_ntris = "<<s1->m_ntris<<std::endl;
#endif

		for(int v=0; v<s1->m_outline.m_edv.size(); v++)
		{
			Vec3f thisv = b->m_sharedv[ s1->m_vindices[v] ];
			
#ifdef PRUNEB_DEBUG
			g_applog<<"vertex "<<v<<" = "<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
			g_applog.flush();
#endif

			float mag = Magnitude(matchv - thisv);

			if(mag > MERGEV_D)
			{
				allclose = false;		
#ifndef PRUNEB_DEBUG
				break;
#endif
			}
		}

		if(allclose)
		{
			b->removeside(i);
			i--;
			continue;
		}

		for(int j=i+1; j<b->m_nsides; j++)
		{
			BrushSide* s2 = &b->m_sides[j];
			bool same = true;

			for(int k=0; k<s1->m_outline.m_edv.size(); k++)
			{
				int matchindex = s1->m_vindices[k];
				bool have = false;

				for(int l=0; l<s2->m_outline.m_edv.size(); l++)
				{
					if(s2->m_vindices[l] == matchindex)
					{
						have = true;
						break;
					}
				}

				if(!have)
				{
					same = false;
					break;
				}
			}

			if(same)
			{
				b->removeside(j);
				break;
			}

			for(int k=0; k<s2->m_outline.m_edv.size(); k++)
			{
				int matchindex = s2->m_vindices[k];
				bool have = false;

				for(int l=0; l<s1->m_outline.m_edv.size(); l++)
				{
					if(s1->m_vindices[l] == matchindex)
					{
						have = true;
						break;
					}
				}

				if(!have)
				{
					same = false;
					break;
				}
			}

			if(same)
			{
				b->removeside(j);
				break;
			}
		}
	}
#endif

#if 1
	//for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		// remove brushes with less than 3 vertices or 4 sides
		if(b->m_nsharedv < 3 || b->m_nsides < 4)
		{
			for(int i=0; i<g_selB.size(); i++)
			{
				//if(g_selB[i] == &*b)
				if(g_selB[i] == b)
				{
					g_selB.erase( g_selB.begin() + i );
					//continue;
					i--;
				}
			}
			
			//if(g_sel1b == &*b)
			if(g_sel1b == b)
			{
				g_sel1b = NULL;
				g_dragV = -1;
				g_dragS = -1;
			}

			//b = map->m_brush.erase(b);

			for(auto i=map->m_brush.begin(); i!=map->m_brush.end(); i++)
			{
				if(&*i == b)
				{
					map->m_brush.erase(i);
					//break;
					return true;
				}
			}

			//g_applog<<"pruned brush"<<std::endl;
			//g_applog.flush();
			//continue;

			return true;
		}
	}
#endif

	//so it seems this function can't do any of these checks now

	return false;
}