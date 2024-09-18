//=======================================================================================================
/**  \file
 *   Filename: Ini.cpp
 *  
 *   Desc:     ÅäÖÃÎÄ¼þ²Ù×÷
 *
 *   His:      yq_sun created @ 2004-11-15.
 *
 */
//=======================================================================================================

# include "stdafx.h"
#include <assert.h>
#include "..\include\ini.h"



CIni::CIni(void)
{
	memset(m_filename,0,MAX_PATH);
	memset(m_section,0,MAX_PATH);
}

CIni::~CIni(void)
{
}

bool CIni::Open( const char *name )
{
	assert(name);

	m_filename[0] = 0;

	::GetModuleFileNameA(NULL,m_filename,sizeof(m_filename));
	
	char* t_slash = strrchr(m_filename,'\\');
	if(t_slash){
		t_slash[1] = 0;
		if(strlen(m_filename) + strlen(name) - 1 > sizeof(m_filename)){
			return false;
		}
		strcat(m_filename,name);
	}else{
		return false;
	}

	// is this file exist ?
	//
	FILE* t_file = ::fopen(m_filename,"rb");
	if(!t_file){
		return false;
	}else{
		::fclose(t_file);
	}
	
	return true;
}

bool CIni::OpenAbsPath( const char * name )
{
	strcpy( m_filename, name );
	return true;
}

void CIni::SetSection( const char *name )
{
	strcpy( m_section, name );
}

char * CIni::get_filename()
{
	return m_filename;
}

//================================================================================
int CIni::ReadLine( const char *key, char *value, int nSize )
{
	return	::GetPrivateProfileString(m_section,key,NULL,value,nSize,m_filename);
}

bool CIni::ReadLine( const char *key, int *value )
{
	*value = ::GetPrivateProfileInt( m_section,key,0, m_filename );
	return true;
}

bool CIni::ReadLine( const char *key, DWORD *value )
{
	*value = (DWORD)::GetPrivateProfileInt( m_section,key,0, m_filename );
	return true;
}

bool CIni::ReadLine( const char *key, float *value )
{
	char buffer[MAX_PATH] = {0};
	::GetPrivateProfileString(m_section,key,NULL,buffer,MAX_PATH,m_filename);
	*value = (float)atof(buffer);
	return true;
}

bool CIni::ReadLine( const char *key, double *value )
{
	char buffer[MAX_PATH] = {0};
	::GetPrivateProfileString(m_section,key,NULL,buffer,MAX_PATH,m_filename);
	*value = (double)atof(buffer);
	return true;
}

//--------------------------------------------------------------------------
bool CIni::WriteLine( const char *key, const char *value )
{
	return ::WritePrivateProfileString( m_section,key,value,m_filename)?true:false;
}

bool CIni::WriteLine( const char *key, int value )
{
	char buffer[MAX_PATH] = {0};
	sprintf(buffer,"%d",value);
	return ::WritePrivateProfileString( m_section,key,buffer,m_filename)?true:false;
}

bool CIni::WriteLine( const char *key, float value )
{
	char buffer[MAX_PATH] = {0};
	sprintf(buffer,"%f",value);
	return ::WritePrivateProfileString( m_section,key,buffer,m_filename)?true:false;
}
