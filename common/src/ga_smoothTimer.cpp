///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: ga_smoothTimer.cpp
 *
 *  Desc:     把底层得到的time时间平滑化处理
 * 
 *  His:      River created @ 2008-5-9
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
#include "..\include\ga_smoothtimer.h"
#include "..\include\timecounter.h"

ga_smoothTimer*				g_ptrSmoothTimer = NULL;
COMMON_API ga_smoothTimer    g_smoothTimer;

// 
// 相机和飞行器的移动使用多帧的平均值
# define FRAME_TIME_ADD 300

ga_smoothTimer* ga_smoothTimer::Instance(void){return &g_smoothTimer;}

ga_smoothTimer::ga_smoothTimer(void)
{
	g_ptrSmoothTimer = this;
	m_fSmoothTime = 0.0f;
}

ga_smoothTimer::~ga_smoothTimer(void)
{

}

//! 每一帧进行处理
void ga_smoothTimer::frame_move( void )
{
	//guard;


	float  t_fEleTime = sg_timer::Instance()->get_lastelatime();

	// TEST CODE:记录过去X帧的时间,以此来平均当前的时间
	static float t_fLastTime[FRAME_TIME_ADD];
	static int t_iCurTime = 0;
	if( t_iCurTime < FRAME_TIME_ADD )
	{
		t_fLastTime[t_iCurTime] = t_fEleTime;
		t_iCurTime ++;

		// 平均...
		t_fEleTime = 0.0f;
		for( int t_i=0;t_i<t_iCurTime;t_i ++ )
			t_fEleTime += t_fLastTime[t_i];
		t_fEleTime /= t_iCurTime;
	}
	else
	{
		for( int t_i=0;t_i<FRAME_TIME_ADD-1;t_i ++ )
			t_fLastTime[t_i] = t_fLastTime[t_i+1];

		t_fLastTime[FRAME_TIME_ADD-1] = t_fEleTime;

		float t_fCurEleTime = 0;
		for( int t_i=0;t_i<FRAME_TIME_ADD;t_i ++ )
			t_fCurEleTime += t_fLastTime[t_i];
		t_fEleTime = t_fCurEleTime/FRAME_TIME_ADD;
	}

	m_fSmoothTime = t_fEleTime;

	return;

	//unguard;
}
