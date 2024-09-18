#pragma once

# include "../common.h"

class COMMON_API CIni
{
public:
	CIni(void);
	~CIni(void);
	
	bool Open( const char *name );
	bool OpenAbsPath( const char * name );

	void SetSection( const char *name );
	char *get_filename();


	int  ReadLine( const char *key, char *value, int nSize );
	bool ReadLine( const char *key, int *value );
	bool ReadLine( const char *key, float *value );
	bool ReadLine( const char *key, double *value );

	bool ReadLine( const char *key, DWORD *value );

	bool WriteLine( const char *key, const char *value );
	bool WriteLine( const char *key, int value );
	bool WriteLine( const char *key, float value );

private:
	char m_filename[MAX_PATH];
	char m_section[MAX_PATH];


};
