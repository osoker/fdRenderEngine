#include "gc_charSound.h"



#define SoundFileName "sound\\g_charAct_fd.snd"

#define EffectFileName "character\\g_charAct_fd.eff"

DWORD gc_charSoundMgr::sActRunHashName = 0;
gc_charSoundMgr::gc_charSoundMgr()
{
	m_ptrSoundMgr = NULL;
	sActRunHashName = string_hash("单手跑步");

}
bool gc_charSoundMgr::init_charSoundMgr()
{
	m_ptrSoundMgr = ::get_soundManagerPtr();
	bool rt = LoadActSndFile(SoundFileName);
	osassertex(rt,"全局声音文件加载出错!");

	mptrSceneMgr = ::get_sceneMgr();

	rt = LoadActEffFile(EffectFileName);

	if (!rt)
	{
		osDebugOut("全局人物特效文件加载出错!");
	}
	return rt;

}
void gc_charSoundMgr::register_skinMesh( I_skinMeshObj* _char ,std::string _meshName)
{
	bool rt = false;
	for (size_t i =0; i < m_vecCharSounds.size();i++)
	{
		if (!strcmp(m_vecCharSounds[i].m_szCharName,_meshName.c_str()))
		{
			CallBackUnit cbUnit;
			cbUnit.mpSkin = _char;
			cbUnit.mActData = m_vecCharSounds[i];
			mCBListSound.push_back(cbUnit);
			rt = true;
		}
	}
	if (!rt)
	{
		osDebugOut("声音动作注册失败!");
	}

	for (size_t i =0; i < mCharEffectList.size();i++)
	{
		if (!strcmp(mCharEffectList[i].mChaName.c_str(),_meshName.c_str()))
		{
			CallBackUnitEffect cbUnit;
			cbUnit.mpSkin = _char;
			cbUnit.mActData = mCharEffectList[i];
			mCBListEffect.push_back(cbUnit);
			rt = true;
		}
	}
	if (!rt)
	{
		osDebugOut("人物特效动作注册失败!");
	}	
}
void gc_charSoundMgr::unregister_skinMesh( I_skinMeshObj* _char )
{
	std::vector<CallBackUnit>::iterator it = find(mCBListSound.begin(),mCBListSound.end(),_char);
	if ( it != mCBListSound.end())
	{
		mCBListSound.erase(it);
	}
	std::vector<CallBackUnitEffect>::iterator it2 = find(mCBListEffect.begin(),mCBListEffect.end(),_char);
	if ( it2 != mCBListEffect.end())
	{
		mCBListEffect.erase(it2);
	}
}
#define epsilon2 0.02f
static inline bool equal2(float a, float b)
{
	const float d = a - b;
	if (d<epsilon2 && d>-epsilon2) return true;
	else return false;
}

void gc_charSoundMgr::framemove_charSoundMgr()
{
	if (!m_ptrSoundMgr)
	{
		return;
	}
	char CurActName[32];
	DWORD CurActNameHash;
	float ActPercent = 0;
	os_initSoundSrc soundSrc;
	soundSrc.m_flMinDistance = 2.0f;
	soundSrc.m_flMaxDistance = 1000.0f;
	
	for( size_t SkinNr = 0; SkinNr < mCBListSound.size(); SkinNr++ )
	{
		osassert(mCBListSound[SkinNr].mpSkin);

		mCBListSound[SkinNr].mpSkin->get_curPlayActIdx( CurActName );
		CurActNameHash = string_hash(CurActName);
		if (CurActNameHash == sActRunHashName)
		{
			return;
		}
		float temp;
		mCBListSound[SkinNr].mpSkin->get_posAndAgl(soundSrc.m_vPosition,temp);
		ActPercent = mCBListSound[SkinNr].mpSkin->get_actionPlayPercent();
		//osassert(mCBList[i].m_dwActCount = mCBList[i].m_vecActSounds.size());

		for( DWORD ActNr=0;ActNr<mCBListSound[SkinNr].mActData.m_dwActCount;ActNr++ )
		{
			if (CurActNameHash == mCBListSound[SkinNr].mActData.m_vecActSounds[ActNr].mActNameHash)
			{
				gcs_ACTSoundData& ActSound =  mCBListSound[SkinNr].mActData.m_vecActSounds[ActNr];
				for (size_t SoundNr = 0 ; SoundNr < ActSound.mSoundItemList.size();SoundNr++)
				{
					if (equal2(ActSound.mSoundItemList[SoundNr].m_fStartTime,ActPercent))
					{
						float NewRate =  (float)OSRAND/RAND_MAX;
						if (ActSound.mSoundItemList[SoundNr].m_fPlayChance >= NewRate)
						{
							m_ptrSoundMgr->PlayFromFile(ActSound.mSoundItemList[SoundNr].m_szSound,
								&soundSrc,false);
						}
					}
				}
			}
		}
	}
//	char CurActName[32];
//	DWORD CurActNameHash;
//	float ActPercent = 0;
	osVec3D EffectPos;


	for( size_t SkinNr = 0; SkinNr < mCBListEffect.size(); SkinNr++ )
	{
		osassert(mCBListEffect[SkinNr].mpSkin);

		mCBListEffect[SkinNr].mpSkin->get_curPlayActIdx( CurActName );
		CurActNameHash = string_hash(CurActName);

		float temp;
		mCBListEffect[SkinNr].mpSkin->get_posAndAgl(EffectPos,temp);
		ActPercent = mCBListEffect[SkinNr].mpSkin->get_actionPlayPercent();
		//osassert(mCBList[i].m_dwActCount = mCBList[i].m_vecActSounds.size());

		for( DWORD ActNr=0;ActNr<mCBListEffect[SkinNr].mActData.mActEffectList.size();ActNr++ )
		{
			if (CurActNameHash == mCBListEffect[SkinNr].mActData.mActEffectList[ActNr].mActNameHash)
			{
				ACTEFFFECT& ActEffect=  mCBListEffect[SkinNr].mActData.mActEffectList[ActNr];
				for (size_t SoundNr = 0 ; SoundNr < ActEffect.mEffectList.size();SoundNr++)
				{
					if (equal2(ActEffect.mEffectList[SoundNr].mPercent,ActPercent))
					{
						float NewRate =  (float)OSRAND/RAND_MAX;
						// 小张没有实现，所有这没有加，
						//if (ActEffect.mEffectList[SoundNr].mRate >= NewRate)
						//{
						mptrSceneMgr->create_effect(ActEffect.mEffectList[SoundNr].mEffectName,EffectPos);

						//}
					}
				}
			}
		}
	}
}

bool gc_charSoundMgr::LoadActSndFile( const char* _szSndFile )
{
	guard;
	char szFileFlag[4] = {0};
	DWORD dwFileVer = 0;
	DWORD dwCharCnt = 0;
	gcs_charSound cs;
	gcs_actSound  as;

	m_vecCharSounds.clear();

	//=================
	if (!file_exist((char * )_szSndFile))
	{
		return false;
	}
	int t_iGBufIdx;
	BYTE*    t_fstart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf((char *) _szSndFile,t_fstart,TMP_BUFSIZE );
	osassertex( t_iSize > 0,va("打开文件<%s>失败..\n",_szSndFile ) );

	READ_MEM_OFF( szFileFlag,t_fstart,sizeof( szFileFlag ) );
	READ_MEM_OFF( &dwFileVer,t_fstart,sizeof( DWORD ) );

	READ_MEM_OFF( &dwCharCnt,t_fstart,sizeof( DWORD ) );

	for ( DWORD i=0; i<dwCharCnt; i++ )
	{
		cs.Clear();
		READ_MEM_OFF( cs.m_szCharName,t_fstart,sizeof( char )*32 );
		READ_MEM_OFF( &cs.m_dwActCount,t_fstart,sizeof( DWORD ) );

		for ( DWORD dwAct=0; dwAct<cs.m_dwActCount; dwAct++ )
		{
			READ_MEM_OFF( as.m_szActName,t_fstart,sizeof( char )*32 );
			READ_MEM_OFF( as.m_vecSounds,t_fstart,sizeof( gcs_sound )*MAX_ACTSOUNDS );

			gcs_ACTSoundData asData;
			asData = as;

			cs.m_vecActSounds.push_back( asData );
		}

		m_vecCharSounds.push_back( cs );
	}
	END_USEGBUF( t_iGBufIdx );
	return true;

	unguard;
}
gcs_ACTSoundData* gc_charSoundMgr::get_CharRunSoundList(std::string _chaPath)
{
	for (size_t i = 0; i < m_vecCharSounds.size();i++)
	{
		if ( !strcmp(m_vecCharSounds[i].m_szCharName ,_chaPath.c_str()) )
		{
			for (size_t iActNr = 0; iActNr < m_vecCharSounds[i].m_vecActSounds.size();iActNr++)
			{
				if (m_vecCharSounds[i].m_vecActSounds[iActNr].mActNameHash == sActRunHashName)
				{
					return &m_vecCharSounds[i].m_vecActSounds[iActNr];
				}
			}
		}
	}
	return NULL;
}

bool gc_charSoundMgr::LoadActEffFile( const char* _szSndFile )
{
	guard;
	if (!file_exist((char * )_szSndFile))
	{
		return false;
	}
	int t_iGBufIdx;
	BYTE*    t_fstart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf((char *) _szSndFile,t_fstart,TMP_BUFSIZE );
	osassertex( t_iSize > 0,va("打开文件<%s>失败..\n",_szSndFile ) );

	ChaEffectHeader ChaHeader;
	READ_MEM( &ChaHeader,t_fstart,sizeof( ChaEffectHeader ) );
	if (ChaHeader.mID != CHAEFFECTID)
	{
		osassert("特效文件格式不正确!");
		return false;
	}
	//! 加载每个人物
	SChaItem* pChaItemList = (SChaItem*)(t_fstart + ChaHeader.mChaEffectOfs);
	for (int i = 0; i < (int)ChaHeader.mChaEffectNr; i++)
	{

		CChaActEffect ChaActEffectData;
		ChaActEffectData.mChaName = pChaItemList[i].mCharName;
		SActItem* pActItemList = (SActItem* )(t_fstart + pChaItemList[i].mActionOfs);
		//! 加载每个人物的每个动作
		for (int iActNr = 0; iActNr < (int)pChaItemList[i].mActionNr;iActNr++)
		{

			ACTEFFFECT ChaActData;

			ChaActData.mActName = pActItemList[iActNr].mActName;
			ChaActData.mActNameHash = string_hash(ChaActData.mActName);


			//! 加载每个动作的特效
			SEffectItem* pEffectItem = (SEffectItem*)(t_fstart + pActItemList[iActNr].mEffectOfs);
			for (int iEffect = 0; iEffect < (int)pActItemList[iActNr].mEffectNr;iEffect++)
			{
				ChaActData.mEffectList.push_back(pEffectItem[iEffect]);
			}
			ChaActEffectData.mActEffectList.push_back(ChaActData);
		}
		mCharEffectList.push_back(ChaActEffectData);
	}

	END_USEGBUF( t_iGBufIdx );

	
	return true;

	unguard;
}
gc_charSoundMgr g_CharSoundMgr;
gc_charSoundMgr* g_pCharSoundMgr = &g_CharSoundMgr;