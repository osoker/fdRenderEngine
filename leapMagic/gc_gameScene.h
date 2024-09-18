///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_gameScene.h
 *
 *  Desc:     ��Ϸ�����Ļ��࣬��Ϸ���õ���ÿһ������������Ҫ�����������������������ͬ
 *            �ĳ�����ص���Ϣ��
 * 
 *  His:      River created @ 2004-4-12 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "stdafx.h"
# include "gc_command.h"
# include "gc_camera.h"

# include "FindPathMap.h"


//! �������ip��ַ�ַ�����
# define  MAX_IPSTRSIZE		64



class gc_creatureMgr;
class lm_soundMgr;
class lm_meManager;
class gc_itemMgr;
class CStateIconMgr;

/** \brief
 *  ��һ�γ�ʼ����Ϸ����ʱ��Ҫ���Ľṹ��
 */
struct gcs_sceneOnceInit
{
	HWND       m_hWnd;
	HINSTANCE   m_hIns;

	int        m_iScrWidth;
	int        m_iSceHeight;

	//@{
	//! ������صĳ�ʼ����
	char       m_szIp[MAX_IPSTRSIZE];
	DWORD      m_dwPort;
	//@}
};


/** \brief
 *  ħ��Demo��Ϸ�ڳ����Ļ��࣬ʹ������������Ƶ�ͼ�����ĳ�ʼ������ʾ.
 *  �������������ά�����Ͷ�ά�Ľ�����س�����
 *  
 */
class gc_gameScene : public lm_gameEntity
{

public:
	//! ȫ�ֵ������
	static gc_camera*               m_ptrCamera;

	//! ��ͼѰ·�㷨���
	static CFindPathMap*            m_ptrPathFinder;

	//! TEST CODE:
	static I_skinMeshObj*           m_ptrHeroSkinObj;
	static I_skinMeshObj*           m_ptrHeroSkinObj2;

protected:
	//@{
	//!  ���ڵĳ�����Ϣ��
	static HWND						m_hWnd;
	static HINSTANCE				m_hIns;
	static int						m_iScrWidth;
	static int						m_iScrHeight;
	//@} 


	//! ��������ܵĽӿڡ�
	static I_resource*				m_ptrResource;

	//! ��ǰ��socket������.
	static I_socketMgr*				m_ptrSocket;

	//! ȫ�ֵļ��ܹ�����
	static lm_meManager*            m_ptrMeMgr;

	//! ȫ�ֵ�װ��������ָ��
	static gc_itemMgr*              m_ptrItemMgr;

	//! ȫ�ֵ�����״ָ̬��
	static CStateIconMgr*           m_ptrStateMgr;

	//! ��ʱִ������ķ���ֵ
	static int                      m_iCmdImmResCode;


	//! ������Χ��ײ��Ϣ�õ���decal
	int                             m_vecColDecalId[__COLL_DISGRID__*__COLL_DISGRID__];

	//! ������ײ��Ϣ��ʾ��decal�б�
	void                            construct_decalList( os_pickTileIdx& _pickIdx );
# if __COLL_MOD__
	os_pickTileIdx                  m_sLastPickIdx;
# endif 

private:
	/** \brief
	 *  ���볡����Դ�Ľӿ�.
	 *
	 *  \param _lsData  ����һ���������뵽��ǰ���������ݣ���ͬ�ĳ�������Ϊ��ͬ��
	 *                  ���ݡ�
	 */
	virtual bool                    load_scene( const char* _lsData = NULL );

	//! �������£���ʾ�����ڵ�Decal
	virtual void                    on_lbuttonDownInScene( lm_command& _cmd );

	//! ����������ƶ���Ϣ�Ĵ���
	virtual void                    on_mousemoveInScene( lm_command& _cmd );


	//! �Ҽ�����£��޸ĳ����ڵ���ײ��Ϣ
	virtual void                    on_rbuttonDownInScene( lm_command& _cmd );

	virtual void					on_ControlDownInScene( lm_command& _cmd );


	//! �������ڵ������仯
	virtual void                    process_weatherChange( lm_command& _cmd );


	//! ��������ħ�����ܲ��������
	virtual void                    process_skillPlay( lm_command& _cmd );
	//! �ڳ����������������ڵ�λ���ϲ���һ����Ч
	virtual void                    process_effplay( lm_command& _cmd );
	//! ���������ֵĲ��š�
	virtual void                    process_musicPlay( lm_command& _cmd );
	//! ���������װ���л�
	virtual void                    process_equipChange( lm_command& _cmd );
	//! �������ڹ����״̬ħ������
	virtual void                    process_stateMagic( lm_command& _cmd );

protected:
	/** \brief
	*  �ͷų�����Դ�Ľӿ�.
	* 
	*/
	virtual void                    release_scene( void );

	//! �����Զ�����������ͼ�������б�
	void                            process_travelWholeMap( const char* _mapName );

	//! ���������ƶ�������
	virtual void                    process_spaceMove( lm_command& _cmd );

public:
	gc_gameScene(void);
	virtual ~gc_gameScene(void);

	/** \brief
	 *  ȫ�ֵĳ�ʼ����ǰ���泡��ָ��ͽ��������ָ��ġ�
	 *
	 *  Ҫ���������ʼ���ĵط������������
	 * \param gcs_sceneOnceInit& _init  ��ʼ���ṹ
	 * \return bool
	 */
	static bool                     init_gameScene( gcs_sceneOnceInit& _init );


	/** \brief
	*  ��Ⱦ�ӿ�.
	* 
	*/
	virtual bool                    render_scene( void );

	//! �Գ�������FrameMove
	virtual void                    frame_moveScene( void ){}

	//! ��ǰ�ĳ����Ƿ��ڿ���Ⱦ״̬
	virtual bool                    is_renderEnable( void ){ return false; };

	//! ɾ�������ڿɼ�����ײDecal�б�
	virtual void                    reset_colDecal( void );

	//! ɾ��������ѡ�еĹ���
	virtual void                    delete_selMonster( void ){};


	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );


	//! ����ײ㳡����ָ��,ȫ��ֻ��һ��ʵ��.
	static I_fdScene*				m_ptrScene;

	//! ȫ�ֵ��������Ź�����
	static lm_soundMgr*             m_ptrSoundMgr;

	//! ȫ�ֵ����������
	static gc_creatureMgr*          m_ptrCreatureMgr;

	//! ���ú͵õ���ʱ����ķ��ؽ��
	static void           set_immCmdCode( int _code );
	static int            get_immCmdCode( void );

	//! ��ά�������������صĺ���
# if __DEPLOY_MONSTERMOD__
	//! �������
	virtual void          deploy_monsterAreaInScene( int _idx ){};
	//! ���ļ�������ÿһ�������λ����Ϣ
	virtual void          change_actAreaMonsterPos( void ){};
	//! �õ�������������
	virtual int           get_curActiveArea( void  ) { return 0;};
	//! �ڳ����ڹ��ﲿ���������һ������
	virtual void          insert_monsterInScene( int _midx ) {};

	//! ���غ����õ�ǰ���ڱ༭�Ĺ����id
	virtual int           get_curEditMonsterId( void ) { return -1;}
	virtual void          set_curEditMonsterId( int _id ) { };

	//! �ڳ����ڲ鿴������ƶ�����
	virtual void          see_monsterMoveInScene( int _monIdx ){};

# endif 

};
