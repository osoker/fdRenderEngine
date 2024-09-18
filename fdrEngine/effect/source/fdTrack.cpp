//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdTrack.cpp
 *
 *  Desc:     处理引擎中的路径数据,用于显示场景中的特效.
 *
 *  His:      River created @ 2004-2-18
 *
 *  "不是东风压倒西风，就是西风压倒东风"
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/fdTrack.h"


//! 初始化track数组的大小。
# define   INIT_TRACKSIZE    64

//! 初始化的track Instance数组的大小
//! River Mod @ 2007-4-6:暂时修改成256,可能还会出错.
# define   INIT_TRACKINSSIZE 256

/** \brief
 *  全局的trackMgr.
 *
 */
osc_trackMgr*       g_ptrTrackMgr = NULL;

//! 声音Track的实例. 
static osc_trackMgr  m_trackIns;


os_track::os_track()
{
	m_iPtNum = 0;
	m_szFname[0] = NULL;
}


/** \brief
*  从一个文件得到一个规迹动画的数据。
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
	// 判断版本号,,,

	READ_MEM_OFF( &m_iPtNum,t_fstart,sizeof( DWORD ) );
	osassert( m_iPtNum>0 );
	m_vec3Interpolate.resize( m_iPtNum );

	// Load file data.
	READ_MEM_OFF( &m_vec3Interpolate[0],t_fstart,sizeof( osVec3D )*m_iPtNum );



	//
	// 长度示表示的是从起始点到当前顶点的矩离。
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

//! 终止当前的规迹动画。
void osc_trackIns::disable_trackAni( void )
{

	m_iPtIdx = 0;
	m_fInterVal = 0;
	m_fMovSpeed = 0.0f;
	m_bLoopAni = FALSE;
	m_bInuse = false;

	m_fTtime = 0.0f;

}


//! 设置当前动画对应的track指针。
void osc_trackIns::set_trackPtr( os_track* _tptr )
{
	guard;

	osassert( _tptr );
	m_ptrTrack = _tptr;

	unguard;
}




/** \brief
*  创建一个trackInstance.
* 
*  \param _loop 　　是否循环播放当前的实例数据。
*  \param _mspeed   当前动画中实体的移动速度。
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
*  得到当前trackIns上的随机的关键点。
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
*  得到当前动画达到的坐标。
*
*  \param _etime      上一帧过去的时间。
*  \param _resVec     返回了动画到达的坐标。
*  \param return bool 如果返回了false,则完成了动画的过程，以后不能再使用这个
*                     规迹动画的实例id了。
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
	// 如果运动的路径矩离已经大于当前的一个track周期。
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
	// 对当前的运动到的位置进行插值。
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

			// River mod @ 2006-7-18: 此处多次修改，容易出错
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
*  从管理器中创建一个Track.
*  
*  \return int 返回我们创建的track的id,使用这个id来操作我们得到的Track.
*  \param      _fname     要创建的trackIns的文件名。
*  \param      _movspeed  要创建的track实例中，使用此track实体的移动速度。
*  \param      _look      是否循环播放当前的track动画。
*  \param      _randS     是否随机初始化位置，用于场景内的使用相同的规道粒子，有不同的效果。
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
	// 先创建track,
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
	// 创建trackInstance.
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
*  从管理器中使一个Track失效。
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
*  从当前的Track中的关键顶点中，得到一个随机的顶点。
*
*  \param _trackId  要得到随机顶点的track instance的id
*  \param _resVec   返回的在当前Track中的随机的顶点。
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
*  得到当前track的动画顶点.
*
*  \param _trackId   要操作的Track实例数据。
*  \param _etime     上一帧过去的时间。
*  \param _resVec    返回当前动画到达的顶点。
*  \return bool      如果是一次性的动画，如果完成了当前的动画过程，则返回false.
*                    否则永远返回真。
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




























































