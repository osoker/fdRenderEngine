//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldStruct.cpp
 *
 *  His:      River created @ 2006-1-10
 *
 *  Des:      ��ͼ��������صĽṹ��
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


//! TG�ڲ�����������ڵĹ�������ָ�롣
osc_TGManager* osc_newTileGround::m_ptrTgMgr = NULL;

//! ����ˮ��߶�ʱ�� snowwin add
std::vector<float> osc_newTileGround::m_vecWaterHeight;

osc_newTileGround::osc_newTileGround()
{
	m_bInuse = FALSE;
	m_strMapFName = "";
	

	m_bInMTQueue = false;

}

osc_newTileGround::~osc_newTileGround()
{

}


//! �õ���Ʒ������BoundingBox.
void osc_newTileGround::get_objBBox( int _objIdx,os_bbox& _box )
{
	guard;

	os_aabbox*      t_ptrBox;
	os_newTGObject*     t_ptrObj;
	osVec3D         t_vec3Tmp;
	
	osassert( _objIdx <= m_iTGObjectNum );

	t_ptrObj = &m_vecTGObject[_objIdx];
	if( t_ptrObj->m_dwType == 0 )
	{
		t_ptrBox = g_ptrMeshMgr->get_meshBBox( m_vecObjectDevIdx[_objIdx] );

		//! river mod @ 2009-4-25:׼������Ʒ������Ϣ
		t_vec3Tmp.x = t_ptrBox->m_vecMax.x * t_ptrObj->m_vec3ObjectScale.x;
		t_vec3Tmp.y = t_ptrBox->m_vecMax.y * t_ptrObj->m_vec3ObjectScale.y;
		t_vec3Tmp.z = t_ptrBox->m_vecMax.z * t_ptrObj->m_vec3ObjectScale.z;
		_box.set_bbMaxVec( t_vec3Tmp   );

		t_vec3Tmp.x = t_ptrBox->m_vecMin.x * t_ptrObj->m_vec3ObjectScale.x;
		t_vec3Tmp.y = t_ptrBox->m_vecMin.y * t_ptrObj->m_vec3ObjectScale.y;
		t_vec3Tmp.z = t_ptrBox->m_vecMin.z * t_ptrObj->m_vec3ObjectScale.z;
		_box.set_bbMinVec( t_vec3Tmp );

		_box.set_bbYRot( t_ptrObj->m_fAngle );
		_box.set_bbPos( t_ptrObj->m_vec3ObjPos );	

		return;
	}
	
	_box.set_bbMaxVec( osVec3D( 0.0f,0.0f,0.0f ) );
	_box.set_bbMinVec( osVec3D( 0.0f,0.0f,0.0f ) );

	return;
	
	unguard;
}


/** \brief
*  �鿴һ��һ�������Ƿ��һ��Tile��os_polygon�ཻ��
*
*  \param _tileIdx  ����͵�ǰ���ӵ��������ཻ�����غ͵�ǰ�����ڵ���
*                   һ����ײ�����ཻ����ײ���ӵ�������zֵ���ģ�xֵ��С�ķ�λΪ0,
*                   x,z���ķ�λΪ1,z��С��x���ķ�λΪ2��
*  \param _poly     ����ĸ��Ӷ�Ӧ����Ⱦ�����Ρ�
*  
*/
bool osc_newTileGround::intersect_polygon( 
					const osVec3D* _arrVerData,osVec3D& _s,osVec3D& _e )
{
	guard;

	float           t_u,t_v,t_dis;
	osassert( _arrVerData );

	// ����͵�һ���������ཻ��
	if( D3DXIntersectTri( &_arrVerData[0],
		&_arrVerData[1],
		&_arrVerData[2],&_s,&_e,&t_u,&t_v,&t_dis ) )
	{
		_e = _arrVerData[0] + 
			t_u*(_arrVerData[1]-_arrVerData[0] )
			+t_v*(_arrVerData[2] - _arrVerData[0] );
		return true;
	}

	//
	// ����͵ڶ����������ཻ,�ı������������������˳�����ڸ��õ�ʹ��
	// �ཻ�������ص�����uv���ꡣ
	if( D3DXIntersectTri( &_arrVerData[3],
		&_arrVerData[2],
		&_arrVerData[1],&_s,&_e,&t_u,&t_v,&t_dis ) )
	{
		_e = _arrVerData[3] + 
			t_u*(_arrVerData[2]-_arrVerData[3] )
			+t_v*(_arrVerData[1] - _arrVerData[3] );
		return true;
	}

	return false;

	unguard;
}


//! ��ǰ�����TG�ڵ���һ��顣
osn_inTgPos osc_newTileGround::m_seCamTgPos = OSN_TGPOS_ERROR;

/** \brief
*  ���ݴ����λ�ã��ж�������ڵ�TG����һ��顣
*
*  ����Ĳ���Ϊ��ײ�������������ͼ�ϵ�λ�á�
*/
void osc_newTileGround::frame_moveCamInTgPos( int _camx,int _camy )
{
	guard;

	int t_x,t_y;


	
	t_x = _camx%GROUNDMASK_WIDTH;
	t_y = _camy%GROUNDMASK_WIDTH;

	m_seCamTgPos = OSN_TGPOS_ERROR;

	if( (t_x<MAX_GROUNDWIDTH)&&(t_y>=MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_TOPLEFT;
	
	if( (t_x>=MAX_GROUNDWIDTH)&&(t_y>=MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_TOPRIGHT;

	if( (t_x<MAX_GROUNDWIDTH)&&(t_y<MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_BOTLEFT;

	if( (t_x>=MAX_GROUNDWIDTH)&&(t_y<MAX_GROUNDWIDTH) )
		m_seCamTgPos = OSN_TGPOS_BOTRIGHT;

	osassert( m_seCamTgPos != OSN_TGPOS_ERROR );

	return;
	unguard;
}

//! �õ������tg�ڵ���һ��.
osn_inTgPos osc_newTileGround::get_camInTgPos( void )
{
	guard;

	return m_seCamTgPos;

	unguard;
}

/** \brief
*  �ѵ�ǰ֡�ɼ���object�ŵ���Ⱦ�б���.
*/ 
void osc_newTileGround::push_objIdx( int _oidx )
{
	guard;

	osassert( _oidx<m_iTGObjectNum );
	osassert( _oidx>=0 );
	m_vecObjInView[_oidx] = true;

	return ;

	unguard;
}


/** \brief
*  �õ��ɼ�����Ʒ�б�
*
*  �������Ż�����㷨���ͼ򵥵��㷨��ֻҪ����������Ʒ�����ǿɼ�����Ʒ��
*/
void osc_newTileGround::get_inviewObjList( void )
{
	guard;

	float           t_fRadius;
	osVec3D          t_vec3Pos;
	int             t_idx;
	os_newTGObject*     t_ptrObj;



	//
	// ������Ʒ�Ĵ�С����Ʒ�Ŀɼ��Խ��д���
	for( int t_i=0;t_i<m_iTGObjectNum;t_i++ )
	{
		// windy mod 12-27
		if( !m_vecObjInView[t_i] )
			continue;

		// ������õ���ǰ����Ʒ�Ƿ�Ӧ����ʾ����Ұ�ڡ�
		t_ptrObj = &m_vecTGObject[t_i];
		if( (t_ptrObj->m_dwType ==0)||
			((t_ptrObj->m_dwType == 1)&&
			 (!g_bUseAnimationObj) )     )
		{
			t_idx = m_vecObjectDevIdx[t_i];
			// �����µ�mesh.
			if( t_idx == -1 )
			{
				create_meshImmediate( t_i );
				t_idx = m_vecObjectDevIdx[t_i];
			}


			t_fRadius = g_ptrMeshMgr->get_meshRadius( t_idx );

			t_vec3Pos = m_vecTGObject[t_i].m_vec3ObjPos;

			// 
			//���������ɼ����жϣ����������Ұ�ڣ���ǰ����ƷΪ���ɼ���
			//   ֻ�о�̬����Ʒ��ʹ��keyObj����
			if( !g_ptrMeshMgr->is_keyObj( t_idx ) )
			{
				m_vecObjInView[t_i] = 
					g_ptrCamera->is_viewByCamera( t_vec3Pos,t_fRadius );
			}
			else
			{
				m_vecObjInView[t_i] = MASK_HALF_VIEW;
			}

		}
		// �����skinMeshObj,���뵽skinMeshObj���С�
		else if( t_ptrObj->m_dwType == 1 )
		{

			osc_skinMesh*    t_smObj;
				
			t_smObj = (osc_skinMesh*)m_vecObjectDevIdx[t_i];
			t_smObj->get_smPos( t_vec3Pos );
			t_fRadius = t_smObj->get_smBSRadius();
			m_vecObjInView[t_i] = 
				g_ptrCamera->is_viewByCamera( t_vec3Pos,t_fRadius );

		}
		else if( t_ptrObj->m_dwType == 4 )
		{
			// �Ȳ�����osaMesh���������;
			m_vecObjInView[t_i] = MASK_HALF_VIEW;
		}
		// ����ˮ��.
		else if( t_ptrObj->m_dwType == 2 )
		{
			m_vecObjInView[t_i] = MASK_HALF_VIEW;
		}
		else if( t_ptrObj->m_dwType == 8 )
		{
			m_vecObjInView[t_i] = MASK_HALF_VIEW;
		}
		else
		{
			m_vecObjInView[t_i] = false;
		}
	}

	// TEST CODE:
	//osassert( m_ivoMgr.m_vecFadeOutObj[0] < (int)m_vecTGObject.size() );
	//osassert( m_vecObjInView.size() <= m_vecTGObject.size() );

	// 
	// ʹ�õ�ǰTG����Ʒ�Ŀɼ����б�����Ʒ���д���
	m_ivoMgr.frame_processIvo( m_vecObjInView );

	// TEST CODE:
	//osassert( m_ivoMgr.m_vecFadeOutObj[0] < (int)m_vecTGObject.size() );

	return;

	unguard;
}


/** \brief
*   �õ������ڵ�ĳ�����ӵĶ���.
*  
*  ��������Ⱦ����ε�ʾ�⣺4�������λ������
*  
*    Z����
*    |
*    |    1--------3
*    |    |        |
*    |    |        |
*    |    |        |
*    |    0--------2
*    |   
*   Origin-------------X����
 */
void osc_newTileGround::get_polyVertex( osVec3D* _vec,int _x,int _z )
{
	guard;

	osassert( _x<64 );
	osassert( _z<64 );

	float   t_fXoffset = m_sTgHeader.m_wMapPosX*64*3.0f;
	float   t_fZoffset = m_sTgHeader.m_wMapPosZ*64*3.0f;

	_vec[0].x = _x*3.0f + t_fXoffset;
	_vec[0].z = _z*3.0f + t_fZoffset;
	_vec[0].y = m_vecHeight[_z*65 + _x];

	_vec[1].x = _vec[0].x;
	_vec[1].z = (_z+1)*3.0f + t_fZoffset;
	_vec[1].y = m_vecHeight[(_z+1)*65 + _x];

	_vec[2].x = (_x+1)*3.0f + t_fXoffset;
	_vec[2].z = _vec[0].z;
	_vec[2].y = m_vecHeight[_z*65 + _x + 1];

	_vec[3].x = _vec[2].x;
	_vec[3].z = _vec[1].z;
	_vec[3].y = m_vecHeight[(_z+1)*65 + _x + 1];

	return;

	unguard;
}

//! ����һ���ر������ChunkΪ�ɻ���״̬
void osc_newTileGround::set_chunkDraw( int _idx,BOOL _draw/* = TRUE*/ )
{
	guard;

	int   t_x,t_z;

	osassert( _idx < CHUNKNUM_PERTG );

	t_z = _idx / MapTile::CHUNKS;
	t_x = _idx - t_z*MapTile::CHUNKS;

	m_sMapTerrain.SetChunkVisible( t_x,t_z,_draw );

	m_bEntireCullOff = FALSE;

	return;

	unguard;
}

//! ����һ������Chunk�ܶ�̬���Ӱ��
void osc_newTileGround::set_chunkAffectByDL( int _idx )
{
	int   t_x,t_z;

	osassert( _idx < CHUNKNUM_PERTG );

	t_z = _idx / MapTile::CHUNKS;
	t_x = _idx - t_z*MapTile::CHUNKS;

	m_sMapTerrain.setChunkAffectByDl( t_x,t_z );

	return;

}



//! FrameSet,ÿһ֡���õ�ǰtg�ڵ�����
void osc_newTileGround::frame_setTG( void )
{
	guard;

	m_bEntireCullOff = TRUE;

	// ���赱ǰTG�����е�ObjectΪ���ɼ���
	if( m_iTGObjectNum > 0 )
		memset( &m_vecObjInView[0],0,sizeof( BYTE )*m_iTGObjectNum );

	//���������е�obj�Ķ�̬��Ϊ��
	for( int t_i=0;t_i<m_iTGObjectNum;t_i ++ )
		m_vecTGObjDL[t_i].reset_dl();

	
	//
	// ���õ�ǰTG��InviewObjMgr��
	m_ivoMgr.frame_setObjMgr();
	    
	osMatrix   t_sMatReflect;
    osPlane    t_sClipPlane;

	osPlaneFromPointNormal( &t_sClipPlane,
		&osVec3D( 0.0f,m_ptrTgMgr->m_sSceneTexData.get_waterHeight(),0.0f ),
		&osVec3D( 0.0f,1.0f,0.0f ) );

	osMatrixReflect( &t_sMatReflect,&t_sClipPlane );

	m_sQuadTree.cal_visualTileSet( t_sMatReflect );

	// 
	// ʹ�õ�ǰTG����Ʒ�Ŀɼ����б�����Ʒ���д���
	get_inviewObjList();


	return;

	unguard;
}

//! ����Chunk���ܵ��ĸ���
void osc_fdChunkPick::construct_clipPlanes( float _minX,float _minZ )
{
	guard;

	osVec3D         t_vec3Pt[4];
	osVec3D         t_vec3Temp;
	float           t_fMaxX,t_fMaxZ;

	t_fMaxX = _minX + TILE_WIDTH*MapTile::GRIDS;
	t_fMaxZ = _minZ + TILE_WIDTH*MapTile::GRIDS;

	t_vec3Pt[0].x = _minX;t_vec3Pt[0].y = 0.0f;t_vec3Pt[0].z = _minZ;
	t_vec3Pt[1].x = _minX;t_vec3Pt[1].y = 0.0f;t_vec3Pt[1].z = t_fMaxZ;
	t_vec3Pt[2].x = t_fMaxX;t_vec3Pt[2].y = 0.0f;t_vec3Pt[2].z = t_fMaxZ;
	t_vec3Pt[3].x = t_fMaxX;t_vec3Pt[3].y = 0.0f;t_vec3Pt[3].z = _minZ;

	
	t_vec3Temp = t_vec3Pt[0]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[0],&t_vec3Pt[0],&t_vec3Temp,&t_vec3Pt[1] );

	t_vec3Temp = t_vec3Pt[1]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[1],&t_vec3Pt[1],&t_vec3Temp,&t_vec3Pt[2] );

	t_vec3Temp = t_vec3Pt[2]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[2],&t_vec3Pt[2],&t_vec3Temp,&t_vec3Pt[3] );

	t_vec3Temp = t_vec3Pt[3]; t_vec3Temp.y = 1.0f;
	osPlaneFromPoints( &m_arrPlane[3],&t_vec3Pt[3],&t_vec3Temp,&t_vec3Pt[0] );

	unguard;
}

void osc_fdChunkPick::push_testTile( int _x,int _z )
{
	guard;

	osassert( this->m_iTestTileNum > 0 );

	// River mod @ 2007-10-8:
	// ���봦��,��������ֵ������,�򲻴����������.
	if( _x>=MapTile::GRIDS )
		return;
	if( _z>=MapTile::GRIDS )
		return;

	osassertex( _x<MapTile::GRIDS,va( "x val is:<%d>",_x) );
	osassertex( _z<MapTile::GRIDS,va( "z val is:<%d>",_z) );
	
	/*
	osassert( _x>=0 );
	osassert( _z>=0 );
	*/
	// River @ 2011-10-14:��������⣬�򲻴���˸���.
	if( _x < 0 )
		return;
	if( _z < 0 )
		return;

	if( (m_arrTestTile[m_iTestTileNum-1].m_iX == _x)&&
		(m_arrTestTile[m_iTestTileNum-1].m_iY == _z) )
		return;
	m_arrTestTile[m_iTestTileNum].m_iX = _x;
	m_arrTestTile[m_iTestTileNum].m_iY = _z;
	m_iTestTileNum ++;
	return;

	unguard;
}




//
//! ���ݴ�������ߺͿ�ʼ�������꣬���β��ҿ�����صĸ���
int osc_fdChunkPick::find_testTileArr( float _minZ,int _startX )
{
	guard;
	osVec3D   t_vec3IntsetPt;
	osVec3D*  t_ptr;
	static const int t_iClip1 = 1,t_iClip2 = 3;

	osassert( m_arrTestTile );
	osassert( m_arrPlane );

	// 
	// �ڣط����ϣ���ÿһ��Chunk�ڵ�Z�����,������ֱ��XZƽ����棬
	// Ȼ����㴫������߸������Ľ��㣬���ݽ�����в�ͬ�Ĵ���
	for( int t_i=_startX;t_i<=MapTile::GRIDS;t_i ++ )
	{
		osPlane   t_tmpPlane;
		t_tmpPlane = m_arrPlane[0];
		t_tmpPlane.d -= (t_i*TILE_WIDTH);

		t_ptr = osPlaneIntersectLine( &t_vec3IntsetPt,
			&t_tmpPlane,&this->m_vec3RayStart,&this->m_vec3RayEnd );
		
		// 
		// ���Ray���߸�Z����ƽ��,��������е�Z�����ϵĸ���
		if( NULL == t_ptr )
		{
			for( int t_z=m_arrTestTile[m_iTestTileNum-1].m_iY;t_z < 16;t_z ++ )
				push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_z );

			return m_iTestTileNum;
		}

		// 
		// �������1,3�����clip����
		ose_geopos  t_pos1 = osn_mathFunc::classify_PointByPlane(
			m_arrPlane[t_iClip1],t_vec3IntsetPt );
		ose_geopos  t_pos2 = osn_mathFunc::classify_PointByPlane(
			m_arrPlane[t_iClip2],t_vec3IntsetPt );

		if( ( OSE_BACK != t_pos1 ) && ( OSE_BACK != t_pos2 ) )
		{
			int t_iZTileIdx = int((t_vec3IntsetPt.z-_minZ)/TILE_WIDTH);

			// ����������ཻ��
			if( t_iZTileIdx > m_arrTestTile[m_iTestTileNum-1].m_iY )
			{
				for( int t_idx = m_arrTestTile[m_iTestTileNum-1].m_iY+1;
					t_idx < t_iZTileIdx;t_idx ++ )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_idx );
				}
			}
			else
			{
				for( int t_idx = m_arrTestTile[m_iTestTileNum-1].m_iY-1;
					t_idx > t_iZTileIdx;t_idx -- )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_idx );
				}
			}

			//! River @ 2006-3-14:��Ϊ��������Ĳ���ȷ���˴����ܻ���ڵ���16��
			if( t_iZTileIdx < MapTile::GRIDS )
				push_testTile( t_i-1,t_iZTileIdx );

			
			if( (t_i < MapTile::GRIDS)&&(t_iZTileIdx < MapTile::GRIDS) )
				push_testTile( t_i,t_iZTileIdx );

		}
		else
		{
			// 
			// �����ߵĽ�����clip��֮��Ĵ���
			// ����ڱ��Ϊ1��clip��֮��,��������һ�������ཻtile�ڴ�Chunk
			// ��Z���������е�Tile.
			if( OSE_BACK == t_pos1 )
			{
				for( int t_i=m_arrTestTile[m_iTestTileNum-1].m_iY+1;
					t_i < MapTile::GRIDS;t_i ++ )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_i );
				}
			}
			// 
			// ����ڱ��Ϊ3��clip��֮��,��������һ�������ཻtile�ڴ�Chunk
			// ��-Z���������е�Tile.
			if( OSE_BACK == t_pos2 )
			{
				for( int t_i=m_arrTestTile[m_iTestTileNum-1].m_iY-1;
					t_i >= 0;t_i -- )
				{
					push_testTile( m_arrTestTile[m_iTestTileNum-1].m_iX,t_i );
				}
			}

			break;
		}
	}

	return m_iTestTileNum;


	unguard;
}



/** \brief
*  ����chunk�ڱ����߷ָ��tileIdx
*
*  \param _rayStart,_rayDir ���pick���ߵķ���
*  \param _minX,_minZ       Ҫ����Chunk��x,z������Сֵ
*  ���µ��㷨������       
*     
*         Z����
*         |      1/
*   pt[1] -------/----------- pt[2]
*         |  |  /  |        |
*         -----/-------------
*         |  |/ |  |        |
*         ---/---------------
*      0  | /|  |           |  2
*         -/-----------------   
*         /  |  |           |
*        /-------------------
*Ray start|  |  |           | pt[3]
*   pt[0] ------------------------X����
*               3
*
*  һ���Ȱ�����Chunk������������߱��Ϊ0,1,2,3.
*
*  �������ı߱߹����ĸ���ֱ��xzƽ�����,��ķ����������ı��ε��ڲ���
*
*  �������Ray���������Ľ����ڵ�1,3��֮ǰ�����ҳ��˽����Ӧ�ĸ���������Ȼ��
*      ���ô˸���Ϊ���ܸ������ཻ�ĸ��ӡ�
*      ������Ϊ������ұ߽����ŵ�Chunk�ڵ�һ���߹���һ����ֱ��xzƽ����棬Ȼ��
*      �ҳ�������ߵĽ������,������Ӧ�Ŀ��ܸ������ཻ�ĸ��ӡ�
*      ֱ�����·�������������1��3֮�⣬���ߵ����˱��Ϊ2���档
*  
*  �ģ��������������������Ե�3������д����ҵ���ȷ�Ľ���,��Ϊ���һ�����߸�
*      һ��Chunk��أ���ô����������������ཻ������ֻ��Ҫ�������͵�1,3���߾�ok�ˡ�
*      
*      �Ե�������Ĵ�������Ҫ�ҵ����߸���������Ľ�����Chunk�ڵĸ���������Xֵ��
*      Ȼ��Ӵ�Xֵ��ʼ�������������Դ�ֵ�ұߵ�Chunk�ڱ߹����棬����
*    
*/
int osc_fdChunkPick::get_pickTile( osVec3D&  _rayStart,
					  osVec3D& _rayDir,float _minX,float _minZ )
{
	guard;

	osVec3D      t_vec3IntsetPt;

	osassert( _minX >= 0.0f );
	osassert( _minZ >= 0.0f );

	m_iTestTileNum = 0;

	//
	m_vec3RayStart = _rayStart;
	m_vec3RayEnd = _rayStart + 100000.0f*_rayDir;
	construct_clipPlanes( _minX,_minZ );

	if( osPlaneIntersectLine( &t_vec3IntsetPt,
		&m_arrPlane[0],&m_vec3RayStart,&m_vec3RayEnd ) )
	{
		if((OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[1],t_vec3IntsetPt))&&
		  (OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[3],t_vec3IntsetPt)) )
		{
			m_arrTestTile[m_iTestTileNum].m_iX = 0;
			m_arrTestTile[m_iTestTileNum].m_iY = int((t_vec3IntsetPt.z-_minZ)/TILE_WIDTH );
			m_iTestTileNum ++;

			find_testTileArr( _minZ,1 );

			return m_iTestTileNum;
		}
	}

	//
	// �͵�3�����ཻ����
	if( osPlaneIntersectLine( &t_vec3IntsetPt,
		&m_arrPlane[3],&m_vec3RayStart,&m_vec3RayEnd ) )
	{
		if( (OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[0],t_vec3IntsetPt))&&
			(OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[2],t_vec3IntsetPt)) )
		{
			m_arrTestTile[m_iTestTileNum].m_iX = 
				int((t_vec3IntsetPt.x - _minX)/TILE_WIDTH);
			m_arrTestTile[m_iTestTileNum].m_iY = 0;
			m_iTestTileNum ++;

			find_testTileArr( _minZ,m_arrTestTile[m_iTestTileNum-1].m_iX+1 );

			// 
			// ��m_iTestTileNum==1ʱ���п��������µ����ε��£������������£�
			// find_testTileArr����������ȷ�Ĵ���������ཻ�ĸ����б������Ƶ���һ������
			// ��һ������ʱ���ᴦ���������߸����Ϊ����clip���ཻ������
			/*
			* 
			*         Z����
			*         |      1
			*   pt[1] -------\------------ pt[2]
			*         |  |  | \|  |  |   |
			*         ---------\----------
			*         |  |  |  |\ |  |   |
			*         -----------\--------
			*      0  |  |  |  |  \  |   |  2
			*         -------------\------   
			*         |  |  |  |  | \|   |
			*         ---------------\----
			*         |  |  |  |  |  |\  | pt[3]
			*   pt[0] -----------------\-------X����
			*               3         Ray start
			*  
			*/
			if( m_iTestTileNum > 1 )
				return m_iTestTileNum;
		}

	}


	// �͵�1�����ཻ�Ĵ���
	if( osPlaneIntersectLine( &t_vec3IntsetPt,
		&m_arrPlane[1],&m_vec3RayStart,&m_vec3RayEnd ) )
	{
		if((OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[0],t_vec3IntsetPt))&&
		   (OSE_BACK!=osn_mathFunc::classify_PointByPlane(m_arrPlane[2],t_vec3IntsetPt)) )
		{
			m_arrTestTile[m_iTestTileNum].m_iX = 
				int((t_vec3IntsetPt.x - _minX)/TILE_WIDTH);
			m_arrTestTile[m_iTestTileNum].m_iY = 15;
			m_iTestTileNum ++;

			find_testTileArr( _minZ,m_arrTestTile[m_iTestTileNum-1].m_iX+1 );
	
			return m_iTestTileNum;
		}
	}

	return m_iTestTileNum;

	unguard;
}




