///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lmc_infoDisplay.h
 *
 *  Desc:     用于在屏幕上显示信息字串
 * 
 *  His:      River created @ 2006-4-29 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# include "StdAfx.h"
# include ".\lmc_infodisplay.h"
# include "gc_gameScene.h"



lmc_infoDisplay::lmc_infoDisplay(void)
{
}

lmc_infoDisplay::~lmc_infoDisplay(void)
{
}

//! 执行指令的接口
BOOL lmc_infoDisplay::process_command( lm_command& _command )
{
	guard;

	return TRUE;

	unguard;
}

