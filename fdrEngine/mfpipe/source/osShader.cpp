//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osShader.cpp
 *
 *   Desc:     O.S.O.K �����shader������,���м������.
 *
 *   His:      River Created @ 4/28 2003
 *
 *  ����֮�ɣ��ǳ��ڳ�֮�գ��Ǳ��������𣻻�֮������������֮�գ�����������ס���
 *  
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osShader.h"
# include "../include/osTexture.h"
# include "../include/osEffect.h"
# include "../../interface/miskFunc.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../../../common/include/tinyxml.h"

#define TEXTURENUM   "TEXTURENUM"
#define TEXNAMESTR   "TEX00"
#define TEXFORMAT    "TEXFOR"
#define EFFECTNAME   "EFFFILE"


# define EFFECT_ALPHAVAL  "AlphaVal"

//! River @ 2009-6-29:��ĳһ��ʱ�ڵ���Ⱦ�ڣ�ǿ��ʹ��ĳһ��shader.
//! ��ʹ����ȷ���������shader��fvfһ��,���磬��λ�ˮ���ʱ��
//�����ܻ�ʹ�ø��Ӷ���ȫ��һ����shader.
BOOL   g_bForceUseCustomShader = false;
int    g_iForceCustomShaderIdx = -1;


//---------------------------------------------

osc_shader::osc_shader()
{
	m_iEffid = -1;
	m_dwHash = 0;
	m_iTemplateIdx = -1;
	m_fEffectAlphaVal = 1.0f;
}

const osc_shader& osc_shader::operator = ( const osc_shader& sha )
{
	guard;

	size_t i, j;
	set_name( sha.m_strShadername.c_str() );
	this->m_iEffid = sha.m_iEffid;
	osc_effect* eff = g_shaderMgr->get_effectMgr()->get_effectfromid( m_iEffid );
	if( eff )eff->add_ref();

	osc_texture* tex;
	m_vTexture.clear();
	m_vTexture.resize( sha.m_vTexture.size() );
	for( i = 0; i < m_vTexture.size(); ++i )
	{
		m_vTexture[i].texList.clear();
		m_vTexture[i].texList.resize( sha.m_vTexture[i].texList.size() );
		for( j = 0; j < m_vTexture[i].texList.size(); ++j )
		{
			m_vTexture[i].texList[j] = sha.m_vTexture[i].texList[j];
			if( m_vTexture[i].texList[j] >= 0 )
			{
				tex = g_shaderMgr->getTexMgr()->get_textureById( m_vTexture[i].texList[j] );
				if( tex )tex->add_ref();
			}
		}
		m_vTexture[i].texDir = sha.m_vTexture[i].texDir;
	}

	//m_vMatrix.clear();
	m_vMatrix.resize( sha.m_vMatrix.size() );
	for( i = 0; i < (int)m_vMatrix.size(); ++i )
		this->m_vMatrix[i] = sha.m_vMatrix[i];

	this->m_iTemplateIdx = this->m_iTemplateIdx;

	this->add_ref();

	return *this;

	unguard;
}

void osc_shader::AnimMat::anim( float time )
{
	guard;

	switch( type )
	{
	case 0:	// ���Զ��仯�����ⲿ���ò���
		break;
	case 1:	// ��ת
		{
			float a = time * speed;
			D3DXMatrixRotationYawPitchRoll( &mat, 0.0, 0.0, a );
			float cosa_1 = 1.f - cos( a );
			float sina = sin( a );
			mat._31 = cosa_1 * center.x + sina * center.y;
			mat._32 = cosa_1 * center.y - sina * center.x;
			mat._33 = center.z;
		}
		break;
	case 2: // ҡ��
		{
			float a = angle * sin( time * speed );
			D3DXMatrixRotationYawPitchRoll( &mat, 0.0, 0.0, a );
			float cosa_1 = 1.f - cos( a );
			float sina = sin( a );
			mat._31 = cosa_1 * center.x + sina * center.y;
			mat._32 = cosa_1 * center.y - sina * center.x;
			mat._33 = center.z;
		}
		break;
	case 3:	// ƽ�� wrap
		{
			float a = time * speed;
			mat._31 =  cosf( angle ) * a;
			mat._32 =  sinf( angle ) * a;
		}
		break;

	case 4: // view matrix
		memcpy( &mat, &g_matView, sizeof(osMatrix) );
		break;

	case 5: // proj matrix
		memcpy( &mat, &g_matProj, sizeof(osMatrix) );
		break;

	case 6: // viewProj
		osMatrixMultiply( &mat, &g_matView, &g_matProj );
		break;

	default:// ��λ����
		D3DXMatrixIdentity( &mat );
		break;
	}

	unguard;
}

void osc_shader::FrameTex::anim( float time )
{
	guard;

	if( texList.size() == 0 )return;

	timing += time;
	if( timing > speed )
	{
		++curIndex;
		if( curIndex >= (int)texList.size() )
			curIndex = 0;
		timing = 0;
	}

	unguard;
}

void osc_shader::frame_move( void )
{
	guard;

	size_t t_i = 0;
	float t_fTime = sg_timer::Instance()->get_talelatime();
	for( t_i = 0; t_i < (int)m_vMatrix.size(); ++t_i )
	{
		m_vMatrix[t_i].anim( t_fTime );
	}
	t_fTime = sg_timer::Instance()->get_lastelatime();
	for( t_i = 0; t_i < m_vTexture.size(); ++t_i )
	{
		m_vTexture[t_i].anim( t_fTime );
	}

	unguard;
}

void osc_shader::release_obj( DWORD _ptr/* = NULL*/ )
{
	m_dwHash = 0;
	m_strShadername = "";
}


//--------------------------------------------------------------


/** \brief
 *  ��meshManager��characterManager��ʹ�����shaderManager,ֻ�����ж˹ܵ�����
 *  ʹ�����ȫ�ֱ���,��̫��ѧ���÷� :-)
 */
osc_shaderMgr*    g_shaderMgr;

bool osc_shaderMgr::set_texture( osc_shader* sha, int _idx, int frame, const char* _texFile, int _format )
{
	guard;

	osassert( _idx < (int)sha->m_vTexture.size() );
	osassert( frame < (int)sha->m_vTexture[_idx].texList.size() );

	// windy mod �е�������� g_refMap.texture �ǲ����ڵĵ���
	char    temp[128];
	int		t_id;
	strcpy( temp, _texFile );

	t_id = m_texMgr->dadd_textomanager( temp, _format );
	sha->m_vTexture[_idx].texList[frame] = t_id;

	if( t_id < 0 )
	{
		char temp2[250];
		sprintf(temp2,"[!]�򲻿������ļ�:[%s]",temp);
		osassertex(false,temp2);
		//osDebugOut( "[!]�򲻿������ļ���%s!\n", temp );
		//MessageBox( NULL,temp,"�򲻿������ļ�",MB_OK ); 
		osassert( false );
	}

	return true;

	unguard;
}

bool osc_shaderMgr::set_effect( osc_shader* sha, const char* _effFile )
{
	guard;

	int t_id;

	t_id = m_effMgr->dadd_effecttomanager( (char*)_effFile );
	if( t_id>=0 )
		sha->m_iEffid = t_id;

	if( t_id < 0 )
	{
		osassert( false );
		return false;
	}

	osc_effect* eff = m_effMgr->get_effectfromid( t_id );
	osassert( eff );

	sha->m_vTexture.resize( eff->get_texCount() );

	for( size_t i = 0; i < sha->m_vTexture.size(); ++i )
	{
		sha->m_vTexture[i].texList.resize( 1 );
		//! river mod @ 2009-5-26:��ʼ��textureIDΪ-1
		sha->m_vTexture[i].texList[0] = -1;
	}

	sha->m_vMatrix.resize( eff->get_matCount() );

	return true;

	unguard;
}


/**  \brief
*   ��һ��shader�����ֵõ�һ��Ĭ�ϵ�shader�ṹ
*/
bool osc_shaderMgr::creat_shaderFromTemplate( int _shaId, char* _shaname, int _dtype/* = 0*/ )
{
	guard;

	s_string             t_texname,t_str;

	osassert( _dtype >= 0 );
	osc_shader* t_shader = &( m_vecShader[_shaId] );

	osassert( strlen( m_arrShaderTemplate[_dtype].m_strEffectName ) > 0 );
	set_effect( t_shader, m_arrShaderTemplate[_dtype].m_strEffectName );

	// �õ����������
	t_str = m_arrShaderTemplate[_dtype].m_strDefaultDir;
	if( m_arrShaderTemplate[_dtype].m_szAdjDirName[0] )
		t_str += m_arrShaderTemplate[_dtype].m_szAdjDirName;
	t_str += _shaname;

	// ������Ԥ����׶���ɡ������е���ĸ��ΪСд
	std::transform( t_str.begin(), t_str.end(), t_str.begin(), tolower );
	t_texname = t_str + ".hon";
	
	// ȷ�������Ƿ����.
	set_texture( t_shader, 0, 0, t_texname.c_str(), 0 );
	return true;

	unguard;
}


osc_shaderMgr::osc_shaderMgr( void )
{
	m_texMgr = NULL;
	m_effMgr = NULL;

	g_shaderMgr = this;

	m_vecShader.resize( INIT_SHADERVECSIZE );
	memset( m_vecIsFramed, 0, INIT_SHADERVECSIZE );
}

osc_shaderMgr::~osc_shaderMgr( void )
{
	SAFE_DELETE( m_texMgr );
	SAFE_DELETE( m_effMgr );
}


/** \brief 
*  �ͷ�shader manager��ȫ������Դ,
*
*  �������ʹ��shader mgr��Ϊ�������ȫ��ģ��.
* 
*  ��Ҫ���ͷ�textureMgr��effectMgr����Դ.
*/
void osc_shaderMgr::release_shaderMgr( bool _finalRelease/* = true */ )
{
	if( m_texMgr )
		m_texMgr->release_textureMgr();
	if( m_effMgr )
		m_effMgr->release_effectMgr( _finalRelease );
}

/** \brief
*  ��̬�ļ���shader.
*
*  ֱ��release manager��ʱ���release this shader.�����¼����shader��id.
*  ���Ǵ���shader����Ҫ�ĺ���.
* 
*  \param  _shadername   Ҫ�����Shader������.  
*  \param  _inifilename  Ҫ�����shader���ڵ��ļ�������.
*
*   �㷨����:
*   �������shader�ǵ���׶�,�򲻴���,ֻ�������е�����.
*   ��������������н׶�,����ʱ�����������Shader.
*   
*   �ϲ㴴��shaderʹ�õ�Ψһ�ӿ�.
*/
int osc_shaderMgr::add_shadertomanager( char* _shadername, char* _inifilename,int _defaultS/*= -1*/ )
{
	guard;

	int    t_idx = -1;
	t_idx = dadd_shadertomanager( _shadername,_inifilename,_defaultS );
	return t_idx;

	unguard;
}


/** \brief
*  ��һ��effect����Ҫ���õĶ�����ȫ������.
*  
*  ��Ҫ����effect����Ҫ��texture&other info,�����ǵ��õõ�
*  shader effect�ĺ���ʱ,��Ҫ����shaderMgr�ڲ������������.
*  Ŀǰֻ��Ҫ����texture�Ϳ�����.
*
*  �㷨����: 
*  1: ��effectMgr�еõ�Ҫ����Effect����Ϣ.
*  2: ���õõ�Effect��Ҫ����.��
*/
bool osc_shaderMgr::set_EffectInfo( int _id )
{
	guard;

	size_t         			t_i;
	bool          			t_b;
	osc_shader*   			t_shader;
	LPDIRECT3DBASETEXTURE9  t_tex;
	osc_effect*             t_eff;

	osassert( _id >= 0 );
	osassert( m_vecShader.validate_id( _id ) );

	t_shader = &m_vecShader[_id];
	update_shader( _id );
	
	//---- �������� -------
	t_eff = m_effMgr->get_effectfromid( t_shader->m_iEffid );

	for( t_i=0; t_i < t_shader->m_vTexture.size(); ++t_i )
	{
		//@{
		//  River mod @ 2006-7-18: �����ϲ�ֱ������Effect�ڵ������������˲�
		if( 0 == t_shader->m_vTexture[t_i].texList.size() )
			continue;
		//@}

		t_tex = m_texMgr->get_texfromid( t_shader->m_vTexture[t_i].get_curTexture() );

		//osassert( t_tex );
		t_b = t_eff->set_texture( (int)t_i, t_tex );
		if( !t_b )
		{
			osassert( t_b );
			return false;
		}
	}

	//----- ���þ��� -------
	for( t_i = 0; t_i < (int)t_shader->m_vMatrix.size(); ++t_i )
	{
		t_eff->set_matrix( (int)t_i, t_shader->m_vMatrix[t_i].mat );
	}

	// ��Effect��͸����
	if( !float_equal( t_shader->m_fEffectAlphaVal,1.0f ) )
		t_eff->set_float( EFFECT_ALPHAVAL,t_shader->m_fEffectAlphaVal );
	

	// ���ñ����Ⱦ״̬��Ϣ

	
	return true;

	unguard;
}


//! ȷ��shaderId�Ŀ����ԡ�
bool osc_shaderMgr::assert_shaderId( int _id,bool _assert/* = true*/  )
{
	guard;

	osc_shader*   			t_shader;
	osc_effect*              t_eff;

	t_shader = &m_vecShader[_id];
	t_eff = m_effMgr->get_effectfromid( t_shader->m_iEffid );
	if( _assert )
	{
		osassertex( t_eff->m_peffect,"Effect ����Ϊ��....\n" );
	}
	else
	{
		if( t_eff->m_peffect != NULL )
			return true;
		else
			return false;
	}

	return true;

	unguard;
}



/** \brief
*  Read a ini file shader section to a shader struct.
*
*  Ҳ���ܴ�XML�ļ���ʽ�����shader��Ϣ.
* 
*  �㷨����:
*  �Ȱ�ini�ļ����뵽�ڴ�,Ȼ����ڴ��д���ini��Ϣ�����osc_shader�ṹ.
*/
bool osc_shaderMgr::create_shaderFromINI( int _shaId, char* _shaname, char* _ininame, int _ds )
{
	guard;

	int         t_i;
	CIniFile    t_file;
	char        t_str[64];
	char        t_name[64];

	osassert( _shaname );
	osassert( _ininame );
	
	osc_shader* t_shader = &( m_vecShader[_shaId] );
	t_shader->m_strShadername = _shaname;
	t_shader->m_dwHash = string_hash( _shaname );

	if( _ds >= 0 )
	{
		// ʹ��Ĭ�ϵ�shaderTemplate
		if( !creat_shaderFromTemplate( _shaId, _shaname, _ds ) )
			return false;

		return true;
	}

	// ����ini�ļ����ڴ滺����,����ini�ļ�.
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_i = read_fileToBuf( _ininame,t_ptrStart,TMP_BUFSIZE );
	t_file.OpenMemRead( t_ptrStart,t_i );

	// ��ʼ����shader��Ϣ.
    if( !t_file.ReadSection( _shaname ) )
	{
		osassert( false );
	}

	// �ȶ���effect name.
	t_file.ReadLine( EFFECTNAME,t_name,64 );

	std::string	eff = t_name;
	std::string	tex[6];
	int			format[6];

	// ����������Ŀ�Ͷ�Ӧ����������.
	int texnum;
	t_file.ReadLine( TEXTURENUM,&texnum );
	osassert( texnum< 8 );

	m_vecShader[_shaId].m_vTexture.resize( texnum );
	for( t_i=0; t_i< texnum; ++t_i )
	{
		t_shader->m_vTexture[t_i].texList.resize(1);

		sprintf( t_str,"%s%d",TEXNAMESTR,t_i );

		if( !t_file.ReadLine( t_str,t_name,64 ) )
			return false;

		osn_mathFunc::convert_lowercase( t_name );

		tex[t_i] = t_name;

		// ���������ʽ,�������ʧ��,��������ͨ������.
		sprintf( t_str,"%s%d",TEXFORMAT,t_i );

		// 0: common texture 1:cubic texture 2:volume texture.
		if( !t_file.ReadLine( t_str,format+t_i ) )
			format[t_i] = 0;
	}

	t_file.CloseFile();

	END_USEGBUF( t_iGBufIdx );

	set_effect( t_shader, eff.c_str() );
	for( t_i = 0; t_i < (int)t_shader->m_vTexture.size(); ++t_i )
		set_texture( t_shader, t_i, 0, tex[t_i].c_str(), format[t_i] );

	return true;

	unguard;
}

//  River added @ 2006-8-24:���ڳ��׵��滻���ĵĵ�ͼ
//! ����һ��Shader��ĳһ������
void osc_shaderMgr::set_shaderTexture( int _shaderId,int _idx,int _texId )
{
	guard;

	osassert( _shaderId >= 0 );
	osassert( _idx >=0 );
	osassert( _texId >= 0 );

	osc_effect*   t_ptrEffect;

	t_ptrEffect = get_effect( _shaderId );	
	t_ptrEffect->set_texture( _idx,get_texturePtr( _texId ) );
	m_vecShader[_shaderId].m_vTexture[_idx].texList[0] = _texId;

	unguard;
}


/** ���������й�������shaderMgr�м���shader.
 *  ��Ҫ��������shader���ݺ�shader�а�����Texture&Effect����.
 */
int osc_shaderMgr::dadd_shadertomanager( char* _shadername, char* _inifilename,int _defaultS )
{
	guard;

	int      t_idx;
	bool     t_b;

	// �ȴ��Ѿ���shader�����в���,����ǰ��������û��Ҫ�����shader.
	DWORD   t_dwHash = string_hash( _shadername );

	::EnterCriticalSection( &osc_mapLoadThread::m_sShaderCS );

	CSpaceMgr<osc_shader>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecShader.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecShader.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->get_ref()>0 );
		if( (t_ptrNode.p->m_dwHash == t_dwHash)&&
			(t_ptrNode.p->m_iTemplateIdx == _defaultS) )
		{
			osassert( t_ptrNode.p->m_strShadername == _shadername );
			osassert( t_ptrNode.p->m_iEffid>=0 );
			t_ptrNode.p->add_ref();

			::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );

			return t_ptrNode.idx;
		}
	}

	// ��ʼ���ҿ��Դ����µ�shader�Ŀռ䡣
	t_idx = m_vecShader.get_freeNode();
	m_vecShader[t_idx].add_ref();
	::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );


	// Read the shader info from ini file.
	t_b = create_shaderFromINI( t_idx, _shadername, _inifilename, _defaultS );
	osassertex( t_b,va("����shader<%s>...\n",_shadername) );

	if( !t_b )
	{
		osassert( false );
		return -1;
	}
	m_vecShader[t_idx].m_iTemplateIdx = _defaultS;

	osassert( m_vecShader[t_idx].m_iEffid>=0 );
	osassert( m_effMgr->get_effectfromid( m_vecShader[t_idx].m_iEffid )->m_peffect );



	return t_idx;

	unguard;
}


/** ��shaderMgr����һ��Lightmap, �ṩ���ϲ�ܵ�Ϊÿһ��Polygon��Lightmap�ṩID
 *  \param int ����Ҫ������Lightmap��id.
 */
int osc_shaderMgr::create_texture( char* _texFile, int _format/* = 0*/ )
{
	guard;

	osassert( _texFile );
	osassert( _texFile[0] );

	// ��̬����texture manager�м���lightmap.
	return m_texMgr->dadd_textomanager( _texFile, _format );

	unguard;
}


void osc_shaderMgr::release_texture( int _texId )
{
	if( m_texMgr )
		m_texMgr->release_texture( _texId );
}

//! �õ�ֱ�Ӵ��������ָ�롣
LPDIRECT3DBASETEXTURE9 osc_shaderMgr::get_texturePtr( int _texId )
{
	osassert( _texId>= 0 );
	return m_texMgr->get_texfromid( _texId );
}


/** \brief 
*  ��ʼ�����ǵ�ShaderMgr,ʹ��shaderMgr�п��õ�3d Device.
*/
void osc_shaderMgr::init_shaderMgr( LPDIRECT3DDEVICE9 _device )
{
	m_pd3dDevice = _device;

	m_texMgr = new osc_textureMgr;
	m_effMgr = new osc_effectMgr;

	m_texMgr->init_textureMgr( _device );
	m_effMgr->init_effectMgr( _device );
}


/** \brief
*  �õ���ǰshaderMgr�ڵ�ĳ��shader��Ӧ��tga�ļ��Ƿ���alphaͨ����
*
*  ֻ�õ���һ�������Ƿ���alphaͨ��������з����档
*/
bool osc_shaderMgr::has_alphaChannel( int& _id )
{
	guard;

	int           t_i;

	osassert( _id >= 0 );

	// River mod @ 2011-3-31:��ʱ�Ȳ�������������������⣬���ȷ��ص����ID��
	if( !m_vecShader.validate_id( _id ) )
	{
		_id = 0;
		return false;
	}

	t_i = m_vecShader[_id].m_vTexture[0].texList[0];
	osassert( t_i>=0 );

	// 
	// River mod @ 2007-4-9:�����effect�ļ���ǿ���������͸��,�򷵻�͸��.
	// ��Щ���ʱ��뵱��͸����������Ⱦ,������������ʶ�Ӧ���������Ƿ�͸��
	// �������µ�ˮ��Ч��
	if( get_effect( _id )->is_forceTrans() )
		return true;

	// �õ���һ�������Ƿ�͸��
	return m_texMgr->is_alphaChannelTex( t_i );

	unguard;
}


/// ��̬���ͷ�һ��shaderռ�õ���Դ.
int osc_shaderMgr::release_shader( int _id,bool _frelease/* = false*/ )
{
	guard;

	osc_shader*  t_shader;
	int          t_iRef;
	
	// River mod @ 2009-5-28:-1��shader���ܳ���.
	if( _id < 0 )
		return 0;

	::EnterCriticalSection( &osc_mapLoadThread::m_sShaderCS );

	// �����ShaderId�Ѿ�������ɾ��
	if( !m_vecShader.validate_id( _id ) )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );
		return 0;
	}

	// �����ǰ��shader�Ѿ�û������,�ͷŵ�shader��texMgr&effMgr�е�����.
	t_iRef = 0;
	if( m_vecShader[_id].get_ref() > 0 )
	{
		// releaseʱ֮ʱ����Ҫ�����ͷ��ٽ���.
		::EnterCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

		t_iRef = m_vecShader[_id].release_ref();
		if( 0==t_iRef )
		{
			t_shader = &m_vecShader[_id];
			for( size_t t_i=0; t_i < t_shader->m_vTexture.size(); ++t_i )
			{
				for( size_t j = 0; j < t_shader->m_vTexture[t_i].texList.size(); ++j )
				{
					m_texMgr->release_texture( t_shader->m_vTexture[t_i].texList[j] );
				}
			}

			m_effMgr->release_effect( t_shader->m_iEffid );

			m_vecShader.release_node( _id );
		}

		::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );
	}

	if( _frelease )
	{
		// �ͷ�shader����Ϊ��.
		if( m_vecShader.validate_id( _id ) )
		{
			if( m_vecShader[_id].get_ref() > 0 )
			{
				//@{
				//! River @ 2009-5-28:�ͷ������effect�ᵽǰ�档�����ͷ���shader��
				//  �޴��ͷ�texture��effect
				t_shader = &m_vecShader[_id];
				for( size_t t_i=0; t_i < t_shader->m_vTexture.size(); ++t_i )
				{
					for( size_t j = 0; j < t_shader->m_vTexture[t_i].texList.size(); ++j )
					{
						m_texMgr->release_texture( t_shader->m_vTexture[t_i].texList[j] );
					}
				}
				m_effMgr->release_effect( t_shader->m_iEffid );
				//@} 

				while( 0 != m_vecShader[_id].release_ref() );
				m_vecShader.release_node( _id );
			}
		}

		t_iRef = 0;
	}
	::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );
	return t_iRef;

	unguard;
}

//! �õ���ǰshader��refֵ��
int osc_shaderMgr::get_shaderRef( int _id )
{
	guard;
	osassert( _id >= 0 );
	osassert( m_vecShader.validate_id(_id) );

	return m_vecShader[_id].get_ref();

	unguard;
}



//! ��Ҫ����IndexedBlend��SkinMesh����Ⱦ����Ҫֱ�ӵõ�texture.
//! �õ���ǰshader�õ��ĵ�һ�������ָ�롣
LPDIRECT3DTEXTURE9  osc_shaderMgr::get_shaderTex( int _id )
{
	guard;

	osc_shader*  t_shader;

	osassert( _id >= 0 );
	osassert( m_vecShader.validate_id(_id) );

	t_shader = &m_vecShader[_id];
	
	osassert( t_shader->m_vTexture.size()>=1 );
	
	return (LPDIRECT3DTEXTURE9)m_texMgr->get_texfromid( t_shader->m_vTexture[0].texList[0] );

	unguard;
}


/// reset ShaderMgrǰ��Ҫ�ͷŵ�����.
void osc_shaderMgr::shaderMgr_onLostDevice( void )
{
	m_effMgr->eff_onLostDevice();
}

/** \brief
*  ����shader�����е��豸�����Դ, �ϲ�Reset�豸����Ҫ�����������.
*
*  ��ǰshader�����ݲ���Ҫ�κεı仯,ֻ��ҪReset EffectMgr�Ϳ�����.
*/
void osc_shaderMgr::shaderMgr_onResetDevice( LPDIRECT3DDEVICE9 _device )
{
	m_effMgr->eff_onResetDevice();
}


bool osc_shaderMgr::load_effect( TiXmlElement* shaElem, osc_shader* sha )
{
	guard;

	TiXmlElement *elem, *elem2;
	elem = shaElem->FirstChildElement( "Effect" );
	osassert( elem );

	elem2 = elem->FirstChildElement( "File" );
	osassert( elem2 );

	const char* file = elem2->Attribute( "value" );

	elem2 = elem->FirstChildElement("Alpha");
	if (elem2)
	{
		// 
		// River mod @ 2008-12-17:����͸��������
		const char* hexValue = elem2->Attribute("value");
		sscanf(hexValue,"%f",&sha->m_fEffectAlphaVal );
		if( sha->m_fEffectAlphaVal > 1.0f )
			sha->m_fEffectAlphaVal = 1.0f;
		if( sha->m_fEffectAlphaVal < 0.1f )
			sha->m_fEffectAlphaVal = 0.1f;
	}
	else
		sha->m_fEffectAlphaVal = 1.0f;

	osassert( file );

	set_effect( sha, file );
	osassert( sha->m_iEffid>=0 );

	return true;

	unguard;
}

bool osc_shaderMgr::load_texture( TiXmlElement* shaElem, osc_shader* sha, osc_effect* eff )
{
	guard;

	int t_i = 0;
	char temp[64];

	TiXmlElement* elem, *elem2;
	int texNum = eff->get_texCount();

	int texType = 0;
	int frame = 1; 
	const char* fr, *sp, *di;

	const char* texName, *sTexType;

	for( t_i = 0; t_i < texNum; ++t_i )
	{
		sprintf( temp, "Texture%d", t_i );
		elem = shaElem->FirstChildElement( temp );
		if( !elem )
			sha->m_vTexture[t_i].texList.clear();
		else
		{
			fr = elem->Attribute( "frame" );
			if( fr )
				frame = atoi( fr );
			else
				frame = 1;
			//--------------------
			sp = elem->Attribute( "speed" );
			if( sp )
				sha->m_vTexture[t_i].speed = (float)atof( sp );
			else
				sha->m_vTexture[t_i].speed = 1.f;
			//--------------------
			di = elem->Attribute( "dir" );
			if( di )
				sha->m_vTexture[t_i].texDir = di;
			else
				sha->m_vTexture[t_i].texDir = "";
			//--------------------

			elem2 = elem->FirstChildElement( "Type" );
			if( !elem2 )
				texType = 0;
			else
			{
				sTexType = elem2->Attribute( "value" );

				//  0:common tex.1:cubic tex.2:volume tex.
				if( !sTexType )texType = 0;
				else if( strcmp( sTexType, "common" ) == 0 )texType = 0;
				else if( strcmp( sTexType, "cubic" ) == 0 )texType = 1;
				else if( strcmp( sTexType, "volume" ) == 0 )texType = 2;
				else osassert( false );
			}
			//-------------------------------------
			sha->m_vTexture[t_i].texList.resize( frame );
			for( int j = 0; j < frame; ++j )
			{
				sprintf( temp, "File%d", j );
				elem2 = elem->FirstChildElement( temp );
				if( !elem2 )
				{
					sha->m_vTexture[t_i].texList[j] = -1;
					continue;
				}
				else
				{
					texName = elem2->Attribute( "name" );
					osassert( texName );
				}

				strcpy( temp, sha->m_vTexture[t_i].texDir.c_str() );
				strcat( temp, texName );
				set_texture( sha, t_i, j, temp, texType );
			}
		}
	}

	return true;

	unguard;
}

bool osc_shaderMgr::load_matrix( TiXmlElement* shaElem, osc_shader* sha, osc_effect* eff )
{
	guard;

	int matNum = eff->get_matCount();
	sha->m_vMatrix.clear();
	sha->m_vMatrix.resize( matNum );
	
	char temp[64];
	TiXmlElement* elem, *elem2;
	const char* str;

	for( int t_i = 0; t_i < matNum; ++t_i )
	{
		D3DXMatrixIdentity( &(sha->m_vMatrix[t_i].mat) );

		sprintf( temp, "Matrix%d", t_i );
		elem = shaElem->FirstChildElement( temp );
		if( elem )
		{
			elem2 = elem->FirstChildElement( "Type" );
			if( !elem2 )
			{
				sha->m_vMatrix[t_i].type = 0;
				continue;
			}
			else
			{
				str = elem2->Attribute( "value" );
				if( !str )
				{
					sha->m_vMatrix[t_i].type = 0;
					continue;
				}
				else
				{
					sha->m_vMatrix[t_i].type = atoi( str );
				}
			}

			elem2 = elem->FirstChildElement( "Angle" );
			if( !elem2 )
			{
				sha->m_vMatrix[t_i].angle = 0.f;
			}
			else
			{
				str = elem2->Attribute( "value" );
				if( !str )
					sha->m_vMatrix[t_i].angle = 0.f;
				else
					sha->m_vMatrix[t_i].angle = (float)atof( str );
			}

			elem2 = elem->FirstChildElement( "Speed" );
			if( !elem2 )
			{
				sha->m_vMatrix[t_i].speed = 0.f;
			}
			else
			{
				str = elem2->Attribute( "value" );
				if( !str )
                    sha->m_vMatrix[t_i].speed = 0.f;
				else
					sha->m_vMatrix[t_i].speed = (float)atof( str );
			}

			elem2 = elem->FirstChildElement( "Center" );
			if( !elem2 )
			{
				sha->m_vMatrix[t_i].center.x	= 0.f;
				sha->m_vMatrix[t_i].center.y	= 0.f;
				sha->m_vMatrix[t_i].center.z	= 0.f;
			}
			else
			{
				str = elem2->Attribute( "x" );
				if( !str )
					sha->m_vMatrix[t_i].center.x = 0.f;
				else
					sha->m_vMatrix[t_i].center.x = (float)atof( str );

				str = elem2->Attribute( "y" );
				if( !str )
					sha->m_vMatrix[t_i].center.y = 0.f;
				else
					sha->m_vMatrix[t_i].center.y = (float)atof( str );

				str = elem2->Attribute( "z" );
				if( !str )
					sha->m_vMatrix[t_i].center.z = 0.f;
				else
					sha->m_vMatrix[t_i].center.z = (float)atof( str );
			}
		}
		else
			sha->m_vMatrix[t_i].type = 0;
	}

	return true;

	unguard;
}


int	osc_shaderMgr::_add_shader( const char* _name, const char* _file, const char* _realName )
{
	guard;

	int  t_idx;

	// ��object.sdr���뵽��̬�ڴ���.
	static BYTE* t_objectSdr = NULL;
	static BYTE* t_charSdr = NULL;
	static BYTE* t_bbSdr = NULL;
	static BYTE* t_decalSdr = NULL;
	static TiXmlDocument t_objXmlDoc;
	static TiXmlDocument t_chrXmlDoc;
	static TiXmlDocument t_bbXmlDoc;
	static TiXmlDocument t_decXmlDoc;

	TiXmlDocument xmlDoc;
	TiXmlElement* shaEle;	

	if( t_objectSdr == NULL )
	{
		int t_iSize = get_fileSize( "ini\\object.sdr" );
		t_objectSdr = new BYTE[t_iSize+10];
		::read_fileToBuf( "ini\\object.sdr",t_objectSdr, t_iSize+10 );
		t_objXmlDoc.Parse( (const char*)t_objectSdr );
		delete[] t_objectSdr;
	}		
	if( t_charSdr == NULL )
	{
		int t_iSize = get_fileSize( "ini\\character.sdr" );
		t_charSdr = new BYTE[t_iSize+10];
		::read_fileToBuf( "ini\\character.sdr",t_charSdr,t_iSize+10 );
		t_chrXmlDoc.Parse( (const char*)t_charSdr );
		delete[] t_charSdr;
	}
	if( t_bbSdr == NULL )
	{
		int t_iSize = get_fileSize( "ini\\billboard.sdr" );
		t_bbSdr = new BYTE[t_iSize+10];
		::read_fileToBuf( "ini\\billboard.sdr",t_bbSdr,t_iSize+10 );	
		t_bbXmlDoc.Parse( (const char*)t_bbSdr );
		delete[] t_bbSdr;
	}
	if( t_decalSdr == NULL )
	{
		int t_iSize = get_fileSize( "ini\\decal.sdr" );
		t_decalSdr = new BYTE[t_iSize+10];
		::read_fileToBuf( "ini\\decal.sdr",t_decalSdr,t_iSize+10 );	
		t_decXmlDoc.Parse( (const char*)t_decalSdr );
		delete[] t_decalSdr;
	}

	//-------------------file-------------------
	int		t_i = 0;
	if( strcmp( _file,"ini\\object.sdr" ) == 0 )
		shaEle = t_objXmlDoc.FirstChildElement("global");
	else if( strcmp( _file,"ini\\character.sdr" ) == 0 )
		shaEle = t_chrXmlDoc.FirstChildElement("global");
	else if( strcmp( _file,"ini\\billboard.sdr" ) == 0 )
		shaEle = t_bbXmlDoc.FirstChildElement("global");
	else if( strcmp( _file,"ini\\decal.sdr" ) == 0 )
		shaEle = t_decXmlDoc.FirstChildElement("global");
	else
	{
		int   t_iGBufIdx = -1;
		BYTE*	t_ptrStart = START_USEGBUF( t_iGBufIdx );
		t_i = ::read_fileToBuf( (char*)_file, t_ptrStart, TMP_BUFSIZE );
		if( t_i <= 0 )
		{
			END_USEGBUF( t_iGBufIdx );
			osassertex( false,va("����shader�ļ�����<%s>..\n", _file ) );
			return -1;
		}
		t_ptrStart[t_i] = 0;
		xmlDoc.Parse( (const char*)t_ptrStart );
		END_USEGBUF( t_iGBufIdx );
		osassert( !xmlDoc.Error() );
		shaEle = xmlDoc.FirstChildElement("global");
	}


	std::string effName;
	if (!shaEle)
		osassert("sdr�ļ�����!");
	shaEle = shaEle->FirstChildElement( _name );
	//osassert( shaEle );
	if( !shaEle )return -1;

	t_idx = m_vecShader.get_freeNode();
	m_vecShader[t_idx].set_name( _realName );
	osc_shader* t_shader = &( m_vecShader[ t_idx ] );

	//----------------effect--------------------
	if( !load_effect( shaEle, t_shader ) )return -1;
	osc_effect* eff = m_effMgr->get_effectfromid( m_vecShader[t_idx].m_iEffid );
	osassert( eff );
	//------------------texture-----------------
	if( !load_texture( shaEle, t_shader, eff ) )return -1;

	//-------------------matrix-----------------
	if( !load_matrix( shaEle, t_shader, eff ) )return -1;

	m_vecShader[t_idx].m_iTemplateIdx = -1;
	m_vecShader[t_idx].add_ref();
	return t_idx;

	unguard;
}


//! ͨ�����е�shader��_shaId���͵�һ������_tex1_file������һ���µ�shader��_name��
int osc_shaderMgr::_add_shader( const char* _realName, int _shaId, const char* tex1_file )
{
	guard;

	if( _shaId < 0 )return -1;

	int  t_idx;

	// ----------------find-------------------
	t_idx = get_shader( _realName );
	if( t_idx != -1 )
	{
		m_vecShader[t_idx].add_ref();
			return t_idx;
	}

	t_idx = m_vecShader.get_freeNode();
	//------------------ copy ----------------
	osc_shader* t_shader = &( m_vecShader[t_idx] );
	*t_shader = m_vecShader[_shaId];
	t_shader->set_name( _realName );

	if( t_shader->m_vTexture.size() > 0 )
	{
		char temp[128];
		strcpy( temp, t_shader->m_vTexture[0].texDir.c_str() );
		strcat( temp, tex1_file );
		set_texture( t_shader, 0, 0, temp, 0 );
	}

	return t_idx;

	unguard;
}

bool osc_shaderMgr::get_realShaderName( const char* _name, const char* _file, char* buf )
{
	guard;

	// River @ 2010-12-29:ȥ���˴��룬�ӿ촴���ٶ�
	//if( !file_exist( (char*)_file ) )
	//	return false;

	s_string str = _file;

	std::replace( str.begin(), str.end(), '/', '\\' );
	s_string::size_type in;
	in = str.rfind( '.' );
	str = str.substr( 0, in );
	in = str.rfind( '\\' );
	str = str.substr( in+1, str.length() );
	std::transform( str.begin(), str.end(), str.begin(), tolower);

	sprintf( buf, "%s_%s", str.c_str(), _name );

	return true;

	unguard;
}

int osc_shaderMgr::get_shader( const char* _name )
{
	guard;

	DWORD   t_dwHash = string_hash( _name );
	CSpaceMgr<osc_shader>::NODE  t_ptrNode;

	::EnterCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

	for( t_ptrNode = m_vecShader.begin_usedNode(); t_ptrNode.p != NULL; 
		t_ptrNode = m_vecShader.next_validNode( &t_ptrNode ) )
	{
		// River mod @ 2011-2-25:���̴߳���,�˴������ͷ�,������release_shader���ͷ�.
		/*
		if( t_ptrNode.p->get_ref() == 0 )
		{
			m_vecShader.release_node( t_ptrNode.idx );
			continue;
		}
		*/

		if( t_ptrNode.p->m_dwHash == t_dwHash )
		{
			osassertex( (t_ptrNode.p->m_strShadername == _name),
				va( "filename<%s><%s><%d><%d>..\n",
				t_ptrNode.p->m_strShadername.c_str(),_name,t_ptrNode.p->m_dwHash,t_dwHash ) );
			osassert( t_ptrNode.p->m_iEffid>=0 );
			::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );
			return t_ptrNode.idx;
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

	return -1;

	unguard;
}


//! ͨ��xml�ļ����ļ��е�һ��shader�ṹ������һ��shader
int osc_shaderMgr::add_shader( const char* _name, const char* _file,const char* _defaultname /*= "default"*/ )
{
	guard;

	if( strlen( _file ) == 0 )
	{
		osDebugOut( va( "�����˿յ�shader�ļ�<%s>...\n",_name ) );
		return -1;
	}

	char shaName[256];
	if( !get_realShaderName( _name, _file, shaName ) )
	{
		osDebugOut( "�õ�������shader����ʧ��...\n" );
		return -1;
	}

	int id = get_shader( shaName );
	if( id != -1 )
	{
		m_vecShader[id].add_ref();
		return id;
	}

	::EnterCriticalSection( &osc_mapLoadThread::m_sShaderCS );

	id = _add_shader( _name, _file, shaName );
	if( id < 0 )
	{
		char temp[256];
		if( !get_realShaderName( _defaultname, _file, temp ) )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );

			osDebugOut( "�õ�Ĭ�ϵ�shaderʧ��...\n" );
			return -1;
		}

		int def = get_shader( temp );
		if( def == -1 )
		{
			def = g_shaderMgr->_add_shader( _defaultname, _file, temp );
			if( def == -1 )
			{
				::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );

				osDebugOut( "���εõ�Ĭ�ϵ�shaderʧ��...\n" );
				return -1;
			}
		}

		strcpy( temp, _name );
		strcat( temp, ".hon" );
		id = g_shaderMgr->_add_shader( shaName, def, temp );
	}
	
	::LeaveCriticalSection( &osc_mapLoadThread::m_sShaderCS );
	return id;

	unguard;
}

osc_effect* osc_shaderMgr::get_effect( int _shaid )
{
	guard;

	if( _shaid >=0 )
	{
		//osassert( m_vecShader.validate_id( _shaid ) );
		// River @ 2011-10-14:������Ĵ���ʽ��
		if( !m_vecShader.validate_id( _shaid ) )
			return NULL;

		return m_effMgr->get_effectfromid( m_vecShader[_shaid].m_iEffid );
	}
	else
		return NULL;

	unguard;
}

// 
void osc_shaderMgr::frame_move( void )
{
	memset( m_vecIsFramed, 0, INIT_SHADERVECSIZE );
}