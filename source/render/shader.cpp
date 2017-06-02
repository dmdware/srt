#include "../utils.h"
#include "shader.h"
#include "../platform.h"
#include "../window.h"
#include "../sim/player.h"
#include "../debug.h"
#include "shadow.h"

Shader g_shader[SHADERS];
int g_curS = 0;

GLint Shader::getuniform(const char* strVariable)
{
    g_applog<<"shader "<<(int)(this-g_shader)<<" uniform "<<strVariable<<" = ";
    
	if(!m_program)
		return -1;
    
    g_applog<<glGetUniformLocation(m_program, strVariable)<<std::endl;

	return glGetUniformLocation(m_program, strVariable);
}

GLint Shader::getattrib(const char* strVariable)
{
	g_applog<<"shader "<<(int)(this-g_shader)<<" attrib "<<strVariable<<" = ";

	if(!m_program)
		return -1;

	g_applog<<glGetAttribLocation(m_program, strVariable)<<std::endl;

	return glGetAttribLocation(m_program, strVariable);
}

void Shader::mapuniform(int slot, const char* variable)
{
	this->slot[slot] = getuniform(variable);
	//g_applog<<"\tmap uniform "<<variable<<" = "<<(int)this->slot[slot]<<std::endl;
}

void Shader::mapattrib(int slot, const char* variable)
{
	this->slot[slot] = getattrib(variable);
	//g_applog<<"\tmap attrib "<<variable<<" = "<<(int)this->slot[slot]<<std::endl;
}

void GetGLVersion(int* major, int* minor)
{
	// for all versions
	char* ver = (char*)glGetString(GL_VERSION); // ver = "3.2.0"
    
    char vermaj[6];
    char vermin[6];

    int i;
    int j;
    
	for(i=0; i<6; i++)
	{
		if(ver[i] != '.')
			vermaj[i] = ver[i];
		else
        {
			vermaj[i] = '\0';
            i++;
            break;
        }
	}
    
    for(j=0; j<6; j++, i++)
    {
        if(ver[i] != '.')
            vermin[j] = ver[i];
        else
        {
            vermin[j] = '\0';
            i++;
            break;
        }
    }
    
	//*major = ver[0] - '0';
	*major = StrToInt(vermaj);
	if( *major >= 3)
	{
		// for GL 3.x
		//glGetIntegerv(GL_MAJOR_VERSION, major); // major = 3
		//glGetIntegerv(GL_MINOR_VERSION, minor); // minor = 2
	}
	else
	{
		//*minor = ver[2] - '0';
    }
    *minor = StrToInt(vermin);

	// GLSL
	ver = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION); // ver = "1.50 NVIDIA via Cg compiler"
}

void InitGLSL()
{
#ifndef PLATFORM_MAC
	//strstr("abab", "ba");
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		ErrMess("Error initializing GLEW!", (const char*)glewGetErrorString( glewError ));
		return;
	}
#endif

	g_applog<<"Renderer: "<<(char*)glGetString(GL_RENDERER)<<std::endl;
	g_applog<<"GL_VERSION = "<<(char*)glGetString(GL_VERSION)<<std::endl;

	int maxtex = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtex);
	g_applog<<"GL_MAX_TEXTURE_SIZE = "<<maxtex<<std::endl;


#ifdef __glew_h__
#if 0
	if( !GLEW_VERSION_4_1 )
	{
		ErrMess("Error", "OpenGL 4.1 not supported!\n" );
		g_quit = true;
		return;
	}
#else
	if( !GLEW_VERSION_1_4 )
	{
		ErrMess("Error", "OpenGL 1.4 not supported!\n" );
		g_quit = true;
		return;
	}
#endif
#endif

#if 1   //glGetString deprecated after 3.0
	char* szGLExtensions = (char*)glGetString(GL_EXTENSIONS);

    g_applog<<szGLExtensions<<std::endl<<std::endl;
	g_applog.flush();
    
#if 0	//on windows its still supported even though it says not?
    if(!strstr(szGLExtensions, "GL_EXT_gpu_shader4"))
    {
        ErrMess("Error", "GL_EXT_gpu_shader4 not supported.");
    }
#endif

#if !defined( PLATFORM_MAC ) && !defined( PLATFORM_IOS )
#ifdef GLDEBUG
	if(!strstr(szGLExtensions, "GL_ARB_debug_output"))
	{
		//ErrMess("Error", "GL_ARB_debug_output extension not supported!");
		//g_quit = true;
		//return;
		g_applog<<"GL_ARB_debug_output extension not supported"<<std::endl;
	}
	else
	{
		g_applog<<"Reging debug handler"<<std::endl;
		g_applog.flush();
		glDebugMessageCallback(&GLMessageHandler, 0);
		CHECKGLERROR();
	}
#endif
#endif

	if(!strstr(szGLExtensions, "GL_ARB_shader_objects"))
	{
		ErrMess("Error", "GL_ARB_shader_objects extension not supported!");
		g_quit = true;
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shading_language_100"))
	{
		ErrMess("Error", "GL_ARB_shading_language_100 extension not supported!");
		g_quit = true;
		return;
	}
#else
    
#if !defined( PLATFORM_MAC ) && !defined( PLATFORM_IOS )
    glDebugMessageCallback(&GLMessageHandler, 0);
    CHECKGLERROR();
#endif
    
#endif

	int major, minor;
	GetGLVersion(&major, &minor);

#if 0
	if(major < 4 || ( major == 4 && minor < 1 ))
	{
		ErrMess("Error", "OpenGL 4.1 is not supported!");
		g_quit = true;
	}
#else
	if(major < 1 || ( major == 1 && minor < 4 ))
	{
		ErrMess("Error", "OpenGL 1.4 is not supported!");
		g_quit = true;
	}
#endif

#if 0
#define SHADER_OWNED			0
#define SHADER_ORTHO			1
#define SHADER_COLOR2D			2
#define SHADER_BILLBOARD		3
#define SHADER_DEEPORTHO		4
#define SHADERS					5
#endif

	LoadShader(SHADER_ORTHO, "shaders/ortho.vert", "shaders/ortho.frag", true, false);
    LoadShader(SHADER_DEPTH, "shaders/depth.vert", "shaders/depth.frag", true, false);
	//LoadShader(SHADER_DEEPORTHO, "shaders/ortho.vert", "shaders/ortho.frag", true, false);
	LoadShader(SHADER_DEEPORTHO, "shaders/deeportho.vert", "shaders/deeportho.frag", true, false);
	LoadShader(SHADER_COLOR2D, "shaders/color2d.vert", "shaders/color2d.frag", false, false);
	LoadShader(SHADER_OWNED, "shaders/owned.vert", "shaders/owned.frag", true, true);
	LoadShader(SHADER_MAP, "shaders/mappersp.vert", "shaders/map.frag", true, true);
	LoadShader(SHADER_PARTICLEBLEND, "shaders/particleblend.vert", "shaders/particleblend.frag", true, false);
    LoadShader(SHADER_PARTICLEBLENDDEPTH, "shaders/particleblenddepth.vert", "shaders/particleblenddepth.frag", true, false);
	LoadShader(SHADER_DEEPCOLOR, "shaders/deepcolor.vert", "shaders/deepcolor.frag", false, false);
	LoadShader(SHADER_COLOR3D, "shaders/color3d.vert", "shaders/color3d.frag", false, false);
	LoadShader(SHADER_COLOR3DPERSP, "shaders/color3dpersp.vert", "shaders/color3d.frag", false, false);
	LoadShader(SHADER_MODEL, "shaders/model.vert", "shaders/model.frag", true, true);
	LoadShader(SHADER_MODELPERSP, "shaders/modelpersp.vert", "shaders/model.frag", true, true);
	LoadShader(SHADER_MAPPERSP, "shaders/mappersp.vert", "shaders/map.frag", true, true);
	//LoadShader(SHADER_WATER, "shaders/water.vert", "shaders/water.frag", true, true);
	
	InitShadows();
}

void LoadShader(int shader, const char* strVertex, const char* strFragment, bool hastexcoords, bool hasnormals)
{
	Shader* s = &g_shader[shader];
	std::string strVShader, strFShader;

	if(s->m_vertshader || s->m_fragshader || s->m_program)
		s->release();

	s->m_hastexcoords = hastexcoords;
	s->m_hasnormals = hasnormals;
	s->m_vertshader = glCreateShader(GL_VERTEX_SHADER);
	s->m_fragshader = glCreateShader(GL_FRAGMENT_SHADER);
    
    char vfull[DMD_MAX_PATH+1];
    char ffull[DMD_MAX_PATH+1];
    
    FullPath(strVertex, vfull);
    FullPath(strFragment, ffull);

	strVShader = LoadTextFile(vfull);
	strFShader = LoadTextFile(ffull);

	const char* szVShader = strVShader.c_str();
	const char* szFShader = strFShader.c_str();

	glShaderSource(s->m_vertshader, 1, &szVShader, NULL);
	glShaderSource(s->m_fragshader, 1, &szFShader, NULL);

	glCompileShader(s->m_vertshader);
	GLint logLength;
	glGetShaderiv(s->m_vertshader, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0)
	{
		GLchar* log = (GLchar*)malloc(logLength);

		if(!log)
		{
			OUTOFMEM();
			return;
		}

		glGetShaderInfoLog(s->m_vertshader, logLength, &logLength, log);
		g_applog<<"Shader "<<strVertex<<" compile log: "<<std::endl<<log<<std::endl;
		free(log);
	}

	glCompileShader(s->m_fragshader);
	glGetShaderiv(s->m_fragshader, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0)
	{
		GLchar* log = (GLchar*)malloc(logLength);

		if(!log)
		{
			OUTOFMEM();
			return;
		}

		glGetShaderInfoLog(s->m_fragshader, logLength, &logLength, log);
		g_applog<<"Shader "<<strFragment<<" compile log: "<<std::endl<<log<<std::endl;
		free(log);
	}

	s->m_program = glCreateProgram();
	glAttachShader(s->m_program, s->m_vertshader);
	glAttachShader(s->m_program, s->m_fragshader);
	glLinkProgram(s->m_program);

	//glUseProgramObject(s->m_program);

	//g_applog<<"shader "<<strVertex<<","<<strFragment<<std::endl;

	g_applog<<"Program "<<strVertex<<" / "<<strFragment<<" :";

	glGetProgramiv(s->m_program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar* log = (GLchar*)malloc(logLength);
		glGetProgramInfoLog(s->m_program, logLength, &logLength, log);
		g_applog<<"Program link log:"<<std::endl<<log<<std::endl;
		free(log);
	}

	GLint status;
	glGetProgramiv(s->m_program, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		g_applog<<"link status 0"<<std::endl;
	}
	else
	{
		g_applog<<"link status ok, program="<<s->m_program<<std::endl;
	}

	g_applog<<std::endl<<std::endl;

	s->mapattrib(SSLOT_POSITION, "position");
	s->mapattrib(SSLOT_NORMAL, "normalin");
	s->mapattrib(SSLOT_TEXCOORD0, "texcoordin0");
	//s->mapattrib(SSLOT_TANGENT, "tangent");
	s->mapuniform(SSLOT_SHADOWMAP, "shadowmap");
	s->mapuniform(SSLOT_LIGHTMATRIX, "lightMatrix");
	s->mapuniform(SSLOT_LIGHTPOS, "lightPos");
	//s->mapuniform(SSLOT_LIGHTDIR, "lightdir");
	s->mapuniform(SSLOT_TEXTURE0, "texture0");
	s->mapuniform(SSLOT_NORMALMAP, "normalmap");
	s->mapuniform(SSLOT_SPECULARMAP, "specularmap");
	s->mapuniform(SSLOT_PROJECTION, "projection");
	s->mapuniform(SSLOT_MODELMAT, "model");
	s->mapuniform(SSLOT_VIEWMAT, "view");
	s->mapuniform(SSLOT_MVP, "mvp");
	s->mapuniform(SSLOT_MODELVIEW, "modelview");
	s->mapuniform(SSLOT_NORMALMAT, "normalMat");
	//s->mapuniform(SSLOT_INVMODLVIEWMAT, "invModelView");
	s->mapuniform(SSLOT_COLOR, "color");
	s->mapuniform(SSLOT_OWNCOLOR, "owncolor");
	s->mapuniform(SSLOT_WIDTH, "width");
	s->mapuniform(SSLOT_HEIGHT, "height");
	s->mapuniform(SSLOT_MIND, "mind");
	s->mapuniform(SSLOT_MAXD, "maxd");
    s->mapuniform(SSLOT_CAMERAPOS, "cameraPos");
    s->mapuniform(SSLOT_SCALE, "scale");
    s->mapuniform(SSLOT_SCREENDEPTH, "screendepthmap");
    s->mapuniform(SSLOT_BASEDEPTH, "basedepth");
    s->mapuniform(SSLOT_SPRITEDEPTH, "spritedepthmap");
    s->mapuniform(SSLOT_SCREENMAPWIDTH, "screenmapwidth");
    s->mapuniform(SSLOT_SCREENMAPHEIGHT, "screenmapheight");
	s->mapuniform(SSLOT_SUNDIRECTION, "sundirection");
	s->mapuniform(SSLOT_MAXELEV, "maxelev");
	s->mapuniform(SSLOT_OWNERMAP, "ownermap");
}

void UseS(int shader)
{
	CHECKGLERROR();
	g_curS = shader;

	Shader* s = &g_shader[g_curS];

	//glUseProgramObject(g_shader[shader].m_program);
	glUseProgram(s->m_program);
	CHECKGLERROR();

	Player* py = &g_player[g_localP];

#ifdef PLATFORM_GLES20
    //glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//opengl 3 way
	CHECKGLERROR();
	/* if(s->slot[SSLOT_POSITION] != -1)	*/ glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	CHECKGLERROR();
	if(s->slot[SSLOT_TEXCOORD0] != -1) glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
	CHECKGLERROR();
	if(s->slot[SSLOT_NORMAL] != -1)	glEnableVertexAttribArray(s->slot[SSLOT_NORMAL]);
	CHECKGLERROR();
#endif
	
#ifdef PLATFORM_GL14
	//opengl 1.4 way
	glEnableClientState(GL_VERTEX_ARRAY);
	if(s->m_hastexcoords)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(s->m_hasnormals)	glEnableClientState(GL_NORMAL_ARRAY);
#endif

	if(s->slot[SSLOT_MIND] != -1) glUniform1f(s->slot[SSLOT_MIND], MIN_DISTANCE);
	if(s->slot[SSLOT_MAXD] != -1) glUniform1f(s->slot[SSLOT_MAXD], MAX_DISTANCE / g_zoom);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EndS()
{
	CHECKGLERROR();

	if(g_curS < 0)
		return;

	Shader* s = &g_shader[g_curS];

#ifdef PLATFORM_GLES20
    //glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//opengl 3 way
	CHECKGLERROR();
	if(s->slot[SSLOT_POSITION] != -1)	glDisableVertexAttribArray(s->slot[SSLOT_POSITION]);
	CHECKGLERROR();
	if(s->slot[SSLOT_TEXCOORD0] != -1) glDisableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
	CHECKGLERROR();
	if(s->slot[SSLOT_NORMAL] != -1)	glDisableVertexAttribArray(s->slot[SSLOT_NORMAL]);
	CHECKGLERROR();
#endif
	
#ifdef PLATFORM_GL14
	//opengl 1.4 way
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
#endif

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	g_curS = -1;
}

void Shader::release()
{
	if(m_vertshader)
	{
		glDetachShader(m_program, m_vertshader);
		glDeleteShader(m_vertshader);
		m_vertshader = NULL;
	}

	if(m_fragshader)
	{
		glDetachShader(m_program, m_fragshader);
		glDeleteShader(m_fragshader);
		m_fragshader = NULL;
	}

	if(m_program)
	{
		glDeleteProgram(m_program);
		m_program = NULL;
	}
}

void TurnOffShader()
{
	glUseProgram(0);
}

void ReleaseShaders()
{
	for(int i=0; i<SHADERS; i++)
		g_shader[i].release();
}

