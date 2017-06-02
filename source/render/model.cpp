#include "../platform.h"
#include "../math/3dmath.h"
#include "model.h"
#include "../texture.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "shader.h"
#include "../debug.h"
#include "vertexarray.h"
#include "dmdmodel.h"
#include "shadow.h"

Model g_model[MODELS];
std::vector<ModelToLoad> g_modelsToLoad;

Model::~Model()
{
	destroy();
}

void Model::destroy()
{
	//if(m_va == NULL)
	//	return;

	g_applog<<"Freeing model "<<m_fullpath<<std::endl;
	g_applog.flush();

	//delete [] m_va;
	//m_va = NULL;

	m_ms3d.destroy();

	m_on = false;
}

int NewModel()
{
	for(int i=0; i<MODELS; i++)
		if(!g_model[i].m_on)
			return i;

	return -1;
}

void QueueModel(int* id, const char* relative, Vec3f scale, Vec3f translate, bool blendnorm)
{
	ModelToLoad toLoad;
	toLoad.id = id;
	strcpy(toLoad.filepath, relative);
	toLoad.scale = scale;
	toLoad.translate = translate;
	toLoad.blendnorm = blendnorm;

	g_modelsToLoad.push_back(toLoad);
}

bool Load1Model()
{
	static int last = -1;

	if(last+1 < g_modelsToLoad.size())
		Status(g_modelsToLoad[last+1].filepath);

	if(last >= 0)
	{
		int id = NewModel();

		g_model[id].load(g_modelsToLoad[last].filepath, g_modelsToLoad[last].scale, g_modelsToLoad[last].translate, false, g_modelsToLoad[last].blendnorm);
		(*g_modelsToLoad[last].id) = id;
	}

	last ++;

	if(last >= g_modelsToLoad.size())
	{
		g_modelsToLoad.clear();
		return false;	// Done loading all models
	}

	return true;	// Not finished loading models
}

void DrawVA(VertexArray* va, Vec3f pos)
{
	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
	modelmat.translation((const float*)&pos);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);

	if(s->slot[SSLOT_NORMAL] != -1)
		glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif
	
#ifdef PLATFORM_GL14
	//glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_POSITION]);
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexPointer(3, GL_FLOAT, 0, 0);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	//glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_TEXCOORD]);
	//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, 0);
	//glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);

	//if(s->slot[SSLOT_NORMAL] != -1)
	{
		//glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_NORMAL]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glNormalPointer(GL_FLOAT, 0, 0);
		glNormalPointer(GL_FLOAT, 0, va->normals);
	}
#endif

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}


void DrawVADepth(VertexArray* va, Vec3f pos)
{
	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
    modelmat.translation((const float*)&pos);
    glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#endif
	
#ifdef PLATFORM_GL14
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
#endif
	
    //glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    //glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    //glVertexPointer(3, GL_FLOAT, 0, va->vertices);
    //glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
    //glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void Model::draw(int frame, Vec3f pos, float yaw, VertexArray* va)
{
	Shader* s = &g_shader[g_curS];

	float pitch = 0;
	Matrix modelmat;
	float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
	modelmat.translation((const float*)&pos);
	Matrix rotation;
	rotation.rotrad(radians);
	modelmat.postmult(rotation);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	//VertexArray* va = &m_va[frame];


	usedifftex();
	usespectex();
	usenormtex();
	useteamtex();

#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);

	if(s->slot[SSLOT_NORMAL] != -1)
		glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#endif
	
#ifdef PLATFORM_GL14
	//glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_POSITION]);
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexPointer(3, GL_FLOAT, 0, 0);
	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	//glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_TEXCOORD]);
	//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, 0);
	//glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);

#if 0
	if(s->slot[SSLOT_NORMAL] != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_NORMAL]);
		glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
#else
	//glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_NORMAL]);
	//glNormalPointer(GL_FLOAT, 0, 0);
	glNormalPointer(GL_FLOAT, 0, va->normals);
#endif
#endif

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

int FindModel(const char* relative)
{
	char full[DMD_MAX_PATH+1];
	FullPath(relative, full);
	char corrected[DMD_MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);

	for(int i=0; i<MODELS; i++)
	{
		Model* m = &g_model[i];

		if(!m->m_on)
			continue;

		if(stricmp(m->m_fullpath.c_str(), corrected) == 0)
			return i;
	}

	return -1;
}


int LoadModel(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue, bool blendnorm)
{
	int i = FindModel(relative);

	if(i >= 0)
		return i;

	i = NewModel();

	if(i < 0)
		return i;

	if(g_model[i].load(relative, scale, translate, dontqueue, blendnorm))
		return i;

	return -1;
}

bool Model::load(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue, bool blendnorm)
{
	m_diffusem = 0;
	m_specularm = 0;
	m_normalm = 0;
	m_ownerm = 0;

	m_scale = scale;
	m_translate = translate;

#if 0
	if(LoadDMDModel(&m_va, m_diffusem, m_specularm, m_normalm, m_ownerm, scale, translate, relative, dontqueue))
	{
		m_on = true;
		char full[DMD_MAX_PATH+1];
		FullPath(relative, full);
		char corrected[DMD_MAX_PATH+1];
		strcpy(corrected, full);
		CorrectSlashes(corrected);
		m_fullpath = corrected;
		m_nframes = 1;
		genvbo();
		return true;
	}
#endif
	if(m_ms3d.load(relative, m_diffusem, m_specularm, m_normalm, m_ownerm, scale, translate, dontqueue))
	{
		m_on = true;

		
	g_applog<<"load ms3d 7"<<std::endl;
	g_applog.flush();

		//m_ms3d.genva(&m_va, scale, translate, relative, blendnorm);
		char full[DMD_MAX_PATH+1];
		FullPath(relative, full);
		char corrected[DMD_MAX_PATH+1];
		strcpy(corrected, full);
		CorrectSlashes(corrected);
		m_fullpath = corrected;
		m_nframes = m_ms3d.m_totalFrames;

		
	g_applog<<"load ms3d 8"<<std::endl;
	g_applog.flush();

		genvbo();

		
	g_applog<<"load ms3d 9"<<std::endl;
	g_applog.flush();

		return true;
	}

	return false;
}

void Model::genvbo()
{
#if 0
	delvbo();

	for(int i=0; i<m_nframes; i++)
		m_va[i].genvbo();
#endif
}

void Model::delvbo()
{
#if 0
	for(int i=0; i<m_nframes; i++)
		m_va[i].delvbo();
#endif
}


void Model::usedifftex() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_diffusem ].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
}


void Model::usespectex() const
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_specularm ].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_SPECULARMAP], 1);
}

void Model::usenormtex() const
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_normalm ].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_NORMALMAP], 2);
}

void Model::useteamtex() const
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_ownerm ].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_OWNERMAP], 3);
}


#if 1
bool PlayAnif(float& frame, int first, int last, bool loop, float rate)
{
	if(frame < first || frame > last+1)
	{
		frame = first;
		return false;
	}

	frame += rate;

	if(frame > last)
	{
		if(loop)
			frame = first;
		else
			frame = last;

		return true;
	}

	return false;
}

//Play animation backwards
bool PlayAniBf(float& frame, int first, int last, bool loop, float rate)
{
	if(frame < first-1 || frame > last)
	{
		frame = last;
		return false;
	}

	frame -= rate;

	if(frame < first)
	{
		if(loop)
			frame = last;
		else
			frame = first;

		return true;
	}

	return false;
}
#endif

void FreeModels()
{
	for(int i=0; i<MODELS; i++)
	{
		Model* m = &g_model[i];

		if(!m->m_on)
			continue;

		m->destroy();
	}
}
