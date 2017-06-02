

#include "../platform.h"
#include "brushside.h"
#include "../math/plane3f.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../utils.h"
#include "brush.h"
#include "../save/compilemap.h"

BrushSide& BrushSide::operator=(const BrushSide &original)
{
	//g_applog<<"edbrushside assignment operator "<<std::endl;
	//g_applog.flush();

	m_plane = original.m_plane;
	m_diffusem = original.m_diffusem;
	m_specularm = original.m_specularm;
	m_normalm = original.m_normalm;
	//m_ownerm = original.m_ownerm;
	m_drawva = original.m_drawva;

	m_ntris = original.m_ntris;
	m_tceq[0] = original.m_tceq[0];
	m_tceq[1] = original.m_tceq[1];
	m_tris = NULL;
	if(m_ntris > 0)
	{
		m_tris = new Triangle2[m_ntris];
		if(original.m_tris)
			memcpy(m_tris, original.m_tris, sizeof(Triangle2)*m_ntris);
	}

	//g_applog<<"copy vindices m_ntris="<<m_ntris<<std::endl;
	//g_applog.flush();
	
	//g_applog<<"end copy vindices"<<std::endl;
	//g_applog.flush();

	m_centroid = original.m_centroid;
	m_sideverts = original.m_sideverts;

	return *this;
}

BrushSide::BrushSide(const BrushSide& original)
{
	//g_applog<<"edbrushside copy constructor"<<std::endl;
	
	m_ntris = 0;
	m_tris = NULL;
	m_tceq[0] = Plane3f(0.1f,0.1f,0.1f,0);
	m_tceq[1] = Plane3f(0.1f,0.1f,0.1f,0);
	m_diffusem = 0;
	m_centroid = Vec3f(0,0,0);
	*this = original;

	/*
	m_plane = original.m_plane;
	m_diffusem = original.m_diffusem;
	m_drawva = original.m_drawva;
	*/
	/*
	int m_ntris;
	Triangle2* m_tris;
	Plane3f m_tceq[2];	//tex coord uv equations
	*/
	/*
	m_ntris = original.m_ntris;
	m_tceq[0] = original.m_tceq[0];
	m_tceq[1] = original.m_tceq[1];
	m_tris = new Triangle2[m_ntris];
	memcpy(m_tris, original.m_tris, sizeof(Triangle2)*m_ntris);
	//for(int i=0; i<m_ntris; i++)
	//	m_tris[i] = original.m_tris[i];

	
	g_applog<<"copy edbrushside plane=n("<<m_plane.m_normal.x<<","<<m_plane.m_normal.y<<","<<m_plane.m_normal.z<<")d="<<m_plane.m_d<<std::endl;
	g_applog<<"\tueq=n("<<m_tceq[0].m_normal.x<<","<<m_tceq[0].m_normal.y<<","<<m_tceq[0].m_normal.z<<std::endl;
	g_applog<<"\tveq=n("<<m_tceq[1].m_normal.x<<","<<m_tceq[1].m_normal.y<<","<<m_tceq[1].m_normal.z<<std::endl;
	g_applog.flush();
	*/
}

BrushSide::BrushSide()
{
	//g_applog<<"edbrushside constructor default "<<std::endl;
	//g_applog.flush();

	m_ntris = 0;
	m_tris = NULL;
	m_tceq[0] = Plane3f(0.1f,0.1f,0.1f,0);
	m_tceq[1] = Plane3f(0.1f,0.1f,0.1f,0);
	m_diffusem = 0;
	m_centroid = Vec3f(0,0,0);
}

BrushSide::~BrushSide()
{
	//g_applog<<"edbrushsid destructor "<<std::endl;

	if(m_tris)
	{
		delete [] m_tris;
		m_tris = NULL;
	}

	m_ntris = 0;
}

Vec3f PlaneCrossAxis(Vec3f normal)
{
	float mag[6];
	mag[0] = Magnitude(normal - Vec3f(0,1,0));
	mag[1] = Magnitude(normal - Vec3f(0,-1,0));
	mag[2] = Magnitude(normal - Vec3f(1,0,0));
	mag[3] = Magnitude(normal - Vec3f(-1,0,0));
	mag[4] = Magnitude(normal - Vec3f(0,0,1));
	mag[5] = Magnitude(normal - Vec3f(0,0,-1));

	int match = 0;

	for(int i=0; i<6; i++)
	{
		if(mag[i] < mag[match])
			match = i;
	}

	//Vec3f vCross = Cross(m_view - m_pos, m_up);

	Vec3f crossaxis[6];
	crossaxis[0] = Vec3f( 0, 0, -1 );
	crossaxis[1] = Vec3f( 0, 0, 1 );
	crossaxis[2] = Vec3f( 0, 1, 0 );
	crossaxis[3] = Vec3f( 0, 1, 0 );
	crossaxis[4] = Vec3f( 0, 1, 0 );
	crossaxis[5] = Vec3f( 0, 1, 0 );

	return crossaxis[match];
}

void BrushSide::gentexeq()
{	
	Vec3f uaxis = Normalize(Cross(PlaneCrossAxis(m_plane.m_normal), m_plane.m_normal)) / STOREY_HEIGHT;
	Vec3f vaxis = Normalize(Cross(uaxis, m_plane.m_normal)) / STOREY_HEIGHT;
	
	m_tceq[0] = Plane3f(uaxis.x, uaxis.y, uaxis.z, 0);
	m_tceq[1] = Plane3f(vaxis.x, vaxis.y, vaxis.z, 0);
}

//#define FITTEX_DEBUG

void BrushSide::fittex()
{
	//Vec3f uaxis = Normalize(Cross(PlaneCrossAxis(m_plane.m_normal), m_plane.m_normal)) / STOREY_HEIGHT;
	//Vec3f vaxis = Normalize(Cross(uaxis, m_plane.m_normal)) / STOREY_HEIGHT;
	
#ifdef FITTEX_DEBUG
	g_applog<<"fittex 1"<<std::endl;
	g_applog.flush();
#endif

	Vec3f texaxis[2];
	texaxis[0] = Normalize(m_tceq[0].m_normal);
	texaxis[1] = Normalize(m_tceq[1].m_normal);

#ifdef FITTEX_DEBUG
	g_applog<<"fittex 2"<<std::endl;
	g_applog.flush();
#endif

	Vec3f newaxis[2];
	float closestmag[] = {-1, -1};
	Vec3f startv[2];

	//get the longest side edges and use those as axises for aligning the texture.

	//for(int i=0; i<m_outline.m_edv.size(); i++)
	//for(auto viter=m_sideverts.begin(); viter!=m_sideverts.end(); viter++)
	for(int i=0; i<m_drawva.numverts; i = (i==0) ? 1 : (i+2))
	{
#ifdef FITTEX_DEBUG
	//g_applog<<"fittex 3 vertex="<<i<<"/"<<m_outline.m_edv.size()<<std::endl;
	g_applog<<"fittex 3 vertex="<<i<<"/"<<m_drawva.numverts<<std::endl;
	g_applog.flush();
#endif
	
		//Vec3f thisv = m_outline.m_drawoutva[i];
		Vec3f thisv = m_drawva.vertices[i];
		Vec3f nextv;
		
		//if(i+1 < m_outline.m_edv.size())
		//if(i+1 < m_outline.m_edv.size())
		//	nextv = m_outline.m_drawoutva[i+1];

		int nexti = (i==0) ? 1 : (i+2);

		if(nexti < m_drawva.numverts)
			nextv = m_drawva.vertices[nexti];
		else
			//nextv = m_outline.m_drawoutva[0];
			nextv = m_drawva.vertices[0];

		for(int j=0; j<2; j++)
		{
			Vec3f thisaxis = Normalize( nextv - thisv );
			float mag = Magnitude(thisaxis - texaxis[j]);

			if(mag < closestmag[j] || closestmag[j] < 0)
			{
				closestmag[j] = mag;
				newaxis[j] = thisaxis;
				startv[j] = thisv;
			}

			thisaxis = Vec3f(0,0,0) - thisaxis;
			mag = Magnitude(thisaxis - texaxis[j]);

			if(mag < closestmag[j] || closestmag[j] < 0)
			{
				closestmag[j] = mag;
				newaxis[j] = thisaxis;
				startv[j] = nextv;
			}
		}
	}
	
#ifdef FITTEX_DEBUG
	g_applog<<"fittex 4"<<std::endl;
	g_applog.flush();
#endif

	float mind[2];
	float maxd[2];

	//for(int i=0; i<m_outline.m_edv.size(); i++)
	for(int i=0; i<m_drawva.numverts; i = (i==0) ? 1 : (i+2))
	{
		Vec3f thisv = m_drawva.vertices[i];

		for(int j=0; j<2; j++)
		{
			//float thisd = Dot( m_outline.m_drawoutva[i], newaxis[j] );
			float thisd = Dot( thisv, newaxis[j] );

			if(thisd < mind[j] || i == 0)
			{
				mind[j] = thisd;
			}

			if(thisd > maxd[j] || i == 0)
			{
				maxd[j] = thisd;
			}
		}
	}

#ifdef FITTEX_DEBUG
	g_applog<<"fittex 5"<<std::endl;
	g_applog.flush();
#endif

	for(int i=0; i<2; i++)
	{
		float span = maxd[i] - mind[i];
		m_tceq[i].m_normal = newaxis[i] / span;
		m_tceq[i].m_d = PlaneDistance(m_tceq[i].m_normal, startv[i]);
	}

#ifdef FITTEX_DEBUG
	g_applog<<"fittex 6"<<std::endl;
	g_applog.flush();
#endif

	remaptex();

#ifdef FITTEX_DEBUG
	g_applog<<"fittex 7"<<std::endl;
	g_applog.flush();
#endif
}

void BrushSide::remaptex()
{
#if 0
	Vec3f* un = &m_tceq[0].m_normal;
	Vec3f* vn = &m_tceq[1].m_normal;
	float ud = m_tceq[0].m_d;
	float vd = m_tceq[1].m_d;

	for(int j=0; j<m_ntris; j++)
	{
		Triangle2* t = &m_tris[j];

		for(int k=0; k<3; k++)
		{
			Vec3f* v = &t->m_vertex[k];
			Vec2f* tc = &t->m_texcoord[k];
				
			tc->x = un->x*v->x + un->y*v->y + un->z*v->z + ud;
			tc->y = vn->x*v->x + vn->y*v->y + vn->z*v->z + vd;
				
			//g_applog<<"-----------rebldg tex side"<<i<<" tri"<<j<<" vert"<<k<<"------------"<<std::endl;
			//g_applog<<"remaptex u = "<<un->x<<"*"<<v->x<<" + "<<un->y<<"*"<<v->y<<" + "<<un->z<<"*"<<v->z<<" + "<<ud<<" = "<<tc->x<<std::endl;
			//g_applog<<"remaptex v = "<<vn->x<<"*"<<v->x<<" + "<<vn->y<<"*"<<v->y<<" + "<<vn->z<<"*"<<v->z<<" + "<<vd<<" = "<<tc->y<<std::endl;
		}

		//for(int j=0; j<va->numverts; j++)
		for(int j=0; j<3; j++)
		{
			Vec2f* tc = &t->m_texcoord[j];
			//g_applog<<"u "<<va->texcoords[j].x<<"\t	v "<<va->texcoords[j].y<<std::endl;
			//g_applog<<"u "<<tc->x<<"\t	v "<<tc->y<<std::endl;
			//g_applog.flush();
		}
	}

	makeva();
#elif 0

	auto piter = wind->points.begin();
		Vec3f first = *piter;
		Vec2f firsttc;
		firsttc.x = first.x * s->m_tceq[0].m_normal.x + first.y * s->m_tceq[0].m_normal.y + first.z * s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
		firsttc.y = first.x * s->m_tceq[1].m_normal.x + first.y * s->m_tceq[1].m_normal.y + first.z * s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;

		piter++;
		Vec3f prev = *piter;
		Vec2f prevtc;
		prevtc.x = prev.x * s->m_tceq[0].m_normal.x + prev.y * s->m_tceq[0].m_normal.y + prev.z * s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
		prevtc.y = prev.x * s->m_tceq[1].m_normal.x + prev.y * s->m_tceq[1].m_normal.y + prev.z * s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;

		int triindex = 0;
		piter++;

		do
		{
			Vec3f curr = *piter;
			Vec2f currtc;
			currtc.x = curr.x * s->m_tceq[0].m_normal.x + curr.y * s->m_tceq[0].m_normal.y + curr.z * s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			currtc.y = curr.x * s->m_tceq[1].m_normal.x + curr.y * s->m_tceq[1].m_normal.y + curr.z * s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			
			s->m_drawva.vertices[triindex*3 + 0] = first;
			s->m_drawva.vertices[triindex*3 + 1] = prev;
			s->m_drawva.vertices[triindex*3 + 2] = curr;
			
#if 1
			//texcoords will be applied in remaptex
			s->m_drawva.texcoords[triindex*3 + 0] = firsttc;
			s->m_drawva.texcoords[triindex*3 + 1] = prevtc;
			s->m_drawva.texcoords[triindex*3 + 2] = currtc;
#endif

			s->m_drawva.normals[triindex*3 + 0] = s->m_plane.m_normal;
			s->m_drawva.normals[triindex*3 + 1] = s->m_plane.m_normal;
			s->m_drawva.normals[triindex*3 + 2] = s->m_plane.m_normal;
			
			prev = curr;
			prevtc = currtc;

			triindex++;
			piter++;
		}while(piter != wind->points.end());
#else
	Vec3f* un = &m_tceq[0].m_normal;
	Vec3f* vn = &m_tceq[1].m_normal;
	float ud = m_tceq[0].m_d;
	float vd = m_tceq[1].m_d;

	for(int i=0; i<m_drawva.numverts; i++)
	{
		Vec3f* v = &m_drawva.vertices[i];
		Vec2f* tc = &m_drawva.texcoords[i];
				
		tc->x = un->x*v->x + un->y*v->y + un->z*v->z + ud;
		tc->y = vn->x*v->x + vn->y*v->y + vn->z*v->z + vd;
	}

#endif
}

BrushSide::BrushSide(Vec3f normal, Vec3f point)
{
	m_ntris = 0;
	m_tris = NULL;
	//m_tceq[0] = Plane3f(1,1,1,0);
	//m_tceq[1] = Plane3f(1,1,1,0);
	m_diffusem = 0;
	m_plane = Plane3f(normal.x, normal.y, normal.z, PlaneDistance(normal, point));

	gentexeq();

	//g_applog<<"new edbrushside plane=n("<<m_plane.m_normal.x<<","<<m_plane.m_normal.y<<","<<m_plane.m_normal.z<<")d="<<m_plane.m_d<<std::endl;
	//g_applog.flush();
	
	CreateTex(m_diffusem, "textures/notex.jpg", false, true);
	m_specularm = m_diffusem;
	m_normalm = m_diffusem;
	//CreateTex(m_ownerm, "textures/notex.team.png", false, true);
	//m_centroid = Vec3f(0,0,0);
}

void BrushSide::usedifftex()
{
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[m_diffusem].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
}

void BrushSide::usespectex()
{
	glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[m_specularm].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_SPECULARMAP], 1);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
}

void BrushSide::usenormtex()
{
	glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[m_normalm].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_NORMALMAP], 2);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
}

void BrushSide::useteamtex()
{
#if 0
	glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[m_ownerm].texname);
	glUniform1i(g_shader[g_curS].slot[SSLOT_OWNERMAP], 3);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
#endif
}

/*
void BrushSide::usetex()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[m_diffusem].tex);
	glUniform1i(g_shader[g_curS].slot[SSLOT_TEXTURE0], 0);
}*/

void BrushSide::makeva()
{
	m_drawva.alloc(m_ntris * 3);

	//Vec3f tangent = Normalize( Cross(m_plane.m_normal, Normalize(m_tris[0].m_vertex[0] - m_tris[0].m_vertex[1])) );

	for(int i=0; i<m_ntris; i++)
	{
		for(int j=0; j<3; j++)
		{
			m_drawva.normals[i*3+j] = m_plane.m_normal;
			m_drawva.vertices[i*3+j] = m_tris[i].m_vertex[j];
			m_drawva.texcoords[i*3+j] = m_tris[i].m_texcoord[j];
			//m_drawva.tangents[i*3+j] = Normalize(m_tceq[0].m_normal);
			//m_drawva.tangents[i*3+j] = tangent;

			//g_applog<<"makeva uv="<<m_drawva.texcoords[i*3+j].x<<","<<m_drawva.texcoords[i*3+j].y<<std::endl;
		}
	}
}

//Remake the drawable vertex array from a std::list of
//triangles in the CutBrushSide class
void BrushSide::vafromcut(CutBrushSide* cutside)
{
	m_drawva.free();

	int ntris = cutside->m_frag.size();
	m_drawva.alloc(ntris*3);

	int triidx = 0;
	for(auto triitr=cutside->m_frag.begin(); triitr!=cutside->m_frag.end(); triitr++, triidx++)
	{
		for(int vertidx=0; vertidx<3; vertidx++)
		{
			m_drawva.vertices[triidx*3+vertidx] = triitr->m_vertex[vertidx];
			m_drawva.normals[triidx*3+vertidx] = m_plane.m_normal;

			//Reconstruct the texture coordinate according 
			//to the plane equation of the brush side

			Vec3f vert = m_drawva.vertices[triidx*3+vertidx];

			m_drawva.texcoords[triidx*3+vertidx].x 
				= m_tceq[0].m_normal.x * vert.x
				+ m_tceq[0].m_normal.y * vert.y
				+ m_tceq[0].m_normal.z * vert.z
				+ m_tceq[0].m_d;
			
			m_drawva.texcoords[triidx*3+vertidx].y
				= m_tceq[1].m_normal.x * vert.x
				+ m_tceq[1].m_normal.y * vert.y
				+ m_tceq[1].m_normal.z * vert.z
				+ m_tceq[1].m_d;
		}
	}
}