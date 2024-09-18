//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldObj.cpp
 *
 *  His:      River created @ 2006-1-14
 *
 *  Des:      ��ͼ����Ʒ����������
 *   
 * ����ּ���֮�ǣ����������������ǽ֮��Ҳ���� 
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
# include "../GeoMipTerrain/osMapTerrain.h"



//! ��������Ƿ����Ʒ�����ཻ.
BOOL osc_newTileGround::is_intersectObj( int _objIdx,
					osVec3D& _rayS,float _dis,osVec3D& _rayDir,float* _resDis/* = NULL*/ )
{
	guard;

	os_newTGObject*   t_ptrObj;
	
	osassert( _objIdx <= m_iTGObjectNum );

	t_ptrObj = &m_vecTGObject[_objIdx];

	// 
	// �����ǵ�������Ʒ���ܴ���
	if( t_ptrObj->m_dwType == 0 )
	{
		float         t_fXoffset,t_fZoffset;
		osVec3D       t_vec3CamS,t_vec3CamDir;
		osMatrix      t_mat,t_matScale;

		t_fXoffset = m_sTgHeader.m_wMapPosX*MAX_GROUNDWIDTH*TILE_WIDTH;
		t_fZoffset = m_sTgHeader.m_wMapPosZ*MAX_GROUNDHEIGHT*TILE_WIDTH;

		bool t_b;
		bool t_bScale = !vec3_equal( t_ptrObj->m_vec3ObjectScale,osVec3D( 1.0f,1.0f,1.0f ) );

		// 
		//! River @ 2009-5-4: ���ź�������ʹ�����������֧�����Ż�
		if( t_bScale )
		{
			osMatrixScaling( &t_matScale,t_ptrObj->m_vec3ObjectScale.x,
				t_ptrObj->m_vec3ObjectScale.y,t_ptrObj->m_vec3ObjectScale.z );

			osMatrixRotationY( &t_mat,t_ptrObj->m_fAngle );
			t_mat = t_matScale * t_mat;

			// ������Ʒ��λ��
			t_mat._41 = t_ptrObj->m_vec3ObjPos.x;
			t_mat._42 = t_ptrObj->m_vec3ObjPos.y;
			t_mat._43 = t_ptrObj->m_vec3ObjPos.z;

			float  t_fTmp;
			osMatrixInverse( &t_matScale,&t_fTmp,&t_mat );

			// ��ת���λ�úͷ�����Ʒ�ռ�
			t_vec3CamS = _rayS;
			osVec3Transform( &t_vec3CamS,&t_vec3CamS,&t_matScale );
			osVec3TransformNormal( &t_vec3CamDir,&_rayDir,&t_matScale );
			osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );

			// 
			// River mod @ 2009-4-24:�������ź���Ʒ����ײ�����Ϣ
			float   t_f = osVec3Length( &t_vec3CamS ); 

			t_b = g_ptrMeshMgr->intersect_mesh( 
				m_vecObjectDevIdx[_objIdx],t_vec3CamS,_dis,t_vec3CamDir,_resDis );

			//! ������ײ��
			if( t_b )
			{
				osVec3D    t_vec3Tmp;
				t_vec3Tmp = t_vec3CamS + t_vec3CamDir*(*_resDis);
				osVec3Transform( &t_vec3CamS,&t_vec3Tmp,&t_mat );

				*_resDis = osVec3Length( &(t_vec3CamS - _rayS ) );
			}
		}
		else
		{
			// �����������ǰ��Mesh�ռ䣬Ȼ�����Mesh�Ĵ�����
			osMatrixRotationY( &t_mat,-t_ptrObj->m_fAngle );

			t_vec3CamS.y  = _rayS.y - t_ptrObj->m_vec3ObjPos.y;
			t_vec3CamS.x  = _rayS.x - t_ptrObj->m_vec3ObjPos.x;
			t_vec3CamS.z  = _rayS.z - t_ptrObj->m_vec3ObjPos.z;

			// ��ת���λ�úͷ�����Ʒ�ռ�
			osVec3Transform( &t_vec3CamS,&t_vec3CamS,&t_mat );
			osVec3TransformNormal( &t_vec3CamDir,&_rayDir,&t_mat );
			osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );

			t_b = g_ptrMeshMgr->intersect_mesh( 
				m_vecObjectDevIdx[_objIdx],t_vec3CamS,_dis,t_vec3CamDir,_resDis );
		}


		return t_b;	
	}

	return false;


	unguard;
}

// 
# define CAMCOLL_OFFSET  0.1f

//! River added @ 2009-5-3:���ڴ������������֮�����Ʒ��͸����ƫ��ֵ��
OSENGINE_API float g_fCamFocusOffset = 0.0f;

/** \brief
*  �Եõ�����Ʒ���б���д���
*
*  �㷨������
*  ��ÿһ����Ʒ���䵽ÿһ��tileGround.
*/
bool osc_TGManager::process_camChrObjList( void )
{
	guard;

	osVec3D            t_vec3CamPos,t_vec3CamDir,t_vec3HeroPos;
	osc_TileGroundPtr  t_ptrTG;
	float              t_fDis;
	static int t_vecObjId[MAXOBJNUM_TOCHAR];
	int        t_iFindNum = 0,t_iObjNum = 0;

	osassert( g_ptrCamera );

	//! ��������Ĵ��¸߶�
	t_vec3HeroPos = g_ptrCamera->get_camFocus();
	t_vec3HeroPos.y += g_fCamFocusOffset;
	
	// �������ڸ��ӵ���Ʒ�б�
	m_iCamChrObjNum = 0;
	for( int t_i=0;t_i<this->m_iActMapNum;t_i ++ )
	{	
		t_iFindNum += m_arrPtrTG[t_i]->m_sQuadTree.get_chunkObj(
			 t_vec3HeroPos,&t_vecObjId[0],t_iObjNum,MAXOBJNUM_TOCHAR );

		if( m_iCamChrObjNum < t_iObjNum )
		{
			for( int t_idx = m_iCamChrObjNum;t_idx<t_iObjNum;t_idx ++ )
			{
				m_vecCamChrObj[t_idx].m_iX = WORD(t_i);
				m_vecCamChrObj[t_idx].m_iY = t_vecObjId[t_idx];
			}
			m_iCamChrObjNum = t_iObjNum;
		}
	}

	//
	// �õ���������ߡ�
	if( m_iCamChrObjNum > 0 )
	{
		g_ptrCamera->get_curpos( &t_vec3CamPos );
		t_vec3CamDir = t_vec3CamPos - t_vec3HeroPos;
		t_fDis = osVec3Length( &t_vec3CamDir );
		osVec3Normalize( &t_vec3CamDir,&t_vec3CamDir );

		//! River: �Ӵ�ֵ��ȷ�����λ��������һЩ,��С���ÿһ֡�ķ���
		t_fDis += CAMCOLL_OFFSET;

	}

	osassert( MAXOBJNUM_TOCHAR >= m_iCamChrObjNum );

	float      t_fClosestLength = 10000.0f;
	float      t_fTmp ;
	for( int t_i=0;t_i<m_iCamChrObjNum;t_i ++ )
	{
		t_ptrTG = m_arrPtrTG[m_vecCamChrObj[t_i].m_iX];

		// CAMERA WORK:
		// River mod @ 2009-2-5:������ཻ�����¼���㣬�ҵ�����������Ľ���
		// ������������λ��,��һ��������ε���ȷ�ظ������ԭ���ĵ�λ�ã�������
		// һ����ײ�Ĵ���������Ʒ�Ļظ����ͻ�����һЩ�鷳
		if( t_ptrTG->is_intersectObj( 
			m_vecCamChrObj[t_i].m_iY,t_vec3HeroPos,t_fDis,t_vec3CamDir,&t_fTmp ) ) 
		{
			//! �����������Ʒ����ʲô������.
			if( t_ptrTG->is_bridgeObj( m_vecCamChrObj[t_i].m_iY ) )
				continue;

			if( t_ptrTG->is_NoAlphaObj( m_vecCamChrObj[t_i].m_iY ) )
				continue;


			// River added @ 2009-3-3:�����������Ʒ�����͸������
			// River mod @ 2009-4-27:����Ĵ���һ����,Ĭ����ƷΪ������Ʒ.
			if( !t_ptrTG->is_treeObj( m_vecCamChrObj[t_i].m_iY ) )
			{
				t_ptrTG->m_ivoMgr.make_halfFade( m_vecCamChrObj[t_i].m_iY );
				continue;
			}

			if( t_fTmp < t_fClosestLength )
				t_fClosestLength = t_fTmp;

		}
	}

	if( m_iCamChrObjNum > 0 )
	{
		if( t_fClosestLength <= (t_fDis-CAMCOLL_OFFSET ) )
			g_ptrCamera->move_cameraToAvoidShelter( t_fClosestLength );
		// �˴��ĵ��ã�Ϊ���ǲ���������ַ���
		else if( t_fClosestLength <= t_fDis )
			g_ptrCamera->move_cameraToAvoidShelter( -1.0f );
	}


	return true;

	unguard;
}

//! River added @ 2007-6-8:����ر�߶ȵ���Ϣ����
//! �˴��ĸ߶ȣ�Ϊ���ر�߶ȣ�����Ⱦ�ر�����ݸ߶ȡ�
bool osc_TGManager::get_earthHeight( osVec3D& _pos )
{
	guard;

	os_tileIdx                    t_tgIdx;
	float                         t_fU,t_fV,t_fOrigX,t_fOrigZ;
	int                           t_iColX,t_iColZ;

	_pos.y = 0.0f;

	// River mod @ 2007-4-18:
	if( (_pos.x < 0.0f)||(_pos.z<0.0f)||
		(_pos.x > g_fMapMaxX) || (_pos.z > g_fMapMaxZ) )
	{
		_pos.y = 0.0f;
		return false;
	}


	// ��������Ŵ��ͼ���ӵ�����
	t_fU = _pos.x / TILE_WIDTH;
	t_fV = _pos.z / TILE_WIDTH;
	t_iColX = int(t_fU);
	t_iColZ = int(t_fV);

	// �õ���ײ��Ϣ
	t_tgIdx.m_iX = WORD(t_iColX / MAX_GROUNDWIDTH);
	t_tgIdx.m_iY = WORD(t_iColZ / MAX_GROUNDWIDTH);

	// windy add 4-27
	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		_pos.y = 0.0f;
		return false;
	}
	const float* t_tileColHeightInfo;
	t_tileColHeightInfo = osc_mapTerrain::
		get_mapTerrainIns()->get_mapTileHeightInfo( t_tgIdx.m_iX,t_tgIdx.m_iY );
	if( NULL ==  t_tileColHeightInfo )
	{
		_pos.y = 0.0f;
		return false;
	}
	
	// �㴫��������һ����ʾ�����ڵ�uv.
	t_fU = t_fU - t_iColX;
	t_fV = t_fV - t_iColZ;

	// ������ʾ���ӵ�ԭ������x,zֵ
	t_fOrigX = t_iColX*TILE_WIDTH;
	t_fOrigZ = t_iColZ*TILE_WIDTH;

	t_iColX -= (t_tgIdx.m_iX*MAX_GROUNDWIDTH);
	t_iColZ -= (t_tgIdx.m_iY*MAX_GROUNDWIDTH);

	osVec3D v00( t_fOrigX,
		t_tileColHeightInfo[65*t_iColZ+t_iColX],
				 t_fOrigZ );
	osVec3D v10( t_fOrigX +TILE_WIDTH,
		t_tileColHeightInfo[65*t_iColZ+t_iColX+1],
				t_fOrigZ );
	osVec3D v01( t_fOrigX,
		t_tileColHeightInfo[65*(t_iColZ+1)+t_iColX],
				t_fOrigZ + TILE_WIDTH );
	osVec3D v11( t_fOrigX +TILE_WIDTH,
		t_tileColHeightInfo[65*(t_iColZ+1)+t_iColX+1],
				t_fOrigZ+TILE_WIDTH );

	osPlane t_plane;
	if( (t_fU+t_fV)<1.0f )
	{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v00);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
	}
	else{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v11);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
	}
    
	osassertex( !_isnan( _pos.y ),"�п����ǵ�ͼ���ϵĸ߶Ȳ��Ǹ�������ɵ�����" );

	return true;

	unguard;
}

/** \brief 
*	����һ���������꣬����һ��DWORD��ֵ�����������������Ĺ���ͼ����ɫ��
*
*	\param osVec3D& _pos		x,z ������,Ϊ��������
*/
DWORD osc_TGManager::get_lightMapColor(const osVec3D& _pos)
{
	guard;

	//! river @ 2009-12-15:�ͷŵ�ͼ���п���ĳЩ�ط�����ô�ֵ��ȷ��������
	if( !m_bLoadedMap )
		return 0xff000000;

	// River mod @ 2007-4-18:
	if( (_pos.x < 0.0f)||(_pos.z<0.0f)||
		(_pos.x > g_fMapMaxX) || (_pos.z > g_fMapMaxZ) )
		return 0xff000000;

	const os_tileIdx t_tgIdx(WORD((int)(_pos.x / TILE_WIDTH) / MAX_GROUNDWIDTH),
							 WORD((int)(_pos.z / TILE_WIDTH) / MAX_GROUNDWIDTH));

	// River mod @ 2007-8-26:
	// ���ڼ����ͼ�б���صĴ���ʹ���ٽ�������Ϊ�ϲ�ʹ���˶��߳�
	::EnterCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

	// windy add 4-27
	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

		return 0xff000000;
	}

	osVec3D t_tmpPos = _pos;
	DWORD t_color = 0xff000000;
	if(get_objHeightInfo(t_tmpPos) <= -9999.0f){
		// ����������Ʒ����,�ͼ�����Ӱ
		//
		t_color = osc_mapTerrain::get_mapTerrainIns()->get_mapTileLightInfo(_pos);
	}
	
	::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

	return t_color;


	unguard;
}
# define INTERSECT_TESTOFFSET 1000.0f

//! �õ�λ�������봫��tg�߶��ཻ�ĸ߶����ݣ����ڸ���ȷ�ĵõ�������Ʒ����ײ��Ϣ
float osc_TGManager::get_intersectHeight( osc_TileGroundPtr  _tg,
						int _objNum,const int* _objList,osVec3D& _pos )
{
	guard;

	osassert( _tg );

	if( _objNum == 0 )
		return INTERSECT_TESTOFFSET;

	osVec3D   t_vec3Start,t_vec3Dir;

	t_vec3Start = _pos;
	t_vec3Start.y += INTERSECT_TESTOFFSET;

	t_vec3Dir = osVec3D( 0.0f,-1.0f,0.0f );

	//! ��ÿһ����Ʒ��id���м��,�鿴���߸�ÿһ����Ʒ���ཻ��Ϣ.
	float      t_fObjHeight = -10*INTERSECT_TESTOFFSET;
	float      t_fDis = INTERSECT_TESTOFFSET*10;
	BOOL       t_bRes;

	for( int t_i = 0;t_i<_objNum;t_i ++ )
	{
		t_bRes = _tg->is_intersectObj( _objList[t_i],
			t_vec3Start,INTERSECT_TESTOFFSET*10,t_vec3Dir,&t_fDis );
		if( TRUE == t_bRes )
		{
			if( t_fObjHeight < (t_vec3Start.y - t_fDis) )
				t_fObjHeight = t_vec3Start.y - t_fDis;
		}
	}

	return t_fObjHeight;

	unguard;
}


/** \brief
 *  �õ�������ĳ��λ���ϸ��ر���Ʒ��صĸ߶���Ϣ,���û����Ʒ,��߶���ϢΪ��С�ĸ�ֵ.
 *
 *  �㷨����:
 *  1: �õ���ǰλ�õ�chunk�ڵ���Ʒ�б�.
 *  2: �鿴��ǰλ�����ϵ����߸�������һ����Ʒ���,�ҵ���ߵĽ���.
 *  3: ���ؽ����Yֵ����.
 *  
 */
float osc_TGManager::get_objHeightInfo( osVec3D& _pos )
{
	guard;

	//! ����λ�õõ�����tg
	osc_TileGroundPtr    t_ptrTG;
	float                t_fObjHeight = -100000.0f;

	int  t_x = int(_pos.x / (TILE_WIDTH*MAX_GROUNDWIDTH));
	int  t_y = int(_pos.z / (TILE_WIDTH*MAX_GROUNDWIDTH));

	t_ptrTG = get_tground( t_x,t_y );
	if( !t_ptrTG )
		return t_fObjHeight;

	//! �õ���Ʒ���б�.
	static int  t_iObjList[32];
	int         t_iObjNum = 0;

	// River mod @ 2007-4-24:������Ʒ����ֻռ��һ��mapTile,���屣������
	// ��ǰ��tile,������Ҫ����Χ��mapTile��quadTree���д����õ�����ȷ
	// ��ײ���
	// �õ���Χ��tileGroundָ�룬Ȼ������ײ���Ƿ�����ײ��Ʒ��
	for( int x = t_x-1;x<=t_x+1;x++ )
	{
		for( int y = t_y-1;y<=t_y+1;y++ )
		{
			if( (x<0)||(x>=m_iMaxXTgNum)||
				(y<0)||(y>=m_iMaxZTgNum) )
				continue;

			t_ptrTG = get_tground( x,y );
			if( !t_ptrTG )
				continue;

			// ����Ҫ��������tileGround�ͳ�����ͼ��Χ�ġ�
			if( (x==t_x)&&(y==t_y) )
			{
				t_iObjNum = t_ptrTG->
					m_sQuadTree.get_posInChunkObj( _pos,t_iObjList,128 );
			}
			else
			{
				t_iObjNum = t_ptrTG->
					m_sQuadTree.get_posExceedQuadObj( _pos,t_iObjList,128 );
			}

			if( t_iObjNum == 0 )
				continue;

			//! ����Ҫ��ײ��������Ʒ�߶�
			float   t_fNewHeight = get_intersectHeight( 
				t_ptrTG,t_iObjNum,t_iObjList,_pos );

			if( t_fNewHeight > t_fObjHeight )
				t_fObjHeight = t_fNewHeight;
		}
	}

	return t_fObjHeight;

	unguard;
}
