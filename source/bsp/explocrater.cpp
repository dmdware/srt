

#include "../render/shader.h"
#include "../platform.h"
#include "../utils.h"
#include "../math/3dmath.h"
#include "../math/vec4f.h"
#include "../gui/gui.h"
#include "../math/plane3f.h"
#include "../sim/simdef.h"
#include "brush.h"
//#include "door.h"
#include "../save/compilemap.h"
//#include "../../app/segui.h"
//#include "../../app/sesim.h"
#include "../debug.h"
#include "explocrater.h"
#include "../render/particle.h"
#include "../gui/layouts/segui.h"
#include "../gui/layouts/sesim.h"

void MakeCrater(Vec3f center, std::list<Plane3f> &crater, float radius, int sides)
{
	BrushSide down(Vec3f(0.0f, -1.0f, 0.0f), center + Vec3f(0, -radius, 0));

	crater.push_back(down.m_plane);

	static int xoff = 13;
	static int yoff = 39;

	int oldxoff = xoff;
	xoff = (xoff + yoff + 1) % 113;
	yoff = (yoff + oldxoff + 1) % 98;

	for(int y=1; y<sides; y++)
		for(int x=1; x<sides; x++)
		{
			Plane3f transformed(down.m_plane);
			RotatePlane(transformed, center, DEGTORAD((360*y/(sides)+xoff)), Vec3f(0,0,1));
			RotatePlane(transformed, center, DEGTORAD((360*x/(sides)+yoff)), Vec3f(0,1,0));
			crater.push_back(transformed);
		}
}

bool BrushInCrater(Brush* b, std::list<Plane3f> &crater)
{
#if 1
	//if there's a plane that all of a brush's verts are in front of, then the brush is not in the crater.
	bool intersect = true;

	for(auto ep=crater.begin(); ep!=crater.end(); ep++)
	{
		bool outall = true;

		for(int bv=0; bv<b->m_nsharedv; bv++)
		{
			Vec3f& v = b->m_sharedv[bv];
			
			if(PointOnOrBehindPlane(v, ep->m_normal, ep->m_d, -CLOSE_EPSILON*2))
			//if(PointOnOrBehindPlane(v, ep->m_normal, ep->m_d, CLOSE_EPSILON*2))
			//if(PointOnOrBehindPlane(v, ep->m_normal, ep->m_d, 0))
			{
				outall = false;
				break;
			}
		}

		if(outall)
		{
			intersect = false;
			break;
		}
	}

	return intersect;
#else
	//if there's a plane that all of a brush's verts are in front of, then the brush is not in the crater.
	for(auto ep=crater.begin(); ep!=crater.end(); ep++)
	{
		bool outall = true;

		for(int bv=0; bv<b->m_nsharedv; bv++)
		{
			Vec3f v = b->m_sharedv[bv];

			if(!PointOnOrBehindPlane(v, ep->m_normal, ep->m_d, 0))
				continue;

			outall = false;
			break;
		}

		if(outall)
			return false;
	}

	return true;
#endif
}


void SubExplosion(EdMap* map, std::list<Plane3f> &crater)
{

	std::list<Brush> hitBs;

	//for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)

	auto b = map->m_brush.begin();

	while(b != map->m_brush.end())
	{
#if 0
		Texture* t = &g_texture[b->m_texture];

		if(t->sky)
			continue;
#endif

		bool intersect = BrushInCrater(&*b, crater);

		if(intersect)
		{
			hitBs.push_back(*b);
			b = map->m_brush.erase(b);
			continue;
		}

		b++;
	}

#if 0
	Brush craterb;

	for(auto pl=crater.begin(); pl!=crater.end(); pl++)
	{
		Vec3f point = PointOnPlane(*pl);
		BrushSide bs(pl->m_normal, point);
		craterb.add(bs);
	}

	craterb.collapse();

	g_edmap.m_brush.push_back(craterb);
#endif

	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;

	CreateTex(diffusem, "textures/destr/destruction.jpg", false, true);
	CreateTex(specularm, "textures/destr/destruction.spec.jpg", false, true);
	CreateTex(normalm, "textures/destr/destruction.norm.jpg", false, true);

#if 1

	for(auto b=hitBs.begin(); b!=hitBs.end(); b=hitBs.erase(b))
	{
		std::list<Brush> outfrags;
		std::list<Brush> infrags;
		
		infrags.push_back(*b);
		
		for(auto ep=crater.begin(); ep!=crater.end(); ep++)
		{
			Vec3f pop = PointOnPlane(*ep);
			std::list<Brush> nextoutfrags;
			std::list<Brush> nextinfrags;
			
			BrushSide news0(ep->m_normal, pop);
			BrushSide news1(Vec3f(0,0,0)-ep->m_normal, pop);
			
			news0.m_diffusem = diffusem;
			news1.m_diffusem = diffusem;
			news0.m_specularm = specularm;
			news1.m_specularm = specularm;
			news0.m_normalm = normalm;
			news1.m_normalm = normalm;
			
#if 1
			if(infrags.size() > 0)
			for(auto fragb=infrags.begin(); fragb!=infrags.end(); fragb++)
			{
				if(!BrushInCrater(&*fragb, crater))
				{
					nextoutfrags.push_back(*fragb);
					continue;
				}

#if 0
				if(fragb->m_nsides > 13)
				{
					//nextoutfrags.push_back(*fragb);
					continue;
				}
#endif
				
				//if any of the brush's verts are behind the plane (!PruneB2), 
				//we can make a new brush side with it.
				//if all the brush's verts are behind the plane (AllBeh),
				//we don't need to add that side.
				if(AllBeh(&*fragb, &news0.m_plane))
				{
					nextinfrags.push_back(*fragb);
					continue;
				}
				else if(!PruneB2(&*fragb, &news0.m_plane))
				{
					Brush newfrag0 = *fragb;
					newfrag0.add(news0);
#if 1
					//newfrag0.collapse();
					newfrag0.colshv();
					//newfrag0.remaptex();
#endif
#if 11
					nextinfrags.push_back(newfrag0);
#if 0
					auto j = nextinfrags.rbegin();
					if(PruneB(map, &*j))
						nextinfrags.erase(--(j.base()));
#endif
#endif
				}
				
#if 11
				//if any of the brush's verts are behind the plane (!PruneB2), 
				//we can make a new brush side with it.
				//if all the brush's verts are behind the plane (AllBeh),
				//we don't need to add that side.
				if(AllBeh(&*fragb, &news1.m_plane))
				{
					//we don't know yet if following crater planes
					//will be outside of brush or not, so
					//keep it in.
					nextinfrags.push_back(*fragb);
					//nextinfrags.push_back(*fragb);
					continue;
				}
				else if(!PruneB2(&*fragb, &news1.m_plane))
				{
					Brush newfrag1 = *fragb;
					newfrag1.add(news1);
#if 1
					//newfrag1.collapse();
					newfrag1.colshv();
					//newfrag1.remaptex();
#endif
					//nextoutfrags.push_back(newfrag1);
					outfrags.push_back(newfrag1);
#if 0
					auto j = nextoutfrags.rbegin();
					if(PruneB(map, &*j))
						nextoutfrags.erase(--(j.base()));
#endif
				}
#endif
			}
#endif

#if 0
			for(auto fragb=outfrags.begin(); fragb!=outfrags.end(); fragb++)
			{
				//if(!BrushInCrater(&*fragb, crater))
				{
					nextoutfrags.push_back(*fragb);
					continue;
				}

#if 0
				if(!PruneB2(&*fragb, &news0.m_plane))
				{
					Brush newfrag0 = *fragb;
					newfrag0.add(news0);
					newfrag0.collapse();
					newfrag0.remaptex();
					nextoutfrags.push_back(newfrag0);
					auto j = nextoutfrags.rbegin();
#if 0
					if(PruneB(map, &*j))
						nextoutfrags.erase(--(j.base()));
#endif
				}
				
				if(!PruneB2(&*fragb, &news1.m_plane))
				{
					Brush newfrag1 = *fragb;
					newfrag1.add(news1);
					newfrag1.collapse();
					newfrag1.remaptex();
					nextoutfrags.push_back(newfrag1);
					auto j = nextoutfrags.rbegin();
#if 0
					if(PruneB(map, &*j))
						nextoutfrags.erase(--(j.base()));
#endif
				}
#endif
			}
#endif
			
			infrags = nextinfrags;
			//outfrags = nextoutfrags;
		}
		
		//final check to keep brushes in
		for(auto fragb=infrags.begin(); fragb!=infrags.end();)
		{
			if(!BrushInCrater(&*fragb, crater))
			{
				outfrags.push_back(*fragb);
				fragb = infrags.erase(fragb);
			}

			fragb++;
		}

#if 11
		for(auto fragb=outfrags.begin(); fragb!=outfrags.end(); fragb++)
		{
#if 0
			bool binall = true;

			for(auto ep=crater.begin(); ep!=crater.end(); ep++)
			{
				Vec3f pop = PointOnPlane(*ep);

				bool vinall = true;

				for(int svi=0; svi<fragb->m_nsharedv; svi++)
				{
					Vec3f sv = fragb->m_sharedv[svi];

					if(!PointOnOrBehindPlane(sv, *ep, CLOSE_EPSILON*2))
					{
						vinall = false;
						break;
					}
				}

				if(!vinall)
				{
					binall = false;
					break;
				}
			}

			if(binall)
				continue;
#endif
#if 1
			//fragb->collapse();
			fragb->colva();
			fragb->coloutl();
			fragb->remaptex();
#endif
			g_edmap.m_brush.push_back(*fragb);
		}
#else
		for(auto fragb=outfrags.begin(); fragb!=outfrags.end(); fragb++)
		{
			g_edmap.m_brush.push_back(*fragb);
		}
#endif

#if 0
		for(auto fragb=infrags.begin(); fragb!=infrags.end(); fragb++)
		{
			g_edmap.m_brush.push_back(*fragb);
		}
#endif

#if 0
	Vec3f pop = PointOnPlane(cuttingp);
	BrushSide news0(cuttingp.m_normal, pop);
	BrushSide news1(Vec3f(0,0,0)-cuttingp.m_normal, pop);

	vector<Brush*> newsel;

	for(auto i=g_selB.begin(); i!=g_selB.end(); )
	{
		Brush* b = *i;

		Brush newb0 = *b;
		Brush newb1 = *b;

		newb0.add(news0);
		newb1.add(news1);

		newb0.collapse();
		newb1.collapse();

		newb0.remaptex();
		newb1.remaptex();

		g_edmap.m_brush.push_back(newb0);
		auto j = map->m_brush.rbegin();
		//PruneB(m, &*j);
		if(!PruneB(map, &*j))
		{
			newsel.clear();
			//g_selB.push_back( &*j );
			newsel.push_back( &*j );
		}
#endif
	}

#if 0
	//remove side polys that are hidden by other brushes
	for(auto biter=g_edmap.m_brush.begin(); biter!=g_edmap.m_brush.end(); biter++)
	{
		Brush* b = &*biter;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* s = &b->m_sides[i];

			for(auto biter=g_edmap.m_brush.begin(); biter!=g_edmap.m_brush.end(); biter++)
			{
				Brush* b2 = &*biter;

				if(b2 == b)
					continue;

				bool allhid = true;

				for(auto viter=s->m_sideverts.begin(); viter!=s->m_sideverts.end(); viter++)
				{

				}

				if(!allhid)
					continue;

				s->m_drawva.free();
			}
		}
	}
#endif

#endif
	
}

void ExplodeCrater(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax)
{
	g_dragV = -1;
	g_dragS = -1;
	//g_dragW = false;
	g_dragD = -1;
	g_selB.clear();
	//g_selM.clear();
	//CloseSideView();
	//CloseView("brush edit");

	//g_applog<<"select brush ("<<line[0].x<<","<<line[0].y<<","<<line[0].z<<")->("<<line[1].x<<","<<line[1].y<<","<<line[1].z<<")"<<std::endl;
	Brush* hitB = NULL;

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[ b->m_texture ];

		if(t->sky)
			continue;

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
			line[1] = trace;
			//g_selB.clear();
			//g_selB.push_back(&*b);
			hitB = &*b;
			//OpenAnotherView("brush edit");
			//return;
		}
	}

	if(!hitB)
		return;

	float hwx = 75;
	float hwz = 75;
	Vec3f p;

#if 0
	for(int i=0; i<3; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 0;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(FIREBALL, p + line[1]);
	}

	for(int i=0; i<2; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 0;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(FIREBALL2, p + line[1]);
	}
#endif
	/*
	for(int i=0; i<5; i++)
	{
	p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
	p.y = 8;
	p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
	EmitParticle(SMOKE, p + pos);
	}

	for(int i=0; i<5; i++)
	{
	p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
	p.y = 8;
	p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
	EmitParticle(SMOKE2, p + pos);
	}
	*/
#if 0
	for(int i=0; i<20; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = 0;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(DEBRIS, p + line[1]);
	}
#endif

	std::list<Plane3f> crater;
	MakeCrater(line[1], crater, 150.0f, 6);
	SubExplosion(map, crater);

	for(int i=0; i<0; i++)
	{
		crater.clear();
		MakeCrater(line[1] + Vec3f(rand()%1000-500, rand()%1000-500, rand()%1000-500), crater, 50.0f, 4);
		SubExplosion(map, crater);
	}
}