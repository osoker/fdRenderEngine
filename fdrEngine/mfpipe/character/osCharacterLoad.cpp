///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterLoad.h
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     用于os引擎中的人物动画相关文件的调入处理。
 *  
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../../interface/miskFunc.h"
# include "../../mfPipe/include/osEffect.h"
# include "../../interface/stringTable.h"

//! 当前act文件的版本号.
# define  ACT_FILEVERSION   100

//! 2004-4-16: 更新文件结构，使用quat和offset表示动作。
# define  ACT_FILEVERSION11  111

//! 2005-3-4:  动作文件版本升级,把人物动作矩阵的Flip相关数据加入.
# define  ACT_FILEVERSION12  112

/*
//! 当前skn mesh文件的版本号.
# define SKN_FILEVERSION  100
*/
//! 从100到101的版本：修改了skinMesh中顶点Normal不对的bug，使用
//! 场景中的人物可以使用方向光。
# define SKN_FILEVERSION  101

//! 定义人物的目录名长度为7
# define SKN_DIRNAMELEN   7


//! 从一个骨骼ID得到骨骼的名字.
const char* os_boneSys::get_boneNameFromId( int _id )
{
	guard;

	for( int t_i=0;t_i<this->m_iBoneNum;t_i ++ )
	{
		if( this->m_vecBoneIdx[t_i].m_iBoneIdx == _id )
			return m_vecBoneNameStr[t_i];
	}
	return NULL;

	unguard;
}

//! 从一个骨骼ID得到此骨骼父骨的名字.
const char* os_boneSys::get_parentBoneNameFromId( int _id )
{
	guard;

	for( int t_i=0;t_i<this->m_iBoneNum;t_i ++ )
	{
		if( m_vecBoneIdx[t_i].m_iBoneIdx == _id )
		{
			if( m_vecBoneIdx[t_i].m_iParentIdx != -1 )
				return m_vecBoneNameStr[m_vecBoneIdx[t_i].m_iParentIdx];
		}
			
	}
	return NULL;

	unguard;
}

/** \brief
 *  从文件中调入一个骨骼系统。
 */
bool os_boneSys::load_boneSysFromFile( const char* _fname )
{
	guard;

	int   t_iSize;
	char  t_str[256];
	BYTE*  t_fstart;

	// Catch bone sys name.
	m_strBoneSysName = _fname;

	//
	// 先把文件读入到内存中。
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)_fname,t_fstart,TMP_BUFSIZE );
	osassertex( t_iSize>0,(char*)_fname );

	
	// 
	// 对内存进行操作，读取我们需要的文件数据
	READ_MEM_OFF( &this->m_iBoneNum,t_fstart,sizeof( int ) );
	osassert( m_iBoneNum > 0 );

	m_vecBoneIdx.resize( m_iBoneNum );
	m_vecBoneNameStr.resize( m_iBoneNum );

	for( int t_i=0;t_i<m_iBoneNum;t_i++ )
	{
		READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
		READ_MEM_OFF( t_str,t_fstart,t_iSize+1 );
		//!骨骼名字大小写敏感
		m_vecBoneNameStr[t_i] = StringTable->insert(t_str,true);
	}

	// 
	// 调入boneId的操作。
	READ_MEM_OFF( &m_vecBoneIdx[0],t_fstart,sizeof( os_boneIdx )*m_iBoneNum );

	END_USEGBUF( t_iGBufIdx );

	m_dwHash = string_hash( _fname );

	return true;

	unguard;
}


os_boneAction::os_boneAction()
{
	m_iStopFrameIdx = 1;
	m_iBoneNum = 0;
	m_iNumFrame = 0;
	m_fTimeToPlay = 1.0f;
	m_iPlaySpeed = 1;
	m_szActName[0] = NULL;
}


/** \brief
*  从文件中调入一个骨骼的bone的动作文件。
*  
*  \param _fname  要调入的动作文件名.
*/
bool os_boneAction::load_boneActFromFile( const char* _fname )
{
	guard;

	int   t_iSize;
	char  t_str[256];
	BYTE*  t_fstart;

	// 
	osassert( _fname );
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx ); 
	t_iSize = read_fileToBuf( (char*)_fname,t_fstart,TMP_BUFSIZE );
	if( t_iSize<=0 )
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}

	//
	// 读入文件头.
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,ACT_FILEMAGIC ) != 0 )
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
	if( t_iSize < ACT_FILEVERSION11 )
	{
		END_USEGBUF( t_iGBufIdx );
		MessageBox( NULL,"出现了错误的动作文件版本",_fname,MB_OK );
		osassert( false );
		return false;
	}

	//
	// 读入骨骼和动画帧数。
	READ_MEM_OFF( &m_iBoneNum,t_fstart,sizeof( int ) );
	osassert( m_iBoneNum > 0 );
	READ_MEM_OFF( &m_iNumFrame,t_fstart,sizeof( int ) );
	osassert( m_iNumFrame > 0 );

	//
	// 
	m_vecMatrix.resize( m_iNumFrame*m_iBoneNum );
	

	// ATTENTION TO OPP:
	if( t_iSize == ACT_FILEVERSION11 )
	{
		for( int t_i=0;t_i<m_iNumFrame*m_iBoneNum;t_i ++ )
		{
			READ_MEM_OFF( &m_vecMatrix[t_i],t_fstart,
				sizeof( os_bactPose )-sizeof( BOOL ) );
			m_vecMatrix[t_i].m_bFlipDeterminant = FALSE;
		}
	}
	else if( t_iSize == ACT_FILEVERSION12 )
	{
		READ_MEM_OFF( &m_vecMatrix[0],t_fstart,
			sizeof( os_bactPose )*m_iNumFrame*m_iBoneNum );
	}
	else
	{
		osassert( false );
	}

	END_USEGBUF( t_iGBufIdx );

	return true;

	unguard;
}





/** \brief 
 *  王凯的动作保存文件中使用的数据结构。
 */
struct t_action
{
	int index;
	int  m_iSpeed;

	//! 停止帧数据。
	int  m_iStopFrameIdx;
};


/** \brief
*  从文件中调入骨骼动作的信息文件，包括每一个动作的名字，每一个动作的播放速度等。
*
*  \param _actIfname  传入的动作信息文件名。
*/
bool os_boneAni::load_actionInfoFile( const char* _actIfname )
{
	guard;

	int       t_iSize,t_iVersion;
	char      t_szMagic[4];
	//BYTE*     t_fstart;
	t_action   t_maxAct;
	//@{
	// Windy mod @ 2005-8-20 14:39:02
	//Desc: 人物编辑器，可以没有ACS文件。避免加载时出错.
	if (!file_exist( (char *)_actIfname )){
		return false;
	}
	//@}
	osassert( _actIfname );

	// tzz add:
	// 使用CReadFile 来读取内容,
	// 防止在osassert 抛出异常之后没有 END_USEGBUF（有太多的osassert）
	// 导致程序不能正常终止,需要用资源管理器来终止
	//
	// 查看 CReadFile 获得更多的信息
	// 
	//t_fstart = START_USEGBUF( t_iGBufIdx );
	//t_iSize = read_fileToBuf( (char*)_actIfname,t_fstart,TMP_BUFSIZE );
	//if( t_iSize<=0 )
	//	return false;

	CReadFile t_file;
	if(!t_file.OpenFile(_actIfname)){
		return false;
	}

	//READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	t_file.ReadBuffer(t_szMagic,sizeof( char )*4);

	if( strcmp( t_szMagic,ACTINFO_FILEMAGIC ) )
	{
		osassert( false );
		return false;
	}

	// 
	// 目前支持的最高版本号。
	//READ_MEM_OFF( &t_iVersion,t_fstart,sizeof( DWORD ) );
	t_file.ReadBuffer(&t_iVersion,sizeof( DWORD ));

	// 如果版本号大于1,文件中加入了停止帧索引的数据。	

	// 如果版本号代低于3,报警。
	if( t_iVersion < 3 )
		osDebugOut( "\n\nWARNNING: 人物动作版本号低于3,是旧的版本!!!!!!!!!\n\n" );


	//READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
	t_file.ReadBuffer(&t_iSize,sizeof( DWORD ));

	osassert( t_iSize > 0 );
	m_vecActName.resize( t_iSize );

	// 
	// 读入每一个动作名字对应的动作信息。
	for( int t_i=0;t_i<t_iSize;t_i++ )
	{
		// 读入当前动作名字对应的实际动作索引和实际动作数目。
		//READ_MEM_OFF( &m_vecActName[t_i].m_iActNum,t_fstart,sizeof( int ) );
		//READ_MEM_OFF( m_vecActName[t_i].m_szActName,t_fstart,sizeof( char )*ACT_NAMELEN );
		t_file.ReadBuffer(&m_vecActName[t_i].m_iActNum,sizeof( int ));
		t_file.ReadBuffer(m_vecActName[t_i].m_szActName,sizeof( char )*ACT_NAMELEN);
	
		osassertex( (m_vecActName[t_i].m_iActNum==1),
			     "在新的规范中,每一个动作名字只能对应一个动作.." );
		
		// River @ 2010-6-22:加入隐蔽武器的动作数据,引擎内使用.
		if( t_iVersion == 4 )
			t_file.ReadBuffer( &m_vecActName[t_i].m_bHideWeapon,sizeof( BOOL ) );

		// TEST CODE:
		os_actNameStruct* t_testAct = &m_vecActName[t_i];

		
		// 根据不同的版本号处理不同的调入。
		if( t_iVersion > 1 )
		{
			// 低于1的人物动作版本，没有停止帧索引，不作处理。
			//READ_MEM_OFF( &t_maxAct,t_fstart,sizeof( t_action ) );
			t_file.ReadBuffer(&t_maxAct,sizeof( t_action ));

		}
		else
		{
			//READ_MEM_OFF( &t_maxAct.index,t_fstart,sizeof( int ) );
			//READ_MEM_OFF( &t_maxAct.m_iSpeed,t_fstart,sizeof( int ) );
			t_file.ReadBuffer(&t_maxAct.index,sizeof( int ));
			t_file.ReadBuffer(&t_maxAct.m_iSpeed,sizeof( int ));
		}

# ifdef _DEBUG
		if( m_vecActName[t_i].m_iActNum != 1 )
		{
			char    t_szInfo[256];
			sprintf( t_szInfo,"动作 <%s> 对应了零或是多个动作...\n",
				m_vecActName[t_i].m_szActName );
			MessageBox( NULL,t_szInfo,"对应动作问题",MB_OK );

			return false;
		}
# endif

		// TEST CODE: 为输出调试信息，最终版本要去掉
		if( t_maxAct.index >= m_iBANum )
		{
			osDebugOut( "\n 注意!!!        动作<%s>对应的动作索引值<%d>超出了*.cha文件内部的索引数目<%d>...\n",
				m_vecActName[t_i].m_szActName,t_maxAct.index,m_iBANum );

			//END_USEGBUF( t_iGBufIdx );
			// t_file (CReadFile) 析构函数调用 END_USEGBUF
			osassert( false );

			return false;
		}

		osassertex( t_maxAct.index < m_iBANum,"动作对应的索引值过大...\n" );
		m_vecActName[t_i].m_vecActIdx = t_maxAct.index;
		osassertex( (t_maxAct.index>=0)&&(t_maxAct.index<m_iBANum),
			"动作名字的动作索引超出正常数据范围...\n" );
		os_boneAction*  t_ptr = &m_vecBATable[t_maxAct.index];


		t_ptr->m_iPlaySpeed = t_maxAct.m_iSpeed;
		osassert( t_ptr->m_iPlaySpeed>0 );

		//
		// 计算完成当前的动作需要的时间。
		t_ptr->m_fTimeToPlay = 
			float( t_ptr->m_iNumFrame ) / float( t_ptr->m_iPlaySpeed );

		// 
		// 动作停止帧索引。
		// River mod @ 2009-8-17:使用一个简单的约定，来处理有可能某个
		// 动作名字没有处理stopFrameIdx的情形
		if( t_ptr->m_iStopFrameIdx < t_maxAct.m_iStopFrameIdx )
			t_ptr->m_iStopFrameIdx = t_maxAct.m_iStopFrameIdx;
	}

	//END_USEGBUF( t_iGBufIdx );
	// t_file (CReadFile) 析构函数调用 END_USEGBUF

	return true;

	unguard;
}

//! River @ 2010-7-1: 当前的动作名字是否需要隐藏武器.
bool os_boneAni::is_actNameHideWeapon( const char* _actName )
{
	guard;

	os_actNameStruct*  t_ptrActName;

	osassert( _actName&&(_actName[0]) );

	bool     t_bHide = false;
	// 
	// 找到我们指定的动作名字，然后使用这个名字，得到动作第一个动作的索引。
	for( int t_i=0;t_i<(int)m_vecActName.size();t_i ++ )
	{
		t_ptrActName = &m_vecActName[t_i];
		if( strcmp( _actName,t_ptrActName->m_szActName ) == 0 )
		{
			osassert( t_ptrActName->m_iActNum>0 );

			t_bHide = t_ptrActName->m_bHideWeapon;

			// 找到.
			break;

		}
	}

	return t_bHide;

	unguard;
}


//! 从一个动作名字得到一个动作的索引。
int os_boneAni::get_actIdxFromName( const char* _actName,int& _nameIdx )
{
	guard;

	os_actNameStruct*  t_ptrActName;
	int               t_iActIdx = -1;

	osassert( _actName&&(_actName[0]) );

	// 
	// 找到我们指定的动作名字，然后使用这个名字，得到动作第一个动作的索引。
	for( int t_i=0;t_i<(int)m_vecActName.size();t_i ++ )
	{
		t_ptrActName = &m_vecActName[t_i];
		if( strcmp( _actName,t_ptrActName->m_szActName ) == 0 )
		{
			osassert( t_ptrActName->m_iActNum>0 );
			_nameIdx = t_i;

			t_iActIdx = t_ptrActName->m_vecActIdx;

			// 找到.
			break;

		}
	}

	return t_iActIdx;

	unguard;
}




//! 从一个actNameIdx和这个名字内动作的索引，得到一个bone Act Table.
int os_boneAni::get_actBoneAction( int _nameIdx )
{
	guard;

	osassert( (_nameIdx>=0) );

	os_actNameStruct*  t_ptrActName;

	t_ptrActName = &m_vecActName[_nameIdx];

	return t_ptrActName->m_vecActIdx;

	unguard;
}





/** \brief
*  从动作文件中得到新的动作数据。
*  
*  \param _id 因为动作文件使用了命名准则，所以只需要
*             给出_id就可以了得到新的动作。
*/
bool os_boneAni::load_boneAction( const char* _actName,int _actNum )
{
	guard;

	int        t_i,t_idx,t_iLength;
	s_string   t_str,t_str1;
	char       t_sz[16];

	m_iBANum = _actNum;
	osassertex( strlen( _actName )<32 ,_actName );
	strcpy( m_szAniName,_actName );
	m_dwHash = string_hash( _actName );
	m_vecBATable.resize( _actNum );


	t_str = _actName;
	t_idx = (int)t_str.rfind( '\\' );
	t_iLength = (int)t_str.length();
	t_str1 = '\\';
	t_str1 += t_str.substr( t_idx+1,t_iLength-t_idx-1 );
	t_str1 = t_str + t_str1;

	for( t_i=0;t_i<_actNum;t_i++ )
	{
		//
		// 根据现在的ActionName,调入所有的动作文件
		t_str = t_str1;

		if( t_i<9 )
			sprintf( t_sz,"0%d",t_i+1 );
		else
			sprintf( t_sz,"%d",t_i+1 );

		t_str += t_sz;
		t_str += ACT_EXTENT;

		if( !m_vecBATable[t_i].load_boneActFromFile( t_str.c_str() ) )
			return false;
	}


	//
	// 处理文件*.act文件名，使调入*.acs文件。
	t_str1 += ACS_EXTENT;



	if( !load_actionInfoFile( t_str1.c_str() ) )
	{
		for( t_i=0;t_i<_actNum;t_i++ )
		{	
			m_vecBATable[t_i].m_iPlaySpeed = 18;
			m_vecBATable[t_i].m_fTimeToPlay = 
				float(m_vecBATable[t_i].m_iNumFrame)/float(m_vecBATable[t_i].m_iPlaySpeed);
		}

		//@{
		// Windy mod @ 2005-8-26 9:31:38
		//Desc: 在人物编辑器中默认没有ACS文件，所以加载不成功也要返回真，否则运行不下去
		return true;
		//@}	
	}

	return true;

	unguard;
}



/** \brief
*  调入*.skn文件的顶点和索引数据。
*
*  此函数假设传入的两个缓冲区的指针已经分配数据，并且空间大小足够大。
*/
bool osc_bodyPartMesh::LoadMeshVNI( char* _file,BYTE* _vbuf,BYTE* _ibuf )
{
	guard;

	osassert( _file&&(_file[0]) );
	osassert( _vbuf );
	osassert( _ibuf );

	int   t_iSize;
	char  t_str[256];
	BYTE*  t_fstart;

	osassert( _file );

	//
	// 读入数据到内存。
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( _file,t_fstart,TMP_BUFSIZE );
	if( t_iSize < 0 )
	{
		osassert( false );
		return false;
	}
	

	//
	// 读入文件头. 
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,SKN_FILEMAGIC ) != 0 )
	{
		osDebugOut( "<%s> file Format error!!!",_file );
		osassert( false );
		return false;
	}

	//
	// 读入文件信息。
	t_fstart += (sizeof( DWORD )*9 );

	//
	// 调入每一个骨骼的matrix.
	t_fstart += ( sizeof( osMatrix )*m_dwNumPalette );
	t_fstart += ( sizeof( DWORD )*m_dwNumPalette );


	//
	// 创建设备相关的资源.
	READ_MEM_OFF( _vbuf,t_fstart, m_dwNumVertex*m_dwVertexSize );
	READ_MEM_OFF( _ibuf,t_fstart,sizeof( WORD )*this->m_dwNumIndices );

	END_USEGBUF( t_iGBufIdx );

	return true;

	unguard;
}



/** \brief
*  处理刀光数据的读入。
*
*  如果和当前的mesh名相同的*.gls文件存在，则数文件为刀光数据文件。
*/
void osc_bodyPartMesh::read_weaponData( void )
{
	guard;

	s_string   t_szGName = m_strFname;

	osn_mathFunc::change_extName( t_szGName,GLOSSY_FHEADER );


	// 如果文件不存在.
	if( !file_exist( (char*)t_szGName.c_str() ) )
		return;
	
	m_ptrSwordGlossy = new osc_swordGlossy();

# if __GLOSSY_EDITOR__
	m_ptrSwordGlossy->m_iMatrixIdx = 
		((os_characterVer*)m_ptrVerData)->m_vecBoneIdx[0];
# else
	m_ptrSwordGlossy->m_iMatrixIdx = m_ptrVerData.m_vecBoneIdx[0];
# endif 


	// 读入刀光数据：
	int   t_iGBufIdx = -1;
	BYTE*   t_fstart = START_USEGBUF( t_iGBufIdx );
	int     t_iSize ;
	DWORD   t_dwVersion;
	char    t_szHeader[4];

	t_iSize = read_fileToBuf( (char*)t_szGName.c_str(),t_fstart,TMP_BUFSIZE );
	osassert( t_iSize >0 );

	// 读入文件头和版本号.
	READ_MEM_OFF( t_szHeader,t_fstart,sizeof( char )*4 );
	osassert( strcmp( t_szHeader,GLOSSY_FHEADER ) == 0 );
	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	osassert( t_dwVersion >= GLOSSY_BPMFVERSION103 );

	READ_MEM_OFF( &m_bDisplay,t_fstart,sizeof( BOOL ) );
	
	READ_MEM_OFF( &m_ptrSwordGlossy->m_vec3Start,t_fstart,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_ptrSwordGlossy->m_vec3End,t_fstart,sizeof( osVec3D ) );

	READ_MEM_OFF( &m_ptrSwordGlossy->m_iActNum,t_fstart,sizeof( int ) );
	osassertex( m_ptrSwordGlossy->m_iActNum<MAX_ACTGLOSSY,"刀光对应的动作过多..\n" );
	READ_MEM_OFF( m_ptrSwordGlossy->m_vecActIdx,
		t_fstart,sizeof( int )*m_ptrSwordGlossy->m_iActNum );

	// 
	// 每一个动作对应的刀光颜色,与纹理相乘得到最终的颜色.
	READ_MEM_OFF( m_ptrSwordGlossy->m_vecGlossyColor,
		t_fstart,sizeof( DWORD )*m_ptrSwordGlossy->m_iActNum );


	//
	// 读入此刀光附加的特效数据。
	// River @ 2009-12-98:之后版本丢弃此数据,effectType.之前分的清，之后
	//                    的版本，都需要内部进行处理
	if( t_dwVersion < GLOSSY_BPMFVERSION106 )
	{
		int t_m = 0;
		READ_MEM_OFF( &t_m,t_fstart,sizeof( int ) );
	}

	READ_MEM_OFF( &m_ptrSwordGlossy->m_fEffectPos,t_fstart,sizeof( float ) );


	//! River @ 2010-3-5:加入多个武器特效位置的支持
	m_ptrSwordGlossy->m_fAddEffectPos[0] = m_ptrSwordGlossy->m_fEffectPos;
	m_ptrSwordGlossy->m_fAddEffectPos[1] = m_ptrSwordGlossy->m_fEffectPos;
	if( t_dwVersion >= GLOSSY_BPMFVERSION107 )
		READ_MEM_OFF( m_ptrSwordGlossy->m_fAddEffectPos,t_fstart,sizeof( float )*2 );


	//@{
	//! River @ 2009-12-7:加入对多个武器特效的支持
	if( t_dwVersion >= GLOSSY_BPMFVERSION106 )
	{
		READ_MEM_OFF( m_ptrSwordGlossy->m_sEffectName,t_fstart,sizeof( I_weaponEffect )*4 );
	}
	else
	{
		char   t_szChar[64];
		READ_MEM_OFF( t_szChar,t_fstart,sizeof( char )*64 );
		osassert( strlen( t_szChar )<48 );
		strcpy( m_ptrSwordGlossy->m_sEffectName[0].m_szEffObjName[0],t_szChar );
	}
	//@} 


	READ_MEM_OFF( m_ptrSwordGlossy->m_szGlossyTex,t_fstart,sizeof( char )*32 );
	READ_MEM_OFF( &m_ptrSwordGlossy->m_fOsaRotYAngle,t_fstart,sizeof( float ) );


	// 104版本加入对刀光长度的处理
	if( t_dwVersion >= GLOSSY_BPMFVERSION104 )
		READ_MEM_OFF( &m_ptrSwordGlossy->m_iGlossyMoveLength,t_fstart,sizeof( int ) );

	// 105的版本加入了对武器特效缩放的支持
	if( (t_dwVersion >= GLOSSY_BPMFVERSION105) )
	{
		if( t_dwVersion < GLOSSY_BPMFVERSION106 )
		{
			READ_MEM_OFF( &m_ptrSwordGlossy->m_fScaleVal[0],t_fstart,sizeof( float ) );
		}
		else
		{
			READ_MEM_OFF( &m_ptrSwordGlossy->m_fScaleVal[0],t_fstart,sizeof( float )*3 );
		}

	}

	END_USEGBUF( t_iGBufIdx );

	// River add @ 2009-9-4:使用最长的刀光.
	m_ptrSwordGlossy->m_iGlossyMoveLength = MAX_SWORDPOS;

	// 计算刀光的中心点，在刀光编辑器模式下，这个点用于调整刀光相关的数据。
	m_ptrSwordGlossy->m_vec3Center = 
		(m_ptrSwordGlossy->m_vec3Start+m_ptrSwordGlossy->m_vec3End)/2.0f;

	return;

	unguard;
}

//@{
// Windy mod @ 2005-9-1 17:48:12
//Desc: ride  保存第一个顶点的数据到文件中，
void osc_bodyPartMesh::SaveCenterPos(osVec3D _pos)
{

	FILE * fp = NULL;
	os_characterVer vertex;
	int offset = 0;
	if( !(fp=fopen( m_strFname, "r+" ) ) )
		return;
	//! id & version
	offset = 4 + 4;
	offset += sizeof(DWORD)*8;
	offset += sizeof( osMatrix )*m_dwNumPalette;
	offset += sizeof( DWORD )*m_dwNumPalette;

	fseek(fp,offset,SEEK_SET);
	fread(&vertex,1,sizeof(os_characterVer),fp);
	vertex.m_vec3Pos = _pos;
	fseek(fp,offset,SEEK_SET);
	fwrite((char *)&vertex,1,sizeof(os_characterVer),fp);
	fclose(fp);
}

//@}
/** \brief
*  调入skinMesh相关的数据。
*
*  从skin文件中调入骨骼动画作用的顶点数据。
*  在这个函数中创建设备相关的数据??????????
*  
*/
bool osc_bodyPartMesh::LoadMesh( char * _file,bool _IsRide,int _ref/* = 0*/ )
{
	guard;

	int   t_iSize;
	char  t_str[256];
	BYTE*  t_fstart;

	osassert( _file );

	//
	// Catch file name.
	strcpy( m_strFname,_file );
	m_bDisplay = TRUE;

	//
	// 读入数据到内存。
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( _file,t_fstart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
		return false;
	

	//
	// 读入文件头. 
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,SKN_FILEMAGIC ) != 0 )
	{
		osDebugOut( "<%s> file Format error!!!",_file );
		osassert( false );
		return false;
	}

	// ATTENTION TO FIX:版本号必须是最终文件的版本号：
	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	if( t_iSize < SKN_FILEVERSION )
	{
		osDebugOut( "<%s>文件版本号不正确...,目前不再支持100版本号的*.skn文件...\n",_file );
		osassert( false );
		return false;
	}
	osassertex( t_iSize == SKN_FILEVERSION,
		"新的程序版本只支持101版本的skn数据，使用程序工具转化*.skn文件到101的文件版本" );

	//
	// 读入文件信息。
	READ_MEM_OFF( &m_dwID,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &m_dwParentID,t_fstart,sizeof( DWORD ) );

	READ_MEM_OFF( &this->m_dwVertexSize,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &this->m_dwFVF,t_fstart,sizeof( DWORD ) );

	READ_MEM_OFF( &this->m_dwNumPalette,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &m_dwNumInfluPV,t_fstart,sizeof( DWORD ) );


	READ_MEM_OFF( &m_dwNumIndices,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &m_dwNumVertex,t_fstart,sizeof( DWORD ) );


	//
	// 调入每一个骨骼的matrix.
	osassertex( (m_dwNumPalette<=MAX_BONEPERBPM),"骨骼数目过多..\n" );
	READ_MEM_OFF( &m_arrBoneMatrix[0],t_fstart,sizeof( osMatrix )*m_dwNumPalette );
	READ_MEM_OFF( &m_arrBoneId[0],t_fstart,sizeof( DWORD )*m_dwNumPalette );

	//
	// 创建设备相关的数据,这个函数中，不需要使用全局的缓冲区.
	//
	os_smDevdepDataInit   t_devdep;
	t_devdep.m_iIdxNum = m_dwNumIndices;
	t_devdep.m_iVerNum = m_dwNumVertex;
	t_devdep.m_iVerSize = m_dwVertexSize;
	t_devdep.m_ptrDStart = t_fstart;
	t_devdep.m_ptrIStart = (WORD*)(t_fstart + m_dwNumVertex*m_dwVertexSize);
	t_devdep.m_strFname = _file;

	m_iDevdepResId = g_smDevdepMgr->create_devdepMesh( t_devdep );
	osassert( m_iDevdepResId>=0 );

	// ATTENTION : 目前只有人物的第7部分可以出现刀光或是特效。
	char   t_ch = m_strFname[strlen(m_strFname)-WEAPON_PARTOFFSET];
	//@{
	// Windy mod @ 2005-8-30 19:20:25
	//Desc: ride 检查是否是是坐骑的定位盒部分
	if( _IsRide  )
	{
		m_IsRideBox = true;
		m_RideVertData =*((os_characterVer*)t_devdep.m_ptrDStart);
		m_ptrRide = new osc_ChrRide();
		m_ptrRide->m_iMatrixIdx = m_RideVertData.m_vecBoneIdx[0];
		m_ptrRide->m_vec3Center = m_RideVertData.m_vec3Pos;
	}
	//@}


# if __GLOSSY_EDITOR__
	// ATTENTION : 目前只有人物的第7部分可以出现刀光或是特效。
	if( (t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR) )
	{
		m_ptrVerData = new BYTE[m_dwVertexSize*m_dwNumVertex];
		memcpy( m_ptrVerData,t_devdep.m_ptrDStart,m_dwVertexSize*m_dwNumVertex );
	}
# else

	// 在结束使用全局的缓冲区之前，保存一个顶点数据。
	if( (t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR) )
		m_ptrVerData = *((os_characterVer*)t_devdep.m_ptrDStart);

# endif 

	END_USEGBUF( t_iGBufIdx );


	//
	// 调入刀光相关的数据.ATTENTION TO FIX: 拆分函数处理。
	osassertex( t_iSize != GLOSSY_BPMFVERSION,"102版本的武器刀光需要重新编辑" );


	//@{
	// River @ 2005-5-30:
	// 如果刀光的文件存在，则调入刀光相关的数据。
	if( (t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR) )
		read_weaponData();



	// 
	// 当前bpm处于使用状态.
	m_bInuse = true;

	// River mod @ 2007-7-16:骑乘的怪物如果预先调入为非骑乘的怪物的话，必须重新调用
	for( int t_i=0;t_i<_ref;t_i ++ )
		add_ref();

	m_dwHash = string_hash( m_strFname );

	return true;

	unguard;
}


//! Set VIP Load，多线程调入，当前skinObj放在队列的最前面，最后设置的char优先级别最高.
void osc_skinMesh::set_vipMTLoad( void )
{
	guard;

	static int t_sVipLevel = 0;

	t_sVipLevel++;
	m_iVipMTLoadLevel = t_sVipLevel;

	unguard;
}



/** \brief
*  处理当前skinMesh的Frame Hierarchy数据
*  
*  IMPORTANT:使用这个函数把整个骨骼系统连结成一个可以使用的层级系统。
*  
*  
*/
void osc_skinMesh::process_frameHierarchy( void )
{
	guard;

	os_boneIdx* t_ptrBIdx;

	osassertex( m_iBFNum == 0,"SkinMeshObj Release Not Clear!!" );

	// 先设置第0个骨骼ID.
	m_pRoot = get_freeBF();
	m_pRoot->set_id( 0 );
	m_pframeToAnimate[0]	= m_pRoot;
	m_pRoot->m_pParentSkin	= this;

	// 
	// ATTENTION: 为什么必须从1开始？
	// 
	// A: 第0个已经在上面设置过了第0个了,如果多次设置,则会出现问题.
	// 
	osassert( m_ptrBoneSys->m_iBoneNum <= MAX_FRAMENUM );
	for( int t_i=1;t_i<m_ptrBoneSys->m_iBoneNum;t_i++ )
	{
		t_ptrBIdx = &m_ptrBoneSys->m_vecBoneIdx[t_i];

		DWORD t_parentID = t_ptrBIdx->m_iParentIdx;
		if( t_parentID == -1 )
			t_parentID = 0;

		DWORD t_myID			    	= t_ptrBIdx->m_iBoneIdx;
		osc_bodyFrame  *t_ptrFrame 	    = get_freeBF();
		osassert( t_ptrFrame );
		t_ptrFrame->set_id( t_myID );


		t_ptrFrame->m_pParentSkin	= this;
		m_pframeToAnimate[t_myID]	= t_ptrFrame;

		osc_bodyFrame*  t_ptrParent;
		if( t_myID != 0 )
			t_ptrParent = m_pRoot->FindFrame(t_parentID);
		else
			t_ptrParent = m_pRoot;

		if( t_ptrParent!=NULL )
			t_ptrParent->AddFrame( t_ptrFrame );

	}


	unguard;
}


//! 创建当前skinMeshSystem的shader.
bool osc_skinMesh::create_smShader( os_skinMeshInit* _smi )
{
	guard;

	s_string        t_str,t_strIni;
	char            t_sz[128];
	osassert( _smi );
	os_bodyLook*    t_ptrLook = &_smi->m_smObjLook;

	//
	// 测试我们的Character Shader创建.
	m_iMlFlagIdx = _smi->m_iMlFlagIdx;

	for( int t_i=0;t_i<m_iNumParts;t_i++ )
	{
		if( t_ptrLook->m_skin[t_i] == 0 )
		{
			continue;
		}

		t_str = _smi->m_strSMDir;
		t_str += '\\';

		//
		// 创建Character shader.
		sprintf( t_sz,"%d_%02d_%02d",t_i+1,t_ptrLook->m_mesh[t_i],t_ptrLook->m_skin[t_i] );

		t_str += t_sz;
		
		// TEST CODE:
		// River added @ 2006-8-31:保证文件名的长度?
		osassert( strlen( t_sz ) > 5 );
		osassert( t_str.length() > 5 );


		t_strIni = t_str + ".";
		t_strIni += CHASEC_EXTENT;

		// 如果是多层的旗帜效果，则使用特殊的创建功能
		if( t_i == m_iMlFlagIdx )
		{
			create_mlFlagShader( t_i,(char*)t_str.c_str() );
			continue;
		}

		// 正常的人物创建功能
		// River mod @ 2007-5-18:把findfirstfile函数修改为引擎内部的file_exist
		if( file_exist( (char*)t_strIni.c_str() ) )
		{
			create_doubleTexShader( t_i,(char*)t_str.c_str() );
		}
		else
		{
			// TEST CODE:
			// River added @ 2006-8-31:保证文件名的长度?
			osassert( t_str.length() > 5 );
			if( !create_singleTexShader( t_i,(char*)t_str.c_str() ) )
				return false;
		}

	}

	// River @ 2007-3-22:由上层统一缩放,此处缩放有问题，包围盒数据还不正确。
	//scale_skinMesh(osVec3D(_smi->m_Scale,_smi->m_Scale,_smi->m_Scale));

	return true;

	unguard;
}





//! 得到调入时用到的bis文件名。
void osc_skinMesh::get_bisFileName( s_string& _res,s_string& _sdir )
{
	guard;

	int       t_idx,t_iLength;

	_res = m_strSMDir;
	t_idx = (int)_res.rfind( '\\' );
	t_iLength = (int)_res.length();
	_sdir = '\\';
	_sdir += _res.substr( t_idx+1,t_iLength-t_idx-1 );
	_res  += _sdir;
	_res  += BIS_EXTENT;

	return;

	unguard;
}


/** \brief
*  从目录文件调入一个骨骼系统。
*
*  River @ 2005-7-19: 加入容错的相应该返回值。
*/
bool osc_skinMesh::load_skinMeshSystem( os_skinMeshInit* _smi )
{
	guard;

	s_string       t_str,t_str1;
	osc_bodyFrame*  t_frame; 
	char           t_sz[128];


	osassert( _smi );
	osassert( strlen( _smi->m_strSMDir ) < MAX_BPMFNLEN );

	// River added @ 2009-12-30:加入二维界面专用的人物，用于渲染人物
	// 附加的特效
	m_bUseInInterface = _smi->m_bUseInInterface;
	m_bPlayWeaponEffect = _smi->m_bPlayWeaponEffect;

	// 是否写入Zbuffer, 是否支持阴影数据。　
	m_bWriteZbuf = _smi->m_bWriteZbuf;
	if( _smi->m_bShadowSupport && g_bHeroShadow )
		m_bShadowSupport = TRUE;
	else
		m_bShadowSupport = FALSE;
	strcpy( m_strSMDir,_smi->m_strSMDir );
	//@{
	// Windy mod @ 2005-9-21 16:50:42
	//Desc: RIDE 加载骑马的偏移
	m_vec3RideOffset = _smi->m_vec3RideOffset;

	//@}
	//ZeroMemory(mszspeFileName,MAX_PATH);
	mszSpeFileName.clear();
	if (strlen(_smi->m_szspeFileName)>0)
	{
		mszSpeFileName = _smi->m_szspeFileName;
	//	strcpy(mszspeFileName,_smi->m_szspeFileName);
		mEffectOffset = _smi->m_spePostion;
		mEffectId = get_sceneMgr()->create_effect(
			_smi->m_szspeFileName,_smi->m_vec3Pos+_smi->m_spePostion,_smi->m_fAngle);
	}
	
	
	// River added @ 2008-5-20
# if GALAXY_VERSION
	m_bProjectShadow = _smi->m_bSupportProjectShadow;
# endif
	

	// 处理得到*.bis的文件名。
	get_bisFileName( t_str,t_str1 );

	//
	// 从bone sys manager中得到当前boneSys的指针。
	m_ptrBoneSys = g_boneSysMgr->create_boneSysFromMgr( t_str.c_str() );
	osassertex( m_ptrBoneSys,"创建人物的骨骼系统失败" );
	if( !m_ptrBoneSys )
		return false;

	//
	// 处理骨骼的层级系统。
	process_frameHierarchy();

	//
	// 测试调入*.act文件。　
	m_ptrBoneAni = g_boneAniMgr->create_boneAni( 
		m_strSMDir,_smi->m_iTotalActTypeNum );
	if( !m_ptrBoneAni )
	{
		osDebugOut( "调入人物<%s>的动作信息失败...\n",_smi->m_strSMDir );
		osassertex( m_ptrBoneAni,"调入act文件失败" );
		return false;
	}

	//
	// 调入顶点数据。根据bodyLook的不同，调入不同的文件数据。
	// 7个身体部位，最后一个身体部位暂不创建
	if( (_smi->m_smObjLook.m_iPartNum == (MAX_SKINPART-1))&&
		(_smi->m_smObjLook.m_mesh[WEAPON_BPMID] == 0 ) )
		_smi->m_smObjLook.m_iPartNum --;

	// 有8个身体部位，最后两个暂时不创建
	if( (_smi->m_smObjLook.m_iPartNum == MAX_SKINPART)&&
		(_smi->m_smObjLook.m_mesh[WEAPON_BPMID] == 0 )&&
		(_smi->m_smObjLook.m_mesh[WEAPON_BPMID+1] == 0) )
		_smi->m_smObjLook.m_iPartNum -= 2;

	m_iNumParts = _smi->m_smObjLook.m_iPartNum;

	os_bodyLook*  t_ptrLook = &_smi->m_smObjLook;
	t_str = m_strSMDir;
	
	for( int t_i=0;t_i<m_iNumParts;t_i++ )
	{
		// 处理空的mesh.
		if( t_ptrLook->m_mesh[t_i] == 0 )
			continue;

		//
		// 得到要调入的Mesh文件名。
		sprintf( t_sz,"_%02d_%02d%s",
			t_i+1,t_ptrLook->m_mesh[t_i],SKN_EXTENT );
		t_str = _smi->m_strSMDir;
		t_str += t_str1;
		t_str += t_sz;
		//@{
		// Windy mod @ 2005-8-30 19:23:50
		//Desc: ride 2 如果传入的结构体标志该SKINMESH为 m_bIsRide则该MESH为坐骑，
		//我们把它的最后一部分特殊处理。即loadBpmIns最后一个参数为真，
		//m_bIsRide = false;
		bool t_bIsRidePart = false;
		
		if (_smi->m_bIsRide&&t_i== (m_iNumParts-1) )
		{
			
			t_bIsRidePart = true;
		}
		//@}



		
		
		if( !m_vecBpmIns[t_i].loadBpmIns( (char*)t_str.c_str(),m_bPlaySGlossy,
			                       m_bPlayWeaponEffect,m_bShadowSupport,
								   t_bIsRidePart,_smi->m_bUseInInterface,this ) )
			return false;

		osc_bodyPartMesh* t_p = m_vecBpmIns[t_i].get_bpmPtr();

		t_frame = m_pRoot->FindFrame( t_p->m_dwID );
		osassertex( t_frame,"找不到空的frame可用" );

		t_frame->AddMesh( &m_vecBpmIns[t_i] );

		// 
		// 编辑刀光相关的数据。创建时和更改人物的mesh时需要更改这个数据。
# if __GLOSSY_EDITOR__
		weapon_process( t_i,t_p );
# endif 

		if( m_vecBpmIns[t_i].is_glossySM()&&t_i == WEAPON_PARTID7 )
			m_ptrSGBpmIns = &m_vecBpmIns[t_i];
		//else
		//	m_ptrSGBpmIns = NULL;
		//!windy add 加载第八部分
		if( m_vecBpmIns[t_i].is_glossySM()&&t_i == WEAPON_PARTID8 )
			m_ptrSGBpmIns8 = &m_vecBpmIns[t_i];
		//else
		//	m_ptrSGBpmIns8 = NULL;

	}

	// 创建skinMesh的shader相关数据。
	if( !create_smShader( _smi ) )
		return false;

	//
	// 把所有的骨骼数据和mesh数据连结起来。
	m_pRoot->LinkBones( m_pRoot );
	m_bInUse = true;


	//
	// Catch body look
	osVec3D   t_vec3Min,t_vec3Max;
	memcpy( &m_Look,&_smi->m_smObjLook,sizeof( os_bodyLook ) );
	m_vec3SMPos = _smi->m_vec3Pos;
	m_fRotAgl = _smi->m_fAngle;	
	m_sBBox.set_bbYRot( m_fRotAgl );
	m_sBBox.set_bbPos( m_vec3SMPos );
	m_sAdjBBox.set_smOffsetAdjBBox( _smi->m_vec3Min,_smi->m_vec3Max,t_vec3Min,t_vec3Max );
	m_sBBox.set_bbMaxVec( t_vec3Max );
	m_sBBox.set_bbMinVec( t_vec3Min );



	// 
	// cha文件数据，全部由手工调整
	osVec3D     t_vec3;

	// 使人物正对着x方向。
	m_fAdjRgl = _smi->m_fAdjRotAgl-OS_PI/2; 
	m_vec3AdjPos = _smi->m_vec3AdjPos;
	m_fBSRadius = osVec3Length( &(_smi->m_vec3Max - _smi->m_vec3Min) )/2.0f;
	
	//  默认为 1.0f
	m_sCurSkinMeshAmbi.r = g_fAmbiR*CHAR_AMBISCALE;
	m_sCurSkinMeshAmbi.g = g_fAmbiG*CHAR_AMBISCALE;
	m_sCurSkinMeshAmbi.b = g_fAmbiB*CHAR_AMBISCALE;
	m_sCurSkinMeshAmbi.a = 1.0f;

	// 包围盒调整.
	m_sAdjBBox.get_smOffsetAdjBBox( t_vec3,m_fAdjRgl + m_fRotAgl );
	m_sBBox.set_bbPos( m_vec3SMPos + t_vec3 );



	//
	// 处理动作速度相关的信息。
	set_actSpeedInfo( m_fNewActSpeed );
	get_defaultAct( _smi );


	//@{ River @ 2009-6-7:把上层的代码直接移进来.
# if __ALLCHR_SIZEADJ__
	// 场景中的sm不随着人物的放大而放大.
	if( !_sceneSm )
		createScale_withAllChrScale();
# endif 


	m_bUseDirLAsAmbient = _smi->m_bUseDirLAsAmbient;

	//@{
	// TEST CODE:确认每一个Frame有正确的bm数目.
	assert_frameBMNum();
	//@} 
	if (_smi->m_bIsRide)
		HideLastPart(true);


	// River @ 2010-1-7:处理人物的基准缩放，人物武器的附加特效以这个基准缩放为基础
	m_fBaseSkinScale = _smi->m_Scale;

	//  River @ 2007-3-22:已经在内部缩放过
	scale_skinMesh( _smi->m_Scale,_smi->m_Scale,_smi->m_Scale );
	//@} 

	//! River@2009-6-23:加入初始化时不显示人物
	if( _smi->m_bInitFade )
	{
		m_fAlphaVal = 0.0f;
		m_bFadeState = true;
	}

	//osDebugOut( "REAL CREATE CHARACTER<%d>...\n",int(this) );

	return true;

	unguard;
}


//@{
// TEST CODE: 
//! 用于确保Frame内身体部数目不为零的函数.在创建人物和切换装备前后调用.
void osc_skinMesh::assert_frameBMNum( void )
{
	guard;

	for( int t_i=0;t_i<m_iNumParts;t_i ++ )
	{
		osc_bodyFrame*     t_frame; 
		osc_bodyPartMesh*  t_p;

		t_p = m_vecBpmIns[t_i].get_bpmPtr();
		if( t_p )
		{
			t_frame = m_pRoot->FindFrame( t_p->m_dwID );

			if( t_frame->m_iBMNum <= 0 )
			{
				char   t_sz[256];
				sprintf( t_sz,"出错的人物目录:<%s>..bm数目为:<%d>..BPM NUM:<%d>..出错的部位:<%d>..\n",
					m_strSMDir,t_frame->m_iBMNum,m_iNumParts,t_i );
				osassertex( false,t_sz );		
			}
		}
	}

	unguard;
}
//@}



/** \brief
*  替换某一身体部位的mesh.
*
*  算法描述：
*  1: 先从frame中把原来的mesh去除。
*  2: 释放这个bpMesh.
*  3: 创建新的bpMesh
*  4: 并更新相关数据。
*  
*  \param _bidx    身体部位的id.
*  \param _midx    要替换的Mesh的id.
*/
bool osc_skinMesh::change_bodyMesh( int _bidx,int _midx )
{
	guard;

	char           t_sz[128];
	s_string       t_str,t_str1;
	bool           t_bMeshExist = false;
	int            t_idx,t_iOldFrameId,t_iLength,t_iShaIdBack = -1;
	osc_bodyFrame*     t_frame; 
	osc_bodyPartMesh*  t_p;
	osc_bpmInstance*   t_oldBpmIns;

	osassertex( m_bInUse&&(m_iBFNum>0),"SkinMesh已经被释放" );

	//@{
	// TEST CODE: 下层assert,使只能把人物武器的部位变为空.可以去除此段代码.
	if( _midx == 0 )
		osassertex( (_bidx==6)||(_bidx==7),"只能把武器的部位换为空....\n" );
	//@}


	t_oldBpmIns = &m_vecBpmIns[_bidx];
	t_p = m_vecBpmIns[_bidx].get_bpmPtr();
	if( t_p )
	{
		// 如果要换为空的mesh,不需要处理
		t_iShaIdBack = m_vecBpmIns[_bidx].m_iShaderID;
		
		t_frame = m_pRoot->FindFrame( t_p->m_dwID );
		if( t_frame == NULL )
			osassert( false );
		t_iOldFrameId = t_p->m_dwID;

		// 释放当前的bpmInstance.
		m_vecBpmIns[_bidx].release_bpmIns();

		t_bMeshExist = true;
	}

	//! river added @ 2010-2-2:确保底层删除不出错,
	//  这么写会出错。
	//if( _bidx >= m_iNumParts )
	//	t_bMeshExist = false;


	// 
	// 处理空的mesh.
	if( _midx == 0 )
	{
		// 释放时需要注意-1.
		if( t_bMeshExist )
		{
			t_frame->change_Mesh( &m_vecBpmIns[_bidx],NULL );
			osassert( t_iShaIdBack>=0 );
		}

		// 
		// 如果对应武器的BPM.
		if( _bidx == WEAPON_BPMID )
		{
			m_ptrSGBpmIns = NULL;
			m_iNumParts -= 1;
			m_Look.m_iPartNum = m_iNumParts;
		}
		else if (_bidx == WEAPON_PARTID8)
		{
			m_ptrSGBpmIns8 = NULL;
			m_iNumParts -= 1;
			m_Look.m_iPartNum = m_iNumParts;
		}
		else
			osassertex( false,"非武器部位不能替换为空" );

		return true;
	}

	//
	// 处理得到要调入的Mesh文件名,没有目录，没有扩展名和数字。
	// ATTENTION TO OPP: 可以更高效。
	t_str = m_strSMDir;
	t_idx = (int)t_str.rfind( '\\' );
	t_iLength = (int)t_str.length();
	t_str1 = '\\';
	t_str1 += t_str.substr( t_idx+1,t_iLength-t_idx-1 );


	//
	// 处理调入新的Mesh.
	sprintf( t_sz,"_%02d_%02d%s",
		_bidx+1,_midx,SKN_EXTENT );
	t_str = m_strSMDir;
	t_str += t_str1;
	t_str += t_sz;


	//! River @ 2010-12-21:加入是否是坐骑的参数，之前一直是错误的调用
	if( !m_vecBpmIns[_bidx].loadBpmIns( (char*)t_str.c_str(),
		m_bPlaySGlossy,m_bPlayWeaponEffect,m_bShadowSupport,
		this->m_bIsRide,m_bUseInInterface,this ) )
		return false;

	osc_bodyPartMesh* t_np = m_vecBpmIns[_bidx].get_bpmPtr();


	//
	// 如果原来当前的身体部位没有对应mesh的话，需要查找frame,并把新创建的mesh
	// 跟frame关联起来。
	if( !t_bMeshExist )
	{
		m_vecBpmIns[_bidx].m_iShaderID = -1;
		t_frame = m_pRoot->FindFrame( t_np->m_dwID );
		osassert( t_frame );

		t_frame->AddMesh( &m_vecBpmIns[_bidx] );
	}
	else
	{
		// River added @ 2005-8-17: 
		// 如果原BpmIns的FrameId与新的BpsIns的FrameId不同。
		// ATTENTION TO OPP: 这个函数,应该有更优雅的程序代码
		if( t_iOldFrameId != t_np->m_dwID )
		{
			// 先把原Frame内清零。
			t_frame->change_Mesh( &m_vecBpmIns[_bidx],NULL );

			// 往新的Frame内加入Mesh.
			m_vecBpmIns[_bidx].m_iShaderID = -1;
			t_frame = m_pRoot->FindFrame( t_np->m_dwID );
			osassert( t_frame );

			t_frame->AddMesh( &m_vecBpmIns[_bidx] );
		}
	}


	// 
	// 如果替换的是武器的bodyPartMesh,检测新的武器是否有刀光。
	if( _bidx == WEAPON_BPMID )
	{	
		// 
		// 重设武器部分的bodyPartMesh.
# if __GLOSSY_EDITOR__
		// 如果当前没有刀光或是武器附加特效，默认处理。
		if( !m_vecBpmIns[_bidx].is_glossySM() )
			weapon_process( _bidx,t_np );
		else
		{
			m_ptrSGIns = m_vecBpmIns[_bidx].m_ptrSGIns;
			m_ptrWeaponBpm = t_np;
		}
# endif 
		if( m_vecBpmIns[_bidx].is_glossySM() )
		{
			m_ptrSGBpmIns = &m_vecBpmIns[_bidx];
		}
		else
			m_ptrSGBpmIns = NULL;
	}
	// 如果替换的是武器的bodyPartMesh,检测新的武器是否有刀光。
	if( _bidx == WEAPON_PARTID8 )
	{	
		// 
		// 重设武器部分的bodyPartMesh.
# if __GLOSSY_EDITOR__
		// 如果当前没有刀光或是武器附加特效，默认处理。
		if( !m_vecBpmIns[_bidx].is_glossySM() )
			weapon_process( _bidx,t_np );
		else
		{
			m_ptrSGIns8 = m_vecBpmIns[_bidx].m_ptrSGIns;
			m_ptrWeaponBpm8 = t_np;
		}
# endif 
		if( m_vecBpmIns[_bidx].is_glossySM() )
		{
			m_ptrSGBpmIns8 = &m_vecBpmIns[_bidx];
		}
		else
			m_ptrSGBpmIns8 = NULL;
	}



	m_Look.m_mesh[_bidx] = _midx;

//	osDebugOut( "The idx is:<%d>..\n",_bidx );

	//  ATTENTION TO OPP: 只需要连结更换部位的Palette
	m_pRoot->LinkBones( m_pRoot );

	// 从空到有，增加一个身体部位。
	// 从空到有，增加一个身体部位。
	if( !t_bMeshExist )
	{
		//！ River @ 2010-1-21:使用更加精确的算法。
		if( m_iNumParts < (_bidx + 1) )
		{
			m_iNumParts = _bidx + 1;
			m_Look.m_iPartNum = m_iNumParts;
		}
	}

	// River added @ 2009-9-8:确认播放刀光特效
	if( (_bidx == WEAPON_BPMID)||( _bidx == WEAPON_PARTID8 ) )
		playSwordGlossy( m_bPlaySGlossy, m_bPlayWeaponEffect );

	osassertex( m_iNumParts<=8,"身体部位数不能超过8...\n" );


	return true;

	unguard;
}

/** \brief
*  替换某一身体部位的skin.
*
*  \param _bidx    身体部位的id.
*  \param _midx    要替换的Mesh的id.
*  \param _sidx    要替换的skin的id.
*/
bool osc_skinMesh::change_bodySkin( int _bidx,int _midx,int _sidx )
{
	guard;// osc_skinMesh::change_bodySkin() );

	s_string    t_str,t_strIni;
	char        t_sz[128];

	//
	// 如果身体部位id为空，则什么都不需要替换。
	if( _midx <= 0 )
		return true;

	//
	// 先释放原来的SkinShader
	// River @ 2005-6-24: 如果此处是先释放了mesh,则下面的函数调用，什么都没有做
	//                    在bodyPartMesh instance的释放中，包含了对这些数据的释放。
	release_skinShader( _bidx );


	//　
	// 处理调入新的Mesh.
	sprintf( t_sz,"%d_%02d_%02d",_bidx+1,_midx,_sidx );
	t_str = m_strSMDir;
	t_str += '\\';
	t_str += t_sz;


	// 
	// 判断要创建的skin的类型。
	t_strIni = t_str + ".";
	t_strIni += CHASEC_EXTENT;

	// 正常的人物创建功能
	// River mod @ 2007-6-5:把findfirstfile函数修改为引擎内部的file_exist
	if( file_exist( (char*)t_strIni.c_str() ) )
		create_doubleTexShader( _bidx,(char*)t_str.c_str() );
	else/**/
	{
	
		if( !create_singleTexShader( _bidx,(char*)t_str.c_str() ) )
			return false;
	}


	//
	// Update Skin Look.
	m_Look.m_skin[_bidx] = _sidx;

	return true;

	unguard;
}

//! 切换旗帜的贴图
bool osc_skinMesh::change_mlFlagTexture( const char* _tex1,
											 const char* _tex2,const char* _tex3 )
{
	guard;

	osc_effect*   t_ptrEffect;

	// ATTENTION TO FIX: 
	// 因为此Effect为全局唯一，所以目前场景内只支持出一面旗帜，如果是多面旗帜，
	// 则一个旗帜换装后，其它的旗帜也会跟着换装。
	t_ptrEffect = g_shaderMgr->get_effect( m_vecBpmIns[m_iMlFlagIdx].m_iShaderID );	
	osassertex( t_ptrEffect,va( "The shader id is<%d>..bpm is:<%d>",
		m_vecBpmIns[m_iMlFlagIdx].m_iShaderID,m_vecBpmIns[m_iMlFlagIdx].m_iBpmId ) );

	int    t_iTmpTexId;

	if( _tex1 )
	{
		if( _tex1[0] )
		{
			t_iTmpTexId = g_shaderMgr->create_texture( (char*)_tex1 );
			//sha->m_vTexture[_idx].texList[frame] = t_id;
			//t_ptrEffect->set_texture( 0,g_shaderMgr->get_texturePtr( t_iTmpTexId ) );
			g_shaderMgr->set_shaderTexture( m_vecBpmIns[m_iMlFlagIdx].m_iShaderID,0,t_iTmpTexId );
		}
	}
	
	if( _tex2 )
	{
		if( _tex2[0] )
		{
			t_iTmpTexId = g_shaderMgr->create_texture( (char*)_tex2 );
			t_ptrEffect->set_texture( 1,g_shaderMgr->get_texturePtr( t_iTmpTexId ) );
		}
	}

	if( _tex3 )
	{
		if( _tex3[0] )
		{
			t_iTmpTexId = g_shaderMgr->create_texture( (char*)_tex3 );
			t_ptrEffect->set_texture( 2,g_shaderMgr->get_texturePtr( t_iTmpTexId ) );
		}
	}

	return true;

	unguard;
}


# if __GLOSSY_EDITOR__

/** brief
*  只显示某个身体部位的部分三角形,公用于调试目的
*
*  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
*  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
*/
int osc_skinMesh::displaytri_bysel( int _paridx,int _triidx[24],int _trinum )
{
	guard;

	osassert( (_paridx >= 0)&&(_paridx<8) );
	osassert( _trinum < 24 );

	return m_vecBpmIns[_paridx].displaytri_bysel( _triidx,_trinum );

	unguard;
}

# endif 

