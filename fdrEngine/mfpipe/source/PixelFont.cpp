# include "stdafx.h"
# include "../include/PixelFont.h"

void CGfxFont::record_RenderState()
{
	

	m_iRenderState = os_stateBlockMgr::Instance()->create_stateBlock();

	os_stateBlockMgr::Instance()->start_stateBlock();
	m_pd3dDevice->SetTexture( 0, 0 );

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
/*	
	HRESULT   t_hr;
	//! 独立在屏幕上显示字的时的问题
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
*/
	os_stateBlockMgr::Instance()->end_stateBlock( m_iRenderState);
}

BOOL CGfxFont::clip_font( RECT* _clipRect, int _x, int _y )
{
	RECT fontRect,ir;
	fontRect.left = _x;
	fontRect.top = _y;
	fontRect.right = _x + m_Header.byFontWidth;
	fontRect.bottom = _y + m_Header.byFontHeight;
	return !(::IntersectRect( &ir, _clipRect, &fontRect ) );
}

void CGfxFont::ShowText( IDirect3DDevice9*pDevice, int x, int y, float z, int nLineChar, DWORD dwColor, 
			    BOOL bShowOutLine, DWORD dwOutLineColor, RECT* _clipRect, const char*szText, int nCount/* = -1*/ )
{
	if( NULL == pDevice || NULL == szText || m_pData == NULL || m_pVB == NULL )
	{
		return;
	}
	if( nLineChar < 0 )
	{
		nLineChar = 0xFFFFFF;
	}

	const char*p = szText;
	int nTotal = nCount;
	int nLength = strlen( szText );
	if( nTotal < 0 || nTotal > nLength )
	{
		nTotal = nLength;
	}
	const char*pEnd = p + nTotal;
	int xOrg = x;
	int yOrg = y;

	int nCharSize;
	if( m_Header.byOutLine )
		nCharSize = (m_Header.byFontWidth+2) >> 1;
	else
		nCharSize = m_Header.byFontWidth >> 1;

	if( m_Header.byFontWidth & 1 )
	{
		nCharSize ++;
	}

	int nLineCharCount = 0;

	os_stateBlockMgr::Instance()->apply_stateBlock( m_iRenderState );
	
	static const DWORD C_FONT_MASK[]=
	{
		1,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,
		1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<14,1<<15
	};
	STD3DGraphicsVertex* pVertices;

	if( FAILED( m_pVB->Lock( 0, m_nMaxPointsCount*sizeof(STD3DGraphicsVertex), (void**)&pVertices, D3DLOCK_DISCARD ) ) )
	{
		return;
	}
	
	int npts = 0;

	//char buffer[16*16*2];
	bool bFontPoint = false;
	bool bOutLinePoint = false;
	int cur=0;
	
	int fontHeight = m_Header.byFontHeight;
	int fontWidth = m_Header.byFontWidth;
	if( m_Header.byOutLine )
	{
		fontHeight += 2;
		fontWidth += 2;
	}

	for( ; p<pEnd; )
	{
		if( char(*p) > 0 )//ASCII
		{
			if( *p == '\n' )
			{
				xOrg = x;
				yOrg += m_Header.byFontWidth + m_nSpaceHeight;
				p++;
				nLineCharCount = 0;
				continue;
			}
			if( *p == '\t' )
			{
				p++;
				nLineCharCount +=4;
				if( nLineCharCount >= nLineChar )
				{
					nLineCharCount = 0;
					xOrg = x;
					yOrg += m_Header.byFontWidth + m_nSpaceHeight;
				}
				else
				{
					xOrg += m_Header.byFontWidth << 1;
				}
				continue;
			}

			//字级裁剪
			if( _clipRect ? (!clip_font( _clipRect, xOrg, yOrg )) : 1 )
			{
				unsigned short*pFontChar = GetASCII(*p);
				unsigned short*pFontOutLine = GetASCIIOutLine(*p);

				for( register int m=0;m<fontHeight;m++ )
				{
					for( register int n=0;n<nCharSize;n++ )
					{
						if( (*pFontChar & C_FONT_MASK[n]) && ( _clipRect ? InClipRect(xOrg+n, yOrg+m,_clipRect ) : 1 ) )
						{
							pVertices[npts].x = float(xOrg+n);
							pVertices[npts].y = float(yOrg+m);
							pVertices[npts].color = dwColor;
							pVertices[npts].z = z;
							pVertices[npts].w = 1;
							if( ++npts >= m_nMaxPointsCount )
							{
								m_pVB->Unlock();

								pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( STD3DGraphicsVertex ) );
								pDevice->SetFVF( D3DFVF_D3DGraphicsVertex );
								pDevice->DrawPrimitive( D3DPT_POINTLIST, 0, npts );
								if( FAILED( m_pVB->Lock( 0, m_nMaxPointsCount*sizeof(STD3DGraphicsVertex), (void**)&pVertices, D3DLOCK_DISCARD ) ) )
								{
									return;
								}

								npts = 0;
							}
						}

						if( (*pFontOutLine & C_FONT_MASK[n] ) && ( _clipRect ? InClipRect(xOrg+n, yOrg+m,_clipRect ) : 1 ) )
						{
							pVertices[npts].x = float(xOrg+n);
							pVertices[npts].y = float(yOrg+m);
							pVertices[npts].color = dwOutLineColor;
							pVertices[npts].z = z;
							pVertices[npts].w = 1;
							if( ++npts >= m_nMaxPointsCount )
							{
								m_pVB->Unlock( );

								pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( STD3DGraphicsVertex ) );
								pDevice->SetFVF( D3DFVF_D3DGraphicsVertex );
								pDevice->DrawPrimitive( D3DPT_POINTLIST, 0, npts );
								if( FAILED( m_pVB->Lock( 0, m_nMaxPointsCount*sizeof(STD3DGraphicsVertex), (void**)&pVertices, D3DLOCK_DISCARD ) ) )
								{
									return;
								}

								npts = 0;
							}
						}
					}
					pFontChar++;
					pFontOutLine++;
				}
			}

			p++;
			nLineCharCount++;
			if( nLineCharCount >= nLineChar )
			{
				nLineCharCount = 0;
				xOrg = x;
				yOrg += fontWidth + m_nSpaceHeight;
			}
			else
			{
				xOrg += nCharSize + m_nSpaceWidth;
			}

		}
		else
		{
			//字级裁剪
			if( _clipRect ? (!clip_font( _clipRect, xOrg, yOrg )) : 1 )
			{
				unsigned short*pFontUnicode=GetUnicode(*p,*(p+1));
				unsigned short*pFontOutLine = GetUnicodeOutLine(*p,*(p+1));
				
				for(register int m=0;m<fontHeight;m++)
				{
					for(register int n=0;n<fontWidth;n++)
					{
						if( (*pFontUnicode & C_FONT_MASK[n]) &&  ( _clipRect ? InClipRect(xOrg+n, yOrg+m,_clipRect ) : 1 ) )
						{
							pVertices[npts].x = float(xOrg+n);
							pVertices[npts].y = float(yOrg+m);
							pVertices[npts].color = dwColor;
							pVertices[npts].z = z;
							pVertices[npts].w = 1;
							if( ++npts >= m_nMaxPointsCount )
							{
								m_pVB->Unlock( );

								pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( STD3DGraphicsVertex ) );
								pDevice->SetFVF( D3DFVF_D3DGraphicsVertex );
								pDevice->DrawPrimitive( D3DPT_POINTLIST, 0, npts );
								if( FAILED( m_pVB->Lock( 0, m_nMaxPointsCount*sizeof(STD3DGraphicsVertex), (void**)&pVertices, D3DLOCK_DISCARD ) ) )
								{
									return;
								}

								npts = 0;
							}
						}

						if( (*pFontOutLine & C_FONT_MASK[n]) &&  ( _clipRect ? InClipRect(xOrg+n, yOrg+m,_clipRect ) : 1 ) )
						{
							pVertices[npts].x = float(xOrg+n);
							pVertices[npts].y = float(yOrg+m);
							pVertices[npts].color = dwOutLineColor;
							pVertices[npts].z = z;
							pVertices[npts].w = 1;
							if( ++npts >= m_nMaxPointsCount )
							{
								m_pVB->Unlock( );

								pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( STD3DGraphicsVertex ) );
								pDevice->SetFVF( D3DFVF_D3DGraphicsVertex );
								pDevice->DrawPrimitive( D3DPT_POINTLIST, 0, npts );
								if( FAILED( m_pVB->Lock( 0, m_nMaxPointsCount*sizeof(STD3DGraphicsVertex), (void**)&pVertices, D3DLOCK_DISCARD ) ) )
								{
									return;
								}

								npts = 0;
							}
						}
					}
					pFontUnicode++;
					pFontOutLine++;
				}
			}

			p+=2;
			nLineCharCount+=2;
			if( nLineCharCount >= nLineChar )
			{
				nLineCharCount = 0;
				xOrg = x;
				yOrg += fontWidth + m_nSpaceHeight;
			}
			else
			{
				xOrg += fontWidth + m_nSpaceWidth;
			}
		}
	}
	m_pVB->Unlock( );

	if( npts > 0 )
	{
		pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( STD3DGraphicsVertex ) );
		pDevice->SetFVF( D3DFVF_D3DGraphicsVertex );
		pDevice->DrawPrimitive( D3DPT_POINTLIST, 0, npts );
	}
}
