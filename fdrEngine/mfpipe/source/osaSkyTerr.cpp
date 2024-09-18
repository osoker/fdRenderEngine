/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osaSkyTerr.cpp
 * 
 *  His:      River created @ 2003-12-27
 *
 *  Desc:     ������Ⱦosa����պ�Զɽ.
 *  
 *  "�����������һ��ǿ�ҵľ�����ʶ,���ֲ�������������˾޴�Ķ������Ժ��С�
 *   �������ֲ�Ը���Լ�����˥��Ķ���֧�������ǡ�"
 *  
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osMesh.h"
# include "../include/osStateBlock.h"
# include "../include/middlePipe.h"
# include "../../interface/miskFunc.h"
# include "../../backPipe/include/osCamera.h"



// windy add 7.4 Ϊ����Զɽ������ò�ͬ����
# define  FAR_SKYMAXSIZE	550.0f

//! ��պ�Զ�������ʱ��
OSENGINE_API float g_fSkyTerrAnitime = 5.0f;
//! ���������ת���ٶ�
OSENGINE_API float g_fBkSkyRotSpeed  = OS_PI/800.0f;

OSENGINE_API float g_fBkSky1RotSpeed  = OS_PI/800.0f;

OSENGINE_API float g_fBkSky2RotSpeed  = OS_PI/800.0f;





osc_osaSkyTerr::osc_osaSkyTerr()
{
	m_ptrMeshMgr = NULL;
	
	this->m_szSky0Name[0] = NULL;
	this->m_szSky1Name[0] = NULL;

	this->m_szTerrName[0] = NULL;
	

	m_iCurActiveSkyIdx = -1;
	m_iCurActiveTerrIdx = -1;

	m_fSkyAlphaVal = 0.0f;
	m_fTerrAlphaVal = 0.0f;

	m_fSkyRotAgl = 0.0f;
	m_fSkyRotAgl1 = 0.0f;
	m_fSkyRotAgl2 = 0.0f;
	memset(m_szSky1,0,128);
	memset(m_szSky2,0,128);
	memset(m_szBolt,0,128);
	m_szBoltRate = 0.5f;
	m_fBoltAngle = 0.0f;
	m_BoltHeight = 20.0f;
	m_BoltColor = osColor(1.0f,1.0f,1.0f,1.0f);
	
	//@{
	// Windy mod @ 2005-9-12 13:49:42
	//Desc: 
//	D3DXMatrixPerspectiveFovLH( &m_sky3matProj, D3DX_PI/4, 1.5, 200.0f, 10000.0f );
	//@}


}

osc_osaSkyTerr::~osc_osaSkyTerr()
{

}

//! ��ʼ����̬����
template<> osc_osaSkyTerr* ost_Singleton<osc_osaSkyTerr>::m_ptrSingleton = NULL;


/** �õ�һ��osc_osaSkyTerr��Instanceָ��.
*/
osc_osaSkyTerr* osc_osaSkyTerr::Instance( void )
{
	guard;
	
	if( !osc_osaSkyTerr::m_ptrSingleton )
	{

		osc_osaSkyTerr::m_ptrSingleton = new osc_osaSkyTerr;
		
		osassert( osc_osaSkyTerr::m_ptrSingleton );
	}
	
	return osc_osaSkyTerr::m_ptrSingleton;

	unguard;
}

/** ɾ��һ��osc_osaSkyTerr��Instanceָ��.
*/
void osc_osaSkyTerr::DInstance( void )
{
	guard;
	
	SAFE_DELETE( osc_osaSkyTerr::m_ptrSingleton );

	unguard;
}



//! 
bool osc_osaSkyTerr::init_skyTerr(  osc_meshMgr* _meshMgr,
								  const char* _skyName,const char* _terrName)
{
	guard;

	osassert( _meshMgr );

	m_ptrMeshMgr = _meshMgr;

	os_aniMeshCreate  t_mc;

	t_mc.m_bAutoDelete = false;
	t_mc.m_bEffectByAmbiL = true;
	t_mc.m_ePlayType = OSE_PLAYFOREVER;
	t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );

	if( file_exist( (char*)_skyName ) )
	{
		strcpy( t_mc.m_szAMName,_skyName );
		if( !m_sSkyMesh[0].create_meshFromfile( t_mc ) )
			osassert( false );
		m_sSkyMesh[0].add_ref();
		strcpy( m_szSky0Name,t_mc.m_szAMName );
		m_iCurActiveSkyIdx = 0;
	}

	//if( file_exist( (char*)_terrName ) )
	//{
	//	strcpy( t_mc.m_szAMName,_terrName );// );
	//	if( !m_sTerrMesh[0].create_meshFromfile( t_mc ) )
	//		osassert( false );
	//	m_sTerrMesh[0].add_ref();
	//	strcpy( m_szTerr0Name,t_mc.m_szAMName );
	//	m_iCurActiveTerrIdx = 0;
	//}

	m_fSkyAlphaVal = 1.0f;
	m_fTerrAlphaVal = 1.0f;
	
	memset(m_szSky1,0,sizeof(m_szSky1));
	memset(m_szSky2,0,sizeof(m_szSky2));

	//@}
	return true;

	unguard;
}

//! ����պ�Զ�����ۺ����ݽ��д���
void osc_osaSkyTerr::frame_moveSkyTerr( bool _reflect )
{
	guard;

	float t_fTime = sg_timer::Instance()->get_lastelatime();

	// ������Ⱦ��ֱ�ӷ���
	if( _reflect )
		return;

	if( m_fSkyAlphaVal < 1.0f )
	{
		m_fSkyAlphaVal += (t_fTime/g_fSkyTerrAnitime);

		if( m_fSkyAlphaVal >= 1.0f )
			m_fSkyAlphaVal = 1.0f;
	}

	/*if( m_fTerrAlphaVal < 1.0f )
	{
		m_fTerrAlphaVal += (t_fTime/g_fSkyTerrAnitime);

		if( m_fTerrAlphaVal >= 1.0f )
			m_fTerrAlphaVal = 1.0f;
	}*/

	m_fSkyRotAgl += (g_fBkSkyRotSpeed*t_fTime);
	if( m_fSkyRotAgl > (OS_PI*2.0f) )
		m_fSkyRotAgl = float_mod( m_fSkyRotAgl,OS_PI*2.0f);

	m_fSkyRotAgl1 += (g_fBkSky1RotSpeed*t_fTime);
	if( m_fSkyRotAgl1 > (OS_PI*2.0f) )
		m_fSkyRotAgl1 = float_mod( m_fSkyRotAgl1,OS_PI*2.0f);

	m_fSkyRotAgl2 += (g_fBkSky2RotSpeed*t_fTime);
	if( m_fSkyRotAgl2 > (OS_PI*2.0f) )
		m_fSkyRotAgl2 = float_mod( m_fSkyRotAgl2,OS_PI*2.0f);
	

	unguard;
}


/** \brief
*  �����µ����,���Ҫ��������ո�Ŀǰʹ�õ���ͬ���򲻴���
*
*  �������ͬ����ͬĿǰ����alpha���ɡ�
*	_skyName3 Ϊ���������OSA�ļ���
*/
bool osc_osaSkyTerr::create_newSky( const char* _skyName,const char* _skyName1,const char* _skyName2 ,const char* _bolt /*= NULL*/,float rate /*= 1.0f*/,float _angle/*=0.0f*/,osColor _color  /*= osColor(1.0f,1.0f,1.0f,1.0f)*/,float _boltHeight /*= 20.0f*/)
{
	guard;

	osassert( m_ptrMeshMgr );

	if( !((m_iCurActiveSkyIdx==0)||(m_iCurActiveSkyIdx==1)) )
	{
		osDebugOut( "���ܴ����µı������...\n" );
		return true;
	} 

	// �ϲ㱣֤������ȷ�Ĳ���
	osassert( _skyName && _skyName[0] );

	os_aniMeshCreate  t_mc;


	t_mc.m_bAutoDelete = false;
	t_mc.m_bEffectByAmbiL = true;
	t_mc.m_ePlayType = OSE_PLAYFOREVER;
	t_mc.m_fAngle = 0.0f;
	t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
	t_mc.m_fSizeScale = 1.0f;
	strcpy( t_mc.m_szAMName,_skyName );
	osassert( strlen( _skyName ) < 64 );

	if( m_iCurActiveSkyIdx == 0 )
	{
		if( m_szSky1Name[0] != NULL )
		{
	
			if( strcmp( _skyName,m_szSky1Name ) != 0 )
			{
				if( m_sSkyMesh[1].get_ref() > 0 )
					m_sSkyMesh[1].release_ref( (DWORD)(m_ptrMeshMgr->m_ptrMPipe) );

				if( !m_sSkyMesh[1].create_meshFromfile( t_mc ) )
					osassert( false );
				m_sSkyMesh[1].add_ref();
				
				// TEST CODE:
				m_sSkyMesh[1].assert_shader();

			}else
			{
				// ��Ϊ�Ѿ�������ͬ��mesh,���Բ���Ҫ���κεĹ�����
			}
		}
		else
		{
			if( !m_sSkyMesh[1].create_meshFromfile( t_mc ) )
					osassert( false );
			m_sSkyMesh[1].add_ref();

			// TEST CODE:
			m_sSkyMesh[1].assert_shader();

		}

		strcpy( m_szSky1Name,_skyName );
		m_iCurActiveSkyIdx = 1;
		m_fSkyAlphaVal = 0.0f;
	}
	else
	{
		if( m_szSky0Name[0] != NULL )
		{
			if( strcmp( _skyName,m_szSky0Name ) != 0 )
			{
				if( m_sSkyMesh[0].get_ref() > 0 )
					m_sSkyMesh[0].release_ref( (DWORD)m_ptrMeshMgr->m_ptrMPipe );
				if( !m_sSkyMesh[0].create_meshFromfile( t_mc ) )
					osassert( false );
				m_sSkyMesh[0].add_ref();

				// TEST CODE: why not add ref??
				m_sSkyMesh[0].assert_shader();

			}
			else
			{
				// ����Ҫ����
			}
		}
		else
		{
			if( !m_sSkyMesh[0].create_meshFromfile( t_mc ) )
				osassert( false );
			m_sSkyMesh[0].add_ref();
			
			// TEST CODE:
			m_sSkyMesh[0].assert_shader();

		}

		strcpy( m_szSky0Name,_skyName );
		m_iCurActiveSkyIdx = 0;
		m_fSkyAlphaVal = 0.0f;
	}

	//@}
	if(_skyName1&&_skyName1[0])
	{
		strcpy( t_mc.m_szAMName,_skyName1 );
		//strcpy( t_mc.m_szAMName,"keyani\\1.OSA" );
		
		if( strcmp( _skyName1,m_szSky1) != 0 )
		{
		//	if( m_sSkyMesh1.get_ref() > 0 )
		//		m_sSkyMesh1.release_ref( (DWORD)m_ptrMeshMgr->m_ptrMPipe );
			m_SkymeshIns1.delete_curIns();
			if( !m_sSkyMesh1.create_meshFromfile( t_mc ) )
				osassert( false );
			t_mc.m_bAutoDelete = false;
			t_mc.m_bEffectByAmbiL = true;
			t_mc.m_ePlayType = OSE_PLAYFOREVER;
			t_mc.m_fAngle = 0.0f;
			t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
			t_mc.m_fSizeScale = 1.0f;
			//strcpy( m_szSky1, _skyName1 );
		
			strcpy( m_szSky1,_skyName1);
			
			m_SkymeshIns1.create_aniMeshIns( &m_sSkyMesh1,t_mc);
		}

		
	}
	else
	{
		memset(m_szSky1,0,128);
	}

	if(_skyName2&&(strlen(_skyName2)!=0))
	{
		strcpy( t_mc.m_szAMName,_skyName2 );
		if( strcmp( _skyName2,m_szSky2) != 0 )
		{
		//	if( m_sSkyMesh2.get_ref() > 0 )
		//		m_sSkyMesh2.release_ref( (DWORD)m_ptrMeshMgr->m_ptrMPipe );
			m_SkymeshIns2.delete_curIns();
			if( !m_sSkyMesh2.create_meshFromfile( t_mc ) )
				osassert( false );

			t_mc.m_bAutoDelete = false;
			t_mc.m_bEffectByAmbiL = true;
			t_mc.m_ePlayType = OSE_PLAYFOREVER;
			t_mc.m_fAngle = 0.0f;
			t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
			t_mc.m_fSizeScale = 1.0f;
			strcpy( m_szSky2,_skyName2);
			//strcpy( m_szSky2, m_szSky2 );
			
			m_SkymeshIns2.create_aniMeshIns( &m_sSkyMesh2,t_mc);
		}
	}
	else
	{
		memset(m_szSky2,0,128);
	}
	//����
	
	if(_bolt&&(strlen(_bolt)!=0))
	{
		m_szBoltRate = rate;
		m_fBoltAngle = _angle;
		m_BoltColor = _color;
		m_BoltHeight = _boltHeight;

		strcpy( t_mc.m_szAMName,_bolt );
		if( strcmp( _bolt,m_szBolt) != 0 )
		{
			m_BoltmeshIns.delete_curIns();
			if( !m_sBoltMesh.create_meshFromfile( t_mc ) )
				osassert( false );

			t_mc.m_bAutoDelete = false;
			t_mc.m_bEffectByAmbiL = true;
			t_mc.m_ePlayType = OSE_PLAYFOREVER;
			t_mc.m_fAngle = 0.0f;
			t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
			t_mc.m_fSizeScale = 1.0f;
		
			strcpy( m_szBolt,_bolt);
			
			m_BoltmeshIns.create_aniMeshIns( &m_sBoltMesh,t_mc);
		}

		
	}
	else
	{
		memset(m_szBolt,0,128);
	}

	return true;
	unguard;
}

/** \brief
*  �����µ�Զ����
*
*  Զ��ͬĿǰ����alpha���ɡ�
*/
bool osc_osaSkyTerr::create_newTerr( const char* _terrName )
{
	guard;


	//osassert( ((m_iCurActiveTerrIdx==0)||(m_iCurActiveTerrIdx==1)) );
	osassert( m_ptrMeshMgr );

	if( (_terrName==NULL)||(_terrName[0]==NULL) )
	{

		m_sTerrMeshIns.delete_curIns();
		m_szTerrName[0]=0;
		return true;
	}

	os_aniMeshCreate  t_mc;

	t_mc.m_bAutoDelete = false;
	t_mc.m_bEffectByAmbiL = true;
	t_mc.m_ePlayType = OSE_PLAYFOREVER;
	t_mc.m_fAngle = 0.0f;
	t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
	strcpy( t_mc.m_szAMName,_terrName );
	osassert( strlen( _terrName ) < 64 );

	m_sTerrMeshIns.delete_curIns();
	if( !m_sTerrMesh.create_meshFromfile( t_mc ) )
		osassert( false );

	t_mc.m_bAutoDelete = false;
	t_mc.m_bEffectByAmbiL = true;
	t_mc.m_ePlayType = OSE_PLAYFOREVER;
	t_mc.m_fAngle = 0.0f;
	t_mc.m_vec3Pos = osVec3D( 0.0f,0.0f,0.0f );
	t_mc.m_fSizeScale = 1.0f;

	m_sTerrMeshIns.create_aniMeshIns( &m_sTerrMesh,t_mc);

	strcpy( m_szTerrName,_terrName );

	return true;


	unguard;
}




//! ���ݲ�ͬ��״̬����Ⱦ��գ�������Ҫ������ջ���Զɽ���ۺϡ�
void osc_osaSkyTerr::set_skyRenderState( void )
{
	guard;

	osassert( m_ptrMeshMgr );

	m_ptrMeshMgr->set_animeshRState( true );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_ALPHABLENDENABLE,true );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_ZENABLE,FALSE );

	// River @ 2007-4-4:��ΪҪʹ��material��Ϊ��չ��ɵ�Alpha,���Եƹ�����.
	//                  Ϊ��ά��ԭ��������Ч��,�˴���ȫ�ֹ�ʹ��ȫ�׵ƹ�
	//m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_LIGHTING,TRUE );
	//m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_AMBIENT,0xffffffff );

	// ��ʹ�÷����
	m_ptrMeshMgr->m_pd3dDevice->LightEnable( 0,FALSE );
	m_ptrMeshMgr->m_pd3dDevice->LightEnable( 1,FALSE );

	unguard;
}


//! ��Ⱦ���.
void osc_osaSkyTerr::render_bkSky( osVec3D& _cen,bool _reflect )
{
	guard;

	// �����Ⱦʱ�����ı���.
	static osVec3D   t_vec3( 1.0f,1.0f,1.0f );

	float   t_fRotAngle;

	if( m_iCurActiveSkyIdx == 0 )
	{
		osassert( m_szSky0Name[0] != NULL );

		if( !float_equal( m_fSkyAlphaVal,1.0f ) )
		{
			if( m_szSky1Name[0] != NULL )
			{
				m_sSkyMesh[1].render_aniMesh( _cen,
					m_fSkyRotAgl,t_vec3,m_ptrMeshMgr->m_ptrMPipe,1.0f );
			}
		}

		if( _reflect )
			t_fRotAngle = m_fSkyRotAgl;
		else
			t_fRotAngle = m_fSkyRotAgl;


		//m_sTerrMeshIns.render_aniMeshIns(t_fEtime,true);

		m_sSkyMesh[0].render_aniMesh( _cen,t_fRotAngle,
			t_vec3,m_ptrMeshMgr->m_ptrMPipe,m_fSkyAlphaVal );
	}
	else
	{
		osassert( m_iCurActiveSkyIdx == 1 );
		osassert( m_szSky1Name[0] != NULL );

		if( _reflect )
			t_fRotAngle = m_fSkyRotAgl;
		else
			t_fRotAngle = m_fSkyRotAgl;

		if( !float_equal( m_fSkyAlphaVal,1.0f ) )
		{
			if( m_szSky0Name[0] != NULL )
			{
				m_sSkyMesh[0].render_aniMesh( _cen,
					t_fRotAngle,t_vec3,m_ptrMeshMgr->m_ptrMPipe,1.0f );
			}
		}

		m_sSkyMesh[1].render_aniMesh( _cen,t_fRotAngle,
			t_vec3,m_ptrMeshMgr->m_ptrMPipe,m_fSkyAlphaVal );
	}
	float   t_fEtime = sg_timer::Instance()->get_lastelatime();
	static float lastEtime = 0;
	float detaTime = t_fEtime - lastEtime;
	lastEtime = t_fEtime;

	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	
//	
# if 1
	if (strlen(m_szSky1)!=0)
	{		
		os_bbox bbox;

		m_SkymeshIns1.set_aniMeshPos(_cen);

		if( _reflect )
			t_fRotAngle = m_fSkyRotAgl1;
		else
			t_fRotAngle = m_fSkyRotAgl1;
		
		m_SkymeshIns1.set_aniMeshRot(t_fRotAngle);
		m_SkymeshIns1.render_aniMeshIns(t_fEtime,false);
		m_SkymeshIns1.render_aniMeshIns(t_fEtime,true);	

	}
# endif 

	if (strlen(m_szSky2)!=0)
	{
		if( _reflect )
			t_fRotAngle = m_fSkyRotAgl2;
		else
			t_fRotAngle = m_fSkyRotAgl2;

		m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,false);
		m_SkymeshIns2.set_aniMeshPos(_cen);
		m_SkymeshIns2.set_aniMeshRot( t_fRotAngle );
		m_SkymeshIns2.render_aniMeshIns(t_fEtime,false);
		m_SkymeshIns2.render_aniMeshIns(t_fEtime,true);
	}
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,true);
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE);
	if (strlen(m_szBolt)!=0)
	{
		m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,false);

		float rate =  fabs((float)rand()/(float)RAND_MAX);

		if (rate < m_szBoltRate)
		{
			osVec3D newPos = _cen;
			newPos.y += m_BoltHeight;
			
			float factor =  (float)rand()/(float)RAND_MAX;
			float AngleFactor = (3.1415926f*2.0f)*g_fBoltFanAngle*factor;

			m_BoltmeshIns.set_aniMeshRot(m_fBoltAngle+AngleFactor);
			newPos.x += 200*cos(m_fBoltAngle+AngleFactor);
			newPos.z += -200*sin(m_fBoltAngle+AngleFactor);
			m_BoltmeshIns.set_aniMeshPos(newPos);
			m_BoltmeshIns.render_aniMeshIns(t_fEtime,false);
			m_BoltmeshIns.render_aniMeshIns(t_fEtime,true);
		}
	}
	

	
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_LIGHTING,TRUE);
	//@}


	unguard;
}

//! ��ȾԶ��.
void osc_osaSkyTerr::render_bkTerr( osVec3D& _cen,float _terrScale )
{
	guard;
	if (m_szTerrName[0]==0/*&&m_szTerr1Name[0]==0*/)
	{
		return;
	}

	static osVec3D   t_vec3Scale( 1.0f,1.0f,1.0f );

	t_vec3Scale.x = _terrScale;
	t_vec3Scale.z = _terrScale;

	float   t_fEtime = sg_timer::Instance()->get_lastelatime();

	osassert( m_szTerrName[0] != NULL );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,false);
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_NONE);
	m_sTerrMeshIns.set_aniMeshPos(_cen);
	//! River @ 2009-8-4:Զ������ת
	m_sTerrMeshIns.set_aniMeshRot( 0.0f );
	m_sTerrMeshIns.render_aniMeshIns(t_fEtime,false);
	m_sTerrMeshIns.render_aniMeshIns(t_fEtime,true);

	unguard;
}


//!  �õ���ǰ����������.
const char* osc_osaSkyTerr::get_skyOsaName( void )
{
	if( m_iCurActiveSkyIdx == -1 )
		return NULL;
	if( m_iCurActiveSkyIdx == 0 )
		return this->m_szSky0Name;
	if( m_iCurActiveSkyIdx == 1 )
		return this->m_szSky1Name;

	osassert( false );
	return NULL;
}

const char*	osc_osaSkyTerr::get_skyOsaName1()
{
	return m_szSky0Name;
}
const char*	osc_osaSkyTerr::get_skyOsaName2()
{
	return m_szSky1Name;
}
const char*	osc_osaSkyTerr::get_Bolt()
{
	return m_szBolt;
}
float	osc_osaSkyTerr::get_BoltRate()
{
	return m_szBoltRate;
}
//!  �õ���ǰ�����Զɽ������.
const char* osc_osaSkyTerr::get_terrOsaName( void )
{
	if( this->m_iCurActiveTerrIdx == -1 )
		return NULL;
	//if( this->m_iCurActiveTerrIdx == 0 )
	return this->m_szTerrName;
	/*if( this->m_iCurActiveTerrIdx == 1 )
		return this->m_szTerr1Name;*/

	osassert( false );
	return NULL;
}


/** \brief
*  ��Ⱦ�����Զ����
*
*  \param _terrScale Զ��Ҫ�����Ӿص�Զ���������š�
*  
*/
void osc_osaSkyTerr::render_skyTerr( osVec3D& _cen,float _fardis,bool _reflect )
{
	guard;

	// �ۺϵĴ���
	frame_moveSkyTerr( _reflect );

	// ������Ⱦ״̬
	set_skyRenderState();


	float           t_fFogStart = FAR_SKYMAXSIZE-g_dwSkyFogDistance;
	float           t_fFogEnd   = FAR_SKYMAXSIZE+20;


	

	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGSTART,*(DWORD*)&t_fFogStart );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGEND,*(DWORD*)&t_fFogEnd );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGCOLOR,g_dwFogColor );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,false);

	m_ptrMeshMgr->m_ptrMPipe->set_sampleState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_ptrMeshMgr->m_ptrMPipe->set_sampleState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );


	//! River @ 2009-8-4:��Ⱦ��Ӱʱ������ת�����Ա���CULLMODE��ת��ֱ��none
	if( _reflect )
		m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_NONE );



	// 
	// Ϊʲô�˴����룬��ʾ������յĵ�Ӱ��
	//m_ptrMeshMgr->m_ptrMPipe->set_viewMatrix( g_matView );

	// ��Ⱦ���.
	if( m_iCurActiveSkyIdx >= 0 )
		render_bkSky( _cen,_reflect );

	// Զ��ʹ�����������,�����ɫ���Ե���????
	float t_fTemp = (FAR_TERRMAXSIZE - 5.0f)/float(FAR_TERRMAXSIZE);
	t_fFogStart = FAR_TERRMAXSIZE-g_dwTerrFogDistance;
	t_fFogEnd   = FAR_TERRMAXSIZE+20;
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGSTART,*(DWORD*)&t_fFogStart );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGEND,*(DWORD*)&t_fFogEnd );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,false);
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_LIGHTING,FALSE);
	m_ptrMeshMgr->m_ptrMPipe->set_sampleState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_ptrMeshMgr->m_ptrMPipe->set_sampleState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );

	//if( m_iCurActiveTerrIdx>= 0 )
	render_bkTerr( _cen,t_fTemp );
	m_ptrMeshMgr->m_ptrMPipe->set_renderState( D3DRS_LIGHTING,TRUE);

	return;

	unguard;
}


