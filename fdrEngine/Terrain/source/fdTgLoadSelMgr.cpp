//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTgLoadSelMgr.cpp
 *
 *  His:      River created @ 2005-5-18
 *
 *  Des:      ��fdMgr�ڵ�TileGround���й���,�õ��ĸ���Ҫ�û����ڴ�,��һ��Ӧ�õ��뵽�ڴ�.
 *            �ڳ��������е�TG�����뵽�ڴ�������,���ļ��к����������ĸ�tgӦ���Ǽ����
 *            ��Ҫ�����tg ,��Щ�����ڴ��еȴ������tg .
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"


# include "../../backPipe/include/osCamera.h"

# include  "../../mfpipe/include/osMesh.h"

# include "../include/fdNewFieldStruct.h"


//! ȫ�ֵĳ����������
int    g_iLoadProgress = 0;

osc_tgLoadReleaseMgr::osc_tgLoadReleaseMgr()
{
	
	m_ptrMidPipe = NULL; 
	m_ptrRealTg = NULL;
	m_iCurTgNum = 0;

	memset( m_ptrActTg,NULL,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );

	m_bTLoadInMem = false;

}


// 
osc_tgLoadReleaseMgr::~osc_tgLoadReleaseMgr()
{

	SAFE_DELETE_ARRAY( m_ptrRealTg );
	m_sThreadMgr.end_thread();
	m_sThreadMgr.End();

}


//  �õ���ͼ��������ʼ����λ����Ϣ.
void osc_tgLoadReleaseMgr::init_insertInfo( os_sceneLoadStruct* _sl )
{
	guard;

	osassert( _sl );
	osassert( _sl->m_iX>=0 );
	osassert( _sl->m_iY >= 0 );

	// 
	// ���ڵ�ͼ��.
	m_sLFCamInIdx.m_iX = _sl->m_iX / GROUNDMASK_WIDTH;
	m_sLFCamInIdx.m_iY = _sl->m_iY / GROUNDMASK_WIDTH;

	// ͼ���ڵ�λ��.
	osc_newTileGround::frame_moveCamInTgPos( _sl->m_iX,_sl->m_iY ); 
	m_eLFTgPos = osc_newTileGround::get_camInTgPos();

	unguard;
}



//! ������ϲ���õĵ�ͼ�б�,������ĵ�ͼ�б�.
void osc_tgLoadReleaseMgr::get_activeTgList( void )
{
	guard;

	memset( m_ptrActTg,0,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );

	int    t_idx = 0;

	// ����ǰ�ȫ���ĵ�ͼ�����뵽�ڴ�
	if( m_bTLoadInMem )
		osassert( false );

	// 
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		if( m_sThreadMgr.is_activeTg( t_i ) )
			m_ptrActTg[t_idx++] = &m_ptrRealTg[t_i];
	}

	unguard;
}


//@{


//! �õ���ǰtg��ΧһȦ��tg,��8������Щtg�϶���Ҫ���뵽�ڴ档
void osc_tgLoadReleaseMgr::get_surroundTG( os_tileIdx* _mapIdx,int _my,int _mx )
{
	guard;


	int    t_iNameIdx = 1;

	osassert( _mapIdx );

	for( int t_y=_my-1;t_y<=_my+1;t_y ++ )
	{
		for( int t_x=_mx-1;t_x<=_mx+1;t_x ++ )
		{
			if( (t_y<0)||(t_y>=m_iMaxZTgNum) )
			{
				// River @ 2009-5-6: û�б�����-1,ֱ�ӽ�����һ��.
				//_mapIdx[t_iNameIdx].m_iX = -1,_mapIdx[t_iNameIdx].m_iY = -1;			
				//t_iNameIdx ++;
				continue;
			}
			if( (t_x<0)||(t_x>=m_iMaxXTgNum) )
			{
				// River @ 2009-5-6: û�б�����-1,ֱ�ӽ�����һ��.
				//_mapIdx[t_iNameIdx].m_iX = -1,_mapIdx[t_iNameIdx].m_iY = -1;		
				//t_iNameIdx ++;
				continue;
			}

			//! �Ѿ���ͷһ��λ�÷����˴˵�ͼ���ļ���
			if( (t_x == _mx)&&(t_y == _my) )
				continue;

			_mapIdx[t_iNameIdx].m_iX = t_x;
			_mapIdx[t_iNameIdx].m_iY = t_y;

			t_iNameIdx ++;

			// River added @ 2009-6-5:���ڵͶ�Ӳ���Ĵ���
			if( t_iNameIdx >= MAXTG_INTGMGRMACRO )
				break;
		}
	}

	return;

	unguard;
}


//  
//! ���ݵ�ǰ��tg�е�λ��,�����뵱ǰtg�ڶ�Ȧ������.
void osc_tgLoadReleaseMgr::get_secSurTG( os_tileIdx* _mapIdx,int _my,int _mx )
{
	guard;


	osassert( _mapIdx );
	osassert( osc_newTileGround::m_seCamTgPos != OSN_TGPOS_ERROR );

	os_tileIdx        t_idx[MAXTG_INTGMGRMACRO];
	int              t_iNum;


	t_iNum = round_float( sqrt( float(MAXTG_INTGMGR) ) );

	switch( osc_newTileGround::m_seCamTgPos )
	{
	case OSN_TGPOS_TOPLEFT:
		t_idx[0].m_iX = _mx- (t_iNum-2);t_idx[0].m_iY = _my + (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X��ͬ��
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY - t_i;

			//! Y��ͬ
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX + t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	case OSN_TGPOS_TOPRIGHT:
		t_idx[0].m_iX = _mx + (t_iNum-2);t_idx[0].m_iY = _my + (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X��ͬ��
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY - t_i;

			//! Y��ͬ
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX - t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	case OSN_TGPOS_BOTLEFT:
		t_idx[0].m_iX = _mx - (t_iNum-2);t_idx[0].m_iY = _my - (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X��ͬ��
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY + t_i;

			//! Y��ͬ
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX + t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	case OSN_TGPOS_BOTRIGHT:
		t_idx[0].m_iX = _mx + (t_iNum-2);t_idx[0].m_iY = _my - (t_iNum - 2);
		for( int t_i=1;t_i<t_iNum;t_i ++ )
		{
			//! X��ͬ��
			t_idx[t_i].m_iX = t_idx[0].m_iX;
			t_idx[t_i].m_iY = t_idx[0].m_iY + t_i;

			//! Y��ͬ
			t_idx[t_i+(t_iNum-1)].m_iX = t_idx[0].m_iX - t_i;
			t_idx[t_i+(t_iNum-1)].m_iY = t_idx[0].m_iY;
		}
		break;
	}

	// ����ڶ�Ȧ��TileGround
	int        t_iBaseIdx = (t_iNum-1)*(t_iNum-1);
	os_tileIdx* t_ptrIdx;
	for( int t_i=t_iBaseIdx;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrIdx = &t_idx[t_i-t_iBaseIdx];

		if( (t_ptrIdx->m_iX <0)||(t_ptrIdx->m_iX>=m_iMaxXTgNum) )
		{
			_mapIdx[t_i].m_iX = -1,_mapIdx[t_i].m_iY = -1;
			continue;
		}

		if( (t_ptrIdx->m_iY<0)||(t_ptrIdx->m_iY>=m_iMaxZTgNum) )
		{
			_mapIdx[t_i].m_iX = -1,_mapIdx[t_i].m_iY = -1;
			continue;
		}

		_mapIdx[t_i].m_iX = t_ptrIdx->m_iX;
		_mapIdx[t_i].m_iY = t_ptrIdx->m_iY;
	}


	return;

	unguard;
}


//@}

/** \brief
*  �ӿͻ������ĳ�������ṹ�е�����Ҫ����ĳ����ļ�����
*
*  \param  _nameArr �����4�����ȵ�s_string�ִ���������ྲ̬
*                   ����4�ŵ�ͼ������������з�������Ҫ����ĵ�ͼ����
*                   �����ͼ��Ϊ�գ�����Ҫ���������ͼ��
*  
* 
*/
bool osc_tgLoadReleaseMgr::get_loadSceneName( 
			s_string* _nameArr,os_sceneLoadStruct* _sl )
{
	guard;

	int      t_iX,t_iY;
	int      t_iMX,t_iMY;
	char     t_str[32];
	s_string  t_mapName;

	osassert( _nameArr );
	osassert( _sl );
	osassert( _sl->m_szMapName[0] );


	t_iX = _sl->m_iX/2;
	t_iY = _sl->m_iY/2;

	//
	// ���λ�ô��ڵ�ͼ�Ŀ�Ⱥ͸߶ȡ�
	if( (t_iX>m_iMaxX)||(t_iY>m_iMaxZ) )
	{
		osDebugOut( 
			"λ�ô��ڵ�ͼ�Ŀ�Ⱥ͸߶�:t_iX:<%d>,m_iMaxX:<%d>, t_iY:<%d>,m_iMaxZ:<%d>\n",
			t_iX,m_iMaxX,t_iY,m_iMaxZ  );
		osassert( false );
		return false;
	}

	// 
	// �õ���ǰ������ڵĵ�ͼ��
	t_iMX = t_iX/MAX_GROUNDWIDTH;
	t_iMY = t_iY/MAX_GROUNDWIDTH;

	_nameArr[0] = SCENEMAP_DIR;
	_nameArr[0] += _sl->m_szMapName;
	_nameArr[0] += "\\";
	_nameArr[0] += _sl->m_szMapName;
	t_mapName = _nameArr[0];

	// catch this name.
	m_strMapName = t_mapName;


	osc_newTileGround::frame_moveCamInTgPos( _sl->m_iX,_sl->m_iY );

	// River added @ 2009-5-6:����Ϊ-1;
	for( int t_i=0;t_i<MAXTG_INTGMGRMACRO;t_i ++ )
	{
		m_vecNeedLoadTile[t_i].m_iX = WORD(-1);
		m_vecNeedLoadTile[t_i].m_iY = WORD(-1);

	}

	// ��ǰ���ڵ�λ��,�������.
	m_vecNeedLoadTile[0].m_iX = t_iMX;
	m_vecNeedLoadTile[0].m_iY = t_iMY;
	get_surroundTG( m_vecNeedLoadTile,t_iMY,t_iMX );

	//! ���ݵ�ǰ��tg�е�λ�ã������뵱ǰtg�ڶ�Ȧ������
	// ���ڴ����������,���ݵ�ǰ��tg�е�λ�ã������뵱ǰtg�ڶ�Ȧ������
	if( MAXTG_INTGMGR > MINTG_INTGMGRMACRO )
		get_secSurTG( m_vecNeedLoadTile,t_iMY,t_iMX ); 

	//
	// �����Ǽ����tg�б���,����Ӧ�õ���ĵ�ͼ�����б�.
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		// ���Ϸ��ĵ�ͼ���ڵ�ͼ�ı߽�����
		if( (m_vecNeedLoadTile[t_i].m_iX==WORD(-1)) ||
			(m_vecNeedLoadTile[t_i].m_iY==WORD(-1)) )
		{
			_nameArr[t_i] = "";
			continue;
		}

		_nameArr[t_i] = t_mapName;	

		sprintf( t_str,"%02d%02d.geo",
			m_vecNeedLoadTile[t_i].m_iX,m_vecNeedLoadTile[t_i].m_iY );
		_nameArr[t_i] += t_str;

		osDebugOut( "The <%d> map's name is:<%s>..\n",
			t_i,_nameArr[t_i].c_str() );
	}

	return true;

	unguard;
}



//! 
void osc_tgLoadReleaseMgr::init_tgLRMgr( osc_TGManager* _tgMgr )
{
	guard;

	osassert( _tgMgr );

	m_ptrMidPipe = &_tgMgr->m_middlePipe;

	// ��ʼ���̹߳������С�
	m_sThreadMgr.init_mtLoadMapMgr(  _tgMgr,this );
	m_sThreadMgr.Begin();

	//@{
	// Windy mod @ 2006-1-11 12:11:41
	//Desc: MapTileֻ��ʼ��һ�εĲ���
	MapTile::OneTimeInit( m_ptrMidPipe );
	//@}



	//! �����Ƿ�ȫ���ѵ�ͼ�����ڴ������䲻ͬ�Ĵ洢�ռ䡣��
	if( m_bTLoadInMem )
	{
		osassertex( false,"ȫ�������ͼ" );
	}
	else
	{
		m_ptrRealTg = new osc_newTileGround[MAXTG_INTGMGR];

		m_iCurTgNum = MAXTG_INTGMGR;
	}

	unguard;
}


/** \brief
*  ���������λ�ã��������ǰ֡��Ҫ�����ͼ���б�.
*
*  ÿһ��TG�ֳ����ĸ�������������λ�ø���һ֡��ͬһ��tg�ڣ�����
*  ����һ֡��ͬ����tg�����ڣ�����Ҫ��ͼ�û���
*  
*  \param return bool ����ֵ��ʾ���Ƿ���Ҫ�Ե�ǰ����ʾ��ͼ������û�.
*                     ���������,����Ҫ�Ե�ͼ�û�.
*/
bool osc_tgLoadReleaseMgr::get_tgNeedInMemList( osVec3D& _pos )
{
	guard;


	osVec3D         t_vec3CamPos;
	osn_inTgPos     t_sInTgPos;
	os_tileIdx      t_sCamInTg;
	bool            t_bRes;

	t_vec3CamPos = _pos;


	m_iCamInX = int( t_vec3CamPos.x / HALF_TWIDTH );
	m_iCamInZ = int( t_vec3CamPos.z / HALF_TWIDTH );

	osc_newTileGround::frame_moveCamInTgPos( m_iCamInX,m_iCamInZ );
	t_sInTgPos = osc_newTileGround::get_camInTgPos();

	t_sCamInTg.m_iX = WORD( t_vec3CamPos.x / (MAX_GROUNDWIDTH*TILE_WIDTH) );
	t_sCamInTg.m_iY = WORD( t_vec3CamPos.z / (MAX_GROUNDWIDTH*TILE_WIDTH) );

	// 
	if( (t_sInTgPos == this->m_eLFTgPos)&&
		(t_sCamInTg.m_iX == this->m_sLFCamInIdx.m_iX)&&
		(t_sCamInTg.m_iY == this->m_sLFCamInIdx.m_iY ) )
	{
		t_bRes = false;
	}
	else
	{
		t_bRes = true;

		// 
		// �õ���ǰӦ�����ڴ��д��ڵĵ�ͼ���б�.
		m_vecNeedLoadTile[0].m_iX = t_sCamInTg.m_iX;
		m_vecNeedLoadTile[0].m_iY = t_sCamInTg.m_iY;
		get_surroundTG( m_vecNeedLoadTile,t_sCamInTg.m_iY,t_sCamInTg.m_iX );
		// ���ڴ����������,���ݵ�ǰ��tg�е�λ�ã������뵱ǰtg�ڶ�Ȧ������
		if( MAXTG_INTGMGR > MINTG_INTGMGRMACRO )
			get_secSurTG( m_vecNeedLoadTile,t_sCamInTg.m_iY,t_sCamInTg.m_iX );

		m_eLFTgPos = t_sInTgPos;
		m_sLFCamInIdx.m_iX = t_sCamInTg.m_iX;
		m_sLFCamInIdx.m_iY = t_sCamInTg.m_iY;

	}

	return t_bRes;

	unguard;
}

//! ���������߳����ڵ����ͼ,��ֹ��ǰ�ĵ�ͼ����״̬.
void osc_tgLoadReleaseMgr::stop_threadMapLoading( void )
{
	guard;

	m_sThreadMgr.reset_loadMapMgr();
	return;

	unguard;
}


/** \brief
*  ���ݴ���ĵ�ͼ�����б����������Ҫ�ͷŵĵ�ͼ����
*
*  �˺������ڶ��߳�״̬�µ����������ƶ�.
*/
void osc_tgLoadReleaseMgr::spacemove_swapOutMap( s_string* _mapList )
{
	guard;

	osc_TileGroundPtr      t_ptrTg;
	int                   t_i,t_j;


	osassert( _mapList );
	// River 2005-10-22:
	// ���Գ���û��ʹ�õĵ�ͼ����Ŀǰ���ܴ��ڵ�ͼ�����״̬
	osassert( m_sThreadMgr.is_freeStatus() );

	m_iNeedRelTgNum = 0;

	// û��ʹ�õĵ�ͼ��Ϊ�����ȵĵ���ȼ����ڶ�����ǰ��
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];

		if( !t_ptrTg->is_tgInuse() )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}
	}

	// 
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];
		if( !t_ptrTg->is_tgInuse() )
			continue;

		for( t_j = 0;t_j<MAXTG_INTGMGR;t_j ++ )
		{
			if( t_ptrTg->m_strMapFName == _mapList[t_j] )
			{
				_mapList[t_j] = "";
				break;
			}
		}
		// ��Ҫ����������б��У������ڵ�ǰ�ĵ�ͼ����ǰ�ĵ�ͼ�����ͷ�
		if( t_j == MAXTG_INTGMGR )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}	
	}

	return;

	unguard;
}


//! ��Ҫ����ĵ�ͼ�ҵ��ռ䣬Ȼ���ƽ���������߳��У����д���
void osc_tgLoadReleaseMgr::process_needSwapOutTgList( void )
{
	guard;

	osc_TileGroundPtr      t_ptrTg;
	WORD                  t_x,t_y;
	int                   t_i,t_j;



	// River 2005-10-22:
	// ���Գ���û��ʹ�õĵ�ͼ����Ŀǰ���ܴ��ڵ�ͼ�����״̬
	osassert( m_sThreadMgr.is_freeStatus() );

	// û��ʹ�õĵ�ͼ��Ϊ�����ȵĵ���ȼ����ڶ�����ǰ��
	m_iNeedRelTgNum = 0;
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];

		if( !t_ptrTg->is_tgInuse() )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}
	}


	// 
	// Ȼ���Ҳ���Ҫ���µĵ�ͼ���б��ڳ��ֵĵ�ͼ��
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_ptrTg = &m_ptrRealTg[t_i];

		if( !t_ptrTg->is_tgInuse() )
			continue;

		t_x = (WORD)t_ptrTg->m_sTgHeader.m_wMapPosX;
		t_y = (WORD)t_ptrTg->m_sTgHeader.m_wMapPosZ;

		// 
		// �����tg�� <<<�ڴ���Ӧ�ô��ڵ�>>> ͼ���б��в�����,���ע��ͼ��ΪҪ�ͷŵ�ͼ��.
		for( t_j = 0;t_j<MAXTG_INTGMGR;t_j ++ )
		{
			if( (t_x == m_vecNeedLoadTile[t_j].m_iX)&&
				(t_y == m_vecNeedLoadTile[t_j].m_iY) )
				break;
		}

		if( t_j == MAXTG_INTGMGR )
		{
			m_vecTgNeedRelease[m_iNeedRelTgNum] = t_i;
			m_iNeedRelTgNum ++;
		}	
	}

	return;

	unguard;
}

//! ������Ҫ���ڴ��д��ڣ�Ŀǰ���ڴ����ֲ����ڵ�ͼ���б� 
void osc_tgLoadReleaseMgr::process_needSwapInTgList( void )
{
	guard;

	osc_TileGroundPtr      t_ptrTg;
	WORD                  t_x,t_y;
	int                   t_i,t_j;

	m_iNeedSwapInNum = 0;

	// 
	for( t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		t_x = m_vecNeedLoadTile[t_i].m_iX;
		t_y = m_vecNeedLoadTile[t_i].m_iY;

		// 
		// �����tg��<�ڴ���Ӧ�ô��ڵ�>ͼ���б��в�����,���ע��ͼ��ΪҪ���뵽�ڴ��еĵ�ͼ��.
		for( t_j = 0;t_j<MAXTG_INTGMGR;t_j ++ )
		{
			t_ptrTg = &m_ptrRealTg[t_j];

			// 
			// River @ 2005-10-31: ����˵�ͼû�����ã��������һ�ֵĴ���
			if( !t_ptrTg->m_bInuse )
				continue;

			if( (t_ptrTg->m_sTgHeader.m_wMapPosX == t_x)&&
				(t_ptrTg->m_sTgHeader.m_wMapPosZ == t_y) )
				break;
		}

		// ��Ŀǰ���ڴ�tg�б���û�����<<<�ڴ���Ӧ�ô��ڵ�>>>��ͼ�顣
		if( t_j == MAXTG_INTGMGR )
		{
			// River mod @ 2007-9-1:ȷ�ϲ����벻���ڵĵ�ͼ��
			if( (t_x>=m_iMaxXTgNum)||(t_y>=m_iMaxZTgNum) )
				continue;

			m_vecTgNeedSwapIn[m_iNeedSwapInNum].m_iX = t_x;
			m_vecTgNeedSwapIn[m_iNeedSwapInNum].m_iY = t_y;
			m_iNeedSwapInNum ++;
		}	
	}

	return;

	unguard;
}


//! ��Ӧ�ô������ڴ�,�ֲ����ڴ��tg�ƽ���������߳�,����.
bool osc_tgLoadReleaseMgr::push_needSwapInMap( void )
{
	guard;

	int          t_i;
	int          t_x,t_y;
	s_string      t_strMapname;
	char          t_sz[16];

	for( t_i=0;t_i<m_iNeedSwapInNum;t_i ++ )
	{
		if( (m_vecTgNeedSwapIn[t_i].m_iX == (WORD)-1)||
			(m_vecTgNeedSwapIn[t_i].m_iY == (WORD)-1) )
			continue;


		osDebugOut( "��ͼ�û�����...\n" );

		t_x = m_vecTgNeedSwapIn[t_i].m_iX;
		t_y = m_vecTgNeedSwapIn[t_i].m_iY;

		//
		// Ҫ�������ڴ��еĵ�ͼ��,Ŀǰ���������ڴ���,�ƽ���������߳���.
		t_strMapname = m_strMapName;

		// ���벻ͬ�ĵ�ͼ
		sprintf( t_sz,"%02d%02d.geo",t_x,t_y );

		t_strMapname += t_sz;

		osassert( m_vecTgNeedRelease[t_i]<MAXTG_INTGMGR );

		m_sThreadMgr.push_loadMap( 
			(char*)t_strMapname.c_str(),m_vecTgNeedRelease[t_i] );

	}


	return true;

	unguard;
}


/** \brief
*  �Լ�����ĵ���ͼ���б����ݵ����Ż������������	
*
*  �����Ϊ���¼���
*  1: tg��bbox�������ཻ�ģ�tg��λ��λ�ڵ�ǰ�������tg�����ӵ�λ�õ��������Σ�
*     ��Ҫʹ������ʽ�ĵ�ͼ���뷽ʽ��
*  2: �����ĵ�ͼʹ�������ĵ�ͼ���뷽ʽ��
*  
*  �����ǰ�����ڵ�ͼ����״̬����ÿһ֡���Ը��ݴ�������ָ�룬�������趨
*  ��ͼ�����̹߳������ڲ��ĵ�ͼ������У������Ż����������˼�㷨.....
* 
*/
bool osc_tgLoadReleaseMgr::sort_needSwapInMapList( osc_camera* _cam )
{
	guard;

	osassert( _cam );

	os_tileIdx           t_vecTgNeedSwapIn[MAXTG_INTGMGRMACRO];

	m_iBlockLoad = 0;

	// 
	// ֱ�ӵĲ��������ݴ������
	memcpy( t_vecTgNeedSwapIn,m_vecTgNeedSwapIn,
		   sizeof( os_tileIdx )*m_iNeedSwapInNum );

	for( int t_i=0;t_i<m_iNeedSwapInNum;t_i ++ )
	{

	}

	return true;

	unguard;
}



/** \brief
*  �������������������õ�Ӧ�ÿɼ��ĵ�ͼ�б�, ����ĵ�ͼ�ұ�.
*
*  ÿһ֡��Ҫ��������������ں�̨�������ͼ�ĵ���.
*  �����ϲ㵱ǰ���õĵ�ͼָ���б�Ϳ��õĵ�ͼ��Ŀ.
*
*  1: ���������λ�ã�����Ƿ���Ҫ�û���ͼ.
*  2: �����û��ĵ�ͼ���ƽ�����ͼ�����̡߳�
*  3: ����ϲ����ʹ�õĵ�ͼ�б� ���ص�ͼ������
*/
int osc_tgLoadReleaseMgr::frame_getInViewTg( 
	osc_camera* _cam,osc_TileGroundPtr* _tgList,int _maxNum )
{
	guard;

	osassert( _cam );
	osassert( _tgList );
	osassert( _maxNum );

	int   t_idx = 0;

# if __QTANG_VERSION__

	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		osassert( t_idx < _maxNum );

		if( !m_ptrRealTg[t_i].is_tgInuse() )
			continue;
		
		_tgList[t_idx] = &m_ptrRealTg[t_i];
		t_idx ++;
	}

# else

	osVec3D   t_vec3CamPos;

	_cam->get_curpos( &t_vec3CamPos );

	//
	// ATTENTION TO OPP: �˶εĴ��������⡣
	// ����λ��,�鿴�Ƿ���Ҫ��ͼ�û�.
	if( get_tgNeedInMemList( t_vec3CamPos ) )
	{		
		// �����̵߳ĵ���.
		stop_threadMapLoading();
		
		// ������Ҫ���ڴ����û����ĵ�ͼͼ���б�
		process_needSwapOutTgList();

		// ������Ҫ���ڴ��д��ڣ�Ŀǰ���ڴ����ֲ����ڵ�ͼ���б�
		process_needSwapInTgList();
		osassertex( m_iNeedSwapInNum <= m_iNeedRelTgNum,
			va( "Ҫ����ĵ�ͼ����Ŀ����С�ڵ�Ҫ�Ƴ��ĵ�ͼ����Ŀ!!����:<%d>,�Ƴ�:<%d>..\n",
			    m_iNeedSwapInNum,m_iNeedRelTgNum ) );

		// �Լ�����ĵ���ͼ���б����ݵ����Ż������������	
		sort_needSwapInMapList( _cam );

		// �ƽ�Ҫ������ͼ���б�ͼ������߳��С�
		push_needSwapInMap();

	}

	// �õ�����ĵ�ͼ���б������������߳�����Ⱦ����ĵ�ͼ���б�
	get_activeTgList();

	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		osassert( t_idx < _maxNum );

		if( !m_ptrActTg[t_i] )
			continue;

		_tgList[t_idx] = m_ptrActTg[t_i];
		t_idx ++;
	}

# endif 

	return t_idx;

	unguard;
}

//! ��ʼ����ͼ�󣬵õ���ǰ����ĵ�ͼ�б���ʱ��û��������룬���Լ��뵥���ĺ�����
int osc_tgLoadReleaseMgr::init_getActMapList( osc_TileGroundPtr* _tgList,int _maxNum )
{
	guard;

	osassert( _tgList );
	osassert( _maxNum );

	osassertex( m_sThreadMgr.is_freeStatus(),"��ǰ��ͼ�����߳�Ӧ�ô��ڿ���״̬...\n" );

	get_activeTgList();

	int   t_idx = 0;
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		osassert( t_idx < _maxNum );

		if( !m_ptrActTg[t_i] )
			continue;

		_tgList[t_idx] = m_ptrActTg[t_i];
		t_idx ++;

	}

	return t_idx;


	return 0;

	unguard;
}



//! �����ͷŵ�ǰ�ĵ�ͼ�ڴ��������
void osc_tgLoadReleaseMgr::release_tgLRMgr( void )
{
	guard;

	// ����ֹ���߳���صĵ���.
	m_sThreadMgr.reset_loadMapMgr();

	//! �����пɲ����ĵ�ͼȫ���ͷš�
	for( int t_i=0;t_i<this->m_iCurTgNum;t_i ++ )
	{
		if( m_ptrRealTg[t_i].is_tgInuse() )
		{
			if( !m_ptrRealTg[t_i].release_TG() )
				osassert( false );
		}
	}

	
	// ȫ�����ͷž�̬��mesh��osa�ļ���Դ
	g_ptrMeshMgr->reset_staticMesh();
	g_ptrMeshMgr->reset_osaMesh();
	

	return;

	unguard;
}




/** \brief
*  ���̰߳�ȫ�ĵ�ͼ�������.
*
*  �滻TGManager�е���غ���.
*/
bool osc_tgLoadReleaseMgr::threadSafe_loadNMap( 
	           char* _nmapFname,osc_TileGroundPtr _tg )
{
	guard;

	osassert( _nmapFname && (_nmapFname[0]) );
	osassert( _tg );
	osassert( !_tg->is_tgInuse() );

	// TEST CODE:
	g_bMapLoadding = true;

	if( !_tg->load_TGData( _nmapFname,this->m_ptrMidPipe ) )
	{
		osassertex( false,"��ͼ������,  ���̵߳����ͼʧ��...\n" );
		return false;
	}

	osDebugOut( "��ͼ������,  ���̵߳����ͼ<%s>�ɹ�...\n",_nmapFname );
	
	// TEST CODE:
	g_bMapLoadding = false;


	return true;

	unguard;
}



/** \brief
*  ���ε����ͼ�ĺ�����
*
*  �൱�ڽ�����ԭ����osc_TGManager�е�load_scene����.
*
*  \param _sl �����ͼ�Ľṹ��
*  \param _maxX,_maxZ ��ͼ��x,z�����ϵ�����ͼ������
*  
*/
bool osc_tgLoadReleaseMgr::load_scene( os_sceneLoadStruct* _sl,int _maxX,int _maxZ )
{
	guard;

	osassert( _sl );

	osassert( _maxX>0 );
	osassert( _maxZ>0 );

	VEC_string     t_vecMapName;

	m_iMaxXTgNum = _maxX;
	m_iMaxZTgNum = _maxZ;
	m_iMaxX = m_iMaxXTgNum*MAX_GROUNDWIDTH;
	m_iMaxZ = m_iMaxZTgNum*MAX_GROUNDWIDTH;

	// ȫ�������ͼ���ڴ���
	if( m_bTLoadInMem )
	{
		osassertex( false,"ȫ�������ͼ" );
	}
	else
	{
		// �ȵõ�Ҫ�����ͼ�����֡�
		t_vecMapName.resize( MAXTG_INTGMGR );

		if( !get_loadSceneName( &t_vecMapName[0],_sl ) )
			osassertex( false,"ȡ�ó�ʼ����ͼ����ʧ��" );

		// syq 5-20 add
		if( osc_TGManager::m_pUpdate_uiLoadingStep )
			(*osc_TGManager::m_pUpdate_uiLoadingStep)( 25,0);
		g_iLoadProgress = 25;

		// ѭ���������ǵõ������е�ͼ����,�˴�Ϊ��������,���������ɺ���ܷ���.
		for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
		{
			if( t_vecMapName[t_i] == "" )
				continue;

			// �ƽ���ͼ��������߳�,���ȴ��������.
			m_sThreadMgr.push_loadMap((char*)t_vecMapName[t_i].c_str(),t_i,TRUE );

			// syq 5-20 add
			g_iLoadProgress += int((float(t_i+1)/float(MAXTG_INTGMGR))*30);
			if( osc_TGManager::m_pUpdate_uiLoadingStep )
				(*osc_TGManager::m_pUpdate_uiLoadingStep)( g_iLoadProgress,0);

		}

	}

    // 
	// ������ֹ��ǰ���̴߳���
	// River @ 2006-2-23:�ڵ�ͼ��Ҫ�������ʱ�򣬴˺����п�������
	stop_threadMapLoading();

	// ��ʼ����ʼ��λ����Ϣ.
	init_insertInfo( _sl );

	return true;

	unguard;
}



//! �����ƶ��Ĵ��������˺�����
bool osc_tgLoadReleaseMgr::space_moveMapLoad( os_sceneLoadStruct* _sl,int _maxX,int _maxZ )
{
	guard;

	int         t_idx = 0;
	s_string     t_szMapNameList[16];

	osassert( _sl );

	m_iMaxX = _maxX;
	m_iMaxZ = _maxZ;
	m_iMaxXTgNum = m_iMaxX/MAX_GROUNDWIDTH;
	m_iMaxZTgNum = m_iMaxZ/MAX_GROUNDWIDTH;

	// ����ֹ�̵߳ĵ�ͼ����
	stop_threadMapLoading();

	if( !get_loadSceneName( t_szMapNameList,_sl ) )
		osassert( false );

	spacemove_swapOutMap( t_szMapNameList );
	osDebugOut( "Swap out tg num is:<%d>..\n",m_iNeedRelTgNum );


	// ѭ���������ǵõ������е�ͼ����,�˴�Ϊ��������,���������ɺ���ܷ���.
	for( int t_i=0;t_i<MAXTG_INTGMGR;t_i ++ )
	{
		if( t_szMapNameList[t_i] == "" )
			continue;

		// �ƽ���ͼ��������߳�,���ȴ��������.
		m_sThreadMgr.push_loadMap( (char*)t_szMapNameList[t_i].c_str(),
			          m_vecTgNeedRelease[t_idx],TRUE  );

		t_idx ++;
	}

    // 
	// ������ֹ��ǰ���̴߳���
	stop_threadMapLoading();
	init_insertInfo( _sl );

	return true;

	unguard;
}

