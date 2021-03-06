


#include "../platform.h"
#include "brush.h"
#include "../math/plane3f.h"
#include "../math/3dmath.h"
#include "../math/line.h"
#include "../math/polygon.h"
#include "../utils.h"
#include "../math/3dmath.h"
#include "../save/edmap.h"
#include "../save/edmap.h"
#include "collapsegeom.h"

//#define DEBUG_COLLAPSE

//collapse geometry into triangles
void Brush::collapse()
{
	colshv();
	colva();
	coloutl();
}

void Brush::collapse2()
{
	colshv();
	colva();
	coloutl();
}


/*
Get the outline lines to draw the orthographic representation of the brush.
Also gets the shared vertex indices.
Basically this is the stuff that's only necessary for the editor but not the game.
*/
void Brush::coloutl()
{
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		if(s->m_tris)
		{
			delete [] s->m_tris;
			s->m_tris = NULL;
		}
		s->m_ntris = 0;
	}
	
#if 0
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		s->m_ntris = s->m_drawva.numverts / 3;
		s->m_tris = new Triangle2 [s->m_ntris];

		for(int j=0; j<s->m_ntris; j++)
		{
			Triangle2* t = &s->m_tris[j];

			int vindex = j*3;
			
			t->m_texcoord[0] = s->m_drawva.texcoords[vindex + 0];
			t->m_texcoord[1] = s->m_drawva.texcoords[vindex + 1];
			t->m_texcoord[2] = s->m_drawva.texcoords[vindex + 2];
			
			t->m_vertex[0] = s->m_drawva.vertices[vindex + 0];
			t->m_vertex[1] = s->m_drawva.vertices[vindex + 1];
			t->m_vertex[2] = s->m_drawva.vertices[vindex + 2];
		}
	}
#endif
	
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		
#if 0
		// This depends on the order which triangles are made up out of 
		// vertices for the vertex array.
		s->m_outline.m_edv.push_back( s->m_drawva.vertices[0] );
		s->m_outline.m_edv.push_back( s->m_drawva.vertices[1] );
		s->m_outline.m_edv.push_back( s->m_drawva.vertices[2] );
		
		for(int j = 5; j<s->m_drawva.numverts; j+=3)
		{
			s->m_outline.m_edv.push_back( s->m_drawva.vertices[j] );
		}

		s->m_outline.makeva();

		Vec3f centroid(0,0,0);
		float count = 0;

		int jj=0;
		for(auto j=s->m_outline.m_edv.begin(); j!=s->m_outline.m_edv.end(); j++, jj++)
		{
			centroid = centroid * (count/(count+1)) + (*j) * (1.0f/(count+1));
#ifdef DEBUG_COLLAPSE
			g_applog<<"centroid "<<count<<" "<<centroid.x<<","<<centroid.y<<","<<centroid.z<<" *j "<<(*j).x<<","<<(*j).y<<","<<(*j).z<<std::endl;
			g_applog.flush();
#endif

			count += 1;

#if 0
			bool found = false;
			for(unsigned int k=0; k<b->m_sharedv.size(); k++)
			{
				if(Close(sharedv[k], *j))
				{
					found = true;
					s->m_vindices[jj] = k;
				}
			}

			if(!found)
			{
				sharedv.push_back(*j);
				s->m_vindices[jj] = sharedv.size()-1;
			}
#endif
		}
#else
		std::list<Vec3f> outline;

			// This depends on the order which triangles are made up out of 
		// vertices for the vertex array.
		outline.push_back( s->m_drawva.vertices[0] );
		outline.push_back( s->m_drawva.vertices[1] );
		outline.push_back( s->m_drawva.vertices[2] );
		
		for(int j = 5; j<s->m_drawva.numverts; j+=3)
		{
			outline.push_back( s->m_drawva.vertices[j] );
		}

		Vec3f centroid(0,0,0);
		float count = 0;

		for(auto j=outline.begin(); j!=outline.end(); j++)
		{
			centroid = centroid * (count/(count+1)) + (*j) * (1.0f/(count+1));

			count += 1;
		}

#endif

		s->m_centroid = centroid;
	}
}

//try to simplify a brush until it has only 10 sides
void Brush::trysimp()
{
	//we want to take out the sides that will cause the least volume increase.
	//possible measure of volume increase: number of side verts. the greater the number of side verts, the greater the detail in that region of the brush.
	//on second thought, no, i see where that could create an infinitely extended brush.

}

/*
Get the shared vertices from the plane intersections.
*/
void Brush::colshv()
{
	if(m_sharedv)
	{
		delete [] m_sharedv;
		m_sharedv = NULL;
	}
	m_nsharedv = 0;

	std::list<Vec3f> sharedv;
#if 0
	std::list<Vec3f>* sideverts;
	sideverts = new std::list<Vec3f>[m_nsides];
#else
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* a = &m_sides[i];
		a->m_sideverts.clear();
	}
#endif

#if 0
	if(m_nsides > 10)
		trysimp();

	static int bigns = 0;

	if(m_nsides > bigns)
	{
		bigns = m_nsides;
		g_applog<<"new nsides record: "<<bigns<<std::endl;
		g_applog.flush();
		//reached as high as 58 pretty quickly
	}
#endif

	//std::list<Line>* sideedges = new std::list<Line>[m_nsides];	// a line along a plane intersecting two other planes. both vertices form the edge of a polygon.

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* a = &m_sides[i];
		Vec3f pointonplanea = PointOnPlane(a->m_plane);	// arbitrary point on plane A

		for(int j=i+1; j<m_nsides; j++)
		//for(int j=0; j<m_nsides; j++)
		{
			//if(j == i)
			//	continue;
			
			BrushSide* b = &m_sides[j];
			
			// Get the point intersection between three planes, if there is one.
			// The intersection will be the line formed by the first two planes,
			// intersecting at a point with the third plane.
			Vec3f linedir = Cross(a->m_plane.m_normal, b->m_plane.m_normal);	// line parallel to plane A and B
			Vec3f linedir2 = Cross(linedir, a->m_plane.m_normal); // direction toward plane B, parallel to plane A
			
			// http://devmaster.net/forums/topic/8676-2-plane-intersection/page__view__findpost__p__47568
			Vec3f l0;	// random point on line 2

			if(!Intersection(pointonplanea, linedir2, b->m_plane, l0))
				continue;

			//for(int k=0; k<m_nsides; k++)
			for(int k=j+1; k<m_nsides; k++)
			{
				//if(k == i || k == j)
				//	continue;

				BrushSide* c = &m_sides[k];

				Vec3f finalpoint;

				if(!Intersection(l0, linedir, c->m_plane, finalpoint))
					continue;

				// Shared vertex must be within all other planes

				bool inall = true;
				
				for(int l=0; l<m_nsides; l++)
				{
					if(l == i || l == j || l == k)
						continue;

					BrushSide* d = &m_sides[l];
					
					//if(!PointOnOrBehindPlane(finalpoint, d->m_plane.m_normal, d->m_plane.m_d, CLOSE_EPSILON))
					if(!PointOnOrBehindPlane(finalpoint, d->m_plane.m_normal, d->m_plane.m_d, 0))
					{
						inall = false;
						break;
					}
				}
				
				if(!inall)
					continue;

				sharedv.push_back(finalpoint);
#if 1
				a->m_sideverts.push_back(finalpoint);
				b->m_sideverts.push_back(finalpoint);
				c->m_sideverts.push_back(finalpoint);
#else
				sideverts[i].push_back(finalpoint);
				sideverts[j].push_back(finalpoint);
				sideverts[k].push_back(finalpoint);
#endif
			}
		}
	}
		
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* a = &m_sides[i];
#if 1
		auto sv = &a->m_sideverts;
#else
		auto sv = &sideverts[i];
#endif

		if(sv->size() <= 0)
			continue;

		auto viter=sv->begin();
		a->m_centroid = *viter;
		viter++;
		for(; viter!=sv->end(); viter++)
			a->m_centroid = a->m_centroid + *viter;
			//a->m_centroid = ( a->m_centroid * j + *viter ) / (float)(j+1);

		a->m_centroid = a->m_centroid / (float)sv->size();

		//m_centroid = (m_centroid * i + a->m_centroid) / (i+1);
	}

	//delete [] sideverts;

	
	for(int i=m_nsides-1; i>=0; i--)
	{
		BrushSide* a = &m_sides[i];

		if(a->m_sideverts.size() == 0)
		{
			//g_applog<<"rem side "<<i<<std::endl;
			//g_applog.flush();
			removeside(i);
		}
	}

	m_nsharedv = sharedv.size();
	m_sharedv = new Vec3f[m_nsharedv];

	int i=0;
	for(auto viter=sharedv.begin(); viter!=sharedv.end(); viter++, i++)
		m_sharedv[i] = *viter;

	//delete [] sideedges;
}

/*
Get the vertex arrays for the sides. 
First make the sides bigger than they actually are,
then clip them by all the planes.
*/
void Brush::colva()
{
	if(m_nsharedv < 2)
		return;
	
	// Get the min and max bounding box for the brush.

	Vec3f vmin(m_sharedv[0]);
	Vec3f vmax(m_sharedv[0]);

	for(int i=1; i<m_nsharedv; i++)
	{
		Vec3f v = m_sharedv[i];

		if(v.x < vmin.x)
			vmin.x = v.x;
		if(v.y < vmin.y)
			vmin.y = v.y;
		if(v.z < vmin.z)
			vmin.z = v.z;

		if(v.x > vmax.x)
			vmax.x = v.x;
		if(v.y > vmax.y)
			vmax.y = v.y;
		if(v.z > vmax.z)
			vmax.z = v.z;
	}
	
	// Pad the bounding box

#if 1
	//unnecessary extra calculations when clipping sides
	vmin.x -= 1.0f;
	vmin.y -= 1.0f;
	vmin.z -= 1.0f;
	vmax.x += 1.0f;
	vmax.y += 1.0f;
	vmax.z += 1.0f;
#endif

	
		g_applog<<"vminx "<<vmin.x<<"\r\n";
		g_applog<<"vminy "<<vmin.y<<"\r\n";
		g_applog<<"vminz "<<vmin.z<<"\r\n";
		g_applog<<"vmaxx "<<vmax.x<<"\r\n";
		g_applog<<"vmaxy "<<vmax.y<<"\r\n";
		g_applog<<"vmaxz "<<vmax.z<<"\r\n";

	// Construct the side polys
	for(int i=0; i<m_nsides; i++)
	{
		
		g_applog<<"side "<<i<<"\r\n";

		BrushSide* s = &m_sides[i];

		Winding *wind = new Winding;

		BigQuad(wind, vmin, vmax, s);

#if 1
		/*
		Fragments the tris by all the other planes.
		*/
		for(int j=0; j<m_nsides; j++)
		{
			if(j == i)
				continue;

			BrushSide* cuts = &m_sides[j];

			Winding* newf = NULL;
			ChopFront(wind, &cuts->m_plane, &newf);
			//ChopBack(wind, &cuts->m_plane, &newf);
			delete wind;
			wind = newf;

			if(!wind)
				break;

			if(wind->points.size() < 3)
				break;
		}
#endif

		s->m_drawva.free();

		if(!wind)
			continue;

		if(wind->points.size() < 3)
		{
			delete wind;
			continue;
		}

		/*
		Assign the vertex positions, normals, texcoords.
		*/
		s->m_drawva.alloc( (wind->points.size()-2)*3 );

		//g_applog<<"colva "<<s->m_drawva.numverts<<std::endl;

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

		g_applog<<"wind "<<wind->points.size()<<" points\r\n";

		if(wind)
			delete wind;
	}
}


// ClipWinding in Quake-Tools / qutils / COMMON / POLYLIB.C
void ClipSide(Winding *in, Plane3f *split, Winding **front, Winding **back)
{
	std::vector<float> dists;
	std::vector<unsigned char> sides;
	int counts[3];
	
	counts[0] = counts[1] = counts[2] = 0;
	
	dists.reserve( in->points.size() + 1 );
	sides.reserve( in->points.size() + 1 );

	auto piter = in->points.begin();

	// determine sides for each point
	for(; piter != in->points.end(); piter++)
	{
		float dot = Dot(*piter, split->m_normal);
		dot += split->m_d;
		dists.push_back(dot);
		if (dot > ON_EPSILON)
			sides.push_back(SIDE_FRONT);
		else if (dot < -ON_EPSILON)
			sides.push_back(SIDE_BACK);
		else
			sides.push_back(SIDE_ON);


		counts[*sides.rbegin()]++;
	}

	if( !counts[SIDE_FRONT] && !counts[SIDE_BACK] && !counts[SIDE_ON] )
	{
		*front = NULL;
		*back = NULL;
		return;
	}

	sides.push_back(sides[0]);
	dists.push_back(dists[0]);

	if (!counts[SIDE_FRONT])
	{
		*front = NULL;
		*back = new Winding;
		**back = *in;
		return;
	}
	if (!counts[SIDE_BACK])
	{
		*front = new Winding;
		**front = *in;
		*back = NULL;
		return;
	}

	Winding* newf = *back = new Winding;
	Winding* new2 = *front = new Winding;

	piter = in->points.begin();
	auto siter = sides.begin();
	int i = 0;

	// distribute the points and generate splits
	for (; piter != in->points.end(); piter++, i++, siter++)
	{
		Vec3f p1 = *piter;

		if (*siter == SIDE_ON)
		{
			newf->points.push_back(p1);
			new2->points.push_back(p1);
			continue;
		}

		if (*siter == SIDE_FRONT)
			new2->points.push_back(p1);
		else if (*siter == SIDE_BACK)
			newf->points.push_back(p1);

		if (*(siter+1) == SIDE_ON || *(siter+1) == *siter)
			continue;

		// generate a split point
		Vec3f p2;
		if(i+1 >= in->points.size())
			p2 = *in->points.begin();
		else
		{
			auto piter2 = piter;
			piter2++;
			p2 = *piter2;
		}
		
		Vec3f mid;
		float dot = dists[i] / (dists[i]-dists[i+1]);
		for (int j=0; j<3; j++)
		{	
			// avoid round off error when possible
			if (split->m_normal[j] == 1)
				mid[j] = -split->m_d;
			else if (split->m_normal[j] == -1)
				mid[j] = split->m_d;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}

		newf->points.push_back(mid);
		new2->points.push_back(mid);
	}
}

/*
Returns only the front side of the winding.
*/
void ChopBack(Winding *in, Plane3f *split, Winding** out)
{
	Winding* back = NULL;
	Winding* front = NULL;

	ClipSide(in, split, &front, &back);

	if(back)
		delete back;

	*out = front;
}

/*
Returns only the back side of the winding.
*/
void ChopFront(Winding *in, Plane3f *split, Winding** out)
{
	Winding* back = NULL;
	Winding* front = NULL;

	ClipSide(in, split, &front, &back);

	if(front)
		delete front;

	*out = back;
}

/*
Form the first quad for the brush side that will be clipped by all the other planes.
*/
void BigQuad(Winding *wind, Vec3f vmin, Vec3f vmax, BrushSide* s)
{
	/*
	Get two axises to form a quad to fill the side.
	*/
	std::list<Vec3f> axises;
	axises.push_back(Vec3f(1,0,0));
	axises.push_back(Vec3f(0,1,0));
	axises.push_back(Vec3f(0,0,1));
	axises.push_back(Normalize(Vec3f(0,1,1)));
	axises.push_back(Normalize(Vec3f(1,0,1)));
	axises.push_back(Normalize(Vec3f(1,1,0)));
	axises.push_back(Normalize(Vec3f(1,1,1)));

	Vec3f forenorm = s->m_plane.m_normal;
	Vec3f backnorm = Vec3f(0,0,0) - s->m_plane.m_normal;

#if 0
#if 0
	Vec3f crossaxis1;

	for(auto aiter=axises.begin(); aiter!=axises.end(); aiter++)
	{
		if(!Close(forenorm, *aiter, 0.01f) && !Close(backnorm, *aiter, 0.01f))
		{
			crossaxis1 = *aiter;
			axises.erase( aiter );
			break;
		}
	}

	Vec3f crossaxis2;

	for(auto aiter=axises.begin(); aiter!=axises.end(); aiter++)
	{
		if(!Close(forenorm, *aiter, 0.01f) && !Close(backnorm, *aiter, 0.01f))
		{
			crossaxis2 = *aiter;
			break;
		}
	}
#else
	Vec3f crossaxis1;
	float farthestd = 0;
	auto farthestiter = axises.begin();
	for(auto aiter=axises.begin(); aiter!=axises.end(); aiter++)
	{
		float thisd = min(Magnitude2(forenorm - *aiter), Magnitude2(backnorm - *aiter));

		if(thisd > farthestd)
		{
			farthestiter = aiter;
		}
	}
	crossaxis1 = *farthestiter;
	axises.erase(farthestiter);
	
	Vec3f crossaxis2;
	farthestd = 0;
	for(auto aiter=axises.begin(); aiter!=axises.end(); aiter++)
	{
		float thisd = min(Magnitude2(forenorm - *aiter), Magnitude2(backnorm - *aiter));

		if(thisd > farthestd)
		{
			farthestiter = aiter;
		}
	}
	crossaxis2 = *farthestiter;
	axises.erase(farthestiter);
#endif

	Vec3f quadaxis1 = Cross(s->m_plane.m_normal, crossaxis1);
	Vec3f quadaxis2 = Cross(s->m_plane.m_normal, crossaxis2);
	//Vec3f quadaxis2 = Cross(quadaxis1, crossaxis2);
#else
#if 0
	Vec3f quadaxis1;
	auto q1iter = axises.begin();
	for(auto aiter=axises.begin(); aiter!=axises.end(); aiter++)
	{
		quadaxis1 = Cross(s->m_plane.m_normal, *aiter);
		float thisd = Magnitude2(quadaxis1);

		if(thisd >= 0.8f*0.8f)
		{
			q1iter = aiter;
			break;
		}
	}
	axises.erase(q1iter);
#else
	Vec3f quadaxis1;
	for(auto aiter=axises.begin(); aiter!=axises.end(); aiter++)
	{
		if(!Close(forenorm, *aiter, 0.01f) && !Close(backnorm, *aiter, 0.01f))
		{
			quadaxis1 = Cross(s->m_plane.m_normal, *aiter);
			break;
		}
	}
#endif

	Vec3f quadaxis2 = Cross(s->m_plane.m_normal, quadaxis1);
#endif

	SideCorners(wind, vmin, vmax, quadaxis1, quadaxis2, s);
}

/*
Find the maximum extent corners for the initial quad for a brush side.
*/
void SideCorners(Winding *wind, Vec3f vmin, Vec3f vmax, Vec3f quadaxis1, Vec3f quadaxis2, BrushSide* s)
{
	//Vec3f pop = PointOnPlane(s->m_plane);
	Vec3f pop = s->m_centroid;

	Vec3f min1min2 = pop;
	Vec3f min1max2 = pop;
	Vec3f max1max2 = pop;
	Vec3f max1min2 = pop;
	
#if 0
	SideCorner(min1min2, quadaxis1, vmin);
	SideCorner(min1min2, quadaxis2, vmin);
	
	SideCorner(min1max2, quadaxis1, vmin);
	SideCorner(min1max2, quadaxis2, vmax);
	
	SideCorner(max1max2, quadaxis1, vmax);
	SideCorner(max1max2, quadaxis2, vmax);
	
	SideCorner(max1min2, quadaxis1, vmax);
	SideCorner(max1min2, quadaxis2, vmin);
#elif 0
	SideCorner2(min1min2, quadaxis1, quadaxis2, vmin);
	SideCorner2(min1max2, quadaxis1, quadaxis2, vmin);
	SideCorner2(max1max2, quadaxis1, quadaxis2, vmax);
	SideCorner2(max1min2, quadaxis1, quadaxis2, vmax);
#elif 0
	SideCorner3(min1min2, &s->m_plane, vmin);
	SideCorner3(min1min2, &s->m_plane, vmin);
	
	SideCorner3(min1max2, &s->m_plane, vmin);
	SideCorner3(min1max2, &s->m_plane, vmax);
	
	SideCorner3(max1max2, &s->m_plane, vmax);
	SideCorner3(max1max2, &s->m_plane, vmax);
	
	SideCorner3(max1min2, &s->m_plane, vmax);
	SideCorner3(max1min2, &s->m_plane, vmin);
#elif 0
	SideCorner4(min1min2, min1max2, max1min2, max1max2, s, quadaxis1, quadaxis2);
#else
	//just make a big quad
	min1min2 = pop + quadaxis1 * -MAX_BRUSH + quadaxis2 * -MAX_BRUSH;
	min1max2 = pop + quadaxis1 * -MAX_BRUSH + quadaxis2 * MAX_BRUSH;
	max1min2 = pop + quadaxis1 * MAX_BRUSH + quadaxis2 * -MAX_BRUSH;
	max1max2 = pop + quadaxis1 * MAX_BRUSH + quadaxis2 * MAX_BRUSH;
#endif
	
	Vec3f foretri[] = {min1min2, min1max2, max1max2};

	Vec3f forenorm = Normal(foretri);
	Vec3f backnorm = Vec3f(0,0,0) - forenorm;
	
	float fored = Magnitude2(forenorm - s->m_plane.m_normal);
	float backd = Magnitude2(backnorm - s->m_plane.m_normal);

	// Do we need to reverse the vertex order?
	if(backd < fored)
	{
		wind->points.push_back(max1min2);
		wind->points.push_back(max1max2);
		wind->points.push_back(min1max2);
		wind->points.push_back(min1min2);
	}
	else
	{
		wind->points.push_back(min1min2);
		wind->points.push_back(min1max2);
		wind->points.push_back(max1max2);
		wind->points.push_back(max1min2);
	}
}

/*
Align a side corner next to a boundary.
*/
void SideCorner(Vec3f &corner, Vec3f quadaxis, Vec3f bound)
{
	float doff = 0;
	Vec3f dir = bound - corner;

	for(int i=0; i<3; i++)
	{
		if(fabs(quadaxis[i]) <= EPSILON)
			continue;

#if 0
		if(fabs(dir[i]) <= EPSILON)
		{
			doff = 0;
			break;
		}
#endif

		if(quadaxis[i] * dir[i] < 0.0f)
			quadaxis[i] = -quadaxis[i];

		//dir[i] *= quadaxis[i];	//should go in the same direction
		
		//float tempoff = (bound[i] - corner[i]) / quadaxis[i];
		float tempoff = dir[i] / quadaxis[i];
		
		//if(fabs(tempoff) > fabs(doff))
		//if(tempoff > doff)
		//if((dir[i] >= 0.0f && tempoff > doff) || (dir[i] < 0.0f && tempoff < doff))
		//if((dir[i] >= 0.0f && tempoff < doff) || (dir[i] < 0.0f && tempoff > doff))
		if(tempoff < doff || i == 0)
			doff = tempoff;
	}

	corner = corner + quadaxis * doff;
}

void SideCorner2(Vec3f &corner, Vec3f quadaxis1, Vec3f quadaxis2, Vec3f bound)
{
	//r1 * qa1 + r2 * qa2 = c
	
	float doff = 0;
	float doff2 = 0;
	Vec3f dir = bound - corner;

	for(int i=0; i<3; i++)
	{
		if(quadaxis2[i] * dir[i] < 0.0f)
			quadaxis2[i] = -quadaxis2[i];
	}

	for(int i=0; i<3; i++)
	{
		if(fabs(quadaxis1[i]) <= EPSILON)
			continue;

		if(quadaxis1[i] * dir[i] < 0.0f)
			quadaxis1[i] = -quadaxis1[i];

		float tempoff = dir[i] / quadaxis1[i];
		
		if(tempoff < doff || i == 0)
			doff = tempoff;
	}

	corner = corner + quadaxis1 * doff + quadaxis2 * doff2;
}

void SideCorner3(Vec3f &corner, Plane3f* pl, Vec3f bound)
{
	Vec3f l[3][2] =
		{
			{bound, bound+Vec3f(1,0,0)},
			{bound, bound+Vec3f(0,1,0)},
			{bound, bound+Vec3f(0,0,1)}
		};

	Vec3f interp[3];
	bool binter[3];

	for(int i=0; i<3; i++)
		binter[i] = RayInterPlane(l[i], pl->m_normal, pl->m_d, &interp[i]);

	float bestmag = -1.0f;
	Vec3f bestcorner;

	for(int i=0; i<3; i++)
	{
		if(!binter[i])
			continue;

		float mag = Magnitude2(corner - interp[i]);

		if(mag < bestmag || bestmag == -1.0f)
		{
			bestmag = mag;
			bestcorner = interp[i];
		}
	}

	corner = bestcorner;
}

//get max range from centroid to sidevert and make sidelength of poly quad that long using quadaxis1,2
void SideCorner4(Vec3f& min1min2, Vec3f& min1max2, Vec3f& max1min2, Vec3f& max1max2, BrushSide* s, Vec3f quadaxis1, Vec3f quadaxis2)
{
	for(auto viter=s->m_sideverts.begin(); viter!=s->m_sideverts.end(); viter++)
	{
	}
}