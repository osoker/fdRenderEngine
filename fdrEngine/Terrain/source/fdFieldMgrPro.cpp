//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrPro.cpp
 *
 *  His:      River created @ 2004-2-24
 *
 *  Des:      地图渲染时的处理，地图调试信息的函数等都在这个文件中。
 *   
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include  "../../mfpipe/include/deviceInit.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../backpipe/include/mainEngine.h"
#include "../GeoMipTerrain/osMapTerrain.h"
#include "../../mfpipe/include/osShader.h"
#include "../../mfpipe/include/ostexture.h"



//! 在得到精确的碰撞检测结果中，可以遍历的最多的格子数目。
# define  MAX_SEARCHTILE  24   


/** \brief
 *  在透明物体的排序算法中用到的camera Ptr
 */
osc_camera*         g_ptrCamera = NULL;
//! 场景中保存的设备管理器类指针,这个指针由场景调入函数传入.
I_deviceManager* osc_TGManager::m_deviceMgr = NULL;


//! 以下两个变量用于控制点击场景内地表产生的地表光标,光标的大小，和光标消失的时间
OSENGINE_API float                    g_fTerrCursorSize = 0.4f;
OSENGINE_API float                    g_fTerrCursorVanishTime = 1.5f;


//  ATTENTION TO OPP:
//! 水面的alpha处理使用预处理.
//! 水面的透明深度.
# define    WATERDEEP_ALPHA       1.5f


//! 场景中是否使用雾。
OSENGINE_API BOOL   g_bUseFog = FALSE;
//! 场景中雾的开始点。
OSENGINE_API float  g_fFogStart = 46.0f;
//! 场景中雾的结束点。
OSENGINE_API float  g_fFogEnd = 60.0f;
//! 场景中雾的颜色信息。
OSENGINE_API DWORD  g_dwFogColor = 0x888888ff;



//! 计算出visual Bounding Square值后，需要偏移的值。
//# define  VBS_OFFSET  TILE_WIDTH*1.333333f
//  ATTENTION TO OPP:
//! River mod @ 2004-12-9: 修改此offset的值，加大可见性的范围。
# define  VBS_OFFSET  TILE_WIDTH*4.0f




//! 相机所在的位置.
static osVec3D     g_sVec3CamPos;
/** \brief
 *  由qsort调用的函数指针，使用这个函数，比较两个物体离相机的远近。
 *  
 */
int   alphaObj_compare( const void *_a, const void *_b ) 
{
	guard;

	os_meshRender    *t_ptra,*t_ptrb;
	osVec3D          t_camPos;
	float            t_fDisa,t_fDisb;

	osassert( _a&&_b );

	t_ptra = (os_meshRender*)_a;
	t_ptrb = (os_meshRender*)_b;

	t_camPos = g_sVec3CamPos - t_ptra->m_vec3Pos;
	t_fDisa = osVec3Length( &t_camPos );

	t_camPos = g_sVec3CamPos - t_ptrb->m_vec3Pos;
	t_fDisb = osVec3Length( &t_camPos );

	if( t_fDisa < t_fDisb )
		return 1;
	else
		return -1;
	
	unguard;
}
/** \brief
 *  非透明物品，根据物品的id来确认渲染的先后顺序，多次渲染同一个物品
 */
int opacityObj_compare( const void* _a,const void* _b )
{
	osassert( _a );
	osassert( _b );

	if( ((os_meshRender*)_b)->m_iMeshId > ((os_meshRender*)_a)->m_iMeshId )
		return 1;
	else if( ((os_meshRender*)_b)->m_iMeshId < ((os_meshRender*)_a)->m_iMeshId )
		return -1;
	else
		return 0;
}


bool osc_TGManager::flush_opacityObj( void )
{
	guard;

	// 根据物品的id来对物品进行排序
	qsort( &m_vecOpacityObj[0],
		m_iOpacityObjNum,sizeof( os_meshRender ),opacityObj_compare );

	g_ptrMeshMgr->render_meshVector( &m_vecOpacityObj,m_iOpacityObjNum );
	m_iOpacityObjNum = 0;

	return true;

	unguard;
}
bool osc_TGManager::flush_opacityObjNoMaterial( ID3DXEffect* _pEffect,osMatrix& _ViewProjMat )
{
	guard;
	g_ptrMeshMgr->render_meshVectorNoMaterial( &m_vecOpacityObj,m_iOpacityObjNum ,_pEffect,_ViewProjMat);
	m_iOpacityObjNum = 0;
	return true;
	unguard;
}


/** \brief
*  把地图上的透明物体排序后渲染。
*  
*  render_TGsObject的时候，如果是透明物体，则放到一个数组中，等待这个函数去做处理。
*  算法描述：等使用qsort对当前tg内的透明物体队列进行排序，然后一个个渲染当前的
*  透明物体数组。
*  
*/
bool osc_TGManager::flush_alphaObj(  bool _opacity  )
{
	guard;

	// 对透明物体排序,排序出来的结果为：从第0个物品开始，离相机越来越远。
	// River @ 2008-10-9:先渲染透明面内的非透明部分，此时需要简单排序
	if( _opacity )
	{
		g_ptrCamera->get_curpos( &g_sVec3CamPos );
		qsort( &this->m_vecAlphaObj[0],
			m_iAlphaObjNum,sizeof( os_meshRender ),alphaObj_compare );
	}
	
	g_ptrMeshMgr->render_meshVector( &m_vecAlphaObj, m_iAlphaObjNum, true );

	// 
	// 清零。
	// River @ 2010-3-20:去除物品内半透明部分的渲染，提升渲染效率
	//if( !_opacity )
	m_iAlphaObjNum = 0;

	return true;

	unguard;
}


//! 往管道中推进一个alpha物品.
void osc_TGManager::push_alphaObj( os_meshRender& _mr )
{
	guard;

	if( m_iAlphaObjNum >= (int)m_vecAlphaObj.size() )
	{
		osDebugOut( "过多的渲染物品...\n" );
		return;
	}
	memcpy( &m_vecAlphaObj[m_iAlphaObjNum],&_mr,sizeof( os_meshRender ) );
	m_iAlphaObjNum ++;
	return; 

	unguard;
}

//! 往管道中推进一个非透明的渲染物品
void osc_TGManager::push_opacityObj( os_meshRender& _mr )
{
	guard;

	if( this->m_iOpacityObjNum >= (int)this->m_vecOpacityObj.size() )
	{
		osDebugOut( "过多的渲染物品...\n" );
		return;
	}
	memcpy( &m_vecOpacityObj[m_iOpacityObjNum],&_mr,sizeof( os_meshRender ) );
	m_iOpacityObjNum ++;
	return; 

	unguard;
}



/** \brief
*  用于渲染天空和远景的函数.
*/
void osc_TGManager::render_skyDome( bool _reflect/* = false*/ )
{
	guard;


	float      t_fTime = sg_timer::Instance()->get_lastelatime();
	float      t_farClip;
	osVec3D     t_camPos;
	osMatrix    t_mat;


	g_ptrCamera->get_curpos( &t_camPos );
	t_farClip = g_ptrCamera->get_farPlaneDis();
	if( !_reflect )
	{
		g_ptrCamera->set_farPlaneDis( SKYRENDER_FARDIS );
		g_ptrCamera->get_projmat( t_mat );
		m_middlePipe.set_projMatrix( t_mat );
	}
	
	// River mod @ 2009-4-22: 为什么使用负的相机raw值.
	m_skyMgr.render_sky( &m_middlePipe,t_fTime,
		t_camPos,g_ptrCamera->get_curYaw(),t_farClip,_reflect ); 
	
	m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,FALSE );
	m_middlePipe.set_renderState( D3DRS_ZENABLE,TRUE );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,TRUE );


	// TEST CODE: River mod @ 2008-5-14:
	m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_NONE );

	// 完成巨型公告板的渲染，使用特大的公告板不受远剪切面的影响
	m_bbMgr->render_mgrHugeBB( (I_camera*)g_ptrCamera );

	// 重设远剪切面
	if( !_reflect )
	{
		g_ptrCamera->set_farPlaneDis( t_farClip );
		g_ptrCamera->get_projmat( t_mat );
		m_middlePipe.set_projMatrix( t_mat );
	}

	return;

	unguard;
}


//@{
//  加入场景中的二维屏幕图片显示需要的接口。
//  二维图片的显示需要由客户程序自己进行排序，把需要放到下边的图片优先推进行到场景
//  中进行显示。

/** \brief
*  创建一个二维显示需要的纹理
*　
*  场景切换的时候，所有的纹理会由场景程序自动的释放。
*/
int osc_TGManager::create_scrTexture( const char* _texname )
{
	guard;// osc_TGManager::create_scrTexture() );

	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	
	return t_ptrMgr->create_sceneScrTex( _texname );

	unguard;
}
//syq_mask 创建渲染目标
int osc_TGManager::create_renderTarget( int _width, int _height, D3DFORMAT _format )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	return t_ptrMgr->create_renderTarget( _width, _height, _format );
	unguard;
}
// 
//syq_mask 获得渲染目标
LPDIRECT3DSURFACE9  osc_TGManager::get_renderTarget( int _id )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	return t_ptrMgr->get_renderTarget( _id );
	unguard;
}
LPDIRECT3DTEXTURE9      osc_TGManager::get_renderTexture( int _id )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	return t_ptrMgr->get_renderTexture( _id );
	unguard;
}

LPDIRECT3DTEXTURE9 osc_TGManager::get_sceneScrTex( int _id )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->get_sceneScrTex( _id );
	unguard;
}

//! test注册一个纹理,返回id
int osc_TGManager::register_texid( LPDIRECT3DTEXTURE9 _pTexture )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->register_texid( _pTexture );

	unguard;

}

void osc_TGManager::unregister_texid( int _texid )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->unregister_texid( _texid );

	unguard;
}

int		osc_TGManager::register_targetAsTex( int _targetIdx )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->register_targetAsTex( _targetIdx );

	unguard;	 
}

//! 创建一个纹理
int osc_TGManager::create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->create_texture( _width, _height, _pTexture );

	unguard;
}

/** \brief
*  释放一张贴图。
*/
void osc_TGManager::release_scrTexture( int _texid )
{
	guard;
	static osc_d3dManager*  t_ptrMgr = NULL;
	if( !t_ptrMgr ) 
		t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	t_ptrMgr->release_sceneScrTex( _texid );

	// TEST CODE:用于确保上层没有释放某个id后再次使用此ID.
	osDebugOut( "释放上层ID<%d>...\n",_texid );

	unguard;
}



/** \brief
*  把需要显示的数据结构推进到场景中进行显示。
*
*  这个数据被推进行到场景中的数据结构中，在显示完场景中的三维元素后，
*  开始显示这些二维的元素。
*/
void osc_TGManager::push_scrDisEle( const os_screenPicDisp* _dis )
{
	guard;

	// 如果携进的纹理过多,先渲染目前场景中的图片,然后再推进.
	// 以下处理后变量(m_iCurScrDisNum)变为零
	if( m_iCurScrDisNum == (int)m_vecScrPicDis.size() )
	{
		osDebugOut( "推进的图片过多,渲染队列中的图片...\n" );
		render_scene( true );
	}

	os_screenPicDisp*  t_ptr;

	memcpy( &m_vecScrPicDis[m_iCurScrDisNum],_dis,sizeof( os_screenPicDisp ) );
	t_ptr = &m_vecScrPicDis[m_iCurScrDisNum];
	for( int t_i=0;t_i<4;t_i++ )
	{
		t_ptr->m_scrVer[t_i].m_vecPos.x -= 0.5f;
		t_ptr->m_scrVer[t_i].m_vecPos.y -= 0.5f;
	}
	m_iCurScrDisNum ++;

	unguard;
}


//! 对场景中的二维图片进行渲染。
void osc_TGManager::render_ScrPic( bool _LensPic/* = false*/,bool _renderPic/* = true*/ )
{
	guard;

	// 如果需要显示的二维图片数目小于零，返回。
	if( m_iCurScrDisNum == 0 )
		return;

	if( _renderPic )
	{
		osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;


		osassert( m_vecScrPicDis[0].tex_id >= 0 );

		//syq_mask
		t_ptrMgr->disp_inscreen( &m_vecScrPicDis[0],m_iCurScrDisNum,
			1,_LensPic);
	}

	// 
	// 此处需要设置下一帧或是下一次需要显示的二维图片数目为0.
	m_iCurScrDisNum = 0;

	return;
	
	unguard;
}

//@}



/** \brief
*  创建keyAniMesh.
*  
*  由中间管道负责创建。
*/
int osc_TGManager::create_keyAniMesh( os_aniMeshCreate& _mc )
{
	guard;

	int              t_idx;
	BOOL             t_bReplace;


	// 在场景中创建的osa文件都要受环境光的影响.
	_mc.m_bEffectByAmbiL = true;

	// 为场景创建osaAniMeshInstance.
	t_idx = g_ptrMeshMgr->create_aniMeshFromFile( &_mc,t_bReplace,true );
	osassert( t_idx >= 0 );

	return t_idx;

	unguard;
}




//! 调试用函数，画一个bounding Box.
void osc_TGManager::draw_bbox( os_bbox* _box,DWORD _color,bool _solid/* = false*/ )
{
	guard;

	os_polygon       t_rPolygon;
	osMatrix         t_rotMat;
	const osVec3D*    t_arrVec3Translated;


	t_arrVec3Translated = _box->get_worldVertexPtr(); 
	
	//
	// 构造Bounding Box的渲染数据。
	os_sceVerDiffuse     t_pt[8];

	for( int t_i=0;t_i<8;t_i ++ )
	{
		t_pt[t_i].m_color = _color;
		t_pt[t_i].m_vecPos = t_arrVec3Translated[t_i];
	}

	WORD       t_idx[36];

	if( !_solid )
	{
		// botton.
		t_idx[0] = 0;t_idx[1] = 1;
		t_idx[2] = 1;t_idx[3] = 2;
		t_idx[4] = 2;t_idx[5] = 3;
		t_idx[6] = 3;t_idx[7] = 0;

		// top
		t_idx[ 8] = 4;t_idx[ 9] = 5;
		t_idx[10] = 5;t_idx[11] = 6;
		t_idx[12] = 6;t_idx[13] = 7;
		t_idx[14] = 7;t_idx[15] = 4;

		// col.
		t_idx[16] = 0;t_idx[17] = 4;
		t_idx[18] = 1;t_idx[19] = 5;
		t_idx[20] = 2;t_idx[21] = 6;
		t_idx[22] = 3;t_idx[23] = 7;

		t_rPolygon.m_iPriNum = 12;
		t_rPolygon.m_iPriType = LINELIST;

		m_middlePipe.set_renderState( D3DRS_FILLMODE,D3DFILL_WIREFRAME );
	}
	else
	{

		// -y方向
		t_idx[0] = 0;t_idx[1] = 1;t_idx[2] = 2;
		t_idx[3] = 0;t_idx[4] = 2;t_idx[5] = 3;

		// y方向
		t_idx[6] = 4;t_idx[7] = 7;t_idx[8] = 6;
		t_idx[9] = 4;t_idx[10] = 6;t_idx[11] = 5;

		// -z方向
		t_idx[12] = 0;t_idx[13] = 4;t_idx[14] = 5;
		t_idx[15] = 0;t_idx[16] = 5;t_idx[17] = 1;

		// -z方向
		t_idx[18] = 2;t_idx[19] = 6;t_idx[20] = 7;
		t_idx[21] = 2;t_idx[22] = 7;t_idx[23] = 3;

		// -x方向
		t_idx[24] = 0;t_idx[25] = 3;t_idx[26] = 7;
		t_idx[27] = 0;t_idx[28] = 7;t_idx[29] = 4;

		// x方向
		t_idx[30] = 1;t_idx[31] = 5;t_idx[32] = 6;
		t_idx[33] = 1;t_idx[34] = 6;t_idx[35] = 2;

		t_rPolygon.m_iPriNum = 12;
		t_rPolygon.m_iPriType = TRIANGLELIST;

		m_middlePipe.set_renderState( D3DRS_FILLMODE,D3DFILL_SOLID );
	}

	//
	// 渲染当前的billBoard.
	// 可以根据当前不同的帧索引得到当前帧应该使用的shaderId。
	t_rPolygon.m_iShaderId = g_shaderMgr->add_shader( "line", osc_engine::get_shaderFile( "line" ).c_str() );
	if( t_rPolygon.m_iShaderId < 0 )
		t_rPolygon.m_iShaderId = g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );

	t_rPolygon.set_rawData( t_pt, t_idx );

	t_rPolygon.m_iVerSize = sizeof( os_sceVerDiffuse );
	t_rPolygon.m_iVerNum = 8;

	osMatrix   t_matIdentity;
	osMatrixIdentity( &t_matIdentity );
	m_middlePipe.set_worldMatrix( t_matIdentity );


	//
	// 
	if( !m_middlePipe.render_polygon( &t_rPolygon  ) )
		osassert( false );

	m_middlePipe.set_renderState( D3DRS_FILLMODE,D3DFILL_SOLID );

	return;

	unguard;
}


# ifdef _DEBUG


//! 用于对下层的变量进行输出.
void osc_TGManager::debug_test( void )
{
	guard;

	// 在此输出我们需要的调试信息
	//m_skinMeshMgr->debug_test();

	unguard;
}


# endif 
//! 引擎内画线
LPD3DXLINE osc_TGManager::m_ptrD3dLine = NULL;


//! 调试用函数，画一条线，此函数仅用于调试，接口不全,效率不高。
void osc_TGManager::draw_line( const osVec3D* _verList,int _vnum,
							  DWORD _color/* = 0xffffffff*/,float _width/* =1.0f*/ )
{
	guard;

	// 没有处理对d3dxline的设备reset.
	if( NULL == m_ptrD3dLine )
		D3DXCreateLine( m_middlePipe.get_device(),&m_ptrD3dLine );

	m_ptrD3dLine->SetWidth( _width );

	osMatrix    t_sViewMat,t_sProjMat;
	g_ptrCamera->get_viewmat( t_sViewMat );
	g_ptrCamera->get_projmat( t_sProjMat );

	t_sViewMat = t_sViewMat * t_sProjMat;


	//! 对线段时间我们自己的剪切处理.
	os_ViewFrustum*  t_vf = g_ptrCamera->get_frustum();
	osVec3D  t_vec3[2];

	for( int t_i=0;t_i<_vnum-1;t_i ++ )
	{
		t_vec3[0] = _verList[t_i];
		t_vec3[1] = _verList[t_i+1];

		if( t_vf->clip_lineSeg( t_vec3[0],t_vec3[1] ) )
			m_ptrD3dLine->DrawTransform( t_vec3,2,&t_sViewMat,_color );
	} 

	return;

	unguard;
}

//! 用于设备lost和回复的功能
void osc_TGManager::on_deviceLost( LPDIRECT3DDEVICE9 pDev )
{
	guard;

	if( NULL != m_ptrD3dLine )
		m_ptrD3dLine->OnLostDevice();


	SAFE_RELEASE(mpRefMapSurface);
	SAFE_RELEASE(mBackUpSurface);
	SAFE_RELEASE(m_pRefMapDepth);


	// River Added @ 2007-2-3:设备lost的处理
	SAFE_RELEASE( m_pDepthStencilBackSur );


	// River Added @ 2007-2-10:完整的释放此处用到的纹理
	osc_texture*  t_tex = g_shaderMgr->getTexMgr()->get_textureById( mrefMapId );
	if( t_tex )
		t_tex->release_texture();	
	t_tex = g_shaderMgr->getTexMgr()->get_textureById( m_iRefractionMapId );
	if( t_tex )
		t_tex->release_texture();



# if GALAXY_VERSION
	//release the galaxy blood system
	for(int i = 0;i< (int)m_vecBlood.size();i++){
		m_vecBlood.at(i).DestoryRes();
	}

	m_gridRect.DestroyGridRes();
# endif //GALAXY_VERSION

	m_growEff.DestroyEffRes();
	m_realHDREff.DestroyEffRes();

	unguard;
}


void osc_TGManager::on_deviceRestor( LPDIRECT3DDEVICE9 pDev )
{
	if( NULL != m_ptrD3dLine )
		m_ptrD3dLine->OnResetDevice();

	// windy add,ID不能改变，底层有一些地方使用
	const D3DSURFACE_DESC* t_ptrDesc = m_deviceMgr->get_backBufDesc();
	osc_texture*  t_tex = g_shaderMgr->getTexMgr()->get_textureById( mrefMapId );
	// River @ 2010-9-4:解决有可能重设设备时，出错的bug.
	if( t_tex )
	{
		t_tex->create_texture( "g_refMap.texture",
			t_ptrDesc->Width>>g_iReflectionGrade,
			t_ptrDesc->Height>>g_iReflectionGrade,
			D3DFMT_A8R8G8B8,pDev,10);
	}

	t_tex = g_shaderMgr->getTexMgr()->get_textureById( m_iRefractionMapId );
	// River @ 2010-9-4:解决有可能重设设备时，出错的bug.
	if( t_tex )
	{
		t_tex->create_texture( "g_refractionMap",
			t_ptrDesc->Width>>1,t_ptrDesc->Height>>1,D3DFMT_A8R8G8B8,pDev,10);
	}

	// river mod @ 2008-12-15:depth stencil buffer.
	HRESULT  t_hr;
	t_hr = pDev->CreateDepthStencilSurface( 
		t_ptrDesc->Width>>g_iReflectionGrade,t_ptrDesc->Height>>g_iReflectionGrade,
		D3DFMT_D24S8,D3DMULTISAMPLE_NONE,0,true,&m_pRefMapDepth,NULL );
	if( FAILED( t_hr ) )
		g_bWaterReflect = false;

	
	// 重新得到RefMapTexture
	m_pRefMapTexture = (IDirect3DTexture9*)g_shaderMgr->
		getTexMgr()->get_texfromid(mrefMapId);
	m_pRefractionTexture = (IDirect3DTexture9*)
		g_shaderMgr->getTexMgr()->get_texfromid( m_iRefractionMapId );

	pDev->GetRenderTarget(0,&mBackUpSurface);

	// River Added @ 2007-2-3:设备lost的处理
	pDev->GetDepthStencilSurface( &m_pDepthStencilBackSur );

	//m_middlePipe.get_device()->GetRenderTarget(0,&mBackUpSurface);

# if GALAXY_VERSION
	//release the galaxy blood system
	for(int i = 0;i< (int)m_vecBlood.size();i++){
		m_vecBlood.at(i).InitBlood(pDev);
	}

# endif //GALAXY_VERSION

	if(m_growEff.IsInitialze()){
		m_growEff.CreateRes(pDev);
	}

	// River @ 2010-6-25:屏幕效果的设备找回
	if( g_bScreenBlastWave )
		m_growEff.reinit_screenWave( pDev );

	if(m_realHDREff.IsInitialze()){
		m_realHDREff.CreateRes(pDev);
	}



}


//! 显示鼠标点击位置的图标与实际格子的高度偏差。
# define  CURSOR_DIS_YOFFSET  0.10f

//! 用于调整格子显示的cursor占碰撞格子的大小。
# define  ADJ_CURSOR_SIZE     0.18f

/** \brief
 *  当前的碰撞格子画Cursor时,使用地表数据还是使用高度数据.
 *
 *  如果格子可通过,并且格子上有物品索引,则画此格子的cursor时,使用此格子的高度数据.
 */
bool osc_TGManager::weather_useTileHeight( int _x,int _y,osVec3D* _pos )
{
	guard;
	
	// 传入的值为碰撞格子值。
	osassert( (_x>=0)&&(_x<(m_iMaxX*2)) );
	osassert( (_y>=0)&&(_y<(m_iMaxZ*2)) );


# if __QTANG_VERSION__

	// river mod @ 2009-5-19:可以暂不使用此数据
	return false;

# else
	// 
	os_tileIdx       t_tgIdx;
	int              t_iTx,t_iTy;
	osc_TileGroundPtr   t_ptrTg = NULL;;
	osVec3D          t_vec3Center;


	t_tgIdx.m_iX = (_x/2)/MAX_GROUNDWIDTH;
	t_tgIdx.m_iY = (_y/2)/MAX_GROUNDHEIGHT;

	t_iTx = _x%(GROUNDMASK_WIDTH);
	t_iTy = _y%(GROUNDMASK_WIDTH);

	t_vec3Center.x = _x*(HALF_TWIDTH)+HALF_CTWIDTH;
	t_vec3Center.z = _y*(HALF_TWIDTH)+HALF_CTWIDTH;

	//
	// 如果当前的地图还没有被调入，高度值设为0.
	t_ptrTg = get_tground( t_tgIdx.m_iX,t_tgIdx.m_iY );
	if( !t_ptrTg )
		return false;

	//
	// 得到当前格子的高度值。
	const os_TileCollisionInfo*   t_tileColInfo;
	t_tileColInfo = get_sceneColData( t_tgIdx );
	osassert( t_tileColInfo );
	if( t_tileColInfo[t_iTy*GROUNDMASK_WIDTH+t_iTx].m_bHinder )
		return false;
# endif 

	return false;

	unguard;
}



//! 根据一个碰撞格子在场景中的位置，得到当前碰撞格子对应的四个顶点数据。
bool osc_TGManager::get_colVertexData( osVec3D* _pos,int _x,int _y )
{
	guard;

	osc_TileGroundPtr   t_ptrTg;
	int              t_iX,t_iY;


	osassert( _pos );
	osassert( _x>=0 );
	osassert( _y>=0 );
	osassert( (_x<(m_iMaxX*2)) );
	osassert( (_y<(m_iMaxZ*2)) );

	
	/*
	// River @ 2009-5-19:后来一直使用精确的高度信息来生成decal.
	// 在地表的物品之上,可以显示场景的cursor.
	// 使用碰撞格子的高度,比如地表有桥的情形
	if( weather_useTileHeight( _x,_y,_pos ) )
		return true;
	*/

	// 
	// 先得到当前碰撞格子对应的显示格子。
	t_ptrTg = get_tground( _x/GROUNDMASK_WIDTH,_y/GROUNDMASK_WIDTH );	
	if( !t_ptrTg )
		return false;


	t_iX = (_x%GROUNDMASK_WIDTH)/2;
	t_iY = (_y%GROUNDMASK_WIDTH)/2;

	osVec3D   t_ptrVer[4];
	t_ptrTg->get_polyVertex( t_ptrVer,t_iX,t_iY );
	t_iX = _x%2;
	t_iY = _y%2;
	
	// 
	// 根据显示格子的数据得到碰撞格子的位置，要比实际格子高一些显示。
	if( (t_iX==0)&&(t_iY==0) )
	{
		_pos[0] = t_ptrVer[0];
		osVec3Lerp( &_pos[1],&t_ptrVer[0],&t_ptrVer[1],0.5f );
		osVec3Lerp( &_pos[2],&t_ptrVer[2],&t_ptrVer[1],0.5f );
		osVec3Lerp( &_pos[3],&t_ptrVer[0],&t_ptrVer[2],0.5f );
	}
	if( (t_iX==0)&&(t_iY==1) )
	{
		osVec3Lerp( &_pos[0],&t_ptrVer[0],&t_ptrVer[1],0.5f );
		_pos[1] = t_ptrVer[1];
		osVec3Lerp( &_pos[2],&t_ptrVer[1],&t_ptrVer[3],0.5f );
		osVec3Lerp( &_pos[3],&t_ptrVer[2],&t_ptrVer[1],0.5f );
	}
	if( (t_iX==1)&&(t_iY==1) )
	{
		osVec3Lerp( &_pos[0],&t_ptrVer[1],&t_ptrVer[2],0.5f );
		osVec3Lerp( &_pos[1],&t_ptrVer[1],&t_ptrVer[3],0.5f );
		_pos[2] = t_ptrVer[3];
		osVec3Lerp( &_pos[3],&t_ptrVer[2],&t_ptrVer[3],0.5f );
	}
	if( (t_iX==1)&&(t_iY==0) )
	{
		osVec3Lerp( &_pos[0],&t_ptrVer[0],&t_ptrVer[2],0.5f );
		osVec3Lerp( &_pos[1],&t_ptrVer[2],&t_ptrVer[1],0.5f );
		osVec3Lerp( &_pos[2],&t_ptrVer[2],&t_ptrVer[3],0.5f );
		_pos[3] = t_ptrVer[2];
	}

	for( int t_i=0;t_i<4;t_i ++ )
		_pos[t_i].y += CURSOR_DIS_YOFFSET;

	// 
	// 把碰撞的显示四边形变小一些,顶点1和3在一条线上，在此条线上缩放.
	// 顶点0和2的缩放会麻烦一些,使用顶点1和3的中点进行缩放。
	osVec3D   t_vec3Back,t_vec3MidPt;
	t_vec3Back = _pos[1];
	osVec3Lerp( &_pos[1],&_pos[1],&_pos[3],ADJ_CURSOR_SIZE );
	osVec3Lerp( &_pos[3],&t_vec3Back,&_pos[3],1.0f-ADJ_CURSOR_SIZE );

	osVec3Lerp( &t_vec3MidPt,&_pos[1],&_pos[3],0.5f );
	osVec3Lerp( &_pos[0],&_pos[0],&t_vec3MidPt,ADJ_CURSOR_SIZE*2.0f );
	osVec3Lerp( &_pos[2],&t_vec3MidPt,&_pos[2],1.0f-ADJ_CURSOR_SIZE*2.0f );

	return true;

	unguard;
}


/** \brief
*  加入一个可以显示鼠标点击位置的标志。
*
*/
void osc_TGManager::draw_cursorInScene( os_sceneCursorInit& _scInit )
{
	guard;

	if( _scInit.m_szCursorName[0] )
	{
		os_decalInit   t_sDecalInit;

		t_sDecalInit.m_dwDecalColor = _scInit.m_dwCursorColor;
		t_sDecalInit.m_fRotAgl = 0.0f;
		t_sDecalInit.m_fSize = g_fTerrCursorSize;
		t_sDecalInit.m_fVanishTime = g_fTerrCursorVanishTime;
		strcpy( t_sDecalInit.m_szDecalTex,_scInit.m_szCursorName );
		t_sDecalInit.m_vec3Pos = _scInit.m_vec3CursorPos;
		t_sDecalInit.m_wXFrameNum = _scInit.m_wXFrameNum;
		t_sDecalInit.m_wYFrameNum = _scInit.m_wYFrameNum;
		t_sDecalInit.m_fFrameTime = _scInit.m_fFrameTime;
		t_sDecalInit.m_iAlphaType = 1;

		m_sDecalMgr.create_sceneCursor( t_sDecalInit );
	}
	else if( _scInit.m_szOsaCursorName[0] )
	{
		os_aniMeshCreate    t_mc;

		t_mc.m_bAutoDelete = true;
		t_mc.m_bCreateIns = true;
		t_mc.m_bEffectByAmbiL = false;
		t_mc.m_dwAmbientL = 0xffffffff;
		t_mc.m_ePlayType = OSE_PLAYWITHFRAME;
		t_mc.m_fAngle = 0.0f;
		t_mc.m_iPlayFrames = _scInit.m_iPlayOsaNum;
		strcpy( t_mc.m_szAMName,_scInit.m_szOsaCursorName );

		t_mc.m_vec3Pos = _scInit.m_vec3CursorPos;
		get_detailPos( t_mc.m_vec3Pos );

		static int t_iOsaId = -1;
		BOOL    t_b;

		if( t_iOsaId != -1 )
		{
			if( g_ptrMeshMgr->validate_aniMeshId( t_iOsaId ) )
			{
				g_ptrMeshMgr->delete_animationMesh( t_iOsaId );
				t_iOsaId = -1;
			}
		}
		
		t_iOsaId = g_ptrMeshMgr->create_aniMeshFromFile( &t_mc,t_b,false );
	}

	return;

	unguard;
}





/** \brief
*  当前的polygon的边能否和输入的射线相交。
*
*  \param _vs        传入的polygon的开始顶点。
*  \param _idx       要判断和polygon的哪一条边是否相交。
*  \param _ray       射线的方向。
*  \param _rayStart  射线的开始位置。
*/
bool osc_TGManager::ray_intersectPolyEdge( osVec3D* _vs,
						int _idx,osVec2D& _ray,osVec2D& _rayStart )
{
	osassert( _vs );
	osassert( (_idx>=0)&&(_idx<4) );

	int        t_idx1,t_idx2;
	osVec2D    t_vec2SRay;
	float      t_fS,t_fE;

	// 得到线段的终点索引。
	switch( _idx )
	{
	case 0:
		t_idx1 = 0;
		t_idx2 = 1;
		break;
	case 1:
		t_idx1 = 1;
		t_idx2 = 3;
		break;
	case 2:
		t_idx1 = 3;
		t_idx2 = 2;
		break;
	case 3:
		t_idx1 = 2;
		t_idx2 = 0;
		break;
	default:
		osassert( false );
	}

	t_vec2SRay = 
		osVec2D( _vs[t_idx1].x,_vs[t_idx1].z ) - _rayStart;
	t_fS = osVec2CCW( &_ray,&t_vec2SRay );

	t_vec2SRay = 
		osVec2D( _vs[t_idx2].x,_vs[t_idx2].z ) - _rayStart;
	t_fE = osVec2CCW( &_ray,&t_vec2SRay );

	// 
	// 如果两个值符号不同，则说明这条线段的两点在相机射线的两测。
	if( ((t_fS>=0)&&(t_fE<0))||((t_fS<=0)&&(t_fE>0)) )
		return true;
	else
		return false;
}




//! 从一个格子坐标得到当前格子对应的渲染polygon.
BOOL osc_TGManager::get_rPolygon( osVec3D* _vec,int _tx,int _tz )
{
	osc_TileGroundPtr  t_ptrTg;

	t_ptrTg = get_tground( _tx/MAX_GROUNDWIDTH,_tz/MAX_GROUNDWIDTH );
	
	if( t_ptrTg )
	{
		t_ptrTg->get_polyVertex( _vec, 
			(_tx)%MAX_GROUNDWIDTH,(_tz)%MAX_GROUNDWIDTH );
		return TRUE;
	}
	else
		return FALSE;

}



/** \brief
*  为解决地图中的瞬移问题加入的接口。
*
*  算法描述：
*  如果要瞬移到的地图和当前地图同属于同一个地图，处理。
*  　　如果瞬移的位置小于128，处理。
*　　　如果要瞬移的地图大于128，处理。  
*  如果要瞬移的地图，跟当前的地图不属于同一个地图，处理。
*  
*  \param _mapName 瞬移后地图的名字，可以使用和当前地图同样的名字。
*  \param _xCord,_yCord 瞬移后人物的坐标。
*
*  ATTENTION TO FIX: 空间移动的地图宽度和高度处理
*  测试不同的地图的处理
*  
*/
bool osc_TGManager::spacemove_process( const char* _mapName,int _xCord,int _yCord )
{
	guard;

	osassert( _mapName );
	osassert( _mapName[0] );

	s_string        t_szNmapName;
	bool            t_bOneMap = false;

	// 
	// 关闭一切对激活地图列表的操作
	m_iActMapNum = 0;

	t_szNmapName = SCENEMAP_DIR;
	t_szNmapName += _mapName;
	t_szNmapName += "\\";

	if( m_strMapName == t_szNmapName )
		t_bOneMap = true;

	// 
	// 保存全局的地图信息。
	strcpy( g_szMapName,_mapName );
	strcat( g_szMapName,"\\" );


	// 
	// 重新调入地图。
	s_string                         t_szMapName[MAXTG_INTGMGRMACRO];
	os_sceneLoadStruct                t_sl;

	m_strMapName = t_szNmapName;
	t_sl.m_iX = _xCord;
	t_sl.m_iY = _yCord;
	strcpy( t_sl.m_szMapName,_mapName ); 

	// 如果更换了大地图,则先释放地图使用到的纹理,然后调入新的地图.
	if( !t_bOneMap )
	{
		// River @ 2007-6-2: 场景内的资料版本号
		osc_newTileGround::m_bMapVersionChange = TRUE;

		//! River added @ 2009-6-18: 先释放地图内的资源,否则动态释放，
		//! 有可能删除掉新创建的资源
		for( int t_i = 0;t_i<m_iActMapNum;t_i ++ )
			m_arrPtrTG[t_i]->release_TG();

		// River @ 2009-2-26:
		g_ptrMeshMgr->sceneChange_reset();

		for( int t_i=0;t_i<this->m_sSceneTexData.m_iTexNum;t_i ++ )
		{
			g_shaderMgr->release_shader( 
				m_sSceneTexData.m_iShaderId[t_i],true );
			m_sSceneTexData.m_iShaderId[t_i] = -1;
		}

		m_sDecalMgr.release_allDecal();
		m_ptrTerrainMgr->release_tgLRMgr();

		osDebugOut( "\n\n\n\n地图释放工作完成...\n\n\n\n" );

		if( !load_sceneInfoFile( t_sl.m_szMapName ) )
		{
			osassertex( false,"空间移动时,调入场景信息失败!!!\n" );
			return false;
		}
	}
		
	osDebugOut( "The spcaceMove insert pt is:<%d,%d>..\n",_xCord,_yCord );


	osassert( m_ptrTerrainMgr );
	m_ptrTerrainMgr->space_moveMapLoad( &t_sl,m_iMaxX,m_iMaxZ );	

	return true;
	
	unguard;
}












