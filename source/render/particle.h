


/*******************************************************
 * Copyright (C) 2015 DMD 'Ware <dmdware@gmail.com>
 * 
 * This file is part of States, Firms, & Households.
 * 
 * You are entitled to use this source code to learn.
 *
 * You are not entitled to duplicate or copy this source code 
 * into your own projects, commercial or personal, UNLESS you 
 * give credit.
 *
 *******************************************************/




#ifndef _PARTICLE_H
#define _PARTICLE_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../utils.h"

class Particle;
class Billboard;

class PlType
{
public:
	int billbT;
	int delay;
	float decay;
	Vec3f minvelocity;
	Vec3f velvariation;
	Vec3f minacceleration;
	Vec3f accelvariation;
	float minsize;
	float sizevariation;
	void (*collision)(Particle* part, Billboard* billb, Vec3f trace, Vec3f normal);
};

#define PARTICLE_EXHAUST		0
#define PARTICLE_EXHAUST2		1
#define PARTICLE_EXHAUSTBIG		2
#define PARTICLE_FIREBALL		3
#define PARTICLE_FIREBALL2		4
#define PARTICLE_SMOKE			5
#define PARTICLE_SMOKE2			6
#define PARTICLE_DEBRIS			7
#define PARTICLE_FLAME			8
#define PARTICLE_PLUME			9
#define PARTICLE_TYPES			10
extern PlType g_particleT[PARTICLE_TYPES];

class EmitterCounter
{
public:
	unsigned long long last;

	EmitterCounter()
	{
		last = GetTicks();
	}
	bool emitnext(int delay)
	{
		if(GetTicks()-last > (unsigned long long)delay)
		{
			last = GetTicks();
			return true;
		}
		else
			return false;
	}
};

class Billboard;

class Particle
{
public:
	bool on;
	int type;
	float life;
	Vec3f vel;
	float dist;

	Particle()
	{
		on = false;
	}
	Particle(Vec3f p, Vec3f v)
	{
		on = true;
		vel = v;
	}

	void update(Billboard* billb);
};

#define PARTICLES 512
extern Particle g_particle[PARTICLES];

class EmitterPlace
{
public:
	EmitterPlace()
	{
		on = false;
	}
	EmitterPlace(int t, Vec3f off)
	{
		on = true;
		type = t;
		offset = off;
	}
	bool on;
	Vec3f offset;
	int type;
};

void LoadParticles();
void Particles();
void EmitParticle(int type, Vec3f pos);
void UpdParts();
void FreeParts();

#endif
