
#include "../app/appmain.h"
#include "decal.h"
#include "../texture.h"
#include "../gui/gui.h"
#include "../save/saveedm.h"

CDecalType g_decalT[DECAL_TYPES];
CDecal g_decal[DECALS];

void Decal(int type, char* tex, float decay, float size)
{
	CDecalType* t = &g_decalT[type];

	CreateTex(t->tex, tex, true, true);
	t->decay = decay;
	t->size = size;
}

void Decals()
{
	Decal(BLOODSPLAT, "billboards/bloodsplat.png", 0.01f, 60.0f);
	Decal(BULLETHOLE, "billboards/bullethole.png", 0.005f, 3.0f);
}

void UpdateDecals()
{
	CDecal* d;
	CDecalType* t;

	for(int i=0; i<DECALS; i++)
	{
		d = &g_decal[i];

		if(!d->on)
			continue;

		t = &g_decalT[d->type];
		
		d->life -= t->decay;

		if(d->life < 0.0f)
			d->on = false;
	}
}

void DrawDecals()
{
	CDecal* d;
	CDecalType *t;
	
	Shader* s = &g_shader[g_curS];

	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);

	for(int i=0; i<DECALS; i++)
    {
        d = &g_decal[i];
        if(!d->on)
            continue;
        
        t = &g_decalT[d->type];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_texture[t->tex].texname);
		glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
		
#ifdef DEBUG_2
		//Chat("drawdecals");
		g_debug2val = DEBUG2DRAWDECALS;
#endif

		float colorf[4];
		colorf[0] = 1;
		colorf[1] = 1;
		colorf[2] = 1;
		colorf[3] = d->life;
		glUniform4f(s->slot[SSLOT_COLOR], colorf[0], colorf[1], colorf[2], colorf[3]);
        
        float vertices[] =
        {
            //posx, posy posz   texx, texy
            d->a.x, d->a.y, d->a.z,          0, 1,
            d->b.x, d->b.y, d->b.z,          1, 1,
            d->c.x, d->c.y, d->c.z,          1, 0,
            
            d->a.x, d->a.y, d->a.z,          0, 1,
            d->c.x, d->c.y, d->c.z,          1, 0,
            d->d.x, d->d.y, d->d.z,          0, 0
        };
		
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_CULL_FACE);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int NewDecal()
{
	for(int i=0; i<DECALS; i++)
		if(!g_decal[i].on)
			return i;

	return -1;
}

void PlaceDecal(int type, Vec3f pos, Vec3f norm)
{
	int i = NewDecal();
	if(i < 0)
		return;

	CDecal* d = &g_decal[i];
	d->on = true;
	d->life = 1;
	d->type = type;

	CDecalType* t = &g_decalT[type];
	
	Vec3f cross = Normalize(Vec3f(rand()%300-150, rand()%10+1, rand()%300-150));

	Vec3f vertical = Normalize(Cross(norm, cross));
	Vec3f horizontal = Normalize(Cross(norm, vertical));

	Vec3f vert = vertical*t->size;
	Vec3f horiz = horizontal*t->size;
        
	d->lpos = pos + norm * 0.1f;
	d->a = pos - horiz + vert;
	d->b = pos + horiz + vert;
	d->c = pos + horiz - vert;
	d->d = pos - horiz - vert;
}