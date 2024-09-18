///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterEffect.cpp
 * 
 *  His:      River created @ 2004-12-15
 *
 *  Desc:     �������������ϵڶ�����ͼ��ص���Ч�������ڶ������ͼ�������Ի�����ͼ��
 *            ���Ծ�������ת���ȵȣ��ڲ��Ķ��������ݵĻ����ϣ������Զ������ĵڶ���
 *            �������ݡ�
 *  
 *  "�Ҳ��г����ʿ�����" 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/frontPipe.h"
# include "../../mfpipe/include/osStateBlock.h"
# include "../../backpipe/include/mainEngine.h"


//! �Ƿ��ڳ�������ʾ����ĵڶ�����ͼ,��û��Ӳ��VertexShader�Ļ����ϣ�
//! ���ܴ�ֵ�Ƿ�Ϊ�棬��������ʾ�ڶ�����ͼ
OSENGINE_API BOOL     g_bUseSecondTexOnCha = TRUE;

//! �ڶ�����ͼ���ļ��汾��.
//# define  CHASEC_EFFVERSION   101
//! river @ 2010-3-11:������Ĳ���ͼ��
# define  CHASEC_EFFVERSION   102

os_chaSecEffect::os_chaSecEffect()
{
	m_iSecTexId = -1;
	m_iShaderId = -1;
	//!������ͨ����,����ָ���ڶ����͸����
	m_iThdId	= -1;

	// River @ 2010-3-11:
	m_iFourthId = -1;
	m_szFourthTexName[0] = NULL;
	m_szThdTexName[0] = NULL;

	m_wEffectType = -1;

	m_fRotSpeed = 0.0f;

	osMatrixIdentity( &m_matFinal );

	m_fTexmoveTime = 0.0f;
	m_szThdTexName[0] = 0;

}

//! ɾ���豸��ص�����
void os_chaSecEffect::delete_devdepRes( void )
{
	if( m_iSecTexId>=0 )
	{
		g_shaderMgr->release_texture( m_iSecTexId );
		m_iSecTexId = -1;
	}
	if( m_iThdId >= 0 )
	{
		g_shaderMgr->release_texture( m_iThdId );
		m_iThdId = -1;
	}
	if( m_iFourthId >= 0 )
	{
		g_shaderMgr->release_texture( m_iFourthId );
		m_iFourthId = -1;
	}
}

os_chaSecEffect::~os_chaSecEffect()
{
	if( m_iSecTexId>=0 )
		g_shaderMgr->release_texture( m_iSecTexId );
}



//! ÿһ֡���㵱ǰ��ͼ��Ч���ڲ����ݡ�
void os_chaSecEffect::frame_moveSecEff( void )
{
	float   t_f = sg_timer::Instance()->get_lastelatime();

	m_fTexmoveTime += t_f;

	if( this->m_wEffectType == 6 )
	{
		m_matFinal._41 = m_fUMoveSpeed*m_fTexmoveTime;
		m_matFinal._42 = m_fVMoveSpeed*m_fTexmoveTime;
	}

	// River @ 2010-3-13:�Է���������������.
	if( m_wEffectType == 9 )
	{
		// ����
		m_matFinal._41 = m_fUMoveSpeed*m_fTexmoveTime;
		m_matFinal._42 = m_fVMoveSpeed*m_fTexmoveTime;

		// �Է���
		int t_i = int(m_fTexmoveTime/m_fRotSpeed);
		t_f = float_mod( m_fTexmoveTime,m_fRotSpeed );
		if( t_i%2 == 1 )
			t_f = m_fRotSpeed-t_f;
		t_f /= m_fRotSpeed;
		m_matFinal._43 = t_f;
	}

	//! river @ 2010-2-25: �������ϵ��Է�������
	//! river @ 2010-3-13: �����Է���Ч���뻷����ĺ���
	if( (m_wEffectType == 7)||( m_wEffectType == 10) )
	{
		int t_i = int(m_fTexmoveTime/m_fRotSpeed);
		t_f = float_mod( m_fTexmoveTime,m_fRotSpeed );
		if( t_i%2 == 1 )
			t_f = m_fRotSpeed-t_f;
		t_f /= m_fRotSpeed;

		m_matFinal._43 = t_f;
	}

	//  ��m_wEffectType == 6��ͬ�ļ��㣬���Ż���
	//! river @ 2010-3-11:����rotsca��env���ϵ�װ��װ��
	if( m_wEffectType == 8 )
	{
		t_f = m_fTexmoveTime * m_fRotSpeed;
		osMatrixRotationAxis( &m_matFinal,&m_vec3RotAxis,t_f );
		m_matFinal._41 = m_fUMoveSpeed*m_fTexmoveTime;
		m_matFinal._42 = m_fVMoveSpeed*m_fTexmoveTime;
	}


	return;
}


//! �ͷ�����ĳһ���ֶ�Ӧ��skinShader.
void osc_skinMesh::release_skinShader( int _idx )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=7) );

	osc_bpmInstance*   t_ptrIns;

	osassert( (_idx>=0)&&(_idx<=7) );

	t_ptrIns = &m_vecBpmIns[_idx];

	if( t_ptrIns->m_iShaderID>=0 )
	{
		//g_shaderMgr->release_shader( t_ptrIns->m_iShaderID );
		t_ptrIns->release_insShader( t_ptrIns->m_iShaderID );
		t_ptrIns->m_iShaderID = -1;
	}

	//SAFE_DELETE( t_ptrIns->m_ptrSecEffect );
	if( t_ptrIns->m_ptrSecEffect )
	{
		t_ptrIns->m_ptrSecEffect->delete_devdepRes();
		release_secEffectPtr( t_ptrIns->m_ptrSecEffect );
		t_ptrIns->m_ptrSecEffect = NULL;
	}

	unguard;
}


//! ���������������ͼ����Shader
bool osc_skinMesh::create_mlFlagShader( int _idx,char* _sname )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=7) );
	osassert( _sname&&(_sname[0]) );

	m_vecBpmIns[_idx].m_iShaderID = 
		g_shaderMgr->add_shader( _sname, "ini\\mlFlag.sdr" );

	osassertex( m_vecBpmIns[_idx].m_iShaderID >= 0 ,_sname );

	// ���ĵ�Shader�ڷ�Alpha�㼶������Ⱦ
	m_vecBpmIns[_idx].m_bAlpha = true;

	return true;

	unguard;
}



//! ������ͨ�ĵ�������������ȾЧ����
bool osc_skinMesh::create_singleTexShader( int _idx,char* _sname )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=7) );
	osassert( _sname&&(_sname[0]) );


	//River @2010-12-28:��¼shader��hashId.����������п���ֱ��ʹ�õ�shaderId,��ֱ��ʹ��
	m_vecBpmIns[_idx].m_dwHashId = string_hash( _sname );
	m_vecBpmIns[_idx].m_iShaderID = osc_bpmInstance::get_bufShaderId( m_vecBpmIns[_idx].m_dwHashId );

	if( m_vecBpmIns[_idx].m_iShaderID < 0 )
		m_vecBpmIns[_idx].m_iShaderID = g_shaderMgr->add_shader( _sname, osc_engine::get_shaderFile( "character" ).c_str() );
	if( m_vecBpmIns[_idx].m_iShaderID < 0 )
		m_vecBpmIns[_idx].m_iShaderID = g_shaderMgr->add_shadertomanager( _sname,CHARACTER_SHADERINI,osc_engine::m_iCharacterShaId );
	if( m_vecBpmIns[_idx].m_iShaderID < 0 )
	{
		osassertex( false,_sname );
		return false;
	}


	// 
	// �õ���ǰshaderId��Ӧ����������IndexedBlend��Ⱦ��
	m_vecBpmIns[_idx].m_ptrBpmTexture = 
		g_shaderMgr->get_shaderTex( m_vecBpmIns[_idx].m_iShaderID );

	// 
	// Get Whether The alpha channel is exist in current Shader.
	m_vecBpmIns[_idx].m_bAlpha = 
		g_shaderMgr->has_alphaChannel( m_vecBpmIns[_idx].m_iShaderID );

	return true;

	unguard;
}

//! ����ĵڶ�����ͼ����Ҳʹ��Ԥ�������ʽ
VEC_secEffProLoad osc_skinMesh::m_vecSecEffPreLoad;
int osc_skinMesh::m_iSecEffPLNum = 0;


//! ����ڶ�����Ч��������ݡ�
bool osc_skinMesh::read_secondTexEff( os_chaSecEffect* _seceff,char* _sname )
{
	guard;

	osassert( _seceff );
	osassert( _sname );

	s_string         t_strName;
	t_strName = _sname;
	t_strName += ".cse";

	// River @ 2010-12-22:
	// ���������Ԥ��ȡ���ҵ�������Ҫ�ظ���ȡ.
	DWORD   t_dwHash = string_hash( t_strName.c_str() );
	for( int t_i =0;t_i<m_iSecEffPLNum;t_i ++ )
	{
		if( m_vecSecEffPreLoad[t_i].m_dwHash != t_dwHash )
			continue;
		if( strcmp( m_vecSecEffPreLoad[t_i].m_szSecFile,t_strName.c_str() ) == 0 )
		{
			memcpy( _seceff,
				&m_vecSecEffPreLoad[t_i].m_sCharSecEffect,sizeof( os_chaSecEffect ) );
			return true;
		}
	}

	int     t_iSize;
	BYTE*   t_fstart;
	char    t_szHeader[4];

	// 
	// �������ݵ��ڴ档
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)t_strName.c_str(),t_fstart,TMP_BUFSIZE );
	if( t_iSize < 0 )
	{
		osassertex( false,"�ļ��������....\n" );
		return false;
	}
	

	// �����ļ�ͷ��汾��.
	READ_MEM_OFF( t_szHeader,t_fstart,sizeof( char )*4 );
	if( strcmp( t_szHeader,CHASEC_EXTENT ) )
	{
		osassertex( false,"������ļ�ͷ....\n" );
		return false;
	}
	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	if( t_iSize == 100 )
	{
		// �������ݡ�
		READ_MEM_OFF( &_seceff->m_wEffectType,t_fstart,sizeof( WORD ) );
		READ_MEM_OFF( &_seceff->m_fRotSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &_seceff->m_vec3RotAxis,t_fstart,sizeof( osVec3D ) );
		READ_MEM_OFF( &_seceff->m_fUMoveSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &_seceff->m_fVMoveSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( _seceff->m_szSecTexName,t_fstart,sizeof( char )*64 );
		_seceff->m_szThdTexName[0] = 0;
		_seceff->m_szFourthTexName[0] = 0;
	}
	else if ( t_iSize == 101 )
	{
		READ_MEM_OFF( &_seceff->m_wEffectType,t_fstart,sizeof( WORD ) );
		READ_MEM_OFF( &_seceff->m_fRotSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &_seceff->m_vec3RotAxis,t_fstart,sizeof( osVec3D ) );
		READ_MEM_OFF( &_seceff->m_fUMoveSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &_seceff->m_fVMoveSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( _seceff->m_szSecTexName,t_fstart,sizeof( char )*64 );
		READ_MEM_OFF( _seceff->m_szThdTexName,t_fstart,sizeof( char )*64 );
		_seceff->m_szFourthTexName[0] = NULL;
	}
	else if( t_iSize == 102 )
	{
		READ_MEM_OFF( &_seceff->m_wEffectType,t_fstart,sizeof( WORD ) );
		READ_MEM_OFF( &_seceff->m_fRotSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &_seceff->m_vec3RotAxis,t_fstart,sizeof( osVec3D ) );
		READ_MEM_OFF( &_seceff->m_fUMoveSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &_seceff->m_fVMoveSpeed,t_fstart,sizeof( float ) );
		READ_MEM_OFF( _seceff->m_szSecTexName,t_fstart,sizeof( char )*64 );
		READ_MEM_OFF( _seceff->m_szThdTexName,t_fstart,sizeof( char )*64 );
		READ_MEM_OFF( _seceff->m_szFourthTexName,t_fstart,sizeof( char)*64 );
	}
	else
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}

	END_USEGBUF( t_iGBufIdx );

	// River @ 2010-12-22:
	// ���뵽Ԥ��ȡ�Ŀ���.
	if( m_iSecEffPLNum < MAX_SMINVIWE*3 )
	{
		m_vecSecEffPreLoad[m_iSecEffPLNum].m_dwHash = t_dwHash;
		strcpy( m_vecSecEffPreLoad[m_iSecEffPLNum].m_szSecFile,t_strName.c_str() );
		memcpy( &m_vecSecEffPreLoad[m_iSecEffPLNum].m_sCharSecEffect,
			_seceff,sizeof( os_chaSecEffect ) );
	}
	else
	{
		os_secEffPreLoad  t_sPL;
		t_sPL.m_dwHash = t_dwHash;
		strcpy( t_sPL.m_szSecFile,t_strName.c_str() );
		memcpy( &t_sPL.m_sCharSecEffect,_seceff,sizeof( os_chaSecEffect ) );
		m_vecSecEffPreLoad.push_back( t_sPL );
	}
	m_iSecEffPLNum ++;

	return true;

	unguard;
}

//! ����ĵڶ�����ͼָ���޸�Ϊ�ڴ��ģʽ
CSpaceMgr<os_chaSecEffect> osc_skinMesh::m_vecSecEffect;


os_chaSecEffect* osc_skinMesh::get_sceEffectPtr( void )
{
	return m_vecSecEffect.get_nodePtr( m_vecSecEffect.get_freeNode() );
}

void osc_skinMesh::release_secEffectPtr( os_chaSecEffect* _ptr )
{
	CSpaceMgr<os_chaSecEffect>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecSecEffect.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecSecEffect.next_validNode( &t_ptrNode ) )
	{
		if( DWORD( _ptr ) == (DWORD)t_ptrNode.p )
		{
			m_vecSecEffect.release_node( t_ptrNode.idx );
			return;
		}
	}

	return;
}


//! ����������Ч��˫������������ȾЧ����
bool osc_skinMesh::create_doubleTexShader( int _idx,char* _sname )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=8) );
	osassert( _sname&&(_sname[0]) );


	//
	// �ȶ���ڶ�����ͼ�������ļ���
	int              t_iTexid;
	os_chaSecEffect*  t_ptrSecEff;


	//
	// �����ڴ档
	t_ptrSecEff = get_sceEffectPtr();//new os_chaSecEffect;
	m_vecBpmIns[_idx].m_ptrSecEffect = t_ptrSecEff;

	if( !read_secondTexEff( t_ptrSecEff,_sname ) )
		return false;

	//River @2010-12-28:��¼shader��hashId.����������п���ֱ��ʹ�õ�shaderId,��ֱ��ʹ��
	m_vecBpmIns[_idx].m_dwHashId = string_hash( _sname );
	m_vecBpmIns[_idx].m_iShaderID = 
		osc_bpmInstance::get_bufShaderId( m_vecBpmIns[_idx].m_dwHashId );

	//
	// ������һ����ͼ��shader.
	if( m_vecBpmIns[_idx].m_iShaderID < 0 )
	{
		m_vecBpmIns[_idx].m_iShaderID = g_shaderMgr->
			add_shadertomanager( _sname,CHARACTER_SHADERINI,t_ptrSecEff->m_wEffectType );
	}
	/*
	else
	{
		int t_m = 0;
		osc_effect*       pEff = 0;
		pEff = g_shaderMgr->get_effect( m_vecBpmIns[_idx].m_iShaderID );
		pEff = NULL;
	}*/
	osassert( m_vecBpmIns[_idx].m_iShaderID>=0 );

	// 
	// �õ���ǰshaderId��Ӧ����������IndexedBlend��Ⱦ��
	m_vecBpmIns[_idx].m_ptrBpmTexture = 
		g_shaderMgr->get_shaderTex( m_vecBpmIns[_idx].m_iShaderID );
	t_ptrSecEff->m_iShaderId = m_vecBpmIns[_idx].m_iShaderID;

	// 
	// Get Whether The alpha channel is exist in current Shader.
	m_vecBpmIns[_idx].m_bAlpha = 
		g_shaderMgr->has_alphaChannel( m_vecBpmIns[_idx].m_iShaderID );

	//
	// �����ڶ�������.
	osassert( t_ptrSecEff->m_szSecTexName[0] );
	t_iTexid = g_shaderMgr->create_texture( t_ptrSecEff->m_szSecTexName );
	osassert( t_iTexid >= 0 );
	m_vecBpmIns[_idx].set_secTexid( t_iTexid );

	if (t_ptrSecEff->m_szThdTexName[0])
	{
		t_iTexid = g_shaderMgr->create_texture( t_ptrSecEff->m_szThdTexName );
		osassert( t_iTexid >= 0 );
		m_vecBpmIns[_idx].set_thdTexid( t_iTexid );

	}
	if( t_ptrSecEff->m_szFourthTexName[0] )
	{
		t_iTexid = g_shaderMgr->create_texture( t_ptrSecEff->m_szFourthTexName );
		osassert( t_iTexid >= 0 );
		t_ptrSecEff->m_iFourthId = t_iTexid;
	}
	



	return true;

	unguard;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
//   ����ڶ�������༭��صĺ�����
//
# if __CHARACTER_EFFEDITOR__


//! �洢��ǰ��effect����,�ļ���Ҫ�Զ����
void os_chaSecEffect::save_chaSecEffToFile( const char* _cname )
{
	guard;

	osassert( _cname&&(_cname[0]) );

	char      t_id[4];
	DWORD     t_dwVersion;
	FILE*     t_file;

	
	t_file = fopen( _cname,"wb" );
	if( !t_file )
		osassert( false );

	strcpy( t_id,"cse" );
	t_dwVersion = CHASEC_EFFVERSION;
	fwrite( t_id,sizeof(char),4,t_file );
	fwrite( &t_dwVersion,sizeof( DWORD ),1,t_file );

	fwrite( &m_wEffectType,sizeof( WORD ),1,t_file );
	fwrite( &m_fRotSpeed,sizeof( float ),1,t_file );
	fwrite( &m_vec3RotAxis,sizeof( osVec3D ),1,t_file );
	fwrite( &m_fUMoveSpeed,sizeof( float ),1,t_file );
	fwrite( &m_fVMoveSpeed,sizeof( float ),1,t_file );
	fwrite( m_szSecTexName,sizeof( char ),64,t_file );
	fwrite( m_szThdTexName,sizeof( char ),64,t_file );
	
	// River @ 2010-3-11:����ĵ��Ĳ�����
	fwrite( m_szFourthTexName,sizeof( char ),64,t_file );

	fclose( t_file );

	return;

	unguard;
}



// �������岿λ�ڶ�����ͼ����Ч�༭��غ���
/** \brief
*  ��������ĳһ��λ��������ͼЧ����
*
*  ������岿λԭ��û�еڶ������������Ϊʹ�õڶ�������
* 
*  \param _seceff ��������ֵΪ�գ����ԭ�������岿λʹ����Ч�������Ϊ��ʹ����Ч��
*  
*/
void osc_skinMesh::update_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx )
{
	guard;

	int      t_idx;
	osc_bpmInstance*   t_ptrIns;
	char              t_szTexName[128];

	osassert( (_idx>=0)&&(_idx<=7) );

	t_idx = _idx;
	t_ptrIns = &m_vecBpmIns[t_idx];

	// ���ԭ����skinû����Ч�����µ���Ч�ṹΪ�գ����ء�
	if( (_seceff == NULL)&&(t_ptrIns->m_ptrSecEffect == NULL) )
		return;

	m_vecBpmIns[_idx].release_insShader( m_vecBpmIns[_idx].m_iShaderID,true );

	m_vecBpmIns[_idx].m_iShaderID = -1;
	sprintf( t_szTexName,"%s\\%d_%02d_%02d",m_strSMDir,
		_idx+1,m_Look.m_mesh[t_idx],m_Look.m_skin[t_idx] );

	// ɾ��ԭ����λ�ĵڶ�����ͼ��Ч��
	if( NULL == _seceff )
	{
		if( t_ptrIns->m_ptrSecEffect == NULL )
			return;
		
		//River @2010-12-28:��¼shader��hashId.����������п���ֱ��ʹ�õ�shaderId,��ֱ��ʹ��
		m_vecBpmIns[_idx].m_dwHashId = string_hash( t_szTexName );
		m_vecBpmIns[_idx].m_iShaderID = osc_bpmInstance::get_bufShaderId( m_vecBpmIns[_idx].m_dwHashId );

		if( m_vecBpmIns[_idx].m_iShaderID < 0 )
			m_vecBpmIns[_idx].m_iShaderID = g_shaderMgr->add_shader( t_szTexName, osc_engine::get_shaderFile( "character" ).c_str() );
		if( m_vecBpmIns[_idx].m_iShaderID < 0 )
			m_vecBpmIns[_idx].m_iShaderID = g_shaderMgr->add_shadertomanager( t_szTexName,CHARACTER_SHADERINI,osc_engine::m_iCharacterShaId );
		
		//SAFE_DELETE( t_ptrIns->m_ptrSecEffect );
		if( t_ptrIns->m_ptrSecEffect )
		{
			t_ptrIns->m_ptrSecEffect->delete_devdepRes();
			release_secEffectPtr( t_ptrIns->m_ptrSecEffect );
			t_ptrIns->m_ptrSecEffect = NULL;
		}

		return;
	}

	os_chaSecEffect*   t_ptrEff;

	// �������岿λ�ĵڶ�����ͼ��Ч��
	if( NULL == t_ptrIns->m_ptrSecEffect )
		t_ptrIns->m_ptrSecEffect = get_sceEffectPtr();//new os_chaSecEffect;
	

	t_ptrEff = t_ptrIns->m_ptrSecEffect;
	if( t_ptrEff->m_iSecTexId>=0 )
		g_shaderMgr->release_texture( t_ptrEff->m_iSecTexId );

	t_ptrEff->m_iSecTexId = g_shaderMgr->create_texture( _seceff->m_szSecTexName );

	//River @2010-12-28:��¼shader��hashId.����������п���ֱ��ʹ�õ�shaderId,��ֱ��ʹ��
	m_vecBpmIns[_idx].m_dwHashId = string_hash( t_szTexName );
	m_vecBpmIns[_idx].m_iShaderID = osc_bpmInstance::get_bufShaderId( m_vecBpmIns[_idx].m_dwHashId );

	if( m_vecBpmIns[_idx].m_iShaderID < 0 )
	{
		m_vecBpmIns[_idx].m_iShaderID = g_shaderMgr->add_shadertomanager( 
			t_szTexName,CHARACTER_SHADERINI,_seceff->m_wEffectType );
	}

	if (t_ptrEff->m_iThdId>=0)
	{
		g_shaderMgr->release_texture( t_ptrEff->m_iThdId);
		t_ptrEff->m_iThdId = -1;
	}
	if(_seceff->m_szThdTexName[0]!=0){
		t_ptrEff->m_iThdId = g_shaderMgr->create_texture( _seceff->m_szThdTexName);
	}
	else
	{
		t_ptrEff->m_iThdId = -1;
	}

	//! river @ 2010-3-11:��������ͼ 
	if (t_ptrEff->m_iFourthId>=0)
	{
		g_shaderMgr->release_texture( t_ptrEff->m_iFourthId);
		t_ptrEff->m_iFourthId = -1;
	}
	if(_seceff->m_szFourthTexName[0]!=0){
		t_ptrEff->m_iFourthId = g_shaderMgr->create_texture( _seceff->m_szFourthTexName);
	}
	else
	{
		t_ptrEff->m_iFourthId = -1;
	}



	t_ptrEff->m_wEffectType = _seceff->m_wEffectType;
	t_ptrEff->m_fRotSpeed = _seceff->m_fRotSpeed;
	t_ptrEff->m_fUMoveSpeed = _seceff->m_fUMoveSpeed;
	t_ptrEff->m_fVMoveSpeed = _seceff->m_fVMoveSpeed;
	t_ptrEff->m_vec3RotAxis = _seceff->m_vec3RotAxis;
	strcpy( t_ptrEff->m_szSecTexName,_seceff->m_szSecTexName );
	strcpy( t_ptrEff->m_szThdTexName,_seceff->m_szThdTexName );
	strcpy( t_ptrEff->m_szFourthTexName,_seceff->m_szFourthTexName );
	t_ptrEff->m_iShaderId = m_vecBpmIns[_idx].m_iShaderID;


	t_ptrEff->m_fTexmoveTime = 0.0f;

	return;

	unguard;
}

//! �õ���ǰ���岿λ��chaSecEffect���ݡ�
bool osc_skinMesh::get_chaSecEffect( os_chaSecEffStruct* _seceff,int _idx )
{
	guard;

	osassert( _seceff );
	osassert( (_idx>=0)&&(_idx<=7) );

	osc_bpmInstance*   t_ptrIns;
	os_chaSecEffect*   t_ptrSecEff;
	osassert( (_idx>=0)&&(_idx<=7) );

	t_ptrIns = &m_vecBpmIns[_idx];

	t_ptrSecEff = t_ptrIns->m_ptrSecEffect;

	if( t_ptrSecEff != NULL )
	{
		_seceff->m_fRotSpeed = t_ptrSecEff->m_fRotSpeed;
		_seceff->m_fUMoveSpeed = t_ptrSecEff->m_fUMoveSpeed;
		_seceff->m_fVMoveSpeed = t_ptrSecEff->m_fVMoveSpeed;
		strcpy( _seceff->m_szSecTexName,t_ptrSecEff->m_szSecTexName );
		strcpy( _seceff->m_szThdTexName,t_ptrSecEff->m_szThdTexName);
		strcpy( _seceff->m_szFourthTexName,t_ptrSecEff->m_szFourthTexName );

		_seceff->m_vec3RotAxis = t_ptrSecEff->m_vec3RotAxis;
		_seceff->m_wEffectType = t_ptrSecEff->m_wEffectType;

		return true;
	}
	else
		return false;

	unguard;
}


//! �洢�����_idx��λ�ĵڶ���������ͼЧ����
bool osc_skinMesh::save_chaSecEffect( int _idx )
{
	guard;

	char     t_szFName[64];
	s_string  t_szEffFname;
	int       t_iIdx;

	osassert( (_idx >=0)&&(_idx<=7) );
	t_iIdx = _idx;

	// ���ݵ�ǰ�Ĳ�λ���ļ�������ҪҪ�洢���ļ��֡�
	sprintf( t_szFName,"\\%d_%02d_%02d.cse",
		_idx+1,m_Look.m_mesh[t_iIdx],m_Look.m_skin[t_iIdx] );

	t_szEffFname = m_strSMDir;
	t_szEffFname += t_szFName;

	if( m_vecBpmIns[t_iIdx].m_ptrSecEffect )
	{
		m_vecBpmIns[t_iIdx].m_ptrSecEffect->
			save_chaSecEffToFile( t_szEffFname.c_str() );
	}
	else
	{
		sprintf( t_szFName,
			"����<%s>�ĵ�<%d>����û�еڶ�����Ч����...\n",m_strSMDir,_idx );
		osassertex(false,t_szFName);
		//MessageBox( NULL,t_szFName,"ERROR",MB_OK );

		WIN32_FIND_DATA    t_fdata;
		if( INVALID_HANDLE_VALUE == 
			::FindFirstFile( t_szEffFname.c_str(),&t_fdata ) )
			return false;

		sprintf( t_szFName,"%s","Ŀ¼�´��ڸ�Skin����Ч���ݣ�ɾ������Ч������?" );
		if( IDOK == MessageBox( NULL,t_szFName,"INFO",MB_OKCANCEL ) )
		{
			if( !::DeleteFile( t_szEffFname.c_str() ) ) 
				MessageBox( NULL,"ɾ���ļ�����","ERROR",MB_OK );
		}

		return false;
	}

	return true;

	unguard;
}

# endif 



