/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osAniMesh.h
 * 
 *  His:      River created @ 2004-5-12
 *
 *  Desc:     动画相关的mesh的数据结构，从大6的导出插件中得到数据。
 *  
 *  
 *  “取法其上，仅得其中，取法其中，仅得其下”
 *   
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "osShader.h"

# include "../../interface/osInterface.h"
# include "../../interface/miskFunc.h"
# include "../../tools/include/fadeMgr.h"
# include "osPolygon.h"


class     osc_middlePipe;


//! aniMesh中用到的vector.
typedef   std::vector<os_bsphere>       VEC_bsphere;
typedef   std::vector<os_objectVertex>  VEC_objectVertex;



//! 存储动画的颜色信息，相当于ARGB的DWORD值.
struct os_colorQuad
{
	unsigned char m_cAlpha;
	unsigned char m_cRed;
	unsigned char m_cGreen;
	unsigned char m_cBlue;
};



/** \brief
 *  存储了每一个mesh的材质，因为每一次渲染都需要改变材质，
 *  所以这个值有可能去掉。
 *
 *  渲染诸如果海浪的动画时，需要使用材质动画。
 */
struct os_material
{
	os_colorQuad m_dwDiffuse;
	os_colorQuad m_dwAmbient;
	os_colorQuad m_dwSpecular;
	os_colorQuad m_dwEmissive;
	float	     m_fShininess;

};


//! 一个面的对应的顶点index. 
struct os_refFace
{
	WORD  v_index[3];
};



//!　meshInstance的声明。
class     osc_aniMeshIns;


//! 材质对应的贴图文件名的长度。
#define FILENAME_LEN  64


/** \brief
 *  调用一次dip的结构。
 *
 *  每一个结点可能用到多个mesh,每一个MeshFace相当于一种材质，
 *  MeshFace内有对应的材质和纹理。
 */
struct os_aniMFace
{
	BYTE        m_cFlag;   //==0: normal,   ==1:blend
   os_material   m_Material;
   D3DMATERIAL9  m_deviceMat;
     
	char        m_szTexName[FILENAME_LEN];          //只包含文件名， 无路径
	int         m_nFace;
	os_refFace*   m_ptrFace;

	//@{
	//! 如果使用材质动画的话，此处有多个material 
	int          m_iMaterialNum;              
	os_material    *m_ptrMaterialList; 
	float         *m_ptrfTimekey;
	VEC_float      m_vecAlpha;
	//@} 

	//! 当前的材质是否是alpha相关的材质。
	bool           m_bAlphaMat;

public:
	os_aniMFace( void );
	~os_aniMFace( void );

	//! 从文件中调入aniMesh数据,读入数据，并移动数据指针。
	bool          load_aniMeshFromMem( BYTE*& _ds,bool _bAniMat );

	//! 得到当前material数据。
	void          get_currentMaterial( D3DMATERIAL9& _mat,osc_aniMeshIns* _ins );

	//! 释放当前的aniMFace.
	void          release_aniMFace( void );

};





/** \brief
 *  max中的AffineParts结构，用于构造一个完整的矩阵。
 *
 *  由于需要帧间插值，所以使用方便于插值的矩阵形式。
 *  每一个成员的函数见max sdk中AffineParts的定义。
 */
struct  os_AffineParts
{
	osVec3D   t;
	osQuat    q;
	osQuat    u;
	osVec3D   k;
	float     f;
};


struct   os_vbufFillData;

/** \brief
 *  对应max结点的数据结构。
 *  
 */
struct os_aniNode
{
	//! 是不是几何数据结点。
	bool         m_bIsGeoNode;

	// 
	//! 使用affineParts结构，使用动画结构更加清晰。
	int             m_iAffinePartsNum;
	osMatrix*        m_arrKeyMatrix;
	os_AffineParts*  m_arrAffineParts;
	float*          m_arrFAffTimeKey;


	// 
	//!  如果为 1， 没有变形动画， 如果>1, 有
	int                m_iMorphFrames;            
	int                m_iVertex;

	//! 普通的顶点数据。
	os_objectVertex*    m_arrVertex;  

	//! 如果使用点着角动画，使用这个变量放置数据。
	os_objectColVertex*  m_arrColVertex;

	//! 以秒为单位, 每一桢对应的时间
	float*            m_fTimeKey;   

	//! 如果是morphFrame,处理顶点熔合后填充的数组。
	VEC_objectVertex    m_vecAPVertex;


	//! 当前结点对应的mesh数据。
	int               m_iNumOfMesh;
	os_aniMFace*        m_arrMesh;


	//! 当前的结点是否全部为透明物品。创建完纹理后处理此值。
	bool              m_bTotalAlphaFace;
	//! 当前的结点中是否有透明物品。
	bool              m_bExistAlphaFace;


	//! the matrix to trans parent matrix to current matrix...
	osMatrix           m_matNodetm;   

	//! 当前ani Node第一帧的Bounding Box.
	os_aabbox            m_bbox;

	//! 当前ani node的Bounding Sphere.
	os_bsphere         m_bsphere;

	//@{
	// 以下为当前node用到的设备相关数据。
	/** \brief
	 *   aniNode用到的顶点缓冲区。
	 *   
	 *   此处为顶点缓冲区的id.由中间管道创建这个id.
	 */
	int               m_iVerBufId;

	/** \brief
	 *  当前buffer中，顶点的开始位置
	 *
	 *  在静态的顶点缓冲区中，永远为0,在动态的顶点缓冲区，每一次都会有变化。
	 */
	int               m_iStartVer;


	/** \brief
	 *  aniNode用到的index Buffer id.由中间管理创建这个index buffer.
	 *
	 *  ATTENTION TO OPP:可以使用一个index buffer来填充当前aniMesh内
	 *  所有的idx数据。
	 */
	VEC_int            m_vecIdxBufId;

	/** \brief
	 *  每一个nodeMesh用到的shaderId.
	 */
	VEC_int            m_vecShaderId;

	//@} 

	//! 渲染当前的node使用到的渲染多边形。
	os_polygon         m_sPolygon;


	//! 如果贴图有动画，是否需要在贴图动画uv坐标间插值。
	bool               m_bNotInterpolate;

	//! 当前结点数据是否使用点着色数据。
	bool               m_bColorVertex;

	/** \brief
	 *  当前动画结点完成动画需要的最大时间
	 *
	 *  包括affine Time 和morph Time两个时间序列.
	 */
	float              m_fPlayTotalTime;

	// River @ 2006-7-22: 加入关键帧矩阵
	bool               m_bUseKeyMat;

private:
	//! 填充当前aniNode渲染使用的多边形。
	void              fill_rPoly( void );

	//! 得到当前的poskey的位置。
	void              get_posKey( osMatrix& _mat,osc_aniMeshIns* _ins );

	//! 得到当前的rotkey数据。
	void              get_rotKey( osMatrix& _mat,osc_aniMeshIns* _ins );

	//! 得到当前的scaleKey数据。
	void              get_scaleKey( osMatrix& _mat,osc_aniMeshIns* _ins );

	//! 处理morph Frame,填充处理后的顶点。
	bool              process_morphFrame( osc_aniMeshIns* _ins );

	//! 对两个objectVertex进行插值。
	void              objver_lerp( os_objectVertex* _res,
		               os_objectVertex* _src,os_objectVertex* _dst,float _factor );


	/** \brief
	 *  释放当前的aniNode数据
	 */
	void              release_aniNode( void );

	//! 得到当前结点播放需要的时间。
	float             get_playNodeTime( void );

	//! 水面相关的状态处理
	void              waterObj_rsSet( osc_aniMeshIns* _ami,osMatrix& _worldMat );



public:
	os_aniNode( void );
	~os_aniNode( void );

	//! 从文件中调入aniNode数据,并偏移数据的指针。
	void       load_aniNodeFromMem( BYTE*& _ds,bool _bAniMat );



	//! 创建当前node的设备相关数据。
	bool       create_devDepData( osc_middlePipe* _pipe,
		         int _vbId,os_vbufFillData* _vbfill,int& _vbNum );


	//! 释放当前node的设备相关　数据。
	bool       release_devdepData( osc_middlePipe* _pipe );

	//  TEST CODE:
	//! 确认shaderId的可用性。
	bool       assert_devdepData( bool _assert = true );



	/** \brief
	 *  得到当前的Bounding Sphere,物品坐标，需要转化到世界空间。
	 */
	void       get_bsphere( os_bsphere& _bs );

	/** \brief
	 *  渲染当前的node,使用aniMeshInstance.
	 *
	 *  使用中间管道，polygon相关数据????
	 *  只需要使用一个polygon,渲染不同的mat时，需要更改idx buf id和shader id.
	 */
	bool       render_aniNode( osc_aniMeshIns* _ami,
		         int _idx,bool _alpha,float _faderate = 1.0f,bool _waterObj = false );

	bool       render_FogAniNode( osc_aniMeshIns* _ami,int _idx );

	/** \brief
	 *  简单的渲染函数，渲染aniMesh的头一帧到指定的位置和缩放。
	 * 
	 *  只能用于只有一个材质的简单模型，比如天空与远山。
	 *
	 *  \param float _alphaV 要渲染结点的透明度
	 */
	bool       render_aniNode( osc_middlePipe* _pipe,float _rotAgl,
		            osVec3D& _pos,osVec3D& _scale,float _alphaV );
	//@{
	// Windy mod @ 2005-8-26 12:25:48
	//Desc:Test Code 打印顶点，纹理等渲染信息，用于调试分析用.
	void		DumpRenderInfo(osc_middlePipe* _pipe);

	//@}
};



/** \brief
 *  root node struct of the scene...
 */
class os_rootNode
{
private:
	/** \brief
	 *  得到当前根结点内部总共顶点的数目。
	 *
	 *  每一个rootNode使用一个大的顶点缓冲区。
	 *  如果有结点的morphFrame数目大于1,则此Node
	 *  不使用大的顶点缓冲区
	 */
	int         get_totalVerNum( void );

public:
	//! 当前场景中几何结点的数目。
	int         m_iChildNum;

	//! 场景中几何结构的指针。
	os_aniNode*  m_arrNodelist;

	//! 当前AniMesh用到的顶点缓冲区的索引。
	int         m_iVBId;

public:
	os_rootNode( void )
	{
		m_arrNodelist = NULL;
		m_iVBId = -1;
	}

	~os_rootNode( void )
	{
		SAFE_DELETE_ARRAY( m_arrNodelist );
	}


	/** \brief
	 *  处理当前rootNode的设备相关数据。
	 *
	 *  填充根结点下每一个结点的顶点数据和索引数据。
	 *  
	 */
	bool        create_devdepData( osc_middlePipe* _pipe );


	//  ATTENTION : 释放一个结点时，先释放设备相关资源数据，再释放内存数据。
	//! 释放当前结点的释放相关数据。
	void        release_devdepData( osc_middlePipe* _pipe );

	//! 释放当前的结点.
	void        release_node( void );

};


/** \brief
 *  动画物品类，从ani文件中调入我们动画物品。
 *
 *  这个类需要存储设备相关的数据，由中间管道存储真正的
 *  设备相关数据。
 *  
 */ 
class osc_aniMesh : public os_refObj
{
	friend class osc_aniMeshIns;
private:
	//! 当前mesh对应的文件名。
	s_string         m_szFname;
	DWORD            m_dwNameHash;

	//! 当前mesh对应的数据。
	os_rootNode  m_sRootNode;

	//! 当前的ani Mesh是否用到了材质动画。
	bool         m_bAniMat;


	//! 当前的aniMesh的动画方式是否需要在两帧间进行插值。
	bool         m_bNotInterpolate;

	//! 当前的aniMesh是否使用了顶点着色动画,如果顶点着色，会使用另外的文件格式。
	bool         m_bColorVertex;


	/** \brief
	 *  当前的aniMesh和背景和渲染方式。
	 *
	 *  目前有6种渲染方式。
	 */
	BYTE         m_btRStyle;

	//! 文件导出时确认是否受环境光影响。
	bool         m_bEffByAmbient;

	//! 是否需要渲染重复纹理
	BOOL         m_bRepeatTexture;

	//! 是否使用关键帧矩阵
	BOOL         m_bKeyMatrix;

	//! 当前的mesh是否在使用。
	bool         m_bInuse;

	//! 是否可以置换出内存.
	bool         m_bCanSwap;

	//! 播放当前的aniMesh需要时间.
	float        m_fPlayTotalTime;

	//! 调入aniMesh后,计算当前Mesh的aabbox
	os_aabbox     m_sAabbox;

private:
	/** \brief
	 *  创建当前mesh的设备相关数据。
	 */
	bool        create_devdepRes( osc_middlePipe* _pipe );


	//! 从文件中调入我们的mesh数据。
	bool        load_meshFromfile( const char *_fname );

	//! 计算mesh的aabbox.
	void        cal_aabbox( void );

protected:
	/** \brief
	 *  释放当前资源的纯虚函数，由release_ref函数调用。
	 */
	virtual void release_obj( DWORD _ptr = NULL );


public:
	//! 当前mesh内node中使用morphFrame的最大值和他对应的aniNode.
	int          m_iMaxMorph;
	os_aniNode*   m_ptrMaxMorphNode;

	//! 当前mesh内使用affine动画数目的最大值和他对应的aniNode.
	int          m_iMaxAffine;
	os_aniNode*   m_ptrMaxAffineNode;


public:
	osc_aniMesh();


	//! 渲染当前的mesh,得到渲染当前的mesh需要数据。

	/** \brief
	 *  从文件中创建我们的mesh数据。
	 *  
	 *  调入数据的时候，创建设备相关的数据。
	 */
	bool          create_meshFromfile( os_aniMeshCreate& _mc );

	//! 渲染当前的aniMesh.
	void          render_aniMesh( osc_aniMeshIns* _ami,
		            bool _alpha,float _fadeRate = 1.0f,bool _waterObj = false ,bool _fogObj = false);

	/** \brief
	 *  渲染aniMesh的第一帧到指定的位置。
	 *
	 *  \param osVec3D _scale 要缩放aniMesh的数值。
	 *  \param float _alphaV 要渲染aniMesh的透明度 
	 */
	void          render_aniMesh( osVec3D& _pos,float _rotAgl,osVec3D& _scale,
		           osc_middlePipe* _pipe,float _alphaV = 1.0f );

	//! 当前的mesh是否在使用。
	bool          is_aniMeshInUse( void );



	//! 当前的mesh是否使用材质动画。
	bool          is_aniMeshAniMat( void );

	os_rootNode*   get_rootNodePtr( void ) { return &m_sRootNode; } 

	//! 得到当前mesh的名字。
	const char*    get_meshName( void )    { return m_szFname.c_str(); } 
	DWORD         get_nameHash( void )    { return m_dwNameHash;}

	//! 得到当前mesh正常播放需要的时间。
	float          get_playTotalTime( void ) { return this->m_fPlayTotalTime; }

	//! 得到当前mesh的aabbox.
	const os_aabbox*     get_aabboxPtr( void ) { return &m_sAabbox; }

	// 
	void           set_swap( bool _s )    { this->m_bCanSwap = _s; }
	bool           get_swap( void )       { return this->m_bCanSwap; } 

	// TEST CODE:
	//! 确认aniMesh对应的shaderid有效。
	bool           assert_shader( bool _assert = true );

	//! 得到此aniMesh对应的第一个effect，用于上层得到水面的effect.
	osc_effect*    get_effect( void );

};

typedef std::vector< osc_aniMesh >    VEC_aniMesh;



/** \brief
 *  渲染时使用的动画物品实例。
 *  
 *  动画相关数据。
 *  
 *  MeshIns使用lru准则。
 *  
 */ 
class osc_aniMeshIns : public os_createVal
{   
	//! 渲染node的时候，可以使用类点的数据。
	friend struct    os_aniNode;    
	friend struct    os_aniMFace;

	//! TEST CODE:
	friend class     osc_meshMgr;
private:
	//! 当前aniMesh对应的aniMesh指针。
	osc_aniMesh*     m_ptrMesh;

	/** \brief
	 *  当前meshIns渲染时使用到的全局光照。
	 *
	 *  动态物品的动态光也需要使用这个光照来模拟。
	 */
	DWORD           m_dwAmbientL;

	//! 当前物品是否受全局光的影响，默认影响。
	bool            m_bEffectByAL;


	//! 当前的aniMesh播放到当前过去的时间。
	float           m_fEleTime;

	//! 循正播放的次数。
	int             m_iLoopTimes;

	//! 当前的positionFrame，
	int             m_iCurPosFrame;
	float           m_fPTime;
	float           m_fPFactor;


	//! 当前的morphFrame.
	int             m_iCurMorphFrame;
	float           m_fMTime;
	float           m_fMFactor;

	//! 材质动画相关的.
	int             m_iCurMatFrame;
	float           m_fMatTime;
	float           m_fMatFactor;


	//@{
	//! 用于检测是否已经播放完一个物品动画。
	bool            m_bOverLoop;
	//@} 


	/** \brief
	 *  世界空间中meshIns的位置和旋转方向。
	 *  
	 *  这些数据需要和物品的动画数据配合使用。
	 */
	osVec3D         m_vec3Pos;
	float           m_fRotAgl;

	//! 当前动画的播放类型。
	ose_amPlayType   m_ePlayType;

	//! 如果是按时间播放，播放的时间。
	float           m_fPlayTime;

	//! 如果是按次播放，播放的次数。
	int             m_iPlayFrames;


	//! 当前的aniMeshIns是否在使用中。
	bool            m_bInuse;

	//! 当前的instance播放完成后是否自动使当前的instance无效。
	bool            m_bAutoDelete;

	//! 当前Mesh的缩放和旋转
	float           m_fSizeScale;

	//@{
	//! River mod 2008-7-16:加入缩放动画，多长时间内由当前的sizeScale基准缩放到一个新的基准
	float           m_fNewScale;
	float           m_fBaseScale;
	float           m_fScaleTime;
	float           m_fScaleEleTime;
	bool            m_bInScaleState;
	//@}

	/** \brief
	 *  当前aniMeshIns每一个结点的Bounding Box.
	 *
	 *  此处为上一帧的bounding sphere,判断可见性的时候也以上一帧的
	 *  Bounding sphere为基础,不会有太大的误差。
	 */
	VEC_bsphere     m_vecBs;
	//! 当前Ins的bsphere.
	os_bsphere      m_bsphere;


	//@{
	//  如果设置，则使用此矩阵。如果设置了位置，则使用位置来得到矩阵。
	//! 当前ins的转换矩阵。
	osMatrix        m_matTrans;
	//@} 


	//! 播放时速度时间压缩比，按次数播放的动画，使用此数据。
	float           m_fTimeScale;
	//@{
	//! 当前是否处于fade状态,0处于无fade状态,1 fadeIn.2 fadeOut.
	int            m_iFadeState;
	float          m_fFadeTime;
	float          m_fCurEleFadeTime;
	float          m_fCurFadeRate;

	//! river added @ 2009-10-10:用于让osa半透明
	float          m_fMinFade;
	//@}

	//! 当前是否隐藏.
	bool           m_bHide;

	//! top layer render
	bool           m_bTopLayer;
	//! River added @ 2010-1-20:
	//! 当前的ins是否已经渲染,用于人物身上的武器特效，
	//! 不会多个人物在界面上渲染的时候出错。
	bool           m_bRenderedInCurFrame;

	//! 是否是水面类的物品
	bool           m_bWaterAni;
	//! 是否是雾类的物品
	bool           m_bFogAni;
	int            m_FogShaderId;
	
	//! River @ 2010-6-29:屏幕光波效果用到.
	bool           m_bIsScreenWave;

private:
	//! 重设我们的meshIns
	void            reset_meshIns( void );

	//! 得到当前meshIns用到的世界旋转和位移矩阵。
	void            get_transMatrix( osMatrix& _mat );

	//@{ 
	//  繁杂的帧间插值运算，根据不同的功用，进行不同的处理。

	//! 处理posKey数据。
	void            process_posKey( os_aniNode* _ptrNode,float _etime );

	//! 处理morphFrame.
	void            process_morphFrame( os_aniNode* _ptrNode,float _etime );

	//! 处理matFrame
	void            process_aniMatFrame( os_aniNode* _ptrNode,float _etime );
	//@} 

	/** \brief
	 *  如果按次数播放的话，是否已经播放完
	 *
	 *  \return bool 如果返回true,则播放完成。
	 */ 
	bool            finish_playWithFrame( void );

	//! 如果按时间播放，是否已经播放完成。
	bool            finish_playWithTime( float _et );

	//! framemove当前的缩放状态
	void            scale_frameMove( void );
	//! frameMove当前osa的fade状态.
	void            fade_frameMove( void );

public:
	//! 中间管道的指针。
	static osc_middlePipe*  m_ptrMPipe;


public:
	osc_aniMeshIns();
	~osc_aniMeshIns();

	/** \brief
	 *  从文件中创建osc_aniMesh运行实例.
	 *
	 *  需要其它的参数，比如位置，旋转方向，播放类型等信息。
	 */
	WORD            create_aniMeshIns( osc_aniMesh* _mptr,os_aniMeshCreate& _mc,bool _topLayer = false );

	/** \brief
	 *   渲染当前的动态mesh实例。
	 */
	bool            render_aniMeshIns( float _etime,bool _alpha ,bool _depth=false);

	/** \brief
	 *  重新开始当前的动画。
	 */
	void            restart_aniMeshIns( void );

	/** \brief
	 *  开始当前osaAni的fade动画.
	 */
	void           start_fade( float _time,bool _fade = true,float _minFade = 0.0f );


	/** \brief
	 *  得到当前mesh Ins的Bonding sphere.
	 */
	os_bsphere*      get_bsphere( void );

	//! 得到os_bbox
	void            get_bbox( os_bbox* _box );

	//@{
	//! 设置和得到当前meshIns的光照系数。
	void            set_light( DWORD _light );
	DWORD           get_light( void );
	//@} 

	/** \brief
	 *  当前的aniMeshIns是否在使用中，
	 */
	bool            is_amInsInuse( void );


	//! 当前的meshIns渲染是不是使用和背景相加的渲染。
	int             get_RenderStyle( void );

	//! 当前的Mesh是否是点着色的mesh
	bool            is_colorVertexMesh( void );

	/** \brief
	 *  删除我们当前的meshIns.
	 */
	void            delete_curIns( void )      { 	reset_meshIns(); } 


	//! 设置aniMesh的位置。
	void            set_aniMeshPos( osVec3D& _pos );
	void			 set_aniMeshRot(float _RotAgl);
	//! 缩放当前的osa动画
	void             scale_aniMesh( float _rate,float _time );


	/** \brief
	 *  设置一个动画的世界矩阵。
	 *
	 *  这个接口主要用于一些特效的播放，需要在游戏层填充世界矩阵数据。
	 */
	void          set_aniMeshTrans( osMatrix& _transMat );



	//! 得到当前的实例数据索引到的mesh的ref数目。
	osc_aniMesh*     get_meshPtr( void );

	//! 设置osa动画的播放时间.
	void            set_osaPlayTime( float _timeScale );
	//! 得到osa动画的正常播放时间。
	float           get_osaPlayTime( void );

	//! 更新特效的动画信息
	bool            update_osaAnimation( float _etime );

	//! 设置和得到隐藏状态。
	void            set_hide( bool _hide ) { m_bHide = _hide; }
	bool            get_hide( void ) { return m_bHide; }

	//! 得到是否top Layer render.
	bool            get_topLayer( void ) { return this->m_bTopLayer; } 

	//! 是否是水面相关的动画.
	bool            is_waterAni( void ) { return this->m_bWaterAni; } 
	//! 是否是雾相关的动画.
	bool            is_FogAni( void ) { return this->m_bFogAni; } 



	// 特效编辑器相关的函数。　
# if __EFFECT_EDITOR__
	//! 是否停止当前的动画播放。
	bool            m_bStopPlay;
	/** \brief
	 *  停止当前动画的播放。
	 */
	void            stop_aniMeshIns( void );

	/** \brief
	 *  设置某一个动画相关的参数.
	 *
	 *  根据传入的参数，设置animationMesh内部的数据。
	 */
	bool            set_animationMesh( os_aniMeshCreate* _mc );

# endif 

};


//! 包含inline函数相关文件。
# include "osAniMesh.inl"

