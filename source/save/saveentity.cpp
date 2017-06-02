

#include "saveentity.h"
#include "../sim/entity.h"
#include "../app/appmain.h"
#include "../utils.h"
#include "../debug.h"

//read entity property
void ReadEProp(Ent* e, FILE* fp)
{
	int ptype = -1;
	int psize = -1;
	char* pdata = NULL;

	fread(&ptype, sizeof(int), 1, fp);
	fread(&psize, sizeof(int), 1, fp);

	pdata = (char*)malloc(psize);

	if(!pdata)
		OUTOFMEM();

	fread(pdata, sizeof(char), psize, fp);

	switch(ptype)
	{
	case EPROP_CAM:
		e->camera = *(Camera*)pdata;
		break;
	case EPROP_ETYPE:
		e->type = *(int*)pdata;
		break;
	default:
		g_applog<<"Unrecognized entity property #"<<ptype<<" for entity #"<<(e-g_entity)<<std::endl;
		g_applog.flush();
		break;
	}

	free(pdata);
}

void SaveEProp(int ptype, Ent* e, FILE* fp)
{
	int psize = 0;
	char* pdata = NULL;
	
	fwrite(&ptype, sizeof(int), 1, fp);

	switch(ptype)
	{
	case EPROP_CAM:
		psize = sizeof(Camera);
		fwrite(&psize, sizeof(int), 1, fp);
		fwrite(&e->camera, sizeof(Camera), 1, fp);
		break;
	case EPROP_ETYPE:
		psize = sizeof(int);
		fwrite(&psize, sizeof(int), 1, fp);
		fwrite(&e->type, sizeof(int), 1, fp);
		break;
	default:
		g_applog<<"Unrecognized entity property #"<<ptype<<" for entity #"<<(e-g_entity)<<std::endl;
		g_applog.flush();
		break;
	}
}

void SaveEntities(FILE* fp)
{
	for(int i=0; i<ENTITIES; i++)
	{
		Ent* e = &g_entity[i];

		fwrite(&e->on, sizeof(bool), 1, fp);

		if(!e->on)
			continue;

		int numprops = EPROP_TYPES;

		fwrite(&numprops, sizeof(int), 1, fp);
		
		SaveEProp(EPROP_ETYPE, e, fp);
		SaveEProp(EPROP_CAM, e, fp);
	}
}

void ReadEntities(FILE* fp)
{
	for(int i=0; i<ENTITIES; i++)
	{
		Ent* e = &g_entity[i];

		fread(&e->on, sizeof(bool), 1, fp);

		if(!e->on)
			continue;

		int numprops = 0;

		fread(&numprops, sizeof(int), 1, fp);

		for(int pi=0; pi<numprops; pi++)
		{
			ReadEProp(e, fp);
		}
	}
}