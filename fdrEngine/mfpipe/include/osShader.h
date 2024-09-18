//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osShader.h
 *
 *   Desc:     O.S.O.K �����shader������,���м������.�������ֱ�ӹ�����
 *             osc_textureMgr&osc_effectMgr������������.
 *
 *   His:      River Created @ 4/28 2003
 *
 *  �� Go VEGAN and EXERCISE like hell - it WILL make you SMARTER ����
 */
//--------------------------------------------------------------------------------------------------------
# pragma   once

# include "../../interface/osInterface.h"

class osc_textureMgr;
class osc_effectMgr;
class osc_effect;


/** O.S.O.K�����Shader��.
 *
 *  O.S.O.K������ÿһ���涼�����Ӧһ��Shader,һ��shader
 *  ��Ӧ����Ⱦ���������Ҫ�����е���Ⱦ״̬��Ϣ.�������ʵ
 *  ֻ��һ���ṹ,����������������Ķ������ϵ���һ��,����Ⱦ
 *  ������ʹ��.
 */
class osc_shader : public os_refObj 
{
	friend class osc_shaderMgr;
public:
	osc_shader( void );

	void frame_move( void );

private:
	void set_name( const char* name );
	const osc_shader& operator = ( const osc_shader& sha );

	//! ��ǰ SHADER������.
	s_string     m_strShadername;
	DWORD        m_dwHash;

	//! Effect��Ӧ��effect�������е�ID.
    int          m_iEffid;



	//! ��������,�����е�ֵ����ݲ����Զ��仯
	struct AnimMat
	{
		int			type;
		D3DXMATRIX	mat;
		float		angle;
		float       speed;
		osVec3D     center;

		//! timeΪ�ܹ���ȥ��ʱ��
		void anim( float time );
	};
	std::vector<AnimMat> m_vMatrix;

	//! �ɴ�֡����������
	struct FrameTex
	{
		FrameTex( void ):curIndex(0),timing(0.f){}

		std::vector<int> texList;
		s_string         texDir;

		float            speed;

		//! timeΪ��֡���õ�ʱ��
		void   anim( float time );
		int    get_curTexture( void )
		{
			return texList[curIndex]; 
		}

	private:
		int              curIndex;
		float            timing;
	};
	std::vector<FrameTex> m_vTexture;


	/** \brief
	 *  ����shaderʱʹ�õ�ShaderTemplate����.
	 *
	 *  ��ʹ����ͬ��shader��������в�ͬTemplate������Ҳ��Ҫ������ͬ��Shader.
	 */
	int          m_iTemplateIdx;

	float		 m_fEffectAlphaVal;


protected:
	virtual void release_obj( DWORD _ptr = NULL );
};


//! ��ʼ�������г����е�shader��Ŀ
//  River @ 2010-4-7:�޸�Ϊ768,��ʼ�������shader��Ŀ
# define INIT_SHADERVECSIZE       768

//! shader�������������ע���ģ��shader��Ŀ
# define MAX_REGISTERSHADER	      24


//! ע��һ��Shader�õ������ݽṹ
struct os_shaderRegister
{
	//! ��ע��shader��Ӧ��effect�ļ���.
	char        m_strEffectName[256];

	//! ��ע��shader��Ӧ��Ĭ������Ŀ¼��.
	char        m_strDefaultDir[256];

	/** \brief
	 *  ���ڵ�Ŀ¼�����˴�����ע��һ��ȫ�ֵĻ�����������
	 *  ����g_szMapName,����ʹ�������Ŀ¼����̬�ĸı䡣
	 */
	char        m_szAdjDirName[128];

public:
	os_shaderRegister(){ 
		m_strEffectName[0] = NULL;m_strDefaultDir[0] = NULL;m_szAdjDirName[0] = NULL; 
	}
};


/** \brief 
 *  osok�����shader��������,�����������������й�������Ҫ�����е�Shader.
 *
 * shader mgr����Ҫ����texture��Ϣ��effect��Ϣ.��Ҫ��texture   
 * mgr��effect mgr�еõ���Ⱦʱ���õ���Ϣ.shader mgr����
 * resource mgr�е�һԱ,�����ڴ漶��ĳ�Ա.texture&effect
 * mgr�����豸��ص�ʵ��,��Ҫ���м�ܵ������.
 *
 * �м�ܵ�ֻ��Ҫ��ShaderMgr�򽻵��Ϳ�����,����shaderMgr��TexMgr&EffMgr�򽻵�.
 *
 * ShaderManager�Ĺ�������:
 * �м�ܵ�һ�����Ĵ���shader.���м�ܵ�flush��Դ��ʱ��.
 * ShaderMgr��ʼ�Լ����ڲ���Դ�Ĵ���:
 * 1: �ȴ�shader�����ж�������Ҫ������shader����Ϣ.
 * 2: ����shader����������е�texture��Texture Manager.
 * 3: ����shader����������е�effect��Effect Manager.
 * 
 * ���������й��̵Ķ�̬����shader��flushʱ�Ĵ���˳����ͬ.
 *
 * River @ 2004-7-17:
 * Ӧ�ÿ�����ȫ���ͷ�shaderMgr,��������һ����ȫ��ͬ�ĳ����ĵ��룬������
 * ��̬�ĵ��볡��һ�����û����ֵ���Դ��
 * shaderMgr������TextureMgr&EffMgr���ͷź͵��빤����
 */
class osc_shaderMgr
{
public:
	osc_shaderMgr( void );
	~osc_shaderMgr( void );

	/// ��ʼ�����ǵ�ShaderMgr,ʹ��shaderMgr�п��õ�3d Device
	void             init_shaderMgr( LPDIRECT3DDEVICE9 _device );

	/** �ͷ�shader manager��ȫ������Դ,�������ʹ��shader mgr��Ϊ�������ȫ��ģ��
	 *  \param _finalRelease �Ƿ������ͷ�shader��������
	 */
	void             release_shaderMgr( bool _finalRelease = true );

	/// reset ShaderMgrǰ��Ҫ�ͷŵ�����.
	void             shaderMgr_onLostDevice( void );

	/// ���µ���shader�����е��豸�����Դ,�ϲ�Reset�豸����Ҫ�����������
	void             shaderMgr_onResetDevice( LPDIRECT3DDEVICE9 _device );

	void             frame_move( void );


	//------------------------------------------------

	/** ��̬�ļ���shader.
	 *
	 *  ֱ��release manager��ʱ���release this shader.�����¼����shader��id.
	 *  ���Ǵ���shader����Ҫ�ĺ���.
	 * 
	 *  \param  _shadername   Ҫ�����Shader������.  
	 *  \param  _inifilename  Ҫ�����shader���ڵ��ļ�������.
	 *  \param  _defaultS     �����ֵС����,������ҵ���Ӧ��shadderIni�ļ�����.
	 *                        �����ֵ���ڵ�����,��Ӽ���Ĭ�ϵ�shaderIni�в��ҳ�
	 *                        ��ǰ��Ҫ��shaderIni�ļ�.
	 *
	 *   �㷨����:
	 *   �������shader�ǵ���׶�,�򲻴���,ֻ�������е�����.
	 *   ��������������н׶�,����ʱ�����������Shader.
	 *   ʹ������ӿڴ���������Ҫ�����е�shader�����Դ��
	 */
	int              add_shadertomanager( char* _shadername, char* _inifilename,int _defaultS = -1 );

	//! ��ϵͳ��ע��shaderģ��
	int              register_shaderTemplate( const char* _effname,	const char* _defaultDirName,char* _adjDirName = NULL );

	/** ��̬���ͷ�һ��shaderռ�õ���Դ.
	 *  \param bool _frelease �Ƿ���ȫ���ͷŵ�ǰ��shader,���Ϊ��,���ͷ�shader������0,
	 *                        Ȼ���ͷŵ�ǰ��shader.
	 */
	int              release_shader( int _id,bool _frelease = false );

	//! ȷ��shaderId�Ŀ����ԡ�
	bool			assert_shaderId( int _shaid,bool _assert = true );


	//! ͨ��xml�ļ����ļ��е�һ��shader�ṹ������һ��shader
	int				add_shader( const char* _name, const char* _file ,const char* _defaultname = "default");



	//--------------------�����ǶԾ���ĳһ��shader�Ĳ���--------------------

	/// �õ�ĳ��shader��Ӧ�ĵ�һ�������tga�ļ��Ƿ���alphaͨ��
	bool                    has_alphaChannel( int& _shaid );

	/// ��Ҫ����effect����Ҫ��texture&other info
	bool                    set_EffectInfo( int _id );
	void                    update_shader( int _id );


	/// ͨ��shader id �õ������õ� effect �� id
	int				        get_effectId( int _shaid );
	osc_effect*             get_effect( int _shaid );


	//! ��Ҫ����IndexedBlend��SkinMesh����Ⱦ����Ҫֱ�ӵõ�texture
	//! �õ���ǰshader�õ��ĵ�һ�������ָ�롣
	LPDIRECT3DTEXTURE9      get_shaderTex( int _id );

	//! �õ���ǰshader��refֵ��
	int                     get_shaderRef( int _id );



	//----------------------------------------
	int                     create_texture( char* _texFile, int _format = 0 );
	void                    release_texture( int _texId );
	LPDIRECT3DBASETEXTURE9  get_texturePtr( int _texId );


	//-----------------------------------------

	osc_effectMgr	*		get_effectMgr( void )	{ return m_effMgr; }
	osc_textureMgr	*		getTexMgr( void )		{ return m_texMgr;}

	//  River added @ 2006-8-24:���ڳ��׵��滻���ĵĵ�ͼ
	//! ����һ��Shader��ĳһ������
	void                    set_shaderTexture( int _shaderId,int _idx,int _texId );


private:
	/** ���������й�������shaderMgr�м���shader.
	 *  ��Ҫ��������shader���ݺ�shader�а�����Texture&Effect����.
	 */
	int                   dadd_shadertomanager( char* _shadername, char* _inifilename, int _defaultS );  

	bool                  create_shaderFromINI( int _shaId, char* _shaname, char* _ininame, int _ds );

	/// ��һ��shader�����ֵõ�һ��Ĭ�ϵ�shader�ṹ, �����ж��ֵ�Ĭ��Shader
	bool                  creat_shaderFromTemplate( int _shaId, char* _shaname, int _dtype = 0 );

	//---------------------------------------

	bool                  set_effect( osc_shader* sha, const char* _effFile );
	bool                  set_texture( osc_shader* sha, int _idx, int frame, const char* _texFile, int _format );


	//! ͨ��xml�ļ����ļ��е�һ��shader�ṹ������һ��shader
	int	                  _add_shader( const char* _name, const char* _file, const char* _realName );

	//! ͨ�����е�shader��_shaId���͵�һ������_tex1_file������һ���µ�shader��_name��
	int                   _add_shader( const char* _realName, int _shaId, const char* tex1_file );

	bool                  load_effect( TiXmlElement* shaElem, osc_shader* sha );
	bool                  load_texture( TiXmlElement* shaElem, osc_shader* sha, osc_effect* eff );
    bool                  load_matrix( TiXmlElement* shaElem, osc_shader* sha, osc_effect* eff );

	bool                  get_realShaderName( const char* _name, const char* _file, char* buf );
	int                   get_shader( const char* _name );


	osc_textureMgr  *     m_texMgr;
	osc_effectMgr   *     m_effMgr;
	CSpaceMgr<osc_shader> m_vecShader;
	unsigned char         m_vecIsFramed[INIT_SHADERVECSIZE];

	LPDIRECT3DDEVICE9     m_pd3dDevice;
	os_shaderRegister     m_arrShaderTemplate[MAX_REGISTERSHADER];


};


/** \brief
 *  ��meshManager��characterManager��ʹ�����shaderManager,ֻ�����ж˹ܵ�����
 *  ʹ�����ȫ�ֱ���,��̫��ѧ���÷� :-)
 */
extern osc_shaderMgr*    g_shaderMgr;


//! River @ 2009-6-29:��ĳһ��ʱ�ڵ���Ⱦ�ڣ�ǿ��ʹ��ĳһ��shader.
//! ��ʹ����ȷ���������shader��fvfһ��,���磬��λ�ˮ���ʱ��
//�����ܻ�ʹ�ø��Ӷ���ȫ��һ����shader.
extern BOOL   g_bForceUseCustomShader;
extern int    g_iForceCustomShaderIdx;



//================================================================================================================
//================================================================================================================
//
//   inline����.
//
//================================================================================================================
//================================================================================================================

inline void osc_shader::set_name( const char* name )
{
	m_strShadername = name;
	m_dwHash = string_hash( m_strShadername );
}

//---------------------------------------------------------

//! ��ϵͳ��ע��shaderģ��
inline int osc_shaderMgr::register_shaderTemplate( const char* _effname,
	            const char* _defaultDirName,char* _adjDirName/* = NULL*/ )
{
	for( int t_i=0;t_i<MAX_REGISTERSHADER;t_i ++ )
	{
		if( 0 == strlen( m_arrShaderTemplate[t_i].m_strEffectName )  )
		{
			strcpy( m_arrShaderTemplate[t_i].m_strDefaultDir,_defaultDirName );
			strcpy( m_arrShaderTemplate[t_i].m_strEffectName,_effname );
			
			if( _adjDirName )
			{
				osassert( strlen( _adjDirName ) < 128  );
				strcpy( m_arrShaderTemplate[t_i].m_szAdjDirName,_adjDirName );
			}

			return t_i;
		}
	}

	osassertex( false,"ϵͳ��Ԥ����shaderTemplate�ռ䲻��..\n" );
	return -1;
}


/// ͨ��shader id �õ������õ� effect �� id
inline int osc_shaderMgr::get_effectId( int _shaid )
{
	osassert( m_vecShader.validate_id( _shaid ) );
	return m_vecShader[_shaid].m_iEffid;
}




inline void osc_shaderMgr::update_shader( int _id )
{
	if( !m_vecIsFramed[_id] )
	{
		m_vecShader[_id].frame_move();
		m_vecIsFramed[_id] = 1;
	}
}
