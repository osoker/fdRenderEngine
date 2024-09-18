//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: dlight.cpp
 *
 *  Desc:     处理引擎中的动态光，最后动态光需要和场景相结合。
 *
 *  His:      River created @ 2004-3-3
 *
 *  "育才造士,为国之本"
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/dlight.h"
# include "../../fdrEngine/light.h"
# include "../../backpipe/include/osCamera.h"

//@{
//! 合局光和动态光相关的全局变量。
OSENGINE_API float   g_fAmbiR = 1.0f;
OSENGINE_API float   g_fAmbiG = 1.0f;
OSENGINE_API float   g_fAmbiB = 1.0f;
OSENGINE_API float   g_fDlR = 1.0f;
OSENGINE_API float   g_fDlG = 1.0f;
OSENGINE_API float   g_fDlB = 1.0f;
OSENGINE_API float   g_fDlRadius = 9.0f;
//@} 

//! 控制全局场景中是否使用动态光。
OSENGINE_API BOOL    g_bEnableDLight = true;

//! 场景内固定管道动态光开始的灯光
# define DYNAMICL_OFFSET 1

//! 全局至少有一个LightMgr
osc_dlightMgr*   g_ptrDLightMgr = NULL;;


osc_dlight::osc_dlight()
{
	m_bInuse = false;

	m_bInLerpState = false;

}


//! 在两个动态光的数据之间进行插值。
void osc_dlight::lerp_DLight( float _time )
{
	guard;

	osassert( _time >= 0.0f );

	this->m_fCurTime += _time;

	if( m_fCurTime >= this->m_fTolLerpTime )
	{
		m_bInLerpState = false;
		m_bInuse = false;
	}

	osColorLerp( &m_dlightColor,&m_sSrcColor,
		&m_sDstColor,m_fCurTime/m_fTolLerpTime );


	unguard;
}

//! 开始动态光插值。
void osc_dlight::start_dLightFadeOut( float _totalTime )
{
	guard;

	static osColor   t_sColor( 0.0f,0.0f,0.0f,1.0f );

	osassert( _totalTime >= 0.0f );
	this->m_fCurTime = 0.0f;
	this->m_fTolLerpTime = _totalTime;

	this->m_sSrcColor = this->m_dlightColor;
	this->m_sDstColor = osColor( 0.0f,0.0f,0.0f,1.0f );

	m_bInLerpState = true;


	unguard;
}



osc_dlightMgr::osc_dlightMgr()
{
	m_ptrMpipe = NULL;

	// ATTENTIO TO OPP:非正规方式
	g_ptrDLightMgr = this;

}

osc_dlightMgr::~osc_dlightMgr()
{
	
}

//! 初始化动态光管理器。
void osc_dlightMgr::init_dlightMgr( osc_middlePipe* _mpipe )
{
	guard;// osc_dlightMgr::init_dlightMgr() );

	osassert( _mpipe );
	this->m_ptrMpipe = _mpipe;



	unguard;
}

//! 释放动态光管理器
void osc_dlightMgr::reset_dlightMgr( void )
{
	guard;

	if(!g_ptrLightMgr->IsEnableLight()){
		for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
		{
			if( !m_vecDlight[t_i].m_bInuse )
				continue;

			m_vecDlight[t_i].m_bInuse = false;

			// 可能因为此值没有重设，产生上层删除灯光的错误。
			m_vecDlight[t_i].m_bInLerpState = false;

		}
	}
	

	unguard;
}



//! 帧间move动态光管理器( 场景中删除动态光时，需要淡出)
void osc_dlightMgr::frame_moveDLMgr( void )
{
	guard;

	if(!g_ptrLightMgr->IsEnableLight()){
		float   t_fTime = sg_timer::Instance()->get_lastelatime();
		for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
		{
			if( !m_vecDlight[t_i].m_bInuse )
				continue;
			if( m_vecDlight[t_i].m_bInLerpState )
				m_vecDlight[t_i].lerp_DLight( t_fTime );
		}
	}

	

	unguard;
}


/** \brief
*  创建一个动态光。
*
*  \return int  如果返回-1,则创建失败。
*/
int osc_dlightMgr::create_dynamicLight( os_dlightStruct& _dl )
{
	guard;

	if(!g_ptrLightMgr->IsEnableLight()){
		osc_dlight*  t_ptrL;

		if( !g_bEnableDLight )
			return -1;

		//
		// River @ 2004-7-2: 最多创建四个动态光.
		t_ptrL = NULL;
		int t_i;
		for( t_i=0;t_i<MAX_LIGHTNUM;t_i++ )
		{
			if( !m_vecDlight[t_i].m_bInuse )
			{
				t_ptrL = &m_vecDlight[t_i];
				break;
			}
		}

		// 如果没有空间创建动态光,返回.
		if( !t_ptrL )
			return -1;

		t_ptrL->m_dlightColor = _dl.m_lightColor;
		t_ptrL->m_fRadius = _dl.m_fRadius;
		t_ptrL->m_vec3Pos = _dl.m_vec3Pos;
		t_ptrL->m_bInuse = true;

		return t_i;

	}
	

	return -1;

	unguard;
}



//! 得到动态光管理器内的信息
void osc_dlightMgr::get_dlINfo( os_dlInfo& _info )
{
	guard;

	_info.m_iDLNum = 0;

	for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
	{
		if( !m_vecDlight[t_i].m_bInuse )
			continue;

		_info.m_vecDLPtr[_info.m_iDLNum] = &m_vecDlight[t_i];
		_info.m_iDLId[_info.m_iDLNum] = t_i;
		_info.m_iDLNum ++;

	}

	return;

	unguard;
}

//! 设置一个动态光
void osc_dlightMgr::set_pointLight( os_meshDLight& _dl,bool _enable/* = true*/ )
{
	guard;

	if(!g_ptrLightMgr->IsEnableLight()){
		
		LPDIRECT3DDEVICE9 t_device = m_ptrMpipe->get_device();
		
		// 动态光使用d3d管道内的第二个灯光
		if( (!_dl.is_usedDl())||(!_enable) )
		{
		 	t_device->LightEnable( 1,FALSE );
		}
		else
		{
		 	D3DLIGHT9   t_dl;
		 	int         t_i;
		
		 	for( t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
		 	{
		 		if( -1 == _dl.m_btDLIdx[t_i] )
		 			break;
		
		 		ZeroMemory( &t_dl,sizeof( D3DLIGHT9 ) );
		 		t_dl.Position = m_vecDlight[_dl.m_btDLIdx[t_i]].m_vec3Pos;
		 		t_dl.Diffuse =  m_vecDlight[_dl.m_btDLIdx[t_i]].m_dlightColor;
		 		t_dl.Diffuse.a = 1.0f;
		 		t_dl.Type = D3DLIGHT_POINT;
		 		t_dl.Range = m_vecDlight[_dl.m_btDLIdx[t_i]].m_fRadius;
		 		t_dl.Attenuation0 = 1.0f;
		
		 		t_device->SetLight( t_i+DYNAMICL_OFFSET,&t_dl );
		 		t_device->LightEnable( t_i+DYNAMICL_OFFSET,TRUE );
		 	}
		 	t_device->LightEnable( t_i+DYNAMICL_OFFSET,FALSE );
		}
	}



	unguard;
}



/** \brief
*  得到一个动态光的各项参数。
*/
void osc_dlightMgr::get_dynamicLight( int _id,os_dlightStruct& _dl )
{
	guard;

	osc_dlight*  t_ptrL;

	osassert( (_id>=0)&&(_id<MAX_LIGHTNUM) );

	t_ptrL = &this->m_vecDlight[_id];
	osassert( t_ptrL->m_bInuse );
	_dl.m_lightColor = t_ptrL->m_dlightColor;
	_dl.m_vec3Pos = t_ptrL->m_vec3Pos;
	_dl.m_fRadius = t_ptrL->m_fRadius;

	return;

	unguard;
}


/** \brief
*  FrameMove一个动态光。
*/
void osc_dlightMgr::set_dynamicLight( int _id,os_dlightStruct& _dl )
{
	guard;
	
	if(!g_ptrLightMgr->IsEnableLight()){
		// osc_dlightMgr::set_dynamicLight() );

		osc_dlight*  t_ptrL;

		if( !g_bEnableDLight )
			return;

		osassert( (_id>=0)&&(_id<MAX_LIGHTNUM) );

		t_ptrL = &this->m_vecDlight[_id];
		osassert( t_ptrL->m_bInuse );
		t_ptrL->m_dlightColor = _dl.m_lightColor;
		t_ptrL->m_fRadius = _dl.m_fRadius;
		t_ptrL->m_vec3Pos = _dl.m_vec3Pos;
	}
	

	unguard;
}



/** \brief
*  删除一个动态光。
*/
void osc_dlightMgr::delete_dynamicLight( int _id,float _fadeTime/* = 1.0f*/ )
{
	guard;

	if(!g_ptrLightMgr->IsEnableLight()){

		if( !g_bEnableDLight )
			return;

		osassert( (_id>=0)&&(_id<MAX_LIGHTNUM) );
		if (!(m_vecDlight[_id].m_bInuse )){
			return;
		}


		m_vecDlight[_id].start_dLightFadeOut( _fadeTime );

	}


	unguard;
}
//
//  ATTENTION : 修改为Direction Light.
//! 根据灯光参数填充一个dx灯光。
void osc_dlightMgr::fill_dxGDirLight( D3DLIGHT9& _dxL )
{
	guard;

	osVec3D   t_vec3 = -g_vec3LPos;

	//
	// River @ 2004-7-21:使用方向光。
	osVec3Normalize( &t_vec3,&t_vec3 );

	ZeroMemory( &_dxL,sizeof( D3DLIGHT9 ) );
	_dxL.Direction = t_vec3;
	_dxL.Diffuse =  osColor( g_fDirectLR,g_fDirectLG,g_fDirectLB,1.0f );
	_dxL.Type = D3DLIGHT_DIRECTIONAL;
	_dxL.Attenuation1 = 0.7f;

	return;

	unguard;
}


/** \brief
*  设置当前场景中的动态光为dx兼容格式。
*
*  \param _enable 此值为true时，打开灯光，否则关闭灯光。
*/
void osc_dlightMgr::dxlight_enable( bool _enable,bool _fullAmbi/* = false */ )
{
	guard;
	
	osassert(g_ptrLightMgr);

	// River @ 2010-2-1:不管哪种方式，都必须设置mat,否则播放某些特效时，
	//                  场景内的透明物品可能不可见.
	osassert( m_ptrMpipe  );
	D3DMATERIAL9  t_mtl;
	m_ptrMpipe->set_renderState( D3DRS_LIGHTING,_enable );
	if( !_enable )
		return ;
	memset( &t_mtl,0,sizeof( D3DMATERIAL9 ) );
	t_mtl.Ambient.a = 1.0f;
	t_mtl.Ambient.r = 1.0f;
	t_mtl.Ambient.g = 1.0f;
	t_mtl.Ambient.b = 1.0f;
	t_mtl.Diffuse.a = 1.0f;
	t_mtl.Diffuse.r = 1.0f;
	t_mtl.Diffuse.g = 1.0f;
	t_mtl.Diffuse.b = 1.0f;
	m_ptrMpipe->set_material( t_mtl );

	if(g_ptrLightMgr->IsEnableLight()){

		extern osc_camera*         g_ptrCamera;

		if(_enable){

			osVec3D t_dirLight(g_vec3LPos);
			osColor t_dirColor(g_fDirectLR,g_fDirectLG,g_fDirectLB,1.0f);

			const osColor t_ambientColor = g_ptrLightMgr->FocusCenterRender(g_ptrCamera->get_camFocus(),18.0f,t_dirLight,t_dirColor);

			g_fAmbiR = t_ambientColor.r;
			g_fAmbiG = t_ambientColor.g;
			g_fAmbiB = t_ambientColor.b;

			g_fDirectLR = t_dirColor.r;
			g_fDirectLG = t_dirColor.g;
			g_fDirectLB = t_dirColor.b;

			g_vec3LPos = -t_dirLight;
		}

	}else{



		m_ptrMpipe->set_renderState( D3DRS_SPECULARENABLE, FALSE );
		m_ptrMpipe->set_renderState( D3DRS_DITHERENABLE,   FALSE );

		// 
		// 设置全局光
		osColor   t_color;

		if( _fullAmbi )
			t_color = osColor( 1.0f,1.0f,1.0f,1.0f );
		else
			t_color = osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
		m_ptrMpipe->set_renderState( D3DRS_AMBIENT,t_color );

		//
		// 如果不使用全局的方向光，返回。
		if( !g_bUseGDirLight )
			return;

		// 
		// 打开和关闭灯光。
		if( _enable )
		{	
			// 最少有一个全局的方向光
			fill_dxGDirLight( m_vecDXLight[0] ); 
			m_iDxLightNum = 1;

			m_ptrMpipe->set_dxLight( m_vecDXLight,m_iDxLightNum );

		}
		else
		{
			m_ptrMpipe->set_dxLightDisable( m_iDxLightNum );
		}

	}
	
	


	unguard;
}
