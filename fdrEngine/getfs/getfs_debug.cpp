//---------------------------------------------------------------------------------------
/**  \file
*   Filename: getfs_debug.cpp
*   Desc:     
*   His:      yq_sun created @ 2005-05-16
*/
//---------------------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

void debug_out( const char* msg, ... )
{
	va_list argptr;
	char	text[1024];
	
	va_start (argptr,msg);
	vsprintf (text, msg,argptr);
	va_end (argptr);
	
	OutputDebugString( text );
}

