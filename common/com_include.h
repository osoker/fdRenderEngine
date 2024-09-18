//=================================================================================
/** \file
 * Filename: com_include.h
 * Desc;     最低层常用库的封装.
 * His:      River created @August 2002.
 */
//=================================================================================
#ifndef __COM_INCLUDE__
#define __COM_INCLUDE__

# include "windows.h"
# include "mmsystem.h"
# include "stdio.h"

//! 打开dx的调试信息.
# define  D3D_DEBUG_INFO
# include "dxerr9.h"
# include "d3dx9.h"
# include "dsound.h"


// - Remed by chenlee, 2002/08/08
//
// ( lee : I think only a little files need this header, so put it into this
//         common header seems inappropriate. )
//
# include "include/IniFile.h"

# include "include/sgdebug.h"


# include "include/timecounter.h"
# include "include/sgdefine.h"
# include "include/sgLru.h"
# include "include/DeprecationMgr.h"

#include "include/Ini.h"

#include "include/com_thread.h"

#include "include/MemoryManager.h"

#include "include/tinyxml.h"
#include "include/ds_extend.h"

# include "include\ga_smoothTimer.h"
/*
//////////////////////////////////////////////////////////////////////////
// add by yuanlin [8/7/2002]
#include "include/sgVar.h"
#include "include/sgCmd.h"
//////////////////////////////////////////////////////////////////////////

#include "include/sgEntity.h"
*/

/************************************************************************************/
//
//rememeber to include the follow .lib file from d3dx&windows:
//winmm.lib, d3dx8.lib, Dxerr8.lib
//
/************************************************************************************/


#endif   //#define __COM_INCLUDE__