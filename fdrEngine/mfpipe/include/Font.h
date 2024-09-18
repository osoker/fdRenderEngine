//////////////////////////////////////////////////////////////////////////////////////////
/** \file  
 *  Filename: font.h
 *  Desc:     ʵ��һ��������ʾ���ࡣ
 *  His:      ���� @ 2/4 2004 16:18]
 *
 *  "Ψ�е������Կ���ˣ����Ī����" 
 * 
 */
//////////////////////////////////////////////////////////////////////////////////////////
# pragma once


#include <vector>
#include <algorithm>
#include <d3d9.h>
#include <d3dx9.h>
//#include "../../common/include/sgdefine.h"
# include "../../interface/osInterface.h"

#define USE_PIXELFONT   1


//! ����������
/**
1.��osInterface �ļ��� E_FontType �м����µ�����
2.��gFontManager.Initialize����ע������.
3.ֱ�ӵ�����ʾ�����
*/
//{@
//! ����TTF������,���ͷ�ļ���������,���"Device:/.../TrueType"����Ϊϵͳͷ�ļ�Ŀ¼��


//}@

class CWFont;



class FontManager:public IFontManager
{
public:
	FontManager();
	virtual ~FontManager();
	void Initialize(LPDIRECT3DDEVICE9	pd3dDevice);
	virtual int  RegisterFont(const char* _fontName,int height,int flWeight= FW_LIGHT);
	CWFont* GetFont(int _fontType);
	void OnLostDevice();
	void OnRestoreDevice();
	void ReleaseFont();
protected:
private:
	int baseId;
	std::vector<CWFont*> mFontList;
	LPDIRECT3DDEVICE9	mpd3dDevice;
};
extern IFontManager* gFontMgrPtr;
extern FontManager gFontManager;
using namespace std;



//!���������fvf
#define D3DFVF_FONT_VER (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)


//
//! ��Ⱦ�����õ��Ķ���ĸ�ʽ.
struct FONT_VER
{
	D3DXVECTOR4 m_vPos;
	D3DCOLOR m_color;
	D3DXVECTOR2 m_fUV;
	void SetValue(D3DXVECTOR4 &_vPos,DWORD _color,D3DXVECTOR2 &_fUV)
	{
		m_vPos = _vPos;
		m_color = _color;
		m_fUV = _fUV;
	}
};

//===================================================================
/** \class  
 *  classname: CWFont
 *  Desc:     ʵ��һ��������ʾ����.
 * 
 *  ��Ա��
	HDC			m_hDc;			 �豸���
	HBITMAP		m_hBmp;			 λͼ���
	HFONT		m_hFont;		 ������
	UINT		m_iMaxTextureCharNum;			 ���������
	int			m_RowNum;		 һ�п������ɵ����ָ���
	DWORD *		m_pBits;		 λͼ������ָ��
	int			m_TextSize;		 ���ֵĴ�С
	int			m_TextureSize;	 ��������Ĵ�С
	vector <WORD> m_vBuf;		 ��¼�Ѿ�ʹ�õ�����
	LPDIRECT3DDEVICE8		m_pd3dDevice;	 D3D device
	LPDIRECT3DVERTEXBUFFER8 m_pVB;			 ���ֵĶ��㻺����
	LPDIRECT3DTEXTURE8		m_pTexture;		 �������ֵ�����
	DWORD			m_dwSaveState;			 ��������ԭ������Ⱦ״̬
	DWORD			m_iRenderState;		 ����������Ⱦ״̬       
 *  ������
	bool CreateFont( LPDIRECT3DDEVICE8 pd3dDevice, char* szFontName, int nSize = 16, int _CharTexSize = 512 );
	\param1		3D�豸
	\param2		��������
	\param3		�����С������ʹ��16��24����
	\param4		�����С�ȼ�������Ϊ1 -- 5.�ֱ�Ϊ1=64 2=128 3=256 4=512 5=1024��
				NOTE: 
				1.���ʹ��С��256��������ʱ����ʹ��֣�����뷨ʱ��ʱһ�����������������ڻ�����Ŀ��
				��������Ĭ�������СΪ256.���ھ�������Կ���˵����һ��256������Ӧ�ò�������ɣ�:)
				2.����ʹ��16�����С������,���Ҫʹ�ô�������Ըı�����ֵ
				3.��ͬ�������Сһ�ο�����Ⱦ����������Ҳ�ǲ�һ���ģ�64��С���������һ����Ⱦ������������16����
				128��С����64����256����256��,512��С��1024,1024��С��4096,����ԽС���ٶ�Խ�졣������˵��
				����16�����С�����壬�����24����Ļ�Ҫ��һЩ��
	return		�ɹ�����true;

	void Begin();
	desc:
		������Ⱦ״̬��׼����ʼ��Ⱦ����
	bool DrawText( char* szText, int x, int y, D3DCOLOR color = 0xFFFFFFFF,float fScale = 1.0f );
	\param1		Ҫ��Ⱦ������
				��NOTE:����Ⱦ�����ֵĸ������ܴ��ڻ�����Ŀ��
	\param2		��Ļ�ϵ�X����
	\param3		��Ļ�ϵ�Y����
	\param4		���ֵ���ɫ
	\param5		���Զ����ִ�С��������
	return		�ɹ�����true;

	void End();
	desc:	
		������Ⱦ��������ǰ����Ⱦ״̬

	void ReleaseFont();
	desc:
		�ͷ���Դ

	HRESULT OnLostDevice();
	desc:
		���豸��ʧ��ʱ��ʹ��

	HRESULT OnRestoreDevice();
	desc:	
		�޸��豸ʱʹ��

	bool SetRenderState();		
	desc:
		������Ⱦ״̬
	//void CharToTexture( char c1, char c2, int & tX, int & tY );
	\param1		���ֵĵ�һ���ֽ�
	\param2		���ֵĵڶ����ֽ�
	\param3		�������������x����
	\param4		�������������y����
	desc:
		�����ַ��������ڵ�����λ��
 */
//===================================================================
class CWFont
{
private:
	friend class osc_szCI;
	//int mCharWidth;	//!< ��ʼ�����������С һ��Ϊ16 ���Ϊ24����ʧ��
	int mTextLine;	//!< �����м�ࡡһ��Ϊ2
	//DEVICE_CHAR_WIDTH
public:
	CWFont();
	~CWFont();

	bool CreateFont( LPDIRECT3DDEVICE9 pd3dDevice, char* szFontName, 
		        int nSize = 12,int lfWeight = FW_LIGHT,int _CharTexSize = 512 );


	//! ��ʼ��Ⱦ���ֵı�����
	void begin_bgRender( void );

	//! ��Ⱦһ���ַ����ı���ɫ��
	bool drawTextBG( char* _szText,int _x,int _y,float _z = 0.9f,DWORD _color = 0,float _scale = 1.0f );

	//! ��ʼ��Ⱦ���֡�
	void Begin();

	bool DrawText( char* szText, int x, int OrigX, int y, float z = 0.9f,
					  D3DCOLOR color = 0xffffffff,  float fScale = 1.0f,
					  bool bShadow = false, D3DCOLOR ShadowColor = 0xb465523F, RECT* _clipRect = NULL  );


	void End();

	void ReleaseFont();

	HRESULT OnLostDevice();

	HRESULT OnRestoreDevice();



private:
	void clip_left( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  );
	void clip_right( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  );
	void clip_top( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   float nw,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  );
	void clip_bottom( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   float nw,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  );
	void clip_leftTop( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   float nw,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  );
	void clip_topRight( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices  );
	void clip_leftBottom( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices );
	void clip_rightBottom( RECT* _clipRect, float sx, float sy, float z,
					   float w, float h,
					   float tx, float ty,
					   float tx1,float tx2,float ty1,float ty2,
					   float tu, float tv,
					   D3DCOLOR color, D3DCOLOR ShadowColor,
					   FONT_VER* pVertices, FONT_VER* pShadowVertices );
private:

	//! �޽���Ⱦ�ļ�����ʱ�õ���RenderBlock,
	bool record_RBGState( void );

	//! �����ļ�����Ⱦʱ�õ���RenderBlock.
	void record_RenderState();
	void SetRenderState();	
	
	//! ��Ⱦ�ı���Ӱʱ�õ���״̬��
	void record_shadowState();
	void set_shadowState();


	//! 
	bool CharToTexture( char _c1, char _c2,int _idx,
		int & _tX, int & _tY,const char* _rstr );

	/** \brief
	 *  ��Ⱦ����ı���ɫ��
	 *
	 *  \param _xsize �᷽���϶�Ӧ���ַ���Ŀ���������ֵ�����Ҫ����
	 *                �ı���ͼƬ�Ĵ�С��
	 *  \param _color Ҫ���ı�������ɫ��
	 */
	void draw_textBGTri( float _x,float _y,float _z,int _xsize,DWORD _color );

	//! �������㻺����,����dip,��Ⱦ���еĻ��������ַ�����
	void vbText_dip( int _triangleNum,bool _bShadow );

private:
	HDC			m_hDc;			
	HBITMAP		m_hBmp;
	HGDIOBJ m_hOldBmp;			
	HFONT		m_hFont;
	HGDIOBJ	m_hOldFont;		
	UINT		m_iMaxTextureCharNum;			
	int			m_RowNum;		
	DWORD *		m_pBits;		

	int			m_TextSize;		
	int			m_TextureSize;	

	//! ��¼ÿһ���ַ������������ݵ��б�
	VEC_word    m_vBuf;					

	LPDIRECT3DDEVICE9		m_pd3dDevice;	
	LPDIRECT3DVERTEXBUFFER9 m_pVB;			
	LPDIRECT3DTEXTURE9		m_pTexture;	

	LPDIRECT3DVERTEXBUFFER9 m_pShadowVB;



	//@{
	//! River �޸�ʹʹ��dx9.0��stateBlock
	int                   m_iRenderState;
	//! �����ֱ���ʱ�õ���state.
	int                   m_iBGRState;
	//@}

	//! syq
	int                   m_iRenderShadow;


};

////////////////////////////////////////////////////////////////////////////////////////////////////
//

