//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osShader.h
 *
 *   Desc:     O.S.O.K 引擎的shader管理器,对中间管理开放.这个管理直接管理了
 *             osc_textureMgr&osc_effectMgr这两个管理器.
 *
 *   His:      River Created @ 4/28 2003
 *
 *  “ Go VEGAN and EXERCISE like hell - it WILL make you SMARTER ”。
 */
//--------------------------------------------------------------------------------------------------------
# pragma   once

# include "../../interface/osInterface.h"

class osc_textureMgr;
class osc_effectMgr;
class osc_effect;


/** O.S.O.K引擎的Shader类.
 *
 *  O.S.O.K引擎中每一个面都必须对应一个Shader,一个shader
 *  对应了渲染这个面所需要的所有的渲染状态信息.这个类其实
 *  只是一个结构,把其它管理器调入的东西集合到了一起,供渲染
 *  三角形使用.
 */
class osc_shader : public os_refObj 
{
	friend class osc_shaderMgr;
public:
	osc_shader( void );

	void frame_move( void );

private:
	void set_name( const char* name );
	const osc_shader& operator = ( const osc_shader& sha );

	//! 当前 SHADER的名字.
	s_string     m_strShadername;
	DWORD        m_dwHash;

	//! Effect对应的effect管理器中的ID.
    int          m_iEffid;



	//! 动画矩阵,矩阵中的值会根据参数自动变化
	struct AnimMat
	{
		int			type;
		D3DXMATRIX	mat;
		float		angle;
		float       speed;
		osVec3D     center;

		//! time为总共过去的时间
		void anim( float time );
	};
	std::vector<AnimMat> m_vMatrix;

	//! 可带帧动画的纹理
	struct FrameTex
	{
		FrameTex( void ):curIndex(0),timing(0.f){}

		std::vector<int> texList;
		s_string         texDir;

		float            speed;

		//! time为上帧所用的时间
		void   anim( float time );
		int    get_curTexture( void )
		{
			return texList[curIndex]; 
		}

	private:
		int              curIndex;
		float            timing;
	};
	std::vector<FrameTex> m_vTexture;


	/** \brief
	 *  创建shader时使用的ShaderTemplate索引.
	 *
	 *  即使有相同的shader名，如果有不同Template索引，也需要创建不同的Shader.
	 */
	int          m_iTemplateIdx;

	float		 m_fEffectAlphaVal;


protected:
	virtual void release_obj( DWORD _ptr = NULL );
};


//! 初始化场景中场景中的shader数目
//  River @ 2010-4-7:修改为768,初始化更大的shader数目
# define INIT_SHADERVECSIZE       768

//! shader管理器可以最多注册的模板shader数目
# define MAX_REGISTERSHADER	      24


//! 注册一个Shader用到的数据结构
struct os_shaderRegister
{
	//! 此注册shader对应的effect文件名.
	char        m_strEffectName[256];

	//! 此注册shader对应的默认纹理目录名.
	char        m_strDefaultDir[256];

	/** \brief
	 *  调节的目录名，此处用于注册一个全局的缓冲区变量。
	 *  比如g_szMapName,可以使用纹理的目录名动态的改变。
	 */
	char        m_szAdjDirName[128];

public:
	os_shaderRegister(){ 
		m_strEffectName[0] = NULL;m_strDefaultDir[0] = NULL;m_szAdjDirName[0] = NULL; 
	}
};


/** \brief 
 *  osok引擎的shader管理器类,这个类调入了引擎运行过程中需要的所有的Shader.
 *
 * shader mgr不需要保存texture信息和effect信息.需要从texture   
 * mgr和effect mgr中得到渲染时所用的信息.shader mgr属于
 * resource mgr中的一员,属于内存级别的成员.texture&effect
 * mgr则是设备相关的实例,需要和中间管道相关联.
 *
 * 中间管道只需要和ShaderMgr打交道就可以了,再由shaderMgr和TexMgr&EffMgr打交道.
 *
 * ShaderManager的工作流程:
 * 中间管道一个个的创建shader.到中间管道flush资源的时候.
 * ShaderMgr开始自己的内部资源的创建:
 * 1: 先从shader队列中读入所有要创建的shader的信息.
 * 2: 创建shader相关联的所有的texture到Texture Manager.
 * 3: 创建shader相关联的所有的effect到Effect Manager.
 * 
 * 在引擎运行过程的动态加入shader和flush时的创建顺序相同.
 *
 * River @ 2004-7-17:
 * 应该可以完全的释放shaderMgr,以用于下一个完全不同的场景的调入，而不是
 * 象动态的调入场景一样，置换部分的资源。
 * shaderMgr管理了TextureMgr&EffMgr的释放和调入工作。
 */
class osc_shaderMgr
{
public:
	osc_shaderMgr( void );
	~osc_shaderMgr( void );

	/// 初始化我们的ShaderMgr,使用shaderMgr有可用的3d Device
	void             init_shaderMgr( LPDIRECT3DDEVICE9 _device );

	/** 释放shader manager的全部的资源,这个函数使用shader mgr变为可重入的全局模块
	 *  \param _finalRelease 是否最终释放shader管理器。
	 */
	void             release_shaderMgr( bool _finalRelease = true );

	/// reset ShaderMgr前需要释放的数据.
	void             shaderMgr_onLostDevice( void );

	/// 重新调入shader中所有的设备相关资源,上层Reset设备后需要调用这个函数
	void             shaderMgr_onResetDevice( LPDIRECT3DDEVICE9 _device );

	void             frame_move( void );


	//------------------------------------------------

	/** 动态的加入shader.
	 *
	 *  直到release manager的时候才release this shader.返回新加入的shader的id.
	 *  这是创建shader的主要的函数.
	 * 
	 *  \param  _shadername   要加入的Shader的名字.  
	 *  \param  _inifilename  要加入的shader所在的文件的名字.
	 *  \param  _defaultS     如果此值小于零,则必须找到相应的shadderIni文件才行.
	 *                        如果此值大于等于零,则从几个默认的shaderIni中查找出
	 *                        当前需要的shaderIni文件.
	 *
	 *   算法描述:
	 *   如果加入shader是调入阶段,则不创建,只缓存所有的数据.
	 *   如果是在引擎运行阶段,则随时创建所加入的Shader.
	 *   使用这个接口创建场景需要的所有的shader相关资源。
	 */
	int              add_shadertomanager( char* _shadername, char* _inifilename,int _defaultS = -1 );

	//! 在系统中注册shader模板
	int              register_shaderTemplate( const char* _effname,	const char* _defaultDirName,char* _adjDirName = NULL );

	/** 动态的释放一个shader占用的资源.
	 *  \param bool _frelease 是否完全的释放当前的shader,如果为真,则释放shader计数到0,
	 *                        然后释放当前的shader.
	 */
	int              release_shader( int _id,bool _frelease = false );

	//! 确认shaderId的可用性。
	bool			assert_shaderId( int _shaid,bool _assert = true );


	//! 通过xml文件和文件中的一个shader结构名创建一个shader
	int				add_shader( const char* _name, const char* _file ,const char* _defaultname = "default");



	//--------------------下面是对具体某一个shader的操作--------------------

	/// 得到某个shader对应的第一层纹理的tga文件是否有alpha通道
	bool                    has_alphaChannel( int& _shaid );

	/// 需要设置effect上需要的texture&other info
	bool                    set_EffectInfo( int _id );
	void                    update_shader( int _id );


	/// 通过shader id 得到它所用的 effect 的 id
	int				        get_effectId( int _shaid );
	osc_effect*             get_effect( int _shaid );


	//! 主要用于IndexedBlend的SkinMesh的渲染，需要直接得到texture
	//! 得到当前shader用到的第一张纹理的指针。
	LPDIRECT3DTEXTURE9      get_shaderTex( int _id );

	//! 得到当前shader的ref值。
	int                     get_shaderRef( int _id );



	//----------------------------------------
	int                     create_texture( char* _texFile, int _format = 0 );
	void                    release_texture( int _texId );
	LPDIRECT3DBASETEXTURE9  get_texturePtr( int _texId );


	//-----------------------------------------

	osc_effectMgr	*		get_effectMgr( void )	{ return m_effMgr; }
	osc_textureMgr	*		getTexMgr( void )		{ return m_texMgr;}

	//  River added @ 2006-8-24:用于彻底的替换旗帜的底图
	//! 设置一个Shader的某一层纹理
	void                    set_shaderTexture( int _shaderId,int _idx,int _texId );


private:
	/** 在引擎运行过程中往shaderMgr中加入shader.
	 *  需要立即创建shader数据和shader中包含的Texture&Effect数据.
	 */
	int                   dadd_shadertomanager( char* _shadername, char* _inifilename, int _defaultS );  

	bool                  create_shaderFromINI( int _shaId, char* _shaname, char* _ininame, int _ds );

	/// 从一个shader的名字得到一个默认的shader结构, 可以有多种的默认Shader
	bool                  creat_shaderFromTemplate( int _shaId, char* _shaname, int _dtype = 0 );

	//---------------------------------------

	bool                  set_effect( osc_shader* sha, const char* _effFile );
	bool                  set_texture( osc_shader* sha, int _idx, int frame, const char* _texFile, int _format );


	//! 通过xml文件和文件中的一个shader结构名创建一个shader
	int	                  _add_shader( const char* _name, const char* _file, const char* _realName );

	//! 通过现有的shader（_shaId）和第一层纹理（_tex1_file）创建一个新的shader（_name）
	int                   _add_shader( const char* _realName, int _shaId, const char* tex1_file );

	bool                  load_effect( TiXmlElement* shaElem, osc_shader* sha );
	bool                  load_texture( TiXmlElement* shaElem, osc_shader* sha, osc_effect* eff );
    bool                  load_matrix( TiXmlElement* shaElem, osc_shader* sha, osc_effect* eff );

	bool                  get_realShaderName( const char* _name, const char* _file, char* buf );
	int                   get_shader( const char* _name );


	osc_textureMgr  *     m_texMgr;
	osc_effectMgr   *     m_effMgr;
	CSpaceMgr<osc_shader> m_vecShader;
	unsigned char         m_vecIsFramed[INIT_SHADERVECSIZE];

	LPDIRECT3DDEVICE9     m_pd3dDevice;
	os_shaderRegister     m_arrShaderTemplate[MAX_REGISTERSHADER];


};


/** \brief
 *  在meshManager和characterManager中使用这个shaderManager,只能在中端管道以内
 *  使用这个全局变量,不太科学的用法 :-)
 */
extern osc_shaderMgr*    g_shaderMgr;


//! River @ 2009-6-29:在某一段时期的渲染内，强制使用某一个shader.
//! 由使用者确保几何体跟shader的fvf一致,例如，多次画水面的时候，
//！可能会使用复杂度完全不一样的shader.
extern BOOL   g_bForceUseCustomShader;
extern int    g_iForceCustomShaderIdx;



//================================================================================================================
//================================================================================================================
//
//   inline函数.
//
//================================================================================================================
//================================================================================================================

inline void osc_shader::set_name( const char* name )
{
	m_strShadername = name;
	m_dwHash = string_hash( m_strShadername );
}

//---------------------------------------------------------

//! 在系统中注册shader模板
inline int osc_shaderMgr::register_shaderTemplate( const char* _effname,
	            const char* _defaultDirName,char* _adjDirName/* = NULL*/ )
{
	for( int t_i=0;t_i<MAX_REGISTERSHADER;t_i ++ )
	{
		if( 0 == strlen( m_arrShaderTemplate[t_i].m_strEffectName )  )
		{
			strcpy( m_arrShaderTemplate[t_i].m_strDefaultDir,_defaultDirName );
			strcpy( m_arrShaderTemplate[t_i].m_strEffectName,_effname );
			
			if( _adjDirName )
			{
				osassert( strlen( _adjDirName ) < 128  );
				strcpy( m_arrShaderTemplate[t_i].m_szAdjDirName,_adjDirName );
			}

			return t_i;
		}
	}

	osassertex( false,"系统内预留的shaderTemplate空间不够..\n" );
	return -1;
}


/// 通过shader id 得到它所用的 effect 的 id
inline int osc_shaderMgr::get_effectId( int _shaid )
{
	osassert( m_vecShader.validate_id( _shaid ) );
	return m_vecShader[_shaid].m_iEffid;
}




inline void osc_shaderMgr::update_shader( int _id )
{
	if( !m_vecIsFramed[_id] )
	{
		m_vecShader[_id].frame_move();
		m_vecIsFramed[_id] = 1;
	}
}
