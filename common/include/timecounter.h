//------------------------------------------------------------------------------------------
/**
 * Filename: timecounter.h
 * Desc:     global time counter func
 * His:      River created @ 11/23 2001.
 *
 * River @ 6/27 2003 �������Ŀ���뵽osokWeb3d������.
 * 
 */
//------------------------------------------------------------------------------------------

#ifndef __TIMECOUNTER_INCLUDE__
#define __TIMECOUNTER_INCLUDE__


# include "windows.h"
# include "mmsystem.h"
# include "stdio.h"
# include "../common.h"
# include <assert.h>



//! ģ������û�ж����m_ptrSingleton
#   pragma warning( disable: 4661)


/** \brief
 *   Singletonģ����.
 *
 *   ATTENTION!!! �̳д�ģ��������ѽ���������������������Ϊ˽�к�����
 *
 *   ���ұ�����дstatic ���� Instance�� Dinstance.
 *   ��Ȼ��assert����
 */
template <typename T> class ost_Singleton
{
protected:
	static T*   m_ptrSingleton;

public:
	// ʵ������ɾ��ʵ���������������д���������д���������ʵ��,assert.
	static T*   Instance( void )
	{
		assert( false );
		return NULL;

		// ���´���Ϊʵ�����������Ĵ��룬��Ϊ������Ľ���������˽�к�����
		// �������¹��ܱ���������������ʵ�֡�
/*
		if( !m_ptrSingleton )
			m_ptrSingleton = new T;
		return m_ptrSingleton;
*/
	}
	static void DInstance( void )
	{
		assert( false );
/*
		if( m_ptrSingleton )
		{
			delete m_ptrSingleton;
			m_ptrSingleton = NULL;
		}
*/
	}

};


//! ȫ�ֵ����ܲ��Թ���:

/********************************************************************************************/
/** \brief ȫ�ֵ�ʱ���֡������
 *  
 *  �������������Ļ�����,��Ӧ����ȫ�ֵ�ʱ����ƺ�֡�ٶȼ���.
 *
 */
/********************************************************************************************/
class COMMON_API sg_timer : public ost_Singleton<sg_timer>
{
private:
	//! �������������ڣ��ܹ���ȥ��ʱ�䡣
	float        total_elatime;                          

	//! ��һ֡�����ڣ��ܹ����ŵ�ʱ�䡣
	float        last_elatime;                           


	float        fps;
	float		 maxFps;

	float		 averageFpsCounter;
	double		 averageFpsSum;

	int          fpscal;
	float        fpsstart;


	//! ��һ֡��Ӧ��ʱ�䡣
	DWORD        m_dwLastTime;


	//@{ ����CPU ռ����ʹ�ñ���
	//! cpu ռ����
	float		m_fCPUUsage;

	//! ���� ������
	float		m_fCPUUsageCounter;

	//! �׶�ʱ�������
	double		m_dCPUIntervalTime;

	//! ֮ǰ�� FILETIME ����¼�߳�ʱ��
	ULONGLONG	m_formerThreadTime;

	//! CPU ����
	int			m_iCPUNumber;

	//@}

	sg_timer();
	~sg_timer();

public:

	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static sg_timer*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void         DInstance( void );


	/** ÿһ֡��������õĺ�����
	 */ 
	void         framemove( void );                      
	
	/** �õ��ӳ���ʼ�����ڹ�ȥ��ʱ��.
	 */
	float        get_talelatime( void )const        { return total_elatime; }

	/** �õ���һ֡��ȥ��ʱ��.
	 */
	float        get_lastelatime( void )const       { return last_elatime ; }

	/** �õ�sg_timer���������fps��Ŀ.
	 */
	float        get_fps( void )const               { return fps; }
	float		 get_maxFps(void)const	 {return maxFps;}
	float		 get_averageFps(void)const {return (float)(averageFpsSum / averageFpsCounter);}

	//! �õ�CPU ʹ����
	float		get_cpuUsage(void)const				{return m_fCPUUsage;}

	//! �õ�CPU ����
	int			get_cpuNum(void)const				{return m_iCPUNumber;}

	/** �õ�ʱ���Identifier.
	 */
	WORD        get_timeId( void )const             { return (WORD)m_dwLastTime; } 

	//! ��ĳ�������������ж�ʱ���ִ��,����������ͣ�������ʹ��һ֡��ȥ��ʱ���Ϊ��
	void        suspend_lastFrameTime( void )  { last_elatime = 0.0f; }

	//! �õ�ĳһ֡��֡��
	float       get_framerate( int _idx );

};




#endif