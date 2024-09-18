/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osAniMesh.cpp
 * 
 *  His:      River created @ 2004-5-12
 *
 *  Desc:     ������ص�mesh�����ݽṹ���Ӵ�6�ĵ�������еõ����ݡ�
 *  
 *  
 *  ��ȡ�����ϣ��������У�ȡ�����У��������¡�
 *   
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osAniMesh.h"
# include "../include/middlePipe.h"
# include "../../backpipe/include/mainEngine.h"
# include "../../mfPipe/include/osCharacter.h"
# include "../../mfPipe/include/osEffect.h"

//! ��ǰaniMesh���ļ��汾��
# define  ANIMESH_FILEVERSION    0x100
//! �����Ƿ��ֵ��aniMesh�ļ��汾��0x101�汾,
# define  ANIMESH_FILEV101       0x101
//! 0x102�汾�����Ƿ�ͱ��������Ⱦ��ѡ��.
# define  ANIMESH_FILEV102       0x102
//! 0x103�汾�������Ƿ�ʹ�ö�����ɫ�����ʹ�ö�����ɫ��������ļ���ʽ��
# define  ANIMESH_FILEV103       0x103
//! 0x104�汾�����˵�ǰosa��Ʒ�Ƿ��ܻ�����Ӱ���ѡ��
# define  ANIMESH_FILEV104       0x104
//! 0x105�İ汾�������Ƿ���Ⱦ�ظ�����
# define  ANIMESH_FILEV105       0x105
//! 0x106�İ汾�����Ƿ�ʹ�ùؼ�֡����������ģ�Ͷ���
# define  ANIMESH_FILEV106       0x106

//! ��ǰaniMesh�ļ���magicid,
# define  ANIMESH_MAGIC          "osa"



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// osc_aniMFace
os_aniMFace::os_aniMFace( void )
{
	m_ptrFace = NULL;
	m_ptrMaterialList = NULL;
	m_ptrfTimekey = NULL;
	m_iMaterialNum = 0;

}
os_aniMFace::~os_aniMFace( void )
{
	release_aniMFace();
}


//! ���ļ��е���aniMesh���ݡ�
bool os_aniMFace::load_aniMeshFromMem( BYTE*& _ds,bool _bAniMat )
{
	guard;

	osassert( _ds );

	READ_MEM_OFF( &this->m_cFlag,_ds,sizeof( BYTE ) );
	READ_MEM_OFF( &this->m_Material,_ds,sizeof( os_material ) );
	READ_MEM_OFF( m_szTexName,_ds,sizeof( char )*FILENAME_LEN );	
	READ_MEM_OFF( &m_nFace,_ds,sizeof( int ) );

	// ȷ�ϵ�ǰ��mesh����������0.
	osassert( m_szTexName[0] );
	osassert( !((m_nFace==0)&&(_bAniMat)) );

	// �������Ϊ��,����Ҫ����.
	if( m_nFace == 0 )
		return false;


	m_ptrFace = new os_refFace[m_nFace];
	READ_MEM_OFF( m_ptrFace,_ds,sizeof( os_refFace )*m_nFace );


	// �����ǰģ���в��ʶ������������ʶ������ݡ�
	if( _bAniMat )
	{
		READ_MEM_OFF( &m_iMaterialNum,_ds,sizeof( int ) );
		osassert( m_iMaterialNum >= 0 );

		m_ptrMaterialList = new os_material[m_iMaterialNum];
		m_ptrfTimekey = new float[m_iMaterialNum];

		READ_MEM_OFF( m_ptrfTimekey,_ds,sizeof( float )*m_iMaterialNum );
		READ_MEM_OFF( m_ptrMaterialList,_ds,sizeof( os_material )*m_iMaterialNum );
	}

	// 
	// �����豸��ص�material���ݡ�
	memset( &m_deviceMat,0,sizeof( D3DMATERIAL9 ) );
	m_deviceMat.Diffuse = osColor( m_Material.m_dwDiffuse.m_cRed / 255.0f,
		m_Material.m_dwDiffuse.m_cGreen / 255.0f,m_Material.m_dwDiffuse.m_cBlue / 255.0f,
		m_Material.m_dwDiffuse.m_cAlpha / 255.0f );
	m_deviceMat.Ambient = osColor( m_Material.m_dwAmbient.m_cRed / 255.0f,
		m_Material.m_dwAmbient.m_cGreen / 255.0f,m_Material.m_dwAmbient.m_cBlue / 255.0f,
		m_Material.m_dwAmbient.m_cAlpha / 255.0f );
	m_deviceMat.Power = m_Material.m_fShininess;

	//
	// ����в��ʶ���, ����alphaֵ.
	if( _bAniMat )
	{
		m_vecAlpha.resize( this->m_iMaterialNum );
		for( int t_i = 0;t_i<m_iMaterialNum;t_i ++ )
		{
			m_vecAlpha[t_i] = m_ptrMaterialList[t_i].m_dwDiffuse.m_cAlpha / 255.0f;
		}
	}

	return true;

	unguard;
}


//  ATTENTION : ʹ�ò�ֵ�㷨?
//! �õ���ǰmaterial���ݡ�
void os_aniMFace::get_currentMaterial( D3DMATERIAL9& _mat,osc_aniMeshIns* _ins )
{
	guard;

	if( m_iMaterialNum > 1 )
	{
		float   t_f;

		memset( &_mat,0,sizeof( D3DMATERIAL9 ) );
		_mat.Ambient.r = _mat.Ambient.g = _mat.Ambient.b = 1.0f;
		
		t_f = m_vecAlpha[_ins->m_iCurMatFrame+1]-m_vecAlpha[_ins->m_iCurMatFrame];
		t_f *= _ins->m_fMatFactor;
		_mat.Ambient.a = m_vecAlpha[_ins->m_iCurMatFrame]+t_f;
		_mat.Power = 1.0f;
		_mat.Diffuse.r = _mat.Diffuse.g = _mat.Diffuse.b = 1.0f;
		_mat.Diffuse.a = _mat.Ambient.a;

	}	
	else
	{
		memcpy( &_mat,&m_deviceMat,sizeof( D3DMATERIAL9 ) );
	}


	return;

	unguard;
}

//! �ͷŵ�ǰ��aniMFace.
void os_aniMFace::release_aniMFace( void )
{
	guard;

	SAFE_DELETE_ARRAY( m_ptrFace );
	SAFE_DELETE_ARRAY( m_ptrMaterialList );
	SAFE_DELETE_ARRAY( m_ptrfTimekey );

	unguard;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// os_rootNode

/** \brief
*  �õ���ǰ������ڲ��ܹ��������Ŀ��
*
*  ÿһ��rootNodeʹ��һ����Ķ��㻺������
*  ����н���morphFrame��Ŀ����1,���rootNode
*  ����ʹ��һ����Ķ��㻺���������ض�����ĿΪ-1;
*  
*/
int os_rootNode::get_totalVerNum( void )
{
	guard;

	os_aniNode*  t_ptrNode;
	bool         t_bUseOneVB = true;

	int t_iVerNum = 0;
	for( int t_i=0;t_i<m_iChildNum;t_i ++ )
	{
		t_ptrNode = &m_arrNodelist[t_i];

		if( t_ptrNode->m_iMorphFrames == 1 )
			t_iVerNum += (t_ptrNode->m_iVertex);
	}

	// �������ʹ�þ�̬�Ķ��㻺�������򷵻�-1;
	return t_iVerNum;

	unguard;
}


//! �ͷŵ�ǰ�����ͷ�������ݡ�
void os_rootNode::release_devdepData( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	if( m_iVBId >= 0 )
	{
		_pipe->release_staticVB( m_iVBId );
		m_iVBId = -1;
	}

	for( int t_i=0;t_i<this->m_iChildNum;t_i ++ )
		m_arrNodelist[t_i].release_devdepData( _pipe );

	m_iChildNum = 0;

	unguard;
}




/** \brief
*  ����ǰrootNode���豸������ݡ�
*
*  ���������ÿһ�����Ķ������ݺ��������ݡ�
*/
# define MAX_OSACHILDNUM       50
bool os_rootNode::create_devdepData( osc_middlePipe* _pipe )
{
	guard;

	int              t_i,t_iTotalVerNum,t_iVerSize;
	os_vbufFillData   t_arrVBFdata[MAX_OSACHILDNUM];
	int              t_iVBNum = 0;

	osassert( _pipe );
	osassertex( (m_iChildNum<MAX_OSACHILDNUM),"�ӽ����Ŀ���ܳ���50��" );

	//
	t_iTotalVerNum = get_totalVerNum();

	if( t_iTotalVerNum > 0 )
	{
		if( m_arrNodelist[0].m_bColorVertex )
			t_iVerSize = sizeof( os_objectColVertex );
		else
			t_iVerSize = sizeof( os_objectVertex );

		m_iVBId = _pipe->create_staticVB( t_iVerSize,t_iTotalVerNum,false );

		// �ռ��������ݡ�
		for( t_i=0;t_i<m_iChildNum;t_i ++ )
		{
			m_arrNodelist[t_i].create_devDepData( 
			      _pipe,m_iVBId,t_arrVBFdata,t_iVBNum );
		}
		
		// ��䶥�㻺�������ݡ�
		if( !_pipe->fill_staticVB( m_iVBId,t_arrVBFdata,t_iVBNum ) )
			return false;
	}
	// ʹ�ö�̬�Ķ��㻺������
	else
	{
		for( t_i=0;t_i<m_iChildNum;t_i ++ )
			m_arrNodelist[t_i].create_devDepData( _pipe,-1,t_arrVBFdata,t_iVBNum );
	}

	return true;

	unguard;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// osc_aniNode
os_aniNode::os_aniNode( void )
{
	//first set all pointer as zero....
	m_arrVertex = NULL;
	m_arrColVertex = NULL;
	m_iMorphFrames = 0;
	m_fTimeKey = NULL;
	m_iVertex = 0;

	m_iNumOfMesh = 0;
	m_arrMesh = NULL;

	m_arrAffineParts = NULL;
	m_arrFAffTimeKey = NULL;
	m_iAffinePartsNum = 0;

	m_bIsGeoNode = false;

	m_bNotInterpolate = false;
	m_bColorVertex = false;
	
	m_bUseKeyMat = false;
	m_arrKeyMatrix = NULL;
}

os_aniNode::~os_aniNode( void )
{
	release_aniNode();
}

//! �ͷŵ�ǰ��aniNode���ݡ�
void os_aniNode::release_aniNode( void )
{
	// �ͷ�aniMFace�ڵ����ݡ�
	if( (m_iNumOfMesh > 0)&&m_arrMesh )
	{
		for( int t_i=0;t_i<this->m_iNumOfMesh;t_i ++ )
			m_arrMesh[t_i].release_aniMFace();
	}

	SAFE_DELETE_ARRAY( m_arrVertex );
	SAFE_DELETE_ARRAY( m_arrColVertex );
	SAFE_DELETE_ARRAY( m_fTimeKey );
	SAFE_DELETE_ARRAY( m_arrMesh );

	SAFE_DELETE_ARRAY( m_arrAffineParts );
	SAFE_DELETE_ARRAY( m_arrFAffTimeKey );

	SAFE_DELETE_ARRAY( m_arrKeyMatrix );

}


//! �õ���ǰ��㲥����Ҫ��ʱ�䡣
float os_aniNode::get_playNodeTime( void )
{
	guard;

	float   t_fRes = 0.0f;

	if( m_iAffinePartsNum > 1 )
		t_fRes = m_arrFAffTimeKey[m_iAffinePartsNum-1];

	if( m_iMorphFrames> 1 )
	{
		if( t_fRes < m_fTimeKey[m_iMorphFrames-1] )
			t_fRes = m_fTimeKey[m_iMorphFrames-1];
	}

	return t_fRes;

	unguard;
}


//! ���ļ��е���aniNode����,��ƫ�����ݵ�ָ�롣
void os_aniNode::load_aniNodeFromMem( BYTE*& _ds,bool _bAniMat )
{
	guard;

	osassert( _ds );

	//
	// ����affine Parts��ص����ݽṹ��
	READ_MEM_OFF( &m_iAffinePartsNum,_ds,sizeof( int ) );

	// 2006-7-22:����ؼ�֡����
	if( this->m_bUseKeyMat )
	{
		this->m_arrKeyMatrix = new osMatrix[m_iAffinePartsNum];
		READ_MEM_OFF( m_arrKeyMatrix,_ds,
			sizeof( osMatrix )*m_iAffinePartsNum );
	}
	else
	{
		m_arrAffineParts = new os_AffineParts[m_iAffinePartsNum];	
		READ_MEM_OFF( m_arrAffineParts,_ds,
			sizeof(os_AffineParts)*m_iAffinePartsNum ); 
	}

	m_arrFAffTimeKey = new float[m_iAffinePartsNum];
	READ_MEM_OFF( m_arrFAffTimeKey,_ds,
		sizeof( float )*m_iAffinePartsNum );
	
	// 
	// ���붥����ص����ݡ�
	READ_MEM_OFF( &m_iMorphFrames,_ds,sizeof( int ) );
	READ_MEM_OFF( &m_iVertex,_ds,sizeof( int ) );
	
	//
	// ���ʹ�ö����ɫ
	if( m_bColorVertex )
	{
		osassert( m_iMorphFrames == 1 );
		m_arrColVertex = new os_objectColVertex[m_iVertex];
		READ_MEM_OFF( m_arrColVertex,_ds,
			sizeof( os_objectColVertex )*m_iVertex );
	}
	else
	{
		m_arrVertex = new os_objectVertex[m_iMorphFrames*m_iVertex];
		READ_MEM_OFF( m_arrVertex,_ds,sizeof
			( os_objectVertex )*m_iMorphFrames*m_iVertex );
	}

	m_fTimeKey = new float[m_iMorphFrames];
	READ_MEM_OFF( m_fTimeKey,_ds,sizeof( float )*m_iMorphFrames );


	// 
	// ����mesh��ص����ݡ�
	READ_MEM_OFF( &m_iNumOfMesh,_ds,sizeof( int ) );
	m_arrMesh = new os_aniMFace[m_iNumOfMesh];
	for( int t_i=0;t_i<m_iNumOfMesh;t_i++ )
	{
		m_arrMesh[t_i].load_aniMeshFromMem( _ds,_bAniMat );
	}


	READ_MEM_OFF( &m_matNodetm,_ds,sizeof(osMatrix) );
	READ_MEM_OFF( &m_bbox.m_vecMax,_ds,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_bbox.m_vecMin,_ds,sizeof( osVec3D ) );

	//
	// ���㵱ǰaniNode��Bounding Sphere.
	osVec3D   t_vec,t_vec3Max,t_vec3Min;
	osMatrix  t_scaleMat;
	
# if 1
	get_scaleKey( t_scaleMat,NULL );	
	osVec3Transform( &t_vec3Min,&m_bbox.m_vecMin,&t_scaleMat );
	osVec3Transform( &t_vec3Max,&m_bbox.m_vecMax,&t_scaleMat );
# endif 

	osVec3Lerp( &m_bsphere.veccen,&m_bbox.m_vecMax,&m_bbox.m_vecMin,0.5f );
	t_vec = (t_vec3Max - t_vec3Min)/2.0f;
	m_bsphere.radius = osVec3Length( &t_vec );


	//@{
	// River mod 2010-7-9:ԭ��û����δ���..
	//osMatrixIdentity( &t_scaleMat );
	//get_posKey( t_scaleMat,NULL );
	//osVec3Transform( &m_bsphere.veccen,&m_bsphere.veccen,&t_scaleMat );
	//@}


	osassertex( fabs(m_bsphere.radius) < 1300.0f,"������Ʒ�İ뾶����" );

	// �õ����ŵ�ǰ�Ľ����Ҫ��ʱ��
	m_fPlayTotalTime = get_playNodeTime();
	
	if( m_iMorphFrames > 1 )
		m_vecAPVertex.resize( m_iVertex );

	return;

	unguard;
}

//! ��䵱ǰaniNode��Ⱦʹ�õĶ���Ρ�
void os_aniNode::fill_rPoly( void )
{
	guard;

	m_sPolygon.m_bUseStaticVB = true;
	m_sPolygon.m_sBufData.m_iVbufId = m_iVerBufId;
	m_sPolygon.m_sBufData.m_iVertexStart = m_iStartVer;

	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerNum = m_iVertex;

	if( m_bColorVertex )
		m_sPolygon.m_iVerSize = sizeof( os_objectColVertex );
	else
		m_sPolygon.m_iVerSize = sizeof( os_objectVertex );

	unguard;
}


//! �ͷŵ�ǰnode���豸��ء����ݡ�
bool os_aniNode::release_devdepData( osc_middlePipe* _pipe )
{
	guard;

	// �����ʹ�ö�̬�Ķ��㻺����������Ҫ�ͷš�
	// �����ʹ�þ�̬�Ķ��㻺����������rootNode���Ѿ��ͷš�

	osassert( _pipe );

	// 
	// �ͷ�shader��ص����ݡ�
	for( int t_i=0;t_i<this->m_iNumOfMesh;t_i ++ )
	{
		osassert( m_vecShaderId[t_i]>=0 );
		g_shaderMgr->release_shader( m_vecShaderId[t_i] ); 
		m_vecShaderId[t_i] = -1;

		if( m_vecIdxBufId[t_i] >= 0 )
			_pipe->release_staticIB( m_vecIdxBufId[t_i] );
	}

	m_vecShaderId.clear();
	m_vecIdxBufId.clear();
	m_iNumOfMesh = 0;

	return true;

	unguard;
}

//  TEST CODE:
//! ȷ��shaderId�Ŀ����ԡ�
bool os_aniNode::assert_devdepData( bool _assert/* = true*/ )
{
	guard;

	// 
	// �ͷ�shader��ص����ݡ�
	for( int t_i=0;t_i<this->m_iNumOfMesh;t_i ++ )
	{
		osassert( m_vecShaderId[t_i]>=0 );
		if( !g_shaderMgr->assert_shaderId( m_vecShaderId[t_i],_assert ) )
			return false;
	}

	return true;

	unguard;
}



//! ������ǰnode���豸������ݡ�
bool os_aniNode::create_devDepData( osc_middlePipe* _pipe,
		         int _vbId,os_vbufFillData* _vbfill,int& _vbNum )
{
	guard;

	osassert( _pipe );
	osassert( _vbfill );

	// 
	// �ȴ������㻺����,�����morphFrame���򴴽���̬�Ķ��㻺������
	if( m_iMorphFrames == 1 )
	{
		osassert( _vbId >= 0 );
		m_iVerBufId = _vbId;

		if( m_bColorVertex )
		{	
			_vbfill[_vbNum].m_iVerNum = m_iVertex;
			_vbfill[_vbNum].m_ptrDstart = (char*)m_arrColVertex;
			m_iStartVer = 0;
			for( int t_i=0;t_i<_vbNum;t_i ++ )
			{
				m_iStartVer += _vbfill[t_i].m_iVerNum;
			}
			_vbNum ++;
		}
		else
		{	
			_vbfill[_vbNum].m_iVerNum = m_iVertex;
			_vbfill[_vbNum].m_ptrDstart = (char*)m_arrVertex;
			m_iStartVer = 0;
			for( int t_i=0;t_i<_vbNum;t_i ++ )
			{
				m_iStartVer += _vbfill[t_i].m_iVerNum;
			}
			_vbNum ++;
		}
	}
	else
	{
		osassert( !m_bColorVertex );
		DWORD t_fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;  
		m_iVerBufId = _pipe->create_dynamicVB( t_fvf );

		m_iStartVer = 0;
		osassert( m_iVerBufId >= 0 );
	}


	// 
	// ��ÿһ��mesh����index buffer.
	m_vecIdxBufId.resize( m_iNumOfMesh );
	m_vecShaderId.resize( m_iNumOfMesh );
	for( int t_i=0;t_i<m_iNumOfMesh;t_i++ )
	{
		m_vecIdxBufId[t_i] = -1;
		if( m_arrMesh[t_i].m_nFace <= 0 )
			continue;

		m_vecIdxBufId[t_i] = _pipe->create_staticIB( 
			m_arrMesh[t_i].m_ptrFace,m_arrMesh[t_i].m_nFace*3 );
		osassert( m_vecIdxBufId[t_i] >= 0 );
	}

	//
	// ����ÿһ��shaderId.
	s_string       t_str;
	int           t_idx;

	m_bTotalAlphaFace = true;
	for( int t_i=0;t_i<m_iNumOfMesh;t_i++ )
	{
		t_str = m_arrMesh[t_i].m_szTexName;
		t_idx = (int)t_str.rfind( '.' );
		t_str = t_str.substr( 0,t_idx );

		// ������ǰMesh�õ���Shader�����Դ��
		m_vecShaderId[t_i] = g_shaderMgr->add_shader( 
			t_str.c_str(), osc_engine::get_shaderFile( "object" ).c_str(),"osa" );
		
		if( m_vecShaderId[t_i] < 0 )
		{
			m_vecShaderId[t_i] = g_shaderMgr->add_shadertomanager( 
				(char*)t_str.c_str(),OBJECT_SHADERINI,osc_engine::m_iObjectShaId );
		}

		osassert( m_vecShaderId[t_i] >= 0 );

		m_arrMesh[t_i].m_bAlphaMat = g_shaderMgr->has_alphaChannel( m_vecShaderId[t_i] );

		// �������һ����͸��������
		if( m_arrMesh[t_i].m_bAlphaMat )
			m_bExistAlphaFace = true;
		else
			m_bTotalAlphaFace = false;
	}

	// �����Ⱦ����Ρ�
	fill_rPoly();

	return true;

	unguard;
}


//! �õ���ǰ��poskey��λ�á�
void os_aniNode::get_posKey( osMatrix& _mat,osc_aniMeshIns* _ins )
{
	guard;

	osVec3D   t_vec3,t_vec3Next;

	// �õ���ǰ֡����һ֡��λ��
	if( m_iAffinePartsNum > 1 && _ins)
	{
		osassert( _ins );

		memcpy( &t_vec3,&m_arrAffineParts[_ins->m_iCurPosFrame].t,sizeof( osVec3D ) );

		memcpy( &t_vec3Next,
			&m_arrAffineParts[_ins->m_iCurPosFrame+1].t,sizeof( osVec3D ) );

		osVec3Lerp( &t_vec3,&t_vec3,&t_vec3Next,_ins->m_fPFactor );
	}
	else
		memcpy( &t_vec3,&m_arrAffineParts[0].t,sizeof( osVec3D ) );

	osMatrixTranslation( &_mat,t_vec3.x,t_vec3.y,t_vec3.z );

	return;

	unguard;
}


//! �õ���ǰ��rotkey���ݡ�
void os_aniNode::get_rotKey( osMatrix& _mat,osc_aniMeshIns* _ins )
{
	guard;

	osQuat   t_quat,t_quatNext;

	if( this->m_iAffinePartsNum > 1 && _ins)
	{
		osassert( _ins );

		memcpy( &t_quat,&this->m_arrAffineParts[_ins->m_iCurPosFrame].q,sizeof( osQuat ) );
		memcpy( &t_quatNext,&m_arrAffineParts[_ins->m_iCurPosFrame+1].q,sizeof( osQuat ) );

		osQuaternionSlerp( &t_quat,&t_quat,&t_quatNext,_ins->m_fPFactor );
	}
	else
		memcpy( &t_quat,&m_arrAffineParts[0].q,sizeof( osQuat ) );

	osMatrixRotationQuaternion( &_mat,&t_quat );

	return;
	
	unguard;
}

//! �õ���ǰ��scaleKey���ݡ�
void os_aniNode::get_scaleKey( osMatrix& _mat,osc_aniMeshIns* _ins )
{
	guard;

	osVec3D   t_vec3,t_vec3Next;
	osQuat    t_quat,t_quatNext,t_quatInverse;
	osMatrix  t_mat,t_mat1;


	if( m_iAffinePartsNum > 1 && _ins)
	{
		osassert( _ins );

		memcpy( &t_vec3,&m_arrAffineParts[_ins->m_iCurPosFrame].k,sizeof( osVec3D ) );
		memcpy( &t_quat,&m_arrAffineParts[_ins->m_iCurPosFrame].u,sizeof( osQuat ) );

		memcpy( &t_vec3Next,
			&m_arrAffineParts[_ins->m_iCurPosFrame+1].k,sizeof( osVec3D ) );
		memcpy( &t_quatNext,
			&m_arrAffineParts[_ins->m_iCurPosFrame+1].u,sizeof( osQuat ) );

		osQuaternionSlerp( &t_quat,&t_quat,&t_quatNext,_ins->m_fPFactor );
		osVec3Lerp( &t_vec3,&t_vec3,&t_vec3Next,_ins->m_fPFactor );
	}
	else
	{
		memcpy( &t_vec3,&m_arrAffineParts[0].k,sizeof( osVec3D ) );
		memcpy( &t_quat,&m_arrAffineParts[0].u,sizeof( osQuat ) );
	}


	// ���ӵ�ת����ϵ����������⣬�ο�aniDisplay�е�ת����
	osQuaternionInverse( &t_quatInverse,&t_quat );
	osMatrixScaling( &t_mat1,t_vec3.x,t_vec3.y,t_vec3.z );
	
	osMatrixRotationQuaternion( &t_mat,&t_quatInverse );
	t_mat = t_mat * t_mat1;

	osMatrixRotationQuaternion( &t_mat1,&t_quat );
	_mat = t_mat * t_mat1;
	return ;

	unguard;
}


/** \brief
 *   ����morph Frame,��䴦���Ķ��㡣
 *
 *   �����ڴ��ж����ݽ��в�����������ɺ�Ѵ���Ķ�����䵽��̬�Ķ�
 *   �㻺�����С�
 */
bool os_aniNode::process_morphFrame( osc_aniMeshIns* _ins )
{
	guard;

	osassert( _ins );

	os_objectVertex    *t_arrSrc,*t_arrDst;


	t_arrSrc = &m_arrVertex[_ins->m_iCurMorphFrame*m_iVertex];
	t_arrDst = &m_arrVertex[(_ins->m_iCurMorphFrame+1)*m_iVertex];

	// River mod @ 2008-12-23:���һ֡�͵�һ֮֡�䣬�������겻����ֵ
	//if( m_iCur

	for( int t_i=0;t_i<this->m_iVertex;t_i ++ )
		objver_lerp( &m_vecAPVertex[t_i],&t_arrSrc[t_i],&t_arrDst[t_i],_ins->m_fMFactor );

	//
	// ����м�ܵ��Ķ�̬���㻺������
	m_iStartVer = _ins->m_ptrMPipe->fill_dnamicVB( m_iVerBufId,
		&m_vecAPVertex[0],sizeof( os_objectVertex ),m_iVertex );
	if( m_iStartVer < 0 )
	{
		osassert( false );
		return false;
	}

	return true;

	unguard;
}


//! �򵥵���Ⱦ��������ȾaniMesh��ͷһ֡��ָ����λ�ú����š�
bool os_aniNode::render_aniNode( osc_middlePipe* _pipe,float _rotAgl,
								osVec3D& _pos,osVec3D& _scale,float _alphaV )
{
	guard;

	osMatrix       t_matWorld,t_posMat,t_tmpRot,t_scaleMat,t_rotMat,t_s1;

	osassert( _pipe );
	osassert( m_iAffinePartsNum == 1 );
//	osassert( m_iMorphFrames == 1 );
	osassert( (_alphaV>=0.0f)&&(_alphaV<=1.0f) );



	osMatrixScaling( &t_scaleMat,_scale.x,_scale.y,_scale.z );
	osMatrixTranslation( &t_posMat,_pos.x,_pos.y,_pos.z );


	get_posKey( t_posMat,NULL );
	get_rotKey( t_rotMat,NULL );
	get_scaleKey( t_scaleMat,NULL );

	// �ϲ㴫������scale,����Զɽ���ر����Զ�ɼ��������ӡ�
	osMatrixScaling( &t_s1,_scale.x,_scale.y,_scale.z );
	t_scaleMat = t_scaleMat*t_s1;
	osMatrixRotationY( &t_tmpRot,_rotAgl );
	t_rotMat = t_rotMat * t_tmpRot;

	t_matWorld = m_matNodetm*t_scaleMat;
	t_matWorld = t_matWorld*t_rotMat;
	t_matWorld = t_matWorld*t_posMat;

	// 
	// ���ת��������ռ䡣
	osMatrixTranslation( &t_posMat,_pos.x,_pos.y,_pos.z );
	t_matWorld = t_matWorld*t_posMat;
	_pipe->set_worldMatrix( t_matWorld );


	// ��䲢��Ⱦ����Ρ�
	m_sPolygon.m_sBufData.m_iIbufId = m_vecIdxBufId[0];
	m_sPolygon.m_iShaderId = m_vecShaderId[0];
	m_sPolygon.m_iPriNum = m_arrMesh[0].m_nFace;


	D3DMATERIAL9    t_material;
	memcpy( &t_material,&m_arrMesh[0].m_deviceMat,sizeof( D3DMATERIAL9 ) );

	t_material.Ambient.a = _alphaV;
	t_material.Diffuse.a = _alphaV;
	_pipe->set_material( t_material );
 
	if( !_pipe->render_polygon( &m_sPolygon ) )
		return false;

	return true;

	unguard;
}



//! ��Ⱦ��ǰ��node,ʹ��aniMeshInstance.
bool os_aniNode::render_aniNode( osc_aniMeshIns* _ami,
	  int _idx,bool _alpha,float _faderate/* = 1.0f*/,bool _waterObj/* = false*/ )
{
	guard;
	
	osassert( _ami );

	osMatrix       t_matWorld,t_posMat,t_scaleMat,t_rotMat;
	osVec3D        t_vec3;

	// �õ����ϲ���������ʹ����������ٳ��Խ��
	// �Լ��ľ��󣬾͵õ�������Ʒ�������е�����λ�á�
	if( m_bUseKeyMat )
	{
		osn_mathFunc::matrix_lerp( 
			t_matWorld,m_arrKeyMatrix[_ami->m_iCurPosFrame],
			m_arrKeyMatrix[_ami->m_iCurPosFrame+1],_ami->m_fPFactor );
	}
	else
	{
		get_posKey( t_posMat,_ami );

		// ATTENTION: �Ƿ�poskeyҲҪ���룿
		// River mod 2007-8-16:���ʹ��morph frame,������ʹ���ϲ��
		// �������󣬷�����ܳ��������Ļ������������ĵط�����������������
		get_rotKey( t_rotMat,_ami );
		get_scaleKey( t_scaleMat,_ami );


		t_matWorld = m_matNodetm*t_scaleMat;
		t_matWorld = t_matWorld*t_rotMat;			

		// sign of determinant
		if( m_arrAffineParts[0].f < 0.0f )
			t_matWorld = t_matWorld*osc_skinMesh::m_sFDMatrix;

		// ���ϲ������ռ�λ�ƺ���һ����.
		t_matWorld = t_matWorld*t_posMat;
	}

	// 
	// ���õ��ϲ�����ռ䡣
	_ami->get_transMatrix( t_scaleMat );

	// 
	// River mod @ 2005-5-31:�����ϲ�����������ת��λ��
	//                       osa����Ч�ļ�. 
	// ���������ռ����ת��λ��.
	t_matWorld = t_matWorld*t_scaleMat;
	_ami->m_ptrMPipe->set_worldMatrix( t_matWorld );

	// River added @ 2008-12-10: �����ˮ����Ʒ������
	if( _waterObj )
		waterObj_rsSet( _ami,t_matWorld );

	// ����б��ζ�����������ζ�������䶥�㻺������
	if( m_iMorphFrames>1 )
	{
		if( !process_morphFrame( _ami ) )
			osassert( false );
	}
		
	//!������Ⱦˮ��ʱ�����ˮ��߶ȴ���һ��ֵ�����ò���ˮ���Ե͸��Ч��
	int TechniqueLevel = 0;

	//
	// ��ÿһ��mesh������Ⱦ��
	for( int t_i=0;t_i<m_iNumOfMesh;t_i ++ )
	{
		// û����Ľ��.
		if( m_arrMesh[t_i].m_nFace <= 0 )
			continue;

		// River @ 2008-12-10:ˮ�治��͸���Ͳ�͸��.
		if( !_waterObj )
		{
			if( _alpha )
			{
				if( !m_arrMesh[t_i].m_bAlphaMat )
					continue;
			}
			else
			{
				if( m_arrMesh[t_i].m_bAlphaMat )
					continue;
			}
		}

		//
		// �����material Animation,�õ���ǰ����material�������á�
		D3DMATERIAL9   t_mat;
		m_arrMesh[t_i].get_currentMaterial( t_mat,_ami );
		t_mat.Ambient.a *= _faderate;
		t_mat.Diffuse.a *= _faderate;
		_ami->m_ptrMPipe->set_material( t_mat );


		// ��䲢��Ⱦ����Ρ�
		m_sPolygon.m_sBufData.m_iIbufId = m_vecIdxBufId[t_i];
		
		// 
		// River @ 2009_6-29:���ǿ��ʹ��Shader,��ʹ���µ�shader.
		if( g_bForceUseCustomShader )
		{
			osassert( g_iForceCustomShaderIdx >= 0 );
			m_sPolygon.m_iShaderId = g_iForceCustomShaderIdx;
		}
		else
		{
			// ˮ�洦��
			m_sPolygon.m_iShaderId = m_vecShaderId[t_i];
			if (_waterObj)
			{
				osc_effect* pEffect = g_shaderMgr->get_effect(m_sPolygon.m_iShaderId);
				osassert(pEffect);
				if (pEffect->m_curTechLevel!=2)
				{
					pEffect->m_curTechLevel = TechniqueLevel;
				}
			}

			/*
			//! River @ 2010-6-29:��Ļ�ⲨЧ���õ�.
			if( _ami->m_bIsScreenWave )
			{
				osc_effect* pEffect = g_shaderMgr->get_effect(m_sPolygon.m_iShaderId);
				osassert(pEffect);

				osMatrixMultiply( &t_matWorld,&t_matWorld,&g_matView );
				osMatrixMultiply( &t_matWorld,&t_matWorld,&g_matProj );
				if( FAILED( pEffect->m_peffect->SetMatrix( "Matrix0",&t_matWorld ) ) )
					osassert( false );
				pEffect->m_peffect->SetFloat( "t_fMatAlpha",_faderate );
			}
			*/
		}

		m_sPolygon.m_iPriNum = m_arrMesh[t_i].m_nFace;

		if( m_iMorphFrames>1 )
			m_sPolygon.m_sBufData.m_iVertexStart = m_iStartVer;

		if( !_ami->m_ptrMPipe->render_polygon( &m_sPolygon ) )
			return false;

	}

	//
	// ���㵱ǰaniNode��bounding Sphere Center������ռ��е����ꡣ
	os_bsphere* t_bs = &_ami->m_vecBs[_idx];
	t_bs->radius = m_bsphere.radius;
	osVec3Transform( &t_bs->veccen,&m_bsphere.veccen,&t_matWorld );

	return true;

	unguard;
}
bool os_aniNode::render_FogAniNode( osc_aniMeshIns* _ami,int _idx)
{
	guard;

	osassert( _ami );



	osMatrix       t_matWorld,t_posMat,t_scaleMat,t_rotMat;
	osVec3D        t_vec3;

	// �õ����ϲ���������ʹ����������ٳ��Խ��
	// �Լ��ľ��󣬾͵õ�������Ʒ�������е�����λ�á�
	if( m_bUseKeyMat )
	{
		osn_mathFunc::matrix_lerp( 
			t_matWorld,m_arrKeyMatrix[_ami->m_iCurPosFrame],
			m_arrKeyMatrix[_ami->m_iCurPosFrame+1],_ami->m_fPFactor );
	}
	else
	{
		get_posKey( t_posMat,_ami );

		// ATTENTION: �Ƿ�poskeyҲҪ���룿
		// River mod 2007-8-16:���ʹ��morph frame,������ʹ���ϲ��
		// �������󣬷�����ܳ��������Ļ������������ĵط�����������������
		get_rotKey( t_rotMat,_ami );
		get_scaleKey( t_scaleMat,_ami );


		t_matWorld = m_matNodetm*t_scaleMat;
		t_matWorld = t_matWorld*t_rotMat;			

		// sign of determinant
		if( m_arrAffineParts[0].f < 0.0f )
			t_matWorld = t_matWorld*osc_skinMesh::m_sFDMatrix;

		// ���ϲ������ռ�λ�ƺ���һ����.
		t_matWorld = t_matWorld*t_posMat;
	}

	// 
	// ���õ��ϲ�����ռ䡣
	_ami->get_transMatrix( t_scaleMat );

	// 
	// River mod @ 2005-5-31:�����ϲ�����������ת��λ��
	//                       osa����Ч�ļ�. 
	// ���������ռ����ת��λ��.
	t_matWorld = t_matWorld*t_scaleMat;
	//_ami->m_ptrMPipe->set_worldMatrix( t_matWorld );

	// River added @ 2008-12-10: �����ˮ����Ʒ������
	//if( _waterObj )
	//	waterObj_rsSet( _ami,t_matWorld );

	
	g_ptrCamera->get_viewmat(g_matView);

	osMatrix t_sMat;
	osMatrixMultiply( &t_sMat,&t_matWorld,&g_matView );
	osMatrixMultiply( &t_sMat,&t_sMat, &g_matProj );

	g_shaderMgr->get_effect(_ami->m_FogShaderId)->m_peffect->SetMatrix("Matrix1",&t_sMat);


	// ����б��ζ�����������ζ�������䶥�㻺������
	if( m_iMorphFrames>1 )
	{
		if( !process_morphFrame( _ami ) )
			osassert( false );
	}

	//
	// ��ÿһ��mesh������Ⱦ��
	for( int t_i=0;t_i<m_iNumOfMesh;t_i ++ )
	{
		// û����Ľ��.
		if( m_arrMesh[t_i].m_nFace <= 0 )
			continue;

		// River @ 2008-12-10:ˮ�治��͸���Ͳ�͸��.
		


		// ��䲢��Ⱦ����Ρ�
		m_sPolygon.m_sBufData.m_iIbufId = m_vecIdxBufId[t_i];
		m_sPolygon.m_iShaderId = _ami->m_FogShaderId;
		m_sPolygon.m_iPriNum = m_arrMesh[t_i].m_nFace;

		if( m_iMorphFrames>1 )
			m_sPolygon.m_sBufData.m_iVertexStart = m_iStartVer;

		if( !_ami->m_ptrMPipe->render_polygon( &m_sPolygon ) )
			return false;

	}

	//
	// ���㵱ǰaniNode��bounding Sphere Center������ռ��е����ꡣ
	os_bsphere* t_bs = &_ami->m_vecBs[_idx];
	t_bs->radius = m_bsphere.radius;
	osVec3Transform( &t_bs->veccen,&m_bsphere.veccen,&t_matWorld );

	return true;

	unguard;
}


void		os_aniNode::DumpRenderInfo(osc_middlePipe* _pipe)
{
	
	osDebugOut("\nskyterr:vecIdxBufId:%d\n",m_vecIdxBufId[0]);
	osDebugOut("skyterr:iVerBufId:%d\n",m_iVerBufId);
	
	osDebugOut("skyterr:m_iShaderId:%d\n",m_vecShaderId[0]);
	osDebugOut("skyterr:m_iIdxNum:%d\n",m_arrMesh[0].m_nFace*3);

}

//! �ͷŵ�ǰ�Ľ��.
void os_rootNode::release_node( void )
{
	SAFE_DELETE_ARRAY( m_arrNodelist );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// osc_aniMesh

osc_aniMesh::osc_aniMesh()
{
	m_bAniMat = false;

	m_bInuse = false;

	m_btRStyle = 0;

	m_bNotInterpolate = true;
	m_bColorVertex = false;

	m_ptrMaxMorphNode = NULL;
	m_ptrMaxAffineNode = NULL;

	m_bCanSwap = true;

	m_bEffByAmbient = true;
	this->m_dwNameHash = 0;

	m_bRepeatTexture = FALSE;

}


/** \brief
*  �ͷŵ�ǰ��Դ�Ĵ��麯������release_ref�������á�
*  
*/
void osc_aniMesh::release_obj( DWORD _ptr/* = NULL*/ )
{
	guard;

	osassert( _ptr );

	//osDebugOut( "The <%s> aniMesh Release from Device..\n",m_szFname.c_str() );

	m_sRootNode.release_devdepData( (osc_middlePipe*)_ptr );
	m_sRootNode.release_node();

	// ����ʹ��״̬��
	m_bInuse = false;
	m_szFname = "";


	unguard;
}


//! �õ���aniMesh��Ӧ�ĵ�һ��effect�������ϲ�õ�ˮ���effect.
osc_effect* osc_aniMesh::get_effect( void )
{
	guard;

	osc_effect*  t_ptrEffect = 
		g_shaderMgr->get_effect( m_sRootNode.m_arrNodelist[0].m_vecShaderId[0] );
	osassert( t_ptrEffect );
	return t_ptrEffect;

	unguard;
}


// TEST CODE:
//! ȷ��aniMesh��Ӧ��shaderid��Ч��
bool osc_aniMesh::assert_shader( bool _assert/* = true*/ )
{
	guard;

	for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i ++ )
	{
		if( !m_sRootNode.m_arrNodelist[t_i].assert_devdepData( _assert ) )
			return false;
	}

	return true;

	unguard;
}


//! ����mesh��aabbox.
void osc_aniMesh::cal_aabbox( void )
{
	guard;


	m_sAabbox.m_vecMax = m_sRootNode.m_arrNodelist[0].m_bbox.m_vecMax;
	m_sAabbox.m_vecMin = m_sRootNode.m_arrNodelist[0].m_bbox.m_vecMin;

	for( int t_i=1;t_i<this->m_sRootNode.m_iChildNum;t_i ++ )
	{
		m_sAabbox.extend( m_sRootNode.m_arrNodelist[t_i].m_bbox.m_vecMax );
		m_sAabbox.extend( m_sRootNode.m_arrNodelist[t_i].m_bbox.m_vecMin );
	}

	return;

	unguard;
}



//! ���ļ��е������ǵ�mesh���ݡ�
bool osc_aniMesh::load_meshFromfile( const char *_fname )
{
	guard;

	int     t_iSize;
	DWORD   t_dwFileVersion;
	char    t_szFileHeader[4];
	BYTE*   t_ptrStart;


	osassert( _fname&&(_fname[0]) );

	m_szFname = _fname ;
	m_dwNameHash = string_hash( _fname );

	int   t_iGBufIdx = -1;
	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( (char*)_fname,t_ptrStart,TMP_BUFSIZE );
	osassert( t_iSize > 0 );

	// 
	// �ȶ����ļ�ͷ���ļ��汾�š�
	READ_MEM_OFF( &t_dwFileVersion,t_ptrStart,sizeof( DWORD ) );
	if( t_dwFileVersion < ANIMESH_FILEVERSION )
	{
		END_USEGBUF( t_iGBufIdx );
		osassert( false );
		return false;
	}
	READ_MEM_OFF( t_szFileHeader,t_ptrStart,sizeof( char )*4 );
	if( strcmp( ANIMESH_MAGIC,t_szFileHeader ) )
	{
		END_USEGBUF( t_iGBufIdx );
		osassert( false );
		return false;
	}
	// �����Ƿ�ʹ�ò��ʶ�����
	READ_MEM_OFF( &this->m_bAniMat,t_ptrStart,sizeof( bool ) );

	//
	//@{
	// River @ 2004-7-21: ����ǰ汾0x103�������£����ܻ�ʹ�ö�����ɫ��
	if( t_dwFileVersion >= ANIMESH_FILEV103 )
		READ_MEM_OFF( &m_bColorVertex,t_ptrStart,sizeof( bool ) );
	//@} 



	// ���101�������ϵİ汾��������֮֡���Ƿ��ֵ��
	if( t_dwFileVersion >= ANIMESH_FILEV101 )
		READ_MEM_OFF( &m_bNotInterpolate,t_ptrStart,sizeof( bool ) );

	// �����102�������ϵİ汾�������Ƿ�ͱ�����ӵ���Ⱦѡ�
	if( t_dwFileVersion >= ANIMESH_FILEV102 )
	{
		READ_MEM_OFF( &m_btRStyle,t_ptrStart,sizeof( BYTE ) );
		osassert( m_btRStyle<6 );
	}

	// River @ 2005-3-10: ����osaMesh�Ƿ��ܻ�����Ӱ���ѡ��.
	if( t_dwFileVersion >= ANIMESH_FILEV104 )
		READ_MEM_OFF( &m_bEffByAmbient,t_ptrStart,sizeof( bool ) );

	if( t_dwFileVersion >= ANIMESH_FILEV105 )
		READ_MEM_OFF( &m_bRepeatTexture,t_ptrStart,sizeof( BOOL ) );

	if( t_dwFileVersion >= ANIMESH_FILEV106 )
		READ_MEM_OFF( &m_bKeyMatrix,t_ptrStart,sizeof( BOOL ) );
	
	// ���뼸�ν����Ŀ�����Ҷ������еļ��ν�㡣
	READ_MEM_OFF( &m_sRootNode.m_iChildNum,t_ptrStart,sizeof( int ) );
	m_sRootNode.m_arrNodelist = new os_aniNode[m_sRootNode.m_iChildNum];

	for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i++ )
	{
		// ����ʱ��Ҫ��������ݣ������ڽṹ����ǰ��ֵ��
		if( t_dwFileVersion >= ANIMESH_FILEV103 )
			m_sRootNode.m_arrNodelist[t_i].m_bColorVertex = m_bColorVertex;

		// 2006-7-22:
		if( t_dwFileVersion >= ANIMESH_FILEV106 )
			m_sRootNode.m_arrNodelist[t_i].m_bUseKeyMat = m_bKeyMatrix;

		//
		// ���������ݡ�
		m_sRootNode.m_arrNodelist[t_i].load_aniNodeFromMem( t_ptrStart,m_bAniMat );

		// ���101�������ϵİ汾����ÿһ��node��ֵ�Ƿ���������ֵ��
		if( t_dwFileVersion >= ANIMESH_FILEV101 )
			m_sRootNode.m_arrNodelist[t_i].m_bNotInterpolate = m_bNotInterpolate;

	}


	// 
	// �õ�ǰ����������morphFrame��affineParts����.
	m_iMaxMorph = 0;
	m_iMaxAffine = 0;
	for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i ++ )
	{
		if( m_sRootNode.m_arrNodelist[t_i].m_iMorphFrames > m_iMaxMorph )
		{
			m_iMaxMorph = m_sRootNode.m_arrNodelist[t_i].m_iMorphFrames;
			m_ptrMaxMorphNode = &m_sRootNode.m_arrNodelist[t_i];
		}

		if( m_sRootNode.m_arrNodelist[t_i].m_iAffinePartsNum>m_iMaxAffine )
		{
			m_iMaxAffine = m_sRootNode.m_arrNodelist[t_i].m_iAffinePartsNum;
			m_ptrMaxAffineNode = &m_sRootNode.m_arrNodelist[t_i];
		}
	}

	osassert( m_sRootNode.m_iChildNum > 0 );
	osassert( m_sRootNode.m_arrNodelist != NULL );
	
	END_USEGBUF( t_iGBufIdx );

	// 
	cal_aabbox();

	return true;

	unguard;
}



//! ������ǰmesh���豸������ݡ�
bool osc_aniMesh::create_devdepRes( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	m_sRootNode.create_devdepData( _pipe );

	return true;

	unguard;
}

/** \brief
*  ���ļ��д������ǵ�mesh���ݡ�
*  
*  �������ݵ�ʱ�򣬴����豸��ص����ݡ�
*/
bool osc_aniMesh::create_meshFromfile( os_aniMeshCreate& _mc )
{
	guard;

	//
	// �ȴ��ڴ��е������ǵ�aniMesh���ݡ�
	if( !load_meshFromfile( _mc.m_szAMName ) )
		return false;

	osassert( m_sRootNode.m_arrNodelist );

	//
	// ������ǰmesh���豸������ݡ�
	if( !create_devdepRes( osc_aniMeshIns::m_ptrMPipe  ) )
		return false;

	// 
	// ATTENTION TO OPP: ����Ԥ����õ�һ��AniMesh�����岥��ʱ�䡣
	m_fPlayTotalTime = 0.0f;
	osassert( m_sRootNode.m_iChildNum > 0 );
	for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i ++ )
	{
		if( m_fPlayTotalTime < m_sRootNode.m_arrNodelist[t_i].m_fPlayTotalTime )
			m_fPlayTotalTime = m_sRootNode.m_arrNodelist[t_i].m_fPlayTotalTime;
	}


	m_bInuse = true;

	return true;

	unguard;
}



//! ��Ⱦ��ǰ��aniMesh.
void osc_aniMesh::render_aniMesh( osc_aniMeshIns* _ami,
	  bool _alpha,float _fadeRate/* = 1.0f*/,bool _waterObj/* = false*/,bool _fogObj /*= false*/ )
{
	guard;

	osassert( _ami );

	if (_fogObj)
	{
		for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i++ )
		{
			if( !m_sRootNode.m_arrNodelist[t_i].
				render_FogAniNode( _ami,t_i ) )
				osassert( false );
		}
	}
	else
	{
		//! windy mod ��ӵ�ǰ�棬��ǰ�ں�������,�����Ƿ��ģ�
		if( m_bRepeatTexture )
			_ami->m_ptrMPipe->set_ssClamp();
		else
			_ami->m_ptrMPipe->set_ssRepeat();


		// TEST CODE:
		osassertex( assert_shader( false ),(char*)m_szFname.c_str() );

		for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i++ )
		{
			// 
			// ��Ⱦ��͸�����֣��������û�в�͸����Face.
			if( (!_alpha)&&m_sRootNode.m_arrNodelist[t_i].m_bTotalAlphaFace )			
				continue;

			// 
			// ��Ⱦ͸�����֣��������û��͸����Face.
			if( _alpha  && (!_waterObj) && 
				(!m_sRootNode.m_arrNodelist[t_i].m_bExistAlphaFace) )
				continue;

			//
			// ��Ⱦÿһ����㡣
			if( !m_sRootNode.m_arrNodelist[t_i].
				render_aniNode( _ami,t_i,_alpha,_fadeRate,_waterObj ) )
				osassert( false );

		} 
	}
	
	

	unguard;
}

//! ��ȾaniMesh�ĵ�һ֡��ָ����λ�á�
void osc_aniMesh::render_aniMesh( osVec3D& _pos,float _rotAgl,osVec3D& _scale,
								 osc_middlePipe* _pipe,float _alphaV/* = 0.0f*/ )
{
	guard;

	osassert( _pipe );


	for( int t_i=0;t_i<m_sRootNode.m_iChildNum;t_i++ )
	{
		if( !m_sRootNode.m_arrNodelist[t_i].
			render_aniNode( _pipe,_rotAgl,_pos,_scale,_alphaV ) )
			osassert( false );
		
	} 

	return;

	unguard;
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// osc_aniMeshIns
osc_aniMeshIns::osc_aniMeshIns()
{	
	m_ptrMesh = NULL;
	reset_meshIns();
}

osc_aniMeshIns::~osc_aniMeshIns()
{
}

//! �������ǵ�meshIns
void osc_aniMeshIns::reset_meshIns( void )
{

	if( m_ptrMesh )
	{
		osassert( m_ptrMPipe );

		// ATTENTION TO FIX:�ݲ��ͷ�;
		m_ptrMesh->release_ref( DWORD(m_ptrMPipe) );

		m_ptrMesh = NULL;
	}

	m_fEleTime = 0.0f;
	
	m_iCurMorphFrame = 0;
	m_fMTime = 0.0f;

	m_iCurPosFrame = 0;
	m_fPTime = 0.0f;
	m_fPFactor = 0.0f;
	
	m_iCurMatFrame = 0;
	m_fMatTime = 0.0f;

	m_fTimeScale = 1.0f;

	m_dwAmbientL = 0xffffffff;

	m_bEffectByAL = true;

	m_bInuse = false;	

	m_bHide = false;

	m_bTopLayer = false;

	m_bWaterAni = false;

	m_bFogAni   = false;


	//! ��ʼ�����������Ŷ���״̬
	m_bInScaleState = false;

	//! 
	m_bRenderedInCurFrame = true;
	


	// ���ڵ�һ�ζ���ʱ�����㶯���ľ���
	m_matTrans._41 = -1.0f;
	// River mod @ 2008-7-25:
	m_iFadeState = 0;
	m_fCurFadeRate = 1.0f;

	m_bIsScreenWave = false;
	


	reset_ins();

}


//! ����osa�����Ĳ���ʱ��.
void osc_aniMeshIns::set_osaPlayTime( float _timeScale )
{
	guard;

	osassert( _timeScale > 0.0f );

	m_fTimeScale = _timeScale;

	unguard;
}

//! �õ�osa��������������ʱ�䡣
float osc_aniMeshIns::get_osaPlayTime( void )
{
	guard;

	return m_ptrMesh->get_playTotalTime();

	unguard;
}

//! �м�ܵ���ָ�롣
osc_middlePipe*  osc_aniMeshIns::m_ptrMPipe = NULL;


/** \brief
*  ���ļ��д���osc_aniMesh����ʵ��.
*
*  ��Ҫ�����Ĳ���������λ�ã���ת���򣬲������͵���Ϣ��
*
*/
WORD osc_aniMeshIns::create_aniMeshIns( osc_aniMesh* _mptr,
			os_aniMeshCreate& _mc,bool _topLayer/* = false*/ )
{
	guard;

	osassert( _mptr );
	osassert( m_ptrMPipe );
	osassertex( _mc.m_fSizeScale > 0.0f,
		va( "Size scale is:<%f>...\n",_mc.m_fSizeScale) );

	reset_meshIns();

	// ���õ�ǰinsΪ���á�
	m_bInuse = true;

	m_vecBs.resize( _mptr->get_rootNodePtr()->m_iChildNum );

	m_bTopLayer = _topLayer;

# if __EFFECT_EDITOR__
	m_bStopPlay = false;
# endif 


	// 
	// �����set_animationMesh�����Ĺ�����
	m_ptrMesh = _mptr;
	m_vec3Pos = _mc.m_vec3Pos;
	m_fRotAgl = _mc.m_fAngle;
	m_ePlayType = _mc.m_ePlayType;
	m_fPlayTime = _mc.m_fPlayTime;
	m_iPlayFrames = _mc.m_iPlayFrames;
	m_bAutoDelete = _mc.m_bAutoDelete;
	m_fSizeScale = _mc.m_fSizeScale;
	m_bWaterAni = _mc.m_bWaterAni;
	m_bFogAni = _mc.m_bFogAni;
	
	// ������λ���ʱ�䲥�Ŷ�����
	m_fEleTime = 0.0f;
	m_iLoopTimes = 0;


	// 
	// ֻҪ�����������ģ���ڲ�������һ����ʾ��ģ�Ͳ��ܻ�����Ӱ�죬
	// ����Ʒ�����ϲ��ܻ������Ӱ�졣
	//m_bEffectByAL = _mc.m_bEffectByAmbiL;
	if( (!m_ptrMesh->m_bEffByAmbient) || 
		(!_mc.m_bEffectByAmbiL) )
		m_bEffectByAL = false;
	else
		m_bEffectByAL = true;


	// ����ǰ��meshIns���õ�meshInsÿһ������Bounding Sphere.
	osMatrix    t_mat;
	osMatrixTranslation( &t_mat,m_vec3Pos.x,m_vec3Pos.y,m_vec3Pos.z );
	int t_i;
	for( t_i=0;t_i<_mptr->get_rootNodePtr()->m_iChildNum;t_i ++)
	{
		os_aniNode* t_ptrNode = &_mptr->get_rootNodePtr()->m_arrNodelist[t_i];
		osassertex( t_ptrNode,va( "The <%s>'s file error!!!",_mc.m_szAMName ) );

		t_ptrNode->get_bsphere( m_vecBs[t_i] );		
		osVec3Transform( &m_vecBs[t_i].veccen,&m_vecBs[t_i].veccen,&t_mat );
	}
	if( t_i > 1 )
		osn_mathFunc::get_bsFromBsArray( m_bsphere,&m_vecBs[0],int(m_vecBs.size()) );	
	else
		m_bsphere = m_vecBs[0];


	// River added @ 2008-12-30: Osa��Ʒ�����ţ�ͬʱ���Ŵ���Ʒ��Radius.
	// ������ɶ����Ⱦ��Ʒ?????�˴��Ż�
	m_bsphere.radius *= _mc.m_fSizeScale;

	m_fBaseScale = _mc.m_fSizeScale;


	m_ptrMesh->add_ref();

	increase_val();
	set_insInit();

	return get_val();
	
	unguard;
}



// 
//! ����posKey���ݡ�
void osc_aniMeshIns::process_posKey( os_aniNode* _ptrNode,float _etime )
{
	float     t_fStime,t_fTime;

	//River @ 2011-10-14:���ʱ�䲻�ԣ�ֱ�ӷ��أ�������.
	if( m_fPTime < -0.01f ) 
		return;

	osassert( m_fPTime > -0.1f );
	
	//River @ 2011-10-14:���ʱ�䲻�ԣ�ֱ�ӷ��أ�������.
	if( _etime < -0.01f ) 
		return;

	osassert( _etime > -0.1f );

	m_fPTime += _etime;

	m_bOverLoop = false;

	while( 1 )
	{
		osassert( m_fPTime > -0.1f );
		int t_i;
		for( t_i=m_iCurPosFrame+1;t_i< (_ptrNode->m_iAffinePartsNum );t_i ++ )
		{
			t_fTime = _ptrNode->m_arrFAffTimeKey[t_i];

			// �ҵ���ǰ֡�ڶ���ʱ��������ʱ��λ�á�
			m_iCurPosFrame = t_i-1;
			if( m_fPTime < t_fTime )
				break;
		}

		//
		// ����������һ֡����ѭ�����š�
		if( (t_i == (_ptrNode->m_iAffinePartsNum))&&
			( m_fPTime >= t_fTime ) ) 
		{
			osassert( (m_fPTime - _ptrNode->m_arrFAffTimeKey[t_i-1]) >= -0.1f );

			m_iCurPosFrame = 0;
			m_bOverLoop = true;

			m_fPTime = float_mod( m_fPTime,t_fTime );

			continue;
		}


		//
		// �õ�֡��Ĳ�ֵfactor.
		t_fStime = _ptrNode->m_arrFAffTimeKey[m_iCurPosFrame];

		if( float_equal( t_fTime,t_fStime ) )
			m_fPFactor = 0.0f;
		else
			m_fPFactor = (m_fPTime - t_fStime)/(t_fTime - t_fStime);

		// ʹֵ����0-1֮��.
		m_fPFactor -= int(m_fPFactor);

		break;

	}


	return;
}




/** \brief
 *  ����morphFrame.
 *
 *  ģ�͵�morph�п��ܻ����Źؼ�֡������
 *  
 */
void osc_aniMeshIns::process_morphFrame( os_aniNode* _ptrNode,float _etime )
{
	guard;

	float     t_fStime,t_fTime;

	osassert( m_fMTime > -0.1f );
	osassert( _etime > -0.1f );
	osassert( _ptrNode );

	// 
	// ����йؼ�֡��������͹ؼ�֡����һ�¡�
	if( _ptrNode->m_iAffinePartsNum > 1 )
	{
		m_iCurMorphFrame = m_iCurPosFrame;
		m_fMFactor = m_fPFactor;
		return;
	}

	m_fMTime += _etime;
	m_bOverLoop = false;
	while( 1 )
	{
		osassert( m_fMTime > -0.1f );
		int t_i;
		for( t_i=m_iCurMorphFrame+1;t_i< (_ptrNode->m_iMorphFrames );t_i ++ )
		{
			t_fTime = _ptrNode->m_fTimeKey[t_i];

			// �ҵ���ǰ֡�ڶ���ʱ��������ʱ��λ�á�
			m_iCurMorphFrame = t_i-1;
			if( m_fMTime < t_fTime )
				break;
		}

		//
		// ����������һ֡����ѭ�����š�
		if( t_i == (_ptrNode->m_iMorphFrames)&&
			( m_fMTime >= t_fTime )   )
		{
			osassert( (m_fMTime - _ptrNode->m_fTimeKey[t_i-1]) >= -0.1f );

			m_bOverLoop = true;
			m_iCurMorphFrame = 0;

			m_fMTime = float_mod( m_fMTime,t_fTime );

			continue;
		}

		//
		// �õ�֡��Ĳ�ֵfactor.
		t_fStime = _ptrNode->m_fTimeKey[m_iCurMorphFrame];


		if( float_equal( t_fTime,t_fStime ) )
			m_fMFactor = 0.0f;
		else
			m_fMFactor = (m_fMTime - t_fStime)/(t_fTime - t_fStime);

		break;

	}


	return;

	unguard;
}

/** \brief
 *  ����matFrame
 *
 *  �����еĲ��ʶ�����Ҫ��������͸���Ľ���Ч�������纣�ˣ�
 *  ħ����Ч����
 *  
 */
void osc_aniMeshIns::process_aniMatFrame( os_aniNode* _ptrNode,float _etime )
{
	guard;

	float         t_fStime,t_fTime;
	int           t_iMatFrameNum;
	float*        t_arrMatTime;

	osassert( m_fMTime > -0.1f );
	osassert( _etime > -0.1f );

	// 
	// ����йؼ�֡��������͹ؼ�֡����һ�¡�
	if( _ptrNode->m_iAffinePartsNum > 1 )
	{
		m_iCurMatFrame = m_iCurPosFrame;
		m_fMatFactor = m_fPFactor;
		return;
	}


	t_iMatFrameNum = _ptrNode->m_arrMesh[0].m_iMaterialNum;
	t_arrMatTime = _ptrNode->m_arrMesh[0].m_ptrfTimekey;

	m_fMatTime += _etime;
	m_bOverLoop = false;
	while( 1 )
	{
		osassert( m_fMatTime > -0.1f );
		int t_i;
		for( t_i=m_iCurMatFrame+1;t_i<t_iMatFrameNum;t_i ++ )
		{
			t_fTime = t_arrMatTime[t_i];

			// �ҵ���ǰ֡�ڶ���ʱ��������ʱ��λ�á�
			m_iCurMatFrame = t_i-1;
			if( m_fMatTime < t_fTime )
				break;
		}

		//
		// ����������һ֡�����ҵ��ﵽ���һ֡�����ʱ��,��ѭ�����š�
		if( t_i == (t_iMatFrameNum)&&(m_fMatTime >= t_fTime) )
		{
			osassert( (m_fMatTime - t_arrMatTime[t_i-1]) >= -0.1f );

			m_bOverLoop = true;
			m_iCurMatFrame = 0;
			
			m_fMatTime = float_mod( m_fMatTime,t_fTime );

			continue;
		}


		//
		// �õ�֡��Ĳ�ֵfactor.
		t_fStime = t_arrMatTime[m_iCurMatFrame];

		if( float_equal( t_fTime,t_fStime ) )
			m_fMatFactor = 0.0f;
		else
			m_fMatFactor = (m_fMatTime - t_fStime)/(t_fTime - t_fStime);

		break;

	}
	return;

	unguard;
}




//! ������Ч�Ķ�����Ϣ
bool osc_aniMeshIns::update_osaAnimation( float _etime )
{
	guard;

	//@{�༭������OSAģ�Ͳ����ٶ�
# if __EFFECT_EDITOR__
	extern float gOSAPLAYERSpeed;
	_etime*=gOSAPLAYERSpeed;
# endif 
	//@}
	os_aniNode*  t_ptrNode;

	float        t_fTimeAfterScale;

	osassert( m_ptrMesh );
	// River Added @ 2010-5-7:
	//osassert( m_ptrMesh->m_bInuse );
	// River Added @ 2010-8-9:ȷ�ϻ������������.
	//@{
	if( !m_ptrMesh->m_bInuse )
	{
		
		delete_curIns();
		return false;
	}
	//@} 
	
	t_fTimeAfterScale = _etime * m_fTimeScale;

# if __EFFECT_EDITOR__
	if( m_bStopPlay )
		t_fTimeAfterScale = 0;
# endif 

	//  
	//! river added @ 2009-6-17:��Ե���m_ptrMeshΪ�յĴ�����˴���ȷ��������
	if( !m_ptrMesh )
		return false;


	// ��ʱ�䲥����ɡ�
	if( finish_playWithTime( t_fTimeAfterScale ) )
		return false;

	// ����������һ��os_aniNode.
	t_ptrNode = &m_ptrMesh->get_rootNodePtr()->m_arrNodelist[0];
	// 
	// ����õ���ǰposKey�����ݡ�
	if( m_ptrMesh->m_iMaxAffine > 1 )
		process_posKey( m_ptrMesh->m_ptrMaxAffineNode,t_fTimeAfterScale );

	// morph frame.
	if( m_ptrMesh->m_iMaxMorph > 1 )
		process_morphFrame( m_ptrMesh->m_ptrMaxMorphNode,t_fTimeAfterScale );

	// 
	// animation material.      
	if( m_ptrMesh->is_aniMeshAniMat() )
		process_aniMatFrame( t_ptrNode,t_fTimeAfterScale );

	//
	// ���������ŵĻ����������ŵ�ѭ�������������š�
	if( finish_playWithFrame() )
		return false;

	return true;

	unguard;
}

//! frameMove��ǰosa��fade״̬.
void osc_aniMeshIns::fade_frameMove( void )
{
	guard;

	if( m_iFadeState == 0 )
		return;

	//osassert( m_iFadeState == 2);
	m_fCurEleFadeTime += sg_timer::Instance()->get_lastelatime();

	if( m_fCurEleFadeTime > m_fFadeTime )
	{
		m_fCurFadeRate = m_fMinFade;
		m_iFadeState = 0;
		return;
	}

	m_fCurFadeRate = 1.0f - m_fCurEleFadeTime/m_fFadeTime;
	if( m_fCurFadeRate < m_fMinFade )
		m_fCurFadeRate = m_fMinFade; 

	return;

	unguard;
}


//! framemove��ǰ������״̬
void osc_aniMeshIns::scale_frameMove( void )
{
	guard;

	if( !m_bInScaleState )
		return ;

	float t_f = sg_timer::Instance()->get_lastelatime();
	m_fScaleEleTime += t_f;

	if( m_fScaleEleTime >= this->m_fScaleTime )
	{
		m_fSizeScale = m_fBaseScale * this->m_fNewScale;
		this->m_bInScaleState = false;
		return;
	}
	else
	{
		float t_fScale = this->m_fScaleEleTime / this->m_fScaleTime;
		m_fSizeScale  = m_fBaseScale * ((m_fNewScale - 1.0f)*t_fScale + 1.0f);
	}

	return;


	unguard;
}


/** \brief
*   ��Ⱦ��ǰ�Ķ�̬meshʵ����
*
*   ���ݵ�ǰʱ�䣬ȷ��aniMesh�Ķ����õ�����һ֡��������Ҫ��֮֡����д���
*/
bool osc_aniMeshIns::render_aniMeshIns( float _etime,bool _alpha,bool _depth/*=false */)
{
	guard;


	// ��������󣬴�meshIns��Ϊ�����ã��˴���顣
	if( (!m_bInuse)||(!get_insInit()) )
		return true;

	osassertex( (_etime>=0.0f),va("The Etime is:<%f>..\n",_etime ) );

	osassert( _etime>= 0 );
	osassert( m_ptrMesh );

	// ��Ⱦ��͸�����ֵ�ʱ�򣬴���MeshIns�еĶ���������ݡ�
	// ���������������Ž�������Ϣʱ��ֱ�ӷ���
	if( !_alpha )
	{
		// River mod @ 2008-7-16:��������
		scale_frameMove();

		//! River mod @ 2008-7-25:����fadeOut������.
		fade_frameMove();

		if( !update_osaAnimation( _etime ) )
			return true;
	}

 	if (!_depth)
 	{
 		// ������ܻ�����Ӱ�죬�����ô���ɫ����ɫ��Ϊ������
  		if( !m_bEffectByAL )
  			m_ptrMPipe->set_renderState( D3DRS_AMBIENT,0xffffffff );	
  		else
		{
			m_ptrMPipe->set_renderState( 
				D3DRS_AMBIENT,osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
		}
 	}

	// 
	// ��Ⱦ��ǰ��mesh.
	//TODO FIXED.... 
	m_ptrMesh->render_aniMesh( this,_alpha,m_fCurFadeRate,m_bWaterAni,_depth);

	//
	// ����ǰ����bs.
	if( m_ptrMesh->get_rootNodePtr()->m_iChildNum > 1 )
		osn_mathFunc::get_bsFromBsArray( m_bsphere,&m_vecBs[0],int(m_vecBs.size()) );	
	else
		m_bsphere = m_vecBs[0];

	// River added @ 2008-12-30:����bs��radius scale,�޸Ĵ˴���Bug.
	m_bsphere.radius *= m_fSizeScale;

	return true;

	unguard;
}


//! �õ�os_bbox
void osc_aniMeshIns::get_bbox( os_bbox* _box )
{
	osassert( _box );
	
	_box->set_bbMaxVec( m_ptrMesh->get_aabboxPtr()->m_vecMax );
	_box->set_bbMinVec( m_ptrMesh->get_aabboxPtr()->m_vecMin );
	_box->set_bbPos( m_vec3Pos );
	_box->set_bbYRot( m_fRotAgl );

}

