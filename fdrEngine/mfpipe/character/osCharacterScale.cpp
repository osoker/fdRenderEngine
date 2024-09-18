///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterScale.cpp
 * 
 *  His:      River created @ 2005-3-5
 *
 *  Desc:     引擎中人物的缩放相关的函数。
 * 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/frontPipe.h"





//! 初始化的时候,设置一些smBBox,然后每一帧根据人物的旋转,来得到新的bboxMin和max.
void osc_smOffsetAdjBBox::set_smOffsetAdjBBox( osVec3D& _min,osVec3D& _max,osVec3D& _minOut,osVec3D& _maxOut )
{
	float   t_xSize,t_zSize;
	t_xSize = ( _max.x - _min.x )/2.0f;
	t_zSize = (_max.z - _min.z )/2.0f;
	m_vec2Offset.x = t_xSize + _min.x;
	m_vec2Offset.y = t_zSize + _min.z;

	_minOut.x = _min.x - m_vec2Offset.x;
	_maxOut.x = _max.x - m_vec2Offset.x;
	_minOut.z = _min.z - m_vec2Offset.y;
	_maxOut.z = _max.z - m_vec2Offset.y;

	_minOut.y = _min.y;
	_maxOut.y = _max.y;

}


// WORD START:此处来确人物的包围盒信息.......
void osc_smOffsetAdjBBox::get_smOffsetAdjBBox( osVec3D& _vecOut,float _rotAngl )
{
	osMatrix   t_sRotMat;
	osVec4D    t_vec4Out;

	osMatrixRotationY( &t_sRotMat,_rotAngl );
	osVec2Transform( &t_vec4Out,&m_vec2Offset,&t_sRotMat );
	_vecOut.y = 0.0f;
	_vecOut.x = m_vec2Offset.x;
	_vecOut.z = m_vec2Offset.y;

	return;
}





# if __ALLCHR_SIZEADJ__

osVec3D osc_skinMesh::m_vec3ScaleAll = osVec3D( 1.0f,1.0f,1.0f );

# endif 


# if __ALLCHR_SIZEADJ__
//! 使用全局的人物缩放信息来得处理人物.用于创建人物时,使用全局的scale.
void osc_skinMesh::createScale_withAllChrScale( void )
{
	guard;

	m_vScale.x *= osc_skinMesh::m_vec3ScaleAll.x;
	m_vScale.y *= osc_skinMesh::m_vec3ScaleAll.y;
	m_vScale.z *= osc_skinMesh::m_vec3ScaleAll.z;

	// 处理bbox的scale.
	m_sBBox.scale_bb( m_vScale );

	unguard;
}
# endif 



// Rriver @ 2010-1-15: TEST CODE:内部使用接口。
bool osc_skinMesh::scale_skinMesh( float _x,float _y,float _z )
{
	guard;

	osassert( (_x > 0.0f)&&(_y>=0.0f)&&(_z>=0.0f) );

	//  先把bbox设置到基准大小.
	osVec3D t_vec3BoxScale;

	t_vec3BoxScale.x = 1.0f/m_vScale.x;	
	t_vec3BoxScale.y = 1.0f/m_vScale.y;
	t_vec3BoxScale.z = 1.0f/m_vScale.z;
	
	m_sBBox.scale_bb( t_vec3BoxScale );

	// 处理人物的scale数值。
	m_vScale = osVec3D( _x,_y,_z );

# if __ALLCHR_SIZEADJ__

	m_vScale.x *= osc_skinMesh::m_vec3ScaleAll.x;
	m_vScale.y *= osc_skinMesh::m_vec3ScaleAll.y;
	m_vScale.z *= osc_skinMesh::m_vec3ScaleAll.z;

# endif 

	// 处理bbox的scale.
	m_sBBox.scale_bb( m_vScale );

	//! 缩放人物武器附加的特效数据
	float t_fScale = _x/m_fBaseSkinScale;
	if( m_ptrSGBpmIns )
		m_ptrSGBpmIns->scale_weaponEffect( t_fScale );
	if( m_ptrSGBpmIns8 )
		m_ptrSGBpmIns8->scale_weaponEffect( t_fScale );


	return true;

	unguard;
}



/** \brief
*  对人物进行缩放。
*
*  \param osVec3D _svec 表示了在三维中的三个方向人物要缩放的比例。
*/
bool osc_skinMesh::scale_skinMesh( osVec3D& _svec ) 
{
	guard;

	osassert( (_svec.x > 0.0f)&&(_svec.y>=0.0f)&&(_svec.z>=0.0f) );

	//  先把bbox设置到基准大小.
	osVec3D t_vec3BoxScale;

	t_vec3BoxScale.x = 1.0f/m_vScale.x;	
	t_vec3BoxScale.y = 1.0f/m_vScale.y;
	t_vec3BoxScale.z = 1.0f/m_vScale.z;
	
	m_sBBox.scale_bb( t_vec3BoxScale );

	// 处理人物的scale数值。
	m_vScale = _svec;

# if __ALLCHR_SIZEADJ__

	m_vScale.x *= osc_skinMesh::m_vec3ScaleAll.x;
	m_vScale.y *= osc_skinMesh::m_vec3ScaleAll.y;
	m_vScale.z *= osc_skinMesh::m_vec3ScaleAll.z;

# endif 

	// 处理bbox的scale.
	m_sBBox.scale_bb( m_vScale );

	//! 缩放人物武器附加的特效数据
	float t_fScale = _svec.x/m_fBaseSkinScale;
	if( m_ptrSGBpmIns )
		m_ptrSGBpmIns->scale_weaponEffect( t_fScale );
	if( m_ptrSGBpmIns8 )
		m_ptrSGBpmIns8->scale_weaponEffect( t_fScale );


	return true;

	unguard;
}


# if __ALLCHR_SIZEADJ__
//! 重设当前smMgr中每一个可用sm的scale值。
void osc_skinMeshMgr::reset_smMgrScale( void )
{
	guard;


	osVec3D t_vecBoxScale;

	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;
	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode(&t_ptrNode ) )
	{
		osassert( t_ptrNode.p->m_bInUse );

		//  先把bbox设置到基准大小.
		t_vecBoxScale.x = 1.0f/t_ptrNode.p->m_vScale.x;
		t_vecBoxScale.y = 1.0f/t_ptrNode.p->m_vScale.y;
		t_vecBoxScale.z = 1.0f/t_ptrNode.p->m_vScale.z;
		t_ptrNode.p->m_sBBox.scale_bb( t_vecBoxScale );

		// Character的scale.
		t_ptrNode.p->m_vScale.x *= osc_skinMesh::m_vec3ScaleAll.x;
		t_ptrNode.p->m_vScale.y *= osc_skinMesh::m_vec3ScaleAll.y;
		t_ptrNode.p->m_vScale.z *= osc_skinMesh::m_vec3ScaleAll.z;

		//
		// 重设bbounding Box.
		t_ptrNode.p->m_sBBox.scale_bb( t_ptrNode.p->m_vScale );

	}

	unguard;
}
# endif 





