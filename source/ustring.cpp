#include "platform.h"
#include "ustring.h"
#include "utils.h"
#include "sys/unicode.h"

UStr::UStr()
{
	m_data = new unsigned int[1];
	m_data[0] = 0;
	m_length = 0;
}

UStr::~UStr()
{
#ifdef USTR_DEBUG
	g_applog<<"delete UStr...";
	g_applog.flush();

	for(int i=0; i<m_length; i++)
	{
		g_applog<<(char)m_data[i];
		g_applog.flush();
	}

	g_applog<<std::endl;
	g_applog.flush();

	//g_applog<<"'"<<rawstr()<<"'"<<std::endl;
	//g_applog.flush();
#endif

	delete [] m_data;
}


UStr::UStr(const UStr& original)
{
	m_data = new unsigned int[1];
	m_data[0] = 0;
	m_length = 0;
	*this = original;
}

UStr::UStr(const char* cstr)
{
#ifdef USTR_DEBUG
	g_applog<<"UStr::UStr(const char* cstr)"<<std::endl;
	g_applog.flush();
#endif
	
#if 0
	m_length = strlen(cstr);
	m_data = new unsigned int [m_length+1];
	for(int i=0; i<m_length+1; i++)
		m_data[i] = (unsigned char)cstr[i];
#else
	m_data = ToUTF32((unsigned char*)cstr);
	for(m_length=0; m_data[m_length]; m_length++)
		;
#endif
}

UStr::UStr(unsigned int k)
{
	m_length = 1;
	m_data = new unsigned int [m_length+1];
	m_data[0] = k;
	m_data[1] = 0;
}

UStr::UStr(unsigned int* k)
{
	if(!k)
	{
		m_length = 0;
		m_data = new unsigned int [m_length+1];
		m_data[0] = 0;
		return;
	}

	for(m_length=0; k[m_length]; m_length++)
		;
	m_data = new unsigned int [m_length+1];
	memcpy(m_data, k, sizeof(unsigned int) * (m_length+1));
}

//#define USTR_DEBUG

UStr& UStr::operator=(const UStr& original)
{
#ifdef USTR_DEBUG
	g_applog<<"UStr= ["<<rawstr()<<"] => ["<<original.rawstr()<<"]"<<std::endl;
	g_applog.flush();
#endif

	delete [] m_data;

	m_length = original.m_length;
	m_data = new unsigned int [m_length+1];
	memcpy(m_data, original.m_data, sizeof(unsigned int) * (m_length+1));

	return *this;
}

UStr UStr::operator+(const UStr &other)
{
	UStr newstr;

	delete [] newstr.m_data;

	newstr.m_length = m_length + other.m_length;

	newstr.m_data = new unsigned int[newstr.m_length+1];

	for(int i=0; i<m_length; i++)
		newstr.m_data[i] = m_data[i];

	for(int i=0; i<other.m_length; i++)
		newstr.m_data[i+m_length] = other.m_data[i];

	newstr.m_data[m_length+other.m_length] = 0;

	return newstr;
}

UStr UStr::substr(int start, int len) const
{
	UStr newstr;

	if(len <= 0)
		return newstr;

	//important fix
	if(start < 0 || start >= m_length)
		return newstr;

	delete [] newstr.m_data;
	newstr.m_length = len;
	newstr.m_data = new unsigned int[len+1];
	for(int i=0; i<len; i++)
		newstr.m_data[i] = m_data[start+i];

	newstr.m_data[len] = 0;

#ifdef USTR_DEBUG
	g_applog<<"USt substr :: "<<newstr.rawstr()<<std::endl;
	g_applog.flush();
#endif

	return newstr;
}

int UStr::firstof(UStr find) const
{
	for(int i=0; i<m_length; i++)
	{
		bool found = true;

		for(int j=0; j<find.m_length; j++)
		{
			if(m_data[i+j] != find.m_data[j])
			{
				found = false;
				break;
			}
		}

		if(found)
			return i;
	}

	return -1;
}

std::string UStr::rawstr() const
{
	std::string finstr;

//#ifdef USTR_DEBUG
#if 0
	g_applog<<"\t\tstring UStr::rawstr() const..."<<std::endl;
	g_applog.flush();

	g_applog<<"\t\t\t"<<std::endl;
	g_applog.flush();

	for(int i=0; i<m_length; i++)
	{
		g_applog<<"#"<<i;
		g_applog<<"'"<<(char)m_data[i]<<"' ";
		g_applog.flush();
	}

#endif

#if 0
	for(int i=0; i<m_length; i++)
		finstr += (char)m_data[i];
#else
	//utf8
	unsigned char* utf8 = ToUTF8(m_data);
	finstr = (char*)utf8;
	delete [] utf8;
#endif

	return finstr;
}

