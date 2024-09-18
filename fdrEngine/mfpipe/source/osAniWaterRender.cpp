/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osMesh.cpp
 * 
 *  His:      River created @ 2008-12-10
 *
 *  Desc:     �����ڵ�ˮ����Ⱦ��
 *  
 * ��һ������"
 *  
 *  ����ss,�ٴβ���ss.
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osMesh.h"
# include "../include/osStateBlock.h"
# include "../include/middlePipe.h"
# include "../../backPipe/include/osCamera.h"

# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"
# include "../include/osEffect.h"

# define  NEW_WATER_TEST 1

D3DVERTEXELEMENT9 t_water[] =
{
	// λ��.
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, 
		D3DDECLUSAGE_POSITION, 0 },

		// ������.
	{ 0, 12,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,
	D3DDECLUSAGE_NORMAL, 0 },

	// ��������
	{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
	D3DDECLUSAGE_TEXCOORD, 0 },

	D3DDECL_END()
};


// ˮ�涥���Declaration
LPDIRECT3DVERTEXDECLARATION9   g_pWaterVertexDeclaration = NULL;


//! ��Ⱦˮ����ص�����
bool osc_meshMgr::render_waterAni()
{
	guard;


	// River mod @ 2009-8-30:���ܲ�����Ӱ��ʱ��Ͳ���ˮ��
	if( !m_ptrWaterEffect )
		return true;

	//! River add 2009-10-9:�����µ��жϣ�����ĳЩ����³���Ĵ���
	//! ��Ҫ����Զ���Ժ�ˮ���effect�����ͷš�
	if( !m_ptrWaterEffect->m_peffect )
		return true;

	float   t_fEtime = sg_timer::Instance()->get_lastelatime();

	HRESULT   t_hr;
	if( !g_pWaterVertexDeclaration )
	{
		m_ptrMPipe->get_device()->
			CreateVertexDeclaration( t_water,&g_pWaterVertexDeclaration );
	}

	t_hr = m_ptrMPipe->get_device()->SetVertexDeclaration( g_pWaterVertexDeclaration );
	osassert( !FAILED( t_hr ) );

	osVec3D  t_pos;
	g_ptrCamera->get_curpos( &t_pos );
	
	extern float gWaterTransparencyDepth;
	t_hr = m_ptrWaterEffect->m_peffect->SetFloat( "WaterTransparencyDepth",gWaterTransparencyDepth );
	osassertex( !FAILED( t_hr ),va( "Effname<%s>..errorstr<%s>...\n",
		m_ptrWaterEffect->get_effectName(),osn_mathFunc::get_errorStr( t_hr ) ) );

	t_hr = m_ptrWaterEffect->m_peffect->SetVector( "view_position",
		&osVec4D( t_pos.x,t_pos.y,t_pos.z,1.0f ) );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr ) );

	t_hr = m_ptrWaterEffect->m_peffect->SetFloat( "time_0_X",sg_timer::Instance()->get_talelatime() );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr ) );

	// River added @ 2009-4-15: ����ˮ�����Ч��,����pixel shader֮������
	// ����pixel shader����Ҫ���κεĴ���
	osVec4D  t_vec4;
	t_vec4.x = 1.0f/(g_fFogEnd-g_fFogStart);
	t_vec4.y = 0.0f;
	t_vec4.z = -g_fFogStart;
	t_vec4.w = 1.0f;
	t_hr = m_ptrWaterEffect->m_peffect->SetVector( "FogVector",&t_vec4 );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr ) );

	// 
	// ��ʼ��ȾaniMeshInstance,�����ٽ�������ȾaniMeshʱ������ɾ���ʹ���Mesh
	//��River @ 2011-1-30:��Ե�Ƕ�̬�����ͼ����Q���Ǿ�̬�ĵ����ͼ������
	//                    �˴�����Ҫ��CS��
	//::EnterCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	for( int t_i = 0;t_i<MAX_MESHRSTYLE;t_i ++ )
	{
		if( m_vecMeshInsNum[t_i] <= 0 )
			continue;

		for( int t_j = 0;t_j<m_vecMeshInsNum[t_i];t_j ++ )
		{
			int t_idx = m_vecMeshInsPtr[t_i][t_j];

			if( t_idx == WORD(-1) )
				continue;

			if( t_idx < MAXEFFECT_OSANUM )
				continue;

			// ��ʼ��Ⱦˮ
			// River Mod @ 2008-12-10:ˮ�治�ڴ���Ⱦ 
			if( (!m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_waterAni()) )
				continue;

			m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].
				render_aniMeshIns( t_fEtime,true );

			if( !m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_amInsInuse() )
			{
				m_vecSceneAMIns.release_node( t_idx-MAXEFFECT_OSANUM );
				m_vecMeshInsPtr[t_i][t_j] = -1;
			}
		}

	}
	//::LeaveCriticalSection( &osc_mapLoadThread::m_sOsaCS );


	// Reset water render state
	m_ptrMPipe->get_device()->SetVertexShader( NULL );
	m_ptrMPipe->get_device()->SetPixelShader( NULL );

	return true;

	unguard;
}

//! ˮ����ص�״̬����
void os_aniNode::waterObj_rsSet( osc_aniMeshIns* _ami,osMatrix& _worldMat )
{
	guard;

	// 
	osassert( _ami );

	if( !::g_bWaterReflect )
		return;

	// ÿһ��ˮ���������ò�ͬ������
	osMatrix   t_sMat,t_sProjMat,mViewProj;
	HRESULT    t_hr;

	osc_effect*  t_ptrEffect = NULL;
	
	if( g_bForceUseCustomShader )
		t_ptrEffect = g_shaderMgr->get_effect( g_iForceCustomShaderIdx );
	else
		t_ptrEffect = g_shaderMgr->get_effect( m_vecShaderId[0] );
	
	osMatrixMultiply( &t_sMat,&_worldMat,&g_matView );
	osMatrixMultiply( &t_sMat,&t_sMat, &g_matProj );

	t_hr = t_ptrEffect->m_peffect->SetMatrix( "worldViewProjMat",&t_sMat );
	osassert( !FAILED( t_hr ) );

	if( !g_bForceUseCustomShader )
	{
		t_hr = t_ptrEffect->m_peffect->SetMatrix( "worldMat",&_worldMat );
		osassert( !FAILED( t_hr ) );
	} 

	unguard;
}

