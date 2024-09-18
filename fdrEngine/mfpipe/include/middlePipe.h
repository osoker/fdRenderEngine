//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: middlePipe.h
 *
 *   Desc:     O.S.O.K 引擎的中间管理,中间管道管理了所有的设备相关的资源.
 *
 *   His:      River Created @ 4/29 2003
 *
 *  “项目成败的关键在于拥有良好的准备工作,并让有见识的项目出资者了解,
 *    项目人员投注了足够的精力在准备工作,以减少后续问题的发生。”
 *
 */
//--------------------------------------------------------------------------------------------------------
#  pragma   once

#  include "../../interface/osInterface.h"
#  include "osShader.h"
#  include "frontPipe.h"
#  include "osPolygon.h"
#  include "osMesh.h"


class  osc_camera;
struct os_mirror;


//! 填充顶点缓冲区的数据指针。
struct os_vbufFillData
{
	char*   m_ptrDstart;
	int     m_iVerNum;

public:
	os_vbufFillData( void )
	{
		m_ptrDstart = NULL;
		m_iVerNum = 0;
	}
};

/// 中间管道所用vertex buffer的结构
struct os_verticesBuf
{
	//! 3d 设备的顶点缓冲区.
	LPDIRECT3DVERTEXBUFFER9    m_verbuf;

	//! 当前的顶点缓冲区是不是已经使用.
	bool                       m_bInuse;

	//!  顶点缓冲区中每个顶点的长度.
	int                        m_iVerSize;

	//!  顶点缓冲区可以容纳顶点的数目.
	int                        m_iCapVerNum;

	//!  顶点缓冲区中顶点的数目.
	int                        m_iVerNum;

	//!  顶点缓冲区的标志位.
	DWORD                      m_vbFlag;

	//!  当前顶点缓冲区已经使用的大小。
	int                        m_iUsedSize;

	//!  当前顶点缓冲区总的大小.
	int                        m_iVBSize;

	//!  当前的顶点缓冲区是否是独立的顶点缓冲区
	bool                       m_bNewVB;

public:
	os_verticesBuf()
	{
		m_verbuf = NULL;
		m_bNewVB = false;
		reset_buf();
	}

	~os_verticesBuf()
	{
		reset_buf();
	}

	void reset_buf( void );
};

/// 中间管道使用的Indices缓冲区的结构.在所有时刻都使用WORD Indices
struct os_indicesBuf
{
	//! 3d 设备用到的Index buffer.
	LPDIRECT3DINDEXBUFFER9     m_idxbuf;

	//! 当前结构是不是已经使用.
	bool                       m_bInuse;

	//! 当前缓冲区可容纳的Indices数目.
	int                        m_iCapIdxNum;

	//! 当前缓冲区中的Indices数目.
	int                        m_iIdxNum;

	//! 当前缓冲区的标志位.
	DWORD                      m_ibFlag;

public:
	os_indicesBuf()
	{
		m_idxbuf = NULL;
		m_bInuse = false;

		m_iCapIdxNum = 0;
		m_iIdxNum = 0;
		m_ibFlag = 0;

	}
	~os_indicesBuf()
	{
		reset_buf();
	}

	void reset_buf( void )
	{
		if( m_bInuse )
		{
			SAFE_RELEASE( m_idxbuf );
		
			m_idxbuf = NULL;
			m_bInuse = false;

			m_iCapIdxNum = 0;
			m_iIdxNum = 0;
			m_ibFlag = 0;
		}
	}

};

/** \brief 
 *  设备相关的顶点缓冲区指针的Vector.
 */
typedef std::vector<os_verticesBuf>          VEC_verBuffer;
typedef std::vector<os_indicesBuf>           VEC_idxBuffer;


//*********************************************************************

/** \brief
 *  O.S.O.K引擎的中间管道.管理了所有的设备相关资源,并且把后端管道的资源
 *  渲染到3D设备.
 *
 *  中间管理管理的资源: 
 *  1: 引擎运行过程中需要的所有的shader.
 *  2: 引擎后端管道中所有的顶点&顶点索引资源.
 *  3: 引擎的前端管道.
 *  4: 当前帧引擎需要渲染的多边形指针列表.
 *
 *  这个类是引擎内部资源管理类,不对上层应用程序公开.
 *  中间管道是一个比较复杂的处理过程,对前端管道发过来的多边形进行排序后,
 *  再送到前端管道进行渲染.
 *
 *  中间管道功能描述:
 *      中间管道初始化阶段:
 *                  需要初始化前端管道,并且把初始化中间管道内部管理的各个资源块.
 *      场景调入阶段:
 *                  中间管道需要把前端管道送过来的内存多边形数据创建成设备相关的数据.
 *  				并且把设备相关的一些数据反馈给要创建设备相关数据的多边形.中间管道
 *  				只需要创建.管理和使用设备相关数据,设备相关数据的信息则由提供数据
 *  				来源的单位进行记录.比如场景中的多边形需要记录这个多边形所包含Shader
 *  				的ID.  
 *      运行阶段: 
 *                  运行阶段中间管道可以接受前端管道送过来的多边形指针.在前端管道要渲染
 *                  的时候,中间管道把已经收到的多边形按shader排序,并渲染这些排完序的多
 *                  边形.
 *                  可以设备渲染设备的一些简单状态,比如世界转换Matrix.
 *                  可以得到所渲染多边形的信息.
 *                  动态的创建一些渲染资源.
 *                  设置一些底层的3d设备信息,比如shader constants.
 *                  应该可以支持设备Reset.......
 * 
 *                  其它功能.......
 */
class osc_middlePipe
{
public:
	osc_middlePipe();
	~osc_middlePipe();


	//! 全局的动态索引缓冲区的ID.
	enum {
		DYNAMIC_IB_ID = 0,
	};

	//===========================================================================================
	//
	//  中间管道初始化阶段的功能:
	//
	//===========================================================================================
	/** \brief
	 *  初始化中间管道.
	 *
	 *  中间管道需要初始化内部各个管理资源模块.
	 *  从deviceMgr接口得到初始化中间管道需要的所有的信息.
	 */
	bool                init_middlePipe( I_deviceManager* _devmgr );

	/// 释放中间管道
	void                release_middelePipe( void );


	//===========================================================================================
	//
	//          中间管道中mesh相关的函数,用于调入，渲染d3dxMesh.
	//
	//===========================================================================================
	/** \brief
	 *  处理aniMesh数据的动态光
	 *
	 *  场景中作用在物品上的动态光是假的动态光，在渲染此物品的时候，作为此物品的
	 *  ambient光来让此物品显的更亮，有动态光的效果.
	 */
	void                process_amDlight( os_dLightData& _dlight );

	/** 为上层提供mesh的渲染函数
	 *  \param _mr 渲染mesh所填充的结构
	 */
	bool                render_mesh( os_meshRender* _mr,bool _alpha = false );


	//! 每一帧上层需要调用的keyAniMesh渲染函数
	bool                frame_renderKeyAniMesh( 
		                  os_ViewFrustum* _vf,bool _alpha,os_FightSceneRType _rtype = OS_COMMONRENDER );

	//! 处理水面的渲染
	bool                render_aniWater( os_ViewFrustum* _vf );

	//! 处理雾的渲染
	bool                render_aniFog(  int ShaderId,int dwtype = 2);
	
		                  

	/** \brief 
     *  为上层创建static vertex buffer.
	 *
	 *  根据传入的数据大小创建一个固定大小的顶点缓冲区。
	 *  返回创建的VB在中间管道buf索引.如果返回-1,创建失败.
	 *
	 *  \param 是否使用系统内存放我们创建的顶点缓冲区。
	 *  
	 */
	int                 create_staticVB( int _vsize,int _vnum,bool _sysmem );
	/** \brief
	 *  根据Vertexbuffer的id,填充VB使用的数据。
	 *
	 *  \param _vstart 填充顶点缓冲区的偏移位置.可以多次DIP使用一个顶点缓冲区
	 */
	bool                fill_staticVB( int _vbIdx,
		                 os_vbufFillData* _vdata,int _num,int _vstart = 0 );

	//! 对已经创建的static vb进年数据填充，用于重用我们已经创建的static vb
	bool                refill_staticVB( const void* _vdata,int _pvsize,int _vnum,int _id );
	//! 释放当前的静态顶点缓冲区。
	void                release_staticVB( int _vbIdx );


	/** \brief
	 *  为上层创建static Index buffer.
	 *
	 *  所有的Index为WORD类型,静态的顶点使用一个Index buffer.
	 *  Index data需要在上层处理,使需要在一起渲染的Index&Vertex
	 *  在缓冲区中也是集中在一起的.
	 *
	 */
    int                 create_staticIB( const void* _idata,int _inum );

	//! 释放静态的IndexBuffer数据。
	void                 release_staticIB( int _id );

	/** \brief
     *  为上层提供填充动态Index buffer的函数.
	 *  
	 *  全局只有一个动态的索引缓冲区,所以上层不需要知道此缓冲区的索引.
	 *  
	 *  \return int 返回了动态填充的Index在整个buf中的位置.
	 */
	int                 fill_dynamicIB( const WORD* _idata,int _inum );

# ifdef _DEBUG
	int                  get_inuseSibNum( void );
	void                 get_textureStageState( DWORD _Stage,D3DTEXTURESTAGESTATETYPE _Type,DWORD* _Value );
# endif 


	/** \brief
	 *  为上层创建dynamic Vertex buffer,返回了创建dvb的索引。
	 *
	 *  \param _int _size  要创建的动态顶点缓冲区的大小.如果传入0,则使用系统默认的大小.
	 *  \return int        返回动态顶点缓冲区的索引.
	 *  \param BOOL _newVB 是否要创建一个新的缓冲区,不管系统内有无此格式的数据.
	 */
	int                 create_dynamicVB( DWORD _fvf,int _size = 0,BOOL _newVB = false );

	//! 使用顶点数组填充我们的动态顶点缓冲区
	int                 fill_dnamicVB( int _id,void* _ver,int _versize,int _vernum );


	/** \brief 
	 *  释放设备相关数据.
	 *
	 *  如果要调入一个样的场景,就需要调用这个函数把中间管道中的设备相关数据释放掉.
	 *  如果要Reset设备,需要释放中间管道所有的资源,并重新创建设备相关的的中间管道资源.
	 */
	void                release_deviceDepData( bool _finalRelease = true );


	//=========================================================================
	//
	//  运行阶段中间管道提供的功能函数.
	//
	//=========================================================================

	/** \brief
	 *  使用中间管道渲染一个多边形,这将使用中间管道调用一次DIP函数,而不是
	 *  将多边形堆到一起到一定数量后再渲染.
	 */
	bool                render_polygon( const os_polygon* _poly );
	bool				render_andClearPolygonSet( os_polygonSet*  _polySet );

	/// 边渲染边清除，一遍完成，速度较快
	bool				render_andClearMeshSet( os_meshSet* meshSet );

	bool                render_meshData( int effectId, int verSize, const os_renderMeshData* mesh );


	/// 简单的设置3d设备状态函数.
	void                set_viewMatrix( osMatrix& _mat );
	void                set_projMatrix( osMatrix& _mat );
	void                set_worldMatrix( osMatrix& _mat );
	void                set_viewport( D3DVIEWPORT9& _vp );
	void                set_fvf( DWORD _fvf );

	//! 设置sampleState为reapeat状态，在渲染地表的时候，使用clamp状态，调用这个函数
	//! 设置为reapeat状态。
	void                set_ssRepeat( void );

	//! 设置为clamp状态，在渲染地图的时候，需要设置为clamp状态。
	void                set_ssClamp( void );

	/** \brief
	 *  3d 填充状态的设置使用这个函数,可以设置是线框模式或是实体模式.
	 *
	 *  \param _mode: 1:point 2是线框模式.3为实体模式.
	 */
	void                set_deviceFillMode( int _mode );


	//! 重设渲染设备的顶点缓冲区状态为空.
	void                set_nullStream( void );
	void                set_renderState( D3DRENDERSTATETYPE _State,DWORD _Value );
	void                get_renderState( D3DRENDERSTATETYPE _State,DWORD* _Value );
	void                set_material( D3DMATERIAL9& _mat );
	void                set_textureStageState( DWORD _Stage,D3DTEXTURESTAGESTATETYPE _Type,DWORD _Value );
	void                set_sampleState( DWORD _sample,D3DSAMPLERSTATETYPE _state,DWORD _value );
	void                set_dxLight( D3DLIGHT9* _dxL,int _num );
	void                set_dxLightDisable( int _num );


	/** \brief 
	 *  从中间管道得到一些渲染信息.
	 */
	int                 get_renderTriNum( void );
	int                 get_renderVerNum( void );
	int                 get_meshFaceNum( void );

	/** \brief 
	 *  每一帧都需要中间管道调用的设置函数,设置一些常用的参数.
	 */
	void              frame_setMiddlePipe( osc_camera* _cam = NULL );

	//! 从中间管道得到device.
	LPDIRECT3DDEVICE9  get_device( void ) { return m_pd3dDevice; } 

	//! 全局光的颜色值,从g_fAmbiR,g_fAmbiG,g_fAmbiB中得到
	static DWORD       m_dwAmbientColor;


	//@{   找回lost device用到的函数。
	/** \brief
	 *  reset MiddlePipe前需要释放的数据.
	 *
	 *  用于找回失去的设备，对付ctl alt del出现的问题
	 */
	void                mdpipe_onLostDevice( void );


	/** \brief 
	 *  Reset中间管道.当前deviceMgr Reset后,中间管道应该调用这个函数.
	 */
	bool                mdpipe_onResetDevice( void );
	//@} 



private:
	/** \brief
	 *  为上层创建dynamic Index buffer.
	 *
	 *  所有的Index为WORD类型,动态的顶点使用一个Index buffer.
	 *  Index data需要在上层处理,使需要在一起渲染的Index&Vertex
	 *  在缓冲区中也是集中在一起的.
	 *  向上层提供填充动态Indexbuf的函数.
	 *  中间管道只需要一个动态的Index buffer就可以了.
	 */
    bool              create_dynamicIB( void );

	/** \brief
	 *  释放dynamic index buffer.
	 *
	 *  reset 中间管道数据的时候需要调用这个函数.
	 */
	bool              release_dynamicIB( void );


	/** \brief
	 *  释放dynamic vertex buffer.
	 *
	 *  \param bool _release 参数如果是false,则当前释放是device lost的设备释放.
	 */
	bool              release_dynamicVB( bool _release = true );

	//! device lost后对动态的顶点缓冲区进行处理,重新创建这些顶点缓冲区
	bool              reset_dynamicVB( void );

	/** 为上层提供动态填充顶点缓冲区的函数
	 */
	//int                 fill_dynamicVB( VEC_polyptr& _polyptr, int _pnum,int _vernum );


	//! 录制中间管理每一帧需要设置的渲染状态块。
	void                record_frameSetRSB( void );


private:
	//!  中间管道管理引擎运行过程中需要的shader.
	osc_shaderMgr          m_shaderMgr;


	//!  中间管道可以对3d Device直接进行操作,3d设备指针.
	LPDIRECT3DDEVICE9      m_pd3dDevice;   


	osc_meshMgr           m_meshMgr;


	//!  中间管道包含了前端管道.
	osc_render            m_frontRender;


	//@{
	//! 中间管道管理的设备相关数据,包含Vertices&Indices buffer. 
	CSpaceMgr<os_verticesBuf>  m_vecVerBuf;
	//! 第零个IdxBuf为全局的动态索引缓冲区.其它的索引缓冲区都为静态
	CSpaceMgr<os_indicesBuf>   m_vecIdxBuf;
	//! 动态的索引缓冲区，用于渲染每一帧都改变的数据,指定m_vecIdxBuf第一个远素
	os_indicesBuf*        m_dynamicIdxBuf;

	int                  m_iTileVbIdx;
	CSpaceMgr<os_verticesBuf>  m_vecDynamicVB;
	//@}

	//@{
	//  中间管道渲染使用的渲染状态块。
	//! 每一帧需要设置的状态块
	int                  m_iFrameSetRsId;
};


extern osc_meshMgr*     g_ptrMeshMgr;

//! 设备重设时，阴影部分调入，使用的临界区
extern CRITICAL_SECTION    g_sShadowSec;


// 包含中间管道的inline 函数。
# include "middlePipe.inl"
