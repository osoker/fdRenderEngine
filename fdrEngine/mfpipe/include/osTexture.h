//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osTexture.h
 *
 *   Desc:     O.S.O.K ��������������,���м������.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  ���ڱ�Ҫ��ʱ��һ��Ҫ�뷽�跨˵"��" ����
 */
//--------------------------------------------------------------------------------------------------------
# pragma    once

# include "../../interface/osInterface.h"

//! �����������ֵĳ���.
# define  TEXNAME_LENGTH   128


//! �����������ÿ������ͼ�������Ŀ��
# define  INIT_LMPVECSIZE    64
//! ����������ĳ�ʼ��
# define  INIT_TEXVECSIZE    768


/// osok���������ָ��,����ʹ�����������е���һ��.
union os_texPointer
{
	LPDIRECT3DTEXTURE9        commonTex;
	LPDIRECT3DCUBETEXTURE9    cubicTex;
	LPDIRECT3DVOLUMETEXTURE9  volumeTex;
};

class CVolumeFog;
/** \brief
 *  O.S.O.K�����õ�������ṹ�ķ�װ.
 *
 *  ��ÿһ��������й���,���ܲ�����ͨ���������������ǻ�������
 *  ���������ֻ��Ҫ������Щ�ṹ�Ϳ�����.
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

	/// ������������
	void             setPixels( const unsigned char *pBuffer );

	/// ��:format = a8r8g8b8,��return 32
	int              get_colorBitCount( void );

	void             dumptex(char* fname){ D3DXSaveTextureToFile(fname,D3DXIFF_BMP,get_texpointer(),NULL); }


	/// ʹ������������Գ�ʼ����cubic map����������
	bool             init_texture( char* _filename, LPDIRECT3DDEVICE9 _device, int _tex_format );

	/// �ͷ�����.
	bool             release_texture( void );

	//! ��������
	bool             reset_texture( void );
	//! �һ�����
	bool             restore_texture( LPDIRECT3DDEVICE9 _device );

	//! �õ��������ָ��
	LPDIRECT3DBASETEXTURE9 get_texpointer( void );              
	const char*            get_texname( void ) { return m_strTexname; }
	DWORD                  get_texnameHash( void ){ return m_dwTexNameHash;}

	//! ֻ�ܵõ�Format,��Ϊ�����ʼ��ʱ�Ѿ���������������Format��.
	int                    get_texformat( void ){ return m_iTexFormat ; }

	//! �õ���ǰ��tgaͼ�Ƿ���Alphaͨ����
	BOOL                   get_tgaAlpha( void ){ return m_bTgaAlpha; }

	//! River added @ 2009-6-10: �Ƿ��������,Ϊ���̵߳��봦��.
	bool                   m_bLoadLock;

protected:
	/// �ͷŵ�ǰ��Դ�Ĵ��麯������
	virtual void           release_obj( DWORD _ptr = NULL );


private:
	bool                   create_texture( const char *name,int w, int h, D3DFORMAT _format, LPDIRECT3DDEVICE9 _device, int _type =0 );

	os_texPointer       m_ptex;

	//!  ��ǰ���������.
	char                m_strTexname[TEXNAME_LENGTH];
	//! ��ǰ�������ֵ�hashVal.
	DWORD               m_dwTexNameHash;

	D3DXIMAGE_INFO      m_imageInfo;

	//!  ��ǰֻʹ�����ָ�ʽ:a8r8g8b8,a4r4g4b4.
	D3DFORMAT           m_format;
	int					m_iWidth;
	int					m_iHeight;

	//!  0:common tex.1:cubic tex.2:volume tex.
	int                 m_iTexFormat;     

	/** ��ǰ�����Ӧ��tgaͼ�Ƿ���alphaͨ�����������tgaͼ���϶�û�С�
	 *  ʹ�������Լ��������ʽ���������tga�ļ���������ֵΪFalse.
	 */
	BOOL                m_bTgaAlpha;

	//! �Ƿ�����Ҫ�ָ�������,�豸��ʧ֮��Ĵ���
	bool                m_bNeedRestore;

};


//-------------------------------------------------------------------------------


/** \brief
 *  ������Texture������,�����˰���Lightmap�ĳ����õ������е�����.
 *  
 *  lightMap�����ڳ����еر�ڶ����������������һ���ӿڣ��Ǹտ�ʼ
 *  д�ر��ʱ��Ϊ�˸���ǰ�ĸ�ʽ���ݣ����Ҫ�޸ĵĻ������Ը�����������
 *  ����һ��ʹ�á�
 *  
 *  TexManager�����е�������ʱ��ʹ��managed Texture,�������豸
 *  lost�ĺ�resetʱ������Ҫ���κεĴ���
 */
class osc_textureMgr
{
public:
	osc_textureMgr();
	~osc_textureMgr();


	/// ��ʼ����ǰ�����������.
	void                    init_textureMgr( LPDIRECT3DDEVICE9 _device );

	/** �ͷ�Texture Manager.
	* \param _finalRelease ��������һ���ͷţ�����ҪΪtexture�ٷ���ռ䡣
	*/
	void                    release_textureMgr( void );   

	/// �´���һ��������������
	int 					addnew_texture( const char *name,int w,int h,D3DFORMAT _format,int _type =0);

	/// �õ�һ������ͨ��ID
	osc_texture			*	get_textureById(int id);
    LPDIRECT3DBASETEXTURE9  get_texfromid( int _id );


	/** Dynamic add texture to texture manager.return the new add texture id.
	 */
	int                     dadd_textomanager( char* _filename, int _format );

	/// �Ը����������������.
	int                     release_texture( int _id );

	/// �õ�ָ���������Ƿ����м���alphaͨ��������
	bool                    is_alphaChannelTex( int _id );


	/// ��:format = a8r8g8b8,��return 32
	static int              get_colorBitCount( const D3DFORMAT& format );

	//! ����textureMgr.
	void                    reset_textureMgr( void );

	//! �һ�textureMgr.
	void                    restore_textureMgr( void );


private:
	LPDIRECT3DDEVICE9       m_pd3dDevice;     

	 /// dynamic load texture.
    CSpaceMgr<osc_texture>  m_vecDtexture;

	//! ��������ʱ�������Critical section.
	CRITICAL_SECTION        m_sCSGetFreeTexSpace;
};


//******************************************************************


/// �õ�ָ���������Ƿ����м���alphaͨ��������
inline bool osc_textureMgr::is_alphaChannelTex( int _id )
{
	return m_vecDtexture[_id].get_tgaAlpha();
}


inline osc_texture* osc_textureMgr::get_textureById( int id )
{
	guard;
	
	// River @ 2011-10-18:���ID�����⣬ֱ�ӷ�ӦNULL.
	//osassertex( (id>=0)&&(id<m_vecDtexture.size()),va( "The tex id:<%d>..\n",id) );
	if( id < 0 )
		return NULL;
	if( id >= m_vecDtexture.size() )
		return	NULL;

	// River mod @ 2010-7-28:ȷ���豸�����ʱ�򣬲�����
	if( m_vecDtexture.validate_id( id ) )
		return &(m_vecDtexture[id]);
	else
		return NULL;

	unguard;
}

//----------------------------------------------------

/// �ͷŵ�ǰ��Դ�Ĵ��麯������
inline void osc_texture::release_obj( DWORD _ptr/* = NULL*/ )
{
	release_texture();
}

/// ��:format = a8r8g8b8,��return 32
inline int osc_texture::get_colorBitCount( void )
{
	return osc_textureMgr::get_colorBitCount( m_format );
}

