//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTGLoadReleaseMgr.h
 *
 *  His:      River created @ 2006-1-10
 *
 *  Des:      ���ڹ���ǰ���������ڵĵ�ͼ,�ĸ�Ӧ�õ���,�ĸ�Ӧ���˳��ڴ��
 *   
 * ������ȡ֮,������֮�� 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# pragma warning( disable : 173 )

//! ������ͼ�������õ�����ص����ݽṹ��
# include "fdTGQuadtree.h"
# include "../include/fdFieldStruct.h"
# include "fdNewFieldStruct.h"

# include "../../effect/include/fdRain.h"
# include "../../effect/include/fdlensflare.h"
# include "../GeoMipTerrain/CeoMipTerrainAll.h"
# include "../../effect/include/fdSpecialEffect.h"



typedef osc_newTileGround* osc_TileGroundPtr;


struct os_tgMTLoader
{
	s_string         m_szMapName;
	osc_TileGroundPtr   m_ptrTG;
	int             m_iMapIdx;

	//! �˵�ͼ������ɺ�, ֪ͨ�ϲ�����˵�ͼ����.
	BOOL             m_bFinishNotify;
public:
	os_tgMTLoader()
	{
		m_bFinishNotify = FALSE;
	}
};


//! TG�������У����ٱ��뱣֤��TG��Ŀ,����Ŀ��֤�����ڳ�������Զ����192�׵ľ���

# if __QTANG_VERSION__
  # define MINTG_INTGMGRMACRO       4
  # define MAXTG_INTGMGRMACRO       4
# else
  # define MINTG_INTGMGRMACRO       9
  # define MAXTG_INTGMGRMACRO       16
# endif 


/** \brief
 *  ���̵߳����ͼ������ݷ�װ����
 *
 *  �㷨����: ÿ����һ����ͼ,����һ��������ڲ�������,���Ƿ���Ҫ�������Ĳ���.
 *  ��: ���߳���ÿһ֡�е��ú���:frame_moveMLMgr
 *  ��: �����ͼ�б��д���Ҫ����ĵ�ͼ,�����߳�,�����ͼ.
 *  ��: ������һ����ͼ��,������ǰ���߳�,Ȼ�������߳��е��¼�.
 *  ��: ���߳̿��ܻ�رյ�ǰ�ĵ����̻߳��Ǽ���ִ�е�ǰ���߳�.
 *  
 *  �����ͼ���߳��ڵ�ͼ��������ʼ����һֱ����,ƽʱ��Ҫ��Sleep,������Ҫ����ĵ�ͼʱ.
 *  ��ʼ�����ͼ,������ɺ�,�������е��߳�.
 *  
 */
class osc_mapLoadThread : public com_thread
{
private:
	//! ��ǰ���ڵ���ĵ�ͼ�б�
	std::list< os_tgMTLoader >    m_sMapNeedLoad;

	//! ��ǰ������ĵ�ͼ.
	os_tgMTLoader                m_sCurLoadMap;

	//! ��ͼ������.
	osc_TGManager*               m_ptrTGMgr;

	//! ��ͼ������ͷŹ�����,�Ժ���������������֮�ϵ�TGManager.
	osc_tgLoadReleaseMgr*         m_ptrLRMgr;

	//! �Ƿ��ѵ�ͼ�����̵߳�event.
	static HANDLE                m_sEventStartMt;

	//! ��ͼ����������ģʽ��,ʹ���������֪ͨ�ϲ�����˵�ͼ����.
	HANDLE                      m_sEventMapLoadFinish;

	//! �߳��˳����¼�����m_sEventStartMt��ϣ���ȷ�����߳��˳���
	HANDLE                      m_sEventExitThread;

private:
	
	//! ��ͼ�����߳��Ƿ��ڿ���״̬���ڵȴ��̱߳����ѵ���Ϣ
	static HANDLE               m_sEventThreadFree;

public:
	//@{
	//! ȫ��ʹ�õ�critical_section.

	//! ���ƽ��Ͷ���Ҫ����ĵ�ͼ���ֺ�ָ��ʱ,ʹ�õ��ٽ���.
	static CRITICAL_SECTION   m_sMapNameCS;


	//! �ٽ紴������ʱ�õ�����Դ.
	static CRITICAL_SECTION   m_sParticleCS;

	//! �ٽ紴��meshʱ�õ�����Դ.
	static CRITICAL_SECTION   m_sMeshCS;

	//! �ٽ紴��osaʱ�õ�����Դ.
	static CRITICAL_SECTION   m_sOsaCS;

	//! �ٽ紴��billboard�õ�����Դ.
	static CRITICAL_SECTION   m_sBbCS;

	//! �ٽ�shader�����õ�����Դ.
	static CRITICAL_SECTION   m_sShaderCS;

	//! ����Effect��Ҫ��CS.
	static CRITICAL_SECTION   m_sDxEffectCS;

	//! �ٽ�shader�����õ�����Դ.
	static CRITICAL_SECTION   m_sShaderReleaseCS;


	//! �ٽ紴����Ч�õ�����Դ
	static CRITICAL_SECTION   m_sEffectCS;

	//! �����ڻ��ͼ�б����ʱ���õ����ٽ�����Դ���ϲ�����õ���ص���Ϣ
	static CRITICAL_SECTION   m_sActiveMapCS;


private:

	//! �Ӷ����е�����ͼ�б�.
	void                    pop_loadMap( void );  

	//! ���Ҫ�����ͼ�Ķ���.
	void                    clear_loadMap( void );

	//! �õ���ǰ�˵ĵ�ͼ.
	BOOL                    get_frontMap( os_tgMTLoader& _tgLoad ); 


public:
	osc_mapLoadThread();
	~osc_mapLoadThread();

	//! �����ڲ�ʹ�õ�ָ��
	void                    init_mtLoadMapMgr( 
		                      osc_TGManager* _mgr,osc_tgLoadReleaseMgr* _lrMgr );


	//! ÿһ֡�Զ��̵߳�ͼ������������д���
	void                    frame_moveMLMgr( void );

	/** \brief
	 *  ��������ָ��һ��Ҫ����ĵ�ͼ��
	 *
	 *  \param _mapName Ҫ����ͼ������.
	 *  \param _mapIdx �����ͼʱ,ʹ�õĵ�ͼ����.ATTENTION TO FIX:
	 *  \param _waitForFinish �Ƿ�ȴ�������߳���ɵ�ͼ�ĵ���,
	 *                  �˻��������������ʽ�ĵ�ͼ����.
	 */
	void                    push_loadMap( char* _mapName,
		                      int _mapIdx,BOOL _waitForFinish = FALSE );

	//! ��չ�����Ҫ����ĵ�ͼ,���ͷ�������ͼʱ��������������߳����ڵ����ͼ������
	void                    reset_loadMapMgr( void );

	/** \brief
	 *  �Ƿ��ǿɲ�����TG,����ʹ�õ�tg.
	 *
	 *  �ڵ�ͼ�����̵߳��û���ͼ������û�е�ǰtg.���tg����activeTg,
	 *  Ϊ�ϲ���Ⱦ��Ҫʹ�õ���tg.
	 */
	bool                    is_activeTg( int _idx );

	//! ��ǰ�Ƿ�������״̬,���ȴ����߳��ƽ�Ҫ����ĵ�ͼ.
	bool                    is_freeStatus( void );


	//! �õ�Ҫ����ĵ�ͼ������.
	void                    get_loadMapName( int _idx,char* _name,int _maxname );

	/**
	 *  ÿһ�������඼�������صĺ���.
	 *
	 *  ������������:
	 *  
	 */
	virtual DWORD            ThreadProc();
	virtual void ThreadExceptionProc(){}


	//! �����̵߳�ִ��
	void                    end_thread( void );
	
};




/** \brief
 *  �Ե�ͼ���й������,����ȷ����һ���ͼӦ�����ڴ���,��һ��Ҫ������һ���ͼ���ڴ�.
 *
 *  �˹��������뿼�����еĵ�ͼ�����뵽���ڴ��е�����.ά��һ��������ڴ��б�,ʹ����
 *  ���б�����tgMgr���������еĿɼ�����Ⱦ,ʹ������б����������������б�.
 *  
 *  ��ͼȫ�����뵽�ڴ��ǰ�Ӳ���ϵĵ�ͼ�ļ����뵽���ڴ���,����Щ�ڴ沢��ȥ�����豸���
 *  ������.
 *
 *  �йص�ͼ���룬�ͷţ��û��Ĺ���ȫ�����������ʵ��? ( �����ƶ��ȵ� )
 * 
 *  ���ϲ��ṩ��ȷ�Ͽ���ʹ�õ�TG�б�: ���������ڲ���������صĶ��̵߳���͵�ͼ�û��㷨.
 * 
 *  ATTENTION : ����ֻ���յ�ͼ�ĵؿ����ݺ��û�,�����ͼ����ϢĿǰ������������.
 *  
 *  ���̵߳����ͼ�ڳ�ʼ����ͼ��ʱ��,������ʽ�ĵ���.ֻ�������еĹ�����,�Ż�����һ���߳���
 *  ��̬�ĵ���
 *
 *  ���������Ŀǰ�����򵥣��ѳ���ͼ����������й������ŵ��ϲ��osc_TGManager����ȥ����
 *
 *  ����ȫ���ѵ�ͼ���뵽�ڴ��е�����,����ֻ������Ӧ�ĵ�ͼӲ������,����Ҫ��ʾ��ͼ��ʱ��,
 *  �ڵ�ͼ�����߳��м����ͼ,�������˵�ͼ��ص��豸�������.
 */
class osc_tgLoadReleaseMgr
{
	friend class    osc_mapLoadThread;

private:
	//! �����ͼ�����֣�����map\vilg
	s_string            m_strMapName;
	//! �м�ܵ���ָ��.
	osc_middlePipe*      m_ptrMidPipe;

	//! �����洢��Ӳ���ϵ����tg���ݵ�ָ��.
	osc_newTileGround*      m_ptrRealTg;
	//! ���������ܹ���tg��Ŀ��
	int                 m_iCurTgNum;

	/** \brief 
	 *  ά�������ϲ�������ɼ��Ժ���Ⱦ��tg�б�.
	 *
	 *  �����ڱ�������Ϊ��.��mgr������ĳ��tgʱ,�˴���Ӧ��tg��Ϊ��.
	 */
	osc_TileGroundPtr    m_ptrActTg[MAXTG_INTGMGRMACRO];

	/** \brief
	 *  ��Ҫ��̬�����tg��������.
	 * 
	 *  ��ǰ�����λ�÷����仯ʱ,ʹ�ô˽ṹ��Ϊ��ʱ����,�õ���Ҫ��̬����ĵ�ͼ�б�.
	 */
	os_tileIdx          m_vecNeedLoadTile[MAXTG_INTGMGRMACRO];
	//! ��Ҫ���´����

	/** \brief
	 *  �Ƿ�ѵ�ͼȫ���ĵ��뵽�ڴ���.
	 *
	 *  �������TotalLoad,���ڲ�ʹ��MAXTG_INTGMGR��tg��Ϊ�ڴ�����tg.
	 *
	 */
	bool                m_bTLoadInMem;

	//! ��ǰӦ���ͷŵ�tgָ���б�.
	int                 m_vecTgNeedRelease[MAXTG_INTGMGRMACRO];
	int                 m_iNeedRelTgNum;

	//! Ӧ���û����ڴ��tgָ���б�
	os_tileIdx           m_vecTgNeedSwapIn[MAXTG_INTGMGRMACRO];
	int                 m_iNeedSwapInNum;
	//! �û����ڴ��tg����ǰ�漸����Ҫ�������롣�Ժ���Ż�Ϊʹ�ò�ͬ�ĵ����Ż�����.
	int                 m_iBlockLoad;

	//! ���ڵ����ͼ���ݵ��̹߳�������
	osc_mapLoadThread    m_sThreadMgr;


	/** \brief
	 *  X,Z�����ϵ�ͼ����������ֵ,��ֵΪ��ʾ���ӵ�ǣ��ֵ.
	 *  ��64Ϊ��׼,Ϊ64�ı���.
	 */
	int              m_iMaxX;
	int              m_iMaxZ;

	//! ������ͼ��x,z��������ͼ��������Ŀ��
	int              m_iMaxXTgNum;
	int              m_iMaxZTgNum;

	//@{ 
	//  �������ݽ����������λ��,�����㵱ǰӦ������Щ��ͼӦ���û����ڴ����
	//  ����Ϊ����ĵ�ͼ.
	//! ��һ֡����ڵ�ͼ�ϵ�λ��.
	osn_inTgPos       m_eLFTgPos;
	//! ��һ֡������ڵ�tg.
	os_tileIdx        m_sLFCamInIdx;

	//! ��ǰ֡������ڵ�x,z��ײ����λ��.
	int              m_iCamInX,m_iCamInZ;



private:
	//! �ڲ�����ض��߳����ݽṹ.

	//  �õ���ͼ��������ʼ����λ����Ϣ.
	void             init_insertInfo( os_sceneLoadStruct* _sl );

	/** \brief
	 *  ���������λ�ã��������ǰ֡��Ҫ�����ͼ���б�.
	 *
	 *  \param return bool ����ֵ��ʾ���Ƿ���Ҫ�Ե�ǰ����ʾ��ͼ������û�.
	 *  
	 */
	bool             get_tgNeedInMemList( osVec3D& _pos );


	//! ������Ҫ���ڴ����û����ĵ�ͼͼ���б� 
	void             process_needSwapOutTgList( void );

	/** \brief
	 *  ���ݴ���ĵ�ͼ�����б����������Ҫ�ͷŵĵ�ͼ����
	 *
	 *  �˺������ڶ��߳�״̬�µ����������ƶ�.
	 */
	void             spacemove_swapOutMap( s_string* _mapList );

	//! ������Ҫ���ڴ��д��ڣ�Ŀǰ���ڴ����ֲ����ڵ�ͼ���б� 
	void             process_needSwapInTgList( void );

	//! ��Ӧ�ô������ڴ�,�ֲ����ڴ��tg�ƽ���������߳�,����.
	bool             push_needSwapInMap( void );


	//! ���ݵ�ǰ���������Ҫ�û���ĵ�ͼ�б�����
	bool             sort_needSwapInMapList( osc_camera* _cam );



	/** \brief
	 *  ������ϲ���õĵ�ͼ�б�,������ĵ�ͼ�б�.
	 *
	 */
	void                get_activeTgList( void );

	//@{

	//! �õ���ǰtg��ΧһȦ��tg,��8������Щtg�϶���Ҫ���뵽�ڴ档
	void                get_surroundTG( os_tileIdx* _mapIdx,int _my,int _mx );


	//! ���ݵ�ǰ��tg�е�λ��,�����뵱ǰtg�ڶ�Ȧ������.
	void                get_secSurTG( os_tileIdx* _mapIdx,int _my,int _mx );

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
	bool                get_loadSceneName( 
		                 s_string* _nameArr,os_sceneLoadStruct* _sl );


	/** \brief
	 *  ���̰߳�ȫ�ĵ�ͼ�������.
	 *
	 *  �滻TGManager�е���غ���.
	 */
	bool      threadSafe_loadNMap( char* _nmapFname,osc_TileGroundPtr _tg );


	/** \brief
	 *  ���������߳����ڵ����ͼ,��ֹ��ǰ�ĵ�ͼ����״̬.
	 *
	 */
	void                 stop_threadMapLoading( void );


public:
	osc_tgLoadReleaseMgr();
	~osc_tgLoadReleaseMgr();

	//! 
	void                init_tgLRMgr( osc_TGManager* _tgMgr );

	/** \brief
	 *  �������������������õ�Ӧ�ÿɼ��ĵ�ͼ�б�, ����ĵ�ͼ�ұ�.
	 *
	 *  ÿһ֡��Ҫ��������������ں�̨�������ͼ�ĵ���.
	 *  �����ϲ㵱ǰ���õĵ�ͼָ���б�Ϳ��õĵ�ͼ��Ŀ.
	 *
	 *  1: ���������λ�ã��������ǰ֡������ڵ�Tg��Tg�ڵķ�λ,
	 *     ����һ֡�洢�����ݱȽ�,����Ƿ���Ҫ�û���ͼ.
	 *  
	 *  2: �����û��ĵ�ͼ���ƽ�����ͼ�����̡߳�
	 *
	 *  3: ����ϲ����ʹ�õĵ�ͼ�б� ���ص�ͼ������
	 */
	int                 frame_getInViewTg( osc_camera* _cam,
		                  osc_TileGroundPtr* _tgList,int _maxNum );

	//! ��ʼ����ͼ�󣬵õ���ǰ����ĵ�ͼ�б���ʱ��û��������룬���Լ��뵥���ĺ�����
	int                 init_getActMapList( osc_TileGroundPtr* _tgList,int _maxNum );

	/** \brief
	 *  ���ε����ͼ�ĺ�����
	 *
	 *  �൱�ڽ�����ԭ����osc_TGManager�е�load_scene����.
	 *
	 *  \param _sl �����ͼ�Ľṹ��
	 *  \param _maxX,_maxZ ��ͼ��x,z�����ϵ�����ͼ������
	 *  
	 */
	bool                load_scene( os_sceneLoadStruct* _sl,int _maxX,int _maxZ );

	//! �����ͷŵ�ǰ�ĵ�ͼ�ڴ��������
	void                release_tgLRMgr( void );


	//! �����ƶ��Ĵ��������˺�����
	bool                space_moveMapLoad( os_sceneLoadStruct* _sl,int _maxX,int _maxZ );


};

