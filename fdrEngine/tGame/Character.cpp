//===================================================================
/** \file  
*  Filename: Character.cpp
*  Desc:   场景人物角色的实现文件
*  His:      王凯 @ 1/12 2004 16:18
*/
//================================================================
# include "stdafx.h"
# include "Character.h"
#include "equipmentmanager.h"

# include "tengine.h"
#include "dxutil.h"
#include <algorithm>

# include "cameraCtrl.h"



//
extern int              g_iShadowId;


//! 是否使用box测试灯光。
# define LIGHT_TEST 1

/**brief*****************************************************************/
/*!构造函数                                                          
/************************************************************************/
I_Hero::I_Hero(void)
{
	m_strName	= "";
	m_strCurEquipName = "";
	m_pObj		= NULL;
	m_nPartNum	= 0;
	m_nState	= OBJ_STATE_WAIT;
	m_nLife		= 100;
	m_fHei		= 0.0f;
	m_verPos	= osVec3D(0,0,0);
	m_fHeight	= 0.0f;
	m_fAngle	= 0.0f;
	m_vCurDir   = osVec3D(0,0,0);
	m_fSpeed    = 0.04f;
	//m_VECPath.clear();
	m_MAPequipment.clear();
	m_VECEquipName.clear();
	m_nTotalEquipCount = 0;
	m_pCurEquip = NULL;
	m_nTotalActCount = 0;
	m_nCurAct = 0;
	m_fFrameTime = 10.0f;
	m_fActTime = 0.0f;
	osMatrixIdentity(&m_matWorld);
	m_pcNpc = NULL;

	m_bSoundPlay = false;
	m_LightID = -1;
	/*os_dlightStruct dlight;
	dlight.m_fRadius = 2.0f;
	dlight.m_lightColor = RGB(200,0,255);
	dlight.m_ltype = OSE_DLIGHT_POINT;
	dlight.m_vec3Pos = g_cHero.m_verPos;
	m_LightID = g_sceneMgr->create_dynamicLight(dlight);*/
	

}

/**brief*****************************************************************/
/*!析构函数                                                          
/************************************************************************/
I_Hero::~I_Hero(void)
{
}

/**brief*****************************************************************/
/*!得到当前位置在整个地图中的索引,并返回格子的中心点
/************************************************************************/
void I_Hero::GetCurMeshIndex()
{
	//！在这里调用小江的函数来得到人物所在的是哪一块儿地图
	//!并得到人物所在哪一个地图格子中。

	os_pickTileIdx t_TileIdx;
	if(!g_sceneMgr->get_sceneTGIdx(t_TileIdx,m_verPos))
		return;
	m_nFieldX = t_TileIdx.m_tgIdx.m_iX;
	m_nFieldY = t_TileIdx.m_tgIdx.m_iY;
	m_sBaseMesh.nX = t_TileIdx.m_colTileIdx.m_iX;
	m_sBaseMesh.nY = t_TileIdx.m_colTileIdx.m_iY;
	//m_fHei = t_TileIdx.m_fTileHeight;



}




//! 调入英雄的时候调入英雄的所有的动作,这个函数用于切换英雄的动作到下一个动作.
void I_Hero::change_nextAction( void )
{
	os_newAction  t_newAction;


	if( m_nCurAct == (m_pObj->get_totalActNum()-1) )
		m_nCurAct = 0;
	else
		m_nCurAct ++;

	t_newAction.m_iNewAct = m_nCurAct;
	
	t_newAction.m_changeType = OSE_DIS_OLDACT;

	m_pObj->change_skinMeshAction( t_newAction );

}

//! 处理当前Character的小量移动，用于平滑地运动相机的位置。
void I_Hero::move_heroX( float _xdis )
{

	m_verPos.x += _xdis;
	g_cCamera.Update();
	
}

void I_Hero::move_heroZ( float _zdis )
{
	m_verPos.z += _zdis;
	g_cCamera.Update();
}

void I_Hero::move_heroY( float _ydis )
{
	m_verPos.y += _ydis;
	g_cCamera.Update();
}






/**breif*****************************************************************/
/*!查找周围的八个格子
/************************************************************************/
void I_Hero::FindMesh(CNpcManager *_pNpcManager, float _fTime)
{
	//River added @ 2004-4-10:
	return;

}

/**brief*****************************************************************/
/*	!根据用户的输入点击来查找路径.
*	\param	_nX
*   \param  _nY
*			目标地图的X,Y坐标
*	\param  _X
*	\param  _Y
*			目标地图的格子的索引
/************************************************************************/
bool I_Hero::FindPath(int _nX, int _nY, int _X, int _Y)
{
	if(m_nState == OBJ_STATE_DEATH)
		return true;
	if (m_pcNpc) 
	{
		m_nState = OBJ_STATE_LEAF;
		m_nCurAct = OBJ_STATE_WALK;
		ChangeAction();
		m_fActTime = 0.0f;

		m_pcNpc->m_nCurAct = OBJ_STATE_WALK;
		m_pcNpc->m_nState = OBJ_STATE_WALK;
		m_pcNpc->ChangeAction();
		m_pcNpc->m_fActTime = 0.0f;

		m_pcNpc = NULL;
	}
	else if(m_nState == OBJ_STATE_LEAF)
	{
		m_nState = OBJ_STATE_WALK;
	}

	
	BaseMesh	t_WorldMesh,t_WorldMeshSrc;
	FromLocalToWorld(_nX, _nY, _X, _Y, t_WorldMesh);

	FromLocalToWorld(m_nFieldX,m_nFieldY,m_sBaseMesh.nX, m_sBaseMesh.nY, t_WorldMeshSrc);
	if(t_WorldMesh == t_WorldMeshSrc)
		return true;

	if(!m_VECPath.empty())
	{
		if(!(t_WorldMeshSrc == *m_VECPath.front()))
		{
			t_WorldMeshSrc = *m_VECPath.front();
		}
	}
	m_VECPath.clear();
/*
	if( (t_WorldMeshSrc.nX==t_WorldMesh.nX)&&
		(t_WorldMeshSrc.nY==t_WorldMesh.nY ) )
		osassert( false );
*/

	//@{
	// River added @ 2004-2-5 : 用于测试A*算法。

	as_colInfo    t_colInfo;
	os_tileIdx    t_tgIdx[4];
	int           t_pathNum;


	t_colInfo.m_iMapNum = g_sceneMgr->get_inViewTGIdx( t_tgIdx );

	if( (m_nFieldX == _nX)&&(m_nFieldY == _nY) )
	{
		t_tgIdx[0].m_iX = _nX;
		t_tgIdx[0].m_iY = _nY;
	}
	else
	{
		t_tgIdx[0].m_iX = _nX;
		t_tgIdx[0].m_iY = _nY;
		t_tgIdx[1].m_iX = m_nFieldX;
		t_tgIdx[1].m_iY = m_nFieldY;
	}

	for( int t_i=0;t_i<t_colInfo.m_iMapNum;t_i++ )
	{
		t_colInfo.m_arrColInfo[t_i] = g_sceneMgr->get_sceneColData( t_tgIdx[t_i] );
		t_colInfo.m_arrTGIdx[t_i].nX = t_tgIdx[t_i].m_iX;
		t_colInfo.m_arrTGIdx[t_i].nY = t_tgIdx[t_i].m_iY;
	}

	t_colInfo.m_iMaxRes = 100;
	t_colInfo.m_resultPtr = &m_VECPath;



	t_pathNum = fd_AstarBase::Instance()->find_path( t_WorldMeshSrc.nX,t_WorldMeshSrc.nY,
		t_WorldMesh.nX,t_WorldMesh.nY,&t_colInfo );

	if( t_pathNum == 0 )
	{
		return true;
	}

	if( t_pathNum == -1 )
	{
		return false;
	}

	//@}


	if( m_VECPath.front() == NULL )
		osassert( false );

	//!因为返回的路径的第一个格子是起始位置,所以要把第一个格子去除掉
	//m_VECPath.pop_front();

	//!得到下一个格子的中心点,
	osVec3D		t_VerCenter;


	if( m_VECPath.empty() )
		osassert( false );
	osVec2D  t_Ver2D = FromWorldIndexToVertex( m_VECPath.front()->nX, m_VECPath.front()->nY);

	t_VerCenter.x = t_Ver2D.x;
	t_VerCenter.y = 0.0f;
	t_VerCenter.z = t_Ver2D.y;

	//!计算方向角度
	m_vCurDir = t_VerCenter - m_verPos;
	m_vCurDir.y = 0.0f;
				
	D3DXVec3Normalize(&m_vCurDir,&m_vCurDir);

	m_fAngle = acosf(D3DXVec3Dot(&m_vCurDir, &osVec3D(1.0f, 0.0f, 0.0f))/D3DXVec3Length(&m_vCurDir));
	if( m_vCurDir.z > 0.0f )
		m_fAngle	=  -m_fAngle;

	if(m_nState == OBJ_STATE_WAIT || m_nState == OBJ_STATE_ACT 
		|| m_nState == OBJ_STATE_ACTION  )
	{
		m_nState = OBJ_STATE_WALK;
		m_nCurAct = OBJ_STATE_WALK;
		ChangeAction();
	}
	return true;
}

/**brief*****************************************************************/
/*	!更新位置,在每一帧中使用
*	\param	_fTime
*			距离上一帧的时间差
/************************************************************************/
void I_Hero::Move(float _fTime)
{
	if(m_nState == OBJ_STATE_DEATH)
		return;

	os_pickTileIdx  t_res;

	FindMesh(&g_cNpcManager,_fTime);

	if(m_VECPath.empty())
		return;
	if(m_nState != OBJ_STATE_WALK && m_nState != OBJ_STATE_RUN && m_nState != OBJ_STATE_LEAF)
		return;

	frame_moveSnd();

	//!得到当前位置在整个地图中格子的索引

	//!如果到达一个节点，则重新计算方向，并删除这个节点
	osVec3D		t_VerCenter;
	osVec2D  t_Ver2D = FromWorldIndexToVertex(m_VECPath.front()->nX,m_VECPath.front()->nY);

	g_sceneMgr->get_tilePos( m_VECPath.front()->nX,m_VECPath.front()->nY,t_VerCenter );
	
	//@{
	// Ipedo added @ 2005-1-25 : 用于记录上一格地图中心点。
	// 初始值为人物当前位置的坐标
	static osVec3D t_preCenter = m_verPos;
	//@}

	float dist = D3DXVec3Dot(&m_vCurDir,&t_VerCenter);
	float dist2 = D3DXVec3Dot(&m_vCurDir,&m_verPos);

	// 如果到达一个节点，则重新计算方向，并删除这个节点
	if(dist2 >= dist)
	{

		float t_d = dist2 - dist;

		osVec3D		t_VerPower, t_VerNext;

		WORD  count = m_VECPath.count();
		for(WORD s = 1; s < count; s++)
		{
			//!得到下一个格子的中心点,
			t_Ver2D = FromWorldIndexToVertex(m_VECPath[s].nX,m_VECPath[s].nY);

			// 得到y值,进行平顺处理.
			t_VerNext.x = t_Ver2D.x;
			t_VerNext.y = 0.0f;
			t_VerNext.z = t_Ver2D.y;

			// 得到高度.
			if( t_VerNext.x < 0 ) t_VerNext.x = 0;
			if( t_VerNext.z < 0 ) t_VerNext.z = 0;
			g_sceneMgr->get_sceneTGIdx( t_res,t_VerNext );
			t_VerNext.y = m_fHei + t_res.m_fTileHeight;

			t_Ver2D = FromWorldIndexToVertex(m_VECPath[s-1].nX,m_VECPath[s-1].nY);
			t_VerPower.x = t_Ver2D.x;
			t_VerPower.y = 0.0f;
			t_VerPower.z = t_Ver2D.y;
			// 得到高度.
			if( t_VerPower.x < 0 ) t_VerPower.x = 0;
			if( t_VerPower.z < 0 ) t_VerPower.z = 0;
			g_sceneMgr->get_sceneTGIdx( t_res,t_VerPower );
			t_VerPower.y = m_fHei + t_res.m_fTileHeight;

			//!计算方向角度
			m_vCurDir = t_VerNext - t_VerPower;
			D3DXVec3Normalize(&m_vCurDir,&m_vCurDir);

			float t_dist = D3DXVec3Dot(&m_vCurDir,&t_VerNext);
			float t_dist2 = D3DXVec3Dot(&m_vCurDir,&m_verPos);

			//@{
			// Ipedo added @ 2005-1-25 : 将上一格地图中心点赋值给t_preCenter。
			t_preCenter = t_VerPower;
			//@}

			if(t_dist2 < t_dist)
			{
				D3DXVec3Add(&m_verPos, &t_VerPower, &(m_vCurDir * t_d));
				break;
			}
		}

		m_fAngle = acosf(D3DXVec3Dot(&m_vCurDir, 
			&osVec3D(1.0f, 0.0f, 0.0f))/D3DXVec3Length(&m_vCurDir));
		if( m_vCurDir.z > 0.0f )
			m_fAngle	=  -m_fAngle;

		m_VECPath.pop_front();

		//!如果到达终点，则停止
		if(m_VECPath.empty())
		{
			//!如果是最后一个节点，则把人物位置移动到节点的中心点
			m_verPos = t_VerCenter;
			m_nState = OBJ_STATE_WAIT;
			m_nCurAct = OBJ_STATE_WAIT;
			ChangeAction();

			if( m_verPos.x < 0 ) m_verPos.x = 0;
			if( m_verPos.z < 0 ) m_verPos.z = 0;
			g_sceneMgr->get_sceneTGIdx( t_res,m_verPos );
			m_verPos.y = m_fHei + t_res.m_fTileHeight;

		    g_sceneMgr->get_detailPos( m_verPos );
			m_verPos.y += 0.08f;
			m_pObj->frame_move( m_verPos, m_fAngle);
			GetCurMeshIndex();
			return;
		}

		// 根据当前的位置得到当前的idx.
		g_sceneMgr->get_detailPos( m_verPos );
		m_verPos.y += 0.08f;
		m_pObj->frame_move( m_verPos, m_fAngle);
		GetCurMeshIndex();
		return;
	}

	// River @ 2006-3-18: 通常人物的移动
	//!移动人物
	D3DXVec3Add(&m_verPos, &m_verPos, &(m_vCurDir * (m_fSpeed * _fTime)));

	//@{
	// Ipedo added @ 2005-1-25 : 实现人物走动时高度的插值。
	// 算法：将人物所处的前后两个节点中心高度做插值，用三点之间的距离做系数
	os_pickTileIdx      t_resWT;
	g_sceneMgr->get_sceneTGIdx( t_resWT,t_preCenter );
	// 上一节点中心高度
	float t_curHeight = t_resWT.m_fTileHeight;
	g_sceneMgr->get_sceneTGIdx( t_resWT,t_VerCenter );
	// 下一节点中心高度
	float t_nextHei = t_resWT.m_fTileHeight;
	// 前后两个节点的距离
	float t_tolDis = osVec3Length(&(t_VerCenter-t_preCenter));
	// 当前位置点到上一节点距离
	float t_curDis = osVec3Length(&(m_verPos-t_preCenter));

	// 插值系数
	float t_tmpCoeff;
	// 判断到达下一节点或超过下一节点，将系数设为1
	if(t_tolDis==0||t_curDis>t_tolDis)
		t_tmpCoeff = 1;
	else{
		t_tmpCoeff = t_curDis/t_tolDis;
	}
	// 人物高度为插值结果（即地表高度）加上人物自身高度
	m_verPos.y = m_fHei+t_curHeight+(t_nextHei-t_curHeight)*t_tmpCoeff;
	//@}


	g_sceneMgr->get_detailPos( m_verPos );
	m_verPos.y += 0.08f;
	m_pObj->frame_move( m_verPos, m_fAngle);


	if (m_LightID!=-1)
	{
		os_dlightStruct dl;
		g_sceneMgr->get_dynamicLight( m_LightID,dl );
		dl.m_vec3Pos = m_verPos;
		g_sceneMgr->set_dynamicLight( m_LightID,dl );
		
	}
	

	//!在这里得到当前人物所在的地图坐标和格子索引，和当前所在格子的高度值
	GetCurMeshIndex();

	hero_areaMove();
	//osDebugOut( "hero move" );


}

/**brief*****************************************************************/
/* !变换装备
/************************************************************************/
void I_Hero::ChangeEquipment()
{
	static WORD n = 0;
	if (m_VECEquipName.size() <= 0) 
	{
		return;
	}
	if(n >= m_VECEquipName.size())
		n = 0;
	m_pCurEquip = &m_MAPequipment[m_VECEquipName[n]];
	if(m_pCurEquip)
		m_pObj->change_equipment(m_MAPequipment[m_VECEquipName[n]]);
	n++;
}

/************************************************************************/
/*!变换动作
/************************************************************************/
void I_Hero::ChangeAction()
{
	os_newAction  t_newAction;
	t_newAction.m_iNewAct = m_nCurAct;
	switch(m_nState) 
	{
	case OBJ_STATE_WAIT:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		break;
	case OBJ_STATE_WALK:
		t_newAction.m_changeType = OSE_DIS_OLDACT;	
		m_fSpeed = MOVE_SPEED;
		break;
	case OBJ_STATE_RUN:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		m_fSpeed = 9.0f;
		break;
	case OBJ_STATE_ACTION:
		t_newAction.m_changeType = OSE_DIS_OLDACT; 
		break;
	case OBJ_STATE_ACT:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		t_newAction.m_iNewAct = -1;
		strcpy( t_newAction.m_strActName,g_strRBtnName[g_iRBtnActIdx] );
		break;

	case OBJ_STATE_BEACT:
		t_newAction.m_changeType = OSE_DIS_NEWACT_TIMES;
		t_newAction.m_iPlayNum = 1;
		break;
	case OBJ_STATE_DEATH:
		t_newAction.m_changeType = OSE_DIS_NEWACT_LASTFRAME;
		t_newAction.m_iPlayNum = 1;
		break;
	case OBJ_STATE_LEAF:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		m_fSpeed = 0.02f;
		break;
	default:
		return;
	}
	
	m_fActTime = m_pObj->change_skinMeshAction(t_newAction);


	if( (t_newAction.m_iNewAct == 2)||
		(t_newAction.m_iNewAct == 1) )
	{
		m_bSoundPlay = true;
		m_fS1 = 0.170f;
		m_fS2 = 0.635f;
	}
	else
		m_bSoundPlay = false;

}

//! 播放人物动作声音的frameMovel
void I_Hero::frame_moveSnd( void )
{
	static float  t_sfActAccutime = 0.0f;

	if( !m_bSoundPlay )
	{
		t_sfActAccutime = 0.0f;
		m_bS1 = false;
		m_bS2 = false;
		return;
	}

	float   t_fTime = sg_timer::Instance()->get_lastelatime();

	t_sfActAccutime += t_fTime;

	if( (t_sfActAccutime >= m_fActTime*m_fS1)&&
		(!m_bS1 ) )
	{
		m_bS1 = true;

		// play Sound.

		play_sound( "hello" );
	}

	if( (t_sfActAccutime >= m_fActTime*m_fS2)&&
		(!m_bS2 ) )
	{
		m_bS2 = true;

		// play sound.
		play_sound( "hello" );
	}

	if( t_sfActAccutime > m_fActTime )
	{
		t_sfActAccutime -= m_fActTime;
		m_bS1 = false;
		m_bS2 = false;
	}

}



/************************************************************************/
/*！变换动作2
/************************************************************************/
void I_Hero::ChangeAction(int _nCurAct)
{
	m_nState = _nCurAct;
	m_nCurAct = _nCurAct;
	ChangeAction();
}


bool readHeroEquipFronIni( char *_iniName,os_skinMeshInit& _init )
{
	guard;

	osassert( _iniName );

	CIniFile    t_iniFile;

	t_iniFile.OpenFileRead( _iniName );

	t_iniFile.ReadSection( "Equip" );

	// 
	// 读入身体7个部位的mesh和skin
	for( int t_i = 1;t_i<=8;t_i ++ )
	{
		char   t_str[3];
		int    t_k;
		sprintf( t_str,"M%d",t_i );
		t_iniFile.ReadLine( t_str,(int*)&t_k );
		_init.m_smObjLook.m_mesh[t_i-1] = t_k;
		sprintf( t_str,"S%d",t_i );
		t_iniFile.ReadLine( t_str,(int*)&t_k );
		_init.m_smObjLook.m_skin[t_i-1] = t_k;
	}
	t_iniFile.CloseFile();


	return true;

	unguard;
}


os_skinMeshInit   g_smInitOne;

VEC_string   g_equFile;

/**brief*****************************************************************/
/*	!从一个ini文件装入一个hero
*	\param	_chPath
*			ini文件的路径名
*	\param	_pHero
*			装入的hero
/************************************************************************/
bool LoadHeroFromIni(char *_chPath,I_Hero *_pHero)
{
	if(!_chPath || !_pHero )
		return false;

	CIniFile   t_iniFile;
	char       t_str[256];
	char t_chDir[MAX_PATH];

	t_iniFile.OpenFileRead( _chPath );

	if( !t_iniFile.ReadSection( HERO ) )
		return false;

	os_skinMeshInit   t_smInit;
	//ZeroMemory(&t_smInit,sizeof(os_skinMeshInit));

	//!读取英雄的名字
	if( !t_iniFile.ReadLine( NAME,t_str,256) )
	{
		return false;
	}
	_pHero->m_strName		= t_str;

	//!英雄所在的目录名
	ZeroMemory(t_str,256);
	ZeroMemory(t_chDir,256);
	if( !t_iniFile.ReadLine( DIRTCTION,t_str,256) )
	{
		return false;
	}

	g_szChrName = t_str;

	sprintf(t_chDir, "%s%s",SHADEDIR,t_str);
	strcpy( t_smInit.m_strSMDir,t_chDir );

	//!读取初始化的动作的索引
	_pHero->m_nCurAct = 0;
	//t_smInit.m_iActIdx = _pHero->m_nCurAct;

	ZeroMemory(t_chDir,256);


	///////////////////////////////////////////////////////////////////
	// 测试CHA文件。
	// 测试调入*.cha文件。
	ChaHeader fileHeader;
	BYTE*   t_fstart;
	sprintf( t_chDir,"%s%s.cha",SHADEDIR,t_str );
	BYTE*	pBuffer = NULL;

	if( file_exist( t_chDir ) )
	{
		t_fstart = START_USEGBUF;
		if( 0 >= read_fileToBuf( t_chDir,t_fstart,TMP_BUFSIZE ) )
		{
			osassert( false );
			return false;
		}
		
		pBuffer = t_fstart;
		READ_MEM_OFF(&fileHeader,t_fstart,sizeof(ChaHeader));
		if (fileHeader.mID != ChaFileID)
		{
			osassertex(false,"CHA文件版本不正确!请使用工具转化到最新版本!");
		}
		
		t_smInit.m_iTotalActTypeNum = fileHeader.mTotalActTypeNum;
		t_smInit.m_smObjLook.m_iPartNum = fileHeader.mPartNum;
		t_smInit.m_vec3Max = fileHeader.mBoxMax;
		t_smInit.m_vec3Min = fileHeader.mBoxMin;
		t_smInit.m_vec3AdjPos = fileHeader.mPos;
		t_smInit.m_fAdjRotAgl = fileHeader.mRot;
		if (fileHeader.mEffectNr >= 1)
		{
			SChaEffectItem* pEffectItem;
			pEffectItem = (SChaEffectItem*)(pBuffer + fileHeader.mEffectOfs);
			memcpy(t_smInit.m_szspeFileName,(pBuffer + pEffectItem->mNameOfs),pEffectItem->mNameNr);
			t_smInit.m_spePostion = pEffectItem->mEffectOffset;
		}
		
		END_USEGBUF;
	}
	else
	{
		// 从bfs文件读入人物相关的数据
		sprintf( t_chDir,"%s%s.bfs",SHADEDIR,t_str );
		FILE*  t_file;

		t_file = fopen( t_chDir,"rt" );
		fscanf( t_file,"%d %d",
			&t_smInit.m_iTotalActTypeNum,&t_smInit.m_smObjLook.m_iPartNum );
		fclose( t_file );
	}
	//test
//	t_smInit.m_speFileName = "speff//s_BAhuoA.spe";
	//memcpy(t_smInit.m_szspeFileName,"speff//s_BAhuoA.spe",sizeof("speff//s_BAhuoA.spe")+1);
	t_smInit.m_spePostion = osVec3D(0.0f,0.0f,0.0f);

	_pHero->m_nTotalActCount = t_smInit.m_iTotalActTypeNum;
	_pHero->m_nPartNum = t_smInit.m_smObjLook.m_iPartNum;



	//!全部一共有多少动作
	t_smInit.m_iTotalActTypeNum = _pHero->m_nTotalActCount;

	//!身体分为几个部分
	t_smInit.m_smObjLook.m_iPartNum = _pHero->m_nPartNum;

	//!初始化装备结构
	os_equipment	t_osEquip;
	s_string      t_strName2 = "first";

	t_osEquip.m_iCPartNum = _pHero->m_nPartNum-1;



	readHeroEquipFronIni( "heroIni\\hinit.ini",t_smInit );
	t_smInit.m_smObjLook.m_iPartNum = 7;



	//t_osEquip.m_arrId[_pHero->m_nPartNum-1] = _pHero->m_nPartNum-1;
	//t_osEquip.m_arrMesh[_pHero->m_nPartNum-1] = 1;
	//t_osEquip.m_arrSkin[_pHero->m_nPartNum-1] = 1;

	if(_pHero->m_MAPequipment.count(t_strName2)==0)
	{
		_pHero->m_MAPequipment[t_strName2] = t_osEquip;
		_pHero->m_VECEquipName.push_back(t_strName2);
	}

	//!读取初始化位置
	if( !t_iniFile.ReadLine( FIELDX,&_pHero->m_nFieldX) )
	{
		return false;
	}
	if( !t_iniFile.ReadLine( FIELDY,&_pHero->m_nFieldY) )
	{
		return false;
	}
	//!读取初始化位置的高度当做固定于地面的高度
	if( !t_iniFile.ReadLine( HEIGHT,&_pHero->m_fHeight) )
	{
		return false;
	}


	_pHero->m_sBaseMesh.nX = g_iInsertX;
	_pHero->m_sBaseMesh.nY = g_iInsertZ;

	osVec3D   t_vec3Pos;
	g_sceneMgr->get_tilePos( g_iInsertX,g_iInsertZ,t_vec3Pos );
	_pHero->m_fHeight += t_vec3Pos.y;



	//
	// 如果baseMesh大于128，则处理。
	if( (_pHero->m_sBaseMesh.nX>=128)||(_pHero->m_sBaseMesh.nY>=128) )
	{
		_pHero->m_nFieldX = _pHero->m_sBaseMesh.nX/128;
		_pHero->m_nFieldY = _pHero->m_sBaseMesh.nY/128;

		_pHero->m_sBaseMesh.nX = _pHero->m_sBaseMesh.nX%128;
		_pHero->m_sBaseMesh.nY = _pHero->m_sBaseMesh.nY%128;
	}

	_pHero->m_VECPath.resize();
	//_pHero->m_VECPath.push_back(_pHero->m_sBaseMesh);

	osVec2D t_Ver2D = FromIndexToVertex(_pHero->m_nFieldX,_pHero->m_nFieldY,
		_pHero->m_sBaseMesh.nX,_pHero->m_sBaseMesh.nY);
	_pHero->m_verPos.x = t_Ver2D.x;
	_pHero->m_verPos.y = _pHero->m_fHeight;
	_pHero->m_verPos.z = t_Ver2D.y;



	t_smInit.m_vec3Pos = _pHero->m_verPos;

	char sss[128];
	sprintf(sss,"%f,%f,%f",_pHero->m_verPos.x,
		_pHero->m_verPos.y,
		_pHero->m_verPos.z);
	_DbgOuts( "pos", 0, S_OK, sss);



	D3DXMatrixTranslation(&_pHero->m_matWorld,_pHero->m_verPos.x,_pHero->m_verPos.y,_pHero->m_verPos.z);

	//!初始化旋转的角度
	_pHero->m_fAngle = 0.0f;
	t_smInit.m_fAngle = _pHero->m_fAngle;

	//!英雄所有可用的装备数据所在的路径

	t_smInit.m_bShadowSupport = true;
	t_smInit.m_iActIdx = 1;

	t_smInit.m_bUseDirLAsAmbient = false;



	t_smInit.m_bWriteZbuf = true;

	
	g_sceneMgr->get_detailPos( t_smInit.m_vec3Pos );

	_pHero->m_pObj = g_sceneMgr->create_character( &t_smInit );


	memcpy( &g_smInitOne,&t_smInit,sizeof( os_skinMeshInit ) );
	//t_smInit.m_vec3Pos.x += 1.0f;
	//g_sceneMgr->create_character( &t_smInit );
	//
	g_iShadowId = g_sceneMgr->create_fakeShadow( t_smInit.m_vec3Pos,3.0f );


	os_dlightStruct dlight;
	dlight.m_fRadius = 5.0f;
	dlight.m_lightColor.r = 1.0f;
	dlight.m_lightColor.g = 0.0f;
	dlight.m_lightColor.b = 0.0f;
	dlight.m_ltype = OSE_DLIGHT_POINT;
	dlight.m_vec3Pos = g_cHero.m_verPos;
	if( g_bEnableDLight )
		_pHero->m_LightID = g_sceneMgr->create_dynamicLight(dlight);


	osVec3D  t_vec3 = osVec3D( 1.0f,1.0f,1.0f );
	_pHero->m_pObj->scale_skinMesh( t_vec3 );


	// 
	// 设置人显示刀光。
	//_pHero->m_pObj->playSwordGlossy( true );

	


	if(!_pHero->m_pObj)
	{
		return false;
	}

	// 读入装备相关
	t_iniFile.ReadSection( "equip" );
	char   t_sz[128];
	int    t_iEquNum;
	t_iniFile.ReadLine( "equnum",&t_iEquNum );
	g_equFile.resize( t_iEquNum );
	for( int t_i=0;t_i<t_iEquNum ;t_i ++ )
	{
		char  t_key[128];
		sprintf( t_key,"%s%d","equ",t_i+1 );
		t_iniFile.ReadLine( t_key,t_sz,128 );
		g_equFile[t_i] = t_sz;
	}
	t_iniFile.CloseFile();

	memcpy( &g_smInitOne,&t_smInit,sizeof( os_skinMeshInit ) );


	// 
	//
	// 得到当前角色应当移动的速度.
	float  t_f;
	t_f = _pHero->m_pObj->get_actUsedTime( OBJ_STATE_WALK );
	if( t_f <= 0.0f )
		t_f = 1.0f;
	MOVE_SPEED = BASESIZE/t_f;


	osVec3D  t_heroPos;
	float    t_fAngl;
	g_cHero.m_pObj->get_posAndAgl( t_heroPos,t_fAngl );


	_pHero->m_pObj->playSwordGlossy( true );


/*
	// 更换装备
	os_equipment  t_equip;
	t_equip.m_iCPartNum = 1;
	t_equip.m_arrId[0] = 6;
	t_equip.m_arrMesh[0] = 13;
	t_equip.m_arrSkin[0] = 1;
	_pHero->m_pObj->change_equipment( t_equip );
*/
	return true;
}


/**brief*****************************************************************/
/*	!npc构造函数
/************************************************************************/
I_Npc::I_Npc(void)
{
	m_strName	= "";
	m_strCurEquipName = "";
	m_pObj		= NULL;
	m_nPartNum	= 0;
	m_nState	= OBJ_STATE_WALK;
	m_nLife		= 50;
	m_fHei		= 0.0f;
	m_verPos	= osVec3D(0,0,0);
	m_fHeight	= 0.0f;
	m_fAngle	= 0.0f;
	m_vCurDir   = osVec3D(0,0,0);
	m_fSpeed    = 0.005f;
	//m_VECPath.clear();
	m_MAPequipment.clear();
	m_VECEquipName.clear();
	m_VECNode.clear();
	m_nTotalEquipCount = 0;
	m_pCurEquip = NULL;
	m_nTotalActCount = 0;
	m_nCurAct = 1;
	m_fFrameTime = 10.0f;
	m_fActTime = 0.0f;
	osMatrixIdentity(&m_matWorld);
}

I_Npc::~I_Npc(void)
{

}

/**brief*****************************************************************/
/*!得到当前位置在整个地图中的索引,并返回格子的中心点
/************************************************************************/
void I_Npc::GetCurMeshIndex()
{
	if(m_nState == OBJ_STATE_DEATH)
		return;

	//！在这里调用小江的函数来得到人物所在的是哪一块儿地图
	//!并得到人物所在哪一个地图块儿中。
	os_pickTileIdx t_TileIdx;
	if(!g_sceneMgr->get_sceneTGIdx(t_TileIdx,m_verPos))
	{
		return;
	}
	m_nFieldX = t_TileIdx.m_tgIdx.m_iX;
	m_nFieldY = t_TileIdx.m_tgIdx.m_iY;
	m_sBaseMesh.nX = t_TileIdx.m_colTileIdx.m_iX;
	m_sBaseMesh.nY = t_TileIdx.m_colTileIdx.m_iY;
	//m_fHei			= t_TileIdx.m_fTileHeight;

}

/**brief*****************************************************************/
/*	!查找路径,到达路径的终点后就要重新寻找路径
*	!使用在ini文件中预定义好的三个节点来查找路径
/************************************************************************/
bool I_Npc::FindPath()
{
	if(m_nState == OBJ_STATE_DEATH)
		return true;

	//m_VECPath.clear();
	//!目前只在三个节点之前寻路，所以不需要传入目的节点
	//!在这里使用汴安提供的查找路径的函数
	os_tileIdx t_FieldInc;
	t_FieldInc.m_iX = m_nFieldX;
	t_FieldInc.m_iY = m_nFieldY;

	const os_TileCollisionInfo*  t_CollData =  
		g_sceneMgr->get_sceneColData( t_FieldInc );

	if(m_sBaseMesh == m_VECNode[0]) 
	{
		m_VECPath.push_back(m_VECNode[1]);
	}else 
	if(m_sBaseMesh == m_VECNode[1])
	{
		m_VECPath.push_back(m_VECNode[2]);
	}else
	if(m_sBaseMesh == m_VECNode[2])
	{
		m_VECPath.push_back(m_VECNode[0]);
	}
	return true;
}

/**brief*****************************************************************/
/*	!更新位置,在每一帧中使用
*	\param	_fTime
*			距离上一帧的时间差
/************************************************************************/
void I_Npc::Move(float _fTime)
{

}

/************************************************************************/
/*!变换动作
/************************************************************************/
void I_Npc::ChangeAction()
{
	os_newAction  t_newAction;

	t_newAction.m_iNewAct = 1;
	if( m_nState != OBJ_STATE_WAIT )
		m_nState = OBJ_STATE_WAIT;
	switch(m_nState) 
	{
	case OBJ_STATE_WAIT:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		break;
	case OBJ_STATE_WALK:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		break;
	case OBJ_STATE_RUN:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		break;
	case OBJ_STATE_ACTION:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		break;
	case OBJ_STATE_ACT:
		t_newAction.m_changeType = OSE_DIS_OLDACT;
		break;
	case OBJ_STATE_BEACT:
		t_newAction.m_changeType = OSE_DIS_NEWACT_TIMES;
		t_newAction.m_iPlayNum = 1;
		break;
	case OBJ_STATE_DEATH:
		t_newAction.m_changeType = OSE_DIS_NEWACT_LASTFRAME;
		t_newAction.m_iPlayNum = 1;
		break;
	default:
		return;
	}

	//@}

	m_pObj->change_skinMeshAction(t_newAction);
}

/************************************************************************/
/*！变换动作2
/************************************************************************/
void I_Npc::ChangeAction(int _nCurAct)
{
	m_nState = _nCurAct;
	m_nCurAct = _nCurAct;
	ChangeAction();
}



/**brief*****************************************************************/
/*!计算从地图格子的索引到世界坐标的位置
*
*	\param	_nX
*   \param  _nY
*			所在地图的X,Y坐标
*	\param  _X
*	\param  _Y
*			所在地图的格子的索引
/************************************************************************/
osVec2D FromIndexToVertex(int _nX, int _nY, int _X, int _Y)
{
	osVec2D t_VerCenter;
	t_VerCenter.x = (_X + (_nX << 7))*BASESIZE + (BASESIZE / 2);
	t_VerCenter.y = (_Y + (_nY << 7))*BASESIZE + (BASESIZE / 2);
	return t_VerCenter;
}

/**brief*****************************************************************/
/*!计算从世界地图格子的索引到世界坐标的位置
*	\param  _X
*	\param  _Y
*			所在世界地图的格子的索引
/************************************************************************/
osVec2D FromWorldIndexToVertex(int _X, int _Y)
{
	osVec2D t_VerCenter;
	t_VerCenter.x = _X * BASESIZE + (BASESIZE / 2);
	t_VerCenter.y = _Y * BASESIZE + (BASESIZE / 2);
	return t_VerCenter;
}

/**brief*****************************************************************/
/*!计算从本地地图格子的索引到世界格子的索引
*	\param	_nX
*   \param  _nY
*			所在地图的X,Y坐标
*	\param  _X
*	\param  _Y
*			所在地图的格子的索引
*	\param  _BaseMesh
*			在世界格子中的索引
/************************************************************************/
void FromLocalToWorld(int _nX, int _nY, int _X, int _Y,BaseMesh& _BaseMesh)
{
	_BaseMesh.nX = _X + (_nX << 7);
	_BaseMesh.nY = _Y + (_nY << 7);
	//_BaseMesh.nY = _Y + (_nY * 128);
	//_BaseMesh.nX = _X + (_nX * 128);
}

/**brief*****************************************************************/
/*!计算从世界地图格子的索引到本地格子的索引
*	\param  _X
*	\param  _Y
*			所在地图的格子的索引
*	\param	_nX
*   \param  _nY
*			所在地图的X,Y坐标
*	\return  _BaseMesh
*			在世界格子中的索引
/************************************************************************/
void FromWorldToLocal(int _X, int _Y, int& _nX, int& _nY, BaseMesh& _BaseMesh)
{
	_nX = _X >> 7;
	_nY = _Y >> 7;
	_BaseMesh.nX = _X % 128;
	_BaseMesh.nY = _Y % 128;
}

bool   gfEqual(  float _1, float _2  )
{
	return( fabs( _1 - _2 ) < 0.01f )?true:false;
}
/**brief*****************************************************************/
/*npc管理类构造函数
/************************************************************************/
CNpcManager::CNpcManager(void)
{
	m_VECNpcs.clear();
}

CNpcManager::~CNpcManager(void)
{
	Reset();
}

void deleteIt (I_Npc* _pNpc)
{
	delete _pNpc;
}

void CNpcManager::Reset()
{
	std::for_each(m_VECNpcs.begin(), m_VECNpcs.end(), deleteIt);
	m_VECNpcs.clear();
}

/**brief*****************************************************************/
/*! 初始化全部的npc
*	\param  _pchDirName
*			全部npc的ini文件的目录名
/************************************************************************/
bool CNpcManager::Create(char *_pchDirName)
{
	return true;
}

/**brief*****************************************************************/
/*！更新全部Npc的位置，在每一帧中使用
*	\param  _fTime
*			距离上一帧的时间差
/************************************************************************/

void CNpcManager::UpdatePos(float _fTime)
{
	for(WORD n = 0; n < m_VECNpcs.size(); ++n)
	{
		m_VECNpcs[n]->Move(_fTime);
	}  
}

/**brief*****************************************************************/
/*!根据名字得到npc指针
*	\param  _strName
*			要删除的npc的名字
/************************************************************************/
I_Npc*	CNpcManager::GetNpcFromName(s_string& _strName)
{
	for(WORD n = 0; n < m_VECNpcs.size(); ++n)
	{
		if(*m_VECNpcs[n] == _strName)
			return m_VECNpcs[n];
	}
	return NULL;
}

/**brief*****************************************************************/
/*!根据名字删除一个npc
*	\param  _strName
*			要删除的npc的名字
/************************************************************************/
void  CNpcManager::RemoveNpcFromName(s_string& _strName)
{
	std::vector<I_Npc*>::iterator t_it;
	for(t_it = m_VECNpcs.begin(); t_it != m_VECNpcs.end(); ++t_it)
	{
		if(*(I_Npc*)&t_it == _strName)
		{
			m_VECNpcs.erase(t_it);
			return;
		}
	}
}



