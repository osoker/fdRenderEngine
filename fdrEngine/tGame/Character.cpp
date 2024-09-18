//===================================================================
/** \file  
*  Filename: Character.cpp
*  Desc:   ���������ɫ��ʵ���ļ�
*  His:      ���� @ 1/12 2004 16:18
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


//! �Ƿ�ʹ��box���Եƹ⡣
# define LIGHT_TEST 1

/**brief*****************************************************************/
/*!���캯��                                                          
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
/*!��������                                                          
/************************************************************************/
I_Hero::~I_Hero(void)
{
}

/**brief*****************************************************************/
/*!�õ���ǰλ����������ͼ�е�����,�����ظ��ӵ����ĵ�
/************************************************************************/
void I_Hero::GetCurMeshIndex()
{
	//�����������С���ĺ������õ��������ڵ�����һ�����ͼ
	//!���õ�����������һ����ͼ�����С�

	os_pickTileIdx t_TileIdx;
	if(!g_sceneMgr->get_sceneTGIdx(t_TileIdx,m_verPos))
		return;
	m_nFieldX = t_TileIdx.m_tgIdx.m_iX;
	m_nFieldY = t_TileIdx.m_tgIdx.m_iY;
	m_sBaseMesh.nX = t_TileIdx.m_colTileIdx.m_iX;
	m_sBaseMesh.nY = t_TileIdx.m_colTileIdx.m_iY;
	//m_fHei = t_TileIdx.m_fTileHeight;



}




//! ����Ӣ�۵�ʱ�����Ӣ�۵����еĶ���,������������л�Ӣ�۵Ķ�������һ������.
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

//! ����ǰCharacter��С���ƶ�������ƽ�����˶������λ�á�
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
/*!������Χ�İ˸�����
/************************************************************************/
void I_Hero::FindMesh(CNpcManager *_pNpcManager, float _fTime)
{
	//River added @ 2004-4-10:
	return;

}

/**brief*****************************************************************/
/*	!�����û���������������·��.
*	\param	_nX
*   \param  _nY
*			Ŀ���ͼ��X,Y����
*	\param  _X
*	\param  _Y
*			Ŀ���ͼ�ĸ��ӵ�����
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
	// River added @ 2004-2-5 : ���ڲ���A*�㷨��

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

	//!��Ϊ���ص�·���ĵ�һ����������ʼλ��,����Ҫ�ѵ�һ������ȥ����
	//m_VECPath.pop_front();

	//!�õ���һ�����ӵ����ĵ�,
	osVec3D		t_VerCenter;


	if( m_VECPath.empty() )
		osassert( false );
	osVec2D  t_Ver2D = FromWorldIndexToVertex( m_VECPath.front()->nX, m_VECPath.front()->nY);

	t_VerCenter.x = t_Ver2D.x;
	t_VerCenter.y = 0.0f;
	t_VerCenter.z = t_Ver2D.y;

	//!���㷽��Ƕ�
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
/*	!����λ��,��ÿһ֡��ʹ��
*	\param	_fTime
*			������һ֡��ʱ���
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

	//!�õ���ǰλ����������ͼ�и��ӵ�����

	//!�������һ���ڵ㣬�����¼��㷽�򣬲�ɾ������ڵ�
	osVec3D		t_VerCenter;
	osVec2D  t_Ver2D = FromWorldIndexToVertex(m_VECPath.front()->nX,m_VECPath.front()->nY);

	g_sceneMgr->get_tilePos( m_VECPath.front()->nX,m_VECPath.front()->nY,t_VerCenter );
	
	//@{
	// Ipedo added @ 2005-1-25 : ���ڼ�¼��һ���ͼ���ĵ㡣
	// ��ʼֵΪ���ﵱǰλ�õ�����
	static osVec3D t_preCenter = m_verPos;
	//@}

	float dist = D3DXVec3Dot(&m_vCurDir,&t_VerCenter);
	float dist2 = D3DXVec3Dot(&m_vCurDir,&m_verPos);

	// �������һ���ڵ㣬�����¼��㷽�򣬲�ɾ������ڵ�
	if(dist2 >= dist)
	{

		float t_d = dist2 - dist;

		osVec3D		t_VerPower, t_VerNext;

		WORD  count = m_VECPath.count();
		for(WORD s = 1; s < count; s++)
		{
			//!�õ���һ�����ӵ����ĵ�,
			t_Ver2D = FromWorldIndexToVertex(m_VECPath[s].nX,m_VECPath[s].nY);

			// �õ�yֵ,����ƽ˳����.
			t_VerNext.x = t_Ver2D.x;
			t_VerNext.y = 0.0f;
			t_VerNext.z = t_Ver2D.y;

			// �õ��߶�.
			if( t_VerNext.x < 0 ) t_VerNext.x = 0;
			if( t_VerNext.z < 0 ) t_VerNext.z = 0;
			g_sceneMgr->get_sceneTGIdx( t_res,t_VerNext );
			t_VerNext.y = m_fHei + t_res.m_fTileHeight;

			t_Ver2D = FromWorldIndexToVertex(m_VECPath[s-1].nX,m_VECPath[s-1].nY);
			t_VerPower.x = t_Ver2D.x;
			t_VerPower.y = 0.0f;
			t_VerPower.z = t_Ver2D.y;
			// �õ��߶�.
			if( t_VerPower.x < 0 ) t_VerPower.x = 0;
			if( t_VerPower.z < 0 ) t_VerPower.z = 0;
			g_sceneMgr->get_sceneTGIdx( t_res,t_VerPower );
			t_VerPower.y = m_fHei + t_res.m_fTileHeight;

			//!���㷽��Ƕ�
			m_vCurDir = t_VerNext - t_VerPower;
			D3DXVec3Normalize(&m_vCurDir,&m_vCurDir);

			float t_dist = D3DXVec3Dot(&m_vCurDir,&t_VerNext);
			float t_dist2 = D3DXVec3Dot(&m_vCurDir,&m_verPos);

			//@{
			// Ipedo added @ 2005-1-25 : ����һ���ͼ���ĵ㸳ֵ��t_preCenter��
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

		//!��������յ㣬��ֹͣ
		if(m_VECPath.empty())
		{
			//!��������һ���ڵ㣬�������λ���ƶ����ڵ�����ĵ�
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

		// ���ݵ�ǰ��λ�õõ���ǰ��idx.
		g_sceneMgr->get_detailPos( m_verPos );
		m_verPos.y += 0.08f;
		m_pObj->frame_move( m_verPos, m_fAngle);
		GetCurMeshIndex();
		return;
	}

	// River @ 2006-3-18: ͨ��������ƶ�
	//!�ƶ�����
	D3DXVec3Add(&m_verPos, &m_verPos, &(m_vCurDir * (m_fSpeed * _fTime)));

	//@{
	// Ipedo added @ 2005-1-25 : ʵ�������߶�ʱ�߶ȵĲ�ֵ��
	// �㷨��������������ǰ�������ڵ����ĸ߶�����ֵ��������֮��ľ�����ϵ��
	os_pickTileIdx      t_resWT;
	g_sceneMgr->get_sceneTGIdx( t_resWT,t_preCenter );
	// ��һ�ڵ����ĸ߶�
	float t_curHeight = t_resWT.m_fTileHeight;
	g_sceneMgr->get_sceneTGIdx( t_resWT,t_VerCenter );
	// ��һ�ڵ����ĸ߶�
	float t_nextHei = t_resWT.m_fTileHeight;
	// ǰ�������ڵ�ľ���
	float t_tolDis = osVec3Length(&(t_VerCenter-t_preCenter));
	// ��ǰλ�õ㵽��һ�ڵ����
	float t_curDis = osVec3Length(&(m_verPos-t_preCenter));

	// ��ֵϵ��
	float t_tmpCoeff;
	// �жϵ�����һ�ڵ�򳬹���һ�ڵ㣬��ϵ����Ϊ1
	if(t_tolDis==0||t_curDis>t_tolDis)
		t_tmpCoeff = 1;
	else{
		t_tmpCoeff = t_curDis/t_tolDis;
	}
	// ����߶�Ϊ��ֵ��������ر�߶ȣ�������������߶�
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
	

	//!������õ���ǰ�������ڵĵ�ͼ����͸����������͵�ǰ���ڸ��ӵĸ߶�ֵ
	GetCurMeshIndex();

	hero_areaMove();
	//osDebugOut( "hero move" );


}

/**brief*****************************************************************/
/* !�任װ��
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
/*!�任����
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

//! �������ﶯ��������frameMovel
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
/*���任����2
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
	// ��������7����λ��mesh��skin
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
/*	!��һ��ini�ļ�װ��һ��hero
*	\param	_chPath
*			ini�ļ���·����
*	\param	_pHero
*			װ���hero
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

	//!��ȡӢ�۵�����
	if( !t_iniFile.ReadLine( NAME,t_str,256) )
	{
		return false;
	}
	_pHero->m_strName		= t_str;

	//!Ӣ�����ڵ�Ŀ¼��
	ZeroMemory(t_str,256);
	ZeroMemory(t_chDir,256);
	if( !t_iniFile.ReadLine( DIRTCTION,t_str,256) )
	{
		return false;
	}

	g_szChrName = t_str;

	sprintf(t_chDir, "%s%s",SHADEDIR,t_str);
	strcpy( t_smInit.m_strSMDir,t_chDir );

	//!��ȡ��ʼ���Ķ���������
	_pHero->m_nCurAct = 0;
	//t_smInit.m_iActIdx = _pHero->m_nCurAct;

	ZeroMemory(t_chDir,256);


	///////////////////////////////////////////////////////////////////
	// ����CHA�ļ���
	// ���Ե���*.cha�ļ���
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
			osassertex(false,"CHA�ļ��汾����ȷ!��ʹ�ù���ת�������°汾!");
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
		// ��bfs�ļ�����������ص�����
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



	//!ȫ��һ���ж��ٶ���
	t_smInit.m_iTotalActTypeNum = _pHero->m_nTotalActCount;

	//!�����Ϊ��������
	t_smInit.m_smObjLook.m_iPartNum = _pHero->m_nPartNum;

	//!��ʼ��װ���ṹ
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

	//!��ȡ��ʼ��λ��
	if( !t_iniFile.ReadLine( FIELDX,&_pHero->m_nFieldX) )
	{
		return false;
	}
	if( !t_iniFile.ReadLine( FIELDY,&_pHero->m_nFieldY) )
	{
		return false;
	}
	//!��ȡ��ʼ��λ�õĸ߶ȵ����̶��ڵ���ĸ߶�
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
	// ���baseMesh����128������
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

	//!��ʼ����ת�ĽǶ�
	_pHero->m_fAngle = 0.0f;
	t_smInit.m_fAngle = _pHero->m_fAngle;

	//!Ӣ�����п��õ�װ���������ڵ�·��

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
	// ��������ʾ���⡣
	//_pHero->m_pObj->playSwordGlossy( true );

	


	if(!_pHero->m_pObj)
	{
		return false;
	}

	// ����װ�����
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
	// �õ���ǰ��ɫӦ���ƶ����ٶ�.
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
	// ����װ��
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
/*	!npc���캯��
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
/*!�õ���ǰλ����������ͼ�е�����,�����ظ��ӵ����ĵ�
/************************************************************************/
void I_Npc::GetCurMeshIndex()
{
	if(m_nState == OBJ_STATE_DEATH)
		return;

	//�����������С���ĺ������õ��������ڵ�����һ�����ͼ
	//!���õ�����������һ����ͼ����С�
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
/*	!����·��,����·�����յ���Ҫ����Ѱ��·��
*	!ʹ����ini�ļ���Ԥ����õ������ڵ�������·��
/************************************************************************/
bool I_Npc::FindPath()
{
	if(m_nState == OBJ_STATE_DEATH)
		return true;

	//m_VECPath.clear();
	//!Ŀǰֻ�������ڵ�֮ǰѰ·�����Բ���Ҫ����Ŀ�Ľڵ�
	//!������ʹ���갲�ṩ�Ĳ���·���ĺ���
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
/*	!����λ��,��ÿһ֡��ʹ��
*	\param	_fTime
*			������һ֡��ʱ���
/************************************************************************/
void I_Npc::Move(float _fTime)
{

}

/************************************************************************/
/*!�任����
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
/*���任����2
/************************************************************************/
void I_Npc::ChangeAction(int _nCurAct)
{
	m_nState = _nCurAct;
	m_nCurAct = _nCurAct;
	ChangeAction();
}



/**brief*****************************************************************/
/*!����ӵ�ͼ���ӵ����������������λ��
*
*	\param	_nX
*   \param  _nY
*			���ڵ�ͼ��X,Y����
*	\param  _X
*	\param  _Y
*			���ڵ�ͼ�ĸ��ӵ�����
/************************************************************************/
osVec2D FromIndexToVertex(int _nX, int _nY, int _X, int _Y)
{
	osVec2D t_VerCenter;
	t_VerCenter.x = (_X + (_nX << 7))*BASESIZE + (BASESIZE / 2);
	t_VerCenter.y = (_Y + (_nY << 7))*BASESIZE + (BASESIZE / 2);
	return t_VerCenter;
}

/**brief*****************************************************************/
/*!����������ͼ���ӵ����������������λ��
*	\param  _X
*	\param  _Y
*			���������ͼ�ĸ��ӵ�����
/************************************************************************/
osVec2D FromWorldIndexToVertex(int _X, int _Y)
{
	osVec2D t_VerCenter;
	t_VerCenter.x = _X * BASESIZE + (BASESIZE / 2);
	t_VerCenter.y = _Y * BASESIZE + (BASESIZE / 2);
	return t_VerCenter;
}

/**brief*****************************************************************/
/*!����ӱ��ص�ͼ���ӵ�������������ӵ�����
*	\param	_nX
*   \param  _nY
*			���ڵ�ͼ��X,Y����
*	\param  _X
*	\param  _Y
*			���ڵ�ͼ�ĸ��ӵ�����
*	\param  _BaseMesh
*			����������е�����
/************************************************************************/
void FromLocalToWorld(int _nX, int _nY, int _X, int _Y,BaseMesh& _BaseMesh)
{
	_BaseMesh.nX = _X + (_nX << 7);
	_BaseMesh.nY = _Y + (_nY << 7);
	//_BaseMesh.nY = _Y + (_nY * 128);
	//_BaseMesh.nX = _X + (_nX * 128);
}

/**brief*****************************************************************/
/*!����������ͼ���ӵ����������ظ��ӵ�����
*	\param  _X
*	\param  _Y
*			���ڵ�ͼ�ĸ��ӵ�����
*	\param	_nX
*   \param  _nY
*			���ڵ�ͼ��X,Y����
*	\return  _BaseMesh
*			����������е�����
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
/*npc�����๹�캯��
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
/*! ��ʼ��ȫ����npc
*	\param  _pchDirName
*			ȫ��npc��ini�ļ���Ŀ¼��
/************************************************************************/
bool CNpcManager::Create(char *_pchDirName)
{
	return true;
}

/**brief*****************************************************************/
/*������ȫ��Npc��λ�ã���ÿһ֡��ʹ��
*	\param  _fTime
*			������һ֡��ʱ���
/************************************************************************/

void CNpcManager::UpdatePos(float _fTime)
{
	for(WORD n = 0; n < m_VECNpcs.size(); ++n)
	{
		m_VECNpcs[n]->Move(_fTime);
	}  
}

/**brief*****************************************************************/
/*!�������ֵõ�npcָ��
*	\param  _strName
*			Ҫɾ����npc������
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
/*!��������ɾ��һ��npc
*	\param  _strName
*			Ҫɾ����npc������
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



