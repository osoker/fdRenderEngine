///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_camera.h
 *
 *  Desc:     ħ��demo�ͻ����õ�������࣬�����������entityһ�����������ڵ���Ϣ
 * 
 *  His:      River created @ 2006-4-14
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "stdafx.h"
# include "gc_command.h"

class lm_configInfoReader;
/** \brief
 *  ȫ�ֵ�����ࡣ����������ֻʹ��һ�������
 *
 *  ��������װ�����ڵ�I_camera����ӿڡ��ڳ�������֮ǰ���ȴ����������������뵽�����ڡ�
 *  ������Խ��ܺʹ�����Ϣ����Ϣ�����������룬Ҳ������������ȵȡ�
 *
 *  ��ʼ�������볡���Ĵ����ɺ�������
 *  
 *  ������ܺʹ������µ���Ϣ��
 *  1: 
 *  2: ������Ϣ�� 
 *  3: Pitch,Rotate,Move����Ϣ��
 *  4: ����...
 */
class gc_camera : public lm_gameEntity
{
	//! Ϊ��������������������
	friend class lm_configInfoReader;
private:

	//! �����ʼ���õ���һЩ��Ϣ
	os_cameraInit        m_sCamInitData;

	//! ������Զ���ٶ�
	float                m_fToFocusSpeed;
	//! ���λ���뽹������ľ���
	float                m_fMinFocusDis;  
	//! ���λ���뽹�����Զ����
	float                m_fMaxFocusDis;

public:
	//! ���������ӿ�
	static I_camera*     m_ptrCamera;

	//! ����ƫ������ĸ߶�
	static float         m_fFocusOffset;

	//! �����ת���ٶȵ���
	static float         m_fRotSpeed;

	//! ����ĽǶ�����
	static float         m_fCamPitchLimit;
	
	//! ����Ϣ�Ĵ���
private:
	//! ���������ص���Ϣ

	//! pitch,rotate��move
	void          process_pitchRot( const lm_command* _cmd );

	//! ������Զ
	void          move_close( const lm_command* _cmd );
	void          move_closef( const lm_command* _cmd );

	//! ����������ƶ��������λ��
	void          move_camera( const lm_command* _cmd );

	//! ����������������bbox
	static void   draw_bbox( I_fdScene* _scene,osVec3D& _pos,osVec3D& _lookvec,osVec3D& _rvec );

	//! �����ǰ�ƶ�
	void          move_forword( const lm_command* _cmd );
	//! ��������ƶ�
	void          move_right( const lm_command* _cmd );
	//! ����Ľ��������ƶ�
	void          move_up( const lm_command* _cmd );
	//! �洢���
	void          save_camera( const lm_command* _cmd );
	//! �������.
	void          load_camera( const lm_command* _cmd );



public:
	gc_camera(void);
	~gc_camera(void);

	//! ��ʼ�������볡�����������Ϣ
	void          init_camera( const os_cameraInit* _camInit );  

	//! ���յ�ǰ�������Ϣ,����һ������ǰ�����һ�������ָ�롣
	I_camera*     snapshot_curCamera( void );

	//! ���������λ�ú�vf
	static void   draw_camera( I_camera* _cam,I_fdScene* _scene );

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );

};



//! �ؼ�����.
struct gcs_keyPoint
{
	osVec3D      m_vec3Pt;
	float        m_fTime;
};

typedef std::vector< gcs_keyPoint >  VEC_keyPoint;

//! �������е�KeyPt��Ŀ.
# define   MAX_KEYPOINT      256

/** \brief
 *  ��������ʹ����Ķ������ӵ����.
 *  
 *  �㷨����:
 *  ����һ�����Ĺؼ�����,Ȼ������Щ�ؼ��������в�ֵ,�õ�������͵Ĺؼ�����.
 *  ��Ϊ�����ʱ�˶���,��������Ĺؼ�����ֻ��������Ľ�������.
 */
class gc_smoothMotion
{
private:
	//! time Axis and Key point.
	VEC_keyPoint  m_vecKeyPt;

	//! �ؼ�������Ƿ�����
	BOOL          m_bFirstFull;
	//! ����smoothPt�Ŀ�ʼ����.
	int           m_iStartPt;
	//! �ܹؼ������ڵ�λ��.
	int           m_iKeyPtEnd;

	//! ��Ͷ����Ƴ�ʱ���ֵ.
	float         m_fDeferTime;
	//! �������ܹ��������Ŀ
	int           m_iMaxPtNum;

	//! ��һ֡�õ���ƽ�������λ��
	osVec3D       m_vec3LastFramePt;

	//! ����smoothMotion�ṹ
	void          reset_smoothMotion( void );

	//! �õ���һ��Pt������.
	int           get_nextPtIdx( int _idx );
	//! �õ���һ��Pt������.
	int           get_previousPtIdx( int _idx );

public:
	gc_smoothMotion();

	/** \brief
	 *  �������ӵ�ʱ��,�����.
	 *
	 *  \param _time �Ӻ��ʱ��.
	 *  \param _maxPtNum ��໺��Ĺؼ�������Ŀ.
	 */
	void   set_deferTime( float _time,int _maxPtNum );

	//! ����ؼ���͹ؼ������ڵ�ʱ��
	void   push_keyPt( osVec3D& _focus,float _curTime )
	{
		m_vecKeyPt[m_iKeyPtEnd].m_vec3Pt = _focus;
		m_vecKeyPt[m_iKeyPtEnd].m_fTime = _curTime;
		m_iKeyPtEnd ++;
	}

	//! �õ���ǰ֡����ʹ�õ�ƽ��������.
	BOOL   get_smoothPt( osVec3D& _resPt,float _curTime );


};


