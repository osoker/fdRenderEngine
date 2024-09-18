//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osCamera.h
 *
 *   Desc:     ʵ��Camera�Ľӿ�,ͬʱ����osok�����ڲ���Ҫ��Camera�ӿ�.
 *          
 *   His:      River Created @ 4/18 2003.
 *
 *   "��Ҫ���׸ı仨�˺ܳ�ʱ�������ľ���,�����͵����Ǳ������Լ�������".
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"


/** \brief
 *  ʹ�����clipPlaneԶһЩ,���ӳ�����ʵ��.������Ʒ�Ŀɼ���ʱ,
 *  �ճ�ʹ�þɵ�clipPlane.
 */
# define CLIPDIS_ADD     60.0f

//! �����Ӿص�һ��������⣬��������Ʒ,���ִ����������Ҫ������Ʒ��
# define MASK_OUTHALF_NOTVIEW  0x01
//! �����Ӿ�һ������ڵĿ�����Ʒ��
# define MASK_HALF_VIEW       0x02
//!   ���Ӿ�һ�������Ŀ�����Ʒ��
# define MASK_OUTHALF_VIEW    0x04

//! ����autoResetͨ�����µ�ʱ��󣬲���������ʼautoReset
# define  AUTORESET_STARTTIME 0.25f



class osc_TGManager;
class osc_middlePipe;
class osc_camera;

/** \brief
 *  Osok�����View frustum��.
 *
 *  �ṹ������򵥵ĺ���,�����ж�bsphere or bbox or point�Ƿ���
 *  ���view frustum�ڲ�.
 *
 */
struct  os_ViewFrustum
{
	//
	// ǰ�ĸ�����Ϊ�������棬���ĸ�����ΪԶ�����涥�㡣
	osVec3D     m_FrustumVer[8];

	/** \brief
	 *  ��Щ����Frustum����,����Frustum�ڲ�.
	 */
	osPlane     m_Frustum[5];


	//! ��ǰvf��aabbox,���ھ�ȷ�ļ��aabbox�Ƿ��vf�ཻ
	os_aabbox   m_sAabbox;

	//! ���һ�������Ƿ�����ң��ϣ��£�Զ�������ཻ���������εľ�ȷ�ཻ
	BOOL        ray_intersectLeftClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectRightClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectTopClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectBottomClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );
	BOOL        ray_intersectFarClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );


	//! һ��������㣬һ�����ȣ�һ�����򣬵����µĶ���
	void        get_pt( osVec3D& _s,osVec3D& _dir,float _len,osVec3D& _res );

	//! ����һ����ʼ��ͽ����㣬�ҵ���clip��Ľ���
	BOOL        get_clipPt( osVec3D& _s,osVec3D& _e,osVec3D& _res );

public:
	os_ViewFrustum();


	/** \brief
	 *  �鿴һ�����Ƿ���һ��view frustum��
	 */
	bool        objInVF( osVec3D& _vec );
	
	/** \brief
	 *  �鿴һ��bounding sphere�Ƿ���һ��view frustum��
	 */
	bool        objInVF( os_bsphere& _bs );

	/** \brief
	 *  �鿴һ��bounding sphere�Ƿ���һ��view frustum��
	 */
	bool        objInVF( osVec3D& _vec,float _rad );

	/** \brief
	 *  �鿴һ��bounding box�Ƿ���һ��view frustum��
	 */
	bool        objInVF( os_bbox& _bb );



	//! �鿴һ��aabbox�Ƿ���һ��vf��.
	bool        objInVF( const os_aabbox& _box );

	//! ��ȷ���vf�Ƿ���aabbox�ཻ
	bool        objInVFAccurate( const os_aabbox& _box );

	/** \brief
	 *  ���ڼ��һ��aabbox��vf�Ĺ�ϵ
	 *
	 *  \return bool aabboxȫ����vf�ڣ�����true.�ཻ����false.
	 *                     ȫ����vf�������objInVF�����ж�
	 */
	bool        objCompleteInVF( const os_aabbox& _box );


	/** \brief
	 *  over load the operator =
	 */
	os_ViewFrustum& operator=( const os_ViewFrustum& _v )
	{
		memcpy( m_FrustumVer,_v.m_FrustumVer,sizeof( osVec3D )*8 );
		memcpy( m_Frustum,_v.m_Frustum,sizeof( osPlane )*5 );
		m_sAabbox.m_vecMax = _v.m_sAabbox.m_vecMax;
		m_sAabbox.m_vecMin = _v.m_sAabbox.m_vecMin;

		return *this;
	}

	//! �˺�����һ���߶�clip��������,�����޸���������
	bool        clip_lineSeg( osVec3D& _s,osVec3D& _e );

	//  TEST CODE:
	//! ����Vf��Զ�����棬Ҫ��ʵ�ʵ�Զ�������һ��
	void        draw_farPlane( osc_middlePipe* _pipe );

	//! ���һ�������Ƿ������Ľ��������ཻ���Ǿ�ȷ�������������������Σ����ǽ��������ƽ�档
	BOOL        ray_intersectNearClipPlane( osVec3D& _start,osVec3D _dir,float* _dis = NULL );


	//! ���������Ϣ
	void        output_debugInfo( void );

};


//! ���ݵ�ǰ��������ݣ�������ʱ��ָ���ԭ�������
struct os_cameraDataBackup
{
	//! ���������ռ��λ����Ϣ
	osVec3D       m_vec3CamPos;
	osVec3D       m_vec3CamFocus;

	float         m_fCurPitch;
	float         m_fCurYaw;

	//@{
	/**
	 *  ��������ProjMatrix������.
	 */
	float        m_fNearp;
	float        m_fFarp;
	float        m_fFov;
	float        m_fAspect;
	//@}

	//@{
	/**
	 *  ��������ViewPort������.
	 */
	int          m_iTlx;
	int          m_iTly;
	int          m_iWid;
	int          m_iHei;
	float        m_fMin_z;
	float        m_fMax_z;
	//@}	

};


/** \brief
 *  �Զ���λ״̬������
 *
 *  ������Զ��˶�ֻ����focus��rot vertical,����ֻ��Ҫ��¼��Ӧ������.
 */
class osc_autoResetMgr
{
private:
	bool      m_bInARState;

	//! ������ľ���
	float     m_fPosFocusDistance;

	//! ��λ��Ŀ��pitch.
	float     m_fDestPitch;

	//! �Զ���λ���ٶ�
	float     m_fResetSpeedPerSec;

	//! �涨��������ڶ೤ʱ����Զ���λ���ͨ���󣬲����ڻ����ϲ����������Զ���λ���ܡ�
	float     m_fAutoResetPassTime;

	//! ��һ֡�ƶ��ľ���
	float     m_fLastFrameMove;
	
	//����������rot�ı䣬��ʼreset
	bool      m_bPauseAutoset;

public:
	osc_autoResetMgr();

	//! ��ʼ�Զ���λ״̬
	void      start_autoResetState( float _dis,float _pitch );

	/** \brief
	 *  �����Զ���λ��λ��.
	 *  
	 *  \param _focus  ����Ľ���λ��.���е��Զ���λ�������µĽ���Ϊ����.
	 *  \param _curPos ����ĵ�ǰ�����λ��.
	 *
	 *  �㷨����:
	 *  1: ���ݴ����λ��,�������ǰ�����pitch
	 *  3: ��������Ļ�λ�ٶ�,������õ��Զ������λӦ�õ����λ��.
	 *  4: �ϲ��ж��µ�λ���Ƿ�Ϸ�,����Ϸ������µ�λ��,����ȴ���һ֡���ٴε���.
	 */
	float     get_autoResetPos( osVec3D& _focus,osVec3D& _curPos );

	//! ����Զ���λ�Ĵ���
	void      process_camAutoResetPos( osc_camera* _cam,float& _move );

	//! ��ǰ�Ƿ����Զ���λ״̬.
	bool      is_autoResetState( void ) { return this->m_bInARState; }

	//! �����Զ���λ��״̬.
	void      end_autoResetState( void ) { m_fAutoResetPassTime = 0.0f;m_bInARState = false; }

	//! ���õ�ǰ֡ͨ���Զ���λ���Ե����.
	void      set_autoResetPass( bool _pass );

	//  
	//! ��ͣautoReset.
	void      pause_autoReset( bool _ar ) { m_bPauseAutoset = _ar; } 

	//! �õ���һ֡�ƶ���λ�á�
	float     get_lastMoveLength( void ) { return this->m_fLastFrameMove; } 

	//! ��ǰautoReset�Ƿ��Ѿ�����autoResetTime
	bool      pass_autoResetTime( void ) { return m_fAutoResetPassTime > AUTORESET_STARTTIME; }


	//@{
	//  River @ 2010-1-9:���ڽ���л�ս������ʱ������ص���λ�ò���ȷ��bug.
	//! �ѵ�ǰ��AutoResetState���ݱ��浽��һ��ʵ���ڡ�
	//! 
	const osc_autoResetMgr& operator=( osc_autoResetMgr& _mgr );
	//@} 


};

//! ������Ӿص����λ�ÿ�ʼ���ر�ʼ�ͱ����ںϡ�
//! River @ 2006-4-5:�˱����������ڵر���������ںϣ���Զ����Ʒ�Ŀɼ������ݸ�������
# define  TA_SCALE   0.90

/** \brief
 *  Camera�ӿڵ�ʵ����.
 *
 *  �����ʵ����Camera�ӿ�,���Ҽ���Osok�����ڲ���Ҫ��һЩ�ӿںͲ���.
 *
 *  ATTENTION TO OPP: �������¼��ǰ�����wvp���󣬼�¼�������ķ�����
 *                 ������ÿ֡��Ҫ�������ʰȡ��Ϣ��ʱ�򣬿��Լ�С��������
 *
 *  ���λ�ú���ת����Ĳ�ֵ������
 *  ��Ҫ������м��뺯����֧�ֲ�ֵ������
 *  
 */
class osc_camera : public I_camera
{
public:

	/**********************************************************************/
	//
	//  ��ʼ��Camera�ĺ���.
	//
	/**********************************************************************/
	/** \brief 
	 *  ��һ���ṹ�г�ʼ��һ��Camera.
	 *
	 *  \param _cdata  ������ṹ�г�ʼ�����ǵ�Camera.
	 *
	 */
	void         init_camera( const os_cameraInit* _cdata );

	//! �õ�����ɿ�����Զ������ľ��룬�������ɼ�����Զ����
	virtual float        get_farClipPlaneDis( void );
	virtual float		 get_nearClipPlaneDis( void );


	/**********************************************************************/
	//
	//  ����Camera��Ҫ�Ľӿ�.
	//
	/**********************************************************************/
	/** \brief
	 *  ����һ��Camera��Projection Matrix
	 */
	virtual void         set_projmat( float _nearp,
		                    float _farp,float _fovh,float _faspect );

	//! ����Զ������.
	virtual void         set_farClipPlane( float _farp );

	/** \brief
	 *  ����һ��Camera��ViewPort.
	 *
	 *  ��ͬ��Camera������Ҫ��ͬ��ViewPort,���ܳ��ֶ��Camera���
	 *   ViewPort���龰(���л�).
	 *
	 *   _tlx,_tly        Viewport����Ļ���ϽǵĶ���.
	 *
	 *   _wid,_hei        Viewport�Ŀ�͸ߡ�
	 *
	 *   _min_z,_max_z    Viewport��������СZֵ.
	 */
	virtual void         set_viewport( int _tlx,int _tly,
		                    int _wid,int _hei,float _min_z,float _max_z );

	/** \brief
	 *  ��������camear��aspect.
	 *
	 *  \param _aspectAdd ��ǰ�����aspect����������ݡ��ڲ���_aspectAdd����Ϊ
	 *                    �ı���aspect���ݣ�������.
	 */
	virtual void         set_camAspect( float& _aspectAdd );



	//! ���������fovֵ��
	virtual void         set_cameraFov( float _fov );
	//! �õ����е�����ӽǿ��.
	virtual float        get_cameraFov( void );




	/** \brief
	 *  �õ������ǰ��ָ��ķ���,����������ں���ģ��,�����˷����
	 *  ����.
	 */
	virtual void         get_godLookVec( osVec3D& _vec );



	/** \brief
	 *  �õ���ǰ��������ҵķ������.
	 */
	virtual void         get_rightvec( osVec3D& _vec ) ;



	/** \brief
	 *  ���������ǰ��λ��.
	 */
	virtual void         set_curpos( osVec3D* _vec,bool _force = false  );

	//! 2009-9-4:һ��������λ�ú�focus,���ڽ��һ���ܱ�̬����Ļ�ζ���bug.
	virtual void         set_curPosFocus( osVec3D& _pos,osVec3D& _focus,bool _force = false );


	/** \brief
	 *  �õ������ǰ��λ��.
	 */
	virtual void			get_curpos( osVec3D* _vec );
	virtual const osVec3D&	get_curpos( void) ;
	
	/** \brief
	 *  ʹ���Pitchһ���Ƕ�.
	 */
	virtual void         pitch( float _radian );

	/** \brief
	 *  ʹ���Yawһ���Ƕ�.
	 */
	virtual void         yaw( float _radian );

	/** \brief
	 *  ʹ�����Godģʽ��ǰ�ƶ�һ�ξ���(��������ɸ�).
	 *
	 *   ִ�в���:
	 *
	 *   �����GodMode
	 *   1: �����Ҫ����Look vector,�ȹ����µ�Look vector.
	 *
	 *   2: ��Look vector�ķ������ƶ�Ҫ�ƶ��ľ���.
     *  
	 *   �����GenMode.
	 *	 1: �ȹ���Camera Look vector��ˮƽ�����ϵ�Vector.
	 *	 
	 *   2: ��Look vector��ˮƽ�������ƶ�Ҫ�ƶ��ľ���.
	 */	 
	//virtual void         move_forward( float _dis );


	/**********************************************************************/
	//
	//  ʹ������е����ݽ���һЩ�������ļ���.
	//
	/**********************************************************************/
	//@{
	/** \brief
	 *  �õ���Ļ��һ����ά�����������ά�ռ��е�����.
	 */
	virtual osVec3D      get_upprojvec( const osVec2D* _svec,osMatrix& _wmat );      
	virtual osVec3D      get_upprojvec( int _x,int _y,osVec3D& _dir );
	//@} 

	/** \brief
	 *  ����һ������ռ�Ķ���,������ǰ����ռ䶥���Ӧ����Ļ�ռ�Ķ���.
	 *
	 *  \param _wpos ���������ռ�Ķ�������,������Ϊ��������.
	 *  \param _spos ����������ռ䶥���Ӧ����Ļ����.
	 *  
	 */
	virtual bool          get_scrCoodFromVec3( const osVec3D& _wpos,osVec3D& _spos );


	//! �ӳ����е���camera,����ʾ�����λ�ú���ת��������ݣ����ӿ��޹ء�
	virtual bool          load_camera( const char* _cname,
		                     bool _ani = false,float _time = 0.0f );

	/** \brief
	 *  ���������ת��Pitch��������
	 *  
	 *  ��ͨ��;�������ر���ײʱ�����������͵ĵ�����λ��
	 *  ��ת��pitch����������Ľ���������С�
	 */
	virtual bool          animate_camRotPicth( 
		                   float _rotRad,float _pitchRad,float _time );

	virtual void  set_autoResetState( bool _autoReset );

	/** \brief
	 *  ����һ�������Ƿ��������������
	 */
	virtual bool          is_ptInCamVF( osVec3D& _pos );


	/** \brief
	 *  ���Ƹ�����Ϊһ�̶�ֵ
	 *
	 *  \param _rad �̶��ĸ����Ƕ�ֵ(����),�����ֵС�ڵ����㣬��ʹ�õ�ǰ
	 *              �����pitchֵ��Ϊ������pitchֵ,
	 *              ATTENTION!!! ��_radֵΪ�����Y�ᣬ��UP�����ת�Ƕȣ���
	 *              ������ǵ��������̧�ĸ�ֱ���൱�ڶ���ͼ�����ֵԽС��
	 *              ��ֵԽ�󣬱�ʾ���Խƽ��_rad�Ĵ����������ʹ�� 
	 *              ��ת�Ƕ�/180* OS_PI
	 *              �����ʽ�ڿͻ��˻�ò��������档
	 *  
	 */
	virtual void   limit_pitch( float _rad );

	/** \brief
	 *  ���Ƹ�����Ϊһ�̶�����ֵ
	 *
	 *  \param _rad     ��ֱ������ӽ�.
	 *  \param _radMin  ��ƽ������ӽ�.
	 *  \param _minFDis ����뽹����Դﵽ��������룬�ڴ�λ���ϣ����ʹ����ƽ���ӽǡ�
	 *  \param _maxFDis ����뽹����Դﵽ����Զ���룬�ڴ�λ���ϣ����ʹ����ֱ���ӽǡ�
	 */
	virtual void   limit_pitch( float _rad,float _radMin,float _minFDis,float _maxFDis );

	/** \brief
	*  ���Ƹ�����Ϊһ�̶�ֵ��֮ǰ��ĳ��limit_pitch��ֵ
	*/
	virtual void   limit_pitch( void );

	/** \brief
	*  ȡ�����Ƹ�����Ϊһ�̶�ֵ
	*
	*  \param _rad �̶��ĸ����Ƕ�ֵ(����)
	*  
	*/
	virtual void   unlimit_pitch();
	
	/** \brief
	*  ʹ����񶯵Ľӿ�
	*  
	*  \param float _fDamp ���������.
	*  \param float _swing      ��������.
	*/
	virtual void shake_camera( float _fDamp,float _swing );


	//! �õ����viewFrustum��8������
	virtual const osVec3D* get_vfPt( void );

	//! ����autoResetState.
	virtual void  save_autoResetState( void );

	//! �ָ�autoResetState.
	virtual void  restore_autoResetState( void );



# if __EFFECT_EDITOR__
	//! �洢�����е�cameraλ�����ݡ�
	virtual bool          save_camera( const char* _cname );
# endif 



	//! �õ�Tile��ʼ͸��������Ӿء�
	float                get_taDis( void ) ;


	//! �ƶ������һ��λ����,�ܿ��ϰ���,
	//! �������Ĳ���ֵС���㣬�����ʹ�����δ��һ��ʱ�䲻�Զ���λ��
	bool                 move_cameraToAvoidShelter( float _disToFocus );

	//! ��������Ķ���״̬
	void                 back_upCamCaState( void );
	void                 reset_CamCaState( void );


public:
	//=====================================================================================
	//
	//  The following public func is the func which will used in inner 
	//  engine part.
	//  ����������Щ���к���ֻ�������������ڲ�,���ṩ�ϲ�ʹ����Щ�����Ľӿ�.
	//
	//=====================================================================================
	osc_camera();

	/** \brief 
	 *  ʹ������������õ�ViewPort
	 * 
	 *  Set_viewport�ǳ�ʼ���Ժ����Ҫ���õĵĺ�����ʹ������������õ�ViewPort.
	 */
	void               get_viewport( D3DVIEWPORT9& _v );                                  

	//@{
	//!  View port releated func.
	osVec2D            get_screencenter( void );
	int                get_viewporttlx( void );
	int                get_viewporttly( void );
	int                get_viewportwidth( void );
	int                get_viewportheight( void );
	int                get_cameramode( void );
	void               get_nearplane( osPlane& _plane );
	void               get_farplane( osPlane& _plane );
    //@}

	// FIXME: USE THIS FUNCTION IN PORTAL ENGINE?????
	//!  �õ����camera��frustum.���ص�planeָ��ָ��6����.
	os_ViewFrustum*    get_frustum( void );

	
	//!  Get the view matrix..
	void               get_viewmat( osMatrix& _mat,bool _reflect = false );	

	//!  Get the proj Matrix.
	//! �Ƿ�ʹ��offset,ʹ�ó���������������ʵ.
	void               get_projmat( osMatrix& _mat,bool _offset = true );

	//! �õ����������view mat.
	void               get_rhViewMat( osMatrix& _mat );

	//! �õ����������proj Matrix.
	void               get_rhProjMat( osMatrix& _mat );


	//!  �õ�ˮƽ�����ϵ�look vec.
	void               get_horlookvec( osVec3D& _look );                       

	//!  �õ���Ļ�ռ��׼�Ǳ仯������ռ��vec.
	osVec3D            get_upprojvec( void );                                 


	//!  �õ�Զ������ľ��롣
	float              get_farPlaneDis( void );

	//!  ����Զ������ľ���.
	void               set_farPlaneDis( float _fdis );

	//!  �õ���ǰ�����yaw
	float              get_curYaw( void );

	//!  �õ���ǰ�����pitchֵ��
	virtual float       get_curPitch( void );


	//!  �Ƿ��������������һ���뾶��λ�á�
	BYTE               is_viewByCamera( osVec3D& _pos,float _radius );

	//!  �����frameMove���������ڴ�������Ķ�����
	void                frame_moveCamera( float _etime );


	//!  ���ú͵õ�����Ľ���.
	virtual void         set_camFocus( const osVec3D& _focus );
	virtual void         get_camFocus( osVec3D& _focus )const ;
	virtual const osVec3D& get_camFocus( void)const ;

	//!����ǰ���ƶ����
	void                move_close( float _length );

	/** \brief
	 *  ��������λ�õ�����ľ���
	 */
	float               get_disFromCamposToFocus();

	//!  �ڹ�ȥ��һ֡�У����������ת��
	bool                is_rotIncurFrame( void ) { return m_bRotInCurFrame; } 

	//! ���ݵ�ǰ�����λ�úͷ�������е��������
	void                backup_curCam( void );
	//! ��������ı�������
	void                use_backupCam( void );

	//! �õ��������λ�úͽ��������γɵ�bsphere,���������ص�һЩ��⴦��,��������������ཻʱ,����ʾ����.
	bool                get_camNearClipBSphere( os_bsphere& _bs );

	//! TEST CODE:����ȷ����������ر�����ײ����.
	void                 assert_camera( void );

private:
	
	//!  Contain the view matrix of the scene...
	osMatrix          m_viewmat ;  
	
	//!  ��������ڵ�matrix.
	osMatrix          m_rhViewMat;


	//!  Range from -2pi--2pi  we can do more accurate if we use wide range.
	float             m_fCuryaw;                     

	//!  Range from   -pi/2   --    +pi/2
	float             m_fCurpitch;  

	//! River mod @ 2008-10-11:lock���pitch����
	float             m_fLockPitch;

	//! River mod @ 2009-4-9:����pitchֵ��һ��������������λ���뽹��Զ�����仯
	float             m_fLockPitchMin;
	float             m_fMinFDis;
	float             m_fMaxFDis;


	//!  ����Ľ���,�����תʱ,�˵㲻��.��Ҫÿһ֡��������.
	osVec3D           m_vec3CamFocus;

	//@{
	/** 
	 *   ��������������Ĳ�������.
	 */
	osVec3D           m_lookvec;
	osVec3D           m_rightvec;
	osVec3D           m_upvec;
	//@}


	//@{
	/** \brief
	 *  ���µ��������������ʵ�����е�����.
	 */
	osVec3D           m_curpos; 
	osVec3D           m_curLookVec;
	osVec3D           m_curRightVec;
	//@}
	
	//! ��ǰ�����Common look vector,����common mode,��������ƶ�.
	//  ʹ�ú���get_horlookvec���õ����ֵ.
	// osVec3D           m_curComLookVec;

	bool              m_bShouldUpdate;


	//! �Ƿ�����������ײ���
	bool m_bCammerLimitY;
	
	//@{
	//! View port releated var.
	int               m_iTopleftx,m_iToplefty;
	int               m_iWidth,m_iHeight;
	float             m_fMinz,m_fMaxz;
	//! Screen center.
	osVec2D           m_scenter;             
	//@}
	
	
	//@{
	//!  Projected matrix releated var.
	float             m_fFov;
	float             m_fNearPlane,m_fFarPlane;
	float             m_fHalfFar;

	//! ���������֮�����Ʒ�϶����ɼ���
	float             m_fObjFullView;
	float             m_fAspect;
	//@}
    

	//! ��ǰCamera��λ�ú�focusλ��֮��ľ���.
	float                   m_fCamDisFocus;

    
	//!  ��ǰCamera��frustum.
	os_ViewFrustum    m_Frustum;
	

	//@{
	// River added @ 2004-9-3:���λ�úͷ����Ĳ�ֵ������
	// ATTENTION : �Ժ�������ʽ�������ֵ������
	//! ��ǰ�Ƿ������ֵ����״̬��
	bool              m_bInCAState;

	//! Ŀ������Ĳ�����
	osVec3D           m_vec3NewPos;
	osVec3D           m_vec3NewLook;
	osVec3D           m_vec3NewRight;
	float             m_fNewPitch;
	float             m_fNewYaw;

	//! �ɵ����������
	osVec3D           m_vec3OldPos;
	osVec3D           m_vec3OldLook;
	osVec3D           m_vec3OldRight;

	float             m_fOldPitch;
	float             m_fOldYaw;
	//@} 

	//! �Ӳ�ֵ���������ڹ�ȥ��ʱ�䡣
	float             m_fEleTime;
	//! ��ǰ��ֵ�����ܵ�ʱ�䡣
	float             m_fCATime;

	//@{
	//! �������λ��֮��Ĳ�ֵ��
	bool              m_bInRotPitchLerpState;
	float             m_fTotalPitch;
	float             m_fTotalRot;

	float             m_fPitchPerSec;
	float             m_fRotPerSec;
	//@} 

	//syq �µ�����˶�����
	DWORD   m_dwVersion;
	osVec3D m_vec3OldCamPos, m_vec3NewCamPos;
	osVec3D m_vec3OldCamFocus, m_vec3NewCamFocus;

	//����������Զ����
	float m_fMaxDis, m_fMinDis; 


	//! �ر���ӿ�ʼ͸��������Ӿء�
	float             m_fTADis;


	//! ����ڵ�ǰ֡�У��Ƿ�����ת�Ķ���
	bool              m_bRotInCurFrame;


	//! �����Ľӿ�
	osc_TGManager*     m_pScene;

	//! ���ڱ��ݵ�ǰ���λ�ú���Ⱦ��ؽṹ������
	os_cameraDataBackup  m_sCamBackup;
	
	//! ����Զ���λ��λ�ò��Ϸ�,����ʹ����һ֡�����λ��,�����ɱ�������Ķ���������.
	osVec3D              m_vec3LastFrameCamPos;
	//! ������Զ���λ���ܹ�����
	osc_autoResetMgr     m_sAutoResetMgr;

	//! �������Զ���λ����
	osc_autoResetMgr     m_sBackUpARM;


	//! �������״̬�ı���
	bool                 m_bBackCaState;

private:
	//=====================================================================================
    //
	//  osc_camera�ڲ��õ��İ�������.
	//
	//=====================================================================================
	
	//! �����Ƕ�ֵ
	void               modify_pitch_yaw();

	/**
	 *  Update the camera.����Camera�еĸ���Matrix.
	 *
	 *  \param bool _colwithTerr �Ƿ���ر�����ײ����
	 */
	void               update_camera( bool _buildFrus = true,bool _colwithTerr = true,bool _reflect = false );

	/** \brief
	 *   ʹ�����Generalģʽ��ǰ�ƶ�һ�ξ���.
	 *
	 *   ִ�в���:
	 *
	 *   1: �ȹ���Camera Look vector��ˮƽ�����ϵ�Vector.
	 *
	 *   2: ��Look vector��ˮƽ�������ƶ�Ҫ�ƶ��ľ���.
     */
	void               move_forward_godmod( float _dis );


	/** \brief
	 *   ʹ�����Godģʽ��ǰ�ƶ�һ�ξ���(��������ɸ�).
	 *
	 *   ִ�в���:
	 *
	 *   1: �����Ҫ����Look vector,�ȹ����µ�Look vector.
	 *
	 *   2: ��Look vector���ƶ�Ҫ�ƶ��ľ���.
	 * 
	 */
	void               move_forward_genmod( float _dis );


	/** \brief
	 *  ʹ�����һ���������ˮƽ��ת��
	 *��
	 */
	virtual ose_camOperateRS rotate_horizon( const osVec3D& _ver,
		                   float _radians,bool _force = false  );

	/** \brief
	 *  ʹ�����һ���������������ת��
	 */
	virtual ose_camOperateRS rotate_vertical( const osVec3D& _ver,
		                   float _radians,bool _force = false  );



	//! ����������������в�ֵ��
	void                lerp_cameraFile( void );
	void                lerp_cameraRotPitch( float _etime );

	/** \brief
	 *  ������ǰCamera��frustum.
	 */
	void               build_camFrustum( bool _guaranteeFrums = false );

	//! ���Զ���λ������λ�ý��д���,�������,������Ϊ�ɵ����λ��.
	void               process_camAutoReset( void );


	// ���������λ���뽹���������
	void set_maxDistance( float _maxDis );

	// ���������λ���뽹�����С����
	void set_minDistance( float _minDis );

	// �޸����λ���뽹��ľ�������Ч����������
	void modify_disdance();

	//! ��ת������������Ƕȼ����㣬��������λ��
	void cam_rot( float _pitch, float _yaw );


	//! ������ر�����ײ���,�����������,����ݵ��ζ������λ�ý����Զ�λ�ô���.
	void modify_CollsionTerrain();

	//! ���������֮����,��λ�������ͷ��
	void pitch_camAboveFocus( void );

	//! �������λ��������ɵķ�Χ��
	void pull_camPosToMaxDis( void );

	//! pitchLock�����pitch�б䶯��һ��ʱ����Զ��ָ�Ϊlockʱ��Pitch.
	void update_camera_toLockPitch( float _time );

	//! River @ 2009-4-9:������Զ������������ǰӦ�õ�pitchֵ
	float update_lockPitch( void );





	void play_camEffect( char * _camEffectName, int _nCntPerSec );
	void update_camEffect();
	std::vector< osVec3D > m_camEffectPoint;
	BOOL m_bPlayingCamEffect;
	int m_nCntPerSec;
	float m_fPlayEffectTime;
	osVec3D m_oldCamPos;
	osVec3D m_oldCamFocus;
	osVec3D m_oldLookVec;
	osVec3D m_offset,m_v3;

	BOOL m_bLockPitch;

	osVec3D mLastFramePos;
	BOOL m_NeedCollsionTerrain;

//---------------------------------------------------------------------------------
//@{ \brief : tzz add in tail of class for galaxy camera shaking @ 2008-6-11
//---------------------------------------------------------------------------------
private: //! member data of shaking
	//! swing of camera shaking
	float			m_fShakeSwing;

	//! damp of camera shaking
	float			m_fDamp;

	//! timer to calculate the
	float			m_fCurrTime;

	//! if shake?
	bool			m_bShaking;

private: //! private function to process shaking

	/*! \fn void ShakeCamera(void);
	*	\brief : shake camera when update camera
	*
	*	\param void
	*	\return void
	*/
	void ShakeCamera(void);

//---------------------------------------------------------------------------------
//@}
//---------------------------------------------------------------------------------

};



# include "osCamera.inl"






