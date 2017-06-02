

#include "../save/edmap.h"
#include "save.h"
#include "../platform.h"
#include "../math/polygon.h"
#include "../texture.h"
#include "../render/vertexarray.h"
#include "../utils.h"

//#define LOADMAP_DEBUG

void SavePolygon(FILE* fp, Polyg* p)
{
	/*
	std::list<Vec3f> m_vertex;	//used for constructing the polygon on-the-fly
	Vec3f* m_drawva;	//used for drawing outline
	*/

	int nvertex = p->m_edv.size();
	fwrite(&nvertex, sizeof(int), 1, fp);

	for(auto i=p->m_edv.begin(); i!=p->m_edv.end(); i++)
	{
		fwrite(&*i, sizeof(Vec3f), 1, fp);
	}

	fwrite(p->m_drawoutva, sizeof(Vec3f), nvertex, fp);
}

void ReadPolygon(FILE* fp, Polyg* p)
{
	int nvertex;
	fread(&nvertex, sizeof(int), 1, fp);

	for(int i=0; i<nvertex; i++)
	{
		Vec3f v;
		fread(&v, sizeof(Vec3f), 1, fp);
		p->m_edv.push_back(v);
	}

	p->m_drawoutva = new Vec3f[nvertex];
	fread(p->m_drawoutva, sizeof(Vec3f), nvertex, fp);
}

void SaveVertexArray(FILE* fp, VertexArray* va)
{
	/*
	int numverts;
	Vec3f* vertices;
	Vec2f* texcoords;
	Vec3f* normals;
	*/

	fwrite(&va->numverts, sizeof(int), 1, fp);
	fwrite(va->vertices, sizeof(Vec3f), va->numverts, fp);
	fwrite(va->texcoords, sizeof(Vec2f), va->numverts, fp);
	fwrite(va->normals, sizeof(Vec3f), va->numverts, fp);
	//fwrite(va->tangents, sizeof(Vec3f), va->numverts, fp);

#if 0
	g_applog<<"write VA"<<std::endl;
	for(int vertindex = 0; vertindex < va->numverts; vertindex++)
	{
		Vec3f vert = va->vertices[vertindex];
		g_applog<<"\twrite vert: "<<vert.x<<","<<vert.y<<","<<vert.z<<std::endl;
	}
#endif
}

void ReadVertexArray(FILE* fp, VertexArray* va)
{
	int nverts;
	fread(&nverts, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"nverts = "<<nverts<<std::endl;
	g_applog.flush();
#endif

	va->alloc(nverts);
	fread(va->vertices, sizeof(Vec3f), va->numverts, fp);
	fread(va->texcoords, sizeof(Vec2f), va->numverts, fp);
	fread(va->normals, sizeof(Vec3f), va->numverts, fp);
	//fread(va->tangents, sizeof(Vec3f), va->numverts, fp);
}

