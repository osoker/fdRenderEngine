//===================================================================
/** \file  
*  Filename: font.cpp
*  Desc:     ʵ��һ��������ʾ���ࡣ
*  His:      ���� @ 2/4 2004 16:18
*
*  "Ψ�е������Կ���ˣ����Ī����"
*  
*/
//===================================================================
# include "stdafx.h"
# include "../include/Font.h"
# include "../../interface/osInterface.h"
# include "../include/osStateBlock.h"
# include "../../TrueType/ft2build.h"
#include "../../mfpipe/include/deviceInit.h"

#pragma warning (disable : 4244) 



#define TEXT_SHADOW

// ��������Ĵ�ϸ�̶�
//const int   g_iFontStyle = FW_LIGHT;


//! �������õ�������Ŀ�ȣ�Ĭ��ֵΪ12,�����ı��С��
//! ���ڳ�ʼ����ʱ���޸����ȫ�ֱ�����
//OSENGINE_API int        DEVICE_CHAR_WIDTH = 24;

//! ���ֱ���������Ӧ�ô��������Ŀ��
const int   BGC_OFFSET  = 1;

//! �������ֵ��м�ء�
int        TEXT_LINEDIS = 2;

//# define DEFAULT_FONT   "����_GB2312"

/**brief*****************************************************************/
/*!���캯��
/************************************************************************/ 
CWFont::CWFont() : m_hDc(NULL), m_hBmp(NULL), m_hFont(NULL),
m_iMaxTextureCharNum(0), m_RowNum(0), m_pBits(NULL), 
m_TextSize(0), m_TextureSize(0),
m_pd3dDevice(NULL), m_pVB(NULL), m_pTexture(NULL), 
m_pShadowVB(NULL)
{
	m_iRenderState = -1;
	m_iBGRState = -1;
	m_iRenderShadow = -1;
	m_TextSize = 12;
}
/**brief*****************************************************************/
/*!��������
/************************************************************************/ 
CWFont::~CWFont()
{
	// ר�ŵ�������ӿ�.
	//	ReleaseFont();
}
/**brief*****************************************************************/
/*desc:	
��������
*
*	\param1		3D�豸
*	\param2		��������
*	\param3		�����С������ʹ��16��24����
*	\param4		�����С
*	\return		�ɹ�����true
/************************************************************************/ 
bool CWFont::CreateFont( LPDIRECT3DDEVICE9 pd3dDevice, 
						char szFontName[], int nSize,int lfWeight /*= FW_LIGHT*/, int _CharTexSize/* = 512*/ )
{
	guard;

	osassert( pd3dDevice!=NULL );
	osassert( szFontName!=NULL );

	ReleaseFont();
	m_hDc = CreateCompatibleDC(NULL);

	m_pd3dDevice = pd3dDevice;

	m_TextSize	 = nSize;	
	m_TextureSize = _CharTexSize;		

	m_RowNum = m_TextureSize / m_TextSize;	
	m_iMaxTextureCharNum = m_RowNum * m_RowNum;

	LOGFONT LogFont;
	ZeroMemory(&LogFont, sizeof(LOGFONT) );
	// ��ֵ��������Ϊ��,������win98��������Ĵ�С���������.
	LogFont.lfHeight = -nSize;

	LogFont.lfWidth = 0;
	LogFont.lfEscapement = 0;
	LogFont.lfOrientation = 0;
	LogFont.lfWeight = lfWeight;
	LogFont.lfItalic = FALSE;
	LogFont.lfUnderline = FALSE;
	LogFont.lfStrikeOut = FALSE;
	LogFont.lfCharSet = DEFAULT_CHARSET;
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality = ANTIALIASED_QUALITY;
	LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lstrcpy(LogFont.lfFaceName, szFontName);

	m_hFont = CreateFontIndirect(&LogFont);

	if ( NULL == m_hFont )
	{
		// ���������岻�ɹ�,������������.
		lstrcpy( LogFont.lfFaceName,"" );
		m_hFont = CreateFontIndirect(&LogFont);

		if( NULL == m_hFont )
		{
			DeleteDC( m_hDc );
			osassertex( false,"��������ʧ��...\n" );
			return false;
		}
	}



	BITMAPINFO bmi;
	ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		= m_TextSize;
	bmi.bmiHeader.biHeight		= -m_TextSize;
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	m_hBmp = CreateDIBSection( m_hDc, &bmi, DIB_RGB_COLORS,
		(void **) &m_pBits, NULL, 0 );
	if ( NULL == m_hBmp || NULL == m_pBits )
	{
		DeleteObject( m_hFont );
		DeleteDC( m_hDc );
		return false;
	}

	m_hOldBmp = SelectObject( m_hDc, m_hBmp );
	m_hOldFont = SelectObject( m_hDc, m_hFont );

	SetTextColor( m_hDc, RGB(255,255,255) );
	SetBkColor( m_hDc, 0 );
	SetTextAlign( m_hDc, TA_TOP );



	if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_iMaxTextureCharNum * 6 * sizeof(FONT_VER),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
		D3DPOOL_DEFAULT, &m_pVB,NULL ) ) )
	{
		DeleteObject( m_hFont );
		DeleteObject( m_hBmp );
		DeleteDC( m_hDc );
		return false;
	}


	if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_iMaxTextureCharNum * 6 * sizeof(FONT_VER),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
		D3DPOOL_DEFAULT, &m_pShadowVB,NULL ) ) )
	{
		DeleteObject( m_hFont );
		DeleteObject( m_hBmp );
		DeleteDC( m_hDc );
		return false;
	}
		
	
	if ( FAILED( m_pd3dDevice->CreateTexture( m_TextureSize, m_TextureSize, 1, 0, 
		D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pTexture,NULL ) ) 

		)
	{
		DeleteObject( m_hFont );
		DeleteObject( m_hBmp );
		DeleteDC( m_hDc );
		SAFE_RELEASE(m_pVB);
		SAFE_RELEASE(m_pShadowVB);

		return false;
	}


	// ATTENTION TO FIX: ȫ�����㣬���ڵ����Ascii�ַ�������
	m_vBuf.resize(m_iMaxTextureCharNum);
	ZeroMemory( &m_vBuf[0],sizeof( WORD )*m_iMaxTextureCharNum );

	//
	// 
	record_shadowState();


	record_RenderState();
	record_RBGState();

	SetRenderState();

	return true;

	unguard;
}



/** \brief
 *  �õ������������е�λ��
 *
 *	\param1		���ֵĵ�һ���ֽ�
 *	\param2		���ֵĵڶ����ֽ�
 *	\param3		�������������x����
 *	\param4		�������������y����
 *  River @ 2006-3-31: Ϊ�˼��ٲ����ַ�����ʱ�䣬�Ż��˶γ���
 *  ʹ������CharToTexture��ʽ֮ǰ����ҪҪ��Ⱦ���ִ����зֽ⣬ȷ��
 *  һ���ִ�û�����е������ַ�ͬʱ���ڣ��������ַ�ռ����ͬһ��hashλ�ã�
 *  ��ȴ����ͬһ���ַ����ݣ�����','��'�ա���ռ����ͬһ��λ�ã�ͬʱ����
 *  �������ַ����ִ���Ҫ�ֶ����Ⱦ��
 */ 
bool CWFont::CharToTexture( char _c1, char _c2,int _idx,
						   int & _tX, int & _tY,const char* _rstr )
{
	guard;
	if( (_c1&0x80)&&(!(_c2&0x80)) )   
		return true;
	if( (!(_c1&0x80))&&(_c2&0x80) ) 
		return true;

	char test[3] = {_c1, _c2, '\0'};
	WORD t_wChar = MAKEWORD(_c1, _c2);
	int  t_idx = t_wChar % int(m_vBuf.size());

	// �����ǰ��λ�ò�������Ҫ�����ֵķ�,��ʧЧ����ַ�
	if( m_vBuf[t_idx] != t_wChar )
	{
		// ������ǵ�һ�γ�ʼ��
		if( m_vBuf[t_idx]!=0 )
		{
			// �Դ��ִ���ǰ�ַ�֮ǰ�������ַ����б���
			WORD t_wTmp;
			for( int t_cidx = 0;t_cidx<_idx;t_cidx ++ )
			{
				if ( _rstr[t_cidx] & 0x80 )
				{
					t_wTmp = MAKEWORD( _rstr[t_cidx],_rstr[t_cidx+1] );
					t_cidx ++;
				}
				else
				{
					t_wTmp = MAKEWORD( _rstr[t_cidx],0 );
				}

				// ��Ҫ�ϲ㺯���Ȱ������ִ���Ⱦ��ɺ�,�������������ڳ���µ�Char
				osassert( t_wTmp != t_wChar );
				if( t_idx == (t_wTmp % int(m_vBuf.size())) )
					return false;
			}
		}


		// ���ض�λ��ʹ���µ�������
		_tX = (t_idx % m_RowNum) * m_TextSize;
		_tY = (t_idx / m_RowNum) * m_TextSize;

		m_vBuf[t_idx] = t_wChar;

		RECT rect = {0, 0, m_TextSize, m_TextSize};
		char sz[3] = {_c1, _c2, '\0'};

		FillRect( m_hDc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH) );
		SetBkMode( m_hDc, TRANSPARENT); 
		::TextOut( m_hDc, 0, 0, sz, _c1 & 0x80 ? 2 : 1 );

		D3DLOCKED_RECT d3dlr;
		m_pTexture->LockRect(0, &d3dlr, NULL, D3DLOCK_NOSYSLOCK);
		BYTE * pDstRow = (BYTE*)( (WORD *)d3dlr.pBits + _tY * m_TextureSize + _tX );

		for (int y=0; y < m_TextSize; y++)
		{
			WORD * pDst16 = (WORD*)pDstRow;
			for (int x=0; x<m_TextSize; x++)
			{
				BYTE bAlpha = (BYTE)((m_pBits[m_TextSize * y + x] & 0xff) >> 4);
				if (bAlpha > 0)
					*pDst16++ = (bAlpha << 12) | 0x0fff;
				else
					*pDst16++ = 0x0000;
			}
			pDstRow += d3dlr.Pitch;
		}

		m_pTexture->UnlockRect( NULL );

	}
	else
	{
		_tX = (t_idx % m_RowNum) * m_TextSize;
		_tY = (t_idx / m_RowNum) * m_TextSize;
	}

	//D3DXSaveTextureToFile("C:\\1.JPG",D3DXIFF_JPG,m_pTexture,NULL);
	return true;

	unguard;
}


/**brief*****************************************************************/
/*!������Ⱦ״̬��׼����ʼ��Ⱦ����
*
/************************************************************************/ 
void CWFont::Begin()
{
	if( (m_iRenderState>=0)&&
		os_stateBlockMgr::Instance()->validate_stateBlockId( m_iRenderState ) )
	{
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderState );
	}
	else
	{
		this->record_RenderState();
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderState );
	}
}


/** \brief
*  ��Ⱦ����ı���ɫ��
*
*  \param _xsize �᷽���϶�Ӧ���ַ���Ŀ���������ֵ�����Ҫ����
*                �ı���ͼƬ�Ĵ�С��
*  \param _color Ҫ���ı�������ɫ��
*/
void CWFont::draw_textBGTri( float _x,float _y,float _z,int _xsize,DWORD _color )
{
	guard;

	if( _xsize==0 )
		return;

	FONT_VER    t_bgVer[4];

	for( int t_i=0;t_i<4;t_i ++ )
	{
		t_bgVer[t_i].m_vPos.z = _z;
		t_bgVer[t_i].m_vPos.w = 1.0f;

		t_bgVer[t_i].m_color = _color;
		t_bgVer[t_i].m_fUV = osVec2D( 0.0f,0.0f );
	}


	// ���ֵı߿��ټ��Ϲ̶���С��offset.
	t_bgVer[0].m_vPos.x = _x-BGC_OFFSET;
	t_bgVer[0].m_vPos.y = _y-BGC_OFFSET;

	t_bgVer[1].m_vPos.x = _x+m_TextSize/2*_xsize+BGC_OFFSET;
	t_bgVer[1].m_vPos.y = _y-BGC_OFFSET;

	t_bgVer[2].m_vPos.x = _x+m_TextSize/2*_xsize+BGC_OFFSET;
	t_bgVer[2].m_vPos.y = _y+m_TextSize+BGC_OFFSET;

	t_bgVer[3].m_vPos.x = _x-BGC_OFFSET;
	t_bgVer[3].m_vPos.y = _y+m_TextSize+BGC_OFFSET;


	HRESULT   t_hr;
	t_hr = m_pd3dDevice->DrawPrimitiveUP( 
		D3DPT_TRIANGLEFAN,2,t_bgVer,sizeof( FONT_VER ) );
	osassert( !FAILED( t_hr ) );

	return;

	unguard;
}


//! ��Ⱦһ���ַ����ı���ɫ��
bool CWFont::drawTextBG( char* _szText,
						int _x,int _y,float _z/* = 0.9f*/,DWORD _color/* = 0*/,float _scale/* = 1.0f*/ )
{
	guard;

	if( !_szText || !m_pd3dDevice)
		return false;

	if( _szText[0] == NULL )
		return true;

	float   sx = float(_x)-0.5f, sy = float(_y)-0.5f;
	float   h = (float)m_TextSize * _scale;
	int     t_iChrNum;
	char    ch[2];

	int nLen = lstrlen( _szText );
	t_iChrNum = 0;
	for (int n=0; n<nLen; n++ )
	{
		ch[0] = _szText[n];

		// 
		// �������ַ����д���
		if ( ch[0]=='\n' )
		{
			draw_textBGTri( sx,sy,_z,t_iChrNum,_color );

			sy += h;

			// �����м�ء�
			sy += TEXT_LINEDIS;

			// ʹ��dx�����ض���׼��.
			sx = float(_x)-0.5f;

			t_iChrNum = 0;

			continue;
		}

		//
		// Tab��ռ��5���ַ���
		if(ch[0] == '\t')
		{
			t_iChrNum += 5;
			continue;
		}

		// ����س���.
		if( ch[0] == 13 )
			continue;

		t_iChrNum ++;
	}

	draw_textBGTri( sx,sy,_z,t_iChrNum,_color );

	return true;

	unguard;
}



//! �������㻺����,����dip,��Ⱦ���еĻ��������ַ�����
void CWFont::vbText_dip( int _triangleNum,bool _bShadow )
{
	guard;

	m_pVB->Unlock();
	if( _bShadow )
		m_pShadowVB->Unlock();

	// С�ڵ��ڣ�����Ҫ��Ⱦ.
	if( _triangleNum <= 0 )
		return ;

	if( _bShadow )
	{
		set_shadowState();
		if(FAILED(m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, _triangleNum )))
			osassertex( false,"��Ⱦ�ַ�ʧ��..." );
	}

	// 
	// ÿ��Ⱦһ���ִ�������һ����Ⱦ״̬����?
	SetRenderState();

	if( FAILED(m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, _triangleNum ) ) )
		osassertex( false,"��Ⱦ�ַ�ʧ��..." );

	unguard;
}


/**brief*****************************************************************/
/*!��Ⱦ����
*
*	\param1		Ҫ��Ⱦ������
*				��NOTE:����Ⱦ�����ֵĸ������ܴ��ڻ�����Ŀ��
*	\param2		��Ļ�ϵ�X����
*	\param3		��Ļ�ϵ�Y����
*	\param4		���ֵ���ɫ
*	\param5		���Զ����ִ�С��������
*	return		�ɹ�����true;
*   
/************************************************************************/ 
bool CWFont::DrawText( char* szText, int x, int OrigX,int y, float z, 
					  D3DCOLOR color,  float fScale, 
					  bool bShadow, D3DCOLOR ShadowColor, RECT* _clipRect/*=NULL*/  )
{
	guard;
	float NewScale = fScale/m_TextSize;
	

//	osassert( (m_TextureSize>=0)&&(m_TextureSize<=1024) );

	if( !szText || !m_pd3dDevice)
		return false;


	HRESULT    t_hr;
	float sx = float(x)-0.5f, sy = float(y)-0.5f,
		offset=0.0f, w=0.0f, h=0.0f, tx1=0.0f, ty1=0.0f, tx2=0.0f, ty2=0.0f;
	w = h = (float)m_TextSize * NewScale;

	char ch[3] = {0,0,0};
	FONT_VER *  pVertices = NULL;
	FONT_VER *  pShadowVertices = NULL;
	UINT        wNumTriangles = 0;

	if( szText[0] == NULL )
		return true;

	t_hr = m_pVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr ) );


	if( bShadow )
		m_pShadowVB->Lock(0,0, (void**)&pShadowVertices, D3DLOCK_DISCARD);
	else
		osassert( pShadowVertices == NULL );

	int tx =0, ty=0, tw=0, th=0,nw = 0, nh = 0;
	float tu=0.f,tv=0.f;
	int nLen = lstrlen( szText );
	int Bitofs = 0;

	for( int n=0; n<nLen; n++ )
	{
		ch[0] = szText[n];

		// 
		// �������ַ����д���
		if ( ch[0]=='\n' )
		{
			sy += h;

			// ����Ĭ�ϵ��м��
			sy += TEXT_LINEDIS;

			// ʹ��dx�����ض���׼��.
			sx = float(OrigX)-0.5f;
			continue;
		}
		if(ch[0] == '\t')
		{
			sx+= w*5;
			continue;
		}
		// ����س���.
		if( ch[0] == 13 )
			continue;
		if(ch[0] == ' ')
		{
			sx+= (w/2);
			continue;
		}

		// 
		// ��Ӣ���ַ����ǿ����ַ�,
		if ( ( ch[0] & 0x80 ) /*&& (ch[1] & 0x80 )*/ )
		{
			n++;
			ch[1] = szText[n];
			offset = w;
			Bitofs+=2;
		}
		else
		{
			Bitofs++;
			ch[1] = '\0';
			offset = w / 2 ;
		}

		int a, b,t_iCharStartIdx;
		// ���CharToTextureʧ��,����Ⱦ��ܹ����
		if( ch[1] == '\0' )
			t_iCharStartIdx = n;
		else
			t_iCharStartIdx = n - 1;
		if( !CharToTexture( ch[0], ch[1],t_iCharStartIdx, a, b,szText  ) )
		{
			int t_cidx;

			// 
			// �������㻺����,����״̬����Ⱦ
			vbText_dip( wNumTriangles,bShadow );

			if( ch[1] == '\0' )
			{
				t_cidx = n;
				
			}
			else
			{
				Bitofs -=2;
				t_cidx = n-1;
			}


			DrawText( &szText[t_cidx],sx,OrigX,sy,z,color,fScale,bShadow,ShadowColor,_clipRect );
			return true;
		}

		//! �����������Ͻ� 0.0-1.0
		tx1 = (float)(a) / m_TextureSize;
		ty1 = (float)(b) / m_TextureSize;

		//! �����������Ͻ� 0.0-1.0
		tx2 = tx1 + (float)m_TextSize / m_TextureSize;
		ty2 = ty1 + (float)m_TextSize / m_TextureSize;

		nw = w;
		nh = h;
		bool bAddRect = false;
		bool bLeft = false, bRight = false, bTop = false, bBottom = false;

		if( _clipRect )
		{
			// out of clip rect...
			if( sx + w <= _clipRect->left || sx >= _clipRect->right || 
				sy + h <= _clipRect->top || sy >= _clipRect->bottom )
			{
				sx += offset;
				continue;
			}

			//inner
			if( sx > _clipRect->left && sx + w < _clipRect->right && sy > _clipRect->top && sy + h < _clipRect->bottom )
			{
				(*pVertices++).SetValue(D3DXVECTOR4(sx,     sy + h, z, 1.0f),color,D3DXVECTOR2(tx1,ty2));
				(*pVertices++).SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));
				(*pVertices++).SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));

				(*pVertices++).SetValue(D3DXVECTOR4(sx + w, sy,     z, 1.0f),color,D3DXVECTOR2(tx2,ty1));
				(*pVertices++).SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
				(*pVertices++).SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));

				// shadow vertices...
				if( bShadow )
				{
					(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1,     sy+1 + h, z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty2));
					(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1,	    sy+1,	z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
					(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1 + w, sy+1 + h, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));

					(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1 + w, sy+1, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty1));
					(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1 + w, sy+1 + h,z,1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));
					(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1,	    sy+1,	z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
				}

				sx += offset;
				wNumTriangles += 2;
				continue;
			}

			// x���� left
			if( sx < _clipRect->left && sx + w > _clipRect->left )
			{
				bLeft = true;
			}
			// x���� right
			if( sx < _clipRect->right && sx + w > _clipRect->right )
			{
				bRight = true;
			}
			// y���� top
			if( sy < _clipRect->top && sy + h > _clipRect->top )
			{
				bTop = true;
			}
			// y���� bottom
			if( sy < _clipRect->bottom && sy + h > _clipRect->bottom )
			{
				bBottom = true;
			}

			if( bLeft && !bRight && !bTop && !bBottom )
			{
				clip_left( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bRight && !bLeft && !bTop && !bBottom )
			{
				clip_right( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bTop && !bBottom && !bLeft && !bRight )
			{
				clip_top( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,nw,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bBottom && !bTop && !bLeft && !bRight )
			{
				clip_bottom( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,nw,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bLeft && bTop && !bRight && !bBottom )
			{
				clip_leftTop( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,nw,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bTop && bRight && !bLeft && !bBottom )
			{
				clip_topRight( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bLeft && bBottom && !bRight && !bTop )
			{
				clip_leftBottom( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,color, ShadowColor, pVertices,pShadowVertices );
				bAddRect = true;
			}

			if( bRight && bBottom && !bTop && !bLeft )
			{

				clip_rightBottom( _clipRect, sx, sy, z, w, h, tx,ty, tx1,tx2,ty1,ty2,tu,tv,color, ShadowColor, pVertices,pShadowVertices   );
				bAddRect = true;
			}

			if( bAddRect )
			{
				pVertices += 6;

				// 
				if( bShadow )
					pShadowVertices += 6;

				sx += offset;
				wNumTriangles += 2;
			}
		}
		else
		{
			// ATTENION TO OPP: ����Щ���ݷ��õ��ڴ���,��һ������䵽���㻺������,����ʹ��һ��
			// �̶���Index buffer.
			(*pVertices++).SetValue(D3DXVECTOR4(sx,     sy + h, z, 1.0f),color,D3DXVECTOR2(tx1,ty2));
			(*pVertices++).SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));
			(*pVertices++).SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));

			(*pVertices++).SetValue(D3DXVECTOR4(sx + w, sy,     z, 1.0f),color,D3DXVECTOR2(tx2,ty1));
			(*pVertices++).SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
			(*pVertices++).SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));


			if( bShadow )
			{
				// shadow vertices...
				(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1,     sy+1 + h, z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty2));
				(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1,	    sy+1,	z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
				(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1 + w, sy+1 + h, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));

				(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1 + w, sy+1, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty1));
				(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1 + w, sy+1 + h,z,1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));
				(*pShadowVertices++).SetValue(D3DXVECTOR4(sx+1,	    sy+1,	z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
			}

			wNumTriangles += 2;
			sx += offset;
		}
	}

	// 
	// �������㻺����,����״̬����Ⱦ
	vbText_dip( wNumTriangles,bShadow );
	
	return true;

	unguard;
}

void CWFont::clip_left( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  )
{
	//leftBottom
	tu = tx1 + ( ( float( _clipRect->left - sx ) / float( w ) ) * float( tx2 - tx1 ) );
	tv = ty2;
	tx = _clipRect->left;
	ty = sy + h;
	pVertices[0].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );

	// 
	if( pShadowVertices )
		pShadowVertices[0].SetValue( D3DXVECTOR4( tx+1, ty+1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );

	//leftTop
	tv = ty1;
	ty = sy;
	pVertices[1].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	pVertices[5].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
		pShadowVertices[5].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
	}

	//other
	pVertices[2].SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
	pVertices[3].SetValue(D3DXVECTOR4(sx + w, sy,     z, 1.0f),color,D3DXVECTOR2(tx2,ty1));
	pVertices[4].SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));

		// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue(D3DXVECTOR4(sx + w + 1, sy + h + 1, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));
		pShadowVertices[3].SetValue(D3DXVECTOR4(sx + w + 1, sy + 1,     z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty1));
		pShadowVertices[4].SetValue(D3DXVECTOR4(sx + w + 1, sy + h + 1, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2)); 			
	}
}

void CWFont::clip_right( RECT* _clipRect, float sx, float sy, float z,
						float w, float h,
						float tx, float ty,
						float tx1,float tx2,float ty1,float ty2,
						float tu, float tv,
						D3DCOLOR color, D3DCOLOR ShadowColor,
						FONT_VER* pVertices, FONT_VER* pShadowVertices  )
{
	//rightBottom
	tu = tx2 - ( ( float( sx + w - _clipRect->right ) / float( w ) ) * float( tx2 - tx1 ) );
	tv = ty2;
	tx = _clipRect->right;
	ty = sy + h;
	pVertices[2].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[4].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[4].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//rightTop
	tv = ty1;
	ty = sy;
	pVertices[3].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[3].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//other
	pVertices[0].SetValue(D3DXVECTOR4( sx,sy + h, z, 1.0f ), color, D3DXVECTOR2( tx1, ty2 ) );
	pVertices[1].SetValue(D3DXVECTOR4( sx,sy,	  z, 1.0f ), color, D3DXVECTOR2( tx1, ty1 ) );
	pVertices[5].SetValue(D3DXVECTOR4( sx,sy,	  z, 1.0f ), color, D3DXVECTOR2( tx1, ty1 ) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue(D3DXVECTOR4( sx + 1, sy + h + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tx1, ty2 ) );
		pShadowVertices[1].SetValue(D3DXVECTOR4( sx + 1, sy + 1,	  z, 1.0f ), ShadowColor, D3DXVECTOR2( tx1, ty1 ) );
		pShadowVertices[5].SetValue(D3DXVECTOR4( sx + 1, sy + 1,	  z, 1.0f ), ShadowColor, D3DXVECTOR2( tx1, ty1 ) );
	}
}

void CWFont::clip_top( RECT* _clipRect, float sx, float sy, float z,
					  float w, float h,
					  float tx, float ty,
					  float tx1,float tx2,float ty1,float ty2,
					  float tu, float tv,
					  float nw,
					  D3DCOLOR color, D3DCOLOR ShadowColor,
					  FONT_VER* pVertices, FONT_VER* pShadowVertices  )
{
	//leftTop
	tu = tx1;
	tv = ty1 + ( ( float( _clipRect->top - sy ) / float( h ) ) * float( ty2 - ty1 ) );
	tx = sx;
	ty = _clipRect->top;
	pVertices[1].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[5].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[5].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//rightTop
	tu = tx2;
	tx = sx + nw;
	pVertices[3].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[3].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//other
	pVertices[0].SetValue(D3DXVECTOR4(sx,     sy + h, z, 1.0f),color,D3DXVECTOR2(tx1,ty2));
	pVertices[2].SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
	pVertices[4].SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue(D3DXVECTOR4( sx + 1,     sy + h + 1, z, 1.0f ),ShadowColor,D3DXVECTOR2(tx1,ty2));
		pShadowVertices[2].SetValue(D3DXVECTOR4( sx + w + 1, sy + h + 1, z, 1.0f ),ShadowColor,D3DXVECTOR2(tx2,ty2));
		pShadowVertices[4].SetValue(D3DXVECTOR4( sx + w + 1, sy + h + 1, z, 1.0f ),ShadowColor,D3DXVECTOR2(tx2,ty2));
	}
}

void CWFont::clip_bottom( RECT* _clipRect, float sx, float sy, float z,
						 float w, float h,
						 float tx, float ty,
						 float tx1,float tx2,float ty1,float ty2,
						 float tu, float tv,
						 float nw,
						 D3DCOLOR color, D3DCOLOR ShadowColor,
						 FONT_VER* pVertices, FONT_VER* pShadowVertices  )
{
	//leftBottom
	tu = tx1;
	tv = ty2 - ( ( float( sy + h - _clipRect->bottom ) / float( h ) ) * float( ty2 - ty1 ) );
	tx = sx;
	ty = _clipRect->bottom;
	pVertices[0].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//rightBottom
	tu = tx2;
	tx = sx + nw;
	pVertices[2].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[4].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[4].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//other
	pVertices[1].SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));
	pVertices[3].SetValue(D3DXVECTOR4(sx + w, sy,     z, 1.0f),color,D3DXVECTOR2(tx2,ty1));
	pVertices[5].SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));
	// 
	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue(D3DXVECTOR4( sx + 1,	    sy + 1,	    z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
		pShadowVertices[3].SetValue(D3DXVECTOR4( sx + w + 1, sy + 1,     z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty1));
		pShadowVertices[5].SetValue(D3DXVECTOR4( sx + 1,	    sy + 1,	    z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
	}
}

void CWFont::clip_leftTop( RECT* _clipRect, float sx, float sy, float z,
						  float w, float h,
						  float tx, float ty,
						  float tx1,float tx2,float ty1,float ty2,
						  float tu, float tv,
						  float nw,
						  D3DCOLOR color, D3DCOLOR ShadowColor,
						  FONT_VER* pVertices, FONT_VER* pShadowVertices  )
{
	guard;

	osassert( pVertices );

	tu = tx1 + ( ( float( _clipRect->left - sx ) / float( w ) ) * float( tx2 - tx1 ) );
	tv = ty2;
	tx = _clipRect->left;
	ty = sy + h;
	pVertices[0].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
	}

	//leftTop
	tv = ty1 + ( ( float( _clipRect->top - sy ) / float( h ) ) * float( ty2 - ty1 ) );
	tx = _clipRect->left;
	ty = _clipRect->top;
	pVertices[1].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	pVertices[5].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
		pShadowVertices[5].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
	}

	//rightTop
	tu = tx2;
	tx = sx + nw;
	pVertices[3].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[3].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//other
	pVertices[2].SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
	pVertices[4].SetValue(D3DXVECTOR4(sx + w, sy + h, z, 1.0f),color,D3DXVECTOR2(tx2,ty2));
	// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue(D3DXVECTOR4( sx + w + 1, sy + h + 1, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));
		pShadowVertices[4].SetValue(D3DXVECTOR4( sx + w + 1, sy + h + 1, z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty2));
	}

	unguard;
}

void CWFont::clip_topRight( RECT* _clipRect, float sx, float sy, float z,
						   float w, float h,
						   float tx, float ty,
						   float tx1,float tx2,float ty1,float ty2,
						   float tu, float tv,
						   D3DCOLOR color, D3DCOLOR ShadowColor,
						   FONT_VER* pVertices, FONT_VER* pShadowVertices  )
{
	//leftTop
	tu = tx1;
	tv = ty1 + ( ( float( _clipRect->top - sy ) / float( h ) ) * float( ty2 - ty1 ) );
	tx = sx;
	ty = _clipRect->top;
	pVertices[1].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[5].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[5].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//rightTop
	tu = tx2 - ( ( float( sx + w - _clipRect->right ) / float( w ) ) * float( tx2 - tx1 ) );
	tx = _clipRect->right;
	ty = _clipRect->top;
	pVertices[3].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[3].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//rightBottom
	tv = ty2;
	ty = sy + h;
	pVertices[2].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[4].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[4].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//other
	pVertices[0].SetValue(D3DXVECTOR4( sx,sy + h, z, 1.0f ), color, D3DXVECTOR2( tx1, ty2 ) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue(D3DXVECTOR4( sx + 1, sy + h + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tx1, ty2 ) );
	}
}

void CWFont::clip_leftBottom( RECT* _clipRect, float sx, float sy, float z,
							 float w, float h,
							 float tx, float ty,
							 float tx1,float tx2,float ty1,float ty2,
							 float tu, float tv,
							 D3DCOLOR color, D3DCOLOR ShadowColor,
							 FONT_VER* pVertices, FONT_VER* pShadowVertices )
{
	//leftTop
	tu = tx1 + ( ( float( _clipRect->left - sx ) / float( w ) ) * float( tx2 - tx1 ) );
	tv = ty1;
	tx = _clipRect->left;
	ty = sy;
	pVertices[1].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	pVertices[5].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2( tu, tv ) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
		pShadowVertices[5].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2( tu, tv ) );
	}

	//leftBottom
	tv = ty2 - ( ( float( sy + h - _clipRect->bottom ) / float( h ) ) * float( ty2 - ty1 ) );
	tx = _clipRect->left;
	ty = _clipRect->bottom;
	pVertices[0].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//rightBottom
	tu = tx2;
	tx = sx + w;
	ty = _clipRect->bottom;
	pVertices[2].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[4].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[4].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//other
	pVertices[3].SetValue(D3DXVECTOR4(sx + w, sy,     z, 1.0f),color,D3DXVECTOR2(tx2,ty1));
	// 
	if( pShadowVertices )
	{
		pShadowVertices[3].SetValue(D3DXVECTOR4( sx + w + 1, sy + 1,     z, 1.0f),ShadowColor,D3DXVECTOR2(tx2,ty1));
	}
}

void CWFont::clip_rightBottom( RECT* _clipRect, float sx, float sy, float z,
							  float w, float h,
							  float tx, float ty,
							  float tx1,float tx2,float ty1,float ty2,
							  float tu, float tv,
							  D3DCOLOR color, D3DCOLOR ShadowColor,
							  FONT_VER* pVertices, FONT_VER* pShadowVertices )
{
	//leftBottom
	tu = tx1;
	tv = ty2 - ( ( float( sy + h - _clipRect->bottom ) / float( h ) ) * float( ty2 - ty1 ) );
	tx = sx;
	ty = _clipRect->bottom;
	pVertices[0].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[0].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//rightBottom
	tu = tx2 - ( ( float( sx + w - _clipRect->right ) / float( w ) ) * float( tx2 - tx1 ) );
	tx = _clipRect->right;
	ty = _clipRect->bottom;
	pVertices[2].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	pVertices[4].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );
	// 
	if( pShadowVertices )
	{
		pShadowVertices[2].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
		pShadowVertices[4].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}

	//rightTop
	tv = ty1;
	ty = sy;
	pVertices[3].SetValue( D3DXVECTOR4( tx, ty, z, 1.0f ), color, D3DXVECTOR2(tu,tv) );

	if( pShadowVertices )
	{
		pShadowVertices[3].SetValue( D3DXVECTOR4( tx + 1, ty + 1, z, 1.0f ), ShadowColor, D3DXVECTOR2(tu,tv) );
	}
	//other
	pVertices[1].SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));
	pVertices[5].SetValue(D3DXVECTOR4(sx,	    sy,	    z, 1.0f),color,D3DXVECTOR2(tx1,ty1));

	if( pShadowVertices )
	{
		pShadowVertices[1].SetValue(D3DXVECTOR4( sx + 1, sy + 1,  z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
		pShadowVertices[5].SetValue(D3DXVECTOR4( sx + 1, sy + 1,  z, 1.0f),ShadowColor,D3DXVECTOR2(tx1,ty1));
	}
}

void CWFont::record_shadowState()
{

	m_iRenderShadow = os_stateBlockMgr::Instance()->create_stateBlock();

	os_stateBlockMgr::Instance()->start_stateBlock();

	m_pd3dDevice->SetTexture( 0, m_pTexture );
	m_pd3dDevice->SetVertexShader( NULL );
	m_pd3dDevice->SetFVF( D3DFVF_FONT_VER );
	m_pd3dDevice->SetStreamSource( 0, m_pShadowVB,0, sizeof(FONT_VER) );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iRenderShadow);

}

void CWFont::set_shadowState()
{
	if( (m_iRenderShadow>=0)&&
		os_stateBlockMgr::Instance()->validate_stateBlockId( m_iRenderShadow) )
	{
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderShadow );
	}
	else
	{
		record_shadowState();
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderShadow );
	}
}

/**brief*****************************************************************/
/*!������Ⱦ��������ǰ����Ⱦ״̬
*
/************************************************************************/ 
void CWFont::End()
{
}

/**brief*****************************************************************/
/*!�ͷ���Դ
*
/************************************************************************/ 
void CWFont::ReleaseFont()
{

	SelectObject( m_hDc, m_hOldBmp );
	SelectObject( m_hDc, m_hOldFont );

	DeleteObject( m_hFont );
	DeleteObject( m_hBmp );
	DeleteDC( m_hDc );


	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pShadowVB);

	os_stateBlockMgr*   t_ptrIns = os_stateBlockMgr::Instance();
	if( int(m_iBGRState)>=0 )
		t_ptrIns->release_stateBlock( m_iBGRState );
	if( int(m_iRenderState)>=0 )
		t_ptrIns->release_stateBlock( this->m_iRenderState );
	if( int(m_iRenderShadow)>=0 )
		t_ptrIns->release_stateBlock( this->m_iRenderShadow );


	m_vBuf.clear();
}

/**brief*****************************************************************/
/*!���豸��ʧ��ʱ��ʹ��
*
/************************************************************************/ 
HRESULT CWFont::OnLostDevice()
{
	SAFE_RELEASE(m_pVB);


	SAFE_RELEASE(m_pShadowVB);


	// TEST CODE:
	os_stateBlockMgr*   t_ptrIns = os_stateBlockMgr::Instance();
	if( int(m_iBGRState)>=0 )
		t_ptrIns->release_stateBlock( m_iBGRState );
	if( int(m_iRenderState)>=0 )
		t_ptrIns->release_stateBlock( this->m_iRenderState );
	if( int(m_iRenderShadow)>=0 )
		t_ptrIns->release_stateBlock( this->m_iRenderShadow );


	return S_OK;
}

/**brief*****************************************************************/
/*!�޸��豸ʱʹ��
*
/************************************************************************/ 
HRESULT CWFont::OnRestoreDevice()
{
	HRESULT hr;
	if ( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( m_iMaxTextureCharNum * 6 * sizeof(FONT_VER),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
		D3DPOOL_DEFAULT, &m_pVB,NULL ) ) )
	{
		osassertex( FALSE,osn_mathFunc::get_errorStr( hr ) );
		return hr;
	}

	if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( m_iMaxTextureCharNum * 6 * sizeof(FONT_VER),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
		D3DPOOL_DEFAULT, &m_pShadowVB,NULL ) ) )
	{
		osassertex( FALSE,osn_mathFunc::get_errorStr( hr ) );
		return hr;
	}

	record_shadowState();
	record_RenderState();
	record_RBGState();

	return S_OK;
}


//! �޽���Ⱦ�ļ�����ʱ�õ���RenderBlock,
bool CWFont::record_RBGState( void )
{
	guard;


	osassert( m_pd3dDevice );

	m_iBGRState = os_stateBlockMgr::Instance()->create_stateBlock();

	os_stateBlockMgr::Instance()->start_stateBlock();
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,		TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,			0x01 );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,			D3DCMP_GREATEREQUAL );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

	m_pd3dDevice->SetVertexShader( NULL );
	m_pd3dDevice->SetFVF( D3DFVF_FONT_VER );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iBGRState );


	return true;

	unguard;
}

void CWFont::record_RenderState()
{
	HRESULT   t_hr;

	m_iRenderState = os_stateBlockMgr::Instance()->create_stateBlock();

	os_stateBlockMgr::Instance()->start_stateBlock();

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,		TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,			0x06 );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,			D3DCMP_GREATEREQUAL );

	//! ��������Ļ����ʾ�ֵ�ʱ������
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );

	t_hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
	osassert( SUCCEEDED(t_hr) );
	t_hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
	osassert( SUCCEEDED(t_hr) );
	t_hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );	
	osassert( SUCCEEDED(t_hr) );

	m_pd3dDevice->SetTexture( 0, m_pTexture );
	m_pd3dDevice->SetVertexShader( NULL );
	m_pd3dDevice->SetFVF( D3DFVF_FONT_VER );
	m_pd3dDevice->SetStreamSource( 0, m_pVB,0, sizeof(FONT_VER) );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iRenderState);
}

/**brief*****************************************************************/
/*!������Ⱦ״̬
*
/************************************************************************/ 
void CWFont::SetRenderState()
{
	if( (m_iRenderState>=0)&&
		os_stateBlockMgr::Instance()->validate_stateBlockId( m_iRenderState ) )
	{
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderState );
	}
	else
	{
		record_RenderState();
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderState );
	}
}


//! ��ʼ��Ⱦ���ֵı�����
void CWFont::begin_bgRender( void )
{
	if( (m_iBGRState>=0)&&
		os_stateBlockMgr::Instance()->validate_stateBlockId( m_iBGRState ) )
	{
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iBGRState );
	}
	else
	{
		this->record_RBGState();
		os_stateBlockMgr::Instance()->apply_stateBlock( m_iBGRState );
	}
}

FontManager::FontManager()
{
	mFontList.clear();
	/*mFontList.resize(EFT_COUNT);
	for (int  i = 0; i <EFT_COUNT;i++)
	{
		mFontList[i] = NULL;
	}*/
}
void FontManager::Initialize(LPDIRECT3DDEVICE9	pd3dDevice)
{
	osassert(pd3dDevice);
	mpd3dDevice = pd3dDevice;
	baseId = 0;
}
FontManager::~FontManager()
{
	
	for (size_t i = 0; i < mFontList.size();i++)
	{
		if (mFontList[i])
		{
			SAFE_DELETE(mFontList[i]);
		}
	}
	mFontList.clear();
}
int FontManager::RegisterFont(const char* _fontName,int height,int flWeight/*= FW_LIGHT*/)
{
	osassert(mpd3dDevice);
	CWFont* pFont  = new CWFont();
	osc_d3dManager* pd3dMgr = (osc_d3dManager*)get_deviceManagerPtr();
	int texSize = max(min(pd3dMgr->get_maxTextureHeight(),pd3dMgr->get_maxTextureWidth()),512);

	// ��ʱʹ�ù̶���512.�����������ʧЧ���ٶȺ�����
	texSize = 512;

	pFont ->CreateFont(mpd3dDevice,(char*)_fontName,height,flWeight,texSize);
	
	mFontList.push_back(pFont);
	return baseId++;
}
CWFont* FontManager::GetFont(int _fontType)
{
	osassert(_fontType>=0&&_fontType<(int)mFontList.size());
	osassert(mFontList[_fontType]);
	return mFontList[_fontType];
}
void FontManager::OnLostDevice()
{
	for (int i = 0; i < (int)mFontList.size();i++)
	{
		if (mFontList[i])
		{
			mFontList[i]->OnLostDevice();
		}
	}
}
void FontManager::OnRestoreDevice()
{
	for (int i = 0; i < (int)mFontList.size();i++)
	{
		if (mFontList[i])
		{
			mFontList[i]->OnRestoreDevice();
		}
	}
}
void FontManager::ReleaseFont()
{
	for (int i = 0; i < (int)mFontList.size();i++)
	{
		if (mFontList[i])
		{
			mFontList[i]->ReleaseFont();
		}
	}
}
FontManager gFontManager;
OSENGINE_API IFontManager* gFontMgrPtr = &gFontManager;





