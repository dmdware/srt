


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




#include "utils.h"
#include "platform.h"
#include "window.h"
#include "version.h"

std::ofstream g_applog;

const std::string DateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

const std::string Time()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%X", &tstruct);

	return buf;
}

const std::string FileDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	for(int i=0; i<strlen(buf); i++)
		if(buf[i] == ':')
			buf[i] = '-';

	return buf;
}


void OpenLog(const char* filename, unsigned long long version)
{
	char fullpath[DMD_MAX_PATH+1];
	FullWritePath(filename, fullpath);
	g_applog.close();
	g_applog.open(fullpath, std::ios_base::out);
	Log("%s\r\n", DateTime().c_str());
	char verstr[64];
	VerStr(version, verstr);
	Log("Version %s\r\n", verstr);
	
}

void Log(const char* format, ...)
{
	if(!g_applog.is_open())
		return;

	char dest[1024 * 16];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(dest, format, argptr);
	va_end(argptr);
	g_applog<<dest;
	g_applog. flush();
#ifdef PLATFORM_IOS
	NSLog(@"%s", dest);
#endif
}

std::string MakeRelative(const char* full)
{
	char full2c[DMD_MAX_PATH+1];
	strcpy(full2c, full);
	CorrectSlashes(full2c);
	std::string full2(full2c);
	char exepath[DMD_MAX_PATH+1];
	ExePath(exepath);
	CorrectSlashes(exepath);

	//Log("exepath: "<<exepath);
	//Log("fulpath: "<<full);

	std::string::size_type pos = full2.find(exepath, 0);

	if(pos == std::string::npos)
	{
		return full2;
	}

	//Log("posposp: "<<pos);

	std::string sub = std::string( full2 ).substr(strlen(exepath), strlen(full)-strlen(exepath));

	//Log("subpath: "<<sub);

	return sub;
}

std::string StripFile(std::string filepath)
{
	int lastof = filepath.find_last_of("/\\");
	if(lastof < 0)
		lastof = strlen(filepath.c_str());
	else
		lastof += 1;

	std::string stripped = filepath.substr(0, lastof);
	return stripped;
}

void StripPath(char* filepath)
{
	std::string s0(filepath);
	size_t sep = s0.find_last_of("\\/");
	std::string s1;

	if (sep != std::string::npos)
		s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	strcpy(filepath, s1.c_str());
}

void StripExt(char* filepath)
{
	std::string s1(filepath);

	size_t dot = s1.find_last_of(".");
	std::string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(filepath, s2.c_str());
}

void StripPathExt(const char* n, char* o)
{
	std::string s0(n);
	size_t sep = s0.find_last_of("\\/");
	std::string s1;

	if (sep != std::string::npos)
		s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	size_t dot = s1.find_last_of(".");
	std::string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(o, s2.c_str());
}

void ExePath(char* exepath)
{
#ifndef PLATFORM_IOS
#if 0
#ifdef PLATFORM_WIN
	//char buffer[DMD_MAX_PATH+1];
	GetModuleFileName(NULL, exepath, DMD_MAX_PATH+1);
	//std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
	//std::string strexepath = std::string( buffer ).substr( 0, pos);
	//strcpy(exepath, strexepath.c_str());
#else
	char szTmp[32];
	//char buffer[DMD_MAX_PATH+1];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = std::min((int)readlink(szTmp, exepath, DMD_MAX_PATH+1), DMD_MAX_PATH);
	if(bytes >= 0)
		exepath[bytes] = '\0';
	//std::string strexepath = StripFile(std::string(buffer));
	//strcpy(exepath, strexepath.c_str());
#endif
#else
	char *base_path = SDL_GetBasePath();
#if 0
	if (base_path) {
        data_path = SDL_strdup(base_path);
        SDL_free(base_path);
    } else {
        data_path = SDL_strdup("./");
    }
#endif
	if(base_path)
	{
		strcpy(exepath, base_path);
		SDL_free(base_path);
	}
#endif
#endif
	
#ifdef PLATFORM_IOS
	char *base_path = SDL_GetBasePath();
	if(base_path)
	{
		strcpy(exepath, base_path);
		SDL_free(base_path);
	}
	strcat(exepath, "testfolder/");
#endif
}

void FullWritePath(const char* filename, char* full)
{
#ifdef PLATFORM_IOS
	/*
	 char exepath[DMD_MAX_PATH+1];
	 GetModuleFileName(NULL, exepath, DMD_MAX_PATH);
	 string path = StripFile(exepath);
	 
	 //char full[DMD_MAX_PATH+1];
	 sprintf(full, "%s", path.c_str());
	 
	 char c = full[ strlen(full)-1 ];
	 if(c != '\\' && c != '/')
	 strcat(full, "\\");
	 
	 strcat(full, filename);*/
	
	//NSString *path = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/"];
	//NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	//NSString *path = [paths objectAtIndex:0];
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *path = [paths objectAtIndex:0];
	sprintf(full, "%s/%s", [path cStringUsingEncoding:NSUTF8StringEncoding], filename);
	
	//NSLog(@"full write %s", full);
#else
	
	FullPath(filename, full);
#endif
}

void FullPath(const char* filename, char* full)
{
	char exepath[DMD_MAX_PATH+1];
	ExePath(exepath);
	std::string path = exepath;
	//std::string path = StripFile(exepath);

	//char full[DMD_MAX_PATH+1];
	sprintf(full, "%s", path.c_str());

	char c = full[ strlen(full)-1 ];
	if(c != '\\' && c != '/')
		strcat(full, "\\");
	//strcat(full, "/");
	
	std::string edfile = filename;

	if(edfile[0] == '.' && edfile[1] == '/')
		edfile = edfile.substr(2, edfile.length() - 2);

	//strcat(full, filename);
	strcat(full, edfile.c_str());
	CorrectSlashes(full);
}

float StrToFloat(const char *s)
{
	if(s[0] == '\0')
		return 1.0f;

	float x;
	std::istringstream iss(s);
	iss >> x;

	if(_isnan(x))
		x = 1.0f;

	return x;
}

int HexToInt(const char* s)
{
	int x;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> x;
	return x;
}

int StrToInt(const char *s)
{
	int x;
	std::istringstream iss(s);
	iss >> x;
	return x;
}

void CorrectSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '\\')
			corrected[i] = '/';
}

void BackSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';
}

void ErrMess(const char* title, const char* message)
{
	SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);
	SDL_ShowCursor(false);
}

void InfoMess(const char* title, const char* message)
{
	SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, NULL);
	SDL_ShowCursor(false);
}

void WarnMess(const char* title, const char* message)
{
	SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, message, NULL);
	SDL_ShowCursor(false);
}

void OutOfMem(const char* file, int line)
{
	char msg[2048];
	sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	ErrMess("Out of memory", msg);
	//g_quit = true;
}

std::string LoadTextFile(char* strFile)
{
	std::ifstream fin(strFile);

	if(!fin)
	{
		Log("Failed to load file %s\r\n", strFile);
		return "";
	}

	std::string strLine = "";
	std::string strText = "";

	getline(fin, strText);	//corpd fix

	while(getline(fin, strLine))
		strText = strText + "\n" + strLine;

	fin.close();

	return strText;
}

void ListFiles(const char* fullpath, std::list<std::string>& files)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp  = opendir(fullpath)) == NULL)
	{
		g_applog << "Error opening " << fullpath << std::endl;
		return;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if(dirp->d_type != DT_REG)
			continue;

		files.push_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return;
}

void ListDirs(const char* fullpath, std::list<std::string>& dirs)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp  = opendir(fullpath)) == NULL)
	{
		g_applog << "Error opening " << fullpath << std::endl;
		return;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if(dirp->d_type != DT_DIR)
			continue;
		
		if(strcmp(dirp->d_name, ".") == 0)
			continue;
		if(strcmp(dirp->d_name, "..") == 0)
			continue;

		dirs.push_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return;
}

//add thousands separators
std::string iform(int n)
{
	std::string s;

	char first[32];
	sprintf(first, "%d", n);

	for(int i=strlen(first)-3; i>=0; i-=3)
	{
		if(first[i] == '-' || i == 0 || first[i-1] == '-')
			break;

		int newl = strlen(first)+1;

		for(int j=newl; j>=i; j--)
		{
			first[j+1] = first[j];
		}

		first[i] = ',';
	}

	s = first;

	return s;
}

std::string ullform(unsigned long long n)
{
	std::string s;

	char first[32];
	sprintf(first, "%llu", n);

	for(int i=strlen(first)-3; i>=0; i-=3)
	{
		if(first[i] == '-' || i == 0 || first[i-1] == '-')
			break;

		int newl = strlen(first)+1;

		for(int j=newl; j>=i; j--)
		{
			first[j+1] = first[j];
		}

		first[i] = ',';
	}

	s = first;

	return s;
}

unsigned long long GetTicks()
{
	//return time(0);
#ifdef PLATFORM_WIN
	//return GetTickCount64();
	SYSTEMTIME st;
	GetSystemTime (&st);
	_FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	//LARGE_INTEGER lint;
	//lint.HighPart = ft.dwHighDateTime;
    //lint.LowPart = ft.dwLowDateTime;
	//convert from 100-nanosecond intervals to milliseconds
	return (*(unsigned long long*)&ft)/(10*1000);
#else
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return
    (unsigned long long)(tv.tv_sec) * 1000 +
    (unsigned long long)(tv.tv_usec) / 1000;
#endif
}

#ifdef PLATFORM_WIN
void MiniDumpFunction( unsigned int nExceptionCode, EXCEPTION_POINTERS *pException )
{
#ifdef USESTEAM
	// You can build and set an arbitrary comment to embed in the minidump here,
	// maybe you want to put what level the user was playing, how many players on the server,
	// how much memory is free, etc...
	SteamAPI_SetMiniDumpComment( "Minidump comment: SteamworksExample.exe\n" );

	// The 0 here is a build ID, we don't set it
	SteamAPI_WriteMiniDump( nExceptionCode, pException, 0 );
#endif
}
#endif