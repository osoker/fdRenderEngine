//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osEffect.h
 *
 *   Desc:     O.S.O.K 引擎的Effect管理器,对中间管理开放.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  微软文化: “我知道这件事看来是做不到的了.但如果他是可以做到的，那
 *    么你会怎么去做呢？”。
 */
//--------------------------------------------------------------------------------------------------------
# pragma  once

# include "../../interface/osInterface.h"


# define    TEXTUREPTR               "Texture"


/** \brief
 *  D3DX EFFECT的包装类,所有的Effect通过这个类进行操作.
 *
 *  fx文件中的Texture和matrix变量必须统一命名：Texture0、Texture1... Matrix0、Matrix1...
 */
class osc_effect : public os_refObj
{
	friend class osc_effectMgr;
	friend class osc_shaderMgr;

public:
	osc_effect( void );
	~osc_effect( void );


	/// 设置当前Effect用到的第n个texture
	bool            set_texture( int _idx,LPDIRECT3DBASETEXTURE9 _tex );
	bool			set_textureById( int _idx, int _texId );
	int				get_texCount( void ){ return (int)m_vTexHandle.size(); }

	/// 设置当前Effect用到的第n个matrix
	void			set_matrix( int _idx, const D3DXMATRIX& _mat );
	int				get_matCount( void ){ return (int)m_vMatHandle.size(); }
	int             get_float2Count( void ) { return (int)m_vFloat2Handle.size(); } 


	//! 设置foat,float3,float4.
	void            set_float( const char* _name,float _val );
	void            set_value( const char* _name,void* _vec4,int _size );
	void            set_float2( int _idx,float* _ft );

	/// 返回要使用的technique的pass的个数
	unsigned int    begin( void );
	void            end();
	/// 返回此pass是否是virtual型的
	bool            pass( unsigned int index );
	//! end pass.
	bool            endPass( void );

	//! 更新beginpass和endpass之间的渲染状态修改
	void            commit_change( void );


	BOOL            is_forceTrans( void ) { return m_bForceTransparency; }


	//!  当前Effect的指针.
	LPD3DXEFFECT    m_peffect;
	unsigned int	m_curTechLevel;
private:
	/// 从一个文件和设备指针初始化effect,只能在osc_effectMgr中使用
	bool            init_effect( char* _filename, LPDIRECT3DDEVICE9 _device ); 

	/// 释放一个Effect,只能在osc_effectMgr中使用
	void            release_effect( void );

	/// 释放当前资源的纯虚函数
	virtual void    release_obj( DWORD _ptr = NULL );

	bool            create_effect( char* _filename, LPDIRECT3DDEVICE9 _device );

	//!  当前Effect相应的Effect文件名.
	s_string		m_strEffFileName;
	DWORD			m_dwHash;

	//  River mod @ 2007-4-9:
	//! 是否强制此Effect对应的shader做为透明纹理渲染.
	BOOL            m_bForceTransparency;

	//! 替换当前要编译的hlslShader版本到当前机器支持的最新shader版本
	bool            update_toNewVS( BYTE* _data, LPDIRECT3DDEVICE9 _device,int _size );

	//! 存放texture的handle，设置纹理时只对相应handle设置就可以了，不用通过string名称
	std::vector<D3DXHANDLE> m_vTexHandle;
	std::vector<D3DXHANDLE> m_vMatHandle;

	//  River @ 2010-4-3:优化。
	//! 存放texture scale的handle,不需要使用整个matrix,使用一个float2就可以。
	std::vector<D3DXHANDLE> m_vFloat2Handle;

	struct Pass
	{
		Pass( void ):handle(0),isVirtual(true){}

		D3DXHANDLE handle;
		bool       isVirtual;
	};
	struct Technique
	{
		Technique( void ):handle(0){ m_vPassList.clear(); }

		D3DXHANDLE        handle;
		std::vector<Pass> m_vPassList;
	};

	std::vector<Technique> m_vTecList;

	/// 用静态的变量，所有的effect的pass flag 和 technique都相同
	static DWORD           m_passFlag;
	static unsigned int    m_techniqueLevel;

public:
	//! 得到effect的Name.
	const char*     get_effectName() { return this->m_strEffFileName.c_str(); } 

};

//----------------------------------------------------------------

/** \brief
 *  D3DX EFFECT的管理器类,所有的Effect都被放进了这个管理器.
 *
 *  EffectManager管理了所有的Shader中的Effect,多个Shader可以对应同一个Effect,只需要纹理不同
 */
class osc_effectMgr
{
public:
	osc_effectMgr();

	/// 初始化当前的effectMgr
	bool              init_effectMgr( LPDIRECT3DDEVICE9 _device );

	/** 把所有的effect file全部释放掉
	 *  \param _finalRelease 如果不是最终释放，还需要为下次使用分配内存
	 */
	bool              release_effectMgr( bool _finalRelease = true );  


	/// reset EffectMgr前需要释放的数据.
	void              eff_onLostDevice( void );
	/// Reset所有的设备相关的Effect.
	bool              eff_onResetDevice( void );

	/// 从effect manager中得到Effect的指针.
	osc_effect*       get_effectfromid( int _id );


	/** 在引擎运行过程中动态的加入Effect
	 *  \return  返回新加入的Effect的id,如果返回-1,出错
	 */
	int               dadd_effecttomanager( char* _filename );

	/// 减少一个effect文件的引用.
	int               release_effect( int _id );

	static void       set_passFlag( DWORD flag ){ osc_effect::m_passFlag = flag; }
	static void       set_techniqueLevel( unsigned int level ){ osc_effect::m_techniqueLevel = level; }

private:
	LPDIRECT3DDEVICE9    m_pd3dDevice;

	/** 如果要动态的加入Effect,可以在这个effect Vector中动态的加入.
	 *  但不可以动态的减少Effect,如果减少的话,需要的变动太大.
	 */
	CSpaceMgr<osc_effect>  m_arrEffectArray;
};


//==========================================================================




/// 减少一个effect文件的引用
inline int osc_effectMgr::release_effect( int _id )
{
	/*
	// River @ 2011-2-24:整个游戏内Effect所用的内存有限，无需释放。
	int t_iRef = 0;
	osassert( m_arrEffectArray.validate_id( _id ) );
	osassert( m_arrEffectArray[_id].get_ref()>0 );

	t_iRef = m_arrEffectArray[_id].release_ref();
	if( 0 == t_iRef )
		m_arrEffectArray.release_node( _id );
	*/

	return 1;
}


/// 释放当前资源的纯虚函数
inline void osc_effect::release_obj( DWORD _ptr/* = NULL*/ )
{
	//osDebugOut( "The <%s> effect release from Device..\n",m_strEffFileName.c_str() );
	release_effect();
}

inline void osc_effect::set_matrix( int _idx, const D3DXMATRIX& _mat )
{
	osassert( _idx < get_matCount() );
	m_peffect->SetMatrix( m_vMatHandle[_idx], &_mat );
}

inline void osc_effect::set_float2( int _idx,float* _ft )
{
	osassert ( _idx < get_float2Count() );
	m_peffect->SetFloatArray( m_vFloat2Handle[_idx],_ft,2 );
}

//! 设置foat,float3,float4.
inline void osc_effect::set_float( const char* _name,float _val )
{
	osassert( m_peffect );
	m_peffect->SetFloat( _name,_val );		
}

inline void osc_effect::set_value( const char* _name,void* _vec4,int _size )
{
	osassert( m_peffect );
	HRESULT hr = m_peffect->SetValue( _name,_vec4,_size );
	if(FAILED(hr))
	{
		osassertex( false, va("设置Effect出错,原因:%s(文件[%s]字段[%s])\n",
			DXGetErrorDescription9(hr), m_strEffFileName.c_str(),_name)    );
	}
}



/// 返回要使用的technique的pass的个数
inline unsigned int osc_effect::begin( void )
{
	osassert( m_peffect );
	osassertex( m_curTechLevel < m_vTecList.size(),
		va( "Tech level:<%d>,tec list size<%d>..\n",m_curTechLevel,m_vTecList.size() ) );
	m_peffect->SetTechnique( m_vTecList[m_curTechLevel].handle );
	UINT t;
	HRESULT   hr;
	hr = m_peffect->Begin( &t, this->m_passFlag );
	osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

	return t;
}
inline void osc_effect::end()
{
	m_peffect->End();
}

