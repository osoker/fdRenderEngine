///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterLoad.h
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     ����os�����е����ﶯ������ļ��ĵ��봦��
 *  
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../../interface/miskFunc.h"
# include "../../mfPipe/include/osEffect.h"
# include "../../interface/stringTable.h"

//! ��ǰact�ļ��İ汾��.
# define  ACT_FILEVERSION   100

//! 2004-4-16: �����ļ��ṹ��ʹ��quat��offset��ʾ������
# define  ACT_FILEVERSION11  111

//! 2005-3-4:  �����ļ��汾����,�����ﶯ�������Flip������ݼ���.
# define  ACT_FILEVERSION12  112

/*
//! ��ǰskn mesh�ļ��İ汾��.
# define SKN_FILEVERSION  100
*/
//! ��100��101�İ汾���޸���skinMesh�ж���Normal���Ե�bug��ʹ��
//! �����е��������ʹ�÷���⡣
# define SKN_FILEVERSION  101

//! ���������Ŀ¼������Ϊ7
# define SKN_DIRNAMELEN   7


//! ��һ������ID�õ�����������.
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

//! ��һ������ID�õ��˹������ǵ�����.
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
 *  ���ļ��е���һ������ϵͳ��
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
	// �Ȱ��ļ����뵽�ڴ��С�
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)_fname,t_fstart,TMP_BUFSIZE );
	osassertex( t_iSize>0,(char*)_fname );

	
	// 
	// ���ڴ���в�������ȡ������Ҫ���ļ�����
	READ_MEM_OFF( &this->m_iBoneNum,t_fstart,sizeof( int ) );
	osassert( m_iBoneNum > 0 );

	m_vecBoneIdx.resize( m_iBoneNum );
	m_vecBoneNameStr.resize( m_iBoneNum );

	for( int t_i=0;t_i<m_iBoneNum;t_i++ )
	{
		READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
		READ_MEM_OFF( t_str,t_fstart,t_iSize+1 );
		//!�������ִ�Сд����
		m_vecBoneNameStr[t_i] = StringTable->insert(t_str,true);
	}

	// 
	// ����boneId�Ĳ�����
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
*  ���ļ��е���һ��������bone�Ķ����ļ���
*  
*  \param _fname  Ҫ����Ķ����ļ���.
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
	// �����ļ�ͷ.
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
		MessageBox( NULL,"�����˴���Ķ����ļ��汾",_fname,MB_OK );
		osassert( false );
		return false;
	}

	//
	// ��������Ͷ���֡����
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
 *  �����Ķ��������ļ���ʹ�õ����ݽṹ��
 */
struct t_action
{
	int index;
	int  m_iSpeed;

	//! ֹͣ֡���ݡ�
	int  m_iStopFrameIdx;
};


/** \brief
*  ���ļ��е��������������Ϣ�ļ�������ÿһ�����������֣�ÿһ�������Ĳ����ٶȵȡ�
*
*  \param _actIfname  ����Ķ�����Ϣ�ļ�����
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
	//Desc: ����༭��������û��ACS�ļ����������ʱ����.
	if (!file_exist( (char *)_actIfname )){
		return false;
	}
	//@}
	osassert( _actIfname );

	// tzz add:
	// ʹ��CReadFile ����ȡ����,
	// ��ֹ��osassert �׳��쳣֮��û�� END_USEGBUF����̫���osassert��
	// ���³�����������ֹ,��Ҫ����Դ����������ֹ
	//
	// �鿴 CReadFile ��ø������Ϣ
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
	// Ŀǰ֧�ֵ���߰汾�š�
	//READ_MEM_OFF( &t_iVersion,t_fstart,sizeof( DWORD ) );
	t_file.ReadBuffer(&t_iVersion,sizeof( DWORD ));

	// ����汾�Ŵ���1,�ļ��м�����ֹͣ֡���������ݡ�	

	// ����汾�Ŵ�����3,������
	if( t_iVersion < 3 )
		osDebugOut( "\n\nWARNNING: ���ﶯ���汾�ŵ���3,�Ǿɵİ汾!!!!!!!!!\n\n" );


	//READ_MEM_OFF( &t_iSize,t_fstart,sizeof( DWORD ) );
	t_file.ReadBuffer(&t_iSize,sizeof( DWORD ));

	osassert( t_iSize > 0 );
	m_vecActName.resize( t_iSize );

	// 
	// ����ÿһ���������ֶ�Ӧ�Ķ�����Ϣ��
	for( int t_i=0;t_i<t_iSize;t_i++ )
	{
		// ���뵱ǰ�������ֶ�Ӧ��ʵ�ʶ���������ʵ�ʶ�����Ŀ��
		//READ_MEM_OFF( &m_vecActName[t_i].m_iActNum,t_fstart,sizeof( int ) );
		//READ_MEM_OFF( m_vecActName[t_i].m_szActName,t_fstart,sizeof( char )*ACT_NAMELEN );
		t_file.ReadBuffer(&m_vecActName[t_i].m_iActNum,sizeof( int ));
		t_file.ReadBuffer(m_vecActName[t_i].m_szActName,sizeof( char )*ACT_NAMELEN);
	
		osassertex( (m_vecActName[t_i].m_iActNum==1),
			     "���µĹ淶��,ÿһ����������ֻ�ܶ�Ӧһ������.." );
		
		// River @ 2010-6-22:�������������Ķ�������,������ʹ��.
		if( t_iVersion == 4 )
			t_file.ReadBuffer( &m_vecActName[t_i].m_bHideWeapon,sizeof( BOOL ) );

		// TEST CODE:
		os_actNameStruct* t_testAct = &m_vecActName[t_i];

		
		// ���ݲ�ͬ�İ汾�Ŵ���ͬ�ĵ��롣
		if( t_iVersion > 1 )
		{
			// ����1�����ﶯ���汾��û��ֹͣ֡��������������
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
			sprintf( t_szInfo,"���� <%s> ��Ӧ������Ƕ������...\n",
				m_vecActName[t_i].m_szActName );
			MessageBox( NULL,t_szInfo,"��Ӧ��������",MB_OK );

			return false;
		}
# endif

		// TEST CODE: Ϊ���������Ϣ�����հ汾Ҫȥ��
		if( t_maxAct.index >= m_iBANum )
		{
			osDebugOut( "\n ע��!!!        ����<%s>��Ӧ�Ķ�������ֵ<%d>������*.cha�ļ��ڲ���������Ŀ<%d>...\n",
				m_vecActName[t_i].m_szActName,t_maxAct.index,m_iBANum );

			//END_USEGBUF( t_iGBufIdx );
			// t_file (CReadFile) ������������ END_USEGBUF
			osassert( false );

			return false;
		}

		osassertex( t_maxAct.index < m_iBANum,"������Ӧ������ֵ����...\n" );
		m_vecActName[t_i].m_vecActIdx = t_maxAct.index;
		osassertex( (t_maxAct.index>=0)&&(t_maxAct.index<m_iBANum),
			"�������ֵĶ������������������ݷ�Χ...\n" );
		os_boneAction*  t_ptr = &m_vecBATable[t_maxAct.index];


		t_ptr->m_iPlaySpeed = t_maxAct.m_iSpeed;
		osassert( t_ptr->m_iPlaySpeed>0 );

		//
		// ������ɵ�ǰ�Ķ�����Ҫ��ʱ�䡣
		t_ptr->m_fTimeToPlay = 
			float( t_ptr->m_iNumFrame ) / float( t_ptr->m_iPlaySpeed );

		// 
		// ����ֹͣ֡������
		// River mod @ 2009-8-17:ʹ��һ���򵥵�Լ�����������п���ĳ��
		// ��������û�д���stopFrameIdx������
		if( t_ptr->m_iStopFrameIdx < t_maxAct.m_iStopFrameIdx )
			t_ptr->m_iStopFrameIdx = t_maxAct.m_iStopFrameIdx;
	}

	//END_USEGBUF( t_iGBufIdx );
	// t_file (CReadFile) ������������ END_USEGBUF

	return true;

	unguard;
}

//! River @ 2010-7-1: ��ǰ�Ķ��������Ƿ���Ҫ��������.
bool os_boneAni::is_actNameHideWeapon( const char* _actName )
{
	guard;

	os_actNameStruct*  t_ptrActName;

	osassert( _actName&&(_actName[0]) );

	bool     t_bHide = false;
	// 
	// �ҵ�����ָ���Ķ������֣�Ȼ��ʹ��������֣��õ�������һ��������������
	for( int t_i=0;t_i<(int)m_vecActName.size();t_i ++ )
	{
		t_ptrActName = &m_vecActName[t_i];
		if( strcmp( _actName,t_ptrActName->m_szActName ) == 0 )
		{
			osassert( t_ptrActName->m_iActNum>0 );

			t_bHide = t_ptrActName->m_bHideWeapon;

			// �ҵ�.
			break;

		}
	}

	return t_bHide;

	unguard;
}


//! ��һ���������ֵõ�һ��������������
int os_boneAni::get_actIdxFromName( const char* _actName,int& _nameIdx )
{
	guard;

	os_actNameStruct*  t_ptrActName;
	int               t_iActIdx = -1;

	osassert( _actName&&(_actName[0]) );

	// 
	// �ҵ�����ָ���Ķ������֣�Ȼ��ʹ��������֣��õ�������һ��������������
	for( int t_i=0;t_i<(int)m_vecActName.size();t_i ++ )
	{
		t_ptrActName = &m_vecActName[t_i];
		if( strcmp( _actName,t_ptrActName->m_szActName ) == 0 )
		{
			osassert( t_ptrActName->m_iActNum>0 );
			_nameIdx = t_i;

			t_iActIdx = t_ptrActName->m_vecActIdx;

			// �ҵ�.
			break;

		}
	}

	return t_iActIdx;

	unguard;
}




//! ��һ��actNameIdx����������ڶ������������õ�һ��bone Act Table.
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
*  �Ӷ����ļ��еõ��µĶ������ݡ�
*  
*  \param _id ��Ϊ�����ļ�ʹ��������׼������ֻ��Ҫ
*             ����_id�Ϳ����˵õ��µĶ�����
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
		// �������ڵ�ActionName,�������еĶ����ļ�
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
	// �����ļ�*.act�ļ�����ʹ����*.acs�ļ���
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
		//Desc: ������༭����Ĭ��û��ACS�ļ������Լ��ز��ɹ�ҲҪ�����棬�������в���ȥ
		return true;
		//@}	
	}

	return true;

	unguard;
}



/** \brief
*  ����*.skn�ļ��Ķ�����������ݡ�
*
*  �˺������贫���������������ָ���Ѿ��������ݣ����ҿռ��С�㹻��
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
	// �������ݵ��ڴ档
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( _file,t_fstart,TMP_BUFSIZE );
	if( t_iSize < 0 )
	{
		osassert( false );
		return false;
	}
	

	//
	// �����ļ�ͷ. 
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,SKN_FILEMAGIC ) != 0 )
	{
		osDebugOut( "<%s> file Format error!!!",_file );
		osassert( false );
		return false;
	}

	//
	// �����ļ���Ϣ��
	t_fstart += (sizeof( DWORD )*9 );

	//
	// ����ÿһ��������matrix.
	t_fstart += ( sizeof( osMatrix )*m_dwNumPalette );
	t_fstart += ( sizeof( DWORD )*m_dwNumPalette );


	//
	// �����豸��ص���Դ.
	READ_MEM_OFF( _vbuf,t_fstart, m_dwNumVertex*m_dwVertexSize );
	READ_MEM_OFF( _ibuf,t_fstart,sizeof( WORD )*this->m_dwNumIndices );

	END_USEGBUF( t_iGBufIdx );

	return true;

	unguard;
}



/** \brief
*  ���������ݵĶ��롣
*
*  ����͵�ǰ��mesh����ͬ��*.gls�ļ����ڣ������ļ�Ϊ���������ļ���
*/
void osc_bodyPartMesh::read_weaponData( void )
{
	guard;

	s_string   t_szGName = m_strFname;

	osn_mathFunc::change_extName( t_szGName,GLOSSY_FHEADER );


	// ����ļ�������.
	if( !file_exist( (char*)t_szGName.c_str() ) )
		return;
	
	m_ptrSwordGlossy = new osc_swordGlossy();

# if __GLOSSY_EDITOR__
	m_ptrSwordGlossy->m_iMatrixIdx = 
		((os_characterVer*)m_ptrVerData)->m_vecBoneIdx[0];
# else
	m_ptrSwordGlossy->m_iMatrixIdx = m_ptrVerData.m_vecBoneIdx[0];
# endif 


	// ���뵶�����ݣ�
	int   t_iGBufIdx = -1;
	BYTE*   t_fstart = START_USEGBUF( t_iGBufIdx );
	int     t_iSize ;
	DWORD   t_dwVersion;
	char    t_szHeader[4];

	t_iSize = read_fileToBuf( (char*)t_szGName.c_str(),t_fstart,TMP_BUFSIZE );
	osassert( t_iSize >0 );

	// �����ļ�ͷ�Ͱ汾��.
	READ_MEM_OFF( t_szHeader,t_fstart,sizeof( char )*4 );
	osassert( strcmp( t_szHeader,GLOSSY_FHEADER ) == 0 );
	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	osassert( t_dwVersion >= GLOSSY_BPMFVERSION103 );

	READ_MEM_OFF( &m_bDisplay,t_fstart,sizeof( BOOL ) );
	
	READ_MEM_OFF( &m_ptrSwordGlossy->m_vec3Start,t_fstart,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_ptrSwordGlossy->m_vec3End,t_fstart,sizeof( osVec3D ) );

	READ_MEM_OFF( &m_ptrSwordGlossy->m_iActNum,t_fstart,sizeof( int ) );
	osassertex( m_ptrSwordGlossy->m_iActNum<MAX_ACTGLOSSY,"�����Ӧ�Ķ�������..\n" );
	READ_MEM_OFF( m_ptrSwordGlossy->m_vecActIdx,
		t_fstart,sizeof( int )*m_ptrSwordGlossy->m_iActNum );

	// 
	// ÿһ��������Ӧ�ĵ�����ɫ,��������˵õ����յ���ɫ.
	READ_MEM_OFF( m_ptrSwordGlossy->m_vecGlossyColor,
		t_fstart,sizeof( DWORD )*m_ptrSwordGlossy->m_iActNum );


	//
	// ����˵��⸽�ӵ���Ч���ݡ�
	// River @ 2009-12-98:֮��汾����������,effectType.֮ǰ�ֵ��壬֮��
	//                    �İ汾������Ҫ�ڲ����д���
	if( t_dwVersion < GLOSSY_BPMFVERSION106 )
	{
		int t_m = 0;
		READ_MEM_OFF( &t_m,t_fstart,sizeof( int ) );
	}

	READ_MEM_OFF( &m_ptrSwordGlossy->m_fEffectPos,t_fstart,sizeof( float ) );


	//! River @ 2010-3-5:������������Чλ�õ�֧��
	m_ptrSwordGlossy->m_fAddEffectPos[0] = m_ptrSwordGlossy->m_fEffectPos;
	m_ptrSwordGlossy->m_fAddEffectPos[1] = m_ptrSwordGlossy->m_fEffectPos;
	if( t_dwVersion >= GLOSSY_BPMFVERSION107 )
		READ_MEM_OFF( m_ptrSwordGlossy->m_fAddEffectPos,t_fstart,sizeof( float )*2 );


	//@{
	//! River @ 2009-12-7:����Զ��������Ч��֧��
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


	// 104�汾����Ե��ⳤ�ȵĴ���
	if( t_dwVersion >= GLOSSY_BPMFVERSION104 )
		READ_MEM_OFF( &m_ptrSwordGlossy->m_iGlossyMoveLength,t_fstart,sizeof( int ) );

	// 105�İ汾�����˶�������Ч���ŵ�֧��
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

	// River add @ 2009-9-4:ʹ����ĵ���.
	m_ptrSwordGlossy->m_iGlossyMoveLength = MAX_SWORDPOS;

	// ���㵶������ĵ㣬�ڵ���༭��ģʽ�£���������ڵ���������ص����ݡ�
	m_ptrSwordGlossy->m_vec3Center = 
		(m_ptrSwordGlossy->m_vec3Start+m_ptrSwordGlossy->m_vec3End)/2.0f;

	return;

	unguard;
}

//@{
// Windy mod @ 2005-9-1 17:48:12
//Desc: ride  �����һ����������ݵ��ļ��У�
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
*  ����skinMesh��ص����ݡ�
*
*  ��skin�ļ��е�������������õĶ������ݡ�
*  ����������д����豸��ص�����??????????
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
	// �������ݵ��ڴ档
	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( _file,t_fstart,TMP_BUFSIZE );
	if( t_iSize <= 0 )
		return false;
	

	//
	// �����ļ�ͷ. 
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str,SKN_FILEMAGIC ) != 0 )
	{
		osDebugOut( "<%s> file Format error!!!",_file );
		osassert( false );
		return false;
	}

	// ATTENTION TO FIX:�汾�ű����������ļ��İ汾�ţ�
	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	if( t_iSize < SKN_FILEVERSION )
	{
		osDebugOut( "<%s>�ļ��汾�Ų���ȷ...,Ŀǰ����֧��100�汾�ŵ�*.skn�ļ�...\n",_file );
		osassert( false );
		return false;
	}
	osassertex( t_iSize == SKN_FILEVERSION,
		"�µĳ���汾ֻ֧��101�汾��skn���ݣ�ʹ�ó��򹤾�ת��*.skn�ļ���101���ļ��汾" );

	//
	// �����ļ���Ϣ��
	READ_MEM_OFF( &m_dwID,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &m_dwParentID,t_fstart,sizeof( DWORD ) );

	READ_MEM_OFF( &this->m_dwVertexSize,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &this->m_dwFVF,t_fstart,sizeof( DWORD ) );

	READ_MEM_OFF( &this->m_dwNumPalette,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &m_dwNumInfluPV,t_fstart,sizeof( DWORD ) );


	READ_MEM_OFF( &m_dwNumIndices,t_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( &m_dwNumVertex,t_fstart,sizeof( DWORD ) );


	//
	// ����ÿһ��������matrix.
	osassertex( (m_dwNumPalette<=MAX_BONEPERBPM),"������Ŀ����..\n" );
	READ_MEM_OFF( &m_arrBoneMatrix[0],t_fstart,sizeof( osMatrix )*m_dwNumPalette );
	READ_MEM_OFF( &m_arrBoneId[0],t_fstart,sizeof( DWORD )*m_dwNumPalette );

	//
	// �����豸��ص�����,��������У�����Ҫʹ��ȫ�ֵĻ�����.
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

	// ATTENTION : Ŀǰֻ������ĵ�7���ֿ��Գ��ֵ��������Ч��
	char   t_ch = m_strFname[strlen(m_strFname)-WEAPON_PARTOFFSET];
	//@{
	// Windy mod @ 2005-8-30 19:20:25
	//Desc: ride ����Ƿ���������Ķ�λ�в���
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
	// ATTENTION : Ŀǰֻ������ĵ�7���ֿ��Գ��ֵ��������Ч��
	if( (t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR) )
	{
		m_ptrVerData = new BYTE[m_dwVertexSize*m_dwNumVertex];
		memcpy( m_ptrVerData,t_devdep.m_ptrDStart,m_dwVertexSize*m_dwNumVertex );
	}
# else

	// �ڽ���ʹ��ȫ�ֵĻ�����֮ǰ������һ���������ݡ�
	if( (t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR) )
		m_ptrVerData = *((os_characterVer*)t_devdep.m_ptrDStart);

# endif 

	END_USEGBUF( t_iGBufIdx );


	//
	// ���뵶����ص�����.ATTENTION TO FIX: ��ֺ�������
	osassertex( t_iSize != GLOSSY_BPMFVERSION,"102�汾������������Ҫ���±༭" );


	//@{
	// River @ 2005-5-30:
	// ���������ļ����ڣ�����뵶����ص����ݡ�
	if( (t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR) )
		read_weaponData();



	// 
	// ��ǰbpm����ʹ��״̬.
	m_bInuse = true;

	// River mod @ 2007-7-16:��˵Ĺ������Ԥ�ȵ���Ϊ����˵Ĺ���Ļ����������µ���
	for( int t_i=0;t_i<_ref;t_i ++ )
		add_ref();

	m_dwHash = string_hash( m_strFname );

	return true;

	unguard;
}


//! Set VIP Load�����̵߳��룬��ǰskinObj���ڶ��е���ǰ�棬������õ�char���ȼ������.
void osc_skinMesh::set_vipMTLoad( void )
{
	guard;

	static int t_sVipLevel = 0;

	t_sVipLevel++;
	m_iVipMTLoadLevel = t_sVipLevel;

	unguard;
}



/** \brief
*  ����ǰskinMesh��Frame Hierarchy����
*  
*  IMPORTANT:ʹ�������������������ϵͳ�����һ������ʹ�õĲ㼶ϵͳ��
*  
*  
*/
void osc_skinMesh::process_frameHierarchy( void )
{
	guard;

	os_boneIdx* t_ptrBIdx;

	osassertex( m_iBFNum == 0,"SkinMeshObj Release Not Clear!!" );

	// �����õ�0������ID.
	m_pRoot = get_freeBF();
	m_pRoot->set_id( 0 );
	m_pframeToAnimate[0]	= m_pRoot;
	m_pRoot->m_pParentSkin	= this;

	// 
	// ATTENTION: Ϊʲô�����1��ʼ��
	// 
	// A: ��0���Ѿ����������ù��˵�0����,����������,����������.
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


//! ������ǰskinMeshSystem��shader.
bool osc_skinMesh::create_smShader( os_skinMeshInit* _smi )
{
	guard;

	s_string        t_str,t_strIni;
	char            t_sz[128];
	osassert( _smi );
	os_bodyLook*    t_ptrLook = &_smi->m_smObjLook;

	//
	// �������ǵ�Character Shader����.
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
		// ����Character shader.
		sprintf( t_sz,"%d_%02d_%02d",t_i+1,t_ptrLook->m_mesh[t_i],t_ptrLook->m_skin[t_i] );

		t_str += t_sz;
		
		// TEST CODE:
		// River added @ 2006-8-31:��֤�ļ����ĳ���?
		osassert( strlen( t_sz ) > 5 );
		osassert( t_str.length() > 5 );


		t_strIni = t_str + ".";
		t_strIni += CHASEC_EXTENT;

		// ����Ƕ�������Ч������ʹ������Ĵ�������
		if( t_i == m_iMlFlagIdx )
		{
			create_mlFlagShader( t_i,(char*)t_str.c_str() );
			continue;
		}

		// ���������ﴴ������
		// River mod @ 2007-5-18:��findfirstfile�����޸�Ϊ�����ڲ���file_exist
		if( file_exist( (char*)t_strIni.c_str() ) )
		{
			create_doubleTexShader( t_i,(char*)t_str.c_str() );
		}
		else
		{
			// TEST CODE:
			// River added @ 2006-8-31:��֤�ļ����ĳ���?
			osassert( t_str.length() > 5 );
			if( !create_singleTexShader( t_i,(char*)t_str.c_str() ) )
				return false;
		}

	}

	// River @ 2007-3-22:���ϲ�ͳһ����,�˴����������⣬��Χ�����ݻ�����ȷ��
	//scale_skinMesh(osVec3D(_smi->m_Scale,_smi->m_Scale,_smi->m_Scale));

	return true;

	unguard;
}





//! �õ�����ʱ�õ���bis�ļ�����
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
*  ��Ŀ¼�ļ�����һ������ϵͳ��
*
*  River @ 2005-7-19: �����ݴ����Ӧ�÷���ֵ��
*/
bool osc_skinMesh::load_skinMeshSystem( os_skinMeshInit* _smi )
{
	guard;

	s_string       t_str,t_str1;
	osc_bodyFrame*  t_frame; 
	char           t_sz[128];


	osassert( _smi );
	osassert( strlen( _smi->m_strSMDir ) < MAX_BPMFNLEN );

	// River added @ 2009-12-30:�����ά����ר�õ����������Ⱦ����
	// ���ӵ���Ч
	m_bUseInInterface = _smi->m_bUseInInterface;
	m_bPlayWeaponEffect = _smi->m_bPlayWeaponEffect;

	// �Ƿ�д��Zbuffer, �Ƿ�֧����Ӱ���ݡ���
	m_bWriteZbuf = _smi->m_bWriteZbuf;
	if( _smi->m_bShadowSupport && g_bHeroShadow )
		m_bShadowSupport = TRUE;
	else
		m_bShadowSupport = FALSE;
	strcpy( m_strSMDir,_smi->m_strSMDir );
	//@{
	// Windy mod @ 2005-9-21 16:50:42
	//Desc: RIDE ���������ƫ��
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
	

	// ����õ�*.bis���ļ�����
	get_bisFileName( t_str,t_str1 );

	//
	// ��bone sys manager�еõ���ǰboneSys��ָ�롣
	m_ptrBoneSys = g_boneSysMgr->create_boneSysFromMgr( t_str.c_str() );
	osassertex( m_ptrBoneSys,"��������Ĺ���ϵͳʧ��" );
	if( !m_ptrBoneSys )
		return false;

	//
	// ��������Ĳ㼶ϵͳ��
	process_frameHierarchy();

	//
	// ���Ե���*.act�ļ�����
	m_ptrBoneAni = g_boneAniMgr->create_boneAni( 
		m_strSMDir,_smi->m_iTotalActTypeNum );
	if( !m_ptrBoneAni )
	{
		osDebugOut( "��������<%s>�Ķ�����Ϣʧ��...\n",_smi->m_strSMDir );
		osassertex( m_ptrBoneAni,"����act�ļ�ʧ��" );
		return false;
	}

	//
	// ���붥�����ݡ�����bodyLook�Ĳ�ͬ�����벻ͬ���ļ����ݡ�
	// 7�����岿λ�����һ�����岿λ�ݲ�����
	if( (_smi->m_smObjLook.m_iPartNum == (MAX_SKINPART-1))&&
		(_smi->m_smObjLook.m_mesh[WEAPON_BPMID] == 0 ) )
		_smi->m_smObjLook.m_iPartNum --;

	// ��8�����岿λ�����������ʱ������
	if( (_smi->m_smObjLook.m_iPartNum == MAX_SKINPART)&&
		(_smi->m_smObjLook.m_mesh[WEAPON_BPMID] == 0 )&&
		(_smi->m_smObjLook.m_mesh[WEAPON_BPMID+1] == 0) )
		_smi->m_smObjLook.m_iPartNum -= 2;

	m_iNumParts = _smi->m_smObjLook.m_iPartNum;

	os_bodyLook*  t_ptrLook = &_smi->m_smObjLook;
	t_str = m_strSMDir;
	
	for( int t_i=0;t_i<m_iNumParts;t_i++ )
	{
		// ����յ�mesh.
		if( t_ptrLook->m_mesh[t_i] == 0 )
			continue;

		//
		// �õ�Ҫ�����Mesh�ļ�����
		sprintf( t_sz,"_%02d_%02d%s",
			t_i+1,t_ptrLook->m_mesh[t_i],SKN_EXTENT );
		t_str = _smi->m_strSMDir;
		t_str += t_str1;
		t_str += t_sz;
		//@{
		// Windy mod @ 2005-8-30 19:23:50
		//Desc: ride 2 �������Ľṹ���־��SKINMESHΪ m_bIsRide���MESHΪ���
		//���ǰ��������һ�������⴦����loadBpmIns���һ������Ϊ�棬
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
		osassertex( t_frame,"�Ҳ����յ�frame����" );

		t_frame->AddMesh( &m_vecBpmIns[t_i] );

		// 
		// �༭������ص����ݡ�����ʱ�͸��������meshʱ��Ҫ����������ݡ�
# if __GLOSSY_EDITOR__
		weapon_process( t_i,t_p );
# endif 

		if( m_vecBpmIns[t_i].is_glossySM()&&t_i == WEAPON_PARTID7 )
			m_ptrSGBpmIns = &m_vecBpmIns[t_i];
		//else
		//	m_ptrSGBpmIns = NULL;
		//!windy add ���صڰ˲���
		if( m_vecBpmIns[t_i].is_glossySM()&&t_i == WEAPON_PARTID8 )
			m_ptrSGBpmIns8 = &m_vecBpmIns[t_i];
		//else
		//	m_ptrSGBpmIns8 = NULL;

	}

	// ����skinMesh��shader������ݡ�
	if( !create_smShader( _smi ) )
		return false;

	//
	// �����еĹ������ݺ�mesh��������������
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
	// cha�ļ����ݣ�ȫ�����ֹ�����
	osVec3D     t_vec3;

	// ʹ����������x����
	m_fAdjRgl = _smi->m_fAdjRotAgl-OS_PI/2; 
	m_vec3AdjPos = _smi->m_vec3AdjPos;
	m_fBSRadius = osVec3Length( &(_smi->m_vec3Max - _smi->m_vec3Min) )/2.0f;
	
	//  Ĭ��Ϊ 1.0f
	m_sCurSkinMeshAmbi.r = g_fAmbiR*CHAR_AMBISCALE;
	m_sCurSkinMeshAmbi.g = g_fAmbiG*CHAR_AMBISCALE;
	m_sCurSkinMeshAmbi.b = g_fAmbiB*CHAR_AMBISCALE;
	m_sCurSkinMeshAmbi.a = 1.0f;

	// ��Χ�е���.
	m_sAdjBBox.get_smOffsetAdjBBox( t_vec3,m_fAdjRgl + m_fRotAgl );
	m_sBBox.set_bbPos( m_vec3SMPos + t_vec3 );



	//
	// �������ٶ���ص���Ϣ��
	set_actSpeedInfo( m_fNewActSpeed );
	get_defaultAct( _smi );


	//@{ River @ 2009-6-7:���ϲ�Ĵ���ֱ���ƽ���.
# if __ALLCHR_SIZEADJ__
	// �����е�sm����������ķŴ���Ŵ�.
	if( !_sceneSm )
		createScale_withAllChrScale();
# endif 


	m_bUseDirLAsAmbient = _smi->m_bUseDirLAsAmbient;

	//@{
	// TEST CODE:ȷ��ÿһ��Frame����ȷ��bm��Ŀ.
	assert_frameBMNum();
	//@} 
	if (_smi->m_bIsRide)
		HideLastPart(true);


	// River @ 2010-1-7:��������Ļ�׼���ţ����������ĸ�����Ч�������׼����Ϊ����
	m_fBaseSkinScale = _smi->m_Scale;

	//  River @ 2007-3-22:�Ѿ����ڲ����Ź�
	scale_skinMesh( _smi->m_Scale,_smi->m_Scale,_smi->m_Scale );
	//@} 

	//! River@2009-6-23:�����ʼ��ʱ����ʾ����
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
//! ����ȷ��Frame�����岿��Ŀ��Ϊ��ĺ���.�ڴ���������л�װ��ǰ�����.
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
				sprintf( t_sz,"���������Ŀ¼:<%s>..bm��ĿΪ:<%d>..BPM NUM:<%d>..����Ĳ�λ:<%d>..\n",
					m_strSMDir,t_frame->m_iBMNum,m_iNumParts,t_i );
				osassertex( false,t_sz );		
			}
		}
	}

	unguard;
}
//@}



/** \brief
*  �滻ĳһ���岿λ��mesh.
*
*  �㷨������
*  1: �ȴ�frame�а�ԭ����meshȥ����
*  2: �ͷ����bpMesh.
*  3: �����µ�bpMesh
*  4: ������������ݡ�
*  
*  \param _bidx    ���岿λ��id.
*  \param _midx    Ҫ�滻��Mesh��id.
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

	osassertex( m_bInUse&&(m_iBFNum>0),"SkinMesh�Ѿ����ͷ�" );

	//@{
	// TEST CODE: �²�assert,ʹֻ�ܰ����������Ĳ�λ��Ϊ��.����ȥ���˶δ���.
	if( _midx == 0 )
		osassertex( (_bidx==6)||(_bidx==7),"ֻ�ܰ������Ĳ�λ��Ϊ��....\n" );
	//@}


	t_oldBpmIns = &m_vecBpmIns[_bidx];
	t_p = m_vecBpmIns[_bidx].get_bpmPtr();
	if( t_p )
	{
		// ���Ҫ��Ϊ�յ�mesh,����Ҫ����
		t_iShaIdBack = m_vecBpmIns[_bidx].m_iShaderID;
		
		t_frame = m_pRoot->FindFrame( t_p->m_dwID );
		if( t_frame == NULL )
			osassert( false );
		t_iOldFrameId = t_p->m_dwID;

		// �ͷŵ�ǰ��bpmInstance.
		m_vecBpmIns[_bidx].release_bpmIns();

		t_bMeshExist = true;
	}

	//! river added @ 2010-2-2:ȷ���ײ�ɾ��������,
	//  ��ôд�����
	//if( _bidx >= m_iNumParts )
	//	t_bMeshExist = false;


	// 
	// ����յ�mesh.
	if( _midx == 0 )
	{
		// �ͷ�ʱ��Ҫע��-1.
		if( t_bMeshExist )
		{
			t_frame->change_Mesh( &m_vecBpmIns[_bidx],NULL );
			osassert( t_iShaIdBack>=0 );
		}

		// 
		// �����Ӧ������BPM.
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
			osassertex( false,"��������λ�����滻Ϊ��" );

		return true;
	}

	//
	// ����õ�Ҫ�����Mesh�ļ���,û��Ŀ¼��û����չ�������֡�
	// ATTENTION TO OPP: ���Ը���Ч��
	t_str = m_strSMDir;
	t_idx = (int)t_str.rfind( '\\' );
	t_iLength = (int)t_str.length();
	t_str1 = '\\';
	t_str1 += t_str.substr( t_idx+1,t_iLength-t_idx-1 );


	//
	// ��������µ�Mesh.
	sprintf( t_sz,"_%02d_%02d%s",
		_bidx+1,_midx,SKN_EXTENT );
	t_str = m_strSMDir;
	t_str += t_str1;
	t_str += t_sz;


	//! River @ 2010-12-21:�����Ƿ�������Ĳ�����֮ǰһֱ�Ǵ���ĵ���
	if( !m_vecBpmIns[_bidx].loadBpmIns( (char*)t_str.c_str(),
		m_bPlaySGlossy,m_bPlayWeaponEffect,m_bShadowSupport,
		this->m_bIsRide,m_bUseInInterface,this ) )
		return false;

	osc_bodyPartMesh* t_np = m_vecBpmIns[_bidx].get_bpmPtr();


	//
	// ���ԭ����ǰ�����岿λû�ж�Ӧmesh�Ļ�����Ҫ����frame,�����´�����mesh
	// ��frame����������
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
		// ���ԭBpmIns��FrameId���µ�BpsIns��FrameId��ͬ��
		// ATTENTION TO OPP: �������,Ӧ���и����ŵĳ������
		if( t_iOldFrameId != t_np->m_dwID )
		{
			// �Ȱ�ԭFrame�����㡣
			t_frame->change_Mesh( &m_vecBpmIns[_bidx],NULL );

			// ���µ�Frame�ڼ���Mesh.
			m_vecBpmIns[_bidx].m_iShaderID = -1;
			t_frame = m_pRoot->FindFrame( t_np->m_dwID );
			osassert( t_frame );

			t_frame->AddMesh( &m_vecBpmIns[_bidx] );
		}
	}


	// 
	// ����滻����������bodyPartMesh,����µ������Ƿ��е��⡣
	if( _bidx == WEAPON_BPMID )
	{	
		// 
		// �����������ֵ�bodyPartMesh.
# if __GLOSSY_EDITOR__
		// �����ǰû�е����������������Ч��Ĭ�ϴ���
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
	// ����滻����������bodyPartMesh,����µ������Ƿ��е��⡣
	if( _bidx == WEAPON_PARTID8 )
	{	
		// 
		// �����������ֵ�bodyPartMesh.
# if __GLOSSY_EDITOR__
		// �����ǰû�е����������������Ч��Ĭ�ϴ���
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

	//  ATTENTION TO OPP: ֻ��Ҫ���������λ��Palette
	m_pRoot->LinkBones( m_pRoot );

	// �ӿյ��У�����һ�����岿λ��
	// �ӿյ��У�����һ�����岿λ��
	if( !t_bMeshExist )
	{
		//�� River @ 2010-1-21:ʹ�ø��Ӿ�ȷ���㷨��
		if( m_iNumParts < (_bidx + 1) )
		{
			m_iNumParts = _bidx + 1;
			m_Look.m_iPartNum = m_iNumParts;
		}
	}

	// River added @ 2009-9-8:ȷ�ϲ��ŵ�����Ч
	if( (_bidx == WEAPON_BPMID)||( _bidx == WEAPON_PARTID8 ) )
		playSwordGlossy( m_bPlaySGlossy, m_bPlayWeaponEffect );

	osassertex( m_iNumParts<=8,"���岿λ�����ܳ���8...\n" );


	return true;

	unguard;
}

/** \brief
*  �滻ĳһ���岿λ��skin.
*
*  \param _bidx    ���岿λ��id.
*  \param _midx    Ҫ�滻��Mesh��id.
*  \param _sidx    Ҫ�滻��skin��id.
*/
bool osc_skinMesh::change_bodySkin( int _bidx,int _midx,int _sidx )
{
	guard;// osc_skinMesh::change_bodySkin() );

	s_string    t_str,t_strIni;
	char        t_sz[128];

	//
	// ������岿λidΪ�գ���ʲô������Ҫ�滻��
	if( _midx <= 0 )
		return true;

	//
	// ���ͷ�ԭ����SkinShader
	// River @ 2005-6-24: ����˴������ͷ���mesh,������ĺ������ã�ʲô��û����
	//                    ��bodyPartMesh instance���ͷ��У������˶���Щ���ݵ��ͷš�
	release_skinShader( _bidx );


	//��
	// ��������µ�Mesh.
	sprintf( t_sz,"%d_%02d_%02d",_bidx+1,_midx,_sidx );
	t_str = m_strSMDir;
	t_str += '\\';
	t_str += t_sz;


	// 
	// �ж�Ҫ������skin�����͡�
	t_strIni = t_str + ".";
	t_strIni += CHASEC_EXTENT;

	// ���������ﴴ������
	// River mod @ 2007-6-5:��findfirstfile�����޸�Ϊ�����ڲ���file_exist
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

//! �л����ĵ���ͼ
bool osc_skinMesh::change_mlFlagTexture( const char* _tex1,
											 const char* _tex2,const char* _tex3 )
{
	guard;

	osc_effect*   t_ptrEffect;

	// ATTENTION TO FIX: 
	// ��Ϊ��EffectΪȫ��Ψһ������Ŀǰ������ֻ֧�ֳ�һ�����ģ�����Ƕ������ģ�
	// ��һ�����Ļ�װ������������Ҳ����Ż�װ��
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
*  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
*
*  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
*  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
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

