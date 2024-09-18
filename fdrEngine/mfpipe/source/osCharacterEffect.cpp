///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterEffect.cpp
 * 
 *  His:      River created @ 2004-12-15
 *
 *  Desc:     引擎中人物身上第二层贴图相关的特效函数。第二层的贴图可能来自环境贴图，
 *            来自矩阵纹理转化等等，在不改动现有数据的基础上，加入自动产生的第二层
 *            纹理数据。
 *  
 *  "靡不有初，鲜克有终" 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/frontPipe.h"
# include "../../mfpipe/include/osStateBlock.h"
# include "../../backpipe/include/mainEngine.h"


//! 是否在场景中显示人物的第二层贴图,在没有硬件VertexShader的机器上，
//! 不管此值是否为真，都不能显示第二层贴图
OSENGINE_API BOOL     g_bUseSecondTexOnCha = TRUE;

//! 第二层贴图的文件版本号.
//# define  CHASEC_EFFVERSION   101
//! river @ 2010-3-11:加入第四层贴图。
# define  CHASEC_EFFVERSION   102

os_chaSecEffect::os_chaSecEffect()
{
	m_iSecTexId = -1;
	m_iShaderId = -1;
	//!第三层通道用,用于指定第二层的透明度
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

//! 删除设备相关的数据
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



//! 每一帧计算当前贴图特效的内部数据。
void os_chaSecEffect::frame_moveSecEff( void )
{
	float   t_f = sg_timer::Instance()->get_lastelatime();

	m_fTexmoveTime += t_f;

	if( this->m_wEffectType == 6 )
	{
		m_matFinal._41 = m_fUMoveSpeed*m_fTexmoveTime;
		m_matFinal._42 = m_fVMoveSpeed*m_fTexmoveTime;
	}

	// River @ 2010-3-13:自发光与流动纹理结合.
	if( m_wEffectType == 9 )
	{
		// 流动
		m_matFinal._41 = m_fUMoveSpeed*m_fTexmoveTime;
		m_matFinal._42 = m_fVMoveSpeed*m_fTexmoveTime;

		// 自发光
		int t_i = int(m_fTexmoveTime/m_fRotSpeed);
		t_f = float_mod( m_fTexmoveTime,m_fRotSpeed );
		if( t_i%2 == 1 )
			t_f = m_fRotSpeed-t_f;
		t_f /= m_fRotSpeed;
		m_matFinal._43 = t_f;
	}

	//! river @ 2010-2-25: 人物身上的自发光纹理
	//! river @ 2010-3-13: 加入自发光效果与环境光的合体
	if( (m_wEffectType == 7)||( m_wEffectType == 10) )
	{
		int t_i = int(m_fTexmoveTime/m_fRotSpeed);
		t_f = float_mod( m_fTexmoveTime,m_fRotSpeed );
		if( t_i%2 == 1 )
			t_f = m_fRotSpeed-t_f;
		t_f /= m_fRotSpeed;

		m_matFinal._43 = t_f;
	}

	//  跟m_wEffectType == 6相同的计算，可优化。
	//! river @ 2010-3-11:加入rotsca和env相结合的装备装备
	if( m_wEffectType == 8 )
	{
		t_f = m_fTexmoveTime * m_fRotSpeed;
		osMatrixRotationAxis( &m_matFinal,&m_vec3RotAxis,t_f );
		m_matFinal._41 = m_fUMoveSpeed*m_fTexmoveTime;
		m_matFinal._42 = m_fVMoveSpeed*m_fTexmoveTime;
	}


	return;
}


//! 释放身体某一部分对应的skinShader.
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


//! 创建特殊的三层贴图旗帜Shader
bool osc_skinMesh::create_mlFlagShader( int _idx,char* _sname )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=7) );
	osassert( _sname&&(_sname[0]) );

	m_vecBpmIns[_idx].m_iShaderID = 
		g_shaderMgr->add_shader( _sname, "ini\\mlFlag.sdr" );

	osassertex( m_vecBpmIns[_idx].m_iShaderID >= 0 ,_sname );

	// 旗帜的Shader在非Alpha层级进行渲染
	m_vecBpmIns[_idx].m_bAlpha = true;

	return true;

	unguard;
}



//! 创建普通的单层纹理人物渲染效果。
bool osc_skinMesh::create_singleTexShader( int _idx,char* _sname )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=7) );
	osassert( _sname&&(_sname[0]) );


	//River @2010-12-28:记录shader的hashId.如果缓存中有可以直接使用的shaderId,则直接使用
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
	// 得到当前shaderId对应的纹理，用于IndexedBlend渲染。
	m_vecBpmIns[_idx].m_ptrBpmTexture = 
		g_shaderMgr->get_shaderTex( m_vecBpmIns[_idx].m_iShaderID );

	// 
	// Get Whether The alpha channel is exist in current Shader.
	m_vecBpmIns[_idx].m_bAlpha = 
		g_shaderMgr->has_alphaChannel( m_vecBpmIns[_idx].m_iShaderID );

	return true;

	unguard;
}

//! 人物的第二层贴图数据也使用预调入的形式
VEC_secEffProLoad osc_skinMesh::m_vecSecEffPreLoad;
int osc_skinMesh::m_iSecEffPLNum = 0;


//! 读入第二层特效纹理的数据。
bool osc_skinMesh::read_secondTexEff( os_chaSecEffect* _seceff,char* _sname )
{
	guard;

	osassert( _seceff );
	osassert( _sname );

	s_string         t_strName;
	t_strName = _sname;
	t_strName += ".cse";

	// River @ 2010-12-22:
	// 如果可以在预读取内找到，则不需要重复读取.
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
	// 读入数据到内存。
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)t_strName.c_str(),t_fstart,TMP_BUFSIZE );
	if( t_iSize < 0 )
	{
		osassertex( false,"文件读入错误....\n" );
		return false;
	}
	

	// 读入文件头与版本号.
	READ_MEM_OFF( t_szHeader,t_fstart,sizeof( char )*4 );
	if( strcmp( t_szHeader,CHASEC_EXTENT ) )
	{
		osassertex( false,"错误的文件头....\n" );
		return false;
	}
	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	if( t_iSize == 100 )
	{
		// 读入数据。
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
	// 加入到预读取的库内.
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

//! 人物的第二层贴图指针修改为内存池模式
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


//! 创建加入特效的双层纹理人物渲染效果。
bool osc_skinMesh::create_doubleTexShader( int _idx,char* _sname )
{
	guard;

	osassert( (_idx>=0)&&(_idx<=8) );
	osassert( _sname&&(_sname[0]) );


	//
	// 先读入第二层贴图的配置文件，
	int              t_iTexid;
	os_chaSecEffect*  t_ptrSecEff;


	//
	// 分配内存。
	t_ptrSecEff = get_sceEffectPtr();//new os_chaSecEffect;
	m_vecBpmIns[_idx].m_ptrSecEffect = t_ptrSecEff;

	if( !read_secondTexEff( t_ptrSecEff,_sname ) )
		return false;

	//River @2010-12-28:记录shader的hashId.如果缓存中有可以直接使用的shaderId,则直接使用
	m_vecBpmIns[_idx].m_dwHashId = string_hash( _sname );
	m_vecBpmIns[_idx].m_iShaderID = 
		osc_bpmInstance::get_bufShaderId( m_vecBpmIns[_idx].m_dwHashId );

	//
	// 创建第一层贴图和shader.
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
	// 得到当前shaderId对应的纹理，用于IndexedBlend渲染。
	m_vecBpmIns[_idx].m_ptrBpmTexture = 
		g_shaderMgr->get_shaderTex( m_vecBpmIns[_idx].m_iShaderID );
	t_ptrSecEff->m_iShaderId = m_vecBpmIns[_idx].m_iShaderID;

	// 
	// Get Whether The alpha channel is exist in current Shader.
	m_vecBpmIns[_idx].m_bAlpha = 
		g_shaderMgr->has_alphaChannel( m_vecBpmIns[_idx].m_iShaderID );

	//
	// 创建第二层纹理.
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
//   人物第二层纹理编辑相关的函数。
//
# if __CHARACTER_EFFEDITOR__


//! 存储当前的effect数据,文件需要自动获得
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
	
	// River @ 2010-3-11:人物的第四层纹理。
	fwrite( m_szFourthTexName,sizeof( char ),64,t_file );

	fclose( t_file );

	return;

	unguard;
}



// 人物身体部位第二层贴图的特效编辑相关函数
/** \brief
*  更新身体某一部位的纹理贴图效果。
*
*  如果身体部位原来没有第二层纹理，则更新为使用第二层纹理。
* 
*  \param _seceff 如果传入的值为空，如果原来此身体部位使用特效，则更新为不使用特效。
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

	// 如果原来的skin没有特效，而新的特效结构为空，返回。
	if( (_seceff == NULL)&&(t_ptrIns->m_ptrSecEffect == NULL) )
		return;

	m_vecBpmIns[_idx].release_insShader( m_vecBpmIns[_idx].m_iShaderID,true );

	m_vecBpmIns[_idx].m_iShaderID = -1;
	sprintf( t_szTexName,"%s\\%d_%02d_%02d",m_strSMDir,
		_idx+1,m_Look.m_mesh[t_idx],m_Look.m_skin[t_idx] );

	// 删除原身本部位的第二层贴图特效。
	if( NULL == _seceff )
	{
		if( t_ptrIns->m_ptrSecEffect == NULL )
			return;
		
		//River @2010-12-28:记录shader的hashId.如果缓存中有可以直接使用的shaderId,则直接使用
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

	// 更新身体部位的第二层贴图特效。
	if( NULL == t_ptrIns->m_ptrSecEffect )
		t_ptrIns->m_ptrSecEffect = get_sceEffectPtr();//new os_chaSecEffect;
	

	t_ptrEff = t_ptrIns->m_ptrSecEffect;
	if( t_ptrEff->m_iSecTexId>=0 )
		g_shaderMgr->release_texture( t_ptrEff->m_iSecTexId );

	t_ptrEff->m_iSecTexId = g_shaderMgr->create_texture( _seceff->m_szSecTexName );

	//River @2010-12-28:记录shader的hashId.如果缓存中有可以直接使用的shaderId,则直接使用
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

	//! river @ 2010-3-11:第四张贴图 
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

//! 得到当前身体部位的chaSecEffect数据。
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


//! 存储身体第_idx部位的第二层纹理贴图效果。
bool osc_skinMesh::save_chaSecEffect( int _idx )
{
	guard;

	char     t_szFName[64];
	s_string  t_szEffFname;
	int       t_iIdx;

	osassert( (_idx >=0)&&(_idx<=7) );
	t_iIdx = _idx;

	// 根据当前的部位和文件名来得要要存储的文件字。
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
			"人物<%s>的第<%d>部分没有第二层特效数据...\n",m_strSMDir,_idx );
		osassertex(false,t_szFName);
		//MessageBox( NULL,t_szFName,"ERROR",MB_OK );

		WIN32_FIND_DATA    t_fdata;
		if( INVALID_HANDLE_VALUE == 
			::FindFirstFile( t_szEffFname.c_str(),&t_fdata ) )
			return false;

		sprintf( t_szFName,"%s","目录下存在该Skin的特效数据，删除此特效数据吗?" );
		if( IDOK == MessageBox( NULL,t_szFName,"INFO",MB_OKCANCEL ) )
		{
			if( !::DeleteFile( t_szEffFname.c_str() ) ) 
				MessageBox( NULL,"删除文件出错","ERROR",MB_OK );
		}

		return false;
	}

	return true;

	unguard;
}

# endif 



