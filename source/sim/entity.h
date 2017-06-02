


#ifndef ENTITY_H
#define ENTITY_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../sound/sound.h"
#include "../math/camera.h"

#define BODY_LOWER   0
#define BODY_UPPER   1

#define ECAT_NOCAT		0
#define ECAT_HUMAN		1
#define ECAT_ZOMBIE		2
#define ECAT_DOOR		3
#define ECAT_FIXEDENT	4
#define ECAT_ITEM		5
#define ECATS			6

enum ENTITYSND{OPENSND, CLOSESND};

class EType
{
public:
	std::string name;
    char lmodel[32];
    int model[2];
	int collider;
	Vec3f vMin, vMax;
	float maxStep;
	float speed;
	float jump;
	float crouch;
	float animrate;
	int category;
	int item;
	Vec3f vCenterOff;
	//vector<CSound> openSound;
	//vector<CSound> closeSound;

	EType()
	{
		model[0] = -1;
		model[1] = -1;
		collider = -1;
		item = -1;
	}
};

#define ETYPE_HUMAN		0
#define ETYPES			1

extern EType g_etype[ETYPES];

#define MID_HEIGHT_OFFSET	-13.49f   //-15.0f
#define HEAD_OFFSET			(-6.9f*0.7143f)

#define STATE_NONE		0
#define STATE_OPENING	1
#define STATE_CLOSING	2

class Ent
{
public:
	bool on;
    float frame[2];
	int type;
	int controller;
	Camera camera;
	float amount;
	float clip;
	int state;
	int cluster;
	float dist;
	bool nolightvol;
	int script;

	Ent()
	{
		on = false;
		controller = -1;
        frame[BODY_LOWER] = 0;
        frame[BODY_UPPER] = 0;
		amount = -1;
		clip = -1;
	}

	Vec3f traceray(Vec3f vLine[]);
	bool Collision(Vec3f vScaleDown, Vec3f vCenter, Vec3f vFirstCenter);
};

#define ENTITIES	1024
extern Ent g_entity[ENTITIES];

class Spawn
{
public:
	Vec3f pos;
	float angle;
	int activity;
	int script;

	Spawn(Vec3f p, float ang, int act, int scrpt)
	{
		pos = p;
		angle = ang;
		activity = act;
		script = scrpt;
	}
};

extern std::vector<Spawn> g_spawn;
extern std::vector<Spawn> g_zspawn;

void DrawHands();
void SortEntities();
void DrawEntities(bool transp);
void UpdateObjects();
int EntityID(char* lmodel);
void DefEnt(int etype, const char* name, int category, const char* lowermodel, Vec3f scale, float animrate, int collider);
bool PlaceEntity(int type, int controller, float amount, float clip, Vec3f pos, float yaw, int* id, bool nolightvol, int script);
void Entities();
void Swizzle(Vec3f &v);
void ReadEntities(char* str);
bool IsHuman(int type);
bool IsZombie(int type);
int RandomHuman();
int RandomZombie();
void FreeEnt(Ent* e);

#endif
