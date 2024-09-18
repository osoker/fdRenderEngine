//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdEleBolt.h
 *
 *  Desc:     场景中产生电弧效果的特交.
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

//! 最多可以使用的bolt多边形数目.
# define MAXBOLT_POLY   32


//==================================================================
//电光类,get from genesis engine.
//如果要做类似于电弧枪的效果,可以从这个类再派生出一个类.来控制什么
//时候产生电弧.
//
//整理出最常用函数,其它函数是选项,可以不使用.
//
//==================================================================
class osc_eleBolt 
{
	friend class    osc_boltMgr;
private:
	bool		m_bInitialized;
	int			m_dwNumPoints;          //必须是2的多次方,16,32,64...
	float 		m_fWildness;

	//! 从Bolt上次改变到现在到现在过去的时间.
	float       m_fLastChangeTime;       
	//! 从上帧到当前帧流逝的时间，
	float       m_fLastBoltTime; 
	//! bolt改变一次需要的时间，相当于加入了频率的慨念,多长时间可以改变一次。
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

	//! 渲染poly用到的顶点
	os_billBoardVertex  m_arrRVertex[MAXBOLT_POLY*4];
	
	//! 
	os_polygon   m_sPoly;  

	//! 是否准备好渲染数据。
	bool         m_bReadyRender;

	//! bolt用到的顶点缓冲区Id和索引缓冲区Id
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

	// 重设电弧相关的属性。
	void    elebolt_reset( os_boltInit* _init );

	void    framemove( I_camera* _camptr,float eletime );

	BOOL    render( osc_middlePipe* _pipe );

};


//! 整个场景中最多可以有128个electric Bolt.
# define   MAX_ELEBOLTINSCENE   64


//! 对外开放的eleBolt效果可以有最多两个的Bolt,宽度和频率可以不同
struct os_boltId
{
	int     m_iBoltIdx[MAX_ELEPERBOLT];
	int     m_iBotNum;
};

//! 整个场景中组织bolt的类。
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

	//! 找到空闲的eleBolt.
	int             get_freeEleBolt( void );

public:
	//! 初始化整个EleBoltMgr.
	void     init_boltMgr( osc_middlePipe* _pipe );

	//! 创建一个bolt.返回一个id.
	int      create_bolt( os_boltInit* _binit,int _bnum );

	//! 重设当前的bolt.
	void     reset_bolt( os_boltInit* _binit,int _idx );

	//! 删除一个bolt
	void     delete_bolt( int _id );

	//!　moveBolt.
	void    frame_move( I_camera* _camptr,float _etime );

	void    render( void );

	/** 得到一个sg_timer的Instance指针.
	 */
	static osc_boltMgr*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void           DInstance( void );

};



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 整个EleBoltMgr的Inline函数。

//! 找到空闲的eleBolt.
inline int osc_boltMgr::get_freeEleBolt( void )
{
	return m_vecBolt.get_freeNode();
}

