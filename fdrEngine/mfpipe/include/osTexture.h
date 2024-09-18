//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osTexture.h
 *
 *   Desc:     O.S.O.K 引擎的纹理管理器,对中间管理开放.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  “在必要的时候，一定要想方设法说"不" ”。
 */
//--------------------------------------------------------------------------------------------------------
# pragma    once

# include "../../interface/osInterface.h"

//! 定义纹理名字的长度.
# define  TEXNAME_LENGTH   128


//! 纹理管理器中每二层贴图的最大数目。
# define  INIT_LMPVECSIZE    64
//! 纹理管理器的初始化
# define  INIT_TEXVECSIZE    768


/// osok引擎的纹理指针,可能使用三种纹理中的任一种.
union os_texPointer
{
	LPDIRECT3DTEXTURE9        commonTex;
	LPDIRECT3DCUBETEXTURE9    cubicTex;
	LPDIRECT3DVOLUMETEXTURE9  volumeTex;
};

class CVolumeFog;
/** \brief
 *  O.S.O.K引擎用到的纹理结构的封装.
 *
 *  对每一个纹理进行管理,可能不是普通纹理，是体积纹理或是环境纹理。
 *  纹理管理器只需要管理这些结构就可以了.
 */
class osc_texture : public os_refObj
{
	friend class osc_textureMgr;
	friend class osc_TGManager;
	friend class CVolumeFog;
	friend class osc_mapTerrain;
public:
	osc_texture();
	~osc_texture();

	/// 设置纹理数据
	void             setPixels( const unsigned char *pBuffer );

	/// 如:format = a8r8g8b8,则return 32
	int              get_colorBitCount( void );

	void             dumptex(char* fname){ D3DXSaveTextureToFile(fname,D3DXIFF_BMP,get_texpointer(),NULL); }


	/// 使用这个函数可以初始比如cubic map等特殊纹理。
	bool             init_texture( char* _filename, LPDIRECT3DDEVICE9 _device, int _tex_format );

	/// 释放纹理.
	bool             release_texture( void );

	//! 重设纹理
	bool             reset_texture( void );
	//! 找回纹理
	bool             restore_texture( LPDIRECT3DDEVICE9 _device );

	//! 得到这个纹理指针
	LPDIRECT3DBASETEXTURE9 get_texpointer( void );              
	const char*            get_texname( void ) { return m_strTexname; }
	DWORD                  get_texnameHash( void ){ return m_dwTexNameHash;}

	//! 只能得到Format,因为纹理初始化时已经决定了这个纹理的Format了.
	int                    get_texformat( void ){ return m_iTexFormat ; }

	//! 得到当前的tga图是否有Alpha通道。
	BOOL                   get_tgaAlpha( void ){ return m_bTgaAlpha; }

	//! River added @ 2009-6-10: 是否调入锁定,为多线程调入处理.
	bool                   m_bLoadLock;

protected:
	/// 释放当前资源的纯虚函数，由
	virtual void           release_obj( DWORD _ptr = NULL );


private:
	bool                   create_texture( const char *name,int w, int h, D3DFORMAT _format, LPDIRECT3DDEVICE9 _device, int _type =0 );

	os_texPointer       m_ptex;

	//!  当前纹理的名字.
	char                m_strTexname[TEXNAME_LENGTH];
	//! 当前纹理名字的hashVal.
	DWORD               m_dwTexNameHash;

	D3DXIMAGE_INFO      m_imageInfo;

	//!  当前只使用两种格式:a8r8g8b8,a4r4g4b4.
	D3DFORMAT           m_format;
	int					m_iWidth;
	int					m_iHeight;

	//!  0:common tex.1:cubic tex.2:volume tex.
	int                 m_iTexFormat;     

	/** 当前纹理对应的tga图是否有alpha通道。如果不是tga图，肯定没有。
	 *  使用我们自己的纹理格式，如果不是tga文件的纹理，此值为False.
	 */
	BOOL                m_bTgaAlpha;

	//! 是否是需要恢复的纹理,设备丢失之后的处理
	bool                m_bNeedRestore;

};


//-------------------------------------------------------------------------------


/** \brief
 *  场景的Texture管理类,管理了包含Lightmap的场景用到的所有的纹理.
 *  
 *  lightMap是用于场景中地表第二层的纹理，单独开出一个接口，是刚开始
 *  写地表的时候，为了跟以前的格式兼容，如果要修改的话，可以跟其它的纹理
 *  放在一起使用。
 *  
 *  TexManager中所有的纹理创建时，使用managed Texture,所以在设备
 *  lost的和reset时，不需要做任何的处理。
 */
class osc_textureMgr
{
public:
	osc_textureMgr();
	~osc_textureMgr();


	/// 初始化当前的纹理管理器.
	void                    init_textureMgr( LPDIRECT3DDEVICE9 _device );

	/** 释放Texture Manager.
	* \param _finalRelease 如果是最后一次释放，则不需要为texture再分配空间。
	*/
	void                    release_textureMgr( void );   

	/// 新创建一个纹理到管理器中
	int 					addnew_texture( const char *name,int w,int h,D3DFORMAT _format,int _type =0);

	/// 得到一个纹理通过ID
	osc_texture			*	get_textureById(int id);
    LPDIRECT3DBASETEXTURE9  get_texfromid( int _id );


	/** Dynamic add texture to texture manager.return the new add texture id.
	 */
	int                     dadd_textomanager( char* _filename, int _format );

	/// 对给定的纹理减少引用.
	int                     release_texture( int _id );

	/// 得到指定的纹理是否是有加了alpha通道的纹理
	bool                    is_alphaChannelTex( int _id );


	/// 如:format = a8r8g8b8,则return 32
	static int              get_colorBitCount( const D3DFORMAT& format );

	//! 重设textureMgr.
	void                    reset_textureMgr( void );

	//! 找回textureMgr.
	void                    restore_textureMgr( void );


private:
	LPDIRECT3DDEVICE9       m_pd3dDevice;     

	 /// dynamic load texture.
    CSpaceMgr<osc_texture>  m_vecDtexture;

	//! 创建纹理时，进入此Critical section.
	CRITICAL_SECTION        m_sCSGetFreeTexSpace;
};


//******************************************************************


/// 得到指定的纹理是否是有加了alpha通道的纹理。
inline bool osc_textureMgr::is_alphaChannelTex( int _id )
{
	return m_vecDtexture[_id].get_tgaAlpha();
}


inline osc_texture* osc_textureMgr::get_textureById( int id )
{
	guard;
	
	// River @ 2011-10-18:如果ID有问题，直接反应NULL.
	//osassertex( (id>=0)&&(id<m_vecDtexture.size()),va( "The tex id:<%d>..\n",id) );
	if( id < 0 )
		return NULL;
	if( id >= m_vecDtexture.size() )
		return	NULL;

	// River mod @ 2010-7-28:确保设备重设的时候，不出错。
	if( m_vecDtexture.validate_id( id ) )
		return &(m_vecDtexture[id]);
	else
		return NULL;

	unguard;
}

//----------------------------------------------------

/// 释放当前资源的纯虚函数，由
inline void osc_texture::release_obj( DWORD _ptr/* = NULL*/ )
{
	release_texture();
}

/// 如:format = a8r8g8b8,则return 32
inline int osc_texture::get_colorBitCount( void )
{
	return osc_textureMgr::get_colorBitCount( m_format );
}

