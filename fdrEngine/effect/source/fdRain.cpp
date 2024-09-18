//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdRain.cpp
 *
 *  Desc:     ������������Ч�Ĵ���
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
//! RAIN SCOPER����ʮ��,�������ı��ľͿ����ˣ���Ϊ��������ε�����ԣ�
//! �������ֵ��
# define RAIN_SCOPEF    160

//! ATTENTION TO FIX: �������fov�仯��ʱ�򣬴�ֵ����������
//! ������ε�����뾶�Ŵ���,
# define RAINSCOPE_MULTINUM  2.0f


//===============================
//! ѩ��ز���
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
	// ʵ������Ч���õ�������С
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



//! ������Ч����FrameMove����,����ÿһ����ε�λ�ã�Ϊÿһ����μ����µ���Ⱦ����Ρ�
void osc_rain::frame_moveRain( osVec3D& _campos )
{
	guard;

	float     t_fEtime;   
	osVec3D   t_vec3CamPos;

	t_fEtime = sg_timer::Instance()->get_lastelatime();

	// ֻҪ�����Ⱦ���ǲ����µ���Σ��ͻ�����������Ϊ�档
	m_bRainning = false;
	for( int t_i=0;t_i<m_sRainStart.m_iRainDotNum;t_i ++ )
	{
		if( m_vecRainPos[t_i].y > 0.0f )
		{
			m_vecRainPos[t_i].y -= (m_vecSpeed[t_i]*t_fEtime);

			// 
			// ������Ⱦ�õ������ݡ�
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
			// ATTENTION TO FIX:��ǰ��������ܽ���ʱ����ε�����µ�Ч��
			//                  �ῴ�����ܴ󣬴���
			// ������ε��䵽���º��Ч����
			m_vecRainPos[t_i] *= RAINSCOPE_MULTINUM;
			m_vecRainPos[t_i] += _campos;
			osc_bbManager::Instance()->
				create_rainEff( m_vecRainPos[t_i] );

			//
			// ���ֹͣ�����꣬���ٲ����µ���Ρ�
			if( m_bStopRaining )
				continue;

			// ��ʼ���µ����Ч����
			m_vecRainPos[t_i].y = 6.0f+OSRAND%7;
			m_vecRainPos[t_i].x = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);
			m_vecRainPos[t_i].z = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);

			// �����µ��ƶ��ٶȡ�
			m_vecSpeed[t_i]			   = (float)(OSRAND%3)*m_sRainStart.m_fRainMoveSpeed;
			if(m_vecSpeed[t_i] == 0)
				m_vecSpeed[t_i] = m_sRainStart.m_fRainMoveSpeed;

			// 
			// ������Ⱦ�õ������ݡ�
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
*  ��Ⱦ��ǰ����Ч����
*
*  \param _pitch  ��ǰ�������������pitchֵ��
*  \param _pos    ��ǰ�����������λ�á�
*/
void osc_rain::render_rain( osVec3D& _pos,float _pitch )
{
	guard;

	int    t_iStartVer;

	// ������״̬�����������û����Σ����ء�
	if( (!m_bRainning) )
		return ;

	// �ȶԵ�ǰ����ν���Frame move,��䶥�㻺������
	frame_moveRain( _pos );
	t_iStartVer = m_ptrMPipe->fill_dnamicVB( m_iVBid,
		m_vecRainVer,sizeof( os_sceVerDiffuse ),m_sRainStart.m_iRainDotNum*4 );
	osassert( t_iStartVer >= 0 );


	//
	// ������Ⱦ���Ч��ʱʹ�õ����,����Ⱦ���Ч����
	set_rainRenderCam( _pitch );
	m_sPolygon.m_sBufData.m_iVertexStart = t_iStartVer;
	bool t_b = m_ptrMPipe->render_polygon( &m_sPolygon );
	osassert( t_b );

	unguard;
}


//! ������Ⱦ���Ч��ʹ�õ������
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
	// �õ��������Ӿ���
	osMatrixLookAtLH( &t_camMat,&t_vec3Eye,&t_vec3At,&t_vec3Up );
	m_ptrMPipe->set_viewMatrix( t_camMat );
	// 
	// River @ 2006-5-29:�������Ϊidentity
	m_ptrMPipe->set_worldMatrix( osc_bbManager::m_smatMatrix );

	unguard;
}




//! ��ʼ������Ч�����豸������ݡ�
void osc_rain::init_rainDevdepData( void )
{
	guard;


	// 
	// �����豸��ص�����,��̬���㻺���������ݺ�effect�����ݡ�
	m_iVBid = m_ptrMPipe->create_dynamicVB( 
		FVF_LINEVERTEX,sizeof( os_sceVerDiffuse )*MAX_RAINNUM*4 );
	osassert( m_iVBid >= 0 );

	m_iEffectId = g_shaderMgr->add_shader( "rain", osc_engine::get_shaderFile( "line" ).c_str() );
	if( m_iEffectId < 0 )
		m_iEffectId = g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );
	osassert( m_iEffectId>=0 );

	// ����Index buffer���ݡ�
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



//! ��ʼ�����Ч�������ݡ�
void osc_rain::init_rainData( osc_middlePipe* _mpipe )
{
	guard;

	osassert( _mpipe );

	m_ptrMPipe = _mpipe;


	//
	// �����������Ϊ�գ�������µĿռ䡣
	m_vecRainVer = new os_sceVerDiffuse[MAX_RAINNUM*4];
	m_vecRainPos = new osVec3D[MAX_RAINNUM];
	m_vecSpeed  =  new float[MAX_RAINNUM];


	//
	// ��ʼ�������������Ⱦpolygon.
	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_sceVerDiffuse );
	m_sPolygon.m_iVerNum = MAX_RAINNUM*4;
	m_sPolygon.m_iPriNum = MAX_RAINNUM*2;


	// ��ʼ������Ч�����豸������ݡ�
	init_rainDevdepData();


	// ��ǰ����״̬
	m_bRainning = false;

	unguard;
}



//! ��ʼ����Ч����
void osc_rain::start_rain( os_rainStruct* _rs/* = NULL*/ )
{
	guard;

	if( _rs )
		memcpy( &m_sRainStart,_rs,sizeof( os_rainStruct ) );
	else
	{
		os_rainStruct   t_rs;

		// ʹ��Ĭ�ϵ����ṹ��
		memcpy( &m_sRainStart,&t_rs,sizeof( os_rainStruct ) );
	}

	osassert( m_sRainStart.m_iRainDotNum >= 0 );
	osassert( m_sRainStart.m_iRainDotNum <= 500 ); 

	// �����ʼ��������С����ɫ
	for( int t_i=0;t_i<4;t_i ++ )
	{
		m_vecRainVerStart[t_i].m_vecPos.x = m_vecRainVerOffset[t_i].m_vecPos.x*m_sRainStart.m_fXScale;
		m_vecRainVerStart[t_i].m_vecPos.y = m_vecRainVerOffset[t_i].m_vecPos.y*m_sRainStart.m_fYScale;
		m_vecRainVerStart[t_i].m_vecPos.z = m_vecRainVerOffset[t_i].m_vecPos.z;
	}
	fg_dwColorB = m_sRainStart.m_dwBottomColor;
	fg_dwColorA = m_sRainStart.m_dwTopColor;



	//
	// ��ʼ��ÿһ����ε�λ�ú���ɫ��
	// ���ʹ��һ����Χ�ڵ���Σ���Ⱦ���Ч����ʱ��ʹ���ر������Ƕȡ�
	// �������ĸ߶Ⱥ�λ���ǹ̶��ģ������������pitchֵ��������������ͬ
	// ��pitchֵ���� 
	for( int t_i=0;t_i<m_sRainStart.m_iRainDotNum;t_i ++ )
	{
		m_vecRainVer[t_i*4+0].m_color = fg_dwColorB;
		m_vecRainVer[t_i*4+1].m_color = fg_dwColorB;
		m_vecRainVer[t_i*4+2].m_color = fg_dwColorA;
		m_vecRainVer[t_i*4+3].m_color = fg_dwColorA;

		// ��ʼ��ÿһ�����Ч����λ�á�
		m_vecRainPos[t_i].x = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);
		m_vecRainPos[t_i].z = (float)(RAIN_SCOPE - (float)(OSRAND%RAIN_SCOPEF)/20.0f);
		m_vecRainPos[t_i].y = (float)( OSRAND%13 );


		m_vecSpeed[t_i]			   = (float)(OSRAND%3)*m_sRainStart.m_fRainMoveSpeed;
		if(m_vecSpeed[t_i] == 0)
			m_vecSpeed[t_i] = m_sRainStart.m_fRainMoveSpeed;

	}

	// ��ǰ���������Ŀ��Ҫ��Ⱦ�Ķ������Ŀ
	m_sPolygon.m_iVerNum = m_sRainStart.m_iRainDotNum*4;
	m_sPolygon.m_iPriNum = m_sRainStart.m_iRainDotNum*2;

	// ��������״̬Ϊ����
	m_bRainning = true;
	m_bStopRaining = false;

	unguard;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// ��������ѩ��صĴ���
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


//! ��ʼ��������ѩ�Ĺ�����
void osc_snowStorm::init_snowStormMgr( osc_middlePipe* _mpipe )
{
	guard;
	//
	// �����������Ϊ�գ�������µĿռ䡣
	m_vecSnowRenderVer = new os_SnowVertex[MAX_SNOWNUM*4];
	m_vecSnowPos = new osVec3D[MAX_SNOWNUM];
	m_vecSpeed  =  new osVec3D[MAX_SNOWNUM];


	//
	// ��ʼ�������������Ⱦpolygon.
	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_SnowVertex );
	m_sPolygon.m_iVerNum = MAX_SNOWNUM*4;
	m_sPolygon.m_iPriNum = MAX_SNOWNUM*2;

	osassert(_mpipe);
	m_ptrMPipe = _mpipe;
	// 
	// �����豸��ص�����,��̬���㻺���������ݺ�effect�����ݡ�
	m_iVBid = m_ptrMPipe->create_dynamicVB( 
		FVF_SNOWVERTEX,sizeof( os_SnowVertex )*MAX_SNOWNUM*4 );
	osassert( m_iVBid >= 0 );

	m_iEffectId = g_shaderMgr->add_shader( "snow", osc_engine::get_shaderFile( "snow" ).c_str() );
	if( m_iEffectId < 0 )
		m_iEffectId = g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );
	osassert( m_iEffectId>=0 );

	// ����Index buffer���ݡ�
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

//! ��ʼ��ѩ
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

		

		// ��ʼ��ÿһ�����Ч����λ�á�
		m_vecSnowPos[t_i].x = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE);
		m_vecSnowPos[t_i].z = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE+SNOW_SCOPE/2);
		m_vecSnowPos[t_i].y = mSnowSetting.mSnowHeight;


		m_vecSpeed[t_i]	 = mSnowSetting.mVelocity + ((float)OSRAND/RAND_MAX-0.5f)*2*mSnowSetting.mVelocityVariation;
		if(m_vecSpeed[t_i] == 0)
			m_vecSpeed[t_i] = mSnowSetting.mVelocity;

	}



	// ������ѩ״̬Ϊ����
	m_bSnowing = true;
	m_bStopSnowing = false;


	return;

	unguard;
}

//! ������ѩ
void osc_snowStorm::stop_snowStorm( void )
{
	guard;
	m_bStopSnowing = true;
	return;

	unguard;
}

/** \brief
*  ��Ⱦ��ǰ��ѩЧ����
*
*  \param _pitch  ��ǰ�������������pitchֵ��
*  \param _pos    ��ǰ�����������λ�á�
*/
void osc_snowStorm::render_snowStorm( osVec3D& _pos,float _pitch )
{
	guard;

	int    t_iStartVer;

	// ������״̬�����������û����Σ����ء�
	if( (!m_bSnowing) )
		return ;

	// �ȶԵ�ǰ����ν���Frame move,��䶥�㻺������
	frame_moveSnow( _pos );
	t_iStartVer = m_ptrMPipe->fill_dnamicVB( m_iVBid,
		m_vecSnowRenderVer,sizeof( os_SnowVertex ),mSnowSetting.mSnowNr*4 );
	osassert( t_iStartVer >= 0 );


	//
	// ������Ⱦ���Ч��ʱʹ�õ����,����Ⱦ���Ч����
	set_snowRenderCam( _pitch );
	m_sPolygon.m_sBufData.m_iVertexStart = t_iStartVer;
	m_sPolygon.m_iVerNum = mSnowSetting.mSnowNr*4;
	m_sPolygon.m_iPriNum = mSnowSetting.mSnowNr*2;
	bool t_b = m_ptrMPipe->render_polygon( &m_sPolygon );
	osassert( t_b );

	unguard;
}


//! �õ��Ƿ���ѩ
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

//! ������ȾѩЧ��ʹ�õ������
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
	// �õ��������Ӿ���
	osMatrixLookAtLH( &t_camMat,&t_vec3Eye,&t_vec3At,&t_vec3Up );
	m_ptrMPipe->set_viewMatrix( t_camMat );
	// 
	// River @ 2006-5-29:�������Ϊidentity
	m_ptrMPipe->set_worldMatrix( osc_bbManager::m_smatMatrix );

	unguard;
}

void osc_snowStorm::frame_moveSnow( osVec3D& _campos )
{
	guard;

	float     t_fEtime;   
	osVec3D   t_vec3CamPos;

	t_fEtime = sg_timer::Instance()->get_lastelatime();

	// ֻҪ�����Ⱦ���ǲ����µ���Σ��ͻ�����������Ϊ�档
	m_bSnowing = false;
	//osColor snowColor = 0xff00ffff;
	for( int t_i=0;t_i<mSnowSetting.mSnowNr;t_i ++ )
	{
		if( m_vecSnowPos[t_i].y > mSnowSetting.mSnowLower &&m_vecSnowPos[t_i].y <= mSnowSetting.mSnowHeight)
		{
			m_vecSnowPos[t_i] += (m_vecSpeed[t_i]*t_fEtime);

			// 
			// ������Ⱦ�õ������ݡ�
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
			// ���ֹͣ�����꣬���ٲ����µ���Ρ�
			if( m_bStopSnowing )
				continue;

			// ��ʼ���µ����Ч����
			m_vecSnowPos[t_i].y = 6;//6.0f+OSRAND%7;
			m_vecSnowPos[t_i].x = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE);
			m_vecSnowPos[t_i].z = (float)(((float)OSRAND/RAND_MAX-0.5f)*2*SNOW_SCOPE+SNOW_SCOPE/2);

			// �����µ��ƶ��ٶȡ�
			m_vecSpeed[t_i]	 = mSnowSetting.mVelocity + ((float)OSRAND/RAND_MAX-0.5f)*2*mSnowSetting.mVelocityVariation;
			if(m_vecSpeed[t_i] == 0)
				m_vecSpeed[t_i] = mSnowSetting.mVelocity;


			// 
			// ������Ⱦ�õ������ݡ�
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