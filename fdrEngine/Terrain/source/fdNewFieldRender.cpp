//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldRendercpp
 *
 *  His:      River created @ 2006-1-11
 *
 *  Des:      newFieldStruct�ṹ�ڵ���Ⱦ����.
 *   
 * ��ֹ֪�����ж�,�������ܾ��� 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdNewFieldStruct.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../effect/include/fdEleBolt.h"
# include "../../effect/include/osHDRLight.h"
# include "../../interface/miskFunc.h"

void osc_newTileGround::PushFogObject( int _idx )
{
	os_meshRender  t_mr;
	os_newTGObject*   t_ptrObj;
	t_ptrObj = &m_vecTGObject[_idx];
	if( (t_ptrObj->m_dwType !=0) )
		return;

	t_mr.m_fAngle = t_ptrObj->m_fAngle;
	t_mr.m_iMeshId = m_vecObjectDevIdx[_idx];		

	t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
	t_mr.m_dwTerrColor = t_ptrObj->m_dwObjTerrColor;
	t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

	// 
	// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
	if( t_mr.m_iMeshId == -1 )
	{
		// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
		if( !create_meshImmediate( _idx ) )
			return;
		t_mr.m_iMeshId = m_vecObjectDevIdx[_idx];		
	}
	m_ptrTgMgr->push_opacityObj( t_mr );
}
BOOL osc_newTileGround::render_AcceptFogObject()
{
	guard;
	
	
	int           t_idx;
	int           *t_ptrInViewObjIdx;

	if( m_ivoMgr.m_iNDObjNum > 0 )
	{
		t_ptrInViewObjIdx = &m_ivoMgr.m_vecNormalDis[0];
		osassert( m_ivoMgr.m_iNDObjNum <= (int)m_ivoMgr.m_vecNormalDis.size() );
	}
	for( int t_i=0;t_i<m_ivoMgr.m_iNDObjNum;t_i++ )
	{
		t_idx = t_ptrInViewObjIdx[t_i];

	
		PushFogObject(t_idx);

	}
	t_ptrInViewObjIdx = &m_ivoMgr.m_vecFadeInObj[0];
	
	for( int t_i=0;t_i<(int)m_ivoMgr.m_vecFadeInObj.size();t_i++ )
	{

		t_idx = t_ptrInViewObjIdx[t_i];
		if (t_idx!=-1)
		{
			PushFogObject(t_idx);
		}
	}


	return TRUE;
	unguard;
}

/** \brief
*  ����Ⱦ�����ڵ�������Ʒ
*/
BOOL osc_newTileGround::render_bridgeObject( osc_middlePipe* _mpipe )
{
	guard;

	os_meshRender  t_mr;
 	os_newTGObject*   t_ptrObj;
	int           t_idx;
	int           *t_ptrInViewObjIdx;

	if( g_bObjWireFrame )
		_mpipe->set_deviceFillMode( 2 );
	else
		_mpipe->set_deviceFillMode( 3 );

	//
	// �����Ƿ�ʹ��alpha�������м�ܵ�����Ⱦ״̬��
	_mpipe->set_renderState( D3DRS_ALPHABLENDENABLE,false );
	//! ��͸�����֣�ʹ��ͳһ��ȫ�ֹ�
// 	osColor   t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );
	

	if( m_ivoMgr.m_iNDObjNum > 0 )
	{
		t_ptrInViewObjIdx = &m_ivoMgr.m_vecNormalDis[0];
		osassert( m_ivoMgr.m_iNDObjNum <= (int)m_ivoMgr.m_vecNormalDis.size() );
	}
	for( int t_i=0;t_i<m_ivoMgr.m_iNDObjNum;t_i++ )
	{
		t_idx = t_ptrInViewObjIdx[t_i];

		if( !is_bridgeObj( t_idx ) )
			continue;

		t_ptrObj = &m_vecTGObject[t_idx];

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		

		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		t_mr.m_dwTerrColor = t_ptrObj->m_dwObjTerrColor;
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// 
		// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// ��̬��Ʒ�����ֹ��գ������̬���������ʹ�ö�̬����Ϊ��Ʒ�Ļ�����
		if( m_vecTGObjDL[t_idx].is_usedDl() )
			t_mr.m_sDLight = m_vecTGObjDL[t_idx];
		else
			t_mr.m_sDLight.reset_dl();
		

		m_ptrTgMgr->push_opacityObj( t_mr );
	}

	return TRUE;

	unguard;
}

/** \brief
*  ��Ⱦ������������Ʒ��͸�����֡�
*/
BOOL osc_newTileGround::render_bridgeObjAlphaPart( osc_middlePipe* _mpipe )
{
	guard;

	os_meshRender  t_mr;
 	os_newTGObject*   t_ptrObj;
	int           t_idx;
	int           *t_ptrInViewObjIdx;

	if( m_ivoMgr.m_iNDObjNum > 0 )
	{
		t_ptrInViewObjIdx = &m_ivoMgr.m_vecNormalDis[0];
		osassert( m_ivoMgr.m_iNDObjNum <= (int)m_ivoMgr.m_vecNormalDis.size() );
	}
	for( int t_i=0;t_i<m_ivoMgr.m_iNDObjNum;t_i++ )
	{
		t_idx = t_ptrInViewObjIdx[t_i];

		if( !is_bridgeObj( t_idx ) )
			continue;

		t_ptrObj = &m_vecTGObject[t_idx];

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		

		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		t_mr.m_dwTerrColor = t_ptrObj->m_dwObjTerrColor;
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// 
		// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// ��̬��Ʒ�����ֹ��գ������̬���������ʹ�ö�̬����Ϊ��Ʒ�Ļ�����
		if( m_vecTGObjDL[t_idx].is_usedDl() )
			t_mr.m_sDLight = m_vecTGObjDL[t_idx];
		else
			t_mr.m_sDLight.reset_dl();
		

		m_ptrTgMgr->push_alphaObj( t_mr );
	}

	return TRUE;

	unguard;
}




/** \brief
*  ��ͼ������Ʒ��Ⱦ�ĺ���. 
*
*  \param _alpha ���Ƶ�ǰ��Ⱦ������Ʒ��͸���Ĳ��ֻ��ǲ�͸���Ĳ��֡�
*                ͸���Ĳ�����Ҫ�ŵ������Ⱦ��
*/
BOOL osc_newTileGround::render_TGsObject( osc_middlePipe* _mpipe,
								   BOOL _alpha/* = false*/ )
{
	guard;

	os_meshRender  t_mr;
 	os_newTGObject*   t_ptrObj;
	int           t_idx;
	int           *t_ptrInViewObjIdx;
	osc_skinMesh*  t_ptrSm;


	if( g_bObjWireFrame )
		_mpipe->set_deviceFillMode( 2 );
	else
		_mpipe->set_deviceFillMode( 3 );

	//
	// �����Ƿ�ʹ��alpha�������м�ܵ�����Ⱦ״̬��
	_mpipe->set_renderState( D3DRS_ALPHABLENDENABLE,_alpha );
	//! ��͸�����֣�ʹ��ͳһ��ȫ�ֹ�
// 	if( !_alpha )
// 	{
// 		osColor   t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
// 		_mpipe->set_renderState( D3DRS_AMBIENT,t_color );
// 	}
	
	if( m_ivoMgr.m_iNDObjNum > 0 )
	{
		t_ptrInViewObjIdx = &m_ivoMgr.m_vecNormalDis[0];
		osassert( m_ivoMgr.m_iNDObjNum <= (int)m_ivoMgr.m_vecNormalDis.size() );
	}
	for( int t_i=0;t_i<m_ivoMgr.m_iNDObjNum;t_i++ )
	{
		t_idx = t_ptrInViewObjIdx[t_i];
		t_ptrObj = &m_vecTGObject[t_idx];
		//
		// �������osaAniMeshIns,����ʹ��osa��ˮ���ļ�.
		// ˮ����͸�������壬����ᱻ�������
		// River mod @ 2006-8-26:ֻ�ڷ�alpha��Ⱦʱ������aniMesh�Ŀ�����
		if( !_alpha )
		{
			if( ((t_ptrObj->m_dwType == 4)||(t_ptrObj->m_dwType == 2)) && (!_alpha) )
			{
				g_ptrMeshMgr->add_sceAniMeshIns( m_vecObjectDevIdx[t_idx] );
				continue;
			}
			if( (t_ptrObj->m_dwType == 8) )
			{
				g_ptrMeshMgr->add_sceAniMeshIns( m_vecObjectDevIdx[t_idx] );
				continue;
			}

		}

		//
		// ����billBoard Object,��billBoard������Ⱦ��
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// ��������˶������壬��Ҫʹ����Ⱦ��������ĺ����Զ������������Ⱦ��
		// �������Ʒ��������������
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			if( !_alpha )
			{
				t_ptrSm = (osc_skinMesh*)(m_vecObjectDevIdx[t_idx]);
				t_ptrSm->set_fadeState( false );
				m_ptrTgMgr->m_skinMeshMgr->
					add_sceneInviewSm( t_ptrSm ) ;
			}

			continue;
		}

	

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		

		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		t_mr.m_dwTerrColor = t_ptrObj->m_dwObjTerrColor;
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// River added @ 2006-8-25:
		// ����е���ɫ���֣���ʹ�û�����͵ر�ĵ���ɫ��ϡ�
		if( t_ptrObj->m_dwObjTerrColor != 0xffffffff )
		{
			osColor   t_colorSrc( t_ptrObj->m_dwObjTerrColor );
			osColor   t_colorDst( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
			osColor   t_colorFinal;
			osColorModulate( &t_colorFinal,&t_colorSrc,&t_colorDst );
			t_ptrObj->m_dwObjTerrColor = t_colorFinal;
		}

		// 
		// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// ��̬��Ʒ�����ֹ��գ������̬���������ʹ�ö�̬����Ϊ��Ʒ�Ļ�����
		if( m_vecTGObjDL[t_idx].is_usedDl() )
			t_mr.m_sDLight = m_vecTGObjDL[t_idx];
		else
			t_mr.m_sDLight.reset_dl();
		
										 
		//								 
		// �������Ⱦalpha���壬�򱣴浽����������һ������
		if( _alpha )
		{
			if( g_ptrMeshMgr->is_alphaMesh( t_mr.m_iMeshId ) )
				m_ptrTgMgr->push_alphaObj( t_mr );
		}
		else
		{
			if( g_ptrMeshMgr->is_opacityMesh( t_mr.m_iMeshId ) )
				m_ptrTgMgr->push_opacityObj( t_mr );
		}
	}

	return TRUE;

	unguard;
}

//! ��Ⱦ�����Object.
BOOL osc_newTileGround::render_TGsFadeInObject( osc_middlePipe* _mpipe,
										 BOOL _alpha/* = false*/ )
{ 
	guard;

	// �õ��������Ʒ���С�
	os_meshRender  t_mr;
	os_newTGObject*   t_ptrObj;
	int           t_idx;


	//
	// ��ȾFadeIn��Ʒʱ��ȫ��ʹ��ͳһ�Ļ�����
// 	osColor  t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );;
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );


# ifdef _DEBUG
	if( g_bObjWireFrame )
		_mpipe->set_deviceFillMode( 2 );
	else
		_mpipe->set_deviceFillMode( 3 );
# endif 


	// 
	// ʹ��IvoMgr�ڲ��ĵ����������Ⱦ��Ʒ��
	int   t_iObjNum = (int)m_ivoMgr.m_vecFadeInObj.size();
	for( int t_i=0;t_i<t_iObjNum;t_i++ )
	{
		if( m_ivoMgr.m_vecFadeInObj[t_i] == -1 )
			continue;

		t_idx = m_ivoMgr.m_vecFadeInObj[t_i];
		t_ptrObj = &m_vecTGObject[t_idx];

		//
		// ����billBoard Object,��billBoard������Ⱦ��
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// ��������˶������壬��Ҫʹ����Ⱦ��������ĺ����Զ������������Ⱦ��
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			// 
			//@ River @ 2009-5-12:ȥ����δ��룬ֱ����ʾskinMesh.
			/*
			// ������skinMesh��alphaֵ��
			((osc_skinMesh*)m_vecObjectDevIdx[t_idx])->set_fadeState( 
				true,m_ivoMgr.m_vecFadeInMgr[t_i].get_alpha() );

			m_ptrTgMgr->m_skinMeshMgr->
				add_sceneInviewSm( (osc_skinMesh*)m_vecObjectDevIdx[t_idx] ) ;
			*/
			continue;
		}

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		


		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos; 

		// river added @ 2009-4-27:��Ʒ��������� 
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// 
		// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		m_ptrTgMgr->set_material( m_ivoMgr.m_vecFadeInMgr[t_i].get_alpha() );

		// 
		// �������Ⱦalpha���壬��Ϊ����ʱÿһ��material��һ��������ֱ����Ⱦ��
		if( _alpha )
		{
			if( g_ptrMeshMgr->is_alphaMesh( t_mr.m_iMeshId ) )
			{
				_mpipe->render_mesh( &t_mr,_alpha );
			}

			continue;
		}

		if( !_mpipe->render_mesh( &t_mr,_alpha ) )
			return false;
	}

	return TRUE; 

	unguard;
}

//! ��Ⱦ������Object.
BOOL osc_newTileGround::render_TGsFadeOutObject( osc_middlePipe* _mpipe,
										  BOOL _alpha/* = false*/ )
{ 
	guard;

	os_meshRender  t_mr;
 	os_newTGObject*   t_ptrObj;
	int           t_idx;


	// Fade out��Ʒʹ��ͳһ�Ļ�����
// 	osColor   t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );

# ifdef _DEBUG
	if( g_bObjWireFrame )
		_mpipe->set_deviceFillMode( 2 );
	else
		_mpipe->set_deviceFillMode( 3 );
# endif 


	// 
	// ʹ��IvoMgr�ڲ��ĵ�����Ʒ��������Ⱦ��Ʒ��
	int   t_iObjNum = (int)m_ivoMgr.m_vecFadeOutObj.size();
	for( int t_i=0;t_i<t_iObjNum;t_i++ )
	{
		if( m_ivoMgr.m_vecFadeOutObj[t_i] == -1 )
			continue;

		t_idx = m_ivoMgr.m_vecFadeOutObj[t_i];
		t_ptrObj = &m_vecTGObject[t_idx];

		//
		// ����billBoard Object,��billBoard������Ⱦ��
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// ��������˶������壬��Ҫʹ����Ⱦ��������ĺ����Զ������������Ⱦ��
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			/* River @ 2009-5-12: 
			// ������skinMesh��alphaֵ��
			((osc_skinMesh*)m_vecObjectDevIdx[t_idx])->set_fadeState( 
				true,m_ivoMgr.m_vecFadeOutMgr[t_i].get_alpha() );

			m_ptrTgMgr->m_skinMeshMgr->
				add_sceneInviewSm( (osc_skinMesh*)m_vecObjectDevIdx[t_idx] ) ;
			*/
			continue;
		}

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		

		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// 
		// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// ATTENTION TO OPP:ͳһ��Ⱦ��ͳһ���������Ʒ???
		//t_mr.m_dwAmbientLight = osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );;

		m_ptrTgMgr->set_material( 
			m_ivoMgr.m_vecFadeOutMgr[t_i].get_alpha() );

		// 
		// �������Ⱦalpha���壬��Ϊ����ʱÿһ��material��һ��������ֱ����Ⱦ��
		if( _alpha )
		{
			if( g_ptrMeshMgr->is_alphaMesh( t_mr.m_iMeshId ) )
				_mpipe->render_mesh( &t_mr,_alpha );

			continue;
		}

		if( !_mpipe->render_mesh( &t_mr,_alpha ) )
			return false;
	}

	return TRUE; 
	unguard;
}


/** \brief
*  ��Ⱦ��͸����object.
*
*  ATTENTION TO OPP:���������������Ժϲ���һ��.
*  û��ǰ��������alphaΪtrue���������.
*/
BOOL osc_newTileGround::render_halfAlphaObj( 
									  osc_middlePipe* _mpipe,BOOL _alpha )
{ 
	guard;

	// �õ��������Ʒ���С�
	os_meshRender  t_mr;
  	os_newTGObject*   t_ptrObj;
	int           t_idx;

	// ��͸������Ʒ��ʹ��ȫ��ͳһ�Ļ�����
// 	osColor   t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );

	// 
	// ʹ��IvoMgr�ڲ��ĵ����������Ⱦ��Ʒ��
	int   t_iObjNum = MAX_HALFALPHAOBJ;
	for( int t_i=0;t_i<t_iObjNum;t_i++ )
	{
		if( m_ivoMgr.m_vecAccuHAObj[t_i] == -1 )
			continue;

		t_idx = m_ivoMgr.m_vecAccuHAObj[t_i];
		osassertex( (t_idx >=0),"��Ʒ����������ڵ���0...\n" );
		t_ptrObj = &m_vecTGObject[t_idx];

		//
		// ����billBoard Object,��billBoard������Ⱦ��
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// ��������˶������壬��Ҫʹ����Ⱦ��������ĺ����Զ������������Ⱦ��
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			// ������skinMesh��alphaֵ��
			((osc_skinMesh*)m_vecObjectDevIdx[t_idx])->set_fadeState( 
				true,m_ivoMgr.m_vecHalfFadeMgr[t_i].get_alpha() );

			m_ptrTgMgr->m_skinMeshMgr->
				add_sceneInviewSm( (osc_skinMesh*)m_vecObjectDevIdx[t_idx] ) ;
			continue;
		}

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// ����豸���������û�е�ǰmesh, ��̬�Ĵ�����ǰ��mesh.
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		if( t_mr.m_iMeshId == -1 )
		{
			// �������ʧ�ܣ�����Ⱦ��ǰ����Ʒ��
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}


		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;

		// 
		//! River added @ 2009-5-2:�԰�͸������Ʒ����һ���Ⱦ�ȷ�Ļ���Zֵ��
		//  ���ڰ�͸������Ҷ����Ʒ����Ҫ���Ӹ��ӵĴ���
		_mpipe->set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
		_mpipe->set_renderState( D3DRS_ALPHAREF,0xa0 );
		_mpipe->set_renderState( D3DRS_ZWRITEENABLE,TRUE );
		_mpipe->set_renderState( D3DRS_COLORWRITEENABLE,FALSE );
		m_ptrTgMgr->set_material( 1.0f );

		if( !_mpipe->render_mesh( &t_mr,_alpha ) )
			return false;


		// 
		//! �ڶ�����Ⱦ����ֱ��������ȷ�����ء�
		_mpipe->set_renderState( D3DRS_ALPHAREF,GLOBAL_MINALPHAREF );
		// �������ʹ����ɫ
		_mpipe->set_renderState( D3DRS_COLORWRITEENABLE, 
			D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA|
			D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED    );
		m_ptrTgMgr->set_material( m_ivoMgr.m_vecHalfFadeMgr[t_i].get_alpha() );
		
		if( !_mpipe->render_mesh( &t_mr,_alpha ) )
			return false;

		// ��ǰ֡��Ⱦ����ס����İ�͸����Ʒ.
		m_ptrTgMgr->m_bHasAlphaShelter = true;
	}

	return TRUE; 
	
	unguard;
}

