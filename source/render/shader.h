#ifndef SHADER_H
#define SHADER_H

#include "../platform.h"

#define SSLOT_SHADOWMAP			0
#define SSLOT_LIGHTMATRIX		1
#define SSLOT_LIGHTPOS			2
//#define SSLOT_LIGHTDIR			3
#define SSLOT_TEXTURE0			4
#define SSLOT_POSITION			5
#define SSLOT_NORMAL			6
#define SSLOT_PROJECTION		7
#define SSLOT_MODELMAT			8
#define SSLOT_VIEWMAT			9
#define SSLOT_MODELVIEW         10
#define SSLOT_NORMALMAT			11
#define SSLOT_INVMODLVIEWMAT	12
#define SSLOT_MVP   			13
#define SSLOT_COLOR				14
#define SSLOT_OWNCOLOR			15
#define SSLOT_WIDTH				16
#define SSLOT_HEIGHT			17
#define SSLOT_CAMERAPOS			18
#define SSLOT_SCALE				19
#define SSLOT_MIND				20
#define SSLOT_MAXD				21
#define SSLOT_NORMALMAP			22
#define SSLOT_SPECULARMAP		23
#define SSLOT_GRADIENTTEX		38
#define SSLOT_DETAILTEX			39
#define SSLOT_OWNERMAP			40
#define SSLOT_TEXCOORD0			51
#define SSLOT_SCREENDEPTH       52
#define SSLOT_BASEDEPTH         53
#define SSLOT_SPRITEDEPTH       54
#define SSLOT_SCREENMAPWIDTH    55
#define SSLOT_SCREENMAPHEIGHT   56
#define SSLOT_SUNDIRECTION		57
#define SSLOT_MAXELEV			58
#define SSLOTS					59

class Shader
{
public:
	Shader()	{			 }
	~Shader()
	{
		release();
	}

	GLint getuniform(const char* strVariable);
	GLint getattrib(const char* strVariable);

	void mapuniform(int slot, const char* variable);
	void mapattrib(int slot, const char* variable);

	void release();

	bool m_hastexcoords;
	bool m_hasnormals;
	GLint slot[SSLOTS];

    // http://lists.apple.com/archives/mac-opengl/2005/Nov/msg00182.html
#if defined( PLATFORM_MAC ) || defined( PLATFORM_IOS )
	GLuint m_vertshader;
	GLuint m_fragshader;
	GLuint m_program;
#else
    GLhandleARB m_vertshader;
    GLhandleARB m_fragshader;
    GLhandleARB m_program;
#endif
};

#define SHADER_OWNED			0
#define SHADER_ORTHO			1
#define SHADER_COLOR2D			2
#define SHADER_BILLBOARD		3
#define SHADER_DEEPORTHO		4
#define SHADER_DEPTH            5
#define SHADER_MAP				6
#define SHADER_PARTICLEBLEND	7
#define SHADER_PARTICLEBLENDDEPTH	8
#define SHADER_DEEPCOLOR		9
#define SHADER_COLOR3D			10
#define SHADER_COLOR3DPERSP		11
#define SHADER_MODEL			12
#define SHADER_MODELPERSP		13
#define SHADER_MAPPERSP			14
#define SHADERS					15

extern Shader g_shader[SHADERS];
extern int g_curS;

void UseS(int shader);
void EndS();
void InitGLSL();
void TurnOffShader();
void ReleaseShaders();
std::string LoadTextFile(char* strFile);
void LoadShader(int shader, const char* strVertex, const char* strFragment, bool hastexcoords, bool hasnormals);

#endif


