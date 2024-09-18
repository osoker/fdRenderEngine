///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_gameScene.h
 *
 *  Desc:     游戏场景的基类，游戏中用到的每一个场景，都需要从这个基类中派生，并处理不同
 *            的场景相关的消息。
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


//! 定义最长的ip地址字符串。
# define  MAX_IPSTRSIZE		64



class gc_creatureMgr;
class lm_soundMgr;
class lm_meManager;
class gc_itemMgr;
class CStateIconMgr;

/** \brief
 *  第一次初始化游戏场景时需要填充的结构。
 */
struct gcs_sceneOnceInit
{
	HWND       m_hWnd;
	HINSTANCE   m_hIns;

	int        m_iScrWidth;
	int        m_iSceHeight;

	//@{
	//! 网络相关的初始化。
	char       m_szIp[MAX_IPSTRSIZE];
	DWORD      m_dwPort;
	//@}
};


/** \brief
 *  魔法Demo游戏内场景的基类，使用这个类来控制地图场景的初始化和显示.
 *  这个场景包括三维场景和二维的界面相关场景。
 *  
 */
class gc_gameScene : public lm_gameEntity
{

public:
	//! 全局的相机类
	static gc_camera*               m_ptrCamera;

	//! 地图寻路算法相关
	static CFindPathMap*            m_ptrPathFinder;

	//! TEST CODE:
	static I_skinMeshObj*           m_ptrHeroSkinObj;
	static I_skinMeshObj*           m_ptrHeroSkinObj2;

protected:
	//@{
	//!  窗口的常规信息。
	static HWND						m_hWnd;
	static HINSTANCE				m_hIns;
	static int						m_iScrWidth;
	static int						m_iScrHeight;
	//@} 


	//! 处理杂项功能的接口。
	static I_resource*				m_ptrResource;

	//! 当前的socket管理器.
	static I_socketMgr*				m_ptrSocket;

	//! 全局的技能管理类
	static lm_meManager*            m_ptrMeMgr;

	//! 全局的装备管理器指针
	static gc_itemMgr*              m_ptrItemMgr;

	//! 全局的人物状态指针
	static CStateIconMgr*           m_ptrStateMgr;

	//! 即时执行命令的返回值
	static int                      m_iCmdImmResCode;


	//! 定义周围碰撞信息用到的decal
	int                             m_vecColDecalId[__COLL_DISGRID__*__COLL_DISGRID__];

	//! 构建碰撞信息显示的decal列表
	void                            construct_decalList( os_pickTileIdx& _pickIdx );
# if __COLL_MOD__
	os_pickTileIdx                  m_sLastPickIdx;
# endif 

private:
	/** \brief
	 *  调入场景资源的接口.
	 *
	 *  \param _lsData  从上一个场景传入到当前场景的数据，不同的场景解释为不同的
	 *                  数据。
	 */
	virtual bool                    load_scene( const char* _lsData = NULL );

	//! 左键点击下，显示场景内的Decal
	virtual void                    on_lbuttonDownInScene( lm_command& _cmd );

	//! 场景内鼠标移动消息的处理
	virtual void                    on_mousemoveInScene( lm_command& _cmd );


	//! 右键点击下，修改场景内的碰撞信息
	virtual void                    on_rbuttonDownInScene( lm_command& _cmd );

	virtual void					on_ControlDownInScene( lm_command& _cmd );


	//! 处理场景内的天气变化
	virtual void                    process_weatherChange( lm_command& _cmd );


	//! 处理场景内魔法技能插入的命令
	virtual void                    process_skillPlay( lm_command& _cmd );
	//! 在场景内主角人物所在的位置上播放一个特效
	virtual void                    process_effplay( lm_command& _cmd );
	//! 处理背景音乐的播放　
	virtual void                    process_musicPlay( lm_command& _cmd );
	//! 处理人物的装备切换
	virtual void                    process_equipChange( lm_command& _cmd );
	//! 处理场景内怪物的状态魔法播放
	virtual void                    process_stateMagic( lm_command& _cmd );

protected:
	/** \brief
	*  释放场景资源的接口.
	* 
	*/
	virtual void                    release_scene( void );

	//! 处理自动遍历整个地图的命令列表
	void                            process_travelWholeMap( const char* _mapName );

	//! 处理自由移动的命令
	virtual void                    process_spaceMove( lm_command& _cmd );

public:
	gc_gameScene(void);
	virtual ~gc_gameScene(void);

	/** \brief
	 *  全局的初始化当前引擎场景指针和界面管理器指针的。
	 *
	 *  要在主程序初始化的地方调用这个程序。
	 * \param gcs_sceneOnceInit& _init  初始化结构
	 * \return bool
	 */
	static bool                     init_gameScene( gcs_sceneOnceInit& _init );


	/** \brief
	*  渲染接口.
	* 
	*/
	virtual bool                    render_scene( void );

	//! 对场景进行FrameMove
	virtual void                    frame_moveScene( void ){}

	//! 当前的场景是否处于可渲染状态
	virtual bool                    is_renderEnable( void ){ return false; };

	//! 删除场景内可见的碰撞Decal列表
	virtual void                    reset_colDecal( void );

	//! 删除场景内选中的怪物
	virtual void                    delete_selMonster( void ){};


	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );


	//! 引擎底层场景的指针,全局只有一个实例.
	static I_fdScene*				m_ptrScene;

	//! 全局的声音播放管理类
	static lm_soundMgr*             m_ptrSoundMgr;

	//! 全局的人物管理类
	static gc_creatureMgr*          m_ptrCreatureMgr;

	//! 设置和得到即时命令的返回结果
	static void           set_immCmdCode( int _code );
	static int            get_immCmdCode( void );

	//! 三维场景部署怪物相关的函数
# if __DEPLOY_MONSTERMOD__
	//! 部署怪物
	virtual void          deploy_monsterAreaInScene( int _idx ){};
	//! 更改激活区域每一个怪物的位置信息
	virtual void          change_actAreaMonsterPos( void ){};
	//! 得到激活区域索引
	virtual int           get_curActiveArea( void  ) { return 0;};
	//! 在场景内怪物部署区域加入一个怪物
	virtual void          insert_monsterInScene( int _midx ) {};

	//! 返回和设置当前正在编辑的怪物的id
	virtual int           get_curEditMonsterId( void ) { return -1;}
	virtual void          set_curEditMonsterId( int _id ) { };

	//! 在场景内查看怪物的移动数据
	virtual void          see_monsterMoveInScene( int _monIdx ){};

# endif 

};
