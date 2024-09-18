///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_testScene.h
 *
 *  Desc:     ħ��Demo�Ĳ��Գ��������ڲ��Ե��볡�����������������
 * 
 *  His:      River created @ 2006-4-13
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

# include "stdafx.h"
# include "gc_gameScene.h"

//! ÿ���೤ʱ���Զ��洢һ��
# define __AUTOSAVE_TIME__   60.0f

//! ��Ϣ��ʾ�೤ʱ��
# define __INFODIS_TIME__    4.0f

//! �Զ��洢����༭��Ϣ����.
class gc_autoSave
{
private:
	//! �ϴ��Զ��洢��֮��,�Ѿ���ȥ��ʱ��,����Ϊ��λ.
	float    m_fAutoTime;

	//! �Ƿ�ո��Զ��洢��
	BOOL     m_bHaveSaved;

public:
	gc_autoSave();

	//! ÿһ֡����Ҫ���Զ��洢�����frameMove
	BOOL     frame_move( float _etime );

	//! �õ��Զ��洢Ҫ��ʾ����Ϣ
	const char* get_displayInfo( void );

};


//! �л���ս�������õ�ʱ��
# define FIGHT_SCENE_FADETIME 1.5f


/** \brief
 *  ���ڲ��Գ����������
 *  
 */
class gc_testScene : public gc_gameScene
{
private:
	int         m_iInsertX,m_iInsertZ;

	bool        m_bLoadScene;

	//! snapShot������
	I_camera*   m_sSnapshotCamera;

	//! ����༭��ص�����
# if __DEPLOY_MONSTERMOD__

	int         m_iPtNum;
	osVec3D     m_vec3AreaPt[MAX_POINTS+1];

	s_string    m_strMapName;

	//! ��ǰ�������������
	int         m_iCurDeployAreaIdx;

	//! ��ǰ���ڱ༭�Ĺ����ID
	int         m_iCurEditMonsterId;

	//! ������豸���handle
	int         m_iCurEditMonsterHandle;

	//! �Զ��洢.
	gc_autoSave m_sAutoSave;
# endif 


private:
	/** \brief
	 *  ���볡����Դ�Ľӿ�.
	 *
	 *  \param _lsData  ����һ���������뵽��ǰ���������ݣ���ͬ�ĳ�������Ϊ��ͬ��
	 *                  ���ݡ�
	 */
	virtual bool                    load_scene( const char* _lsData = NULL );

	/** \brief
	*  �ͷų�����Դ�Ľӿ�.
	* 
	*/
	virtual void                    release_scene( void );


	//! �����һ��Ĭ�ϵ����λ��
	void                            cal_defaultCam( void );

	//! ����ˢ����ص���Ϣ
	void                            load_deployMonsterInfo( const char* _map );

protected:
	//! ���������ƶ�������
	virtual void                    process_spaceMove( lm_command& _cmd );


	//@{ ս���������л�
	BOOL      m_bFadeInFightScene;
	float     m_fFadeTime;
	//! fadeInս�������Ĵ���
	void                            framemove_fightSceneFade( float _time );
	//@} 



# if __DEPLOY_MONSTERMOD__
	
	//! ɾ����ǰ�༭����Ĺ�����Ϣ
	void                            reset_deviceMonster( void );
	//! �������������Ϣ
	void                            draw_monsterQuad( osVec3D& _cenPos,
		                              float _dis,DWORD _color,int _width = 3 );

	//! ��ѡ�й�Ϸ��������Ϣ
	void                            draw_activeMonsterQuad( void );

	//! �����������й����������Ϣ
	void                            draw_allMonsterQuad( void );

# endif 

public:
	gc_testScene(void);
	virtual ~gc_testScene(void);

	//! �л����غ��ƵĲ��Գ���
	void                            change_fightScene( void );


	/** \brief
	*  ��Ⱦ�ӿ�.
	* 
	*/
	virtual bool                    render_scene( void );

	//! �Գ�������FrameMove
	virtual void                    frame_moveScene( void );

	//! ��ǰ�ĳ����Ƿ��ڿ���Ⱦ״̬
	virtual bool                    is_renderEnable( void ){ return m_bLoadScene; };

	//! �����snopShot�Ĵ���
	void           process_cameraSnapshot( void );

	//! ɾ��������ѡȡ�Ĺ���
	virtual void                    delete_selMonster( void );

	//! ��ά�������������صĺ���
# if __DEPLOY_MONSTERMOD__
	//! �������
	virtual void          deploy_monsterAreaInScene( int _idx );


	//! ���ļ�������ÿһ�������λ����Ϣ
	virtual void          change_actAreaMonsterPos( void );

	//! �õ�������������
	virtual int           get_curActiveArea( void  ){ return m_iCurDeployAreaIdx; } 

	//! ѡ�й�����ص���Ϣ,λ��,��Ұ�ȵ�
	void                  draw_monsterArea( void );

	//! �ڳ����ڹ��ﲿ���������һ������
	virtual void          insert_monsterInScene( int _midx );

	//! ���ص�ǰ���ڱ༭�Ĺ����id
	virtual int           get_curEditMonsterId( void ){ return m_iCurEditMonsterId;}
	virtual void          set_curEditMonsterId( int _id ) { m_iCurEditMonsterId = _id; }

	//! �ڳ����ڲ鿴������ƶ�����
	virtual void          see_monsterMoveInScene( int _monIdx );

# endif 

};
