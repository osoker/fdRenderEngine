//------------------------------------------------------------------------------------------
/**
 * Filename: timecounter.cpp
 * Desc:     global time counter func
 * His:      River created @ 11/23 2001.
 */
//------------------------------------------------------------------------------------------
# include "stdafx.h"
#include "../include/timecounter.h"
#include "../include/sgdebug.h"

# if DETAIL_PROFILE
static float    m_fFrameFps[MAX_PROFILE_FRMAE+1];
static int      m_iFrameIdx = 0;
# endif


//! 初始化静态变量
template<> sg_timer* ost_Singleton<sg_timer>::m_ptrSingleton = NULL;


sg_timer::sg_timer()
{
	this->total_elatime = 0;
	this->last_elatime = 0;
	
	this->fps = 0;
	this->fpsstart = 0;
	this->fpscal = 0;
	maxFps = 0;
	averageFpsCounter = 0;
	averageFpsSum = 0;
	m_fCPUUsage  = 0.0f;
	m_formerThreadTime = 0;
	m_fCPUUsageCounter = 0;
	m_dCPUIntervalTime	= 0;
	
	SYSTEM_INFO t_info;
	GetSystemInfo(&t_info);
	m_iCPUNumber = (int)t_info.dwNumberOfProcessors;
}

sg_timer::~sg_timer()
{
}

sg_timer* sg_timer::Instance()
{
	guard;
	
	if( !sg_timer::m_ptrSingleton )
	{

		 sg_timer::m_ptrSingleton = new sg_timer;
		
		osassert( sg_timer::m_ptrSingleton );
	}
	
	return sg_timer::m_ptrSingleton;
	
	unguard;
}

void sg_timer::DInstance()
{
	guard;// sg_timer::DInstance() );
	
	SAFE_DELETE( sg_timer::m_ptrSingleton );
	
	unguard;
}

# define COUNTERPOISE_NUM  1


//-----------------------------------------------------------------------------------------
//目前使用最新简单的不精确方法得到时间，以后要以改进，接口不变。
//-----------------------------------------------------------------------------------------
void sg_timer::framemove()
{
	guard;
	
	static BOOL firsttime = TRUE;
	static LARGE_INTEGER	count;	//! 时钟频率
	static BOOL          t_bHigh = FALSE;
	static LARGE_INTEGER  t_lLasttime;	//! 起始值
	static DWORD          t_iLasttime;


	//! 得到时钟频率起始值
	if( firsttime )
	{

# if 0
		// River move @ 2007-8-25:
		if( QueryPerformanceFrequency( &count ) )
		{
			t_bHigh = TRUE;
			QueryPerformanceCounter( &t_lLasttime );
		}
		else
# endif 
		t_iLasttime = timeGetTime();

		firsttime = FALSE;	//! 己得到起始值


		// 初始值
		last_elatime = 0.001f;

		/*
		t_bHigh = false;
		t_iLasttime = timeGetTime();
		*/

		FILETIME t_create;
		FILETIME t_exit;
		FILETIME t_user;
		FILETIME t_kernel;

		GetThreadTimes(GetCurrentThread(),
						&t_create,
						&t_exit,
						&t_kernel,
						&t_user);

		m_formerThreadTime = *(ULONGLONG*)(&t_kernel) + *(ULONGLONG*)&t_user;

		return;
	}


	//! 采用高精度计时器进行计时
	if( t_bHigh )	//! 得到机器内部时钟频率
	{
		LARGE_INTEGER t_sCurtime;			//! 当前值
		LARGE_INTEGER backtime;			//! 当前值副本



		//! 计算时钟频率经过值
		QueryPerformanceCounter( &t_sCurtime );	//! 取得当前值
		backtime.QuadPart = t_sCurtime.QuadPart;	//! 备份当前值
		t_sCurtime.QuadPart -= t_lLasttime.QuadPart;	//! 计算经过的频率值
		t_lLasttime.QuadPart = backtime.QuadPart;	//! 替换起始值

		last_elatime = float((double)t_sCurtime.QuadPart/(double)count.QuadPart);	//! 计算经过时间（秒）

		if( last_elatime < 0.0001f )
		{
				osassertex( false,va( "last elatime:<%f>, curQuad<%d,%d>,lastQuad<%d,%d>..\n",
				last_elatime,t_sCurtime.HighPart,t_sCurtime.LowPart,
				t_lLasttime.HighPart,t_lLasttime.LowPart ) );
		}

	}
	//! 如果硬件不支持性能计数器，则采用不精确方法(timeGetTime)进行计时
	else
	{

		DWORD      curtime,backtime;

		curtime = timeGetTime();
		if( curtime == t_iLasttime )
		{
			while( curtime == t_iLasttime )
			{
				//osDebugOut( "Equal Timer!!!!\n" );
				curtime = timeGetTime();
			}
		}

		backtime = curtime;
		curtime -= t_iLasttime;
		t_iLasttime = backtime;

		last_elatime = curtime/(float)1000.0;


		// River mod @ 2007-8-25:修改成健壮一些的出错处理方式
		// 用于测试时间出错的次数，如果出错的将数比较多，则assert.
		static int t_iErrorTime = 0;
		if( last_elatime < 0.0f )
		{
		# if DETAIL_PROFILE
			osDebugOut( va( "<%f,%f>",m_fFrameFps[m_iFrameIdx],last_elatime ) );
		# endif
			t_iErrorTime ++;
			osassertex( t_iErrorTime<10,"时间出错的次数过多" );
			last_elatime = 0.001f;
		}

	}

	total_elatime += this->last_elatime;	//! 计算总时间（秒）

	//cal fps.
	if( fpsstart > 1.0f )
	{
		fps = fpscal/this->fpsstart;
		this->fpsstart = 0;
		fpscal = 0;

		if(fps > maxFps){
			maxFps = fps;
		}

		averageFpsCounter += 1.0f;
		averageFpsSum += fps;

	}
	else
	{
		this->fpsstart += this->last_elatime;
		fpscal++;
	}
	

# if DETAIL_PROFILE
	if( m_iFrameIdx < MAX_PROFILE_FRMAE )
	{
		m_fFrameFps[m_iFrameIdx] = 1.0f/last_elatime;
	
		if( last_elatime < 0.00001f )
			osassertex( false,va( "<%f,%f>",m_fFrameFps[m_iFrameIdx],last_elatime ) );

		m_iFrameIdx ++;
	}
# endif


	m_fCPUUsageCounter += get_lastelatime();
	m_dCPUIntervalTime += (double)get_lastelatime();

	if(m_fCPUUsageCounter > 3.0f){

		// 计算 CPU使用率
		//
		FILETIME t_create;
		FILETIME t_exit;
		FILETIME t_user;
		FILETIME t_kernel;

		GetThreadTimes(GetCurrentThread(),
			&t_create,
			&t_exit,
			&t_kernel,
			&t_user);

		const ULONGLONG t_currTime = *(ULONGLONG*)(&t_kernel) + *(ULONGLONG*)&t_user;
		m_fCPUUsage = (float)( (double)(t_currTime - m_formerThreadTime ) * 1.0e-7 / m_dCPUIntervalTime );
		m_formerThreadTime = t_currTime;

		m_dCPUIntervalTime = 0.0f;
		m_fCPUUsageCounter = 0.0f;
	}
	

	return;
		
	unguard;
}

//! 得到某一帧的帧速
float sg_timer::get_framerate( int _idx )
{
# if DETAIL_PROFILE
	if( _idx < MAX_PROFILE_FRMAE )
		return m_fFrameFps[_idx];
	else
		return 0.0f;
# else
	return 0.0f;
# endif
}

