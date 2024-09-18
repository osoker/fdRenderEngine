//-----------------------------------------------------------------------------
// File: DXUtil.cpp
//
// Desc: Shortcut macros and functions for using DX objects
//
//
// Copyright (c) 1997-2000 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------


#include "stdafx.h"
#include "dxutil.h"





//-----------------------------------------------------------------------------
// Name: _DbgOut()
// Desc: Outputs a message to the debug stream
//-----------------------------------------------------------------------------
HRESULT _DbgOuts( TCHAR* strFile, DWORD dwLine, HRESULT hr, TCHAR* strMsg )
{
    TCHAR buffer[256];
    wsprintf( buffer, _T("%s(%ld): "), strFile, dwLine );
    OutputDebugString( buffer );
    OutputDebugString( strMsg );

    if( hr )
    {
        wsprintf( buffer, _T("(hr=%08lx)\n"), hr );
        OutputDebugString( buffer );
    }

    OutputDebugString( _T("\n") );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_Trace()
// Desc: Outputs to the debug stream a formatted string with a variable-
//       argument list.
//-----------------------------------------------------------------------------
VOID DXUtil_Traces( TCHAR* strMsg, ... )
{
#if defined(DEBUG) | defined(_DEBUG)
    TCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    _vsntprintf( strBuffer, 512, strMsg, args );
    va_end(args);

    OutputDebugString( strBuffer );
#endif
}



