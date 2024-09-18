//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdParticle.h
 *
 *  Desc:     ������������Ч��
 *
 *  His:      River created @ 2004-5-20.
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once 


# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"

//! �����ص��ļ���
# include "fdTrack.h"


//! ���ڴ���ÿһ�����Բ�ֵ��Ԫ��.
# define  MAX_DIVISIONNUM   101

//! ÿһ�����ӳ��������4��emissionControl.
# define  MAX_EMISSION      3

//! ����ÿһ�����ӿ�����������ӵ�е����ӵ���Ŀ
# define  MAX_PARTICLENUM   100



//! ���ӳ���������.
class         os_particleScene;
//! ����������.
class         osc_camera;


//
// �����������ļ������ӱ༭���е��ļ�������ͬ��ȥ�����ô�����Ĳ��֣�
// ʹ�ýṹ���������Ϳ����á�
//
// ���ӵ���Ⱦʹ���м�ܵ��Ķ�̬���㻺����,ÿһ֡��,������,����м�ܵ���̬����
// ������,����Ⱦ��Щ��������.

//
// River @ 2005-5-26:
// �������ӳ����趨����ʱ��ķ������������Ӽ��ٲ��ŵķ���:
// �ҵ����ӳ����Ĳ������ڣ�Ȼ��ʹ�ô����ʱ��������������
// ��ֵ���õ����Ӳ��ŵļ��ٱȡ�
//  
// 


/** \brief
 *  �������п����࣬�����Ӳ��������������ӵ��������̽��п��Ƶ��ࡣ
 * 
 *  �����Ҳ���������ӵ���ʾ��ʽ��������Ƕ����ӵ�������ƣ�������
 *  ��������������ÿһ��������Ȼ�������������еģ���ȴ���Լ���
 *  ���й��ɣ�������ʾ���������еĹ��ɡ�
 */
struct os_particleRunCtl
{
	//! ���ӵ��������ٶ�.
	float      m_fGravity;

	//! ���Ӳ��ܿ�������Ӱ����ٶȵ���.
	float      m_fSpeedNoResis;

	//! ������Ƭ�����С������Ӱ��ķ�ʽ.0:û�� 1:����.
	int        m_iAreaSizeResisType;
	//! �����������СӰ���ϵ��.
	float      m_fAreaSizeCons;


	//! ��ǰ���Ӷ�Ӧ�������ļ����洢Ϊ���·��.
	char       m_szTextureName[128];
	//! ��ǰ���Ӷ�Ӧ������������Ŀ.16����32����õ����ִ�С.
	float      m_fParticleYRot;

	//! ���Ӷ�Ӧ������֡��,�����з�.
	int        m_iFrameNum;
	//! ���Ӷ�Ӧ����������,�����з�.
	int        m_iFrameSets;

	//! ��ǰ������Ⱦʱ�ͱ����Ļ�Ϸ�ʽ.0: ���  1:���.
	int        m_iBlendMode;

	//! ��ǰ���ӵĴ�С����.
	float      m_fSizeMulti;


	//@{
	//! �����������ڴ�С������.
	float     m_vecParSize[MAX_DIVISIONNUM];

	//! ��������������ת������.
	float     m_vecRotation[MAX_DIVISIONNUM];

	//! �����������ڶ����͸����.
	float     m_vecAlpha[MAX_DIVISIONNUM];

	//! �����������ڵ���ɫ.x����r,y����g,z����b.
	osVec3D   m_vecColor[MAX_DIVISIONNUM];
	//@} 

	//! ��ǰ��particle�Ƿ�����ʹ�á�
	bool      m_bInuse;



	//! ��ǰ���ӵ��豸������ݣ�shaderId�š�
	int       m_iShaderId;

public:
	os_particleRunCtl();
	~os_particleRunCtl();


	//! ���ڴ��е��뵱ǰ��particle����.
	bool     load_particleRunCtl( BYTE*& _sdata,int _ver );

	//! ������ǰparticle���豸������ݣ�����id.
	bool     create_shader( void );


};
typedef std::vector< os_particleRunCtl > VEC_parRunCtl;


/** \brief
 *  �������п������ʵ����ÿһ���ɼ���������Ƭ��Ӧ����һ��ʵ����
 *
 */
struct  os_parRunCtlIns
{
	//! ��ǰ��particleInstance�Ƿ񼤻
	bool         m_bAlive;

	//! �����ӳ�������ǰ��ȥ��ʱ�䡣
	float        m_fElapseTime;

	//! ��ǰ���ӵ��������ڳ���.
	float        m_fLifeTime;

	//! ������T���ϵ�ֵ�����ڵõ�������ص��������ݡ�
	float        m_fTval;


	//! ��ǰ�����õ���ͼ֡��
	int          m_iTexFrame;
	//! ��ǰ�����õ���������ͼ������������
	int          m_iTexSetsIdx;


	//! ���ӵ�ǰ�ļ��ٶ�ֵ��
	osVec3D      m_vec3Velocity;
	//! ���ӵ�ǰ֡�ܵ�������ֵ��
	osVec3D      m_vec3Resistance;

	//! ���ӵ�ǰλ��������
	osVec3D      m_vec3ParticlePos;

	//! ���ӵ�ǰ�Ĵ�С��
	float        m_fSize;
	//! ���Ӳ��ʵ�ǰAlphaֵ��
	float        m_fAlpha;
	//! ���ӵ�ǰ��ɫֵ����alphaֵ�ϳɣ���Ϊ�������յ���ɫֵ��
	osVec3D      m_vec3Color;
	//! ���ӵ�ǰ��ת�Ƕȡ�
	float        m_fRotation;

public:
	//! ��Ⱦ��صĺ�����
	void         reset_particle( void );

};


/** \brief
 *  ���Ӳ������Ŀ����ࡣ
 *
 */
struct os_particleEmitter
{
	//! ��ǰemitter�������ӵĵ����λ��,�����(0,0,0).
	osVec3D       m_vec3RelativePosition;

	//! ��ǰemitter��������ʱ�����������ٶȵķ�Χ.
	osVec3D       m_vec3EmissionSpeedScope;

	// 
	//! ��ǰemitter����״.0: square 1:sphere, 2: custom shape.
	//! River added @ 2006-6-30: ������ĳ���߶εķ�Χ�ڲ������ӵĹ��� 3: LineSeg shape.
	//  River @ 2008-6-11:�����������״��ֻ��fallOff���δ��ϲ������� 4: Ring Shape
	//                    ring shape��sphereShape��������ͬ������sphereShape��hotPt���������ӣ�
	//                    ���е����Ӷ���FallOff�ڲ���
	//! �߶εĿ�ʼ�ͽ���������Զ��壬��һ��Ĭ�ϵ�����.
	int          m_iEmitterShape;


	//! ��ǰemitter�������ӵ�Bounding Box.
	os_aabbox       m_sbox;
	

	//  
	//! River mod @ 2008-6-11:������fallOff���δ��������ӣ���
	//! HotPt�����������ӣ����������������м�������Ч����
	//! ��ǰemitter�������ӵ�bounding Sphere.
	float        m_fHotPt;
	float        m_fFallOff;

	//! �����ǰ��emitter�õ����������״,��Ӧ�ļ���.
	char          m_szCSFname[64];

	//! ��ǰemitterÿһ�����ڵĳ���.
	float         m_fCycleLength;

	//! ��ǰemitter�Ƿ�������ѭ����������.
	BOOL          m_bLoopCycle;

	//! ��ǰemitter�������ӵ��ٶȵı���.
	float         m_fGenSpeedMulti;


	//@{
	//! emitter��ص���������,��Щ��������������Ҫ��ֵ.
	osVec3D       m_vec3Dir[MAX_DIVISIONNUM];

	//! ���Ӳ��������������ӵ��������ֵ.
	osVec2D       m_vec2Rand[MAX_DIVISIONNUM];

	//! ���Ӳ��������������ӵ�ֵ.
	osVec2D       m_vec2EmissionRate[MAX_DIVISIONNUM];

	//! ���Ӳ��������������ӵ���������ֵ.
	osVec2D       m_vec2EleLife[MAX_DIVISIONNUM];

	//@} 

	//! River @ 2006-5-18:������Զ������������״������trackId
	int           m_iTrackId;

public:
	os_particleEmitter();
	~os_particleEmitter();

	//! ���ڴ��е���particleEmitter�����ݡ�
	void         load_emitterFromMem( BYTE*& _ps );


	//! ��һ��tvalue�õ���ǰemitter����������ӵķ���.
	void         get_emitDirection( osVec3D& _dir,float _tval );

	//! ��һ��tvalue�õ���ǰemitter��������ӵ�velocity.
	void         get_emitRandVelocity( osVec2D& _randVel,float _tval );

	//! ��һ��tvalue�õ���ǰemitter�������ӵ��ٶ�.
	void         get_emissionRate( osVec2D& _erate,float _tval );

	//! ��һ��tvalue�õ���ǰemitter����������������е���������.
	void         get_emitParticleLifeTime( osVec2D& _ltime,float _tval );


};
typedef std::vector< os_particleEmitter > VEC_parEmitter;


/** \brief
 * ���Ӳ�������ʵ���ࡣ
 *
 * ÿһ�����������һ��������ʵ�����������������������ʵ����
 *  
 */
struct os_particleEmtIns
{
	//! emitter��ǰ�Ƿ񼤻�.
	bool         m_bAlive;

	//! eimtter��ǰ��T���ϵ�����ֵ��
	float        m_fTVal;

	//! emitter�������ӵĳ�ʼ������
	osVec3D      m_vec3Dir;

	//@{
	//  ���еĶ�ά���ֵ����Сֵ:xΪ��Сֵ,yΪ���ֵ.
	//! emitter��ǰ������ٶȵ�������Сֵ��
	osVec2D      m_vec2RandVelocity;

	//! emitter����������Ŀ�����ֵ����Сֵ����
	osVec2D       m_vec2EmissionRate;

	//! emitter�����������ӵ��������ڵ����ֵ����Сֵ��
	osVec2D       m_vec2ParLifeTime;
	//@} 
    
};


/** \brief
 *  ���Ӳ���ϵͳ���ࡣ
 *
 *  ���Ӳ���ϵͳ��Ӧ�����Ӳ���������������п����ࡣ
 *  ʹ�����Ӳ������࣬���Բ��������ӣ������Ӳ��������󣬿���ʹ��
 *  �������п��������������������������е���Ϊ��
 */
struct os_particleEmissionCtl
{
	//! ��Ӧ���������п��ƽṹ������.
	int             m_iParticleRunIdx;

	//! ��Ӧ�����Ӳ��������Ƶ�����.
	int             m_iParEmitterIdx;

	//! �������������û�пռ�,�������������ӿռ�.
	int             m_iReplaceStyle;

	//! emission��ѭ��ʱ��.
	float           m_fCycleLength;


public:
	os_particleEmissionCtl();
	~os_particleEmissionCtl();

	//! ���ڴ����emissionControl�����ݡ�
	void           load_emiCtlFromMem( BYTE*& _ps );

};

typedef std::vector< os_particleEmissionCtl > VEC_parEmissionCtl;






/** \brief
 *  emissonControl������ʵ����
 *
 *  �����пɼ����ӵļ���֯����һ�����Ӳ�����ʵ������������Ӳ�����
 *  ������һ��ѵ�����ʵ�����ɡ�
 *
 *  ����ϵͳ������Ҫ�Ĵ���λ,emissionControlInstanceʹemitter��������,
 *  Ȼ��Բ��������ӽ��и���,Ȼ����ȡÿһ�����ӵ���Ⱦ����,��ʹ���м����
 *  ����Щ���ݽ�����Ⱦ.
 *  
 *  ʹ��emission Instance��Ϊһ����Ⱦ��λ?????
 */
class os_emssionCtlIns
{
private:
	//! ���ӳ�����ָ��.
	os_particleScene*        m_ptrParScene;
	//! ָ���emissionControl��ָ��.
	os_particleEmissionCtl*   m_ptrEmissionCtl;



	//! �������µ����ӵ��滻��ʽ��
	int                    m_iNpwmr;

	//! ���ӿ���emitter��ʵ����
	os_particleEmtIns       m_particleEmtIns;

	//! ��ǰemissionCtlIns��Ӧ�����ӵ��б�.
	os_parRunCtlIns         m_vecParticle[MAX_PARTICLENUM];

	os_billBoardVertex      m_vecRenderVer[MAX_PARTICLENUM*4];

	//! ָ���emitter��ָ��.
	os_particleEmitter*     m_ptrEmitter;

	//! ָ������ӿ��Ƶ�ָ��.
	os_particleRunCtl*      m_ptrParticleRunCtl;



	//! ��һ֡�����ڹ�ȥ��ʱ��.
	float              m_fTimeLastFrame;

	//! ��emission��ѭ����ʼ�����ڹ�ȥ��ʱ��.from loop start.
	float              m_fTimeFromLS;

	//! ��һ֡û�����ȥ��������Ŀ��
	float              m_fAccumulate;




	//! ��ǰ�õ������Ӷ�����Ŀ��
	int                m_iParVertexNum;

	//! ��Ⱦ��ǰ�����ӿ����õ���polygonָ�롣
	os_polygon          m_sPolygon;

	//@{
	//! River added @ 2006-6-30:
	//! �����߶η�Χ�ڲ������ӵĿ�ʼ��ͽ�����
	osVec3D             m_vec3LSStart;
	osVec3D             m_vec3LSEnd;
	//@}

	//! ָ������Ӳ��������Ƶ�ָ�롣

	//! River 2006-4-25:�������ӵ�����λ��֮���ֵ��ʹ�������������ȷ�ɢ������
	osVec3D             m_vec3LastEmitPos;
	//! ���Ӳ�������λ���������֡�Ƿ�ı���
	BOOL                m_bEmitPosChanged;

	//! River @ 2006-6-10:���ӵ����Ź��ܼ��롣
	float               m_fSizeScale;
	//! ��һ�β������ӣ����ڴ�����������ر��(����һ����Ⱦ֡ʱ��)��������Ч��
	BOOL                m_bFirstFrameGenPar;

	//! �����²�����ÿһ�����ӵ�λ��
	void                cal_newParticlePos( osVec3D* _pos,osVec3D& _offset,int _num );


private:

	/** \brief
	 *   ��ʼ��һ������.
	 *   
	 */
	void           init_particle( int _idx,osVec3D& _offset );
	/** \brief
	 *   �ӵ�ǰ�����Ӷ��е�,���һ����Ŀ������.
	 *   
	 */
	void           replace_particle( int _rnum,osVec3D* _offset );
	/** \brief
	 *  �����ӽ��и���.
	 */
	void           update_particle( int _idx,osVec3D& _offset );


	//@{
	//  ������������ݽ��в��������Ժ���.

	//! ���Ӳ������Ƿ������ս�,����.
	void           emission_cycleEnd( void );
	//! ���¿�ʼһ��emitter��cycle.
	void           emitter_cycleStart( void );
	//! �õ�emitterInstantce�ڵ�ǰʱ�̵�����.
	void           get_emitterInsData( void );

	//! �õ���ǰʱ�̲���emitter�������ӵ���Ŀ.
	int            get_emitterGenParNum( void );


	//! �õ����ӳ�ʼ����velocity.
	void           get_ptlInitVelocity( int _idx );

	//! �õ����ӵ�resistantance.
	void           get_particleResistance( int _idx );

	//! �õ����ӵĳ�ʼ��λ��.
	void           get_ptlInitPosition( int _idx );

	//! �õ����ӵĴ�С.
	void           get_parSize( int _idx );

	//! �õ����ӵ�alpha
	void           get_parAlpha( int _idx );

	//! �õ����ӵ�color
	void           get_parColor( int _idx );

	//! �õ����ӵ���ת.
	void           get_parRotation( int _idx );

	//! �õ���ǰ���ӵ�tvalue
	void           get_tvalue( int _idx );
	//! �õ���ǰ�����õ�������֡��.
	void           get_textureFrame( int _idx );

	//@} 


	/** \brief
	 *  ÿһ֡�Ե�ǰ��emissionControlInstance���и��¡�
	 *
	 *  �������ӿ����ڵ�emitterIns��particleInsArray.
	 *
	 *  \param _offset ������ӷ�ɢ������ռ�,��Ҫ���_offset�����ӽ��д���.
	 */
	void          framemove_emission( float _etime,osVec3D& _offset ); 

	/** \brief
	 *  ��䵱ǰemissionControl Instance�ڵ�����Array��ָ�������ݻ������С�
	 *
	 */
	void          fill_renderVerBuf( os_billBoardVertex* _verStart,osc_camera* _cam );

public:
	os_emssionCtlIns();

	//! ��ʼ����ǰ�����ӿ��ơ�
	void          init_emissionCtlIns( os_particleScene* _sce,
		                             os_particleEmissionCtl* _emi,float _sizeScale );

	//! �Ե�ǰ��emiControlIns����FrameMove.
	void          frameMove_eci( osVec3D& _offset,float _etime,osc_camera* _cam );


	/** \brief
	 *  ��Ⱦ��ǰ��emissionControlInstance.
	 *
	 *  \param _offset �й����ӵ�λ���ƶ�ʹ���������������.
	 *  
	 */
	bool          render_emissionControlIns( osc_middlePipe* _pipe,osVec3D& _offset );


	//! ���赱ǰ��emssionInstance.
	void          reset_emssionIns( void );

	//! �õ�emitter��ָ���б�
	os_particleEmitter*  get_emitterList ( void ) { return m_ptrEmitter; } 

	//! �������Ӳ������ڵ��߶η�Χ
	void          set_parGetLineSeg( osVec3D& _start,osVec3D& _end );

	//! ��ǰ�����ӳ����Ƿ��ǵ����߶ε�����
	bool          is_lineSegPar( void ) ;

	//! ��ǰ��emicontrolIns�Ƿ񼤻
	bool          is_alive( void );

	//! ֹͣ���䵱ǰ������,����ƽ�͵Ľ������ӵ�����
	void stop_emissionIns( void ) { m_particleEmtIns.m_bAlive = false; }

	//! �Ƿ���ѭ�����ŵ�emitter.
	bool          is_loopPlayEmitter( void ) 
	{ 
		if( m_ptrEmitter ) return m_ptrEmitter->m_bLoopCycle;
		else return false;
	}

};





/** \brief
 *  ���ӳ����࣬������һ�����߶�������Ӳ���ϵͳ.
 *
 *  
 */
class os_particleScene : public os_lruEle
{
	//! ʹ��os_particleScene�������ࡣ
	friend class    os_particleSceneIns;
	friend class    os_emssionCtlIns;
private:

	//! ��ǰparticle scene������.
	s_string        m_szParSceneName;
	//! hash val.
	DWORD           m_dwHash;

	//! ������emission Control����Ŀ.
	int          m_iEmissionCtlNum;
	VEC_parEmissionCtl   m_vecEmssionCtl;


	//! �������������п���������Ŀ.
	int          m_iParRunCtlNum;
	VEC_parRunCtl m_vecParRunCtl;


	//! ���������Ӳ���������Ŀ.
	int          m_iEmitterNum;
	VEC_parEmitter m_vecParEmitter;

	//! ���ӳ������Ƶ����������Ŀ.
	int           m_iMaxParticleNum;

	//! ���ӷ���Ŀռ�:��:����ռ�.1:���ӿռ�.
	int           m_iEmitterTo;

	//! �²������������������е�����. 1:�滻���ϵ�����. 2:������滻����.
	int           m_iNpwmr;

	//! ���ӵķŴ���.
	float         m_fMultiSize;



	//@{
	//  ·����ص�����.
	//! ����ʹ��ʹ����·���ļ�.
	bool          m_bUseTrack;
	//! ·���ļ�����.
	char          m_szTrackFileName[128];
	//! �������е��ٶ�.
	float         m_fRunSpeed;

	//@} 

	//! ��ǰ�����Ƿ�ʹ����.
	bool          m_bInuse;

	//! �Ƿ�����û����ڴ�.
	bool          m_bCanSwap;

public:
	os_particleScene();
	~os_particleScene();

	//! ���ļ��ж������ǵ�Particle���ݡ�
	bool          load_parSceneFromFile( char * _fname );

	//! ���ڴ����ͷ�һ������.
	void          release_parScene( void );

	//! �Ƿ�������Ҫ�����ĳ���.
	bool          is_needScene( int _hash ) { return (_hash==m_dwHash); }

	//! ��ǰ�����Ƿ���ʹ����
	bool          is_sceneInuse( void )             { return m_bInuse; } 

	const char*   get_sceneName( void )   { return m_szParSceneName.c_str(); } 

	//
	bool          get_canSwap( void ) { return m_bCanSwap; }
	void          set_swap( bool _s )  {  m_bCanSwap = _s; } 


};
typedef std::vector< os_particleScene >  VEC_particleScene;

/** \brief
 *  ���ӳ�����ʵ����,�����в�����ͬ�ļ������ӳ�����ʱ��,
 *  ���Թ������ӳ�����,��ʹ�ò�ͬ�����ӳ���ʵ��.
 *
 *  
 */ 
class os_particleSceneIns : public os_createVal
{
	friend class osc_particleMgr;
private:
	//! ��ǰ���ӳ�����ָ�롣
	os_particleScene*   m_ptrParScene;


	//! ���ӳ����ڵ�emissonControlʵ�����ơ�
	os_emssionCtlIns    m_arrEmissonCtlIns[MAX_EMISSION];

	//! ��ǰ���ӳ���������ռ��е�λ��,���ϲ㺯�����п���.
	osVec3D            m_vec3Offset;

	//! ��ǰ���ӵ�����ƫ��λ��,��ɢ�����ӿռ������ʹ��
	osVec3D            m_vec3FinalOffset;

	//! ��ǰʵ���Ƿ�ʹ�á�
	bool               m_bInuse;
	//! ��ǰʵ���Ƿ񶥲���Ⱦ
	bool               m_bTopLayer;

	//! ���ŵ�ǰ������ʱ���õ�ʱ����ٱȡ�
	float              m_fTimeScale;

	//! ��ǰ���ӳ�����·��id.
	int                m_iTrackId;

	//! ��ǰ�������Ƿ���ʾ
	bool               m_bHide;
public:
	os_particleSceneIns();
	~os_particleSceneIns();


	/** \brief
	 *  ʹ�ó���ָ������ʼ��һ��particleInstance.
	 *
	 *  \param _scale �������ӵ�����ֵ,���������
	 */
	WORD               init_particleIns( os_particleScene* _psce,osVec3D& _offset,
		                    float _scale,BOOL _inScene = FALSE,bool _topLayer = false );

	//! ��partileIns����framemove.
	void               frameMove_curIns( float _etime,osc_camera* _cam,bool _canDeleteCurIns = false );

	/** \brief
	 *  ��Ⱦ��ǰ��particleSceneInstance.
	 *
	 *  \param _etime ��һ֡��ȥ��ʱ�䡣
	 *  
	 */
	bool               render_currentIns( osc_camera* _cam,osc_middlePipe* _mpipe );
		                 
	//! ���õ�ǰparticleIns��λ��.
	void               set_curInsPos( osVec3D& _vec3 )    { m_vec3Offset = _vec3; } 
	

	//! ��ǰ���ӳ���ʵ���Ƿ���ʹ����,
	bool               is_inUse( void )  { return m_bInuse; } 

	//! ���õ�ǰ���ӳ���ʵ���Ĳ���ʱ�䡣
	void               set_parPlayTime( float _timeScale );

	//! �õ���ǰ���ӳ���ʵ���Ĳ���ʱ�䡣
	float              get_parPlayTime( void );

	//! ��ǰ�����ӳ����Ƿ���ѭ�����ŵ����ӳ���
	bool               is_loopPlayParticle( void );

	//! ʹ�õ�ǰ��instance��Ϊ������.
	void               delete_parSceIns( bool _finalRelease = false ) ;

	//! ƽ�͵�ֹͣ��ǰ�����ӳ���
	void               stop_parSceIns( void );

	//! �Ƿ�topLayer Render.
	bool               get_topLayer( void ) { return m_bTopLayer; } 

	//! ���ػ�����ʾ��ǰ��particle.
	void               hide_particle( bool _hide ) { m_bHide = _hide; } 

	//! �����Զ������ӵķ�ɢ�߶η�Χ
	void               set_parGetLineSeg( osVec3D& _start,osVec3D& _end );

	//! �Ƿ������ϲ������߶η�����������
	bool               is_lineSegParticle( void ) { return m_arrEmissonCtlIns[0].is_lineSegPar(); };


	//! River added @ 2010-6-9:
	//! ��ǰ��ins�Ƿ��Ѿ���Ⱦ,�����������ϵ�������Ч��
	//! �����������ڽ�������Ⱦ��ʱ�����
	bool               m_bRenderedInCurFrame;

};


//===================================================================================================
// particle Mgr��صĺ�������
//! ����������п���ʹ���������ӳ�����Ŀ��
# define      MAX_PARTICLEFILE     64
# define      MAX_PARTICLEINS      48 


/** \brief
 *  ���ӹ�������.
 *
 *  ���ӹ��������������ӳ����Ĵ���,���ӳ�������Ⱦ.
 *  �ϲ�ʹ�õ����нӿڶ��������õ�.
 *
 *  River @ 2006-6-2:
 *  ���ӵ�ɾ����������FrameMove����Ⱦ������Ҫ�������ٽ�����
 */
class osc_particleMgr
{
	friend class os_particleSceneIns;
private:
	//! ���ӳ�����������.
	CSpaceMgr<os_particleScene> m_vecParticleScene;

	//! ��������ʵ����������,os_particleSceneIns�������Ƚϴ�����ʹ��48�������顣
	CSpaceMgr<os_particleSceneIns> m_vecParSceIns;


	//! �����豸������ݵ��м�ܵ���
	osc_middlePipe*      m_ptrMPipe;

	//! �ͷ��ʱ�䲻ʹ��particle.
	void      discard_lruParticleScene( void );


public:
	//@{
	//  ���������е�����ʹ��ͬһ�����㻺��������䶥�㵽�������ڲ�ͬ��λ���ϡ�
	//  ��ʼ��particleMgr��ʱ��Ҫ��ʼ����Щ���ݡ�
	//  
	//! ��ǰ���ӳ����õ��Ķ��㻺������id.
	static int                 m_iVbId;

	/** \brief
	 *  ��ǰ���ӳ����õ���index buffer��������������
	 *
	 *  ���е����ӳ�����ʹ����һ��idx buffer,����Ҫ�������idxBuffer,
	 *  ֻ��Ҫһ���Գ�ʼ�������ˡ�
	 */
	static int                 m_iIbId;

	//@} 

public:
	osc_particleMgr();
	~osc_particleMgr();

	//! ��ʼ����ǰ��particle������,��ʼ����ȫ�����豸��ص����ݡ�
	void              init_particleMgr( osc_middlePipe* _pipe );

	//! ��ParticleMgr�е�particleInstance����FrameMove,��Ⱦ�ʹ���ֿ�.
	void              framemove_particle( osc_camera* _cam );

	//! ������Ⱦparticle.
	bool              render_particle( osc_camera* _cam,os_FightSceneRType _rtype = OS_COMMONRENDER );


	/** \bief
	 *  ����һ�����ӳ���ʵ��.
	 * 
	 *  \return int �����˵�ǰparticle��id.
	 */
	int               create_parSceIns( os_particleCreate& _pcreate,BOOL _inScene = FALSE,bool _topLayer = false );

	/** \brief
	 *  ����һ��particle����λ������.
	 *
	 *  \param _forceUpdate ǿ�Ƹ��µ�ǰ������λ�õ���Ⱦ���ݣ����ڸ���������������������ȷ����Ⱦ
	 */
	void              set_particlePos( osVec3D& _pos,int _idx,bool _forceUpdate = false );

	//! ɾ��һ��particle.
	void              delete_particle( int _idx,bool _finalRelease = false );

	const char*       get_parNameFromId( int _id );


	//! �������ӳ����Ĳ���ʱ�䡣
	void              set_parPlayTime( int _id,float _timeScale );
	//! �������ӳ����Ĳ���ʱ�䡣
	float             get_parPlayTime( int _id );

	//! �Ƿ���ѭ�����ŵĹ����
	bool              is_loopPlayParticle( int _id );

	//! ���ش����particle idЧ��
	bool              hide_particle( int _id,bool _hide );

	//! River @ 2010-6-9:Ϊ���������ϵ�������Ⱦ�ڽ�������ȷ.
	void              reset_particleInsRender( int _id );


	/** \brief
	 *  �������id��Ӧ���ӵ���Ч��.
	 *
	 *  �����Ӧ�����Ӵ�������״̬������ǰ
	 *  ������id.
	 *  \return bool �����Ч������true.
	 */
	bool              validate_parId( int _id );

	//! �������ӵķ����߶εĿ�ʼ�ͽ���λ��
	void              set_parGenLineSeg( int _id,osVec3D& _start,osVec3D& _end );

};



//! ����particle��inline����.
# include "fdParticle.inl"





