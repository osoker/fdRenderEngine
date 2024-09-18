//===================================================================
/** \file  
* Filename :   osChrRide.cpp
* Desc     :   
* His      :   Windy create @2005-8-27 19:00:07
*/
//===================================================================
//# include "../../interface/osInterface.h"
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/middlePipe.h"


//=======================================================================
osc_ChrRide::osc_ChrRide():m_vec3Center(0,0,0),m_iMatrixIdx(0)
{
}
//=======================================================================
osc_ChrRide::~osc_ChrRide()
{
}
//=======================================================================
osc_ChrRideInst::osc_ChrRideInst()
{
	m_ptrRide = NULL;
}
//=======================================================================
 osc_ChrRideInst::~osc_ChrRideInst()
{
}
void osc_ChrRideInst::update_poskey( osMatrix& _rotMat )
{
	guard;
	osVec3Transform( &m_vec3Pos,
		&m_ptrRide->m_vec3Center,&_rotMat );
	unguard;
}

void osc_bodyFrame::UpdateRideBoxPos()
{
	guard;


	osMatrix         t_matWorld,t_matTmp,t_matNewView;;
	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	static osMatrix   t_matArray[MAX_BONEPERBPM];

	//
	// 渲染当前Frame中所有的body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		if( !t_ptrPM->m_ptrRide )
			continue;


		for( int t_j=0;t_j<(int)t_ptrPM->m_dwNumPalette;t_j++ )
		{
			osMatrixMultiply( &t_matArray[t_j],
				&t_ptrPM->m_arrBoneMatrix[t_j],t_offsetMatPtr[t_j] );
		}

		// ATTENTION TO FIX: 如果此部位对应多个mesh,会有问题:
		//osassertex( m_vecPtrBpmIns[t_i]->m_ptrSGIns,"刀光部位只对应一个mesh..." );


		m_vecPtrBpmIns[t_i]->m_ptrRideIns->update_poskey( 
			t_matArray[t_ptrPM->m_ptrRide->m_iMatrixIdx] );
		m_pParentSkin->set_ridePos(m_vecPtrBpmIns[t_i]->m_ptrRideIns->m_vec3Pos);

		// 对应完了后,不需要再遍历其它的子结点,返回.
		return;
	}

	//
	// 递归渲染当前结构的子兄弟结构和子结点.
	if( m_pSibling )
		m_pSibling->UpdateRideBoxPos();
	if( m_pFirstChild )
		m_pFirstChild->UpdateRideBoxPos();

	return ;

	unguard;

}

//@{
// Windy mod @ 2005-8-30 19:26:59
//Desc: ride 
/** \brief
* \param _IsHide true 为不显示定位盒子
*/
void osc_skinMesh::HideLastPart(bool _IsHide)
{
	guard;
	//!
	osc_bpmInstance *pInst = &m_vecBpmIns[m_iNumParts-1];

	//!
	osc_bodyPartMesh* pPart = pInst->get_bpmPtr();


	//!隐藏掉
	pPart->m_bDisplay = !_IsHide;

	unguard;
}
void osc_skinMesh::get_RideBoxPos( osVec3D& _pos )
{
	guard;


	osMatrix  t_sRotMat;
	osVec3D   t_vec3Offset = m_vec3RideOffset;
	osMatrixRotationY( &t_sRotMat,m_fFinalRotAgl );
	
	osVec3Transform( &t_vec3Offset,&t_vec3Offset,&t_sRotMat );

	_pos= m_vec3RidePos + t_vec3Offset;
	
	unguard;
}


//! 得到坐骑绑定点的世界位置坐标.
bool osc_skinMesh::GetRidePos( osVec3D& _vec3 )
{
	guard;

	// 如果坐骑身上没有人物，则返回false.
	if( this->m_HumObj == NULL ) 
		return false;

	// 绑定成功，则直接返回。
	get_RideBoxPos( _vec3 );

	return true;

	unguard;
}

//! 如果当前是坐骑，得到坐骑上人物的指针，有可能返回空。
I_skinMeshObj* osc_skinMesh::get_humanObjPtr( void  )
{ 
	guard;

	int t_i = 0;
	os_ridePtr*   t_ptrRide = osc_skinMeshMgr::Instance()->m_vecRidePtrArray;
	for( t_i =0 ;t_i<MAX_RIDENUM;t_i ++ )
	{
		if( (DWORD)t_ptrRide[t_i].m_ptrRide == (DWORD)this )
			return t_ptrRide[t_i].m_ptrHuman;
	}

	return m_HumObj;

	unguard;
}

void osc_skinMesh::SetRide(I_skinMeshObj * _rideobj)
{
	//!to do添加人物骑马的动作
	//!
	guard;

	osassert(this != _rideobj);

	int t_i = 0;
	os_ridePtr*   t_ptrRide = osc_skinMeshMgr::Instance()->m_vecRidePtrArray;
	
	//! River added @ 2011-3-2:坐骑的是否坐骑指针变为真。
	osc_skinMesh* t_ptrSkin = static_cast<osc_skinMesh *>(_rideobj);
	t_ptrSkin->m_bIsRide = true;

	for( t_i =0 ;t_i<MAX_RIDENUM;t_i ++ )
	{
		if( t_ptrRide[t_i].m_ptrHuman == NULL )
		{
			t_ptrRide[t_i].m_ptrHuman = this;
			t_ptrRide[t_i].m_ptrRide = static_cast<osc_skinMesh *>(_rideobj);
			break;
		}
	}

	unguard;
}
void osc_skinMesh::DelRide() 
{
	guard;

	int t_i = 0;
	os_ridePtr*   t_ptrRide = osc_skinMeshMgr::Instance()->m_vecRidePtrArray;
	for( t_i =0 ;t_i<MAX_RIDENUM;t_i ++ )
	{
		if( t_ptrRide[t_i].m_ptrHuman == this )
		{
			t_ptrRide[t_i].m_ptrHuman = NULL;
			t_ptrRide[t_i].m_ptrRide = NULL;
			break;
		}
	}

	if( m_RideObj )
	{
		m_RideObj->DelLinkHumObj();
		m_RideObj->m_bIsRide = false;
		m_RideObj = NULL;
	}

	unguard;
}
# if __RIDE_EDITOR__
//!得到最后一部分的位置
void osc_skinMesh::GetLastPartPos( osVec3D & _v3d )
{
	//!
	//osc_bpmInstance *pInst = &m_vecBpmIns[m_iNumParts-1];

	//!对bmpinstance有效性进行检查
	//osc_bodyPartMesh* pPart = pInst->get_bpmPtr();
	//!
	//_v3d = pPart->m_ptrRide->m_vec3Center;
	_v3d = m_vec3RideOffset;

}
//!设置最后一部分的位置
void osc_skinMesh::SetLastPartPos( osVec3D & _v3d)
{
	m_vec3RideOffset = _v3d;
	//!
	//osc_bpmInstance *pInst = &m_vecBpmIns[m_iNumParts-1];

	//!对bmpinstance有效性进行检查
	//osc_bodyPartMesh* pPart = pInst->get_bpmPtr();
	//!

	//pPart->m_ptrRide->m_vec3Center = _v3d;
}
//!保存最后一部分的位置到文件中
void osc_skinMesh::SaveLastPart()
{
	//osVec3D t_pos;
	//!
	//osc_bpmInstance *pInst = &m_vecBpmIns[m_iNumParts-1];

	//!对bmpinstance有效性进行检查
	//osc_bodyPartMesh* pPart = pInst->get_bpmPtr();
	//GetLastPartPos(t_pos);
	//!
	//pPart->SaveCenterPos(t_pos);

}
# endif
//@}