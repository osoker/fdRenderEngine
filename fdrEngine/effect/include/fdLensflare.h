//===================================================================
/** \file  
* Filename :   fdLensflare.h
* Desc     :   
* His      :   Windy create @2005-9-14 14:34:05
*/
//===================================================================
#ifndef __FDLENSFLARE_H__
#define __FDLENSFLARE_H__
class os_Lensflare 
{
public:
	I_fdScene*			m_ptrScene;
	//LPDIRECT3DDEVICE9	m_pD3DDevice;
//	D3DVIEWPORT9		m_ViewPort;
	//!view ,project ,world mat
	//D3DXMATRIX			m_vMat ,m_pMat,m_wMat;
	int					m_viewWidth,m_viewHeight;
	bool				m_IsRender;
	std::string			m_fname;

	os_Lensflare();
	~os_Lensflare();


	///��ʼ��
	bool init();
	void clear();
	/// ���� lensflare ��λ��.
	void setPosition(const D3DXVECTOR3& nPos)  {  m_SunRealPos = nPos;  }

	///���� lensflare
	void loadStdFlare(const char * _fname);

	/// ��Ⱦ Lensflare.
	void render(I_camera * pCam,int step);


	/// ���� lensflare �� alpha ֵ.
	/**
	*  alpha ֵԽ�ͣ�Խ͸������ alpha = 0.0 ʱΪȫ͸��
	*  _Alpha �ķ�ΧΪ [0.0-1.0]
	*/
	void setBaseAlpha(float _Alpha)  {  m_baseAlpha = _Alpha;  }
	///�Ƿ���Ⱦ
	
	///�����Ǳ༭���Ľӿ�
	int	 get_partNum();
	void addOnePart();
	void delOnePart(int iPartID);
	void save_file(const char * _fname);

	void set_pos(int iPartID,const float & _fpos);
	void get_pos(int iPartID,float & _fpos);

	void set_texid(int iPartID,const int & _texID);
	void get_texid(int iPartID,int & _texID);

	void get_size(int iPartID,float & _size);
	void set_size(int iPartID,const float & _size);

	void set_color(int iPartID,const D3DXCOLOR & _color);
	void get_color(int iPartID,D3DXCOLOR & _color);

	void set_alpha(int iPartID,const float & _alpha);
	void get_alpha(int iPartID,float & _alpha);



protected:
	typedef class _FlareDef{
	public:
		float	m_fPos;
		int		m_nIndex;
		float	m_fSize;
		float	m_fRed;
		float	m_fGreen;
		float	m_fBlue;
		float	m_fAlpha;
		_FlareDef():m_fPos(0.0f),m_nIndex(0),m_fSize(0),m_fRed(0.0f),m_fGreen(0.0f),m_fBlue(0.0f),m_fAlpha(0.0f){}
		~_FlareDef(){}
	}FlareDef;

	std::vector<FlareDef>		m_FlareDefList;
	std::vector<int>			m_FlareTexIDList;
	char                        m_vecFlareTexName[24][128];

	void setVertsPosSize(float _fPosX, float _fPosY, float _fSize);
	void setVertsColor(float _fRed, float _fGreen, float _fBlue, float _fAlpha);

	void drawFlarePart(FlareDef & _flareDef,int step);
	void drawFlarePart(float _fPos, int _nIndex, float _fSize, float _fRed, float _fGreen, float _fBlue, float _fAlpha=1.0f);

	int							m_numFlareImages;
	
	
	///
	os_screenPicDisp			m_srcPic;
	
	D3DXVECTOR3					m_SunRealPos;
	D3DXVECTOR2					m_screenPos;
	float						m_baseAlpha;
	float						m_ClipSize;
};


#endif //__FDLENSFLARE_H__