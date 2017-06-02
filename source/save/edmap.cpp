

#include "edmap.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../utils.h"
#include "../save/modelholder.h"
#include "../debug.h"
#include "../render/shadow.h"
#include "../sim/entity.h"

EdMap g_edmap;
std::vector<Brush*> g_selB;
Ent* g_selE = NULL;
Brush* g_sel1b = NULL;	//drag selected brush (brush being dragged or manipulated currently)
int g_dragV = -1;	//drag vertex of selected brush?
int g_dragS = -1;	//drag side of selected brush?
bool g_dragW = false;	//drag whole brush or model?
int g_dragD = -1;	// dragging DRAG_DOOR_POINT or DRAG_DOOR_AXIS ?
int g_dragM = -1;	//dragging model holder?
bool g_dragE_r = false;	//dragging entity rotation box?
bool g_dragE_p = false;	//dragging entity position box?
std::vector<ModelHolder*> g_selM;
ModelHolder* g_sel1m = NULL;	//drag selected model (model being dragged or manipulated currently)

void DrawEdMap(EdMap* map, bool showsky)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* s = &g_shader[g_curS];

	//glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, g_camproj.m_matrix);
	//glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, g_camview.m_matrix);

	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
    CheckGLError(__FILE__, __LINE__);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);
			CheckGLError(__FILE__, __LINE__);

	Matrix modelview;
    modelview.set(g_camview.m_matrix);
    modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);
			CheckGLError(__FILE__, __LINE__);

#if 1
	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
    Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);
#endif
	
			CheckGLError(__FILE__, __LINE__);

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			side->usedifftex();
			CheckGLError(__FILE__, __LINE__);
			side->usespectex();
			CheckGLError(__FILE__, __LINE__);
			side->usenormtex();
			CheckGLError(__FILE__, __LINE__);
			side->useteamtex();

			CheckGLError(__FILE__, __LINE__);
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1i(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &side->m_drawva;
			glActiveTexture(GL_TEXTURE0);

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
#endif
			
#ifdef PLATFORM_GL14
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
			CheckGLError(__FILE__, __LINE__);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			CheckGLError(__FILE__, __LINE__);
            glNormalPointer(GL_FLOAT, 0, va->normals);
			CheckGLError(__FILE__, __LINE__);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif

			CheckGLError(__FILE__, __LINE__);
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
			CheckGLError(__FILE__, __LINE__);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
		}
	}
}

void DrawEdMapDepth(EdMap* map, bool showsky)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* shader = &g_shader[g_curS];

	Matrix modelmat;
	modelmat.reset();
    glUniformMatrix4fv(shader->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];
			/*
			CreateTex(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			side->usedifftex();
			/*
			unsigned int atex;
			CreateTex(atex, "gui/dmd.jpg", false);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1i(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
	*/

			glActiveTexture(GL_TEXTURE0);
			glUniform1i(shader->slot[SSLOT_TEXTURE0], 0);

			VertexArray* va = &side->m_drawva;

#ifdef PLATFORM_GLES20
			glVertexAttribPointer(shader->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			glVertexAttribPointer(shader->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#endif
			
#ifdef PLATFORM_GL14
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif
			
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
		}
	}
}
