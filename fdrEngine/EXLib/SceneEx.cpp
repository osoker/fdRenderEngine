//===================================================================
/** \file  
 *  Filename: SceneEX.cpp
 *  Desc:     装入EX文件和文件信息的类。
 *  His:      王凯 @ 6/12 2003 16:18
 */
//==================================================================
# include "stdafx.h"
#include "sceneex.h"
# include "../interface/fileOperate.h"



# define   MYTGA_FILEVERSION   10
# define   MYTGA_MAGIC        "WT"

BOOL       g_bUseLDir = FALSE;

/** \brief
 *  把我们自己格式的tga文件转化我们需要的内存tga文件格式.
 *
 *  卡安完成的文件格式转化和*.HON文件处理.
 */
int    load_tgaFile( const char* _szFileName,BOOL& _alpha,BYTE* _mem,DWORD _maxLength,int& _dOffset  ,BOOL& _avoidMipMap)
{
	guard;

	char*   t_pBuffer = NULL;//定义一个字符指针
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
		sprintf(temp,"%s调入文件时出错，请重新下载客户端解决此问题",_szFileName);
		osassertex(false,temp);
		return -1;
	}

	// 
	// 读入当前的文件的ID是否正确.
	READ_MEM_OFF( t_szID,t_ptrStart,sizeof( char )*2 );
	t_szID[2] = NULL;
	osassertex( (0 == strcmp( t_szID ,MYTGA_MAGIC )),
		va( "错误的文件格式<%s>..请重新下载最新的客户端修复错误\n",(char*)_szFileName) );

	_dOffset += sizeof( char )*2;

	READ_MEM_OFF( &t_fVersion,t_ptrStart,sizeof( int ) );
	osassertex( (t_fVersion >= MYTGA_FILEVERSION),
		va( "调入以下文件时出错<%s>..\n",_szFileName) );

	_dOffset += sizeof( int );
	READ_MEM_OFF( &t_iTgaFlength,t_ptrStart,sizeof( int ) );
	osassert( t_iTgaFlength>0 );
	_dOffset += sizeof( int );

	// 
	// 读入当前的文件是否是加入了alpha通道的tga文件.
	READ_MEM_OFF( &_alpha,t_ptrStart,sizeof( BOOL ) );
	_dOffset += sizeof( BOOL );
	osassert( _maxLength > DWORD(t_iTgaFlength) );

	if(t_fVersion >= 12){
		READ_MEM_OFF(&_avoidMipMap,t_ptrStart,sizeof( BOOL ) );
		_dOffset += sizeof( BOOL );
	}

	if(t_fVersion == MYTGA_FILEVERSION){

		char    t_szTGAHeader[18];//定义m_szTGAHeader字符数组，大小为18
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
 *  Desc:     用来装入一个EX文件和它的信息文件。
 *
 *       BOOL LoadEXFile(char* _chFileName);
 *       这个函数会把一个X文件从EX文件中分离出来，数据保存在m_XData指针中。
 *       可以从m_sEXHeader.m_dwXSize来得到数据的字节大小
 *            
 */
//===================================================================================================
///////////////////////
//obj_bound g_sBound;
/** \brief
 * 构造函数
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
 * 释放全部资源

 *   在每次装入新的文件之前使用
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
 * 装入一个ex文件
 * 
 * 这个函数先读取文件头。然后将.x文件数据保存在内存中
 * 可以从m_sEXHeader.m_dwXSize来得到数据的字节大小
 *
 * \param _chFileName: 
 *     要装入的ex文件名
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

	//! 接下来的内存块为d3dxMesh相关的内存块。
	t_ptrMeshStart = t_ptrStart;

	// 
	// 把*.x文件的数据转移到tmpBuffer缓冲区的开头部分。
	osassert( m_sEXHeader.m_dwXSize < TMP_BUFSIZE );
	t_ptrStart += m_sEXHeader.m_dwXSize;
	READ_MEM_OFF( &m_sBound,t_ptrStart,sizeof(obj_bound) );

	//
	// ATTENTION TO OPP: 全部使用新版本数据。　
	// 如果版本是2,读入新的数据。
	m_sMeshVipInfo.m_bVipObj = false;
	m_sMeshVipInfo.m_bTreeObj = false;
	m_sMeshVipInfo.m_AcceptFog = false;
	m_sMeshVipInfo.m_bIsBridge = false;

	m_bUseDirLAsAmbient = FALSE;
	if( ( m_dwFileVersion==2 )||( m_dwFileVersion==3 ) )
	{
		READ_MEM_OFF( &m_sMeshVipInfo,t_ptrStart,sizeof( BOOL ) );

		// 存储的是明暗的显示，跟此变量正好相反。
		READ_MEM_OFF( &m_bUseDirLAsAmbient,t_ptrStart,sizeof( BOOL ) );
	}


	HRESULT   t_hr;
	// 此处从内存中调入d3dx的设备相关mesh.
	t_hr = D3DXLoadMeshFromXInMemory( (void**)t_ptrMeshStart,
		 m_sEXHeader.m_dwXSize,D3DXMESH_MANAGED,
		 _dev,NULL ,&_mtrlBuf,NULL,&_numMtl,&_pmesh );
	if( FAILED( t_hr ) )
	{
		END_USEGBUF( t_iGBufIdx );
		osassertex(false,
			va( "%s从内存中创建*.x文件失败,原因<%s>..\n",
			_chFileName,osn_mathFunc::get_errorStr( t_hr ) ) );

		return false;
	}

	END_USEGBUF( t_iGBufIdx );

	if( m_sMeshVipInfo.m_bIsBridge )
		osDebugOut( "TheBridge obj is:<%s>...\n",_chFileName );

	return TRUE;
}

//! 从内存中装入一个ex文件
BOOL CSceneEx::loadEXFromMem( BYTE* _mem )
{
	guard;

	BYTE*   t_ptrStart;

	osassertex( _mem,"物品内存不能为空...\n" );

	t_ptrStart = _mem;
	READ_MEM_OFF( &m_dwFileVersion,t_ptrStart, sizeof(DWORD) );	
	READ_MEM_OFF( &m_sEXHeader, t_ptrStart,sizeof(EX_HEADER) );

	// 读入x文件的结束Bounding部分.
	m_XData = t_ptrStart;
	t_ptrStart += m_sEXHeader.m_dwXSize;
	READ_MEM_OFF( &m_sBound,t_ptrStart,sizeof(obj_bound) );

	return true;

	unguard;
}


