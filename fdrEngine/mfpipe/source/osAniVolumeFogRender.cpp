# include "stdafx.h"
# include "../include/osMesh.h"
# include "../include/osStateBlock.h"
# include "../include/middlePipe.h"
# include "../../backPipe/include/osCamera.h"

# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"
# include "../include/osEffect.h"
#include "../../effect/include/VolumeFog.h"

D3DVERTEXELEMENT9 t_FogDeclElems[] =
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
LPDIRECT3DVERTEXDECLARATION9   g_pFogVertexDeclaration = NULL;

//! �������ز���
bool  osc_meshMgr::render_FogAni( int ShaderId,int dwType )
{
	guard;

	float   t_fEtime = sg_timer::Instance()->get_lastelatime()*0.1f;

	//float   t_fEtime = 0;

	if( !g_pFogVertexDeclaration )
	{
		m_ptrMPipe->get_device()->
			CreateVertexDeclaration( t_FogDeclElems,&g_pFogVertexDeclaration );
	}
	HRESULT t_hr;
	t_hr = m_ptrMPipe->get_device()->SetVertexDeclaration( g_pFogVertexDeclaration );
	osassert( !FAILED( t_hr ) );

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
			if (dwType==8)
			{
				if( (!m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_FogAni()) )
					continue;
			}
			else if (dwType==2)
			{
				if( (!m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].is_waterAni()) )
					continue;
			}

			extern BOOL gHaveVolumeFog;
			gHaveVolumeFog = TRUE;
			m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].m_FogShaderId = ShaderId;
			m_vecSceneAMIns[t_idx-MAXEFFECT_OSANUM].
				render_aniMeshIns( t_fEtime,false,true);

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