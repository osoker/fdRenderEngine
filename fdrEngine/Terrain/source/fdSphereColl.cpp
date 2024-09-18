//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdSphereColl.cpp
 *
 *  His:      River created @ 2006-7-4
 *
 *  Des:      ��һ�������ڵ�bounding Sphere������ײ���.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"



//! ���ʹ��10*10�����ӵ�bsphere. 
//! River @ 2010-7-8:��10�޸�Ϊ13,��������������о���δ���
# define MAX_BSTILENUM    13

//!
# define MAX_TILEIDXSUM   64

//! Ϊ��ʡ���ҵر�߶���Ϣ�Ĺ�����
struct os_tgHeightInfo
{
	const float*  m_ptrHeightInfo;
	WORD          m_wXIndex;
	WORD          m_wZIndex;
};

/** \brief
 *  ��tgMgr��������ײ�������й������
 */
class osc_tgSphereColl
{
private:
	osc_TGManager*    m_ptrTGMgr;

	//! bsphereӰ��ĵر����
	os_tileIdx        m_sTileIdxMin;
	os_tileIdx        m_sTileIdxMax;

	//! ��Ӱ��ĸ�����x��z�����ϵ���Ŀ
	int               m_iTileNumInXZDir;

	//! bsphereӰ���tileGround.
	os_tileIdx        m_sTgIdxMin;
	os_tileIdx        m_sTgIdxMax;

	//! bsphereӰ��ĵر���ӵĸ߶����ݼ���
	osVec3D*          m_vec3TilePos;
	float             m_fMinHeight;
	float             m_fMaxHeight;

	//! ��ǰbsphereӰ��ĵر���������ɵ�aabbox.
	os_aabbox         m_sTileAABBox;

	//! ������ĸ�tg�ܵ�Ӱ��
	os_tgHeightInfo   m_arrHeightPtr[4];

	//! 
	osVec2D           m_vec3NegaXzDir;
	osVec2D           m_vec3PosiXzDir;

	//! ��ǰ��ײ��Ϣ�õ��������ε���������
	VEC_word          m_vecTriIdx;
	int               m_iTriangleNum;
	//! ÿһ�������ζ�Ӧ����
	osPlane*          m_sTriPlane;

	//! ��sphere��ײ�������ε�����
	VEC_word          m_vecCollTriIdx;
	int               m_iCollTriNum;

private:
	//! �õ�ĳһ��tg��ָ��
	const float*      find_height( WORD _x,WORD _z )
	{
		guard;

		if( (m_arrHeightPtr[0].m_wXIndex == _x)&&
			(m_arrHeightPtr[0].m_wZIndex == _z) )
			return m_arrHeightPtr[0].m_ptrHeightInfo;
		if( (m_arrHeightPtr[1].m_wXIndex == _x)&&
			(m_arrHeightPtr[1].m_wZIndex == _z) )
			return m_arrHeightPtr[1].m_ptrHeightInfo;
		if( (m_arrHeightPtr[2].m_wXIndex == _x)&&
			(m_arrHeightPtr[2].m_wZIndex == _z) )
			return m_arrHeightPtr[2].m_ptrHeightInfo;
		if( (m_arrHeightPtr[3].m_wXIndex == _x)&&
			(m_arrHeightPtr[3].m_wZIndex == _z) )
			return m_arrHeightPtr[3].m_ptrHeightInfo;

		//! River mod @ 2007-3-9:�˴���assertȥ��,��һ�ν��д���
		//osassertex( false,"������!!!���εر���ײ�Ҳ����߶���Ϣ...\n" );

		return NULL;

		unguard;
	}

	void              construct_triangleList( void )
	{
		guard;

		int    t_iPlaneIdx = 0;
		m_iTriangleNum = 0;

		for( WORD t_z = m_sTileIdxMin.m_iY;t_z<=m_sTileIdxMax.m_iY;t_z ++ )
		{
			for( WORD t_x = m_sTileIdxMin.m_iX;t_x<=m_sTileIdxMax.m_iX;t_x ++ )
			{
				int     t_iXIdx = t_x - m_sTileIdxMin.m_iX;
				int     t_iZIdx = t_z - m_sTileIdxMin.m_iY;

				// �����ε�����
				int     t_iTriIdx = m_iTriangleNum*3;
				m_vecTriIdx[t_iTriIdx]   = t_iZIdx*m_iTileNumInXZDir + t_iXIdx;
				m_vecTriIdx[t_iTriIdx+1] = (t_iZIdx+1)*m_iTileNumInXZDir + t_iXIdx;
				m_vecTriIdx[t_iTriIdx+2] = t_iZIdx*m_iTileNumInXZDir + t_iXIdx + 1;

				osPlaneFromPoints( &m_sTriPlane[t_iPlaneIdx],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+1]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+2]]  );

				m_vecTriIdx[t_iTriIdx+3] = t_iZIdx*m_iTileNumInXZDir + t_iXIdx + 1;
				m_vecTriIdx[t_iTriIdx+4] = (t_iZIdx+1)*m_iTileNumInXZDir + t_iXIdx;
				m_vecTriIdx[t_iTriIdx+5] = (t_iZIdx+1)*m_iTileNumInXZDir + t_iXIdx + 1;

				osPlaneFromPoints( &m_sTriPlane[t_iPlaneIdx+1],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+3]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+4]],
					&m_vec3TilePos[m_vecTriIdx[t_iTriIdx+5]]  );

				m_iTriangleNum += 2;
				t_iPlaneIdx += 2;
			}
		}

		unguard;
	}



	//! �õ�������Ӱ����Ӹ߶�ֵ�ĺ���
	bool              get_affectTileHeight( void )
	{
		guard;

		m_fMinHeight = 10000.0f;
		m_fMaxHeight = -10000.0f;

		int   t_idx = 0;
		for( WORD t_z = m_sTgIdxMin.m_iY;t_z<=m_sTgIdxMax.m_iY;t_z ++ )
		{
			for( WORD  t_x = m_sTgIdxMin.m_iX;t_x<=m_sTgIdxMax.m_iX;t_x ++ )
			{
				os_tileIdx   t_sTileIdx;
				t_sTileIdx.m_iX = t_x;
				t_sTileIdx.m_iY = t_z;
 
				// River @ 2010-9-30:���뻺�͵Ŀͻ��˵ĳ�����
				if( (t_x + t_z)>=MAX_TILEIDXSUM )
					return false;

				m_arrHeightPtr[t_idx].m_ptrHeightInfo = 
					m_ptrTGMgr->get_sceneColHeightData( t_sTileIdx );

				// River @ 2007-3-7:��������Ĵ�����ʽ:
				if( NULL == m_arrHeightPtr[t_idx].m_ptrHeightInfo )
					continue;

				m_arrHeightPtr[t_idx].m_wXIndex = t_x;
				m_arrHeightPtr[t_idx].m_wZIndex = t_z;
				t_idx ++;
			}
		}
		for( int t_iNotUsed = t_idx;t_iNotUsed<4;t_iNotUsed ++ )
		{
			m_arrHeightPtr[t_iNotUsed].m_wXIndex = -1;
			m_arrHeightPtr[t_iNotUsed].m_wZIndex = -1;
		}

		//
		// ��ÿһ������,����˸��ӵĸ߶���Ϣ
		for( WORD t_z = m_sTileIdxMin.m_iY;t_z<=(m_sTileIdxMax.m_iY+1);t_z ++ )
		{
			for( WORD t_x = m_sTileIdxMin.m_iX;t_x<=(m_sTileIdxMax.m_iX+1);t_x ++ )
			{
				// �߶���Ϣÿһ��TG��65*65��
				const float* t_ptrHeight = find_height( t_x/64,t_z/64 );

				// River mod @ 2006-9-11:�˴������������ֿյĸ߶���Ϣ����ΪactTglistʹ�õ�����һ֡������
				// WORK START:ATTENTION TO FIX:�˴���һЩ���봦��Ķ����������ָ��ֻҪ���֣�����ض�Ҫ��
				// �����ڴ˴�������Ӻ��õĳ�������ƣ�
				if( !t_ptrHeight )
					continue;

				int   t_iXOffset = t_x%64;
				int   t_iZOffset = t_z%64;
				int   t_iTileHeightIdx = 
					(m_iTileNumInXZDir)*(t_z-m_sTileIdxMin.m_iY) + (t_x-m_sTileIdxMin.m_iX);

				m_vec3TilePos[t_iTileHeightIdx].y = t_ptrHeight[t_iZOffset*65+t_iXOffset];
				m_vec3TilePos[t_iTileHeightIdx].x = t_x*TILE_WIDTH;
				m_vec3TilePos[t_iTileHeightIdx].z = t_z*TILE_WIDTH;

				if( m_vec3TilePos[t_iTileHeightIdx].y < m_fMinHeight )
					m_fMinHeight = m_vec3TilePos[t_iTileHeightIdx].y;

				if( m_vec3TilePos[t_iTileHeightIdx].y > m_fMaxHeight )
					m_fMaxHeight = m_vec3TilePos[t_iTileHeightIdx].y;
			}
		}

		// 
		// River mod 2008-4-17:�õ��������ĸ�����Ϣ
		if( m_fMinHeight > 9999.0f )
		{
			osDebugOut( va( "�����������ݳ���<%f,%f>.tile min<%d,%d>,tile Max<%d,%d>...",m_fMinHeight,m_fMaxHeight,
				m_sTileIdxMin.m_iX,m_sTileIdxMin.m_iY,m_sTileIdxMax.m_iX,m_sTileIdxMax.m_iY  ) );
			return false;
		}
		else
			return true;

		unguard;
	}

public:
	osc_tgSphereColl()
	{
		m_ptrTGMgr = NULL;
	
		// -xz�����xy����ĳ���
		m_vec3NegaXzDir = osVec2D( -0.70710671f,-0.70710671f );
		m_vec3PosiXzDir = osVec2D(  0.70710671f, 0.70710671f );

		m_vec3TilePos = new osVec3D[(MAX_BSTILENUM+1)*(MAX_BSTILENUM+1)];
		m_sTriPlane = new osPlane[2*(MAX_BSTILENUM+1)*(MAX_BSTILENUM+1)];
		
		// 
		m_vecTriIdx.resize( (MAX_BSTILENUM*MAX_BSTILENUM*2)*3 );
		m_vecCollTriIdx.resize( MAX_BSTILENUM*MAX_BSTILENUM );
	}
	~osc_tgSphereColl()
	{
		release();
	}

	void release(void)
	{
		m_vecTriIdx.clear();
		m_vecCollTriIdx.clear();
		SAFE_DELETE_ARRAY( m_vec3TilePos );
		SAFE_DELETE_ARRAY( m_sTriPlane );
	}

	//! ��ʼ��������
	void         init_tgSphereColl( osc_TGManager* _tgMgr )
	{
		guard;

		osassert( _tgMgr );
		m_ptrTGMgr = _tgMgr;
		
		unguard;
	}


	//! �������ǰ��bsphereӰ��ĵر����.
	void         cal_affectTile( os_bsphere* _bs )
	{
		guard;

		// 
		// ����x,z�������Сֵ�����ֵ.ֱ�������� a*a + b*b = c*c
		osVec2D        t_vec2Min,t_vec2Max;
		float          t_fBoundRadius = sqrt( 2*_bs->radius*_bs->radius );


		// River mod @ 2010-7-8:�˴������ȷ�ϴ�ֵ�Ĵ�С��
		osassertex( (t_fBoundRadius/TILE_WIDTH+1)<MAX_BSTILENUM,
			va( "%s<%f>...\n","�����bsphere�����˳������ܴ���Ĵ�С",t_fBoundRadius ) );

		t_vec2Min.x = _bs->veccen.x;
		t_vec2Min.y = _bs->veccen.z;
		t_vec2Max   = t_vec2Min;

		t_vec2Min = t_vec2Min + m_vec3NegaXzDir*t_fBoundRadius;
		t_vec2Max = t_vec2Max + m_vec3PosiXzDir*t_fBoundRadius;
	
		m_sTileIdxMin.m_iX = int( t_vec2Min.x/TILE_WIDTH );
		m_sTileIdxMin.m_iY = int( t_vec2Min.y/TILE_WIDTH );
		m_sTileIdxMax.m_iX = int( t_vec2Max.x/TILE_WIDTH );
		m_sTileIdxMax.m_iY = int( t_vec2Max.y/TILE_WIDTH );

		m_iTileNumInXZDir = m_sTileIdxMax.m_iX + 2 - m_sTileIdxMin.m_iX;

		m_sTgIdxMin.m_iX = m_sTileIdxMin.m_iX/64;
		m_sTgIdxMin.m_iY = m_sTileIdxMin.m_iY/64;
		m_sTgIdxMax.m_iX = (m_sTileIdxMax.m_iX+1)/64;
		m_sTgIdxMax.m_iY = (m_sTileIdxMax.m_iY+1)/64;

		osassert( (m_sTgIdxMax.m_iX-m_sTgIdxMin.m_iX)<2 );
		osassert( (m_sTgIdxMax.m_iY-m_sTgIdxMin.m_iY)<2 );

		return;

		unguard;
	}



	//! �������Щ�ر���ӵ�AABBox
	os_aabbox*   cal_etileAABBox( void )
	{
		//! ��Ҫ�õ�������Ӱ����ӵ���߸߶Ⱥ���͸߶�
		// River mod @ 2008-4-17:�õ����������������� 
		if( !get_affectTileHeight() )
			return NULL;

		//
		m_sTileAABBox.m_vecMax.x = m_sTileIdxMax.m_iX*TILE_WIDTH+TILE_WIDTH;
		m_sTileAABBox.m_vecMax.z = m_sTileIdxMax.m_iY*TILE_WIDTH+TILE_WIDTH;
		m_sTileAABBox.m_vecMax.y = m_fMaxHeight;

		m_sTileAABBox.m_vecMin.x = m_sTileIdxMin.m_iX*TILE_WIDTH;
		m_sTileAABBox.m_vecMin.z = m_sTileIdxMin.m_iY*TILE_WIDTH;
		m_sTileAABBox.m_vecMin.y = m_fMinHeight;

		return &m_sTileAABBox;
	}

	//! �������һ��������Ƭ����bsphere�ཻ,�õ��������б�
	int         cal_intersectTriList( os_bsphere* _bs )
	{
		//! ����ÿһ���߶���Ϣ�������������б�
		construct_triangleList();


		// �����ཻ
		m_iCollTriNum = 0;
		for( int t_i=0;t_i<m_iTriangleNum;t_i ++ )
		{
			if( _bs->collision( &m_vec3TilePos[m_vecTriIdx[t_i*3]],
				&m_vec3TilePos[m_vecTriIdx[t_i*3+1]],
				&m_vec3TilePos[m_vecTriIdx[t_i*3+2]],&m_sTriPlane[t_i] ) )
			{
				m_vecCollTriIdx[m_iCollTriNum] = t_i;
				m_iCollTriNum ++;
			}
		}

		return m_iCollTriNum;
	}

	//!  ��ÿһ��TriList�ڵ�������,�����bsphere��posVector���ƶ���Զ����ʹ��bsphere����
	//!  ����������ײ,�õ�һ�������б�.������������ֵ.
	float        cal_passDistance( os_bsphere* _bs,osVec3D& _moveDir )
	{

		return 0.0f;
	}

};



//! ֻ��Ҫ������ļ���ʹ��
static osc_tgSphereColl   t_sTgSC;

/** \brief
*  ����һ����Χ����ر����ײ���.
*
*  �㷨�������:
*  ÿһ�μ���bsphere����ײʱ,��Ҫ������һ��û����ײ��bsphere���ڵ�λ��.�����ǰ��bsphere��
*  �ر�֮������ײ,���ҵ���bsphere����ܵ��ﵱǰ��λ��,����û�и��ر���ײ��λ��.
*  
*  �㷨����:
*  1: �ȼ������ǰ��bsphereӰ��ĵر����.
*  2: �������Щ�ر���ӵ�AABBox.
*  3: �����AABBox��bsphere����ײ,�򷵻� false
*  4: �������һ��������Ƭ����bsphere�ཻ,�õ��������б�.TriList
*  5: �����bsphere��ǰλ�ø���һ֡û����ײ�ر��λ�õ�����. posVector
*  6: ��ÿһ��TriList�ڵ�������,�����bsphere��posVector���ƶ���Զ����ʹ��bsphere����
*     ����������ײ,�õ�һ�������б� distanceList
*  7: ��distanceList��,�ҳ���Զ�ľ���,ʹ��posVector��������λ��,��ֵ��_resPos,����true.
*   
*  \return bool �����ײ,����true,���򷵻�false.
*  \param  os_bsphere* _bs  Ҫ�����ε���ײ��Ϣ����ײ����bounding sphere
*/
bool osc_TGManager::get_sphereInterTerrPos( os_bsphere* _bs )
{
	guard;

	os_aabbox*    t_sAABBox = NULL;

	t_sTgSC.init_tgSphereColl( this );

	// River mod @ 20109-9-18��ȷ�ϲ���ײ����.
	if( _bs->radius > 10000.0 )
		return true;

	t_sTgSC.cal_affectTile( _bs );
	t_sAABBox = t_sTgSC.cal_etileAABBox();

	// River mod @ 2008-4-17:�������������������
	if( !t_sAABBox )
	{
		// ������???�õ�sphere��λ�ú�������ݣ�
		osDebugOut( "The ERROR sphere is:<%f,%f,%f,%f>....\n",_bs->veccen.x,
			_bs->veccen.y,_bs->veccen.z,_bs->radius );
		//osassertex( false,"�ڴ�λ���ϣ��������!!!!\n" );

		// River @ 2010-8-21:���ٳ���ֱ�ӷ��ز�����
		return false;
	}

	// �����aabbox���ཻ,ֱ�ӷ���false.
	if( !t_sAABBox->collision( *_bs ) )
		return false;


	// ���ʵ����û���������ཻ,���ز���ײ.
	if( 0 == t_sTgSC.cal_intersectTriList( _bs ) )
		return false;

	return true;

	unguard;
}

//
//! tzz:
//		�����osc_tgSphereColl �������������ͷ�osc_tgSphereColl�ڲ�����Դ�Ļ���
//		�ͻᵼ�� MFC ������Ŀ��ʱ��Զѵ��ڴ������⣬��Ϊ�ڴ��Ѿ�й©�ˣ���ʵ��û��
//		���Ծͽ�osc_TGManager::ReleaseStatic���壬��osc_engine::release�е��á�
void osc_TGManager::ReleaseStatic(void)
{
	guard;
	t_sTgSC.release();	
	unguard;
}



/** \brief
*  ����һ���������õ��Ƿ���ر��ཻ���Լ��ཻ���λ�á�
*  
*  ʹ�ø�����ѡ�ر�һ���ĺ����㷨��
*  \param osVec3D& _rayStart  ���ߵĿ�ʼλ�á�
*  \param osVec3D& _rayDir    ���ߵķ���
*  \param osVec3D& _resPos    ����ཻ����ֵΪ���صĽ���λ�á�
*  
*/
bool osc_TGManager::get_rayInterTerrPos( osVec3D& _rayStart,osVec3D& _rayDir,osVec3D& _resPos )
{
	guard;

	osVec3D    t_vec3RayDir = _rayDir,t_vec3RayEnd;
	osVec3D    t_vec3Tmp;
	osPlane         t_pickPlane;
	ose_geopos       t_arrPPClip[4];
	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );	
	os_Ray     pickRay(_rayStart,t_vec3RayDir );


	// �ȵõ���ǰ���߶����ҷ�������,���ڹ������߷����ƽ��
	t_vec3Tmp = osVec3D( 0.0f,1.0f,0.0f );
	osVec3Cross( &t_vec3Tmp,&t_vec3Tmp,&t_vec3RayDir );
	t_vec3Tmp = _rayStart + t_vec3Tmp;

	// 
	// ����һ�����߷����ƽ�棬���һ���ر���ӵ��ĸ�����ȫ��ƽ���һ�⣬��
	// ������ӿ϶���������������ཻ����Ϊ�ر������͹���塣
	t_vec3RayEnd = _rayStart + t_vec3RayDir;
	osPlaneFromPoints( &t_pickPlane,&_rayStart,&t_vec3Tmp,&t_vec3RayEnd );

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// ���Ĳ���Ϊ��׼�Ĳ���:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// �ӵ�ǰTileGround��QuadTree�õ��ཻ��Chunk�б�
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       _rayStart,t_vec3RayDir,t_ptrChunkIdx,t_ptrChunkXZ );
		if( 0 == t_iPickChunkNum )
			continue;

		// ��ÿһ��ray�ཻ��Chunk���и�ϸ�µĴ���
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// ��ʼ���ҿ����ཻ��tile����	�б�
			int t_tileNum = m_sChunkPick.get_pickTile(
				_rayStart,t_vec3RayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// ��ÿһ��tile���Ӳ鿴�Ƿ��ཻ
			const os_tileIdx*   t_ptrIdx = m_sChunkPick.get_testTileArr();
			int   t_iBeginx = t_ptrChunkIdx[t_i].m_iX*16;
			int   t_iBeginy = t_ptrChunkIdx[t_i].m_iY*16;
			int   t_iTx,t_iTz;

			for( int t_i=0;t_i<t_tileNum;t_i ++ )
			{
				//! �������64���򲻴����������
				t_iTx = t_iBeginx + t_ptrIdx[t_i].m_iX;
				t_iTz = t_iBeginy + t_ptrIdx[t_i].m_iY;

				if( (t_iTx >= MAX_GROUNDWIDTH) ||
					(t_iTz >= MAX_GROUNDWIDTH) )
					continue;

				TilePtr->get_polyVertex( t_poly,t_iTx,t_iTz );


				// 
				// ��������ļ��,����͹���ԭ��,�˴����Թ��˵������ı���
				t_arrPPClip[0] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[0] );
				t_arrPPClip[1] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[1] );
				t_arrPPClip[2] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[2] );
				t_arrPPClip[3] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[3] );
				if( (t_arrPPClip[0] == t_arrPPClip[1])&&
					(t_arrPPClip[1] == t_arrPPClip[2])&&
					(t_arrPPClip[2] == t_arrPPClip[3]) )
					continue;

				// �������μ�����ཻ����
				if( pick_nearestTri( t_poly,pickRay,
					_resPos,_rayStart,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	return t_hr;

	unguard;
}


/** \brief
*  ����һ���߶�,��ȷ�ϴ��߶��Ƿ���ر��ཻ,�Լ������λ��.
*
*  ���������Ҫ�����Ż�������ر����ײ����,�����㷨���Ժ���get_rayInterTerrPos
*
*  \param osVec3D& _ls �߶εĿ�ʼλ��.
*  \param osVec3D& _le �߶εĽ���λ��.
*  \param osVec3D& _resPos ����ཻ,�����ཻ�Ľ���λ��.
*  \return bool            ����true,����ر��ཻ.
*/
bool osc_TGManager::get_lineSegInterTerrPos( osVec3D& _ls,osVec3D& _le,osVec3D& _resPos )
{
	guard;


	osVec3D    t_vec3RayDir,t_vec3RayEnd;
	osVec3D    t_vec3Tmp;
	osPlane         t_pickPlane;
	ose_geopos       t_arrPPClip[4];
	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	t_vec3RayDir = _le -_ls;
	osVec3Normalize( &t_vec3RayDir,&t_vec3RayDir );	
	os_Ray     pickRay( _ls,t_vec3RayDir );


	// �ȵõ���ǰ���߶����ҷ�������,���ڹ������߷����ƽ��
	t_vec3Tmp = osVec3D( 0.0f,1.0f,0.0f );
	osVec3Cross( &t_vec3Tmp,&t_vec3Tmp,&t_vec3RayDir );
	t_vec3Tmp = _ls + t_vec3Tmp;

	// 
	// ����һ�����߷����ƽ�棬���һ���ر���ӵ��ĸ�����ȫ��ƽ���һ�⣬��
	// ������ӿ϶���������������ཻ����Ϊ�ر������͹���塣
	t_vec3RayEnd = _ls + t_vec3RayDir;
	
	// River @ 2006-9-9:���㷨���_ls��_leͬ��Y���ϣ���������⡣
	osassert( _ls != t_vec3Tmp );

	osPlaneFromPoints( &t_pickPlane,&_ls,&t_vec3Tmp,&t_vec3RayEnd );

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// ���Ĳ���Ϊ��׼�Ĳ���:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// �ӵ�ǰTileGround��QuadTree�õ��ཻ��Chunk�б�
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       _ls,t_vec3RayDir,t_ptrChunkIdx,t_ptrChunkXZ, &_le );
		if( 0 == t_iPickChunkNum )
			continue;

		// ��ÿһ��ray�ཻ��Chunk���и�ϸ�µĴ���
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// ��ʼ���ҿ����ཻ��tile����	�б�
			int t_tileNum = m_sChunkPick.get_pickTile(
				_ls,t_vec3RayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// ��ÿһ��tile���Ӳ鿴�Ƿ��ཻ
			const os_tileIdx*   t_ptrIdx = m_sChunkPick.get_testTileArr();
			int   t_iBeginx = t_ptrChunkIdx[t_i].m_iX*16;
			int   t_iBeginy = t_ptrChunkIdx[t_i].m_iY*16;
			int   t_iTx,t_iTz;

			for( int t_i=0;t_i<t_tileNum;t_i ++ )
			{
				//! �������64���򲻴����������
				t_iTx = t_iBeginx + t_ptrIdx[t_i].m_iX;
				t_iTz = t_iBeginy + t_ptrIdx[t_i].m_iY;

				if( (t_iTx >= MAX_GROUNDWIDTH) ||
					(t_iTz >= MAX_GROUNDWIDTH) )
					continue;

				TilePtr->get_polyVertex( t_poly,t_iTx,t_iTz );


				// 
				// ��������ļ��,����͹���ԭ��,�˴����Թ��˵������ı���
				t_arrPPClip[0] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[0] );
				t_arrPPClip[1] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[1] );
				t_arrPPClip[2] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[2] );
				t_arrPPClip[3] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[3] );
				if( (t_arrPPClip[0] == t_arrPPClip[1])&&
					(t_arrPPClip[1] == t_arrPPClip[2])&&
					(t_arrPPClip[2] == t_arrPPClip[3]) )
					continue;

				// �������μ�����ཻ����
				if( pick_nearestTri( t_poly,pickRay,
					_resPos,_ls,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	return t_hr;

	unguard;
}

