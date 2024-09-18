//////////////////////////////////////////////////////////////////////////////////////////
/** \file  
 *  Filename: font.h
 *  Desc:     实现一个中文显示的类。
 *  His:      王凯 @ 2/4 2004 16:18]
 *
 *  "唯有德者能以宽服人，其次莫如猛" 
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


//! 如何添加字体
/**
1.在osInterface 文件中 E_FontType 中加入新的类型
2.在gFontManager.Initialize　后注册字体.
3.直接调用显示，完毕
*/
//{@
//! 加入TTF字体类,如果头文件包函错误,请把"Device:/.../TrueType"设置为系统头文件目录。


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



//!定义字体的fvf
#define D3DFVF_FONT_VER (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)


//
//! 渲染字体用到的顶点的格式.
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
 *  Desc:     实现一个中文显示的类.
 * 
 *  成员：
	HDC			m_hDc;			 设备句柄
	HBITMAP		m_hBmp;			 位图句柄
	HFONT		m_hFont;		 字体句柄
	UINT		m_iMaxTextureCharNum;			 缓冲最大数
	int			m_RowNum;		 一行可以容纳的文字个数
	DWORD *		m_pBits;		 位图的数据指针
	int			m_TextSize;		 文字的大小
	int			m_TextureSize;	 缓冲纹理的大小
	vector <WORD> m_vBuf;		 记录已经使用的文字
	LPDIRECT3DDEVICE8		m_pd3dDevice;	 D3D device
	LPDIRECT3DVERTEXBUFFER8 m_pVB;			 文字的顶点缓冲区
	LPDIRECT3DTEXTURE8		m_pTexture;		 包含文字的纹理
	DWORD			m_dwSaveState;			 用来保存原来的渲染状态
	DWORD			m_iRenderState;		 用来设置渲染状态       
 *  函数：
	bool CreateFont( LPDIRECT3DDEVICE8 pd3dDevice, char* szFontName, int nSize = 16, int _CharTexSize = 512 );
	\param1		3D设备
	\param2		字体名称
	\param3		字体大小（可以使用16或24点阵）
	\param4		纹理大小等级（可以为1 -- 5.分别为1=64 2=128 3=256 4=512 5=1024。
				NOTE: 
				1.如果使用小于256的纹理里时，在使用郑码输入法时有时一次输入的文字数会大于缓冲数目，
				所以我设默认纹理大小为256.对于绝大多数显卡来说创建一个256的纹理应该不是问题吧？:)
				2.尽量使用16点阵大小的字体,如果要使用大字体可以改变缩放值
				3.不同的纹理大小一次可以渲染的文字数量也是不一样的，64大小的纹理可以一次渲染的文字数量是16个，
				128大小的是64个，256的是256个,512大小是1024,1024大小是4096,纹理越小，速度越快。以上所说的
				都是16点阵大小的字体，如果是24点阵的还要少一些）
	return		成功返回true;

	void Begin();
	desc:
		设置渲染状态，准备开始渲染文字
	bool DrawText( char* szText, int x, int y, D3DCOLOR color = 0xFFFFFFFF,float fScale = 1.0f );
	\param1		要渲染的文字
				（NOTE:所渲染的文字的个数不能大于缓冲数目）
	\param2		屏幕上的X坐标
	\param3		屏幕上的Y坐标
	\param4		文字的颜色
	\param5		可以对文字大小进行缩放
	return		成功返回true;

	void End();
	desc:	
		结束渲染，返回以前的渲染状态

	void ReleaseFont();
	desc:
		释放资源

	HRESULT OnLostDevice();
	desc:
		在设备丢失的时候使用

	HRESULT OnRestoreDevice();
	desc:	
		修复设备时使用

	bool SetRenderState();		
	desc:
		设置渲染状态
	//void CharToTexture( char c1, char c2, int & tX, int & tY );
	\param1		文字的第一个字节
	\param2		文字的第二个字节
	\param3		返回所在纹理的x坐标
	\param4		返回所在纹理的y坐标
	desc:
		根据字符查找所在的纹理位置
 */
//===================================================================
class CWFont
{
private:
	friend class osc_szCI;
	//int mCharWidth;	//!< 初始化字体纹理大小 一般为16 大的为24不会失真
	int mTextLine;	//!< 字体行间距　一般为2
	//DEVICE_CHAR_WIDTH
public:
	CWFont();
	~CWFont();

	bool CreateFont( LPDIRECT3DDEVICE9 pd3dDevice, char* szFontName, 
		        int nSize = 12,int lfWeight = FW_LIGHT,int _CharTexSize = 512 );


	//! 开始渲染文字的背景。
	void begin_bgRender( void );

	//! 渲染一个字符串的背景色。
	bool drawTextBG( char* _szText,int _x,int _y,float _z = 0.9f,DWORD _color = 0,float _scale = 1.0f );

	//! 开始渲染文字。
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

	//! 修建渲染文件背景时用到的RenderBlock,
	bool record_RBGState( void );

	//! 创建文件字渲染时用到的RenderBlock.
	void record_RenderState();
	void SetRenderState();	
	
	//! 渲染文本阴影时用到的状态块
	void record_shadowState();
	void set_shadowState();


	//! 
	bool CharToTexture( char _c1, char _c2,int _idx,
		int & _tX, int & _tY,const char* _rstr );

	/** \brief
	 *  渲染字体的背景色。
	 *
	 *  \param _xsize 横方向上对应的字符数目，根据这个值，算出要画的
	 *                的背景图片的大小。
	 *  \param _color 要画的背景的颜色。
	 */
	void draw_textBGTri( float _x,float _y,float _z,int _xsize,DWORD _color );

	//! 解销顶点缓冲区,调用dip,渲染所有的缓冲区内字符顶点
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

	//! 记录每一个字符缓冲区内内容的列表
	VEC_word    m_vBuf;					

	LPDIRECT3DDEVICE9		m_pd3dDevice;	
	LPDIRECT3DVERTEXBUFFER9 m_pVB;			
	LPDIRECT3DTEXTURE9		m_pTexture;	

	LPDIRECT3DVERTEXBUFFER9 m_pShadowVB;



	//@{
	//! River 修改使使用dx9.0的stateBlock
	int                   m_iRenderState;
	//! 画文字背景时用到的state.
	int                   m_iBGRState;
	//@}

	//! syq
	int                   m_iRenderShadow;


};

////////////////////////////////////////////////////////////////////////////////////////////////////
//

