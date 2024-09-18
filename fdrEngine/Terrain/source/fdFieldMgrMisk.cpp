//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrMisk.cpp
 *
 *  His:      River created @ 2004-10-28
 *
 *  Des:      地图格子相关的类，实现了场景中的杂项功能
 *   
 *
 *  "宰宁国以礼，治乱邦以法"
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../mfpipe/include/font.h"
# include "../../effect/include/fdEleBolt.h"
# include "../../backpipe/include/mainEngine.h"



//! 整个地图的纹理信息.
# define  MAPINFO_FILE   "\\texture.lst"
# define  MAPINFO_ID     "MP"

//! 地表纹理用到的声音相关信息
# define  TERRAIN_TEXSND_EXT   "mat"
# define  TERRAIN_TEXSND_VER   0x00


//! 地图在x,z方向上的最大矩离值.是地表格子数*3
float    g_fMapMaxX = 196.0f;
float    g_fMapMaxZ = 196.0f;





//! 把当前场景的天气信息收集到当前的天气数据结构中.
void osc_TGManager::gather_curWeather( void )
{
	guard;

	// 是否下雨。
	m_sCurrentWeather.m_bRain = m_effectRain.get_rainning();

	// 收集雾相关信息.
	m_sCurrentWeather.m_fFogEnd = g_fFogEnd;
	m_sCurrentWeather.m_fFogStart = g_fFogStart;
	m_sCurrentWeather.m_sFogColor = g_dwFogColor;

	// 收集环境光相关的信息。
	m_sCurrentWeather.m_sAmbientColor.a = 1.0f;
	m_sCurrentWeather.m_sAmbientColor.r = g_fAmbiR;
	m_sCurrentWeather.m_sAmbientColor.g = g_fAmbiG;
	m_sCurrentWeather.m_sAmbientColor.b = g_fAmbiB;

	// 方向光相关。
	m_sCurrentWeather.m_sDirLightColor.a = 1.0f;
	m_sCurrentWeather.m_sDirLightColor.r = g_fDirectLR;
	m_sCurrentWeather.m_sDirLightColor.g = g_fDirectLG;
	m_sCurrentWeather.m_sDirLightColor.b = g_fDirectLB;
	m_sCurrentWeather.m_vec3DirLight = g_vec3LPos;

	// 收集动态光
	m_sCurrentWeather.m_charDLightColor.a = 1.0f;
	m_sCurrentWeather.m_charDLightColor.r = g_fDlR;
	m_sCurrentWeather.m_charDLightColor.g = g_fDlG;
	m_sCurrentWeather.m_charDLightColor.b = g_fDlB;

	// 收集远景显示的颜色
	m_sCurrentWeather.m_sFarTerrColor = g_dwFarTerrColor;

	m_sCurrentWeather.m_fSkyMoveSpeed = g_fBkSkyRotSpeed;//m_skyMgr.get_skyMoveSpeed();

	// windy add 7.6
	m_sCurrentWeather.m_TerrFogDistance = (float)g_dwTerrFogDistance;
	m_sCurrentWeather.m_SkyFogDistance = (float)g_dwSkyFogDistance;
	m_sCurrentWeather.m_fSky1MoveSpeed = g_fBkSky1RotSpeed;
	m_sCurrentWeather.m_fSky2MoveSpeed = g_fBkSky2RotSpeed;

	return;

	unguard;
}


//! 只有当三个环境光分量的的任一个变化大于等于这个值的时候，才会修改全局的环境光
# define AMBIENT_CHANGEOFFSET    0.01

/** \brief
*  用于判断当前的天气变化是否大到可以重设当前的重局变量.
*
*  因为重设环境光用到的计算量比较大，如果环境光的变化太小，
*  则等到变化到一定的程度后再变化
*
*  \param _etime 当前帧过去的时间。
*/
bool osc_TGManager::change_sceneAmbient( float _etime )
{
	guard;

	static float   t_fOffset = 0.0f;
	static float   t_fRBack,t_fGBack,t_fBBack;
	float          t_fR,t_fG,t_fB,t_fS;

	t_fS = m_fTimeFromChangeW / m_fWeatherChangeTime;

	// 环境光。
	t_fR = osn_mathFunc::float_lerp( m_sCurrentWeather.m_sAmbientColor.r,
		m_sNewWeather.m_sAmbientColor.r,t_fS );
	t_fG = osn_mathFunc::float_lerp( m_sCurrentWeather.m_sAmbientColor.g,
		m_sNewWeather.m_sAmbientColor.g,t_fS );
	t_fB = osn_mathFunc::float_lerp( m_sCurrentWeather.m_sAmbientColor.b,
		m_sNewWeather.m_sAmbientColor.b,t_fS );

	// 
	if( t_fR < FLOATBOTTOM )
		t_fR = FLOATBOTTOM;
	if( t_fG < FLOATBOTTOM )
		t_fG = FLOATBOTTOM;
	if( t_fB < FLOATBOTTOM )
		t_fB = FLOATBOTTOM;

	if( float_equal( _etime,m_fTimeFromChangeW ) )
	{
		t_fRBack = g_fAmbiR;
		t_fGBack = g_fAmbiG;
		t_fBBack = g_fAmbiB;
	}

	// 如果任一个分量大于指定的值，则重设全局光
	if( (fabs(t_fR-t_fRBack) >= AMBIENT_CHANGEOFFSET)||
		(fabs(t_fG-t_fGBack) >= AMBIENT_CHANGEOFFSET)||
		(fabs(t_fB-t_fBBack) >= AMBIENT_CHANGEOFFSET) )
	{
		change_amibentCoeff( t_fR,t_fG,t_fB );

		t_fRBack = g_fAmbiR;
		t_fGBack = g_fAmbiG;
		t_fBBack = g_fAmbiB;		
	}

	return true;

	unguard;
}


//! 天气间的熔合。
void osc_TGManager::frame_moveWeatherChanging( float _etime )
{
	guard;

	osassert( m_bWeatherChanging );

	m_fTimeFromChangeW += _etime;
	
	// 
	// 可以使用新的天气数据了。
	if( m_fTimeFromChangeW > m_fWeatherChangeTime )
	{
		m_bWeatherChanging = false;

		// 先设置全局的环境光.
		change_amibentCoeff( m_sNewWeather.m_sAmbientColor.r,
			m_sNewWeather.m_sAmbientColor.g,m_sNewWeather.m_sAmbientColor.b );

		// 再设置方向光.

		g_fDirectLR = m_sNewWeather.m_sDirLightColor.r;
		g_fDirectLG = m_sNewWeather.m_sDirLightColor.g;
		g_fDirectLB = m_sNewWeather.m_sDirLightColor.b;
		g_vec3LPos = m_sNewWeather.m_vec3DirLight;
		osVec3Normalize( &g_vec3LPos,&g_vec3LPos );


		// 设置雾相关的参数.
		g_dwFogColor = m_sNewWeather.m_sFogColor;

		g_fFogEnd = m_sNewWeather.m_fFogEnd;
		g_fFogStart = m_sNewWeather.m_fFogStart;

		// 设置天空运行速度。
		m_skyMgr.reset_skyMoveSpeed( m_sNewWeather.m_fSkyMoveSpeed );
		g_fBkSkyRotSpeed = m_sNewWeather.m_fSkyMoveSpeed;
		g_fBkSky1RotSpeed = m_sNewWeather.m_fSky1MoveSpeed;
		g_fBkSky2RotSpeed = m_sNewWeather.m_fSky2MoveSpeed;



		// 全局的用于远景的颜色。
		g_dwFarTerrColor = m_sNewWeather.m_sFarTerrColor;


		// 开空和远景的雾效
		g_dwTerrFogDistance = (int)m_sNewWeather.m_TerrFogDistance;
		g_dwSkyFogDistance = (int)m_sNewWeather.m_SkyFogDistance;

		return;
	}

	//
	// 在新旧天气之间进行插值。
	float    t_fS;


	t_fS = m_fTimeFromChangeW / m_fWeatherChangeTime;

	change_sceneAmbient( _etime );


	// 方向光。
	g_fDirectLR = osn_mathFunc::float_lerp( m_sCurrentWeather.m_sDirLightColor.r,
		m_sNewWeather.m_sDirLightColor.r,t_fS );
	g_fDirectLG = osn_mathFunc::float_lerp( m_sCurrentWeather.m_sDirLightColor.g,
		m_sNewWeather.m_sDirLightColor.g,t_fS );
	g_fDirectLB = osn_mathFunc::float_lerp( m_sCurrentWeather.m_sDirLightColor.b,
		m_sNewWeather.m_sDirLightColor.b,t_fS );
	osVec3Lerp( &g_vec3LPos,&m_sCurrentWeather.m_vec3DirLight,
		&m_sNewWeather.m_vec3DirLight,t_fS );
	osVec3Normalize( &g_vec3LPos,&g_vec3LPos );

	//
	// 雾相关。
	osColor   t_sColor;
	osColorLerp( &t_sColor,&m_sCurrentWeather.m_sFogColor,
		&this->m_sNewWeather.m_sFogColor,t_fS );
	g_dwFogColor = t_sColor;
	g_fFogEnd = osn_mathFunc::float_lerp( m_sCurrentWeather.m_fFogEnd,
		this->m_sNewWeather.m_fFogEnd,t_fS );
	g_fFogStart = osn_mathFunc::float_lerp( m_sCurrentWeather.m_fFogStart,
		this->m_sNewWeather.m_fFogStart,t_fS );



	// 
	// 天空纹理的移动速度相关。
	float   t_fMovSpd;
	t_fMovSpd = osn_mathFunc::float_lerp( m_sCurrentWeather.m_fSkyMoveSpeed,
		m_sNewWeather.m_fSkyMoveSpeed,t_fS );
	m_skyMgr.reset_skyMoveSpeed( t_fMovSpd );


	osColor   t_lightColor;

	//
	// 远景显示使用的颜色。
	osColorLerp( &t_lightColor,&m_sCurrentWeather.m_sFarTerrColor,
		&m_sNewWeather.m_sFarTerrColor,t_fS );
	g_dwFarTerrColor = t_lightColor;


	// 开空和远景的雾效
	g_dwTerrFogDistance = (int)osn_mathFunc::float_lerp(
		 m_sCurrentWeather.m_TerrFogDistance,
		 m_sNewWeather.m_TerrFogDistance,t_fS );
	g_dwSkyFogDistance = (int)osn_mathFunc::float_lerp(
		 m_sCurrentWeather.m_SkyFogDistance,
		 m_sNewWeather.m_SkyFogDistance,t_fS );

	return;

	unguard;
}


/** \brief
*  改变场景的天气。
*
*  \param _weather  新的场景天气数据结构。
*  \param _itime    改变天气需要的天气熔合时间,以秒为单位.如果此值
*                   小于等于零，则直接修改天气。
*  
*/
void osc_TGManager::change_sceneWeather( os_weather& _weather,float _itime )
{
	guard;

	// River added @ 2007-5-30:如果场景闪电处于打开状态，关闭
	if( m_SceneFlashLightMgr->is_sceneLightOn() )
		m_SceneFlashLightMgr->stop();


	//
	// 如果不需要天气间的熔合的话.直接改变天气.
	if( _itime <= 0.0f )
	{
		// 先设置全局的环境光.
		change_amibentCoeff( _weather.m_sAmbientColor.r,
			_weather.m_sAmbientColor.g,_weather.m_sAmbientColor.b );

		// 再设置方向光.
		g_fDirectLR = _weather.m_sDirLightColor.r;
		g_fDirectLG = _weather.m_sDirLightColor.g;
		g_fDirectLB = _weather.m_sDirLightColor.b;
		g_vec3LPos = _weather.m_vec3DirLight;
		osVec3Normalize( &g_vec3LPos,&g_vec3LPos );


		// 设置雾相关的参数.
		g_dwFogColor = _weather.m_sFogColor;
		g_fFogEnd = _weather.m_fFogEnd;
		g_fFogStart = _weather.m_fFogStart;

		// 处理天空的移动速度。
		m_skyMgr.reset_skyMoveSpeed( _weather.m_fSkyMoveSpeed );


		g_fBkSkyRotSpeed = _weather.m_fSkyMoveSpeed;
		g_fBkSky1RotSpeed = _weather.m_fSky1MoveSpeed;
		g_fBkSky2RotSpeed = _weather.m_fSky2MoveSpeed;

		//g_fBkSky1RotSpeed = _weather.m_fSky1MoveSpeed ;
		//g_fBkSky2RotSpeed = _weather.m_fSky2MoveSpeed ;

		g_dwTerrFogDistance = (int)_weather.m_TerrFogDistance;
		g_dwSkyFogDistance = (int)_weather.m_SkyFogDistance;

		g_dwFarTerrColor = _weather.m_sFarTerrColor;

		m_bWeatherChanging = false;

	}
	else
	{
		gather_curWeather();
		
		m_bWeatherChanging = true;
		memcpy( &m_sNewWeather,&_weather,sizeof( os_weather ) );
		m_fWeatherChangeTime = _itime;
		m_fTimeFromChangeW = 0.0f;
	}

	// 下雨与否?
	if( m_effectRain.get_rainning() && (!_weather.m_bRain) )
		m_effectRain.stop_rain();
	if( _weather.m_bRain )
		m_effectRain.start_rain( &_weather.m_sRainStruct );

	// windy 7.11 add 加入开始播放雪花
	m_effectSnow.stop_snowStorm();

	if ( _weather.m_bSnow )
	{
		m_effectSnow.start_snowStorm(_weather.m_SnowSetting);

	}

	// 
	// 天空中的云没有熔合，直接替换。
	// 背景天空使用g_fSkyTerrAnitime这个变量内的时间进行熔合.
	//@{
	// Windy mod @ 2005-9-11 11:11:56
	//Desc: sky _weather.m_szOsaSky3 为第三层天空OSA文件
	m_skyMgr.reset_skyTexture( _weather.m_szOsaBkSky ,_weather.m_szOsaBkSkyEffect[0],_weather.m_szOsaBkSkyEffect[1],_weather.m_szOsaBolt,_weather.m_iBoltRate,_weather.m_BoltAngle,_weather.m_BoltColor,_weather.m_BoltHeight);
	m_skyMgr.reset_osaFarTerr(_weather.m_szTerrain);
	strcpy(g_CurFarTerrain,_weather.m_szTerrain);

	//stop_sceneLight();
	if (strlen(_weather.m_szOsaBolt)!=0)
	{
		g_bHasBolt = true;
		start_sceneLight( -1.0f,2,_weather.m_BoltType,_weather.m_iBoltRate,_weather.m_BoltColor);
	}
	else
	{
		g_bHasBolt = false;
		stop_sceneLight();
		
	}
	//@}


	// 处理人物的动态光的颜色
	g_fDlR  = _weather.m_charDLightColor.r;
	g_fDlG  = _weather.m_charDLightColor.g;
	g_fDlB  = _weather.m_charDLightColor.b;
	//!加载lens flare
	osassert(m_pLensflare);
	m_pLensflare->loadStdFlare(_weather.m_szLensPath);
	//m_pLensflare->setPosition(_weather.m_vec3LensPos);
	m_pLensflare->m_IsRender = TRUE;
	g_fBoltFanAngle = _weather.m_fBoltFanAngle;
	

	return ;

	unguard;
}

/** \brief
*  改变场景的远景显示osa模型.
*  
*  \param _terr 远山的模型文件名.
*/
void osc_TGManager::change_sceneFarTerr( const char* _terr )
{
	guard;

	m_skyMgr.reset_osaFarTerr( _terr );

	unguard;
}



# if __EFFECT_EDITOR__
//! 得到当前场景的天气。
void osc_TGManager::get_sceneWeather( os_weather& _weather )
{
	guard;

	gather_curWeather();

	memcpy( &_weather,&this->m_sCurrentWeather,sizeof( os_weather ) );

	_weather.m_fSkyMoveSpeed = this->m_skyMgr.get_skyMoveSpeed();
	strcpy( _weather.m_szSkyTex,m_skyMgr.get_skyTexture() );

	if( NULL == m_skyMgr.get_bkSkyOsa() )
		_weather.m_szOsaBkSky[0] = NULL;
	else
		strcpy( _weather.m_szOsaBkSky,m_skyMgr.get_bkSkyOsa() );


	/*_weather.m_szOsaBkSkyEffect[0][0] = NULL ;
	_weather.m_szOsaBkSkyEffect[1][0] = NULL ;
	_weather.m_szOsaBolt[0] = NULL ;
	_weather.m_iBoltRate = 0.0f;*/

	if (NULL == m_skyMgr.get_bkSkyOsa1())
	{
		_weather.m_szOsaBkSkyEffect[0][0] = NULL ;
	}
	else
	{
		strcpy( _weather.m_szOsaBkSkyEffect[0],m_skyMgr.get_bkSkyOsa1() );
	}
	//
	if (NULL == m_skyMgr.get_bkSkyOsa2())
	{
		_weather.m_szOsaBkSkyEffect[1][0] = NULL ;
	}
	else
	{
		strcpy( _weather.m_szOsaBkSkyEffect[1],m_skyMgr.get_bkSkyOsa2() );
	}
	//
	if (NULL == m_skyMgr.get_bolt())
	{
		_weather.m_szOsaBolt[0] = NULL ;
	}
	else
	{
		strcpy( _weather.m_szOsaBolt,m_skyMgr.get_bolt() );
	}
	//
	_weather.m_iBoltRate = m_skyMgr.get_boltRate();
	


	_weather.m_bSnow = m_effectSnow.get_weatherSnowing(&_weather.m_SnowSetting);

	//!设置默认的LENS flare
	_weather.m_bIsLensFlare = false;
	_weather.m_vec3LensPos = osVec3D(0.0f,0.0f,0.0f);
	_weather.m_szLensPath[0] = NULL; 

	_weather.m_fBoltFanAngle = g_fBoltFanAngle;
	strcpy(_weather.m_szTerrain,g_CurFarTerrain);
	unguard;
}
# endif 




//! 对下雨效果的处理。
void osc_TGManager::render_rain( void )
{
	guard;

	osVec3D      t_vec3Pos;
	
	g_ptrCamera->get_curpos( &t_vec3Pos );

	t_vec3Pos = t_vec3Pos + m_vec3HeroPos;
	t_vec3Pos /= 2.0f;

	m_effectRain.render_rain( t_vec3Pos,g_ptrCamera->get_curPitch() );
	//! windy add 7.4 
//	m_effectSnow.render_snowStorm(t_vec3Pos,g_ptrCamera->get_curPitch());

	unguard;
}
void osc_TGManager::render_snow(void)
{
	guard;

	osVec3D      t_vec3Pos;

	g_ptrCamera->get_curpos( &t_vec3Pos );

	t_vec3Pos = t_vec3Pos + m_vec3HeroPos;
	t_vec3Pos /= 2.0f;

	//! windy add 7.4 
	m_effectSnow.render_snowStorm(t_vec3Pos,g_ptrCamera->get_curPitch());

	unguard;
}






//! 渲染遮挡物品列表。
void osc_TGManager::render_shelterObj( bool _alpha/* = true*/ )
{
	guard;

	if( !_alpha )
	{
		m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_CW );
		m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,true );

		m_middlePipe.set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
		m_middlePipe.set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		m_middlePipe.set_fvf( FVF_OBJECTVERTEX );

		// River added @ 2006-9-11:对于某些树在半透明时的渲染错误
		m_middlePipe.set_ssRepeat();
	}

	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );
		if( !m_arrPtrTG[t_i]->render_halfAlphaObj( &m_middlePipe,_alpha ) )
			osassertex( false,"Render Shelter failed....\n" );

	}

	// 渲染完alpha部分的shelterobj后再回复渲染状态
	if( _alpha )
	{
		set_material( 1.0f );
		m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,false );
		m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_CCW );
		m_middlePipe.set_renderState( D3DRS_FOGENABLE,FALSE );
	}

	unguard;
}

//! 渲染场景中物品的不透明部分。
bool osc_TGManager::render_opacityObjPart( bool _bridgeDraw/* = false*/ )
{
	guard;

	m_dlightMgr.dxlight_enable( true,true );
	m_middlePipe.set_fvf( FVF_OBJECTVERTEX );
	m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_CW );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,TRUE );


	//  
	//! 如果仅仅渲染bridge
	//  仅渲染bridge用于场景内的倒影渲染
	if( _bridgeDraw )
	{
		// River: 为什么原来设置ZwriteEnable为false?这样会有桥类物品之间的问题
		// m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,FALSE );
		m_middlePipe.set_renderState( D3DRS_FOGENABLE,TRUE );
		m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_NONE );

		for( int t_i=0;t_i<m_iActMapNum;t_i++ )
		{
			osassert( this->m_arrPtrTG[t_i] );

			if( !m_arrPtrTG[t_i]->render_bridgeObject( &m_middlePipe ) )
				return false;

		}
		flush_opacityObj();

		
		// River mod @ 2009-2-25:
		// 测试渲染桥类物品的透明面，用于水面边上的花类物品渲染
		for( int t_i=0;t_i<m_iActMapNum;t_i++ )
		{
			osassert( this->m_arrPtrTG[t_i] );
			if( !m_arrPtrTG[t_i]->render_bridgeObjAlphaPart( &m_middlePipe ) )
				return false;
		}

		// RIVER OPT: 使用一个渲染状态块
		m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,TRUE );
		m_middlePipe.set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
		m_middlePipe.set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
		m_middlePipe.set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		m_middlePipe.set_renderState( D3DRS_ALPHAREF, 0xaf );
		m_middlePipe.set_renderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

		flush_alphaObj( false );

		// After render
		m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,FALSE );
		m_middlePipe.set_renderState( D3DRS_FOGENABLE,false );
		m_middlePipe.set_renderState( D3DRS_ALPHAREF,GLOBAL_MINALPHAREF );
		m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_CW );

		return true;
	}
	
	//
	// 渲染场景中的不透明物体。
	osassert( m_iOpacityObjNum == 0 );
	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );

		if( !m_arrPtrTG[t_i]->render_TGsObject( &m_middlePipe,false ) )
			return false;

	}
	flush_opacityObj();

	// 
	// 如果要透明的渲染遮挡住人物的物品，则需要先渲染人物。
	// 渲染淡入淡出的物品队列中的不透明部分。
	m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,true );
	m_middlePipe.set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_middlePipe.set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	m_middlePipe.set_ssRepeat();

	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );

		if( !m_arrPtrTG[t_i]->render_TGsFadeInObject( &m_middlePipe,false ) )
			return false;
		if( !m_arrPtrTG[t_i]->render_TGsFadeOutObject( &m_middlePipe,false ) )
			return false;
	}
	set_material( 1.0f );
	m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,false );

	return true;

	unguard;
}


//! 渲染场景中物品的透明部分。
bool osc_TGManager::render_transparentObjPart( void )
{
	guard;

	osassert( m_iAlphaObjNum == 0 );

	// 使用一个渲染状态块。
	m_middlePipe.set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
	m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_CW );

	m_middlePipe.set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_middlePipe.set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	m_middlePipe.set_fvf( FVF_OBJECTVERTEX );
	m_middlePipe.set_renderState( D3DRS_FOGENABLE,TRUE );
	m_middlePipe.set_renderState( D3DRS_ALPHAREF, 0x8f );
	m_middlePipe.set_renderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	
	//
	// 渲染场景中的透明物体,渲染透明物体时，透明物体需要进行简单的排序。
	m_dlightMgr.dxlight_enable( true );

	//
	// 渲染场景中的透明物体。
	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( m_arrPtrTG[t_i] );

		if( !m_arrPtrTG[t_i]->render_TGsObject( &m_middlePipe,true ) )
			return false;

	}
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,true );
	flush_alphaObj( true );

	// River @ 2008-10-8:渲染透明物品内的透明部分
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,false );


	// River @ 2010-3-20:去除场景内物品半透明部的渲染,提高渲染效率
	/*
	m_middlePipe.set_renderState( D3DRS_ALPHAREF, 0xcf );
	m_middlePipe.set_renderState(D3DRS_ALPHAFUNC, D3DCMP_LESS );
	m_dlightMgr.dxlight_enable( true );
	flush_alphaObj( false );
	*/

	// 
	// 渲染淡入淡出的物品队列。
	m_middlePipe.set_renderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
    m_middlePipe.set_renderState( D3DRS_ALPHAREF,GLOBAL_MIDALPHAREF );
	m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,true );
	m_middlePipe.set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_middlePipe.set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );

	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( m_arrPtrTG[t_i] );

		if( !m_arrPtrTG[t_i]->render_TGsFadeInObject( &m_middlePipe,true ) )
			return false;
		if( !m_arrPtrTG[t_i]->render_TGsFadeOutObject( &m_middlePipe,true ) )
			return false;
	}

	m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,false );
	set_material( 1.0f );

	return true;

	unguard;
}


os_wholeSceneData::os_wholeSceneData()
{
	this->m_arrTexnameList = NULL;
	this->m_iTexNum = 0;
	this->m_szBgMusic[0] = NULL;
	this->m_iObjectNum = 0;
	this->m_arrObjNameList = NULL;

	this->m_fWaterHeight = 10000.0f;
}

os_wholeSceneData::~os_wholeSceneData()
{
	this->release();
}

//! 调入整个场景中的物品到内存
bool os_wholeSceneData::init_wholeSceneData( void )
{
	guard;

	int        t_iSize = 0;
	BOOL       t_bReplace;


	int  t_iWholeMeshSize = 0;

	
	for( int t_i=0;t_i<m_iObjectNum;t_i ++ )
	{
		if( this->m_arrObjNameList[t_i].m_dwObjType != 0 )
		{
			if( g_bUseAnimationObj )
				continue;
			else if( m_arrObjNameList[t_i].m_dwObjType != 1 )
				continue;
			else if( m_arrObjNameList[t_i].m_dwObjType){
			}
		}

# ifdef _DEBUG
		
		t_iSize = ::get_fileSize( m_arrObjNameList[t_i].m_szObjName );
		t_iWholeMeshSize += t_iSize;
# endif 

		t_iSize = g_ptrMeshMgr->creat_meshFromFile( 
			m_arrObjNameList[t_i].m_szObjName,t_bReplace );
		osassertex( t_iSize>=0,"找不到要调入ex文件.." );
		osassertex( !t_bReplace,"不能替换底层的mesh" );
		osDebugOut( "创建<%s>文件..\n",m_arrObjNameList[t_i].m_szObjName );
		// syq 5-20 add
		int t_iUpdate = int(float(t_i)/float(m_iObjectNum)*25.f);
		if( osc_TGManager::m_pUpdate_uiLoadingStep )
			(*osc_TGManager::m_pUpdate_uiLoadingStep)( t_iUpdate, 0 );
	}

	t_iWholeMeshSize /= (1024);

	osDebugOut( "整个场景用到的Ex内存大小为:<%d>K...\n",t_iWholeMeshSize );

	m_iWHNum = 0;

	return true;

	unguard;
}


//! push water height.
void os_wholeSceneData::push_waterHeight( float _wh )
{
	guard;

	for( int t_i = 0;t_i<m_iWHNum;t_i ++ )
	{
		if( fabs( _wh - this->m_vecWaterHeight[t_i] ) < 0.003f )
			return;
	}

	if( m_iWHNum == MAX_WATERHEIGHT )
		return;

	int t_idx = 0;
	for( t_idx = 0;t_idx<m_iWHNum;t_idx ++ )
	{
		if( _wh < m_vecWaterHeight[t_idx] )
			break;
	}

	for( int t_i = m_iWHNum-1;t_i>=t_idx;t_i -- )
		m_vecWaterHeight[t_i+1] = m_vecWaterHeight[t_i];

	m_vecWaterHeight[t_idx] = _wh;

	m_iWHNum ++;

	return;


	unguard;
}

//! 得到当前最科学的水面高度.
float os_wholeSceneData::get_waterHeight( void )
{
	guard;

	//! 得到全局相机的焦点位置.
	float t_fFocusHeight = g_ptrCamera->get_camFocus().y;

	// 无水面,返回很小不影响场景的值.
	if( m_iWHNum == 0 )
		return -100.0f;

	int t_i = 0;
	for( t_i =0;t_i<m_iWHNum;t_i ++ )
	{
		if( t_fFocusHeight < m_vecWaterHeight[t_i] )
			break;
	}


	return m_vecWaterHeight[t_i-1];

	unguard;
}


void os_wholeSceneData::release( void )
{

	SAFE_DELETE_ARRAY( m_arrTexnameList );

# ifdef _DEBUG
	if( m_iObjectNum > 0 )
	{
		osassertex( m_arrObjNameList,"物品名字列表不能为空" );
	}
# endif 

	SAFE_DELETE_ARRAY( m_arrObjNameList );

	m_iObjectNum = 0;
	m_iTexNum = 0;
	m_fWaterHeight = 100000;

	m_iWHNum = 0;

}


#define MAKEFOURCC32(ch0, ch1, ch2, ch3)                \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))


//! 场景内lst文件的文件头
struct LstHeader 
{
	char		m_szId[4];
	DWORD		m_dwVersion;
	int			m_iXChunks;
	int			m_iYChunks;
	DWORD		m_dwModels;
	DWORD		m_dwModelOfs;
	DWORD		m_dwTextures;
	DWORD		m_dwTexturesOfs;
};

//! 地表声音和特效相关的信息
bool osc_TGManager::load_terrainTexSnd( const char* _mapName )
{
	guard;

	char       t_szFname[256];
	int        t_iSize;
	BYTE*      t_fstart;

	sprintf( t_szFname,"%s%s%s%s%s%s",
		DEFAULT_MAPTEXDIR,_mapName,"\\",_mapName,".",TERRAIN_TEXSND_EXT );

	if( ::file_exist( t_szFname ) )
	{
		int   t_iGBufIdx = -1;
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_iSize = read_fileToBuf( t_szFname,t_fstart,TMP_BUFSIZE );
		osassert( t_iSize > 0 );
		READ_MEM_OFF( t_szFname,t_fstart,sizeof( char )*4 );
		if( 0 != strcmp( t_szFname,TERRAIN_TEXSND_EXT ) )
		{
			osassertex( false,va( "文件<%s>的格式不对...\n",t_szFname ) );
			return false;
		}
		READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
		osassert( TERRAIN_TEXSND_VER == t_iSize );

		// 读入实际数据
		m_vecTerrainTexSnd.clear();
		READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
		m_vecTerrainTexSnd.resize( t_iSize );
		// 必须大于等于场景预调入的地表纹理数目
		osassert( t_iSize >= m_sSceneTexData.m_iTexNum );
		READ_MEM_OFF( &m_vecTerrainTexSnd[0],t_fstart,
			sizeof( os_terrainTexSnd )*t_iSize );
		END_USEGBUF( t_iGBufIdx );
	}
	else
		m_vecTerrainTexSnd.resize( m_sSceneTexData.m_iTexNum );

	return true;

	unguard;
}


//! 从lst文件中调入场景的相关信息.
bool osc_TGManager::load_sceneInfoFile(  const char* _mapName  )
{
	guard;

	s_string   t_szName;
	int        t_iSize;
	BYTE*      t_fstart;
	const LstHeader*  t_ptrHeader;


	t_szName = SCENEMAP_DIR;
	t_szName += _mapName;
	t_szName += MAPINFO_FILE;

	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)t_szName.c_str(),t_fstart,TMP_BUFSIZE );
	osassert( t_iSize > 0 );
	t_ptrHeader = (const LstHeader*)t_fstart;

	if( strcmp( t_ptrHeader->m_szId,MAPINFO_ID ) )
	{
		osassertex( false,"错误的lst文件类型....\n" );
		return false;
	}
	osassertex( MAKEFOURCC32( 1,0,0,1 ) == t_ptrHeader->m_dwVersion,
		"地图信息texture.lst文件版本号过低,至少要14版本..\n" );

	m_iMaxXTgNum = t_ptrHeader->m_iXChunks;
	m_iMaxZTgNum = t_ptrHeader->m_iYChunks;
	osassert( (m_iMaxXTgNum>0)&&(m_iMaxZTgNum>0) );
	m_iMaxX = m_iMaxXTgNum*MAX_GROUNDWIDTH;
	m_iMaxZ = m_iMaxZTgNum*MAX_GROUNDWIDTH;
	g_fMapMaxX = m_iMaxX*TILE_WIDTH;
	g_fMapMaxZ = m_iMaxZ*TILE_WIDTH;

	// River added @ 2009-7-29:重置水面的高度信息
	m_sSceneTexData.m_fWaterHeight = 10000.0f;
	

	//syq
	SAFE_DELETE_ARRAY( m_sSceneTexData.m_arrObjNameList );
	SAFE_DELETE_ARRAY( m_sSceneTexData.m_arrTexnameList );

	// 读入模型数目和模型名字数据.
	m_sSceneTexData.m_iObjectNum = t_ptrHeader->m_dwModels;
	m_sSceneTexData.m_arrObjNameList = new 
		os_sceneObjectName[m_sSceneTexData.m_iObjectNum];
	memcpy( m_sSceneTexData.m_arrObjNameList,
		t_fstart + t_ptrHeader->m_dwModelOfs,
		sizeof( os_sceneObjectName )*m_sSceneTexData.m_iObjectNum );

	// 读入纹理数据.
	m_sSceneTexData.m_iTexNum = t_ptrHeader->m_dwTextures;
	osassert( this->m_sSceneTexData.m_iTexNum>=0 );
	m_sSceneTexData.m_arrTexnameList = 
		new os_sceneTexName[m_sSceneTexData.m_iTexNum];
	memcpy( m_sSceneTexData.m_arrTexnameList,
		t_fstart + t_ptrHeader->m_dwTexturesOfs,
		sizeof( os_sceneTexName )*m_sSceneTexData.m_iTexNum );
	m_sSceneTexData.m_iShaderId.resize( m_sSceneTexData.m_iTexNum );

	END_USEGBUF( t_iGBufIdx );

	//! River mod @ 2009-6-20:如果此处不初始化，则可能多次释放第零个纹理
	for( int t_i=0;t_i<m_sSceneTexData.m_iTexNum;t_i ++ )
		m_sSceneTexData.m_iShaderId[t_i] = -1;


	// 读入当前场景用到的地表纹理声音和特效信息
	if( !load_terrainTexSnd( _mapName ) )
		return false;

	if( !m_sSceneTexData.init_wholeSceneData() )
		osassertex( false,"初始整个场景的数据时,失败...\n" );

	return true;

	unguard;
}




	//@{
	//  修改地图的高度信息和碰撞信息的接口。
# if __COLHEI_MODIFY__
/** \brief
*  修改主角人物所在位置的高度信息，os_sceneFM内的m_iCx,m_iCy变量表示的位置。
*  windy 4-27 mod 可能不能正常工作，因为高度信息格式改变了
*  \param float _mod  要修改的高度，可正可负。
*/
bool osc_TGManager::modify_heightInHeroPos( float _mod )
{
	guard;

	osassert( (m_iCenterX>=0)&&(m_iCenterX<(m_iMaxX*2)) );
	osassert( (m_iCenterY>=0)&&(m_iCenterY<(m_iMaxZ*2)) );

	os_tileIdx       t_tileIdx;
	os_TileCollisionInfo*   t_ptrColInfo = NULL;
	int                   t_idx;


	t_tileIdx.m_iX = WORD( m_iCenterX/GROUNDMASK_WIDTH );
	t_tileIdx.m_iY = WORD( m_iCenterY/GROUNDMASK_WIDTH );

	t_ptrColInfo = (os_TileCollisionInfo *)get_sceneColData( t_tileIdx );
	osassertex( t_ptrColInfo,"修改地图的高度信息找不到信息块指针" );
	t_idx = (m_iCenterY%GROUNDMASK_WIDTH)*GROUNDMASK_WIDTH +
		      m_iCenterX%GROUNDMASK_WIDTH;

	//t_ptrColInfo[t_idx].m_fHeight += _mod;

	return true;

	unguard;
}

//! 把鼠标点击的位置设置为跟原来相反的可通过性
bool osc_TGManager::modify_colAtMousePick(  os_pickTileIdx& _ps ,BOOL bUseParm/*=FALSE*/,bool bPass/*=false*/)
{
	guard;

	osassert( (_ps.m_colTileIdx.m_iX<128)&&(_ps.m_colTileIdx.m_iX>=0) );
	osassert( (_ps.m_colTileIdx.m_iY<128)&&(_ps.m_colTileIdx.m_iY>=0) );

# if __QTANG_VERSION__

	//! 相当于直接修改上层.map文件内的碰撞信息
	int t_idxX = _ps.m_tgIdx.m_iX*GROUNDMASK_WIDTH + _ps.m_colTileIdx.m_iX;
	int t_idxY = _ps.m_tgIdx.m_iY*GROUNDMASK_WIDTH + _ps.m_colTileIdx.m_iY;

	if( bUseParm && m_pMapCollInfo )
		m_pMapCollInfo[t_idxY*m_iMaxX*2+t_idxX] = bPass;
	else
		m_pMapCollInfo[t_idxY*m_iMaxX*2+t_idxX] = !m_pMapCollInfo[t_idxY*m_iMaxX*2+t_idxX];

	return true;
# else
	os_TileCollisionInfo*   t_ptrColInfo = NULL;
	int                   t_idx;


	t_ptrColInfo = (os_TileCollisionInfo *)get_sceneColData( _ps.m_tgIdx );
	t_idx = _ps.m_colTileIdx.m_iY*GROUNDMASK_WIDTH+_ps.m_colTileIdx.m_iX;
	if(bUseParm)
	{
		t_ptrColInfo[t_idx].m_bHinder  = bPass;
	}
	else
	{
		t_ptrColInfo[t_idx].m_bHinder = !t_ptrColInfo[t_idx].m_bHinder;
	}
# endif 

	return true;

	unguard;
}


//! 存储修改后的碰撞信息文件,以后批处理融合到地图文件中。
bool osc_TGManager::save_colheiModInfo( FILE*& _file,int _x,int _z )
{
	guard;


# if __QTANG_VERSION__

	return true;

# else

	FILE*             t_file;
	char              t_szFname[256];
	osc_TileGroundPtr   t_ptrTg;
	DWORD             t_dwVer;

	// 存储当前在内存中所有的地图
	for( int t_i=0;t_i<this->m_iActMapNum;t_i ++ )
	{
    	t_ptrTg = m_arrPtrTG[t_i];
		if( NULL == t_ptrTg )
			continue;

		strcpy( t_szFname,t_ptrTg->m_strMapFName.c_str() );
		t_szFname[strlen( t_szFname )-3] = NULL;
		strcat( t_szFname,CCOL_HEADER );

		t_file = fopen( t_szFname,"wb" );
		if(!t_file)
		{
			MessageBox( NULL,t_szFname,va("打开写碰撞信息文件失败[%s]",t_szFname),MB_OK );
			return false;
		}
		fwrite( CCOL_HEADER,sizeof( char ),4,t_file );
		t_dwVer = CCOL_VERSION;
		fwrite( &t_dwVer,sizeof( DWORD ),1,t_file );

		fwrite( t_ptrTg->m_arrCollInfo,sizeof( os_TileCollisionInfo ),
			GROUNDMASK_WIDTH*GROUNDMASK_WIDTH,t_file );

		fclose( t_file );
	}
# endif 

	return true;

	unguard;
}

//! 调入地图的高度和碰撞调节信息。
bool osc_TGManager::load_colheiAdjData( 
			const char* _chName,os_TileCollisionInfo* _arr )
{
	guard;

	osassert( _chName && _chName[0] );
	assertMem( _arr,
		sizeof( os_TileCollisionInfo )*GROUNDMASK_WIDTH*GROUNDMASK_WIDTH,TRUE );

	int   t_iSize;
	char  t_szFname[256];
	BYTE*     t_fstart;

	strcpy( t_szFname,_chName );
	t_szFname[strlen( t_szFname )-3] = NULL;
	strcat( t_szFname,CCOL_HEADER );

	// 不能使用打包数据

	if( file_exist( t_szFname,true ) )
	{
		int   t_iGBufIdx = -1;
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_iSize = read_fileToBuf( t_szFname,t_fstart,TMP_BUFSIZE,true );
		osassertex( t_iSize>0,t_szFname );
		osDebugOut( "调入碰撞信息文件<%s>..\n",t_szFname );


		READ_MEM_OFF( t_szFname,t_fstart,sizeof( char )*4 );
		osassert( strcmp( t_szFname,CCOL_HEADER ) == 0 );
		READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
		osassertex( t_iSize >= CCOL_VERSION,"CCL 文件过低的版本号" );

		READ_MEM_OFF( _arr,t_fstart,
			sizeof( os_TileCollisionInfo )*GROUNDMASK_WIDTH*GROUNDMASK_WIDTH );

		END_USEGBUF( t_iGBufIdx );

	}

	return true;

	unguard;
}

# endif
//@} 

# if __ALLCHR_SIZEADJ__
//! 使用这个接口，可以调整整个场景中所有skinMesh物品的大小，
//! 此处的调整为统一调整。包括树和草相关的物品。
void osc_TGManager::set_allCharSizeAdj( osVec3D& _sizeAdj )
{
	guard;

	osc_skinMesh::m_vec3ScaleAll = _sizeAdj;

	m_skinMeshMgr->reset_smMgrScale();

	unguard;
}
# endif 




//@{
//  电弧相关的特效。
//! 创建一个电弧。
int osc_TGManager::create_eleBolt( os_boltInit* _binit,int _bnum )
{
	return m_boltMgr->create_bolt( _binit,_bnum );
}
//! 重新设置弧。
void osc_TGManager::reset_eleBolt( os_boltInit* _bset,int _idx )
{
	m_boltMgr->reset_bolt( _bset,_idx );
}

//! 删除一个电弧资源。
void osc_TGManager::delete_eleBolt( int _id )
{
	m_boltMgr->delete_bolt( _id );
}
//@}

//@{
/** \brief
*  Draw text on the screen.  
*
*  这个函数是引擎显示文字信息的主要的函数,必须支持Unicode.这个函数必须
*  用在beginScene和endScene函数之间.
*/
int osc_TGManager::disp_string( os_stringDisp* _dstr,int _dnum,int _fontType /*= EFT_DefaultType*/,bool _useZ/* = */,bool _bShadow, RECT* _clipRect )
{
	return ((osc_d3dManager*)m_deviceMgr)->disp_string( _dstr,_dnum,_fontType,_useZ, _bShadow, _clipRect );
}

//! 立即在屏幕上显示字串，参数跟disp_string参数一致。
int osc_TGManager::disp_stringImm( os_stringDisp* _dstr,int _fontType /*= EFT_DefaultType*/,os_screenPicDisp* _cursor/* = NULL */ )
{
	return ((osc_d3dManager*)m_deviceMgr)->disp_stringImm( _dstr,_fontType,_cursor );
}


//! 渲染有颜色解晰功能的文字字串。
int osc_TGManager::disp_string( os_colorInterpretStr* _str,int _fontType /*= EFT_DefaultType*/,bool _useZ/* = false*/, bool _bShadow, RECT* _clipRect/* = NULL*/ )
{
	int            t_iResCINum,t_iResRowNum;

	t_iResRowNum = osc_szCI::interpret_strDisp( m_ptrDstr,_str,t_iResCINum ,_fontType);
	if( t_iResRowNum > 0 )
	{
		disp_string( m_ptrDstr,t_iResRowNum,_fontType,_useZ,  _bShadow, _clipRect );
		return _str->m_iResRowNum;
	}
	else 
		return -1;

}
//@}


osVec2D osc_TGManager::get_mapSize()
{
	osVec2D size;
	size.x = m_iMaxX * TILE_WIDTH;
	size.y = m_iMaxZ * TILE_WIDTH;
	return size;
}