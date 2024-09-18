///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_testScene.h
 *
 *  Desc:     魔法Demo的测试场景，用于测试调入场景，插入相机的命令
 * 
 *  His:      River created @ 2006-4-13
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

# include "stdafx.h"
# include "gc_gameScene.h"

//! 每隔多长时间自动存储一次
# define __AUTOSAVE_TIME__   60.0f

//! 信息显示多长时间
# define __INFODIS_TIME__    4.0f

//! 自动存储怪物编辑信息的类.
class gc_autoSave
{
private:
	//! 上次自动存储过之后,已经过去的时间,以秒为单位.
	float    m_fAutoTime;

	//! 是否刚刚自动存储过
	BOOL     m_bHaveSaved;

public:
	gc_autoSave();

	//! 每一帧都需要对自动存储类进行frameMove
	BOOL     frame_move( float _etime );

	//! 得到自动存储要显示的信息
	const char* get_displayInfo( void );

};


//! 切换到战斗场景用的时间
# define FIGHT_SCENE_FADETIME 1.5f


/** \brief
 *  用于测试场景调入的类
 *  
 */
class gc_testScene : public gc_gameScene
{
private:
	int         m_iInsertX,m_iInsertZ;

	bool        m_bLoadScene;

	//! snapShot后的相机
	I_camera*   m_sSnapshotCamera;

	//! 怪物编辑相关的数据
# if __DEPLOY_MONSTERMOD__

	int         m_iPtNum;
	osVec3D     m_vec3AreaPt[MAX_POINTS+1];

	s_string    m_strMapName;

	//! 当前部署区域的索引
	int         m_iCurDeployAreaIdx;

	//! 当前正在编辑的怪物的ID
	int         m_iCurEditMonsterId;

	//! 怪物的设备相关handle
	int         m_iCurEditMonsterHandle;

	//! 自动存储.
	gc_autoSave m_sAutoSave;
# endif 


private:
	/** \brief
	 *  调入场景资源的接口.
	 *
	 *  \param _lsData  从上一个场景传入到当前场景的数据，不同的场景解释为不同的
	 *                  数据。
	 */
	virtual bool                    load_scene( const char* _lsData = NULL );

	/** \brief
	*  释放场景资源的接口.
	* 
	*/
	virtual void                    release_scene( void );


	//! 计算出一个默认的相机位置
	void                            cal_defaultCam( void );

	//! 调入刷怪相关的信息
	void                            load_deployMonsterInfo( const char* _map );

protected:
	//! 处理自由移动的命令
	virtual void                    process_spaceMove( lm_command& _cmd );


	//@{ 战斗场景的切换
	BOOL      m_bFadeInFightScene;
	float     m_fFadeTime;
	//! fadeIn战斗场景的处理
	void                            framemove_fightSceneFade( float _time );
	//@} 



# if __DEPLOY_MONSTERMOD__
	
	//! 删除当前编辑区域的怪物信息
	void                            reset_deviceMonster( void );
	//! 画怪物的区域信息
	void                            draw_monsterQuad( osVec3D& _cenPos,
		                              float _dis,DWORD _color,int _width = 3 );

	//! 画选中怪戏的区域信息
	void                            draw_activeMonsterQuad( void );

	//! 画场景内所有怪物的区域信息
	void                            draw_allMonsterQuad( void );

# endif 

public:
	gc_testScene(void);
	virtual ~gc_testScene(void);

	//! 切换到回合制的测试场景
	void                            change_fightScene( void );


	/** \brief
	*  渲染接口.
	* 
	*/
	virtual bool                    render_scene( void );

	//! 对场景进行FrameMove
	virtual void                    frame_moveScene( void );

	//! 当前的场景是否处于可渲染状态
	virtual bool                    is_renderEnable( void ){ return m_bLoadScene; };

	//! 对相机snopShot的处理
	void           process_cameraSnapshot( void );

	//! 删除场景内选取的怪物
	virtual void                    delete_selMonster( void );

	//! 三维场景部署怪物相关的函数
# if __DEPLOY_MONSTERMOD__
	//! 部署怪物
	virtual void          deploy_monsterAreaInScene( int _idx );


	//! 更改激活区域每一个怪物的位置信息
	virtual void          change_actAreaMonsterPos( void );

	//! 得到激活区域索引
	virtual int           get_curActiveArea( void  ){ return m_iCurDeployAreaIdx; } 

	//! 选中怪物相关的信息,位置,视野等等
	void                  draw_monsterArea( void );

	//! 在场景内怪物部署区域加入一个怪物
	virtual void          insert_monsterInScene( int _midx );

	//! 返回当前正在编辑的怪物的id
	virtual int           get_curEditMonsterId( void ){ return m_iCurEditMonsterId;}
	virtual void          set_curEditMonsterId( int _id ) { m_iCurEditMonsterId = _id; }

	//! 在场景内查看怪物的移动数据
	virtual void          see_monsterMoveInScene( int _monIdx );

# endif 

};
