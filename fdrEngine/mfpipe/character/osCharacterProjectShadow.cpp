///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterProjectShadow.cpp
 * 
 *  His:      River created @ 2008-5-19
 *
 *  Desc:     ������Ⱦ�����ڶ�ÿһ��character��Ͷ����Ӱ
 * 
 *  ""
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../include/deviceInit.h"
# include "../include/frontPipe.h"
# include "../../mfpipe/include/osStateBlock.h"
# include "../../mfpipe/include/osEffect.h"
# include "../../backPipe/include/osCamera.h"

//! River added @ 2008-5-23: �Ƿ��ڳ����ڻ������ƽ��Ͷ����Ӱ
OSENGINE_API BOOL g_bDrawProjectShadow = TRUE;

//! ֻ����������Ⱦ
# if GALAXY_VERSION

//! ������ȾIndexedBlend��SkinMesh���ݣ�֧��Geforce���Կ�,���һ�������ȥ��???
//#define PROJECTSHADOW_FVF ( D3DFVF_XYZB2|D3DFVF_DIFFUSE|D3DFVF_NORMAL|D3DFVF_TEX1 )
#define PROJECTSHADOW_FVF ( D3DFVF_XYZB2|D3DFVF_NORMAL|D3DFVF_SPECULAR|D3DFVF_TEX1 )

//! ͶӰ��Ӱ�õ���ƽ��
osMatrix osc_skinMeshMgr::m_matShadow;


/**  �㷨����
1: ������Ⱦ״̬���뱳����ϵ�״̬��ʹ��ȫ�ֵ�anbient��ɫ��
2: ��������Ϊ�գ�Zд�룬Z�Ƚ�,������ɫΪ�գ��뱳��Alpha Blend.
3: ���������ڵķ����������ò�ͬ�ľ������������ProjectShadow����Ĺ���?
4: ��ĳһ�����������κζ���ʹ��project����ѹ����һ��ƽ���ϣ�����Щ������


ע�����������������Ҳ���뿼�ǽ�ȥ.

*/

//! ������ǰͶӰ��Ӱ�õ��ľ���
void osc_skinMesh::construct_shadowMat( osMatrix& _shaMat )
{
	guard;

	osMatrix         t_matWorld;
	osMatrix         t_matRot;
	osMatrix         t_matScale;

	// ��������ڲ�ֵ״̬
	osMatrixScaling(&t_matScale,m_vScale.x,m_vScale.y,m_vScale.z );	
	osMatrixTranslation( &t_matWorld, m_vec3SMPos.x+m_vec3AdjPos.x,
		m_vec3SMPos.y+m_vec3AdjPos.y,m_vec3SMPos.z+m_vec3AdjPos.z );
	osMatrixMultiply( &t_matWorld,&t_matScale,&t_matWorld );
	osMatrixRotationY( &t_matRot,m_fFinalRotAgl );	
	osMatrixMultiply( &t_matWorld,&t_matRot,&t_matWorld );

	_shaMat = t_matWorld * osc_skinMeshMgr::m_matShadow;

	return;

	unguard;
}

//! ����ǰskinMesh��Ӧ��projectShadow.
void osc_skinMesh::draw_projectShadow( void )
{
	guard;

	if( !this->m_bProjectShadow )
		return;

	// TEST CODE:��ʱֻ����һ�������������岿λ
	// �������ﵱǰ��λ�ú���ת�Ƕȣ������������ݴ�����һ�㣬���д���
	os_smMeshDevdep* t_ptrSm;
	osMatrix         t_sMatWorld;
	t_ptrSm = g_smDevdepMgr->get_smMesh( m_vecBpmIns[0].m_ptrBodyPartMesh->m_iDevdepResId );
	osassert( t_ptrSm );
	construct_shadowMat( t_sMatWorld );
	m_pd3dDevice->SetTransform( D3DTS_WORLD,&t_sMatWorld );
	m_pd3dDevice->SetStreamSource( 0,t_ptrSm->m_pBuf,0,t_ptrSm->m_iVerSize );
	m_pd3dDevice->SetIndices( t_ptrSm->m_pIdxBuf );
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0,0,
			               t_ptrSm->m_iVerNum,0,t_ptrSm->m_iIdxNum/3	);


	return;

	unguard;
}

//! Ͷ����Ӱ��״̬��
int osc_skinMeshMgr::m_iProjectShadowSB = -1;

//! ����Ͷ����Ӱ��״̬��
void osc_skinMeshMgr::create_smMgrProjectShadowSB( void )
{
	guard;

	osc_d3dManager*     t_ptrDevMgr = NULL;
	t_ptrDevMgr = (osc_d3dManager*)::get_deviceManagerPtr();

	m_iProjectShadowSB = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iProjectShadowSB>= 0 );
	os_stateBlockMgr::Instance()->start_stateBlock();

	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESS );

	m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );

	m_pd3dDevice->SetTexture( 0,NULL );
	m_pd3dDevice->SetFVF( PROJECTSHADOW_FVF );
	
	osColor   t_color( 0.1f,0.1f,0.1f,0.5f );
	m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,(DWORD)t_color );
	m_pd3dDevice->LightEnable( 0,false );

	//! ATTENTION TO FIX:��������㴦�������£���������µĴ���
	//  "Cannot invert world-view matrix",���ʹ�ô�Ӳ����Ⱦ��
	//   ��Ӳ����������ִ���
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,TRUE );

	D3DMATERIAL9   t_sMaterial;
	memset( &t_sMaterial,0,sizeof( D3DMATERIAL9 ) );
	t_sMaterial.Diffuse.r = 1.0f;
	t_sMaterial.Diffuse.g = 1.0f;
	t_sMaterial.Diffuse.b = 1.0f;
	t_sMaterial.Diffuse.a = 0.4f;

	t_sMaterial.Ambient.r = 1.0f;
	t_sMaterial.Ambient.g = 1.0f;
	t_sMaterial.Ambient.b = 1.0f;
	t_sMaterial.Ambient.a = 1.0f;

	m_pd3dDevice->SetMaterial( &t_sMaterial );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iProjectShadowSB );


	unguard;
}




//! ������������skinMesh��projectShadow
void osc_skinMeshMgr::draw_projectShadow( void )
{
	guard;

	// ȫ�ֵĻ�Ͷ����Ӱ����
	if( !g_bDrawProjectShadow )
		return ;		

	//! һ���Թ�����Ӱ��Ⱦ�õ���Ͷ�����
	osPlane   t_planeGround( 0.0f,1.0f,0.0f,-0.2f );
	osVec4D   t_vec4 = D3DXVECTOR4( g_vec3LPos.x,g_vec3LPos.y,g_vec3LPos.z,0.0f );
	osMatrixShadow( &osc_skinMeshMgr::m_matShadow,&t_vec4,&t_planeGround );

	os_stateBlockMgr* t_ptrSBM = os_stateBlockMgr::Instance();

	// ȷ����Ⱦ״̬��,��ʹ�õ��豸.
	if( (m_iProjectShadowSB >=0)&&
		t_ptrSBM->validate_stateBlockId( m_iProjectShadowSB ) )
	{
		t_ptrSBM->apply_stateBlock( m_iProjectShadowSB );
	}
	else
	{
		create_smMgrProjectShadowSB();	
		t_ptrSBM->apply_stateBlock( m_iProjectShadowSB );
	}



	for( int t_i=0;t_i<m_iInviewSMNum;t_i++ )
	{
		m_vecInViewSM[t_i]->draw_projectShadow();
	}

	unguard;
}



# endif 
