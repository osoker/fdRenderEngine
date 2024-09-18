# pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
# include "../../interface/osInterface.h"
# include "../include/osStateBlock.h"

#define GFX_FONT_MAX_POINT_COUNT	(1024*8)

//预处理字体数据
#define GFX_FAST_DATA_COPY   1

//namespace Gfx
//{
	struct STD3DGraphicsVertex
	{
		float x, y, z, w;
		D3DCOLOR color;
	};
	static const DWORD D3DFVF_D3DGraphicsVertex = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	
	class CGfxFont
	{
	protected:
		struct FontFileHeader
		{
			unsigned short	wID;				//字体文件ID
			unsigned char	byVerMajor;			//主版本号
			unsigned char	byVerMinon;			//次版本号
			unsigned char	byFontWidth;		//字体宽,不得>16
			unsigned char	byFontHeight;		//字体高,不得>16
			int             byOutLine;
			unsigned char	Reserve[2];			//保留
			unsigned long	dwFontDataSize;		//字体数据大小
		};
	public:
		CGfxFont( ) : m_pData( NULL ),m_pOutLineData(NULL)
		{
			m_pd3dDevice = 0;
			m_nMaxPointsCount = GFX_FONT_MAX_POINT_COUNT;
			m_nSpaceHeight = 0;
			m_nSpaceWidth = 0;
			memset( &m_Header, 0, sizeof( FontFileHeader ) );
		}
		virtual ~CGfxFont( )
		{
		}
		void release()
		{
			if( m_pVB )m_pVB->Release();
			if( m_pData ) delete [] m_pData;
			if( m_pOutLineData ) delete [] m_pOutLineData;

			if( m_iRenderState>=0 )
				os_stateBlockMgr::Instance()->release_stateBlock( m_iRenderState );
		}
		virtual bool OnCreateDevice( IDirect3DDevice9*pDevice )
		{
			m_pd3dDevice = pDevice;
			IDirect3D9*pD3D;
			if( FAILED( pDevice->GetDirect3D( &pD3D ) ) )
			{
				return false;
			}
			D3DCAPS9 Cap;
			if( FAILED( pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Cap ) ) )
			{
				pD3D->Release();
				return false;
			}
			pD3D->Release();
			if( (DWORD)m_nMaxPointsCount > Cap.MaxPrimitiveCount )
			{
				m_nMaxPointsCount = Cap.MaxPrimitiveCount;
			}
			if( FAILED( pDevice->CreateVertexBuffer( m_nMaxPointsCount*sizeof(STD3DGraphicsVertex),
				D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_D3DGraphicsVertex,
				D3DPOOL_DEFAULT, &m_pVB, NULL ) ) )
			{
				return false;
			}
			record_RenderState();

			return true;
		}
 
		virtual bool OnResetDevice( IDirect3DDevice9*pDevice)
		{
			HRESULT hr;
			if( FAILED( hr = pDevice->CreateVertexBuffer( m_nMaxPointsCount*sizeof(STD3DGraphicsVertex),
				D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_D3DGraphicsVertex,
				D3DPOOL_DEFAULT, &m_pVB, NULL ) ) )
			{
				osassertex( false,"字体reset Device失败...\n" );
				return false;
			}
			record_RenderState();
			return true;
		}
		virtual bool OnLostDevice( IDirect3DDevice9*pDevice )
		{
			if( m_pVB )
			{
				m_pVB->Release( );
				m_pVB = NULL;
			}
			return true;
		}
		virtual bool OnDestroyDevice( IDirect3DDevice9*pDevice )
		{
			if( m_pVB )
			{
				m_pVB->Release( );
				m_pVB = NULL;
			}
			return true;
		}
		void record_RenderState();
		
		bool LoadFromFile( const char* szFileName )
		{
			int               t_size;
			BYTE*             t_ptrBuf;
			osassert( (szFileName)&&(szFileName[0]!=NULL) );
			int   t_iGBufIdx = -1;
			t_ptrBuf = START_USEGBUF( t_iGBufIdx );
			t_size = read_fileToBuf( (char*)szFileName,t_ptrBuf,TMP_BUFSIZE );
			osassert( t_size>=0 );
			READ_MEM_OFF( &m_Header,t_ptrBuf,sizeof( m_Header ) );
			if( m_pData )
				delete [] m_pData;
			m_pData = new unsigned short[ m_Header.dwFontDataSize ];
			READ_MEM_OFF( m_pData,t_ptrBuf, m_Header.dwFontDataSize*sizeof( unsigned short ) );
			END_USEGBUF( t_iGBufIdx );
/*
			FILE*fp = fopen( szFileName, "rb" );
			if( !fp )
			{
				return false;
			}
			fseek(fp,0,SEEK_END );
			long len = ftell(fp);
			fseek(fp,0,SEEK_SET );

			fread( &m_Header, 1, sizeof( m_Header ), fp );

			if( m_pData )
			{
				delete [] m_pData;
			}
			m_pData = new unsigned short[ m_Header.dwFontDataSize ];
			if( !m_pData)
			{
				return false;
			}
			fread( m_pData, m_Header.dwFontDataSize*sizeof( unsigned short ),  1 , fp );
			fclose(fp);
*/
			if( m_Header.byOutLine )
			{
				char outlinefile[256];
				sprintf( outlinefile, "%so", szFileName );
				t_ptrBuf = START_USEGBUF( t_iGBufIdx );
				t_size = read_fileToBuf( (char*)outlinefile,t_ptrBuf,TMP_BUFSIZE );
				osassert( t_size>=0 );
				if( m_pOutLineData )
						delete [] m_pOutLineData;
				m_pOutLineData = new unsigned short[ m_Header.dwFontDataSize ];
				READ_MEM_OFF( m_pOutLineData,t_ptrBuf, m_Header.dwFontDataSize*sizeof( unsigned short ) );
				END_USEGBUF( t_iGBufIdx );
/*
				fp = fopen( outlinefile, "rb" );
				if(fp)
				{
					if( m_pOutLineData )
						delete [] m_pOutLineData;
					m_pOutLineData = new unsigned short[ m_Header.dwFontDataSize ];
					fread( m_pOutLineData, m_Header.dwFontDataSize*sizeof( unsigned short ), 1, fp );
					fclose(fp);				
				}*/
			}
		
			return true;
		}
		inline int GetFontSize( ) const { return m_Header.byFontWidth; }
		inline unsigned short* GetData( ) { return m_pData; }
		inline unsigned short* GetASCII( unsigned char c ) { return m_pData+(c<<4); }
		inline unsigned short* GetASCIIOutLine( unsigned char c ) { return m_pOutLineData+(c<<4); }
		inline unsigned short* GetUnicode( unsigned char u1,unsigned char u2 ) { return m_pData+((((u1-128)<<8)+u2+128)<<4); }
		inline unsigned short* GetUnicodeOutLine( unsigned char u1,unsigned char u2 ) { return m_pOutLineData+((((u1-128)<<8)+u2+128)<<4); }
		inline void SetSpaceWidth( int nWidth )
		{
			m_nSpaceWidth = nWidth;
		}
		inline int GetSpaceWidth( )const
		{
			return m_nSpaceWidth;
		}
		inline void SetSpaceHeight( int nHeight )
		{
			m_nSpaceHeight = nHeight;
		}
		inline int GetSpaceHeight( )const
		{
			return m_nSpaceHeight;
		}
		inline bool InClipRect( int x, int y, RECT* _clipRect )
		{
			if( x >= _clipRect->left && x < _clipRect->right && 
				y >= _clipRect->top && y < _clipRect->bottom )
			{
				return true;
			}
			return false;
		}
		BOOL clip_font( RECT* _clipRect, int _x, int _y );
		void ShowText( IDirect3DDevice9*pDevice, int x, int y, float z, int nLineChar, DWORD dwColor, 
			           BOOL bShowOutLine, DWORD dwOutLineColor, RECT* _clipRect, const char*szText, int nCount = -1 );

		void SetPointsBufferSize( int nMaxPointsCount )
		{
			m_nMaxPointsCount = nMaxPointsCount;
		}
	protected:
		FontFileHeader							m_Header;
		unsigned short							*m_pData;
		unsigned short                          *m_pOutLineData;
		IDirect3DVertexBuffer9					*m_pVB;
		int										m_nSpaceWidth;
		int										m_nSpaceHeight;
		int										m_nMaxPointsCount;
		IDirect3DDevice9*						m_pd3dDevice;

		int			m_iRenderState;	
	};

//}