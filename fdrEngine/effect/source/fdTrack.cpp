//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdTrack.cpp
 *
 *  Desc:     ���������е�·������,������ʾ�����е���Ч.
 *
 *  His:      River created @ 2004-2-18
 *
 *  "���Ƕ���ѹ�����磬��������ѹ������"
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/fdTrack.h"


//! ��ʼ��track����Ĵ�С��
# define   INIT_TRACKSIZE    64

//! ��ʼ����track Instance����Ĵ�С
//! River Mod @ 2007-4-6:��ʱ�޸ĳ�256,���ܻ������.
# define   INIT_TRACKINSSIZE 256

/** \brief
 *  ȫ�ֵ�trackMgr.
 *
 */
osc_trackMgr*       g_ptrTrackMgr = NULL;

//! ����Track��ʵ��. 
static osc_trackMgr  m_trackIns;


os_track::os_track()
{
	m_iPtNum = 0;
	m_szFname[0] = NULL;
}


/** \brief
*  ��һ���ļ��õ�һ���漣���������ݡ�
*/
bool os_track::load_trackFromfile( const char* _fname )
{
	guard;

	char      t_cHeader[4];
	DWORD     t_dwVersion;
	osVec3D   t_vec;
	int       t_i;
	BYTE*     t_fstart;

	osassert( _fname&&(_fname[0]) );
	
	t_cHeader[3] = NULL;

	int  t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_i = read_fileToBuf( (char*)_fname,t_fstart,TMP_BUFSIZE );
	if( t_i<0 )
		return false;

	// confirm file header.
	READ_MEM_OFF( t_cHeader,t_fstart,sizeof( char )*4 );
	if( strcmp( t_cHeader,"csf" ) )
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}

	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	// �жϰ汾��,,,

	READ_MEM_OFF( &m_iPtNum,t_fstart,sizeof( DWORD ) );
	osassert( m_iPtNum>0 );
	m_vec3Interpolate.resize( m_iPtNum );

	// Load file data.
	READ_MEM_OFF( &m_vec3Interpolate[0],t_fstart,sizeof( osVec3D )*m_iPtNum );



	//
	// ����ʾ��ʾ���Ǵ���ʼ�㵽��ǰ����ľ��롣
	this->m_vecDistance.resize( m_iPtNum-1  );
	for( int i=0;i<m_iPtNum-1;i++ )
	{
		t_vec = m_vec3Interpolate[i+1]-m_vec3Interpolate[i];
		m_vecDistance[i] = osVec3Length( &t_vec );

		if( i>0 )
			m_vecDistance[i] += m_vecDistance[i-1];
	}

	END_USEGBUF( t_iGBufIdx );

	// catch filename.
	strcpy( m_szFname,_fname );

	return  true;

	unguard;
}






osc_trackIns::osc_trackIns()
{
	m_ptrTrack = NULL;
	disable_trackAni();
}

//! ��ֹ��ǰ�Ĺ漣������
void osc_trackIns::disable_trackAni( void )
{

	m_iPtIdx = 0;
	m_fInterVal = 0;
	m_fMovSpeed = 0.0f;
	m_bLoopAni = FALSE;
	m_bInuse = false;

	m_fTtime = 0.0f;

}


//! ���õ�ǰ������Ӧ��trackָ�롣
void osc_trackIns::set_trackPtr( os_track* _tptr )
{
	guard;

	osassert( _tptr );
	m_ptrTrack = _tptr;

	unguard;
}




/** \brief
*  ����һ��trackInstance.
* 
*  \param _loop �����Ƿ�ѭ�����ŵ�ǰ��ʵ�����ݡ�
*  \param _mspeed   ��ǰ������ʵ����ƶ��ٶȡ�
*  
*/
void osc_trackIns::create_trackIns( BOOL _loop,float _mspeed,BOOL _randS )
{
	guard;

	osassert( _mspeed >= 0 );

	m_bLoopAni = _loop;
	m_fMovSpeed = _mspeed;

	if( _randS )
		m_fTtime = float(OSRAND % 20);

	this->m_bInuse = true;

	unguard;
}



/** \brief
*  �õ���ǰtrackIns�ϵ�����Ĺؼ��㡣
*/
void osc_trackIns::get_trackRandKeyPos( osVec3D& _resVec )
{
	guard;

	int    t_i;

	t_i = getrand_fromintscope( 0,m_ptrTrack->m_iPtNum-1 );

	_resVec = m_ptrTrack->m_vec3Interpolate[t_i];

	return;

	unguard;
}




/** \brief
*  �õ���ǰ�����ﵽ�����ꡣ
*
*  \param _etime      ��һ֡��ȥ��ʱ�䡣
*  \param _resVec     �����˶�����������ꡣ
*  \param return bool ���������false,������˶����Ĺ��̣��Ժ�����ʹ�����
*                     �漣������ʵ��id�ˡ�
*/
bool osc_trackIns::get_aniVec( float _etime,osVec3D& _resVec )
{
	guard;

	float       t_dis,t_fInter;
	int         t_i,t_ptNum;
	osVec3D     t_vec;

	osassert( m_ptrTrack );

	m_fTtime += _etime;
	t_dis = m_fTtime*m_fMovSpeed;

	t_ptNum = m_ptrTrack->m_iPtNum;

	//
	// ����˶���·�������Ѿ����ڵ�ǰ��һ��track���ڡ�
	if( t_dis > m_ptrTrack->m_vecDistance[t_ptNum-2] )
	{
		if( !m_bLoopAni )
		{
			this->disable_trackAni();
			return false;
		}
		else
			t_dis = float_mod( t_dis,m_ptrTrack->m_vecDistance[t_ptNum-2] );

	}



	//
	// �Ե�ǰ���˶�����λ�ý��в�ֵ��
	for( t_i=0;t_i<t_ptNum-1;t_i++ )
	{
		float t1 = m_ptrTrack->m_vecDistance[t_i];
		if( t1 >= t_dis )
		{
			m_iPtIdx = t_i;

			t_fInter = m_ptrTrack->m_vecDistance[t_i] - t_dis;

			if( t_i > 0 )
				t_fInter /= (m_ptrTrack->m_vecDistance[t_i]-m_ptrTrack->m_vecDistance[t_i-1]);
			else
				t_fInter /= m_ptrTrack->m_vecDistance[t_i];

			t_vec = m_ptrTrack->m_vec3Interpolate[t_i] - m_ptrTrack->m_vec3Interpolate[t_i+1];

			// River mod @ 2006-7-18: �˴�����޸ģ����׳���
			t_vec *= t_fInter;
			_resVec = t_vec + m_ptrTrack->m_vec3Interpolate[t_i + 1];

			break;
		}
	}

	return true;

	unguard;
}








osc_trackMgr::osc_trackMgr()
{
	

	g_ptrTrackMgr = this;
}




/** \brief
*  �ӹ������д���һ��Track.
*  
*  \return int �������Ǵ�����track��id,ʹ�����id���������ǵõ���Track.
*  \param      _fname     Ҫ������trackIns���ļ�����
*  \param      _movspeed  Ҫ������trackʵ���У�ʹ�ô�trackʵ����ƶ��ٶȡ�
*  \param      _look      �Ƿ�ѭ�����ŵ�ǰ��track������
*  \param      _randS     �Ƿ������ʼ��λ�ã����ڳ����ڵ�ʹ����ͬ�Ĺ�����ӣ��в�ͬ��Ч����
*/
int osc_trackMgr::create_track( const char* _fname,float _movspeed,BOOL _loop,BOOL _randS/* = FALSE*/ )
{
	guard;

	if(m_vecTrack.size() == 0){
		m_vecTrack.resize( INIT_TRACKSIZE );
		m_vecTrackIns.resize( INIT_TRACKINSSIZE );
	}

    os_track*   t_ptrTrack;

	s_string    t_szTrackName = "data\\";
	t_szTrackName += _fname;
	
	//
	// �ȴ���track,
	t_ptrTrack = NULL;
	int t_i;
	for(  t_i=0;t_i<(int)m_vecTrack.size();t_i++ )
	{
		if( m_vecTrack[t_i].m_szFname[0] == NULL )
			continue;

		if( strcmp( m_vecTrack[t_i].m_szFname,t_szTrackName.c_str() ) )
			continue;

		t_ptrTrack = &m_vecTrack[t_i];
		
		break;
	}


	if( t_ptrTrack == NULL )
	{
		for( t_i=0;t_i<(int)m_vecTrack.size();t_i++ )
		{
			if( m_vecTrack[t_i].m_szFname[0] != NULL )
				continue;

			if( !m_vecTrack[t_i].load_trackFromfile( t_szTrackName.c_str() ) )
				return -1;

			t_ptrTrack = &m_vecTrack[t_i];
			break;
		}
	}


	// 
	// ����trackInstance.
	for( t_i=0;t_i<(int)m_vecTrackIns.size();t_i++ )
	{
		if( m_vecTrackIns[t_i].is_insInuse() )
			continue;

		m_vecTrackIns[t_i].set_trackPtr( t_ptrTrack );
		m_vecTrackIns[t_i].create_trackIns( _loop,_movspeed,_randS );

		break;
	}

	if( t_i==m_vecTrackIns.size() )
	{
		osDebugOut( "Create track failed...\n" );
		t_i = -1;
	}

	return t_i;

	unguard;
}


/** \brief
*  �ӹ�������ʹһ��TrackʧЧ��
*/
void osc_trackMgr::delete_track( int _trackId )
{
	guard;

	if( !m_vecTrackIns[_trackId].is_insInuse() )
		osassert( false );

	m_vecTrackIns[_trackId].disable_trackAni();

	return;

	unguard;
}



/** \brief
*  �ӵ�ǰ��Track�еĹؼ������У��õ�һ������Ķ��㡣
*
*  \param _trackId  Ҫ�õ���������track instance��id
*  \param _resVec   ���ص��ڵ�ǰTrack�е�����Ķ��㡣
*/
void osc_trackMgr::get_randomVec( int _trackId,osVec3D& _resVec )
{
	guard;

	osassert( _trackId >=0 );

	if( !m_vecTrackIns[_trackId].is_insInuse() )
		osassert( false );

	m_vecTrackIns[_trackId].get_trackRandKeyPos( _resVec );

	return;

	unguard;
}


/** \brief
*  �õ���ǰtrack�Ķ�������.
*
*  \param _trackId   Ҫ������Trackʵ�����ݡ�
*  \param _etime     ��һ֡��ȥ��ʱ�䡣
*  \param _resVec    ���ص�ǰ��������Ķ��㡣
*  \return bool      �����һ���ԵĶ������������˵�ǰ�Ķ������̣��򷵻�false.
*                    ������Զ�����档
*/
bool osc_trackMgr::get_aniVec( int _trackId,float _etime,osVec3D& _resVec )
{
	guard;

	osassert( _trackId >=0 );

	if( !m_vecTrackIns[_trackId].is_insInuse() )
		osassert( false );

	if( !m_vecTrackIns[_trackId].get_aniVec( _etime,_resVec ) )
		return false;


	return true;

	unguard;
}




























































