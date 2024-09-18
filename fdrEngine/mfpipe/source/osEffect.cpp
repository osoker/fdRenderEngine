//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osEffect.h
 *
 *   Desc:     O.S.O.K 引擎的Effect管理器,对中间管理开放.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  微软文化: “我知道这件事看来是做不到的了.但如果他是可以做到的，那
 *    么你会怎么去做呢？ ”。
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osEffect.h"
# include "../include/osTexture.h"
# include "../include/osShader.h"
# include "../../Terrain/include/fdFieldMgr.h"

# define VSVERSION_KEY  "vs_2_0"
# define PSVERSION_KEY  "ps_2_0"
# define TECHNIQUE_KEY  "technique"
//! River @ 2010-4-7:为了兼容低端的显卡
# define PIXEL_SHADER_HIGH "PShader_pass_hig"
# define PIXEL_SHADER_LOW  "PShader_pass_low"

/** \brief
 *   是否使用shaderDebug功能。
 *   如果使用shaderDebug的话，编译effect的时候就不能使用优化。
 */
OSENGINE_API BOOL     g_bShaderDebug = FALSE;


//! 设置初始化的effect数目。
# define INIT_EFFECTNUM    128

DWORD osc_effect::m_passFlag = D3DXFX_DONOTSAVESTATE;
unsigned int osc_effect::m_techniqueLevel = 0;

osc_effect::osc_effect()
{
	m_peffect = NULL;
	m_strEffFileName = "";
	m_dwHash = 0;
	m_curTechLevel = m_techniqueLevel;
}

osc_effect::~osc_effect()
{
	release_effect();
}

bool osc_effect::pass( unsigned int index )
{
	HRESULT   hr;

	hr = m_peffect->BeginPass( index );
	osassertex( !FAILED(hr),osn_mathFunc::get_errorStr( hr ) );

	if( m_vTecList[m_curTechLevel].m_vPassList[index].isVirtual )
	{
		m_peffect->EndPass();
		return true;
	}
	else
		return false;
}

//! end pass.
bool osc_effect::endPass( void )
{
	HRESULT hr = m_peffect->EndPass();
	osassert(SUCCEEDED(hr));
	return true;
}

//! 更新beginpass和endpass之间的渲染状态修改
void osc_effect::commit_change( void )
{
	guard;

	osassert( m_peffect );
	if( FAILED( m_peffect->CommitChanges() ) )
		osassertex( false,"Commit changes failed....\n" );

	unguard;
}




//! 替换当前要编译的hlslShader版本到当前机器支持的最新shader版本
bool osc_effect::update_toNewVS( BYTE* _data, LPDIRECT3DDEVICE9 _device,int _size )
{
	guard;

	osassert( _data );
	osassert( _device );
	osassert( _size >= 0 );

	BYTE*     t_ptrBt = _data;

	LPCSTR     t_szNewProfile = NULL;
	LPCSTR     t_szNewPsProfile = NULL;

	//! 
	int   t_iTimes = 0;

	for( int t_i=0;t_i<_size;t_i ++ )
	{
		// 
		// 对于vs_1_1关键字,替换成当前的平台支持的最高shader版本
		if( *t_ptrBt == VSVERSION_KEY[0] )
		{
			static int t_iLen = strlen( VSVERSION_KEY );
			if(  0 == memcmp( t_ptrBt,VSVERSION_KEY,t_iLen ) )
			{
				t_szNewProfile = D3DXGetVertexShaderProfile( _device );
				memcpy( t_ptrBt,t_szNewProfile,t_iLen );
				osDebugOut( "The target is:<%s>..\n",t_szNewProfile );
			}
		}

		//! 对于ps_1_4的关键字，替换成最高的版本
		if( *t_ptrBt == PSVERSION_KEY[0] )
		{
			static int t_iPsLen = strlen( PSVERSION_KEY );
			if(  0 == memcmp( t_ptrBt,PSVERSION_KEY,t_iPsLen ) )
			{
				t_szNewPsProfile = D3DXGetPixelShaderProfile( _device );
				memcpy( t_ptrBt,t_szNewPsProfile,t_iPsLen );
				osDebugOut( "The target PS is:<%s>..\n",t_szNewPsProfile );
			}
		}

		//! 注掉高端版本的ps.
		if( *t_ptrBt == PIXEL_SHADER_HIGH[0] )
		{
			static int t_iPsHighLen = strlen( PIXEL_SHADER_HIGH );
			if(  0 == memcmp( t_ptrBt,PIXEL_SHADER_HIGH,t_iPsHighLen ) )
			{
				if( t_iTimes != 0 )
				{
					memcpy( t_ptrBt,PIXEL_SHADER_LOW,t_iPsHighLen );
					osDebugOut( "Replace low effect....\n" );
				}
				else
					t_iTimes ++;
			}			
		}
		
		// 
		// 如果没有vs_1_1的标志，则碰到technique标志后退出
		if( *t_ptrBt == TECHNIQUE_KEY[0] )
		{
			static int t_iLen = strlen( TECHNIQUE_KEY );
			if( 0 == memcmp( t_ptrBt,TECHNIQUE_KEY,t_iLen ) )
				return true;
		}

		t_ptrBt ++;
	}

	return false;

	unguard;
}


bool osc_effect::create_effect( char* _filename, LPDIRECT3DDEVICE9 _device )
{
	guard;

	int      t_i;
	HRESULT  t_hr;

	osassert( _filename );
	osassert( _device );

	m_strEffFileName = _filename;
	m_dwHash = string_hash( _filename );

	// Read the Effect file into tmp buffer.
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );


	t_i = ::read_fileToBuf( _filename,t_ptrStart,TMP_BUFSIZE );
	if( t_i <= 0 )
	{
		END_USEGBUF( t_iGBufIdx );
		osassertex( false,va("调入Effect文件出错<%s>..\n",_filename ) );
		return false;
	}else
		osDebugOut( "Create effect<%s>....\n",_filename );

	DWORD  t_flag;

	// River @ 2006-9-4:更改要编译的shader平台,可能是人物的问题.
	// River @ 2009-3-24:使用老版的vs
	if( (g_iMajorVSVersion < 2) ||(g_iMajorPSVersion <2 ) )
		update_toNewVS( t_ptrStart,_device,t_i );

	if( g_bShaderDebug )
	{
		t_flag = D3DXSHADER_DEBUG 
			| D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT ;

		LPD3DXBUFFER     t_errorBuf;
		t_hr = D3DXCreateEffectFromFile( 
			_device,_filename,NULL,NULL,t_flag,NULL,&m_peffect,&t_errorBuf );
		if( FAILED( t_hr ) )
		{
			char*  t_str;
			t_str = (char*)t_errorBuf->GetBufferPointer();
			osassertex( false,t_str );
		}
	}
	else
	{
		// 此段代码用于定位出错的位置,并validate效果内的Technique.
		DWORD t_dwFlag = NULL;
		if( (g_iMajorVSVersion < 2) ||(g_iMajorPSVersion <2 ) )
			t_dwFlag = D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;

		LPD3DXBUFFER     t_errorBuf;
		t_hr = D3DXCreateEffect( _device, t_ptrStart,t_i,
			NULL,NULL,t_dwFlag,NULL,&m_peffect,&t_errorBuf );		
		if( FAILED( t_hr ) )
		{
			char*  t_str;
			t_str = (char*)t_errorBuf->GetBufferPointer();
			osassertex( false,t_str );
		}

		D3DXHANDLE  t_handle = NULL;
		t_handle = m_peffect->GetTechnique( 0 );
				
		// test 测试不支持BUMP　MAP的显卡,写死代码，以后优化
		if( strcmp(_filename,"data\\effect\\waterRefractionReflection.fx")==0 && 
			(!::g_bWaterReflect)  )
		{
			//!2 for LowerWater
			m_curTechLevel = 2;
		}

		if( FAILED( t_hr ) )
		{
			if( D3DERR_DEVICELOST != t_hr )
			{	
				// 可以出错在有的显卡上比如GF2级别的，有的状态的确不支持.
				osDebugOut( "DX Error <%s>..Filename is:<%s>..\n",
					DXGetErrorDescription9(t_hr),_filename );
				osDebugOut(" 可能是你的显卡不支持BUMP MAP贴图!\n");
				m_curTechLevel = 2;

			}
			else
			{
				END_USEGBUF( t_iGBufIdx );
				return true;
			}
		}
	}

	END_USEGBUF( t_iGBufIdx );

	return true;

	unguard;
}


/// 从一个文件和设备指针初始化effect.
bool osc_effect::init_effect( char* _filename,LPDIRECT3DDEVICE9 _device )
{
	guard;


	m_bForceTransparency = FALSE;

	//----------------- d3deffect -------------------
	if( !create_effect( _filename, _device ) )return false;

	char temp[16];
	//----------------- texture --------------------
	D3DXHANDLE hdl;
	m_vTexHandle.clear();
	for( int i = 0; i < 8; ++i )
	{
		sprintf( temp, "%s%d", TEXTUREPTR, i );
		hdl = m_peffect->GetParameterByName( 0, temp );
		if( hdl )
			m_vTexHandle.push_back( hdl );
		else
			break;
	}
	//---------------- matrix ---------------------
	m_vMatHandle.clear();
	for( int i = 0; i < 8; ++i )
	{
		sprintf( temp, "Matrix%d", i );
		hdl = m_peffect->GetParameterByName( 0, temp );
		if( hdl )
			m_vMatHandle.push_back( hdl );
		else
			break;
	}
	// River @ 2010-4-3:优化，effect内不需要那么我的矩阵
	for( int i=0;i< 4;i++ )
	{
		sprintf( temp,"Float2_%d",i );
		hdl = m_peffect->GetParameterByName( 0, temp );
		if( hdl )
			m_vFloat2Handle.push_back( hdl );
		else
			break;
	}


	D3DXEFFECT_DESC       effDesc;
	D3DXTECHNIQUE_DESC    tecDesc;
	D3DXPASS_DESC         pasDesc;
	std::string           pasName;

	//--------------- Technique ------------------
	m_vTecList.clear();

	m_peffect->GetDesc( &effDesc );
	unsigned int n, m;
	m_vTecList.resize( effDesc.Techniques );
	for( n = 0; n < effDesc.Techniques; ++n )
	{
		m_vTecList[n].handle = m_peffect->GetTechnique( n );
		m_peffect->GetTechniqueDesc( m_vTecList[n].handle, &tecDesc );
		m_vTecList[n].m_vPassList.resize( tecDesc.Passes );

		for( m = 0; m < tecDesc.Passes; ++m )
		{
			m_vTecList[n].m_vPassList[m].handle = m_peffect->GetPass( m_vTecList[n].handle, m );
			m_peffect->GetPassDesc( m_vTecList[n].m_vPassList[m].handle, &pasDesc );
			if( pasDesc.Name )
			{
				pasName = pasDesc.Name;
				std::transform( pasName.begin(), pasName.end(), pasName.begin(), tolower);
				if( pasName.find( "virtual" ) != -1 )
					m_vTecList[n].m_vPassList[m].isVirtual = true;
				else
					m_vTecList[n].m_vPassList[m].isVirtual = false;
			}
			else
			{
				m_vTecList[n].m_vPassList[m].isVirtual = false;
			}
		}
	}

	// River added @ 2007-4-9:是否强制透明:
	if( FAILED( m_peffect->GetBool( "forceTrans",&m_bForceTransparency ) ) )
		m_bForceTransparency = FALSE;

	return true;

	unguard;
}


/// 设置当前Effect用到的第n个texture
bool osc_effect::set_texture( int _tidx,LPDIRECT3DBASETEXTURE9 _tex )
{
	guard;

	HRESULT   t_hr;

	osassert( _tidx < (int)m_vTexHandle.size() );
	t_hr = m_peffect->SetTexture( m_vTexHandle[ _tidx ], _tex );
	if( FAILED( t_hr ) )
	{
		//osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		//! river @ 2011-10-14:去掉此assert.输出调试信息.
		osDebugOut( "SetTexture failed:<%s>...\n",
			      osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}

	return true;

	unguard;
}



/// 释放一个Effect.
void osc_effect::release_effect( void )
{
	guard;

	SAFE_RELEASE( m_peffect );

	unguard;
}

/// 初始化当前的effectMgr
bool osc_effectMgr::init_effectMgr( LPDIRECT3DDEVICE9 _device )
{
	m_pd3dDevice = _device;
	return true;
}


osc_effectMgr::osc_effectMgr()
{
	m_pd3dDevice = NULL;

	m_arrEffectArray.resize( INIT_EFFECTNUM );
}


/// 把所有的effect file全部释放掉
bool osc_effectMgr::release_effectMgr( bool _finalRelease/* = true*/ )
{
	m_arrEffectArray.destroy_mgr();

	if( !_finalRelease )
		m_arrEffectArray.resize( INIT_EFFECTNUM );

	return true;
}


/// 释放EffectMgr的设备相关数据.
void osc_effectMgr::eff_onLostDevice( void )
{
	CSpaceMgr<osc_effect>::NODE  t_ptrNode;
	for( t_ptrNode = m_arrEffectArray.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrEffectArray.next_validNode(&t_ptrNode) )
	{
		osassert( t_ptrNode.p->get_ref() > 0 );
		t_ptrNode.p->m_peffect->OnLostDevice();
	}

	return;
}


/// Reset所有的设备相关的Effect.
bool osc_effectMgr::eff_onResetDevice( void )
{
	guard;

	CSpaceMgr<osc_effect>::NODE  t_ptrNode;
	for( t_ptrNode = m_arrEffectArray.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrEffectArray.next_validNode(&t_ptrNode) )
	{
		osassert( t_ptrNode.p->get_ref() > 0 );
		t_ptrNode.p->m_peffect->OnResetDevice();
	}
	return true;

	unguard;
}


/// 从effect manager中得到Effect的指针
osc_effect* osc_effectMgr::get_effectfromid( int _id )
{
	guard;

	osassert( (_id>=0) );
	osassert( m_arrEffectArray.validate_id( _id ) );
	osassertex( m_arrEffectArray[_id].m_peffect, 
		va("Effect m_peffect <%d> is NULL.\n", _id) );

	return &m_arrEffectArray[_id];

	unguard;
}



/** \brief 
*  在引擎运行过程中动态的加入Effect.
*  
*  \return  返回新加入的Effect的id,如果返回-1,出错.
*/
int osc_effectMgr::dadd_effecttomanager( char* _filename )
{
	guard;

	int       t_idx;
	bool      t_b;

	s_string   t_strEffName = "data\\";
	t_strEffName += _filename;

	DWORD  t_dwHash = string_hash( (char*)t_strEffName.c_str() );

	// River: 多线程减少shader创建时的资源竞争。
	::EnterCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );
	// 
	// 如果数组中存在我们需要的effect,则不需要创建.
	CSpaceMgr<osc_effect>::NODE  t_ptrNode;
	for( t_ptrNode = m_arrEffectArray.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrEffectArray.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->get_ref() > 0 );
		if( t_ptrNode.p->m_dwHash == t_dwHash )
		{
			osassert( t_ptrNode.p->m_strEffFileName == t_strEffName );
			t_ptrNode.p->add_ref();
			::LeaveCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );
			return t_ptrNode.idx;
		}
	}

	// 得到要创建的effect可以使用的指针.
	t_idx = m_arrEffectArray.get_freeNode();
	m_arrEffectArray[t_idx].add_ref();
	::LeaveCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );

	// 
	// 从文件中调入新的effect文件.
	t_b = m_arrEffectArray[t_idx].init_effect( (char*)t_strEffName.c_str(),m_pd3dDevice );
	if( !t_b )
	{
		osassert( false );
		return -1;
	}
	

	return t_idx;

	unguard;
}


bool osc_effect::set_textureById( int _idx, int _texId )
{
	guard;
	return set_texture( _idx, g_shaderMgr->getTexMgr()->get_texfromid( _texId ) );
	unguard;
}

