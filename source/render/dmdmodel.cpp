#include "dmdmodel.h"
#include "../texture.h"
#include "../utils.h"
#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"

bool LoadDMDModel(VertexArray** va, unsigned int& diffusem, unsigned int& specularm, unsigned int& normalm, unsigned int& ownerm, Vec3f scale, Vec3f translate, const char* relative, bool dontqueue)
{
	char fullpath[DMD_MAX_PATH+1];
	FullPath(relative, fullpath);
	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
		return false;

	char realtag[] = TAG_BUILDINGM;
	char tag[5];
	fread(tag, sizeof(char), 5, fp);

	if(memcmp(tag, realtag, sizeof(char)*5) != 0)
		return false;

	float realversion = BUILDINGM_VERSION;
	float version;
	fread(&version, sizeof(float), 1, fp);

	if(version != realversion)
		return false;

	(*va) = new VertexArray[1];
	int numv;

	fread(&numv, sizeof(int), 1, fp);
	(*va)[0].alloc(numv);

	fread((*va)[0].vertices, sizeof(Vec3f), numv, fp);
	fread((*va)[0].texcoords, sizeof(Vec2f), numv, fp);
	fread((*va)[0].normals, sizeof(Vec3f), numv, fp);

	fclose(fp);

	for(int i=0; i<numv; i++)
		(*va)->vertices[i] = (*va)->vertices[i] * scale + translate;

#if 0
	g_applog<<"numv "<<numv<<std::endl;

	for(int i=0; i<numv; i++)
	{
		g_applog<<"\t"<<(*va)[0].vertices[i].x<<","<<(*va)[0].vertices[i].y<<","<<(*va)[0].vertices[i].z<<std::endl;
	}
#endif

	g_applog<<relative<<std::endl;

	char diffpath[DMD_MAX_PATH+1];
	char diffpathpng[DMD_MAX_PATH+1];
	char specpath[DMD_MAX_PATH+1];
	char normpath[DMD_MAX_PATH+1];
	char ownpath[DMD_MAX_PATH+1];

	DiffPath(relative, diffpath);
	DiffPathPNG(relative, diffpathpng);
	SpecPath(relative, specpath);
	NormPath(relative, normpath);
	OwnPath(relative, ownpath);

	bool isdiffpng = false;
	fp = NULL;
	fp = fopen(diffpathpng, "rb");
	if(fp)
	{
		isdiffpng = true;
		fclose(fp);
	}

	if(dontqueue)
	{
		if(isdiffpng)
			CreateTex(diffusem, diffpathpng, true, true);
		else
			CreateTex(diffusem, diffpath, true, true);
		//CreateTex(specularm, specpath, true, true);
		//CreateTex(normalm, normpath, true, true);
		CreateTex(ownerm, ownpath, true, true);
	}
	else
	{
		if(isdiffpng)
			QueueTexture(&diffusem, diffpathpng, true, true);
		else
			QueueTexture(&diffusem, diffpath, true, true);
		//QueueTexture(&specularm, specpath, true, true);
		//QueueTexture(&normalm, normpath, true, true);
		QueueTexture(&ownerm, ownpath, true, true);
	}

	return true;
}
