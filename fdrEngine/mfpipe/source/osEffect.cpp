//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osEffect.h
 *
 *   Desc:     O.S.O.K �����Effect������,���м������.
 *
 *   His:      River Created @ 4/27 2003
 *
 *  ΢���Ļ�: ����֪������¿���������������.��������ǿ��������ģ���
 *    ô�����ôȥ���أ� ����
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
//! River @ 2010-4-7:Ϊ�˼��ݵͶ˵��Կ�
# define PIXEL_SHADER_HIGH "PShader_pass_hig"
# define PIXEL_SHADER_LOW  "PShader_pass_low"

/** \brief
 *   �Ƿ�ʹ��shaderDebug���ܡ�
 *   ���ʹ��shaderDebug�Ļ�������effect��ʱ��Ͳ���ʹ���Ż���
 */
OSENGINE_API BOOL     g_bShaderDebug = FALSE;


//! ���ó�ʼ����effect��Ŀ��
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

//! ����beginpass��endpass֮�����Ⱦ״̬�޸�
void osc_effect::commit_change( void )
{
	guard;

	osassert( m_peffect );
	if( FAILED( m_peffect->CommitChanges() ) )
		osassertex( false,"Commit changes failed....\n" );

	unguard;
}




//! �滻��ǰҪ�����hlslShader�汾����ǰ����֧�ֵ�����shader�汾
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
		// ����vs_1_1�ؼ���,�滻�ɵ�ǰ��ƽ̨֧�ֵ����shader�汾
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

		//! ����ps_1_4�Ĺؼ��֣��滻����ߵİ汾
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

		//! ע���߶˰汾��ps.
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
		// ���û��vs_1_1�ı�־��������technique��־���˳�
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
		osassertex( false,va("����Effect�ļ�����<%s>..\n",_filename ) );
		return false;
	}else
		osDebugOut( "Create effect<%s>....\n",_filename );

	DWORD  t_flag;

	// River @ 2006-9-4:����Ҫ�����shaderƽ̨,���������������.
	// River @ 2009-3-24:ʹ���ϰ��vs
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
		// �˶δ������ڶ�λ�����λ��,��validateЧ���ڵ�Technique.
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
				
		// test ���Բ�֧��BUMP��MAP���Կ�,д�����룬�Ժ��Ż�
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
				// ���Գ������е��Կ��ϱ���GF2����ģ��е�״̬��ȷ��֧��.
				osDebugOut( "DX Error <%s>..Filename is:<%s>..\n",
					DXGetErrorDescription9(t_hr),_filename );
				osDebugOut(" ����������Կ���֧��BUMP MAP��ͼ!\n");
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


/// ��һ���ļ����豸ָ���ʼ��effect.
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
	// River @ 2010-4-3:�Ż���effect�ڲ���Ҫ��ô�ҵľ���
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

	// River added @ 2007-4-9:�Ƿ�ǿ��͸��:
	if( FAILED( m_peffect->GetBool( "forceTrans",&m_bForceTransparency ) ) )
		m_bForceTransparency = FALSE;

	return true;

	unguard;
}


/// ���õ�ǰEffect�õ��ĵ�n��texture
bool osc_effect::set_texture( int _tidx,LPDIRECT3DBASETEXTURE9 _tex )
{
	guard;

	HRESULT   t_hr;

	osassert( _tidx < (int)m_vTexHandle.size() );
	t_hr = m_peffect->SetTexture( m_vTexHandle[ _tidx ], _tex );
	if( FAILED( t_hr ) )
	{
		//osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		//! river @ 2011-10-14:ȥ����assert.���������Ϣ.
		osDebugOut( "SetTexture failed:<%s>...\n",
			      osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}

	return true;

	unguard;
}



/// �ͷ�һ��Effect.
void osc_effect::release_effect( void )
{
	guard;

	SAFE_RELEASE( m_peffect );

	unguard;
}

/// ��ʼ����ǰ��effectMgr
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


/// �����е�effect fileȫ���ͷŵ�
bool osc_effectMgr::release_effectMgr( bool _finalRelease/* = true*/ )
{
	m_arrEffectArray.destroy_mgr();

	if( !_finalRelease )
		m_arrEffectArray.resize( INIT_EFFECTNUM );

	return true;
}


/// �ͷ�EffectMgr���豸�������.
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


/// Reset���е��豸��ص�Effect.
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


/// ��effect manager�еõ�Effect��ָ��
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
*  ���������й����ж�̬�ļ���Effect.
*  
*  \return  �����¼����Effect��id,�������-1,����.
*/
int osc_effectMgr::dadd_effecttomanager( char* _filename )
{
	guard;

	int       t_idx;
	bool      t_b;

	s_string   t_strEffName = "data\\";
	t_strEffName += _filename;

	DWORD  t_dwHash = string_hash( (char*)t_strEffName.c_str() );

	// River: ���̼߳���shader����ʱ����Դ������
	::EnterCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );
	// 
	// ��������д���������Ҫ��effect,����Ҫ����.
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

	// �õ�Ҫ������effect����ʹ�õ�ָ��.
	t_idx = m_arrEffectArray.get_freeNode();
	m_arrEffectArray[t_idx].add_ref();
	::LeaveCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );

	// 
	// ���ļ��е����µ�effect�ļ�.
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

