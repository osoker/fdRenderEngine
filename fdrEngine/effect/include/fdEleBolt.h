//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdEleBolt.h
 *
 *  Desc:     �����в����绡Ч�����ؽ�.
 *
 *  His:      River created @ 2005-4-18
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once 


# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"

#define	ELECTRIC_BOLT_REDDOMINANT	  0
#define	ELECTRIC_BOLT_GREENDOMINANT	  1
#define	ELECTRIC_BOLT_BLUEDOMINANT	  2

//! ������ʹ�õ�bolt�������Ŀ.
# define MAXBOLT_POLY   32


//==================================================================
//�����,get from genesis engine.
//���Ҫ�������ڵ绡ǹ��Ч��,���Դ��������������һ����.������ʲô
//ʱ������绡.
//
//�������ú���,����������ѡ��,���Բ�ʹ��.
//
//==================================================================
class osc_eleBolt 
{
	friend class    osc_boltMgr;
private:
	bool		m_bInitialized;
	int			m_dwNumPoints;          //������2�Ķ�η�,16,32,64...
	float 		m_fWildness;

	//! ��Bolt�ϴθı䵽���ڵ����ڹ�ȥ��ʱ��.
	float       m_fLastChangeTime;       
	//! ����֡����ǰ֡���ŵ�ʱ�䣬
	float       m_fLastBoltTime; 
	//! bolt�ı�һ����Ҫ��ʱ�䣬�൱�ڼ�����Ƶ�ʵĿ���,�೤ʱ����Ըı�һ�Ρ�
	float       m_fFreqTime;


	/* For rendering */
	osVec3D		 m_vec3Start;
	osVec3D		 m_vec3End;

	/* For generating the geometry */
	osVec3D 	m_vec3CenterPoints[MAXBOLT_POLY+1];
	osVec3D* 	m_vec3CurrentPoint;

	osColor		m_sCurrentColor;
	float       m_arrBaseColors[3];
	float       m_vecCurrentColors[3];

	int			m_iDecayRate;
	int			m_iDominantColor;

	float		m_fWidth;

	//! ��Ⱦpoly�õ��Ķ���
	os_billBoardVertex  m_arrRVertex[MAXBOLT_POLY*4];
	
	//! 
	os_polygon   m_sPoly;  

	//! �Ƿ�׼������Ⱦ���ݡ�
	bool         m_bReadyRender;

	//! bolt�õ��Ķ��㻺����Id������������Id
	static int   m_iVerbufId;
	static int   m_iIdxbufId;


protected:

	void    elebolt_fillpoly( I_camera* _camptr );

	void    subdivide( const osVec3D* start,const osVec3D* end,float s,int n );

	void    genLightning( int RangeLow,int RangeHigh,const osVec3D* start,
		                          const osVec3D* end );

	void    elebolt_animate( const osVec3D* start,const osVec3D* end );

	void    elebolt_setcolor( osColor& _color );

	int     get_dominantColor( osColor& _color );

public:
	osc_eleBolt();
	~osc_eleBolt();


	BOOL    elebolt_init( os_boltInit* _init );

	// ����绡��ص����ԡ�
	void    elebolt_reset( os_boltInit* _init );

	void    framemove( I_camera* _camptr,float eletime );

	BOOL    render( osc_middlePipe* _pipe );

};


//! ������������������128��electric Bolt.
# define   MAX_ELEBOLTINSCENE   64


//! ���⿪�ŵ�eleBoltЧ�����������������Bolt,��Ⱥ�Ƶ�ʿ��Բ�ͬ
struct os_boltId
{
	int     m_iBoltIdx[MAX_ELEPERBOLT];
	int     m_iBotNum;
};

//! ������������֯bolt���ࡣ
class  osc_boltMgr : public ost_Singleton<osc_boltMgr>
{
private:
	CSpaceMgr<osc_eleBolt> m_vecBolt;
	CSpaceMgr<os_boltId>   m_vecBoltIdx;
	
	osc_middlePipe*   m_ptrMPipe;

private:
	osc_boltMgr()
	{
		m_ptrMPipe = NULL;
		m_vecBolt.resize( MAX_ELEBOLTINSCENE );
		m_vecBoltIdx.resize( MAX_ELEBOLTINSCENE );

	}

	//! �ҵ����е�eleBolt.
	int             get_freeEleBolt( void );

public:
	//! ��ʼ������EleBoltMgr.
	void     init_boltMgr( osc_middlePipe* _pipe );

	//! ����һ��bolt.����һ��id.
	int      create_bolt( os_boltInit* _binit,int _bnum );

	//! ���赱ǰ��bolt.
	void     reset_bolt( os_boltInit* _binit,int _idx );

	//! ɾ��һ��bolt
	void     delete_bolt( int _id );

	//!��moveBolt.
	void    frame_move( I_camera* _camptr,float _etime );

	void    render( void );

	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static osc_boltMgr*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void           DInstance( void );

};



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ����EleBoltMgr��Inline������

//! �ҵ����е�eleBolt.
inline int osc_boltMgr::get_freeEleBolt( void )
{
	return m_vecBolt.get_freeNode();
}

