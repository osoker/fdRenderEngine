///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lmc_infoDisplay.h
 *
 *  Desc:     用于在屏幕上显示信息字串,字串显示不在命令处理的范围内，此文件暂不使用
 * 
 *  His:      River created @ 2006-4-29 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once


# include "stdafx.h"
# include "gc_command.h"

/** \brief
 *  在屏幕显示字串信息的类
 */
class lmc_infoDisplay : public lm_gameEntity
{
private:

public:
	lmc_infoDisplay(void);
	~lmc_infoDisplay(void);

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );	

};
