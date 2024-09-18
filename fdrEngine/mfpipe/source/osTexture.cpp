//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osTexture.cpp
 *
 *   Desc:     O.S.O.K 引擎的纹理管理器,对中间管理开放.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  “在必要的时候，一定要想方设法说"不" ”。
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osTexture.h"
# include "../../interface/miskFunc.h"
# include "../../exLib/SceneEx.h"


// 定义我们自己文件的扩展名. 
# define  MYTEX_EXTNAME      "hon"

//! River added @ 2009-5-4:用于上层得到当前引擎用到的纹理内存信息。
OSENGINE_API int                    g_iTextureSize = 0;

//! 是否dx管理纹理.默认为false,节约内存，但设备找回慢。
OSENGINE_API BOOL                   g_bManagedTexture = true;                  



//! 用于控制纹理的精细度.纹理的大小会除以这个值.
OSENGINE_API DWORD  g_dwTexDiv = 1; 

OSENGINE_API BOOL g_disableYY = FALSE; 


//! 全局是否使用纹理压缩.
OSENGINE_API BOOL   g_bUseDXTC = FALSE;

BOOL                g_bUse16Color = FALSE;




osc_texture::osc_texture():m_iWidth(0),m_iHeight(0)
{
	m_ptex.commonTex = NULL;
	memset( &m_imageInfo,0,sizeof( D3DXIMAGE_INFO ) );

	// ATTENTION:
	// 找出各种压缩方式的区别，在此处对所有的纹理使用压缩。
	if( g_bUseDXTC )
		m_format = D3DFMT_DXT2;
	else
	{
		if( g_bUse16Color )
			m_format = D3DFMT_A4R4G4B4;
		else
			m_format = D3DFMT_A8R8G8B8 ;
	}

	m_iTexFormat = 0;

	m_bTgaAlpha = FALSE;
	m_dwTexNameHash = 0;

	m_bLoadLock = false;

	m_bNeedRestore = false;

}

osc_texture::~osc_texture()
{
	release_texture();
}



//! River:此函数有出错的可能。
LPDIRECT3DBASETEXTURE9  osc_textureMgr::get_texfromid( int _id )
{
	guard;

	osassert( (_id<m_vecDtexture.size()) );
	if( _id == -1 )
		return NULL;

	// River: 兼容低端水面。
	if( !m_vecDtexture.validate_id( _id ) )
	{
		if( ::g_bWaterReflect )
		{
			// River @ 2010-9-13:不出错，返回NULL.让上层确保。
			//osassertex( false,va( "The tex is:<%d>...\n",_id ) );
			osDebugOut( "The tex is:<%d>...\n",_id );
			return NULL;
		}
		else
		{
			return NULL;
		}
	}


	return m_vecDtexture[_id].get_texpointer();

	unguard;
}



/** \brief
*  使用这个函数可以初始比如cubic map等特殊纹理。
*/
bool osc_texture::init_texture( char* _filename,LPDIRECT3DDEVICE9 _device,
									   int _tex_format )
{
	guard;

	int       t_fsize;
	DWORD     t_dwTexDiv = g_dwTexDiv;
	HRESULT   t_hr;
	BOOL	  t_avoidMipMap = FALSE;

	osassertex( t_dwTexDiv != 0 ,"全局的纹理缩放不能为零" );

	m_iTexFormat = 0;

	osassert( strlen( _filename ) < TEXNAME_LENGTH );
	strcpy( m_strTexname,_filename );
	m_dwTexNameHash = string_hash( _filename );
	m_iTexFormat = _tex_format;

	// 先把纹理读入到临时缓冲区,然后创建纹理.
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int    t_iOffset;

	//@{ snowwin 2006-1-4 为载入其他类型的纹理文件,必须是不经打包的，比如dds,tga.
	std::string tmpStr = m_strTexname;
	std::transform( tmpStr.begin(), tmpStr.end(), tmpStr.begin(), tolower );
	if( tmpStr.rfind( ".hon" ) == -1 )
	{
		//  River @ 2009-5-4:
		//! 通常地表纹理使用了dds纹理，不缩放
		if( tmpStr.rfind( ".dds" )!=-1 )
			t_dwTexDiv = 1;

		t_fsize = read_fileToBuf( (char*)_filename,t_ptrStart, TMP_BUFSIZE );
		t_iOffset = 0;
		m_bTgaAlpha = false;
	}
	else
		t_fsize = load_tgaFile( _filename,m_bTgaAlpha, t_ptrStart,TMP_BUFSIZE,t_iOffset,t_avoidMipMap);
	//@}

	//! river added @ 2009-5-14:统计全局用到的纹理内存。
	g_iTextureSize += t_fsize;

	t_ptrStart += t_iOffset;
	if( t_fsize == -1 )
	{
		MessageBox( NULL,_filename,"调入纹理出错",MB_OK );
		osassertex( false,va( "调入纹理<%s>时出错",_filename ) );
		m_bLoadLock = false;
		return false;
	}

	t_hr = D3DXGetImageInfoFromFileInMemory( t_ptrStart,t_fsize,&this->m_imageInfo );
	if( FAILED( t_hr ) )
	{
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		m_bLoadLock = false;
		return false;
	}

# if __TEXTURE_SIZELIMIT__
	if( (m_imageInfo.Width > 256)||(m_imageInfo.Height>256) )
		osassertex( false,va( "Texture <%s>'s size over 256 !!!\n",_filename ) );
# endif 

	// 让图像的格式 决定这个Texture格式（DDS）
	//
	m_format = m_imageInfo.Format;

	//! River @ 2010-4-8:
	D3DPOOL    t_sPool;
	if( g_bManagedTexture || (g_dwTexDiv==1) )
		t_sPool = D3DPOOL_MANAGED;
	else
		t_sPool = D3DPOOL_DEFAULT;

	switch( _tex_format )
	{
	case 0:
		{
			// Create texture from file's memory.
			t_hr = D3DXCreateTextureFromFileInMemoryEx( _device,
				t_ptrStart,t_fsize,
				m_imageInfo.Width/t_dwTexDiv,m_imageInfo.Height/t_dwTexDiv,
				t_avoidMipMap?1:D3DX_DEFAULT,
				0,m_format,
				t_sPool, 
				D3DX_DEFAULT ,
				D3DX_DEFAULT ,
				0, &m_imageInfo, NULL, &m_ptex.commonTex  );

			// 
			// River @ 2010-5-8:如果创建纹理失败，试试创建小一号的Managed纹理
			if( FAILED( t_hr ) )
			{
				t_dwTexDiv = 2;
				// Create texture from file's memory.
				t_hr = D3DXCreateTextureFromFileInMemoryEx( _device,
					t_ptrStart,t_fsize,
					m_imageInfo.Width/t_dwTexDiv,m_imageInfo.Height/t_dwTexDiv,
					t_avoidMipMap?1:D3DX_DEFAULT,
					0,m_format,
					D3DPOOL_MANAGED, 
					D3DX_DEFAULT ,
					D3DX_DEFAULT ,
					0, &m_imageInfo, NULL, &m_ptex.commonTex  );
				osassertex( !FAILED( t_hr ),
					va("Create Texture <%s> failed,Because<%s>",
					_filename,osn_mathFunc::get_errorStr( t_hr ) ) );
				m_bNeedRestore = false;
			}
			else 
				if( t_sPool == D3DPOOL_DEFAULT )
					m_bNeedRestore = true;

		}
		break;

	case 1:
		{
			//crearte texture from file.
			t_hr = D3DXCreateCubeTextureFromFileInMemory( _device,
				t_ptrStart,t_fsize,&m_ptex.cubicTex );

			m_bNeedRestore = false;
			osassertex( !FAILED( t_hr ),
				va( "Create cubic texture <%s> failed",_filename ) );
			break;

		}
	case 2: 
		{
			// create volume texture from file.
			t_hr = D3DXCreateVolumeTextureFromFileInMemory( _device,
				t_ptrStart,t_fsize,&m_ptex.volumeTex );

			if( FAILED( t_hr ) )
			{
				// River: 兼容低端水面。
				if( ::g_bWaterReflect )
				{
					osassertex( false,va( "Create volume texture <%s> failed",_filename ) );
				}
				else
				{
					m_ptex.volumeTex = NULL;
					osDebugOut( "创建体积纹理失败" );
				}
			}
			m_bNeedRestore = false;
	
			break;
		}
	default:
		osassertex( false,"不支持的格式..." );
	}

	END_USEGBUF( t_iGBufIdx );

	m_bLoadLock = false;

	return true;

	unguard;
}


bool osc_texture::create_texture( const char *name,int w, 
		int h, D3DFORMAT _format, LPDIRECT3DDEVICE9 _device, int _type /*=0*/ )
{
	guard;

	osassert( strlen( name ) < TEXNAME_LENGTH );
	strcpy( m_strTexname, name );
	m_dwTexNameHash = string_hash( name );

	HRESULT   t_hr;
	m_iTexFormat = _type;
	m_format = _format;
	m_iWidth = w;
	m_iHeight = h;

	m_bNeedRestore = false;

	switch( m_iTexFormat )
	{
	case 0:
		t_hr = _device->CreateTexture( w, h, 1, 0, 
			m_format, D3DPOOL_MANAGED, &m_ptex.commonTex, NULL );
		if( FAILED( t_hr ) )
		{
			osassertex( false,va("Create Texture <%s> failed",name ) );
			m_bLoadLock = false;
			return false;
		}
		break;

	case 1:
		t_hr = _device->CreateCubeTexture( w, 1, 0, 
			m_format, D3DPOOL_MANAGED, &m_ptex.cubicTex, NULL );
		if( FAILED( t_hr ) )
		{
			osDebugOut( "Create cubic texture <%s> failed",name );
			osassert( false );
			m_bLoadLock = false;
			return false;
		}
	// windy add 用于渲染到表面的纹理
	case 10:
		{
			t_hr = _device->CreateTexture( w, h, 1, 
				D3DUSAGE_RENDERTARGET, m_format, D3DPOOL_DEFAULT, &m_ptex.commonTex, NULL );
			if( FAILED( t_hr ) )
			{
				osDebugOut( "Create Texture <%s> failed",name );
				m_bLoadLock = false;
				return false;
			}
			break;
		}
	default:
		osassert(false);
	}

	m_bLoadLock = false;
	return true;

	unguard;
}

/// 释放纹理.
bool osc_texture::release_texture( void )
{
	m_bLoadLock = false;
	switch( m_iTexFormat )
	{
	case 10:
	case 0:
		if( m_ptex.commonTex )
			SAFE_RELEASE( m_ptex.commonTex );
		return true;
	case 1:
		if( m_ptex.cubicTex )
			SAFE_RELEASE( m_ptex.cubicTex );
		return true;
	case 2:
		if( m_ptex.volumeTex )
			SAFE_RELEASE( m_ptex.volumeTex );
		return true;
	default:
		osassert( false );
		return false;
	}

}

//! 重设纹理
bool osc_texture::reset_texture( void )
{
	guard;

	if( m_bNeedRestore )
		release_texture();
	
	return true;

	unguard;
}


//! 重新创建纹理
bool osc_texture::restore_texture( LPDIRECT3DDEVICE9 _device )
{
	guard;

	if( !m_bNeedRestore )
		return true;

	osassert( m_iTexFormat == 0 );

	DWORD     t_dwTexDiv = g_dwTexDiv;
	HRESULT   t_hr;
	int       t_fsize;
	BOOL	  t_avoidMipMap = FALSE;

	// 先把纹理读入到临时缓冲区,然后创建纹理.
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int    t_iOffset;

	//@{ snowwin 2006-1-4 为载入其他类型的纹理文件,必须是不经打包的，比如dds,tga.
	std::string tmpStr = m_strTexname;
	std::transform( tmpStr.begin(), tmpStr.end(), tmpStr.begin(), tolower );
	if( tmpStr.rfind( ".hon" ) == -1 )
	{
		//  River @ 2009-5-4:
		//! 通常地表纹理使用了dds纹理，不缩放
		if( tmpStr.rfind( ".dds" )!=-1 )
			t_dwTexDiv = 1;

		t_fsize = read_fileToBuf( m_strTexname,t_ptrStart, TMP_BUFSIZE );
		t_iOffset = 0;
		m_bTgaAlpha = false;
	}
	else
		t_fsize = load_tgaFile( m_strTexname,m_bTgaAlpha, t_ptrStart,TMP_BUFSIZE,t_iOffset ,t_avoidMipMap);
	//@}

	t_ptrStart += t_iOffset;
	if( t_fsize == -1 )
	{
		MessageBox( NULL,m_strTexname,"调入纹理出错",MB_OK );
		osassertex( false,va( "调入纹理<%s>时出错",m_strTexname ) );
		m_bLoadLock = false;
		return false;
	}

	t_hr = D3DXGetImageInfoFromFileInMemory( t_ptrStart,t_fsize,&this->m_imageInfo );
	if( FAILED( t_hr ) )
	{
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		m_bLoadLock = false;
		return false;
	}


	// 让图像的格式 决定这个Texture格式（DDS）
	//
	m_format = m_imageInfo.Format;

	// Create texture from file's memory.
	t_hr = D3DXCreateTextureFromFileInMemoryEx( _device,
		t_ptrStart,t_fsize,
		m_imageInfo.Width/t_dwTexDiv,m_imageInfo.Height/t_dwTexDiv,
		t_avoidMipMap?1:D3DX_DEFAULT,
		0/*D3DUSAGE_WRITEONLY*/,m_format,
		/*D3DPOOL_MANAGED*/D3DPOOL_DEFAULT, 
		D3DX_DEFAULT ,
		D3DX_DEFAULT ,
		0, &m_imageInfo, NULL, &m_ptex.commonTex  );

	END_USEGBUF( t_iGBufIdx );

	osassertex( !FAILED( t_hr ),
		va("Create Texture <%s> failed,Because<%s>",
		m_strTexname,osn_mathFunc::get_errorStr( t_hr ) ) );

	m_bLoadLock = false;

	return true;

	unguard;
}



//! 得到这个纹理指针
LPDIRECT3DBASETEXTURE9 osc_texture::get_texpointer( void )
{
	guard;

	// 确认纹理在使用中.
	osassertex( (0<get_ref()),m_strTexname );

	switch( this->m_iTexFormat )
	{
	case 10:
	case 0: 	return m_ptex.commonTex;
	case 1:		return m_ptex.cubicTex;
	case 2:		return m_ptex.volumeTex;
	default:
		osassert( false );
		return NULL;
	}

	unguard;
}

/// 设置纹理数据
void osc_texture::setPixels(const unsigned char *pBuffer)
{
	guard;

	osassertex( this->m_iTexFormat == 0, "目前只能对一般纹理进行此操作！" );

	int byte = get_colorBitCount()/8;
	int width = m_iWidth * byte;

	D3DLOCKED_RECT d3dlr;
	m_ptex.commonTex->LockRect( 0, &d3dlr, 0, 0 );

	unsigned char* pBuf = (unsigned char*)d3dlr.pBits;
	int pitch = d3dlr.Pitch;
	int ofsWidth = 0;
	for( int i = 0; i < m_iHeight; ++i )
	{
		memcpy( pBuf+ofsWidth, pBuffer+i*width, width );
		ofsWidth += pitch;
	}

	m_ptex.commonTex->UnlockRect( 0 );

	unguard;
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//  TextureManager数.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/// 初始化当前的纹理管理器.
void  osc_textureMgr::init_textureMgr( LPDIRECT3DDEVICE9 _device )
{
	m_pd3dDevice = _device;
	::InitializeCriticalSection( &this->m_sCSGetFreeTexSpace );
}

osc_textureMgr::osc_textureMgr()
{
	m_pd3dDevice = NULL;

	// 初始化当前的
	m_vecDtexture.resize( INIT_TEXVECSIZE );
}

osc_textureMgr::~osc_textureMgr()
{
	release_textureMgr();

	::DeleteCriticalSection( &this->m_sCSGetFreeTexSpace );
}


/** \brief 
 *  释放Texture Manager.
 */
void osc_textureMgr::release_textureMgr( void )
{
	//
	// 在上层已经对所有的纹理释放完成
}

//! 重设textureMgr.
void osc_textureMgr::reset_textureMgr( void )
{
	guard;

	// River @ 2010-9-4:解决从游戏场景内返回login场景后，Ctl+Alt即设备找回出错的bug.
	if( g_bManagedTexture || (g_dwTexDiv==1) )
		return ;

	CSpaceMgr<osc_texture>::NODE  t_ptrNode;

	for( t_ptrNode = m_vecDtexture.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDtexture.next_validNode( &t_ptrNode ) )
	{
		if( (t_ptrNode.p->get_ref() == 0)||(t_ptrNode.p->m_ptex.commonTex == NULL) )
			continue;
		t_ptrNode.p->reset_texture();
	}

	unguard;
}

//! 找回textureMgr.
void osc_textureMgr::restore_textureMgr( void )
{
	guard;

	// River @ 2010-9-4:解决从游戏场景内返回login场景后，Ctl+Alt即设备找回出错的bug.
	if( g_bManagedTexture || (g_dwTexDiv==1) )
		return ;

	CSpaceMgr<osc_texture>::NODE  t_ptrNode;

	for( t_ptrNode = m_vecDtexture.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDtexture.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p->get_ref() == 0 )
			continue;
		t_ptrNode.p->restore_texture( m_pd3dDevice );
	}

	return;

	unguard;
}




/** \brief 
*  Dynamic add texture to texture manager.return the new add texture id.
*/
int osc_textureMgr::dadd_textomanager( char* _filename,int _format )
{
	guard;

	int        t_idx;
	bool       t_b;

	DWORD      t_dwHash = string_hash( _filename );

	// 如果要加入的纹理已经创建，则不需要再次加入：
	// 但需要为当前已经创建的纹理加入一个引用。
	CSpaceMgr<osc_texture>::NODE  t_ptrNode;

	// River @ 2006-3-2: 此处需要加一个Critical Section,两个线程可能在此出错.
	::EnterCriticalSection( &this->m_sCSGetFreeTexSpace );
	for( t_ptrNode = m_vecDtexture.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDtexture.next_validNode( &t_ptrNode ) )
	{
		if( (t_ptrNode.p->get_ref() == 0)||(t_ptrNode.p->m_ptex.commonTex == NULL) )
		{
			//! River added @ 2011-1-5:可能引起设备找回的错误，此处修改
			//! 人物调入线程工作的时候，可能设备reset,此时会出现这种情况。
			if( t_ptrNode.p->m_iTexFormat == 10 ) 
				continue;

			if( !m_vecDtexture[t_ptrNode.idx].m_bLoadLock )
			{
				DWORD   t_dw = 0;
				if( 0 == t_ptrNode.p->get_ref() )
					t_dw |= 1;
				if( NULL == t_ptrNode.p->m_ptex.commonTex )
					t_dw |= 1<<1;
				if( !m_vecDtexture[t_ptrNode.idx].m_bLoadLock )
					t_dw |= 1<<2;

				//! River @ 2009-8-29:在极限的情况下，此段代码会非法执行
				//! 情况描述见另一个同样的释放函数。
				if( !t_ptrNode.p->m_ptex.commonTex )
				{
					osDebugOut( "Unsafe1 ReleaseTexture<%d>...\n",t_ptrNode.idx );
					m_vecDtexture.release_node( t_ptrNode.idx );
				}
			}
			continue;
		}

		osassert( t_ptrNode.p->get_texnameHash() != 0 );
		if( t_ptrNode.p->get_texnameHash() == t_dwHash )
		{
			osassert( 0 == strcmp( t_ptrNode.p->get_texname(),_filename) );
			t_ptrNode.p->add_ref();
			::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );

			return t_ptrNode.idx;
		}
	}
	t_idx = m_vecDtexture.get_freeNode();
	m_vecDtexture[t_idx].add_ref();

	osassert( m_vecDtexture[t_idx].get_ref() > 0 );
	osassert( m_vecDtexture.validate_id( t_idx ) );

	m_vecDtexture[t_idx].m_bLoadLock = true;

	// 
	//　在此处退出cs,可能其它地方会释放这个cs.
	::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );
	t_b = m_vecDtexture[t_idx].init_texture( _filename,m_pd3dDevice,_format );
	//osDebugOut( "dadd_textomanager:Texture Create<%d><%s>..\n",t_idx,_filename );
	osassertex( t_b,va( "创建纹理<%s>失败...\n",_filename ) );
	osassert( m_vecDtexture.validate_id( t_idx ) );

	return t_idx;

	unguard;
}



/** 新创建一个纹理到管理器中
@remarks
*/
int osc_textureMgr::addnew_texture( const char *name,int w,int h,D3DFORMAT _format,int _type/* =0*/)
{
	guard;

	int        t_idx;
	bool       t_b;

	DWORD      t_dwHash = string_hash( name );

	// River @ 2006-3-2:
	// 加入这个Critical Section,不然地图多线程调入时，此处可能释放另一个线程中
	// dadd_textomanager函数内刚刚得到要使用的texture
	::EnterCriticalSection( &this->m_sCSGetFreeTexSpace );
	CSpaceMgr<osc_texture>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecDtexture.begin_usedNode(); 
		t_ptrNode.p != NULL; t_ptrNode = m_vecDtexture.next_validNode( &t_ptrNode ) )
	{
		// River mod @ 2006-2-18:容错，可能其它的地方释放掉纹理后，没有在spaceMgr内释放
		if( (0 == t_ptrNode.p->get_ref()) ||( NULL == t_ptrNode.p->m_ptex.commonTex ) )
		{
			//! River added @ 2011-1-5:可能引起设备找回的错误，此处修改
			if( t_ptrNode.p->m_iTexFormat == 10 ) 
				continue;

			if( !m_vecDtexture[t_ptrNode.idx].m_bLoadLock )
			{
				DWORD   t_dw = 0;
				if( 0 == t_ptrNode.p->get_ref() )
					t_dw |= 1;
				if( NULL == t_ptrNode.p->m_ptex.commonTex )
					t_dw |= 1<<1;
				if( !m_vecDtexture[t_ptrNode.idx].m_bLoadLock )
					t_dw |= 1<<2;

				//! River @ 2009-8-29:在极限的情况下，此段代码会非法执行
				/*  情况描述：
                    1: 另一线程dadd_textomanager执行到	::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );
	                   还没有执行：
					   t_b = m_vecDtexture[t_idx].init_texture( _filename,m_pd3dDevice,_format );
				    2: 当前函数执行：
					   if( (0 == t_ptrNode.p->get_ref()) ||( NULL == t_ptrNode.p->m_ptex.commonTex ) )
				    3: 另一线程执行完：
					   t_b = m_vecDtexture[t_idx].init_texture( _filename,m_pd3dDevice,_format );
				    4: 当前线程开始执行：
					   if( !m_vecDtexture[t_ptrNode.idx].m_bLoadLock )
				    修改方式：加入新代码：if( t_ptrNode.p->m_ptex.commonTex )
					以观后效
				*/
				if( !t_ptrNode.p->m_ptex.commonTex )
				{
					osDebugOut( "Unsafe2 ReleaseTexture<%d>...\n",t_ptrNode.idx );
					m_vecDtexture.release_node( t_ptrNode.idx );				
				}
			}
			continue;
		}

		osassert( t_ptrNode.p->get_texnameHash() != 0 );

		if( t_ptrNode.p->get_texnameHash() == t_dwHash )
		{
			osassert( 0 == strcmp( t_ptrNode.p->get_texname(), name) );
			t_ptrNode.p->add_ref();
			::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );

			return t_ptrNode.idx;
		}
	}
	t_idx = m_vecDtexture.get_freeNode();
	m_vecDtexture[t_idx].add_ref();
	osassert( m_vecDtexture[t_idx].get_ref() > 0 );

	m_vecDtexture[t_idx].m_bLoadLock = true;

	::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );

	osassert( m_vecDtexture.validate_id( t_idx ) );
	t_b = m_vecDtexture[t_idx].create_texture( name, w, h, _format, m_pd3dDevice, _type );
	//osDebugOut( "addnew_texture:Texture Create<%d><%s>..\n",t_idx,name );
	if( _type != 10 )
	{
		osassertex( m_vecDtexture.validate_id( t_idx ),va( "The idx is<%d>...\n",t_idx ) );
		osassertex( t_b,va( "创建纹理<%s>失败...\n",name ) );
	}

	// River mod 2007-9-10:如果是创建环境纹理失败,则返回-1ID
	if( _type == 10 )
	{
		if( !t_b )
		{	//! 如果还是创建不出来，就创建一个小纹理
			t_b = m_vecDtexture[t_idx].create_texture( name, 1, 1, _format, m_pd3dDevice, _type );
			return -1;
		}
	}

	return t_idx;

	unguard;
}


/** 对给定的纹理减少引用.
 *  如果引用计数变为零,则纹理元素会自动释放自己的数据.
 */
int osc_textureMgr::release_texture( int _id )
{
	if( _id < 0 )return -1;

	::EnterCriticalSection( &this->m_sCSGetFreeTexSpace );

	//@{
	// River added @ 2009-5-26:暂时的解决代码,人物换装为零会出错
	if( !m_vecDtexture.validate_id( _id ) )
	{
		::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );
		return 0;
	}
	//@} 

	if( m_vecDtexture[_id].get_ref() > 0 )
	{
		int t_ref = m_vecDtexture[_id].release_ref();
		if( t_ref == 0 )
		{
			if( !m_vecDtexture[_id].m_bLoadLock )
				m_vecDtexture.release_node( _id );				
		}
		::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );

		return t_ref;
	}
	else
	{
		::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );

		return 0;
	}
}


/// 如:format = a8r8g8b8,则return 32
int osc_textureMgr::get_colorBitCount( const D3DFORMAT& format )
{
	switch( format )
	{
	case D3DFMT_R8G8B8		:	return 24;
	case D3DFMT_A8R8G8B8	:	return 32;
	case D3DFMT_X8R8G8B8	:	return 32;
	case D3DFMT_R5G6B5		:	return 16;
	case D3DFMT_X1R5G5B5	:	return 16;
	case D3DFMT_A1R5G5B5	:	return 16;
	case D3DFMT_A4R4G4B4	:	return 16;
	case D3DFMT_A8			:	return 8;
	case D3DFMT_R3G3B2		:	return 8;
	case D3DFMT_A8R3G3B2	:	return 16;
	case D3DFMT_X4R4G4B4	:	return 16;
	case D3DFMT_A2B10G10R10	:	return 32;
	case D3DFMT_G16R16		:	return 32;
	default					:	return 0;
	}
	return 0;
}

