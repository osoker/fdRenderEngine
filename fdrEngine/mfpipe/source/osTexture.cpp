//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osTexture.cpp
 *
 *   Desc:     O.S.O.K ��������������,���м������.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  ���ڱ�Ҫ��ʱ��һ��Ҫ�뷽�跨˵"��" ����
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osTexture.h"
# include "../../interface/miskFunc.h"
# include "../../exLib/SceneEx.h"


// ���������Լ��ļ�����չ��. 
# define  MYTEX_EXTNAME      "hon"

//! River added @ 2009-5-4:�����ϲ�õ���ǰ�����õ��������ڴ���Ϣ��
OSENGINE_API int                    g_iTextureSize = 0;

//! �Ƿ�dx��������.Ĭ��Ϊfalse,��Լ�ڴ棬���豸�һ�����
OSENGINE_API BOOL                   g_bManagedTexture = true;                  



//! ���ڿ�������ľ�ϸ��.����Ĵ�С��������ֵ.
OSENGINE_API DWORD  g_dwTexDiv = 1; 

OSENGINE_API BOOL g_disableYY = FALSE; 


//! ȫ���Ƿ�ʹ������ѹ��.
OSENGINE_API BOOL   g_bUseDXTC = FALSE;

BOOL                g_bUse16Color = FALSE;




osc_texture::osc_texture():m_iWidth(0),m_iHeight(0)
{
	m_ptex.commonTex = NULL;
	memset( &m_imageInfo,0,sizeof( D3DXIMAGE_INFO ) );

	// ATTENTION:
	// �ҳ�����ѹ����ʽ�������ڴ˴������е�����ʹ��ѹ����
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



//! River:�˺����г���Ŀ��ܡ�
LPDIRECT3DBASETEXTURE9  osc_textureMgr::get_texfromid( int _id )
{
	guard;

	osassert( (_id<m_vecDtexture.size()) );
	if( _id == -1 )
		return NULL;

	// River: ���ݵͶ�ˮ�档
	if( !m_vecDtexture.validate_id( _id ) )
	{
		if( ::g_bWaterReflect )
		{
			// River @ 2010-9-13:����������NULL.���ϲ�ȷ����
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
*  ʹ������������Գ�ʼ����cubic map����������
*/
bool osc_texture::init_texture( char* _filename,LPDIRECT3DDEVICE9 _device,
									   int _tex_format )
{
	guard;

	int       t_fsize;
	DWORD     t_dwTexDiv = g_dwTexDiv;
	HRESULT   t_hr;
	BOOL	  t_avoidMipMap = FALSE;

	osassertex( t_dwTexDiv != 0 ,"ȫ�ֵ��������Ų���Ϊ��" );

	m_iTexFormat = 0;

	osassert( strlen( _filename ) < TEXNAME_LENGTH );
	strcpy( m_strTexname,_filename );
	m_dwTexNameHash = string_hash( _filename );
	m_iTexFormat = _tex_format;

	// �Ȱ�������뵽��ʱ������,Ȼ�󴴽�����.
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int    t_iOffset;

	//@{ snowwin 2006-1-4 Ϊ�����������͵������ļ�,�����ǲ�������ģ�����dds,tga.
	std::string tmpStr = m_strTexname;
	std::transform( tmpStr.begin(), tmpStr.end(), tmpStr.begin(), tolower );
	if( tmpStr.rfind( ".hon" ) == -1 )
	{
		//  River @ 2009-5-4:
		//! ͨ���ر�����ʹ����dds����������
		if( tmpStr.rfind( ".dds" )!=-1 )
			t_dwTexDiv = 1;

		t_fsize = read_fileToBuf( (char*)_filename,t_ptrStart, TMP_BUFSIZE );
		t_iOffset = 0;
		m_bTgaAlpha = false;
	}
	else
		t_fsize = load_tgaFile( _filename,m_bTgaAlpha, t_ptrStart,TMP_BUFSIZE,t_iOffset,t_avoidMipMap);
	//@}

	//! river added @ 2009-5-14:ͳ��ȫ���õ��������ڴ档
	g_iTextureSize += t_fsize;

	t_ptrStart += t_iOffset;
	if( t_fsize == -1 )
	{
		MessageBox( NULL,_filename,"�����������",MB_OK );
		osassertex( false,va( "��������<%s>ʱ����",_filename ) );
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

	// ��ͼ��ĸ�ʽ �������Texture��ʽ��DDS��
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
			// River @ 2010-5-8:�����������ʧ�ܣ����Դ���Сһ�ŵ�Managed����
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
				// River: ���ݵͶ�ˮ�档
				if( ::g_bWaterReflect )
				{
					osassertex( false,va( "Create volume texture <%s> failed",_filename ) );
				}
				else
				{
					m_ptex.volumeTex = NULL;
					osDebugOut( "�����������ʧ��" );
				}
			}
			m_bNeedRestore = false;
	
			break;
		}
	default:
		osassertex( false,"��֧�ֵĸ�ʽ..." );
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
	// windy add ������Ⱦ�����������
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

/// �ͷ�����.
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

//! ��������
bool osc_texture::reset_texture( void )
{
	guard;

	if( m_bNeedRestore )
		release_texture();
	
	return true;

	unguard;
}


//! ���´�������
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

	// �Ȱ�������뵽��ʱ������,Ȼ�󴴽�����.
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int    t_iOffset;

	//@{ snowwin 2006-1-4 Ϊ�����������͵������ļ�,�����ǲ�������ģ�����dds,tga.
	std::string tmpStr = m_strTexname;
	std::transform( tmpStr.begin(), tmpStr.end(), tmpStr.begin(), tolower );
	if( tmpStr.rfind( ".hon" ) == -1 )
	{
		//  River @ 2009-5-4:
		//! ͨ���ر�����ʹ����dds����������
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
		MessageBox( NULL,m_strTexname,"�����������",MB_OK );
		osassertex( false,va( "��������<%s>ʱ����",m_strTexname ) );
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


	// ��ͼ��ĸ�ʽ �������Texture��ʽ��DDS��
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



//! �õ��������ָ��
LPDIRECT3DBASETEXTURE9 osc_texture::get_texpointer( void )
{
	guard;

	// ȷ��������ʹ����.
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

/// ������������
void osc_texture::setPixels(const unsigned char *pBuffer)
{
	guard;

	osassertex( this->m_iTexFormat == 0, "Ŀǰֻ�ܶ�һ��������д˲�����" );

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
//  TextureManager��.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/// ��ʼ����ǰ�����������.
void  osc_textureMgr::init_textureMgr( LPDIRECT3DDEVICE9 _device )
{
	m_pd3dDevice = _device;
	::InitializeCriticalSection( &this->m_sCSGetFreeTexSpace );
}

osc_textureMgr::osc_textureMgr()
{
	m_pd3dDevice = NULL;

	// ��ʼ����ǰ��
	m_vecDtexture.resize( INIT_TEXVECSIZE );
}

osc_textureMgr::~osc_textureMgr()
{
	release_textureMgr();

	::DeleteCriticalSection( &this->m_sCSGetFreeTexSpace );
}


/** \brief 
 *  �ͷ�Texture Manager.
 */
void osc_textureMgr::release_textureMgr( void )
{
	//
	// ���ϲ��Ѿ������е������ͷ����
}

//! ����textureMgr.
void osc_textureMgr::reset_textureMgr( void )
{
	guard;

	// River @ 2010-9-4:�������Ϸ�����ڷ���login������Ctl+Alt���豸�һس����bug.
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

//! �һ�textureMgr.
void osc_textureMgr::restore_textureMgr( void )
{
	guard;

	// River @ 2010-9-4:�������Ϸ�����ڷ���login������Ctl+Alt���豸�һس����bug.
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

	// ���Ҫ����������Ѿ�����������Ҫ�ٴμ��룺
	// ����ҪΪ��ǰ�Ѿ��������������һ�����á�
	CSpaceMgr<osc_texture>::NODE  t_ptrNode;

	// River @ 2006-3-2: �˴���Ҫ��һ��Critical Section,�����߳̿����ڴ˳���.
	::EnterCriticalSection( &this->m_sCSGetFreeTexSpace );
	for( t_ptrNode = m_vecDtexture.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDtexture.next_validNode( &t_ptrNode ) )
	{
		if( (t_ptrNode.p->get_ref() == 0)||(t_ptrNode.p->m_ptex.commonTex == NULL) )
		{
			//! River added @ 2011-1-5:���������豸�һصĴ��󣬴˴��޸�
			//! ��������̹߳�����ʱ�򣬿����豸reset,��ʱ��������������
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

				//! River @ 2009-8-29:�ڼ��޵�����£��˶δ����Ƿ�ִ��
				//! �����������һ��ͬ�����ͷź�����
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
	//���ڴ˴��˳�cs,���������ط����ͷ����cs.
	::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );
	t_b = m_vecDtexture[t_idx].init_texture( _filename,m_pd3dDevice,_format );
	//osDebugOut( "dadd_textomanager:Texture Create<%d><%s>..\n",t_idx,_filename );
	osassertex( t_b,va( "��������<%s>ʧ��...\n",_filename ) );
	osassert( m_vecDtexture.validate_id( t_idx ) );

	return t_idx;

	unguard;
}



/** �´���һ��������������
@remarks
*/
int osc_textureMgr::addnew_texture( const char *name,int w,int h,D3DFORMAT _format,int _type/* =0*/)
{
	guard;

	int        t_idx;
	bool       t_b;

	DWORD      t_dwHash = string_hash( name );

	// River @ 2006-3-2:
	// �������Critical Section,��Ȼ��ͼ���̵߳���ʱ���˴������ͷ���һ���߳���
	// dadd_textomanager�����ڸոյõ�Ҫʹ�õ�texture
	::EnterCriticalSection( &this->m_sCSGetFreeTexSpace );
	CSpaceMgr<osc_texture>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecDtexture.begin_usedNode(); 
		t_ptrNode.p != NULL; t_ptrNode = m_vecDtexture.next_validNode( &t_ptrNode ) )
	{
		// River mod @ 2006-2-18:�ݴ����������ĵط��ͷŵ������û����spaceMgr���ͷ�
		if( (0 == t_ptrNode.p->get_ref()) ||( NULL == t_ptrNode.p->m_ptex.commonTex ) )
		{
			//! River added @ 2011-1-5:���������豸�һصĴ��󣬴˴��޸�
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

				//! River @ 2009-8-29:�ڼ��޵�����£��˶δ����Ƿ�ִ��
				/*  ���������
                    1: ��һ�߳�dadd_textomanagerִ�е�	::LeaveCriticalSection( &this->m_sCSGetFreeTexSpace );
	                   ��û��ִ�У�
					   t_b = m_vecDtexture[t_idx].init_texture( _filename,m_pd3dDevice,_format );
				    2: ��ǰ����ִ�У�
					   if( (0 == t_ptrNode.p->get_ref()) ||( NULL == t_ptrNode.p->m_ptex.commonTex ) )
				    3: ��һ�߳�ִ���꣺
					   t_b = m_vecDtexture[t_idx].init_texture( _filename,m_pd3dDevice,_format );
				    4: ��ǰ�߳̿�ʼִ�У�
					   if( !m_vecDtexture[t_ptrNode.idx].m_bLoadLock )
				    �޸ķ�ʽ�������´��룺if( t_ptrNode.p->m_ptex.commonTex )
					�Թۺ�Ч
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
		osassertex( t_b,va( "��������<%s>ʧ��...\n",name ) );
	}

	// River mod 2007-9-10:����Ǵ�����������ʧ��,�򷵻�-1ID
	if( _type == 10 )
	{
		if( !t_b )
		{	//! ������Ǵ������������ʹ���һ��С����
			t_b = m_vecDtexture[t_idx].create_texture( name, 1, 1, _format, m_pd3dDevice, _type );
			return -1;
		}
	}

	return t_idx;

	unguard;
}


/** �Ը����������������.
 *  ������ü�����Ϊ��,������Ԫ�ػ��Զ��ͷ��Լ�������.
 */
int osc_textureMgr::release_texture( int _id )
{
	if( _id < 0 )return -1;

	::EnterCriticalSection( &this->m_sCSGetFreeTexSpace );

	//@{
	// River added @ 2009-5-26:��ʱ�Ľ������,���ﻻװΪ������
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


/// ��:format = a8r8g8b8,��return 32
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

