

#include "../save/edmap.h"
#include "save.h"
#include "../platform.h"
#include "../math/polygon.h"
#include "../texture.h"
#include "../render/vertexarray.h"
#include "../utils.h"
#include "saveedm.h"
#include "../save/edmap.h"
#include "../gui/layouts/segui.h"
#include "../sim/entity.h"
#include "../ai/ai.h"
#include "saveentity.h"

#if 0
void SaveEdBrushSideOLD(FILE* fp, BrushSide* s, int* texrefs)
{
	fwrite(&s->m_plane, sizeof(Plane), 1, fp);

	SaveVertexArray(fp, &s->m_drawva);

	fwrite(&texrefs[ s->m_diffusem ], sizeof(int), 1, fp);
	fwrite(&s->m_ntris, sizeof(int), 1, fp);
	fwrite(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fwrite(s->m_tceq, sizeof(Plane), 2, fp);

	SavePolygon(fp, &s->m_outline);

	fwrite(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);
	fwrite(&s->m_centroid, sizeof(Vec3f), 1, fp);
}
#endif

void SaveEdBrushSide(FILE* fp, BrushSide* s, int* texrefs)
{
	fwrite(&s->m_plane, sizeof(Plane3f), 1, fp);

	SaveVertexArray(fp, &s->m_drawva);

	fwrite(&texrefs[ s->m_diffusem ], sizeof(int), 1, fp);
	fwrite(&s->m_ntris, sizeof(int), 1, fp);
	fwrite(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fwrite(s->m_tceq, sizeof(Plane3f), 2, fp);
	
	//fwrite(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);
	fwrite(&s->m_centroid, sizeof(Vec3f), 1, fp);

#if 1
	int nshv = s->m_sideverts.size();
	fwrite(&nshv, sizeof(int), 1, fp);
	for(auto shvi=s->m_sideverts.begin(); shvi!=s->m_sideverts.end(); shvi++)
	{
		fwrite(&*shvi, sizeof(Vec3f), 1, fp);
	}
#endif
}

void SaveEdBrushSides(FILE* fp, Brush* b, int* texrefs)
{
	int nsides = b->m_nsides;
	fwrite(&nsides, sizeof(int), 1, fp);

	for(int i=0; i<nsides; i++)
	{
		/*
	Plane3f m_plane;
	VertexArray m_drawva;
	unsigned int m_diffusem;
		*/
	/*
	int m_ntris;
	Triangle2* m_tris;
	Plane3f m_tceq[2];	//tex coord uv equations
	Polyg m_outline;
	int* m_vindices;	//indices into parent brush's shared vertex array
	Vec3f m_centroid;
	*/
		BrushSide* s = &b->m_sides[i];

		SaveEdBrushSide(fp, s, texrefs);
	}
}

#if 0
void SaveEdBrushSide(FILE* fp, BrushSide* s, int* texrefs)
{
	fwrite(&s->m_plane, sizeof(Plane3f), 1, fp);

	SaveVertexArray(fp, &s->m_drawva);

	fwrite(&texrefs[ s->m_diffusem ], sizeof(int), 1, fp);
	fwrite(&s->m_ntris, sizeof(int), 1, fp);
	fwrite(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fwrite(s->m_tceq, sizeof(Plane3f), 2, fp);
	
	//fwrite(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);
	fwrite(&s->m_centroid, sizeof(Vec3f), 1, fp);

#if 1
	int nshv = s->m_sideverts.size();
	fwrite(&nshv, sizeof(int), 1, fp);
	for(auto shvi=s->m_sideverts.begin(); shvi!=s->m_sideverts.end(); shvi++)
	{
		fwrite(&*shvi, sizeof(Vec3f), 1, fp);
	}
#endif
}
#endif

void ReadBrushSide(FILE* fp, BrushSide* s, TexRef* texrefs)
{
	fread(&s->m_plane, sizeof(Plane3f), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"s->m_plane = "<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<","<<s->m_plane.m_d<<std::endl;
	g_applog.flush();
#endif

	ReadVertexArray(fp, &s->m_drawva);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 1"<<std::endl;
	g_applog.flush();
#endif

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	s->m_diffusem = texrefs[ texrefindex ].diffindex;
	s->m_specularm = texrefs[ texrefindex ].specindex;
	s->m_normalm = texrefs[ texrefindex ].normindex;
	//s->m_ownerm = texrefs[ texrefindex ].ownindex;
	fread(&s->m_ntris, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 2"<<std::endl;
	g_applog.flush();
#endif

	s->m_tris = new Triangle2[ s->m_ntris ];
	fread(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fread(s->m_tceq, sizeof(Plane3f), 2, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 3"<<std::endl;
	g_applog.flush();
#endif

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 4"<<std::endl;
	g_applog.flush();
#endif
	
	//s->m_vindices = new int[ s->m_outline.m_edv.size() ];
	//fread(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);

	fread(&s->m_centroid, sizeof(Vec3f), 1, fp);

#if 1
	int nshv = 0;
	fread(&nshv, sizeof(int), 1, fp);
	for(int shvi=0; shvi<nshv; shvi++)
	{
		Vec3f shv;
		fread(&shv, sizeof(Vec3f), 1, fp);
		s->m_sideverts.push_back(shv);
	}
#endif
}

//#define LOADMAP_DEBUG
void ReadBrushSideOLD(FILE* fp, BrushSide* s, TexRef* texrefs)
{
#if 1
	ReadBrushSide(fp,s,texrefs);
	return;
#endif

	fread(&s->m_plane, sizeof(Plane3f), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"s->m_plane = "<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<","<<s->m_plane.m_d<<std::endl;
	g_applog.flush();
#endif

	ReadVertexArray(fp, &s->m_drawva);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 1"<<std::endl;
	g_applog.flush();
#endif

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	s->m_diffusem = texrefs[ texrefindex ].diffindex;
	s->m_specularm = texrefs[ texrefindex ].specindex;
	s->m_normalm = texrefs[ texrefindex ].normindex;
	fread(&s->m_ntris, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 2"<<std::endl;
	g_applog.flush();
#endif

	s->m_tris = new Triangle2[ s->m_ntris ];
	fread(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fread(s->m_tceq, sizeof(Plane3f), 2, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 3"<<std::endl;
	g_applog.flush();
#endif

	Polyg dump;
	ReadPolygon(fp, &dump);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 4"<<std::endl;
	g_applog.flush();
#endif

	//s->m_vindices = new int[ s->m_outline.m_edv.size() ];
	//fread(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);
	//fseek(fp, sizeof(int) * dump.m_edv.size(), SEEK_CUR);
	
	int* tempi = new int[ dump.m_edv.size() ];
	fread(tempi, sizeof(int), dump.m_edv.size(), fp);
	delete [] tempi;
	fread(&s->m_centroid, sizeof(Vec3f), 1, fp);

	dump.freeva();
}

void ReadBrushSides(FILE* fp, Brush* b, TexRef* texrefs)
{
	int nsides;
	fread(&nsides, sizeof(int), 1, fp);

	if(b->m_sides)
	{
		delete [] b->m_sides;
		b->m_sides = NULL;
		b->m_nsides = 0;
	}

	for(int i=0; i<nsides; i++)
	{
		BrushSide s;
		
		//ReadBrushSide(fp, &s, texrefs);
		ReadBrushSideOLD(fp, &s, texrefs);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 5"<<std::endl;
	g_applog.flush();
#endif

		b->add(s);

#ifdef LOADMAP_DEBUG
	g_applog<<"load ed brush side 6"<<std::endl;
	g_applog.flush();
#endif
	}

	b->collapse();
}

Brush* GetBrushNum(int target, EdMap* map)
{
	int cnt = 0;
	for(auto i=map->m_brush.begin(); i!=map->m_brush.end(); i++, cnt++)
	{
		if(cnt == target)
			return &*i;
	}

	return NULL;
}

void SaveEdDoor(FILE* fp, EdDoor* door, int* texrefs)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//open degrees
	bool startopen;
	Brush* brushp;
	Brush closedstate;*/

	fwrite(&door->axis, sizeof(Vec3f), 1, fp);
	fwrite(&door->point, sizeof(Vec3f), 1, fp);
	fwrite(&door->opendeg, sizeof(float), 1, fp);
	fwrite(&door->startopen, sizeof(bool), 1, fp);

	fwrite(&door->m_nsides, sizeof(int), 1, fp);
	for(int i=0; i<door->m_nsides; i++)
		SaveEdBrushSide(fp, &door->m_sides[i], texrefs);

#if 0
	g_applog<<"save ed door"<<std::endl;
	for(int i=0; i<door->m_nsides; i++)
	{
		g_applog<<"side "<<i<<std::endl;
		Plane3f* p = &door->m_sides[i].m_plane;

		g_applog<<"plane = "<<p->m_normal.x<<","<<p->m_normal.y<<","<<p->m_normal.z<<",d="<<p->m_d<<std::endl;
	}
#endif
}

void ReadEdDoor(FILE* fp, EdDoor* door, TexRef* texrefs)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//open degrees
	bool startopen;
	Brush* brushp;
	Brush closedstate;*/

#if 0
	MessageBox(g_hWnd, "read door", "aasd", NULL);
#endif

	fread(&door->axis, sizeof(Vec3f), 1, fp);
	fread(&door->point, sizeof(Vec3f), 1, fp);
	fread(&door->opendeg, sizeof(float), 1, fp);
	fread(&door->startopen, sizeof(bool), 1, fp);

	fread(&door->m_nsides, sizeof(int), 1, fp);


#if 0
	char msg[128];
	sprintf(msg, "door sides %d", door->m_nsides);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif

	door->m_sides = new BrushSide[door->m_nsides];
	for(int i=0; i<door->m_nsides; i++)
		//ReadBrushSide(fp, &door->m_sides[i], texrefs);
		ReadBrushSideOLD(fp, &door->m_sides[i], texrefs);

#if 0
	g_applog<<"read ed door"<<std::endl;
	for(int i=0; i<door->m_nsides; i++)
	{
		g_applog<<"side "<<i<<std::endl;
		Plane3f* p = &door->m_sides[i].m_plane;

		g_applog<<"plane = "<<p->m_normal.x<<","<<p->m_normal.y<<","<<p->m_normal.z<<",d="<<p->m_d<<std::endl;
	}
#endif
}

void SaveBrush(FILE* fp, int* texrefs, Brush* b)
{
	/*
		int m_nsides;
		BrushSide* m_sides;
		int m_nsharedv;
		Vec3f* m_sharedv;	//shared vertices array
	*/

	SaveEdBrushSides(fp, b, texrefs);
	fwrite(&b->m_nsharedv, sizeof(int), 1, fp);
	fwrite(b->m_sharedv, sizeof(Vec3f), b->m_nsharedv, fp);
	fwrite(&texrefs[b->m_texture], sizeof(int), 1, fp);

	bool hasdoor = false;

	if(b->m_door)
		hasdoor = true;

	fwrite(&hasdoor, sizeof(bool), 1, fp);

	if(hasdoor)
		SaveEdDoor(fp, b->m_door, texrefs);
}

void SaveBrushes(FILE* fp, int* texrefs, std::list<Brush>* brushes)
{
	int nbrush = brushes->size();
	fwrite(&nbrush, sizeof(int), 1, fp);

	for(auto b=brushes->begin(); b!=brushes->end(); b++)
	{
		SaveBrush(fp, texrefs, &*b);
	}
}

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b)
{
	ReadBrushSides(fp, b, texrefs);

	if(b->m_sharedv)
	{
		delete [] b->m_sharedv;
		b->m_sharedv = NULL;
		b->m_nsharedv = 0;
	}

	fread(&b->m_nsharedv, sizeof(int), 1, fp);
#ifdef LOADMAP_DEBUG
	g_applog<<"b.m_nsharedv = "<<b->m_nsharedv<<std::endl;
	g_applog.flush();
#endif

	b->m_sharedv = new Vec3f[ b->m_nsharedv ];
	fread(b->m_sharedv, sizeof(Vec3f), b->m_nsharedv, fp);

	//b.remaptex();	//comment this out

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	b->m_texture = texrefs[texrefindex].diffindex;

	bool hasdoor = false;
	fread(&hasdoor, sizeof(bool), 1, fp);

	if(hasdoor)
	{
		b->m_door = new EdDoor();
		ReadEdDoor(fp, b->m_door, texrefs);
	}
}

void ReadBrushes(FILE* fp, TexRef* texrefs, std::list<Brush>* brushes)
{
	int nbrush;
	fread(&nbrush, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"nbrush = "<<nbrush<<std::endl;
	g_applog.flush();
#endif

	for(int i=0; i<nbrush; i++)
	{
		Brush b;
		ReadBrush(fp, texrefs, &b);
		brushes->push_back(b);

#ifdef LOADMAP_DEBUG
	g_applog<<"added b"<<std::endl;
	g_applog.flush();
#endif
	}
}


// Compile a list of textures used by map brushes
// and save that table to file. Also, a list of
// texture references is set (texrefs) that
// indexes into the written texture table based
// on the diffuse texture index (which indexes into
// g_texture, the global texture array).
void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes)
{
	for(int i=0; i<TEXTURES; i++)
		texrefs[i] = -1;

	// the compiled index of textures used in the map
	std::list<TexRef> compilation;

	for(auto b=brushes.begin(); b!=brushes.end(); b++)
	{
		//List of sides we will check the textures of.
		//We make a list because we include not only
		//sides of the brush, but the door closed-state model
		//sides too, which might theoretically have different
		//textures if the door model/whatever wasn't updated.
		std::list<BrushSide*> sides;

		for(int i=0; i<b->m_nsides; i++)
			sides.push_back(&b->m_sides[i]);

		if(b->m_door)
		{
			EdDoor* door = b->m_door;

			for(int i=0; i<door->m_nsides; i++)
				sides.push_back(&door->m_sides[i]);
		}

		for(auto si=sides.begin(); si!=sides.end(); si++)
		{
			BrushSide* s = *si;

			bool found = false;
			for(auto j=compilation.begin(); j!=compilation.end(); j++)
			{
				if(j->diffindex == s->m_diffusem)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				Texture* t = &g_texture[s->m_diffusem];
				TexRef tr;
				tr.filepath = t->fullpath;
				tr.texname = t->texname;
				tr.diffindex = s->m_diffusem;
				compilation.push_back(tr);
			}
		}

		bool found = false;
		for(auto j=compilation.begin(); j!=compilation.end(); j++)
		{
			if(j->diffindex == b->m_texture)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			Texture* t = &g_texture[b->m_texture];
			TexRef tr;
			tr.filepath = t->fullpath;
			tr.texname = t->texname;
			tr.diffindex = b->m_texture;
			compilation.push_back(tr);
		}
	}

	//Write the texture table to file.
	int nrefs = compilation.size();
	fwrite(&nrefs, sizeof(int), 1, fp);

#if 0
	g_applog<<"writing "<<nrefs<<" tex refs"<<std::endl;
	g_applog.flush();
#endif

	int j=0;
	for(auto i=compilation.begin(); i!=compilation.end(); i++, j++)
	{
		texrefs[ i->diffindex ] = j;
		int strl = i->filepath.length()+1;
		fwrite(&strl, sizeof(int), 1, fp);

#if 0
		g_applog<<"writing "<<strl<<"-long tex ref"<<std::endl;
		g_applog.flush();
#endif

		fwrite(i->filepath.c_str(), sizeof(char), strl, fp);
	}
}

void ReadEdTexs(FILE* fp, TexRef** texrefs)
{
	int nrefs;
	fread(&nrefs, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"nrefs = "<<nrefs<<std::endl;
	g_applog.flush();
#endif

	(*texrefs) = new TexRef[nrefs];

	for(int i=0; i<nrefs; i++)
	{
		TexRef* tr = &(*texrefs)[i];
		int strl;
		fread(&strl, sizeof(int), 1, fp);

		char* filepath = new char[strl];
		fread(filepath, sizeof(char), strl, fp);
#ifdef LOADMAP_DEBUG
	g_applog<<"filepath = "<<filepath<<std::endl;
	g_applog.flush();
#endif

		if(strstr(filepath, "fonts/") ||
			strstr(filepath, ".spec.") ||
			strstr(filepath, ".norm."))
		{
			delete [] filepath;
			tr->filepath = "textures/notex.jpg";
			CreateTex(tr->diffindex, tr->filepath.c_str(), false, true);
			tr->texname = g_texture[tr->diffindex].texname;
			tr->specindex = tr->diffindex;
			tr->normindex = tr->diffindex;
			//tr->ownindex = g_texture[tr->diffindex].texname;
			continue;
		}

		tr->filepath = filepath;
		delete [] filepath;
		CreateTex(tr->diffindex, tr->filepath.c_str(), false, true);
		tr->texname = g_texture[tr->diffindex].texname;

		char basepath[DMD_MAX_PATH+1];
		strcpy(basepath, tr->filepath.c_str());
		StripExt(basepath);

		char specpath[DMD_MAX_PATH+1];
		SpecPath(basepath, specpath);

		CreateTex(tr->specindex, specpath, false, true);

		char normpath[DMD_MAX_PATH+1];
		NormPath(basepath, normpath);

		CreateTex(tr->normindex, normpath, false, true);

		char ownpath[DMD_MAX_PATH+1];
		OwnPath(basepath, ownpath);

		//CreateTex(tr->ownindex, ownpath, false, true);
	}
}

int BrushNum(Brush* b, EdMap* map)
{
	int cnt = 0;
	for(auto i=map->m_brush.begin(); i!=map->m_brush.end(); i++, cnt++)
	{
		if(&*i == b)
			return cnt;
	}

	return -1;
}

void SaveModelHolder(FILE* fp, ModelHolder* pmh)
{
	Model* m = &g_model[pmh->model];

	std::string relative = MakeRelative(m->m_fullpath.c_str());
	int nrelative = relative.size() + 1;

	fwrite(&nrelative, sizeof(int), 1, fp);
	fwrite(relative.c_str(), sizeof(char), nrelative, fp);

	fwrite(&pmh->rotdegrees, sizeof(Vec3f), 1, fp);
	fwrite(&pmh->translation, sizeof(Vec3f), 1, fp);
	fwrite(&pmh->scale, sizeof(Vec3f), 1, fp);
}

void ReadModelHolder(FILE* fp, ModelHolder* pmh)
{
	int nrelative =0;
	fread(&nrelative, sizeof(int), 1, fp);

	char* relative = new char[nrelative];
	fread(relative, sizeof(char), nrelative, fp);
	pmh->model = LoadModel(relative, Vec3f(1,1,1), Vec3f(0,0,0), true, true);
	delete [] relative;

	fread(&pmh->rotdegrees, sizeof(Vec3f), 1, fp);
	fread(&pmh->translation, sizeof(Vec3f), 1, fp);
	fread(&pmh->scale, sizeof(Vec3f), 1, fp);
}

void SaveModelHolders(FILE* fp, std::list<ModelHolder>& modelholders)
{
	int nmh = modelholders.size();

	fwrite(&nmh, sizeof(int), 1, fp);

	for(auto iter = modelholders.begin(); iter != modelholders.end(); iter++)
	{
		SaveModelHolder(fp, &*iter);
	}
}

void ReadModelHolders(FILE* fp, std::list<ModelHolder>& modelholders)
{
	int nmh = 0;

	fread(&nmh, sizeof(int), 1, fp);

	for(int i = 0; i < nmh; i++)
	{
		ModelHolder mh;
		ReadModelHolder(fp, &mh);
		mh.retransform();
		modelholders.push_back(mh);
	}
}

void ReadEnts(FILE* fp)
{
	//return;

	for(int i=0; i<ENTITIES; i++)
	{
#if 0
		bool on = false;

		fread(&on, sizeof(bool), 1, fp);

		if(!on)
			continue;

		Ent* e = g_entity[i] = new Ent;
#if 0
    float frame[2];
	int type;
	int controller;
	Camera camera;
	float amount;
	float clip;
	int state;
	int equipped;
	Vec3f goal;
	bool crouched;	//implies state
	bool crouching;	//implies intention
	bool forward;
	bool left;
	bool right;
	bool backward;
	bool jump;
	bool run;
	float stamina;
	VertexArray drawva;
#endif

		fread(e->frame, sizeof(float), 2, fp);
		fread(&e->type, sizeof(int), 1, fp);
		fread(&e->controller, sizeof(int), 1, fp);
		fread(&e->camera, sizeof(Camera), 1, fp);
		fread(&e->amount, sizeof(float), 1, fp);
		fread(&e->clip, sizeof(float), 1, fp);
		fread(&e->state, sizeof(int), 1, fp);
		fread(&e->equipped, sizeof(int), 1, fp);
		fread(&e->goal, sizeof(Vec3f), 1, fp);
		fread(&e->crouched, sizeof(bool), 1, fp);
		fread(&e->crouching, sizeof(bool), 1, fp);
		fread(&e->forward, sizeof(bool), 1, fp);
		fread(&e->left, sizeof(bool), 1, fp);
		fread(&e->right, sizeof(bool), 1, fp);
		fread(&e->backward, sizeof(bool), 1, fp);
		fread(&e->jump, sizeof(bool), 1, fp);
		fread(&e->run, sizeof(bool), 1, fp);
		fread(&e->stamina, sizeof(float), 1, fp);
	}

	for(int i=0; i<ENTITIES; i++)
	{
		Ent* e = g_entity[i];

		if(!e)
			continue;

		if(!IsActor(e))
			continue;

		e->frame[BODY_LOWER] = -1;
		AnimateEntity(e, i);
#endif
	}
}

void SaveEdMap(const char* fullpath, EdMap* map)
{
	FILE* fp = fopen(fullpath, "wb");

	char tag[] = TAG_EDMAP;
	fwrite(tag, sizeof(char), 5, fp);

	float version = EDMAP_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

	int texrefs[TEXTURES];
	SaveTexs(fp, texrefs, map->m_brush);

	int nframes = GetNumFrames();
	//fwrite(&nframes, sizeof(int), 1, fp);

	SaveBrushes(fp, texrefs, &map->m_brush);
	SaveModelHolders(fp, g_modelholder);
	SaveEntities(fp);

	fclose(fp);
}

void ScaleAll(float factor)
{
	EdMap* map = &g_edmap;

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		std::list<float> oldus;
		std::list<float> oldvs;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* s = &b->m_sides[i];

			Vec3f sharedv = *s->m_sideverts.begin();
			float u = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float v = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->m_tceq[0].m_normal = s->m_tceq[0].m_normal / factor;
			s->m_tceq[1].m_normal = s->m_tceq[1].m_normal / factor;
			Vec3f pop = PointOnPlane(s->m_plane);
			pop = pop * factor;
			s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);
		}

		b->collapse();

		auto oldu = oldus.begin();
		auto oldv = oldvs.begin();

		for(int i=0; i<b->m_nsides; i++, oldu++, oldv++)
		{
			BrushSide* s = &b->m_sides[i];

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

	char msg[128];
	sprintf(msg, "scaled %f", factor);
	InfoMess("asd", msg);
}

bool LoadEdMap(const char* fullpath, EdMap* map)
{
	FreeEdMap(map);

	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
		return false;

	char tag[5];
	fread(tag, sizeof(char), 5, fp);

	char realtag[] = TAG_EDMAP;
	//if(false)
	if(tag[0] != realtag[0] ||  tag[1] != realtag[1] || tag[2] != realtag[2] || tag[3] != realtag[3] || tag[4] != realtag[4])
	{
		fclose(fp);
		ErrMess("Error", "Not a map file (invalid header tag).");
		return false;
	}

	float version;
	fread(&version, sizeof(float), 1, fp);

	if(version != EDMAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		sprintf(msg, "Map version %f doesn't match %f.", version, EDMAP_VERSION);
		ErrMess("Error", msg);
		return false;
	}

//#ifdef LOADMAP_DEBUG
	g_applog<<"load map 1"<<std::endl;
	g_applog.flush();
//#endif

	TexRef* texrefs = NULL;

	ReadEdTexs(fp, &texrefs);

#if 1
	int nframes = 0;
	//fread(&nframes, sizeof(int), 1, fp);
	//SetNumFrames(nframes);
#endif

	ReadBrushes(fp, texrefs, &map->m_brush);
	ReadModelHolders(fp, g_modelholder);

	if(texrefs)
	{
		delete [] texrefs;
		texrefs = NULL;
	}

	ReadEntities(fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"load map 2"<<std::endl;
	g_applog.flush();
#endif

	fclose(fp);

#ifdef LOADMAP_DEBUG
	g_applog<<"load map 3"<<std::endl;
	g_applog.flush();
#endif

	//ScaleAll(1.75f);

	map->m_max = map->m_min = Vec3f(0,0,0);

	for(auto biter=map->m_brush.begin(); biter!=map->m_brush.end(); biter++)
	{
		for(int i=0; i<biter->m_nsharedv; i++)
		{
			Vec3f v = biter->m_sharedv[i];

			for(auto e=0; e<3; e++)
			{
				if(v[e] < map->m_min[e])
					map->m_min[e] = v[e];
				
				if(v[e] > map->m_max[e])
					map->m_max[e] = v[e];
			}
		}
	}

	return true;
}

void FreeEdMap(EdMap* map)
{
	g_sel1b = NULL;
	g_selB.clear();
	g_selE = NULL;
	g_dragV = -1;
	g_dragS = -1;
	g_dragD = -1;
	g_dragW = false;
	g_dragM = -1;
	g_selM.clear();

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* s = &b->m_sides[i];

			if(s->m_diffusem != 0)
				FreeTexture(s->m_diffusem);
			if(s->m_specularm != 0)
				FreeTexture(s->m_specularm);
			if(s->m_normalm != 0)
				FreeTexture(s->m_normalm);
			//if(s->m_ownerm != 0)
			//	FreeTexture(s->m_ownerm);
		}
	}

	map->m_brush.clear();

	//FreeModels();
	FreeModelHolders();

#if 0
	for(int i=0; i<ENTITIES; i++)
	{
		if(g_entity[i])
		{
			delete g_entity[i];
			g_entity[i] = NULL;
		}
	}
#endif
}
