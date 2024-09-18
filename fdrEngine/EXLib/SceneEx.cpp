//===================================================================
/** \file  
 *  Filename: SceneEX.cpp
 *  Desc:     װ��EX�ļ����ļ���Ϣ���ࡣ
 *  His:      ���� @ 6/12 2003 16:18
 */
//==================================================================
# include "stdafx.h"
#include "sceneex.h"
# include "../interface/fileOperate.h"



# define   MYTGA_FILEVERSION   10
# define   MYTGA_MAGIC        "WT"

BOOL       g_bUseLDir = FALSE;

/** \brief
 *  �������Լ���ʽ��tga�ļ�ת��������Ҫ���ڴ�tga�ļ���ʽ.
 *
 *  ������ɵ��ļ���ʽת����*.HON�ļ�����.
 */
int    load_tgaFile( const char* _szFileName,BOOL& _alpha,BYTE* _mem,DWORD _maxLength,int& _dOffset  ,BOOL& _avoidMipMap)
{
	guard;

	char*   t_pBuffer = NULL;//����һ���ַ�ָ��
	char    t_szID[32];
	DWORD   t_fVersion;
	int     t_iTgaFlength;
	BYTE*   t_ptrStart;

	osassert( _szFileName );
	osassert( _mem );
	osassert( _maxLength>0 );

	_alpha=FALSE;
	
	_dOffset = 0;


	if( g_bUseLDir )
	{
		s_string   t_szName = _szFileName;

		int   t_idx = (int)t_szName.rfind( "\\" );
		if( (t_szName[0] == 's')&&(t_szName[1] == 'c') )
		{
			char  t_sz = t_szName[t_idx+1];
			if( (( t_sz >= 'a')&&(t_sz <= 'j')) ||
				(( t_sz >= 'A')&&(t_sz <= 'J')) )
				t_szName.insert( t_szName.rfind( "\\" )+1,"ta\\aj\\" );
			else
				if( (( t_sz >= 'k')&&(t_sz <= 't' )) ||
					(( t_sz >= 'K')&&(t_sz <= 'T' )))
					t_szName.insert( t_szName.rfind( "\\" )+1,"ta\\kt\\" );
				else
					t_szName.insert( t_szName.rfind( "\\" )+1,"ta\\uz\\" );
		}
	}

	t_ptrStart = _mem;
	int   t_iSize = read_fileToBuf( (char*)_szFileName,t_ptrStart,_maxLength );
	if( t_iSize <= 0 )
	{
		char temp[250];
		sprintf(temp,"%s�����ļ�ʱ�������������ؿͻ��˽��������",_szFileName);
		osassertex(false,temp);
		return -1;
	}

	// 
	// ���뵱ǰ���ļ���ID�Ƿ���ȷ.
	READ_MEM_OFF( t_szID,t_ptrStart,sizeof( char )*2 );
	t_szID[2] = NULL;
	osassertex( (0 == strcmp( t_szID ,MYTGA_MAGIC )),
		va( "������ļ���ʽ<%s>..�������������µĿͻ����޸�����\n",(char*)_szFileName) );

	_dOffset += sizeof( char )*2;

	READ_MEM_OFF( &t_fVersion,t_ptrStart,sizeof( int ) );
	osassertex( (t_fVersion >= MYTGA_FILEVERSION),
		va( "���������ļ�ʱ����<%s>..\n",_szFileName) );

	_dOffset += sizeof( int );
	READ_MEM_OFF( &t_iTgaFlength,t_ptrStart,sizeof( int ) );
	osassert( t_iTgaFlength>0 );
	_dOffset += sizeof( int );

	// 
	// ���뵱ǰ���ļ��Ƿ��Ǽ�����alphaͨ����tga�ļ�.
	READ_MEM_OFF( &_alpha,t_ptrStart,sizeof( BOOL ) );
	_dOffset += sizeof( BOOL );
	osassert( _maxLength > DWORD(t_iTgaFlength) );

	if(t_fVersion >= 12){
		READ_MEM_OFF(&_avoidMipMap,t_ptrStart,sizeof( BOOL ) );
		_dOffset += sizeof( BOOL );
	}

	if(t_fVersion == MYTGA_FILEVERSION){

		char    t_szTGAHeader[18];//����m_szTGAHeader�ַ����飬��СΪ18
		sprintf(t_szTGAHeader, "TRUEVISION-XFILE");
		t_szTGAHeader[16] = 46;
		t_szTGAHeader[17] = 0;
		memcpy( &t_ptrStart[t_iTgaFlength-18],t_szTGAHeader,sizeof( char )*18 );
	}

	return t_iTgaFlength;

	unguard;
}


//===================================================================================================
/** \class
 *  classname:  CSceneEx
 *  Desc:     ����װ��һ��EX�ļ���������Ϣ�ļ���
 *
 *       BOOL LoadEXFile(char* _chFileName);
 *       ����������һ��X�ļ���EX�ļ��з�����������ݱ�����m_XDataָ���С�
 *       ���Դ�m_sEXHeader.m_dwXSize���õ����ݵ��ֽڴ�С
 *            
 */
//===================================================================================================
///////////////////////
//obj_bound g_sBound;
/** \brief
 * ���캯��
 * 
 */
CSceneEx::CSceneEx(void)
{
	for(int n = 0; n< 8; n++)
		m_pBaseMesh[n] = NULL;
	m_XData = NULL;
	 m_pAttribute = NULL;
	 m_pTexName = NULL;

}

CSceneEx::~CSceneEx(void)
{
	Reset();
}

//===============================
/** \brief
 * �ͷ�ȫ����Դ

 *   ��ÿ��װ���µ��ļ�֮ǰʹ��
 */
void CSceneEx::Reset()
{
	int n;
	for(n = 0; n< 8; n++)
	{
		if(m_pBaseMesh[n])
		{
			delete []m_pBaseMesh[n];
			m_pBaseMesh[n] = NULL;
		}
	}

	if(m_pAttribute)
	{
		delete []m_pAttribute;
		m_pAttribute = NULL;
	}
	if(m_pTexName)
	{
		for(n = 0; n< (int)m_sObjectInfo.m_dwTexCount;n++)
		{
			delete []m_pTexName[n];
			m_pTexName[n] = NULL;
		}
		delete []m_pTexName ;
		m_pTexName = NULL;
	}

	ZeroMemory(&m_sBound, sizeof(obj_bound));

	ZeroMemory(&m_sEXHeader, sizeof(EX_HEADER));
	ZeroMemory(&m_sObjectInfo,sizeof(OBJECT_INFO));


}


//===============================
/** \brief
 * װ��һ��ex�ļ�
 * 
 * ��������ȶ�ȡ�ļ�ͷ��Ȼ��.x�ļ����ݱ������ڴ���
 * ���Դ�m_sEXHeader.m_dwXSize���õ����ݵ��ֽڴ�С
 *
 * \param _chFileName: 
 *     Ҫװ���ex�ļ���
 */
BOOL CSceneEx::LoadEXFile(char* _chFileName,LPDIRECT3DDEVICE9 _dev,
		     LPD3DXBUFFER& _mtrlBuf,DWORD& _numMtl,LPD3DXMESH& _pmesh )
{
	BYTE*   t_ptrStart;
	BYTE*   t_ptrMeshStart;
	int     t_iSize;

	if( g_bUseLDir )
	{
		s_string   t_szName = _chFileName;
		int   t_idx = (int)t_szName.rfind( "\\" );
		if( (t_szName[0] == 's')&&(t_szName[1] == 'c') )
		{
			char  t_sz = t_szName[t_idx+1];
			if( (( t_sz >= 'a')&&(t_sz <= 'j')) ||
				(( t_sz >= 'A')&&(t_sz <= 'J')) )
				t_szName.insert( t_szName.rfind( "\\" )+1,"a\\aj\\" );
			else
				if( (( t_sz >= 'k')&&(t_sz <= 't' )) ||
					(( t_sz >= 'K')&&(t_sz <= 'T' )))
					t_szName.insert( t_szName.rfind( "\\" )+1,"a\\kt\\" );
				else
					t_szName.insert( t_szName.rfind( "\\" )+1,"a\\uz\\" );
		}
	}

	int   t_iGBufIdx = -1;
	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)_chFileName,t_ptrStart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
	{
		osassertex( false,_chFileName );
		END_USEGBUF( t_iGBufIdx );
		return FALSE;
	}


	READ_MEM_OFF( &m_dwFileVersion,t_ptrStart, sizeof(DWORD) );	

	READ_MEM_OFF( &m_sEXHeader, t_ptrStart,sizeof(EX_HEADER) );

	//! ���������ڴ��Ϊd3dxMesh��ص��ڴ�顣
	t_ptrMeshStart = t_ptrStart;

	// 
	// ��*.x�ļ�������ת�Ƶ�tmpBuffer�������Ŀ�ͷ���֡�
	osassert( m_sEXHeader.m_dwXSize < TMP_BUFSIZE );
	t_ptrStart += m_sEXHeader.m_dwXSize;
	READ_MEM_OFF( &m_sBound,t_ptrStart,sizeof(obj_bound) );

	//
	// ATTENTION TO OPP: ȫ��ʹ���°汾���ݡ���
	// ����汾��2,�����µ����ݡ�
	m_sMeshVipInfo.m_bVipObj = false;
	m_sMeshVipInfo.m_bTreeObj = false;
	m_sMeshVipInfo.m_AcceptFog = false;
	m_sMeshVipInfo.m_bIsBridge = false;

	m_bUseDirLAsAmbient = FALSE;
	if( ( m_dwFileVersion==2 )||( m_dwFileVersion==3 ) )
	{
		READ_MEM_OFF( &m_sMeshVipInfo,t_ptrStart,sizeof( BOOL ) );

		// �洢������������ʾ�����˱��������෴��
		READ_MEM_OFF( &m_bUseDirLAsAmbient,t_ptrStart,sizeof( BOOL ) );
	}


	HRESULT   t_hr;
	// �˴����ڴ��е���d3dx���豸���mesh.
	t_hr = D3DXLoadMeshFromXInMemory( (void**)t_ptrMeshStart,
		 m_sEXHeader.m_dwXSize,D3DXMESH_MANAGED,
		 _dev,NULL ,&_mtrlBuf,NULL,&_numMtl,&_pmesh );
	if( FAILED( t_hr ) )
	{
		END_USEGBUF( t_iGBufIdx );
		osassertex(false,
			va( "%s���ڴ��д���*.x�ļ�ʧ��,ԭ��<%s>..\n",
			_chFileName,osn_mathFunc::get_errorStr( t_hr ) ) );

		return false;
	}

	END_USEGBUF( t_iGBufIdx );

	if( m_sMeshVipInfo.m_bIsBridge )
		osDebugOut( "TheBridge obj is:<%s>...\n",_chFileName );

	return TRUE;
}

//! ���ڴ���װ��һ��ex�ļ�
BOOL CSceneEx::loadEXFromMem( BYTE* _mem )
{
	guard;

	BYTE*   t_ptrStart;

	osassertex( _mem,"��Ʒ�ڴ治��Ϊ��...\n" );

	t_ptrStart = _mem;
	READ_MEM_OFF( &m_dwFileVersion,t_ptrStart, sizeof(DWORD) );	
	READ_MEM_OFF( &m_sEXHeader, t_ptrStart,sizeof(EX_HEADER) );

	// ����x�ļ��Ľ���Bounding����.
	m_XData = t_ptrStart;
	t_ptrStart += m_sEXHeader.m_dwXSize;
	READ_MEM_OFF( &m_sBound,t_ptrStart,sizeof(obj_bound) );

	return true;

	unguard;
}


