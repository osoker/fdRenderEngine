//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osEffect.h
 *
 *   Desc:     O.S.O.K �����Effect������,���м������.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  ΢���Ļ�: ����֪������¿���������������.��������ǿ��������ģ���
 *    ô�����ôȥ���أ�����
 */
//--------------------------------------------------------------------------------------------------------
# pragma  once

# include "../../interface/osInterface.h"


# define    TEXTUREPTR               "Texture"


/** \brief
 *  D3DX EFFECT�İ�װ��,���е�Effectͨ���������в���.
 *
 *  fx�ļ��е�Texture��matrix��������ͳһ������Texture0��Texture1... Matrix0��Matrix1...
 */
class osc_effect : public os_refObj
{
	friend class osc_effectMgr;
	friend class osc_shaderMgr;

public:
	osc_effect( void );
	~osc_effect( void );


	/// ���õ�ǰEffect�õ��ĵ�n��texture
	bool            set_texture( int _idx,LPDIRECT3DBASETEXTURE9 _tex );
	bool			set_textureById( int _idx, int _texId );
	int				get_texCount( void ){ return (int)m_vTexHandle.size(); }

	/// ���õ�ǰEffect�õ��ĵ�n��matrix
	void			set_matrix( int _idx, const D3DXMATRIX& _mat );
	int				get_matCount( void ){ return (int)m_vMatHandle.size(); }
	int             get_float2Count( void ) { return (int)m_vFloat2Handle.size(); } 


	//! ����foat,float3,float4.
	void            set_float( const char* _name,float _val );
	void            set_value( const char* _name,void* _vec4,int _size );
	void            set_float2( int _idx,float* _ft );

	/// ����Ҫʹ�õ�technique��pass�ĸ���
	unsigned int    begin( void );
	void            end();
	/// ���ش�pass�Ƿ���virtual�͵�
	bool            pass( unsigned int index );
	//! end pass.
	bool            endPass( void );

	//! ����beginpass��endpass֮�����Ⱦ״̬�޸�
	void            commit_change( void );


	BOOL            is_forceTrans( void ) { return m_bForceTransparency; }


	//!  ��ǰEffect��ָ��.
	LPD3DXEFFECT    m_peffect;
	unsigned int	m_curTechLevel;
private:
	/// ��һ���ļ����豸ָ���ʼ��effect,ֻ����osc_effectMgr��ʹ��
	bool            init_effect( char* _filename, LPDIRECT3DDEVICE9 _device ); 

	/// �ͷ�һ��Effect,ֻ����osc_effectMgr��ʹ��
	void            release_effect( void );

	/// �ͷŵ�ǰ��Դ�Ĵ��麯��
	virtual void    release_obj( DWORD _ptr = NULL );

	bool            create_effect( char* _filename, LPDIRECT3DDEVICE9 _device );

	//!  ��ǰEffect��Ӧ��Effect�ļ���.
	s_string		m_strEffFileName;
	DWORD			m_dwHash;

	//  River mod @ 2007-4-9:
	//! �Ƿ�ǿ�ƴ�Effect��Ӧ��shader��Ϊ͸��������Ⱦ.
	BOOL            m_bForceTransparency;

	//! �滻��ǰҪ�����hlslShader�汾����ǰ����֧�ֵ�����shader�汾
	bool            update_toNewVS( BYTE* _data, LPDIRECT3DDEVICE9 _device,int _size );

	//! ���texture��handle����������ʱֻ����Ӧhandle���þͿ����ˣ�����ͨ��string����
	std::vector<D3DXHANDLE> m_vTexHandle;
	std::vector<D3DXHANDLE> m_vMatHandle;

	//  River @ 2010-4-3:�Ż���
	//! ���texture scale��handle,����Ҫʹ������matrix,ʹ��һ��float2�Ϳ��ԡ�
	std::vector<D3DXHANDLE> m_vFloat2Handle;

	struct Pass
	{
		Pass( void ):handle(0),isVirtual(true){}

		D3DXHANDLE handle;
		bool       isVirtual;
	};
	struct Technique
	{
		Technique( void ):handle(0){ m_vPassList.clear(); }

		D3DXHANDLE        handle;
		std::vector<Pass> m_vPassList;
	};

	std::vector<Technique> m_vTecList;

	/// �þ�̬�ı��������е�effect��pass flag �� technique����ͬ
	static DWORD           m_passFlag;
	static unsigned int    m_techniqueLevel;

public:
	//! �õ�effect��Name.
	const char*     get_effectName() { return this->m_strEffFileName.c_str(); } 

};

//----------------------------------------------------------------

/** \brief
 *  D3DX EFFECT�Ĺ�������,���е�Effect�����Ž������������.
 *
 *  EffectManager���������е�Shader�е�Effect,���Shader���Զ�Ӧͬһ��Effect,ֻ��Ҫ����ͬ
 */
class osc_effectMgr
{
public:
	osc_effectMgr();

	/// ��ʼ����ǰ��effectMgr
	bool              init_effectMgr( LPDIRECT3DDEVICE9 _device );

	/** �����е�effect fileȫ���ͷŵ�
	 *  \param _finalRelease ������������ͷţ�����ҪΪ�´�ʹ�÷����ڴ�
	 */
	bool              release_effectMgr( bool _finalRelease = true );  


	/// reset EffectMgrǰ��Ҫ�ͷŵ�����.
	void              eff_onLostDevice( void );
	/// Reset���е��豸��ص�Effect.
	bool              eff_onResetDevice( void );

	/// ��effect manager�еõ�Effect��ָ��.
	osc_effect*       get_effectfromid( int _id );


	/** ���������й����ж�̬�ļ���Effect
	 *  \return  �����¼����Effect��id,�������-1,����
	 */
	int               dadd_effecttomanager( char* _filename );

	/// ����һ��effect�ļ�������.
	int               release_effect( int _id );

	static void       set_passFlag( DWORD flag ){ osc_effect::m_passFlag = flag; }
	static void       set_techniqueLevel( unsigned int level ){ osc_effect::m_techniqueLevel = level; }

private:
	LPDIRECT3DDEVICE9    m_pd3dDevice;

	/** ���Ҫ��̬�ļ���Effect,���������effect Vector�ж�̬�ļ���.
	 *  �������Զ�̬�ļ���Effect,������ٵĻ�,��Ҫ�ı䶯̫��.
	 */
	CSpaceMgr<osc_effect>  m_arrEffectArray;
};


//==========================================================================




/// ����һ��effect�ļ�������
inline int osc_effectMgr::release_effect( int _id )
{
	/*
	// River @ 2011-2-24:������Ϸ��Effect���õ��ڴ����ޣ������ͷš�
	int t_iRef = 0;
	osassert( m_arrEffectArray.validate_id( _id ) );
	osassert( m_arrEffectArray[_id].get_ref()>0 );

	t_iRef = m_arrEffectArray[_id].release_ref();
	if( 0 == t_iRef )
		m_arrEffectArray.release_node( _id );
	*/

	return 1;
}


/// �ͷŵ�ǰ��Դ�Ĵ��麯��
inline void osc_effect::release_obj( DWORD _ptr/* = NULL*/ )
{
	//osDebugOut( "The <%s> effect release from Device..\n",m_strEffFileName.c_str() );
	release_effect();
}

inline void osc_effect::set_matrix( int _idx, const D3DXMATRIX& _mat )
{
	osassert( _idx < get_matCount() );
	m_peffect->SetMatrix( m_vMatHandle[_idx], &_mat );
}

inline void osc_effect::set_float2( int _idx,float* _ft )
{
	osassert ( _idx < get_float2Count() );
	m_peffect->SetFloatArray( m_vFloat2Handle[_idx],_ft,2 );
}

//! ����foat,float3,float4.
inline void osc_effect::set_float( const char* _name,float _val )
{
	osassert( m_peffect );
	m_peffect->SetFloat( _name,_val );		
}

inline void osc_effect::set_value( const char* _name,void* _vec4,int _size )
{
	osassert( m_peffect );
	HRESULT hr = m_peffect->SetValue( _name,_vec4,_size );
	if(FAILED(hr))
	{
		osassertex( false, va("����Effect����,ԭ��:%s(�ļ�[%s]�ֶ�[%s])\n",
			DXGetErrorDescription9(hr), m_strEffFileName.c_str(),_name)    );
	}
}



/// ����Ҫʹ�õ�technique��pass�ĸ���
inline unsigned int osc_effect::begin( void )
{
	osassert( m_peffect );
	osassertex( m_curTechLevel < m_vTecList.size(),
		va( "Tech level:<%d>,tec list size<%d>..\n",m_curTechLevel,m_vTecList.size() ) );
	m_peffect->SetTechnique( m_vTecList[m_curTechLevel].handle );
	UINT t;
	HRESULT   hr;
	hr = m_peffect->Begin( &t, this->m_passFlag );
	osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

	return t;
}
inline void osc_effect::end()
{
	m_peffect->End();
}

