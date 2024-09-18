///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: ga_smoothTimer.h
 *
 *  Desc:     把底层得到的time时间平滑化处理
 * 
 *  His:      River created @ 2008-5-9
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once
# include "windows.h"
# include "mmsystem.h"
# include "stdio.h"
# include "../common.h"
# include <assert.h>

/** \brief
 *  把底层上来的时间平滑处理一下，使上层的运动更加平顺
 *
 */
class COMMON_API ga_smoothTimer
{
private:
	float    m_fSmoothTime;

public:
	ga_smoothTimer(void);
	~ga_smoothTimer(void);

	//! 每一帧进行处理
	void     frame_move( void );
	//! 上层得到平滑时间
	float    get_smoothTime( void ) { return m_fSmoothTime; }

	static ga_smoothTimer* Instance(void);

};

extern COMMON_API ga_smoothTimer*   g_ptrSmoothTimer;