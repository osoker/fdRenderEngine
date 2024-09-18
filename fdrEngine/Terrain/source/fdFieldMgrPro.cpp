//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrPro.cpp
 *
 *  His:      River created @ 2004-2-24
 *
 *  Des:      ��ͼ��Ⱦʱ�Ĵ�����ͼ������Ϣ�ĺ����ȶ�������ļ��С�
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



//! �ڵõ���ȷ����ײ������У����Ա��������ĸ�����Ŀ��
# define  MAX_SEARCHTILE  24   


/** \brief
 *  ��͸������������㷨���õ���camera Ptr
 */
osc_camera*         g_ptrCamera = NULL;
//! �����б�����豸��������ָ��,���ָ���ɳ������뺯������.
I_deviceManager* osc_TGManager::m_deviceMgr = NULL;


//! ���������������ڿ��Ƶ�������ڵر�����ĵر���,���Ĵ�С���͹����ʧ��ʱ��
OSENGINE_API float                    g_fTerrCursorSize = 0.4f;
OSENGINE_API float                    g_fTerrCursorVanishTime = 1.5f;


//  ATTENTION TO OPP:
//! ˮ���alpha����ʹ��Ԥ����.
//! ˮ���͸�����.
# define    WATERDEEP_ALPHA       1.5f


//! �������Ƿ�ʹ����
OSENGINE_API BOOL   g_bUseFog = FALSE;
//! ��������Ŀ�ʼ�㡣
OSENGINE_API float  g_fFogStart = 46.0f;
//! ��������Ľ����㡣
OSENGINE_API float  g_fFogEnd = 60.0f;
//! �����������ɫ��Ϣ��
OSENGINE_API DWORD  g_dwFogColor = 0x888888ff;



//! �����visual Bounding Squareֵ����Ҫƫ�Ƶ�ֵ��
//# define  VBS_OFFSET  TILE_WIDTH*1.333333f
//  ATTENTION TO OPP:
//! River mod @ 2004-12-9: �޸Ĵ�offset��ֵ���Ӵ�ɼ��Եķ�Χ��
# define  VBS_OFFSET  TILE_WIDTH*4.0f




//! ������ڵ�λ��.
static osVec3D     g_sVec3CamPos;
/** \brief
 *  ��qsort���õĺ���ָ�룬ʹ������������Ƚ����������������Զ����
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
 *  ��͸����Ʒ��������Ʒ��id��ȷ����Ⱦ���Ⱥ�˳�򣬶����Ⱦͬһ����Ʒ
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

	// ������Ʒ��id������Ʒ��������
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
*  �ѵ�ͼ�ϵ�͸�������������Ⱦ��
*  
*  render_TGsObject��ʱ�������͸�����壬��ŵ�һ�������У��ȴ��������ȥ������
*  �㷨��������ʹ��qsort�Ե�ǰtg�ڵ�͸��������н�������Ȼ��һ������Ⱦ��ǰ��
*  ͸���������顣
*  
*/
bool osc_TGManager::flush_alphaObj(  bool _opacity  )
{
	guard;

	// ��͸����������,��������Ľ��Ϊ���ӵ�0����Ʒ��ʼ�������Խ��ԽԶ��
	// River @ 2008-10-9:����Ⱦ͸�����ڵķ�͸�����֣���ʱ��Ҫ������
	if( _opacity )
	{
		g_ptrCamera->get_curpos( &g_sVec3CamPos );
		qsort( &this->m_vecAlphaObj[0],
			m_iAlphaObjNum,sizeof( os_meshRender ),alphaObj_compare );
	}
	
	g_ptrMeshMgr->render_meshVector( &m_vecAlphaObj, m_iAlphaObjNum, true );

	// 
	// ���㡣
	// River @ 2010-3-20:ȥ����Ʒ�ڰ�͸�����ֵ���Ⱦ��������ȾЧ��
	//if( !_opacity )
	m_iAlphaObjNum = 0;

	return true;

	unguard;
}


//! ���ܵ����ƽ�һ��alpha��Ʒ.
void osc_TGManager::push_alphaObj( os_meshRender& _mr )
{
	guard;

	if( m_iAlphaObjNum >= (int)m_vecAlphaObj.size() )
	{
		osDebugOut( "�������Ⱦ��Ʒ...\n" );
		return;
	}
	memcpy( &m_vecAlphaObj[m_iAlphaObjNum],&_mr,sizeof( os_meshRender ) );
	m_iAlphaObjNum ++;
	return; 

	unguard;
}

//! ���ܵ����ƽ�һ����͸������Ⱦ��Ʒ
void osc_TGManager::push_opacityObj( os_meshRender& _mr )
{
	guard;

	if( this->m_iOpacityObjNum >= (int)this->m_vecOpacityObj.size() )
	{
		osDebugOut( "�������Ⱦ��Ʒ...\n" );
		return;
	}
	memcpy( &m_vecOpacityObj[m_iOpacityObjNum],&_mr,sizeof( os_meshRender ) );
	m_iOpacityObjNum ++;
	return; 

	unguard;
}



/** \brief
*  ������Ⱦ��պ�Զ���ĺ���.
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
	
	// River mod @ 2009-4-22: Ϊʲôʹ�ø������rawֵ.
	m_skyMgr.render_sky( &m_middlePipe,t_fTime,
		t_camPos,g_ptrCamera->get_curYaw(),t_farClip,_reflect ); 
	
	m_middlePipe.set_renderState( D3DRS_ALPHABLENDENABLE,FALSE );
	m_middlePipe.set_renderState( D3DRS_ZENABLE,TRUE );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,TRUE );


	// TEST CODE: River mod @ 2008-5-14:
	m_middlePipe.set_renderState( D3DRS_CULLMODE,D3DCULL_NONE );

	// ��ɾ��͹�������Ⱦ��ʹ���ش�Ĺ���岻��Զ�������Ӱ��
	m_bbMgr->render_mgrHugeBB( (I_camera*)g_ptrCamera );

	// ����Զ������
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
//  ���볡���еĶ�ά��ĻͼƬ��ʾ��Ҫ�Ľӿڡ�
//  ��άͼƬ����ʾ��Ҫ�ɿͻ������Լ��������򣬰���Ҫ�ŵ��±ߵ�ͼƬ�����ƽ��е�����
//  �н�����ʾ��

/** \brief
*  ����һ����ά��ʾ��Ҫ������
*��
*  �����л���ʱ�����е�������ɳ��������Զ����ͷš�
*/
int osc_TGManager::create_scrTexture( const char* _texname )
{
	guard;// osc_TGManager::create_scrTexture() );

	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	
	return t_ptrMgr->create_sceneScrTex( _texname );

	unguard;
}
//syq_mask ������ȾĿ��
int osc_TGManager::create_renderTarget( int _width, int _height, D3DFORMAT _format )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	return t_ptrMgr->create_renderTarget( _width, _height, _format );
	unguard;
}
// 
//syq_mask �����ȾĿ��
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

//! testע��һ������,����id
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

//! ����һ������
int osc_TGManager::create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture )
{
	guard;
	osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->create_texture( _width, _height, _pTexture );

	unguard;
}

/** \brief
*  �ͷ�һ����ͼ��
*/
void osc_TGManager::release_scrTexture( int _texid )
{
	guard;
	static osc_d3dManager*  t_ptrMgr = NULL;
	if( !t_ptrMgr ) 
		t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	t_ptrMgr->release_sceneScrTex( _texid );

	// TEST CODE:����ȷ���ϲ�û���ͷ�ĳ��id���ٴ�ʹ�ô�ID.
	osDebugOut( "�ͷ��ϲ�ID<%d>...\n",_texid );

	unguard;
}



/** \brief
*  ����Ҫ��ʾ�����ݽṹ�ƽ��������н�����ʾ��
*
*  ������ݱ��ƽ��е������е����ݽṹ�У�����ʾ�곡���е���άԪ�غ�
*  ��ʼ��ʾ��Щ��ά��Ԫ�ء�
*/
void osc_TGManager::push_scrDisEle( const os_screenPicDisp* _dis )
{
	guard;

	// ���Я�����������,����ȾĿǰ�����е�ͼƬ,Ȼ�����ƽ�.
	// ���´�������(m_iCurScrDisNum)��Ϊ��
	if( m_iCurScrDisNum == (int)m_vecScrPicDis.size() )
	{
		osDebugOut( "�ƽ���ͼƬ����,��Ⱦ�����е�ͼƬ...\n" );
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


//! �Գ����еĶ�άͼƬ������Ⱦ��
void osc_TGManager::render_ScrPic( bool _LensPic/* = false*/,bool _renderPic/* = true*/ )
{
	guard;

	// �����Ҫ��ʾ�Ķ�άͼƬ��ĿС���㣬���ء�
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
	// �˴���Ҫ������һ֡������һ����Ҫ��ʾ�Ķ�άͼƬ��ĿΪ0.
	m_iCurScrDisNum = 0;

	return;
	
	unguard;
}

//@}



/** \brief
*  ����keyAniMesh.
*  
*  ���м�ܵ����𴴽���
*/
int osc_TGManager::create_keyAniMesh( os_aniMeshCreate& _mc )
{
	guard;

	int              t_idx;
	BOOL             t_bReplace;


	// �ڳ����д�����osa�ļ���Ҫ�ܻ������Ӱ��.
	_mc.m_bEffectByAmbiL = true;

	// Ϊ��������osaAniMeshInstance.
	t_idx = g_ptrMeshMgr->create_aniMeshFromFile( &_mc,t_bReplace,true );
	osassert( t_idx >= 0 );

	return t_idx;

	unguard;
}




//! �����ú�������һ��bounding Box.
void osc_TGManager::draw_bbox( os_bbox* _box,DWORD _color,bool _solid/* = false*/ )
{
	guard;

	os_polygon       t_rPolygon;
	osMatrix         t_rotMat;
	const osVec3D*    t_arrVec3Translated;


	t_arrVec3Translated = _box->get_worldVertexPtr(); 
	
	//
	// ����Bounding Box����Ⱦ���ݡ�
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

		// -y����
		t_idx[0] = 0;t_idx[1] = 1;t_idx[2] = 2;
		t_idx[3] = 0;t_idx[4] = 2;t_idx[5] = 3;

		// y����
		t_idx[6] = 4;t_idx[7] = 7;t_idx[8] = 6;
		t_idx[9] = 4;t_idx[10] = 6;t_idx[11] = 5;

		// -z����
		t_idx[12] = 0;t_idx[13] = 4;t_idx[14] = 5;
		t_idx[15] = 0;t_idx[16] = 5;t_idx[17] = 1;

		// -z����
		t_idx[18] = 2;t_idx[19] = 6;t_idx[20] = 7;
		t_idx[21] = 2;t_idx[22] = 7;t_idx[23] = 3;

		// -x����
		t_idx[24] = 0;t_idx[25] = 3;t_idx[26] = 7;
		t_idx[27] = 0;t_idx[28] = 7;t_idx[29] = 4;

		// x����
		t_idx[30] = 1;t_idx[31] = 5;t_idx[32] = 6;
		t_idx[33] = 1;t_idx[34] = 6;t_idx[35] = 2;

		t_rPolygon.m_iPriNum = 12;
		t_rPolygon.m_iPriType = TRIANGLELIST;

		m_middlePipe.set_renderState( D3DRS_FILLMODE,D3DFILL_SOLID );
	}

	//
	// ��Ⱦ��ǰ��billBoard.
	// ���Ը��ݵ�ǰ��ͬ��֡�����õ���ǰ֡Ӧ��ʹ�õ�shaderId��
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


//! ���ڶ��²�ı����������.
void osc_TGManager::debug_test( void )
{
	guard;

	// �ڴ����������Ҫ�ĵ�����Ϣ
	//m_skinMeshMgr->debug_test();

	unguard;
}


# endif 
//! �����ڻ���
LPD3DXLINE osc_TGManager::m_ptrD3dLine = NULL;


//! �����ú�������һ���ߣ��˺��������ڵ��ԣ��ӿڲ�ȫ,Ч�ʲ��ߡ�
void osc_TGManager::draw_line( const osVec3D* _verList,int _vnum,
							  DWORD _color/* = 0xffffffff*/,float _width/* =1.0f*/ )
{
	guard;

	// û�д����d3dxline���豸reset.
	if( NULL == m_ptrD3dLine )
		D3DXCreateLine( m_middlePipe.get_device(),&m_ptrD3dLine );

	m_ptrD3dLine->SetWidth( _width );

	osMatrix    t_sViewMat,t_sProjMat;
	g_ptrCamera->get_viewmat( t_sViewMat );
	g_ptrCamera->get_projmat( t_sProjMat );

	t_sViewMat = t_sViewMat * t_sProjMat;


	//! ���߶�ʱ�������Լ��ļ��д���.
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

//! �����豸lost�ͻظ��Ĺ���
void osc_TGManager::on_deviceLost( LPDIRECT3DDEVICE9 pDev )
{
	guard;

	if( NULL != m_ptrD3dLine )
		m_ptrD3dLine->OnLostDevice();


	SAFE_RELEASE(mpRefMapSurface);
	SAFE_RELEASE(mBackUpSurface);
	SAFE_RELEASE(m_pRefMapDepth);


	// River Added @ 2007-2-3:�豸lost�Ĵ���
	SAFE_RELEASE( m_pDepthStencilBackSur );


	// River Added @ 2007-2-10:�������ͷŴ˴��õ�������
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

	// windy add,ID���ܸı䣬�ײ���һЩ�ط�ʹ��
	const D3DSURFACE_DESC* t_ptrDesc = m_deviceMgr->get_backBufDesc();
	osc_texture*  t_tex = g_shaderMgr->getTexMgr()->get_textureById( mrefMapId );
	// River @ 2010-9-4:����п��������豸ʱ�������bug.
	if( t_tex )
	{
		t_tex->create_texture( "g_refMap.texture",
			t_ptrDesc->Width>>g_iReflectionGrade,
			t_ptrDesc->Height>>g_iReflectionGrade,
			D3DFMT_A8R8G8B8,pDev,10);
	}

	t_tex = g_shaderMgr->getTexMgr()->get_textureById( m_iRefractionMapId );
	// River @ 2010-9-4:����п��������豸ʱ�������bug.
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

	
	// ���µõ�RefMapTexture
	m_pRefMapTexture = (IDirect3DTexture9*)g_shaderMgr->
		getTexMgr()->get_texfromid(mrefMapId);
	m_pRefractionTexture = (IDirect3DTexture9*)
		g_shaderMgr->getTexMgr()->get_texfromid( m_iRefractionMapId );

	pDev->GetRenderTarget(0,&mBackUpSurface);

	// River Added @ 2007-2-3:�豸lost�Ĵ���
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

	// River @ 2010-6-25:��ĻЧ�����豸�һ�
	if( g_bScreenBlastWave )
		m_growEff.reinit_screenWave( pDev );

	if(m_realHDREff.IsInitialze()){
		m_realHDREff.CreateRes(pDev);
	}



}


//! ��ʾ�����λ�õ�ͼ����ʵ�ʸ��ӵĸ߶�ƫ�
# define  CURSOR_DIS_YOFFSET  0.10f

//! ���ڵ���������ʾ��cursorռ��ײ���ӵĴ�С��
# define  ADJ_CURSOR_SIZE     0.18f

/** \brief
 *  ��ǰ����ײ���ӻ�Cursorʱ,ʹ�õر����ݻ���ʹ�ø߶�����.
 *
 *  ������ӿ�ͨ��,���Ҹ���������Ʒ����,�򻭴˸��ӵ�cursorʱ,ʹ�ô˸��ӵĸ߶�����.
 */
bool osc_TGManager::weather_useTileHeight( int _x,int _y,osVec3D* _pos )
{
	guard;
	
	// �����ֵΪ��ײ����ֵ��
	osassert( (_x>=0)&&(_x<(m_iMaxX*2)) );
	osassert( (_y>=0)&&(_y<(m_iMaxZ*2)) );


# if __QTANG_VERSION__

	// river mod @ 2009-5-19:�����ݲ�ʹ�ô�����
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
	// �����ǰ�ĵ�ͼ��û�б����룬�߶�ֵ��Ϊ0.
	t_ptrTg = get_tground( t_tgIdx.m_iX,t_tgIdx.m_iY );
	if( !t_ptrTg )
		return false;

	//
	// �õ���ǰ���ӵĸ߶�ֵ��
	const os_TileCollisionInfo*   t_tileColInfo;
	t_tileColInfo = get_sceneColData( t_tgIdx );
	osassert( t_tileColInfo );
	if( t_tileColInfo[t_iTy*GROUNDMASK_WIDTH+t_iTx].m_bHinder )
		return false;
# endif 

	return false;

	unguard;
}



//! ����һ����ײ�����ڳ����е�λ�ã��õ���ǰ��ײ���Ӷ�Ӧ���ĸ��������ݡ�
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
	// River @ 2009-5-19:����һֱʹ�þ�ȷ�ĸ߶���Ϣ������decal.
	// �ڵر����Ʒ֮��,������ʾ������cursor.
	// ʹ����ײ���ӵĸ߶�,����ر����ŵ�����
	if( weather_useTileHeight( _x,_y,_pos ) )
		return true;
	*/

	// 
	// �ȵõ���ǰ��ײ���Ӷ�Ӧ����ʾ���ӡ�
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
	// ������ʾ���ӵ����ݵõ���ײ���ӵ�λ�ã�Ҫ��ʵ�ʸ��Ӹ�һЩ��ʾ��
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
	// ����ײ����ʾ�ı��α�СһЩ,����1��3��һ�����ϣ��ڴ�����������.
	// ����0��2�����Ż��鷳һЩ,ʹ�ö���1��3���е�������š�
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
*  ����һ��������ʾ�����λ�õı�־��
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
*  ��ǰ��polygon�ı��ܷ������������ཻ��
*
*  \param _vs        �����polygon�Ŀ�ʼ���㡣
*  \param _idx       Ҫ�жϺ�polygon����һ�����Ƿ��ཻ��
*  \param _ray       ���ߵķ���
*  \param _rayStart  ���ߵĿ�ʼλ�á�
*/
bool osc_TGManager::ray_intersectPolyEdge( osVec3D* _vs,
						int _idx,osVec2D& _ray,osVec2D& _rayStart )
{
	osassert( _vs );
	osassert( (_idx>=0)&&(_idx<4) );

	int        t_idx1,t_idx2;
	osVec2D    t_vec2SRay;
	float      t_fS,t_fE;

	// �õ��߶ε��յ�������
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
	// �������ֵ���Ų�ͬ����˵�������߶ε�������������ߵ����⡣
	if( ((t_fS>=0)&&(t_fE<0))||((t_fS<=0)&&(t_fE>0)) )
		return true;
	else
		return false;
}




//! ��һ����������õ���ǰ���Ӷ�Ӧ����Ⱦpolygon.
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
*  Ϊ�����ͼ�е�˲���������Ľӿڡ�
*
*  �㷨������
*  ���Ҫ˲�Ƶ��ĵ�ͼ�͵�ǰ��ͼͬ����ͬһ����ͼ������
*  �������˲�Ƶ�λ��С��128������
*���������Ҫ˲�Ƶĵ�ͼ����128������  
*  ���Ҫ˲�Ƶĵ�ͼ������ǰ�ĵ�ͼ������ͬһ����ͼ������
*  
*  \param _mapName ˲�ƺ��ͼ�����֣�����ʹ�ú͵�ǰ��ͼͬ�������֡�
*  \param _xCord,_yCord ˲�ƺ���������ꡣ
*
*  ATTENTION TO FIX: �ռ��ƶ��ĵ�ͼ��Ⱥ͸߶ȴ���
*  ���Բ�ͬ�ĵ�ͼ�Ĵ���
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
	// �ر�һ�жԼ����ͼ�б�Ĳ���
	m_iActMapNum = 0;

	t_szNmapName = SCENEMAP_DIR;
	t_szNmapName += _mapName;
	t_szNmapName += "\\";

	if( m_strMapName == t_szNmapName )
		t_bOneMap = true;

	// 
	// ����ȫ�ֵĵ�ͼ��Ϣ��
	strcpy( g_szMapName,_mapName );
	strcat( g_szMapName,"\\" );


	// 
	// ���µ����ͼ��
	s_string                         t_szMapName[MAXTG_INTGMGRMACRO];
	os_sceneLoadStruct                t_sl;

	m_strMapName = t_szNmapName;
	t_sl.m_iX = _xCord;
	t_sl.m_iY = _yCord;
	strcpy( t_sl.m_szMapName,_mapName ); 

	// ��������˴��ͼ,�����ͷŵ�ͼʹ�õ�������,Ȼ������µĵ�ͼ.
	if( !t_bOneMap )
	{
		// River @ 2007-6-2: �����ڵ����ϰ汾��
		osc_newTileGround::m_bMapVersionChange = TRUE;

		//! River added @ 2009-6-18: ���ͷŵ�ͼ�ڵ���Դ,����̬�ͷţ�
		//! �п���ɾ�����´�������Դ
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

		osDebugOut( "\n\n\n\n��ͼ�ͷŹ������...\n\n\n\n" );

		if( !load_sceneInfoFile( t_sl.m_szMapName ) )
		{
			osassertex( false,"�ռ��ƶ�ʱ,���볡����Ϣʧ��!!!\n" );
			return false;
		}
	}
		
	osDebugOut( "The spcaceMove insert pt is:<%d,%d>..\n",_xCord,_yCord );


	osassert( m_ptrTerrainMgr );
	m_ptrTerrainMgr->space_moveMapLoad( &t_sl,m_iMaxX,m_iMaxZ );	

	return true;
	
	unguard;
}












