//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldRendercpp
 *
 *  His:      River created @ 2006-1-11
 *
 *  Des:      newFieldStruct结构内的渲染函数.
 *   
 * “知止而后有定,定而后能静” 
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
	// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
	if( t_mr.m_iMeshId == -1 )
	{
		// 如果创建失败，不渲染当前的物品。
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
*  仅渲染场景内的桥类物品
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
	// 根据是否使用alpha，设置中间管道的渲染状态。
	_mpipe->set_renderState( D3DRS_ALPHABLENDENABLE,false );
	//! 非透明部分，使用统一的全局光
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
		// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// 如果创建失败，不渲染当前的物品。
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// 静态物品有两种光照：如果动态光更亮，则使用动态光做为物品的环境光
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
*  渲染场景内桥类物品的透明部分。
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
		// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// 如果创建失败，不渲染当前的物品。
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// 静态物品有两种光照：如果动态光更亮，则使用动态光做为物品的环境光
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
*  地图块上物品渲染的函数. 
*
*  \param _alpha 控制当前渲染的是物品中透明的部分还是不透明的部分。
*                透明的部分需要放到最后渲染。
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
	// 根据是否使用alpha，设置中间管道的渲染状态。
	_mpipe->set_renderState( D3DRS_ALPHABLENDENABLE,_alpha );
	//! 非透明部分，使用统一的全局光
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
		// 如果碰到osaAniMeshIns,或是使用osa的水面文件.
		// 水面是透明的物体，否则会被添加两次
		// River mod @ 2006-8-26:只在非alpha渲染时，加入aniMesh的可视性
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
		// 碰到billBoard Object,对billBoard进行渲染。
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// 如果碰到了动画物体，需要使用渲染动画物体的函数对动画物体进行渲染。
		// 加入此物品到动画管理器。
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
		// 如果有点着色出现，则使用环境光和地表的点着色混合。
		if( t_ptrObj->m_dwObjTerrColor != 0xffffffff )
		{
			osColor   t_colorSrc( t_ptrObj->m_dwObjTerrColor );
			osColor   t_colorDst( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
			osColor   t_colorFinal;
			osColorModulate( &t_colorFinal,&t_colorSrc,&t_colorDst );
			t_ptrObj->m_dwObjTerrColor = t_colorFinal;
		}

		// 
		// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// 如果创建失败，不渲染当前的物品。
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// 静态物品有两种光照：如果动态光更亮，则使用动态光做为物品的环境光
		if( m_vecTGObjDL[t_idx].is_usedDl() )
			t_mr.m_sDLight = m_vecTGObjDL[t_idx];
		else
			t_mr.m_sDLight.reset_dl();
		
										 
		//								 
		// 如果是渲染alpha物体，则保存到数组留待进一步处理。
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

//! 渲染淡入的Object.
BOOL osc_newTileGround::render_TGsFadeInObject( osc_middlePipe* _mpipe,
										 BOOL _alpha/* = false*/ )
{ 
	guard;

	// 得到淡入的物品队列。
	os_meshRender  t_mr;
	os_newTGObject*   t_ptrObj;
	int           t_idx;


	//
	// 渲染FadeIn物品时，全局使用统一的环境光
// 	osColor  t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );;
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );


# ifdef _DEBUG
	if( g_bObjWireFrame )
		_mpipe->set_deviceFillMode( 2 );
	else
		_mpipe->set_deviceFillMode( 3 );
# endif 


	// 
	// 使用IvoMgr内部的淡入队列来渲染物品。
	int   t_iObjNum = (int)m_ivoMgr.m_vecFadeInObj.size();
	for( int t_i=0;t_i<t_iObjNum;t_i++ )
	{
		if( m_ivoMgr.m_vecFadeInObj[t_i] == -1 )
			continue;

		t_idx = m_ivoMgr.m_vecFadeInObj[t_i];
		t_ptrObj = &m_vecTGObject[t_idx];

		//
		// 碰到billBoard Object,对billBoard进行渲染。
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// 如果碰到了动画物体，需要使用渲染动画物体的函数对动画物体进行渲染。
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			// 
			//@ River @ 2009-5-12:去掉这段代码，直接显示skinMesh.
			/*
			// 先设置skinMesh的alpha值。
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

		// river added @ 2009-4-27:物品的缩放相关 
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// 
		// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// 如果创建失败，不渲染当前的物品。
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		m_ptrTgMgr->set_material( m_ivoMgr.m_vecFadeInMgr[t_i].get_alpha() );

		// 
		// 如果是渲染alpha物体，因为淡出时每一个material不一样，所以直接渲染。
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

//! 渲染淡出的Object.
BOOL osc_newTileGround::render_TGsFadeOutObject( osc_middlePipe* _mpipe,
										  BOOL _alpha/* = false*/ )
{ 
	guard;

	os_meshRender  t_mr;
 	os_newTGObject*   t_ptrObj;
	int           t_idx;


	// Fade out物品使用统一的环境光
// 	osColor   t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );

# ifdef _DEBUG
	if( g_bObjWireFrame )
		_mpipe->set_deviceFillMode( 2 );
	else
		_mpipe->set_deviceFillMode( 3 );
# endif 


	// 
	// 使用IvoMgr内部的淡出物品队列来渲染物品。
	int   t_iObjNum = (int)m_ivoMgr.m_vecFadeOutObj.size();
	for( int t_i=0;t_i<t_iObjNum;t_i++ )
	{
		if( m_ivoMgr.m_vecFadeOutObj[t_i] == -1 )
			continue;

		t_idx = m_ivoMgr.m_vecFadeOutObj[t_i];
		t_ptrObj = &m_vecTGObject[t_idx];

		//
		// 碰到billBoard Object,对billBoard进行渲染。
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// 如果碰到了动画物体，需要使用渲染动画物体的函数对动画物体进行渲染。
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			/* River @ 2009-5-12: 
			// 先设置skinMesh的alpha值。
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
		// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
		if( t_mr.m_iMeshId == -1 )
		{
			// 如果创建失败，不渲染当前的物品。
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}

		//
		// ATTENTION TO OPP:统一渲染有统一环境光的物品???
		//t_mr.m_dwAmbientLight = osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );;

		m_ptrTgMgr->set_material( 
			m_ivoMgr.m_vecFadeOutMgr[t_i].get_alpha() );

		// 
		// 如果是渲染alpha物体，因为淡出时每一个material不一样，所以直接渲染。
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
*  渲染半透明的object.
*
*  ATTENTION TO OPP:以下三个函数可以合并成一个.
*  没有前两个函数alpha为true的情况发生.
*/
BOOL osc_newTileGround::render_halfAlphaObj( 
									  osc_middlePipe* _mpipe,BOOL _alpha )
{ 
	guard;

	// 得到淡入的物品队列。
	os_meshRender  t_mr;
  	os_newTGObject*   t_ptrObj;
	int           t_idx;

	// 半透明的物品，使用全局统一的环境光
// 	osColor   t_color( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );
// 	_mpipe->set_renderState( D3DRS_AMBIENT,t_color );

	// 
	// 使用IvoMgr内部的淡入队列来渲染物品。
	int   t_iObjNum = MAX_HALFALPHAOBJ;
	for( int t_i=0;t_i<t_iObjNum;t_i++ )
	{
		if( m_ivoMgr.m_vecAccuHAObj[t_i] == -1 )
			continue;

		t_idx = m_ivoMgr.m_vecAccuHAObj[t_i];
		osassertex( (t_idx >=0),"物品索引必须大于等于0...\n" );
		t_ptrObj = &m_vecTGObject[t_idx];

		//
		// 碰到billBoard Object,对billBoard进行渲染。
		if( (t_ptrObj->m_dwType != 0) &&
			(t_ptrObj->m_dwType != 1)   )
			continue;

		//
		// 如果碰到了动画物体，需要使用渲染动画物体的函数对动画物体进行渲染。
		if( g_bUseAnimationObj&&(t_ptrObj->m_dwType == 1) )
		{
			// 先设置skinMesh的alpha值。
			((osc_skinMesh*)m_vecObjectDevIdx[t_idx])->set_fadeState( 
				true,m_ivoMgr.m_vecHalfFadeMgr[t_i].get_alpha() );

			m_ptrTgMgr->m_skinMeshMgr->
				add_sceneInviewSm( (osc_skinMesh*)m_vecObjectDevIdx[t_idx] ) ;
			continue;
		}

		t_mr.m_fAngle = t_ptrObj->m_fAngle;
		t_mr.m_vec3Scale = t_ptrObj->m_vec3ObjectScale;

		// 如果设备相关数据中没有当前mesh, 动态的创建当前的mesh.
		t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		if( t_mr.m_iMeshId == -1 )
		{
			// 如果创建失败，不渲染当前的物品。
			if( !create_meshImmediate( t_idx ) )
				continue;
			t_mr.m_iMeshId = m_vecObjectDevIdx[t_idx];		
		}


		t_mr.m_vec3Pos = t_ptrObj->m_vec3ObjPos;

		// 
		//! River added @ 2009-5-2:对半透明的物品，第一次先精确的画出Z值。
		//  对于半透明的树叶类物品，需要更加复杂的处理。
		_mpipe->set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
		_mpipe->set_renderState( D3DRS_ALPHAREF,0xa0 );
		_mpipe->set_renderState( D3DRS_ZWRITEENABLE,TRUE );
		_mpipe->set_renderState( D3DRS_COLORWRITEENABLE,FALSE );
		m_ptrTgMgr->set_material( 1.0f );

		if( !_mpipe->render_mesh( &t_mr,_alpha ) )
			return false;


		// 
		//! 第二次渲染，才直正画出正确的象素。
		_mpipe->set_renderState( D3DRS_ALPHAREF,GLOBAL_MINALPHAREF );
		// 重设可以使用颜色
		_mpipe->set_renderState( D3DRS_COLORWRITEENABLE, 
			D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA|
			D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED    );
		m_ptrTgMgr->set_material( m_ivoMgr.m_vecHalfFadeMgr[t_i].get_alpha() );
		
		if( !_mpipe->render_mesh( &t_mr,_alpha ) )
			return false;

		// 当前帧渲染过挡住相机的半透明物品.
		m_ptrTgMgr->m_bHasAlphaShelter = true;
	}

	return TRUE; 
	
	unguard;
}

