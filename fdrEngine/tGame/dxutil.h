//-----------------------------------------------------------------------------
// File: DXUtil.h
//
// Desc: Helper functions and typing shortcuts for DirectX programming.
//
// Copyright (c) 1997-2000 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#ifndef DXUTIL_H
#define DXUTIL_H


//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath() and DXUtil_FindMediaFile() 
// Desc: Returns the DirectX SDK path, as stored in the system registry
//       during the SDK install.
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: DXUtil_Read*RegKey() and DXUtil_Write*RegKey()
// Desc: Helper functions to read/write a string registry key 
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: DXUtil_Timer()
// Desc: Performs timer opertations. Use the following commands:
//          TIMER_RESET           - to reset the timer
//          TIMER_START           - to start the timer
//          TIMER_STOP            - to stop (or pause) the timer
//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
//          TIMER_GETABSOLUTETIME - to get the absolute system time
//          TIMER_GETAPPTIME      - to get the current time
//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between 
//                                  TIMER_GETELAPSEDTIME calls
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// UNICODE support for converting between CHAR, TCHAR, and WCHAR strings
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Debug printing support
//-----------------------------------------------------------------------------
VOID    DXUtil_Traces( TCHAR* strMsg, ... );
HRESULT _DbgOuts( TCHAR*, DWORD, HRESULT, TCHAR* );

#if defined(DEBUG) | defined(_DEBUG)
    #define DXTRACE           DXUtil_Traces
#else
    #define DXTRACE           sizeof
#endif

#if defined(DEBUG) | defined(_DEBUG)
    #define DEBUG_MSG(str)    _DbgOut( __FILE__, (DWORD)__LINE__, 0, str )
#else
    #define DEBUG_MSG(str)    (0L)
#endif




#endif // DXUTIL_H
