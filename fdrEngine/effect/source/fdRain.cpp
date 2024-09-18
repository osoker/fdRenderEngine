//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdRain.cpp
 *
 *  Desc:     场景中下雨特效的处理。
 *
 *  His:      River created @ 2004-7-3
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdRain.h"
# include "../include/fdBillboard.h"
# include "../../backpipe/include/mainEngine.h"


static DWORD fg_dwColorA = 0x22afbfbf;
static DWORD fg_dwColorB = 0x55afbfbf;







# define RAIN_SCOPE     4
//! RAIN SCOPER的四十倍,本来用四倍的就可以了，但为了增加雨滴的随机性，
//! 增大这个值。
# define RAIN_SCOPEF    160

//! ATTENTION TO FIX: 当相机的fov变化的时候，此值调整？？？
//! 定义雨滴的下落半径放大倍数,
# define RAINSCOPE_MULTINUM  2.0f


//===============================
//! 雪相关参数
# define  MAX_SNOWNUM 1000

//static const DWORD fg_dwSnowColor = 0xffffffff;
# define SNOW_SCOPE	10


static float CamDistance = -6;
osc_rain::osc_rain()
{
	m_fHeight = m_fRainLength = m_fWidth = 0.0f;

	m_vec3Velocity = osVec3D( 0.0f,0.0f,0.0f );

	m_ptrMPipe = NULL;

	m_vecRainVer = NULL;
	m_vecRainPos = NULL;
	m_vecSpeed = NULL;

	m_bRainning = false;
	

	m_vecRainVerOffset[0].m_vecPos = osVec3D( -0.007f, -0.3f, 0.0f );
	m_vecRainVerOffset[1].m_vecPos = osVec3D( 0.007f, -0.3f, 0.0f  );
	m_vecRainVerOffset[2].m_vecPos = osVec3D( 0.007f,  0.3f, 0.0f   );
	m_vecRainVerOffset[3].m_vecPos = osVec3D(-0.007f,  0.3f, 0.0f   );

	// 
	// 实际下雨效果用到的雨点大小
	m_vecRainVerStart[0].m_vecPos = m_vecRainVerOffset[0].m_vecPos;
	m_vecRainVerStart[1].m_vecPos = m_vecRainVerOffset[1].m_vecPos;
	m_vecRainVerStart[2].m_vecPos = m_vecRainVerOffset[2].m_vecPos;
	m_vecRainVerStart[3].m_vecPos = m_vecRainVerOffset[3].m_vecPos;


}

osc_rain::~osc_rain()
{

	SAFE_DELETE_ARRAY( m_vecRainVer );
	SAFE_DELETE_ARRAY( m_vecRainPos );
	SAFE_DELETE_ARRAY( m_vecSpeed );

}



//! 对下雨效果的FrameMove处理,更新每一个雨滴的位置，为每一个雨滴计算新的渲染多边形。
void osc_rain::frame_moveRain( osVec3D& _campos )
{
	guard;

	float     t_fEtime;   
	osVec3D   t_vec3CamPos;

	t_fEtime = sg_timer::Instance()->get_lastelatime();

	// 只要雨滴渲染或是产生新的雨滴，就会把这个变量变为真。
	m_bRainning = false;
	for( int t_i=0;t_i<m_sRainStart.m_iRainDotNum;t_i ++ )
	{
		if( m_vecRainPos[t_i].y > 0.0f )
		{
			m_vecRainPos[t_i].y -= (m_vecSpeed[t_i]*t_fEtime);

			// 
			// 处理渲染用到的数据。
			m_vecRainVer[t_i*4+0].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[0].m_vecPos;
			m_vecRainVer[t_i*4+1].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[1].m_vecPos;
			m_vecRainVer[t_i*4+2].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[2].m_vecPos;
			m_vecRainVer[t_i*4+3].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[3].m_vecPos;

			m_bRainning = true;
		}
		else
		{
			// 
			// ATTENTION TO FIX:当前相机离地面很近的时候，雨滴掉落地下的效果
			//                  会看起来很大，处理。
			// 加入雨滴掉落到地下后的效果。
			m_vecRainPos[t_i] *= RAINSCOPE_MULTINUM;
			m_vecRainPos[t_i] += _campos;
			osc_bbManager::Instance()->
				create_rainEff( m_vecRainPos[t_i] );

			//
			// 如果停止了下雨，则不再产生新的雨滴。
			if( m_bStopRaining )
				continue;

			// 初始化新的雨滴效果。
			m_vecRainPos[t_i].y = 6.0f+OSRAND%7;
			m_vecRainPos[t_i].x = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);
			m_vecRainPos[t_i].z = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);

			// 设置新的移动速度。
			m_vecSpeed[t_i]			   = (float)(OSRAND%3)*m_sRainStart.m_fRainMoveSpeed;
			if(m_vecSpeed[t_i] == 0)
				m_vecSpeed[t_i] = m_sRainStart.m_fRainMoveSpeed;

			// 
			// 处理渲染用到的数据。
			m_vecRainVer[t_i*4+0].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[0].m_vecPos;
			m_vecRainVer[t_i*4+1].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[1].m_vecPos;
			m_vecRainVer[t_i*4+2].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[2].m_vecPos;
			m_vecRainVer[t_i*4+3].m_vecPos = 
				m_vecRainPos[t_i] + m_vecRainVerStart[3].m_vecPos;


			m_bRainning = true;
		}
	}
	
	unguard;
}



/** \brief
*  渲染当前的雨效果。
*
*  \param _pitch  当前场景中主相机的pitch值。
*  \param _pos    当前场景中相机的位置。
*/
void osc_rain::render_rain( osVec3D& _pos,float _pitch )
{
	guard;

	int    t_iStartVer;

	// 不下雨状态，并且天空中没有雨滴，返回。
	if( (!m_bRainning) )
		return ;

	// 先对当前的雨滴进行Frame move,填充顶点缓冲区。
	frame_moveRain( _pos );
	t_iStartVer = m_ptrMPipe->fill_dnamicVB( m_iVBid,
		m_vecRainVer,sizeof( os_sceVerDiffuse ),m_sRainStart.m_iRainDotNum*4 );
	osassert( t_iStartVer >= 0 );


	//
	// 设置渲染雨滴效果时使用的相机,并渲染雨滴效果。
	set_rainRenderCam( _pitch );
	m_sPolygon.m_sBufData.m_iVertexStart = t_iStartVer;
	bool t_b = m_ptrMPipe->render_polygon( &m_sPolygon );
	osassert( t_b );

	unguard;
}


//! 设置渲染雨滴效果使用的相机。
void osc_rain::set_rainRenderCam( float _pitch )
{
	guard;

	osassert( m_ptrMPipe );

	static float     t_sfP2 = OS_PI/2.0f;

	osMatrix         t_camMat;
	osVec3D          t_vec3Eye,t_vec3At,t_vec3Up;

	t_vec3Eye = osVec3D( 0.0f,0.0f,-8.0f );
	t_vec3At =  osVec3D( 0.0f,6.0f, 0.0f );
	t_vec3Up =  osVec3D( 0.0f,1.0f, 0.0f );

	osMatrixRotationX( &t_camMat,float(t_sfP2-fabs(_pitch)) );
	osVec3Transform( &t_vec3Eye,&t_vec3Eye,&t_camMat );
	t_vec3Eye += t_vec3At;

	// 
	// 得到并设置视矩阵。
	osMatrixLookAtLH( &t_camMat,&t_vec3Eye,&t_vec3At,&t_vec3Up );
	m_ptrMPipe->set_viewMatrix( t_camMat );
	// 
	// River @ 2006-5-29:世界矩阵为identity
	m_ptrMPipe->set_worldMatrix( osc_bbManager::m_smatMatrix );

	unguard;
}




//! 初始化下雨效果的设备相关数据。
void osc_rain::init_rainDevdepData( void )
{
	guard;


	// 
	// 创建设备相关的数据,动态顶点缓冲区的数据和effect的数据。
	m_iVBid = m_ptrMPipe->create_dynamicVB( 
		FVF_LINEVERTEX,sizeof( os_sceVerDiffuse )*MAX_RAINNUM*4 );
	osassert( m_iVBid >= 0 );

	m_iEffectId = g_shaderMgr->add_shader( "rain", osc_engine::get_shaderFile( "line" ).c_str() );
	if( m_iEffectId < 0 )
		m_iEffectId = g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );
	osassert( m_iEffectId>=0 );

	// 创建Index buffer数据。
	int    t_iGBufIdx = -1;
	WORD*    t_idxData = (WORD*)START_USEGBUF( t_iGBufIdx );
	for( int t_i=0;t_i<MAX_RAINNUM;t_i ++ )
	{
		t_idxData[t_i*6+0] = t_i*4;
		t_idxData[t_i*6+1] = t_i*4+2;
		t_idxData[t_i*6+2] = t_i*4+1;

		t_idxData[t_i*6+3] = t_i*4;
		t_idxData[t_i*6+4] = t_i*4+3;
		t_idxData[t_i*6+5] = t_i*4+2;

	}
	m_iIBid = m_ptrMPipe->create_staticIB( t_idxData,MAX_RAINNUM*6 );
	osassert( m_iIBid>= 0 );
	END_USEGBUF( t_iGBufIdx );


	m_sPolygon.m_sBufData.m_iIbufId = m_iIBid;
	m_sPolygon.m_sBufData.m_iVbufId = m_iVBid;

	m_sPolygon.m_iShaderId = m_iEffectId;


	return;

	unguard;
}



//! 初始化雨滴效果的数据。
void osc_rain::init_rainData( osc_middlePipe* _mpipe )
{
	guard;

	osassert( _mpipe );

	m_ptrMPipe = _mpipe;


	//
	// 如果顶点数据为空，分配出新的空间。
	m_vecRainVer = new os_sceVerDiffuse[MAX_RAINNUM*4];
	m_vecRainPos = new osVec3D[MAX_RAINNUM];
	m_vecSpeed  =  new float[MAX_RAINNUM];


	//
	// 初始化并填充我们渲染polygon.
	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_sceVerDiffuse );
	m_sPolygon.m_iVerNum = MAX_RAINNUM*4;
	m_sPolygon.m_iPriNum = MAX_RAINNUM*2;


	// 初始化下雨效果的设备相关数据。
	init_rainDevdepData();


	// 当前下雨状态
	m_bRainning = false;

	unguard;
}



//! 开始下雨效果。
void osc_rain::start_rain( os_rainStruct* _rs/* = NULL*/ )
{
	guard;

	if( _rs )
		memcpy( &m_sRainStart,_rs,sizeof( os_rainStruct ) );
	else
	{
		os_rainStruct   t_rs;

		// 使用默认的雨点结构。
		memcpy( &m_sRainStart,&t_rs,sizeof( os_rainStruct ) );
	}

	osassert( m_sRainStart.m_iRainDotNum >= 0 );
	osassert( m_sRainStart.m_iRainDotNum <= 500 ); 

	// 处理初始化的雨点大小和颜色
	for( int t_i=0;t_i<4;t_i ++ )
	{
		m_vecRainVerStart[t_i].m_vecPos.x = m_vecRainVerOffset[t_i].m_vecPos.x*m_sRainStart.m_fXScale;
		m_vecRainVerStart[t_i].m_vecPos.y = m_vecRainVerOffset[t_i].m_vecPos.y*m_sRainStart.m_fYScale;
		m_vecRainVerStart[t_i].m_vecPos.z = m_vecRainVerOffset[t_i].m_vecPos.z;
	}
	fg_dwColorB = m_sRainStart.m_dwBottomColor;
	fg_dwColorA = m_sRainStart.m_dwTopColor;



	//
	// 初始化每一个雨滴的位置和颜色。
	// 雨滴使用一定范围内的雨滴，渲染雨滴效果的时候，使用特别的相机角度。
	// 这个相机的高度和位置是固定的，根据主相机的pitch值来做出这个相机不同
	// 的pitch值来。 
	for( int t_i=0;t_i<m_sRainStart.m_iRainDotNum;t_i ++ )
	{
		m_vecRainVer[t_i*4+0].m_color = fg_dwColorB;
		m_vecRainVer[t_i*4+1].m_color = fg_dwColorB;
		m_vecRainVer[t_i*4+2].m_color = fg_dwColorA;
		m_vecRainVer[t_i*4+3].m_color = fg_dwColorA;

		// 初始化每一个雨滴效果的位置。
		m_vecRainPos[t_i].x = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);
		m_vecRainPos[t_i].z = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);
		m_vecRainPos[t_i].y = (float)( OSRAND%13 );


		m_vecSpeed[t_i]			   = (float)(OSRAND%3)*m_sRainStart.m_fRainMoveSpeed;
		if(m_vecSpeed[t_i] == 0)
			m_vecSpeed[t_i] = m_sRainStart.m_fRainMoveSpeed;

	}

	// 当前下雨雨点数目需要渲染的多边形数目
	m_sPolygon.m_iVerNum = m_sRainStart.m_iRainDotNum*4;
	m_sPolygon.m_iPriNum = m_sRainStart.m_iRainDotNum*2;

	// 设置下雨状态为开。
	m_bRainning = true;
	m_bStopRaining = false;

	unguard;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// 场景内下雪相关的代码
//
//////////////////////////////////////////////////////////////////////////////////////////////
osc_snowStorm::osc_snowStorm()
{
	m_vecSnowData = NULL;
	m_ptrMPipe = NULL;
	m_vecSnowPos = NULL;
	m_vecSpeed = NULL;

	m_iVBid = -1;
	m_iIBid = -1;
	m_iEffectId = -1;

	m_vecSnowRenderVer = NULL;
	m_bStopSnowing = true;
	m_bSnowing = false;

	mSnowSetting.mSnowHeight = 6.0f;
	mSnowSetting.mSnowLower = -10.0f;
	mSnowSetting.mSnowSize = 0.15f;
	mSnowSetting.mSnowNr = 50;
	mSnowSetting.mVelocity = osVec3D(0.0f,-20.0f,0.0f);
	mSnowSetting.mVelocityVariation = osVec3D(5.0f, -10.0f, -4.0f);


	m_vecSnowVerOffset[0].m_vec3Pos = osVec3D( -mSnowSetting.mSnowSize, -mSnowSetting.mSnowSize, 0.0f );
	m_vecSnowVerOffset[1].m_vec3Pos = osVec3D( mSnowSetting.mSnowSize, -mSnowSetting.mSnowSize, 0.0f  );
	m_vecSnowVerOffset[2].m_vec3Pos = osVec3D( mSnowSetting.mSnowSize,  mSnowSetting.mSnowSize, 0.0f   );
	m_vecSnowVerOffset[3].m_vec3Pos = osVec3D(-mSnowSetting.mSnowSize,  mSnowSetting.mSnowSize, 0.0f   );
}

osc_snowStorm::~osc_snowStorm()
{
	SAFE_DELETE_ARRAY( m_vecSnowData );
	SAFE_DELETE_ARRAY( m_vecSnowPos );
	SAFE_DELETE_ARRAY( m_vecSpeed );
	SAFE_DELETE_ARRAY(m_vecSnowRenderVer);
}


//! 初始化整个下雪的管理器
void osc_snowStorm::init_snowStormMgr( osc_middlePipe* _mpipe )
{
	guard;
	//
	// 如果顶点数据为空，分配出新的空间。
	m_vecSnowRenderVer = new os_SnowVertex[MAX_SNOWNUM*4];
	m_vecSnowPos = new osVec3D[MAX_SNOWNUM];
	m_vecSpeed  =  new osVec3D[MAX_SNOWNUM];


	//
	// 初始化并填充我们渲染polygon.
	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_SnowVertex );
	m_sPolygon.m_iVerNum = MAX_SNOWNUM*4;
	m_sPolygon.m_iPriNum = MAX_SNOWNUM*2;

	osassert(_mpipe);
	m_ptrMPipe = _mpipe;
	// 
	// 创建设备相关的数据,动态顶点缓冲区的数据和effect的数据。
	m_iVBid = m_ptrMPipe->create_dynamicVB( 
		FVF_SNOWVERTEX,sizeof( os_SnowVertex )*MAX_SNOWNUM*4 );
	osassert( m_iVBid >= 0 );

	m_iEffectId = g_shaderMgr->add_shader( "snow", osc_engine::get_shaderFile( "snow" ).c_str() );
	if( m_iEffectId < 0 )
		m_iEffectId = g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );
	osassert( m_iEffectId>=0 );

	// 创建Index buffer数据。
	int   t_iGBufIdx = -1;
	WORD*    t_idxData = (WORD*)START_USEGBUF( t_iGBufIdx );
	for( int t_i=0;t_i<MAX_SNOWNUM;t_i ++ )
	{
		t_idxData[t_i*6+0] = t_i*4;
		t_idxData[t_i*6+1] = t_i*4+2;
		t_idxData[t_i*6+2] = t_i*4+1;

		t_idxData[t_i*6+3] = t_i*4;
		t_idxData[t_i*6+4] = t_i*4+3;
		t_idxData[t_i*6+5] = t_i*4+2;

	}
	m_iIBid = m_ptrMPipe->create_staticIB( t_idxData,MAX_SNOWNUM*6 );
	osassert( m_iIBid>= 0 );

	m_sPolygon.m_sBufData.m_iIbufId = m_iIBid;
	m_sPolygon.m_sBufData.m_iVbufId = m_iVBid;

	m_sPolygon.m_iShaderId = m_iEffectId;

	for( int t_i=0;t_i<MAX_SNOWNUM;t_i ++ )
	{
		m_vecSnowRenderVer[t_i*4+0].m_vec2Uv = osVec2D(0,0);
		m_vecSnowRenderVer[t_i*4+1].m_vec2Uv = osVec2D(1,0);
		m_vecSnowRenderVer[t_i*4+2].m_vec2Uv = osVec2D(1,1);
		m_vecSnowRenderVer[t_i*4+3].m_vec2Uv =  osVec2D(0,1);
	}

	END_USEGBUF( t_iGBufIdx );
	m_bSnowing = false;
	m_bStopSnowing = true;

	return;

	unguard;
}

//! 开始下雪
void osc_snowStorm::start_snowStorm( os_snow& snowSetting)
{
	guard;
	mSnowSetting = snowSetting;


	for( int t_i=0;t_i<mSnowSetting.mSnowNr;t_i ++ )
	{
		m_vecSnowVerOffset[0].m_vec3Pos = osVec3D( -mSnowSetting.mSnowSize, -mSnowSetting.mSnowSize, 0.0f );
		m_vecSnowVerOffset[1].m_vec3Pos = osVec3D( mSnowSetting.mSnowSize, -mSnowSetting.mSnowSize, 0.0f  );
		m_vecSnowVerOffset[2].m_vec3Pos = osVec3D( mSnowSetting.mSnowSize,  mSnowSetting.mSnowSize, 0.0f   );
		m_vecSnowVerOffset[3].m_vec3Pos = osVec3D(-mSnowSetting.mSnowSize,  mSnowSetting.mSnowSize, 0.0f   );

		

		// 初始化每一个雨滴效果的位置。
		m_vecSnowPos[t_i].x = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE);
		m_vecSnowPos[t_i].z = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE+SNOW_SCOPE/2);
		m_vecSnowPos[t_i].y = mSnowSetting.mSnowHeight;


		m_vecSpeed[t_i]	 = mSnowSetting.mVelocity + ((float)OSRAND/RAND_MAX-0.5f)*2*mSnowSetting.mVelocityVariation;
		if(m_vecSpeed[t_i] == 0)
			m_vecSpeed[t_i] = mSnowSetting.mVelocity;

	}



	// 设置下雪状态为开。
	m_bSnowing = true;
	m_bStopSnowing = false;


	return;

	unguard;
}

//! 结束下雪
void osc_snowStorm::stop_snowStorm( void )
{
	guard;
	m_bStopSnowing = true;
	return;

	unguard;
}

/** \brief
*  渲染当前的雪效果。
*
*  \param _pitch  当前场景中主相机的pitch值。
*  \param _pos    当前场景中相机的位置。
*/
void osc_snowStorm::render_snowStorm( osVec3D& _pos,float _pitch )
{
	guard;

	int    t_iStartVer;

	// 不下雨状态，并且天空中没有雨滴，返回。
	if( (!m_bSnowing) )
		return ;

	// 先对当前的雨滴进行Frame move,填充顶点缓冲区。
	frame_moveSnow( _pos );
	t_iStartVer = m_ptrMPipe->fill_dnamicVB( m_iVBid,
		m_vecSnowRenderVer,sizeof( os_SnowVertex ),mSnowSetting.mSnowNr*4 );
	osassert( t_iStartVer >= 0 );


	//
	// 设置渲染雨滴效果时使用的相机,并渲染雨滴效果。
	set_snowRenderCam( _pitch );
	m_sPolygon.m_sBufData.m_iVertexStart = t_iStartVer;
	m_sPolygon.m_iVerNum = mSnowSetting.mSnowNr*4;
	m_sPolygon.m_iPriNum = mSnowSetting.mSnowNr*2;
	bool t_b = m_ptrMPipe->render_polygon( &m_sPolygon );
	osassert( t_b );

	unguard;
}


//! 得到是否下雪
bool osc_snowStorm::get_weatherSnowing( os_snow* snowSetting /*= NULL*/)
{
	guard;

	if (snowSetting!=NULL)
	{
		*snowSetting = mSnowSetting;
	}
	
	return !m_bStopSnowing;

	unguard;
}

//! 设置渲染雪效果使用的相机。
void osc_snowStorm::set_snowRenderCam( float _pitch )
{
	guard;

	osassert( m_ptrMPipe );

	//_pitch = 0;
	static float     t_sfP2 = OS_PI/2.0f;

	osMatrix         t_camMat;
	osVec3D          t_vec3Eye,t_vec3At,t_vec3Up;

	t_vec3Eye = osVec3D( 0.0f,0.0f,CamDistance );
	t_vec3At =  osVec3D( 0.0f,0.0f, 0.0f );
	t_vec3Up =  osVec3D( 0.0f,1.0f, 0.0f );

	osMatrixRotationX( &t_camMat,float(t_sfP2-fabs(_pitch)) );
	osVec3Transform( &t_vec3Eye,&t_vec3Eye,&t_camMat );
	t_vec3Eye += t_vec3At;

	// 
	// 得到并设置视矩阵。
	osMatrixLookAtLH( &t_camMat,&t_vec3Eye,&t_vec3At,&t_vec3Up );
	m_ptrMPipe->set_viewMatrix( t_camMat );
	// 
	// River @ 2006-5-29:世界矩阵为identity
	m_ptrMPipe->set_worldMatrix( osc_bbManager::m_smatMatrix );

	unguard;
}

void osc_snowStorm::frame_moveSnow( osVec3D& _campos )
{
	guard;

	float     t_fEtime;   
	osVec3D   t_vec3CamPos;

	t_fEtime = sg_timer::Instance()->get_lastelatime();

	// 只要雨滴渲染或是产生新的雨滴，就会把这个变量变为真。
	m_bSnowing = false;
	//osColor snowColor = 0xff00ffff;
	for( int t_i=0;t_i<mSnowSetting.mSnowNr;t_i ++ )
	{
		if( m_vecSnowPos[t_i].y > mSnowSetting.mSnowLower &&m_vecSnowPos[t_i].y <= mSnowSetting.mSnowHeight)
		{
			m_vecSnowPos[t_i] += (m_vecSpeed[t_i]*t_fEtime);

			// 
			// 处理渲染用到的数据。
			m_vecSnowRenderVer[t_i*4+0].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[0].m_vec3Pos;
			m_vecSnowRenderVer[t_i*4+1].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[1].m_vec3Pos;
			m_vecSnowRenderVer[t_i*4+2].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[2].m_vec3Pos;
			m_vecSnowRenderVer[t_i*4+3].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[3].m_vec3Pos;
			m_bSnowing = true;
		}
		else
		{
			//
			// 如果停止了下雨，则不再产生新的雨滴。
			if( m_bStopSnowing )
				continue;

			// 初始化新的雨滴效果。
			m_vecSnowPos[t_i].y = 6;//6.0f+OSRAND%7;
			m_vecSnowPos[t_i].x = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE);
			m_vecSnowPos[t_i].z = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE+SNOW_SCOPE/2);

			// 设置新的移动速度。
			m_vecSpeed[t_i]	 = mSnowSetting.mVelocity + ((float)OSRAND/RAND_MAX-0.5f)*2*mSnowSetting.mVelocityVariation;
			if(m_vecSpeed[t_i] == 0)
				m_vecSpeed[t_i] = mSnowSetting.mVelocity;


			// 
			// 处理渲染用到的数据。
			m_vecSnowRenderVer[t_i*4+0].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[0].m_vec3Pos;
			m_vecSnowRenderVer[t_i*4+1].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[1].m_vec3Pos;
			m_vecSnowRenderVer[t_i*4+2].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[2].m_vec3Pos;
			m_vecSnowRenderVer[t_i*4+3].m_vec3Pos = 
				m_vecSnowPos[t_i] + m_vecSnowVerOffset[3].m_vec3Pos;


			m_bSnowing = true;
		}
	}

	unguard;

}