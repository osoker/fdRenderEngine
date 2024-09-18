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


//! ��ʼ����̬����
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
//Ŀǰʹ�����¼򵥵Ĳ���ȷ�����õ�ʱ�䣬�Ժ�Ҫ�ԸĽ����ӿڲ��䡣
//-----------------------------------------------------------------------------------------
void sg_timer::framemove()
{
	guard;
	
	static BOOL firsttime = TRUE;
	static LARGE_INTEGER	count;	//! ʱ��Ƶ��
	static BOOL          t_bHigh = FALSE;
	static LARGE_INTEGER  t_lLasttime;	//! ��ʼֵ
	static DWORD          t_iLasttime;


	//! �õ�ʱ��Ƶ����ʼֵ
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

		firsttime = FALSE;	//! ���õ���ʼֵ


		// ��ʼֵ
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


	//! ���ø߾��ȼ�ʱ�����м�ʱ
	if( t_bHigh )	//! �õ������ڲ�ʱ��Ƶ��
	{
		LARGE_INTEGER t_sCurtime;			//! ��ǰֵ
		LARGE_INTEGER backtime;			//! ��ǰֵ����



		//! ����ʱ��Ƶ�ʾ���ֵ
		QueryPerformanceCounter( &t_sCurtime );	//! ȡ�õ�ǰֵ
		backtime.QuadPart = t_sCurtime.QuadPart;	//! ���ݵ�ǰֵ
		t_sCurtime.QuadPart -= t_lLasttime.QuadPart;	//! ���㾭����Ƶ��ֵ
		t_lLasttime.QuadPart = backtime.QuadPart;	//! �滻��ʼֵ

		last_elatime = float((double)t_sCurtime.QuadPart/(double)count.QuadPart);	//! ���㾭��ʱ�䣨�룩

		if( last_elatime < 0.0001f )
		{
				osassertex( false,va( "last elatime:<%f>, curQuad<%d,%d>,lastQuad<%d,%d>..\n",
				last_elatime,t_sCurtime.HighPart,t_sCurtime.LowPart,
				t_lLasttime.HighPart,t_lLasttime.LowPart ) );
		}

	}
	//! ���Ӳ����֧�����ܼ�����������ò���ȷ����(timeGetTime)���м�ʱ
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


		// River mod @ 2007-8-25:�޸ĳɽ�׳һЩ�ĳ�����ʽ
		// ���ڲ���ʱ�����Ĵ������������Ľ����Ƚ϶࣬��assert.
		static int t_iErrorTime = 0;
		if( last_elatime < 0.0f )
		{
		# if DETAIL_PROFILE
			osDebugOut( va( "<%f,%f>",m_fFrameFps[m_iFrameIdx],last_elatime ) );
		# endif
			t_iErrorTime ++;
			osassertex( t_iErrorTime<10,"ʱ�����Ĵ�������" );
			last_elatime = 0.001f;
		}

	}

	total_elatime += this->last_elatime;	//! ������ʱ�䣨�룩

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

		// ���� CPUʹ����
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

//! �õ�ĳһ֡��֡��
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

