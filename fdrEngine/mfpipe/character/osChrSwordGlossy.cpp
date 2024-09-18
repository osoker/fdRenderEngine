///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osChrSwordGlossy.cpp
 * 
 *  His:      River created @ 2004-8-13
 *
 *  Desc:     ���������е�����ص����ݡ�
 * 
 *  "���б�ĩ������ʼ�գ�֪���Ⱥ��������" 
 *
 *  ATTENTION TO FIX:
 *  Ŀǰ�����������������֡�ٶȷǳ���,�����ֱȽϴ�������,��֡��ĵ����ֵ����
 *  ���������,���Ҫ����������,������ֹ����������ǰ�����ؼ�֡���ڵ�λ��,���
 *  ��ʱ���޹�,��������������ܶ�.
 *  
 *  �������������ֵ��ʱ���м������˹ؼ�֡�����������Ҫ����Ĵ���
 *  ��Ҫ���������Ⱦ�����м���ǳ��ĸ��ӵ���Ⱦ����......
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/middlePipe.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"
# include "../../effect/include/fdBillboard.h"

/*
��ʹ�õ����mesh��������Ĵ���.

�����ǰ֡��λ��A����һ֡��λ��B֮������˶����Ĺؼ�֡,�ѹؼ���λ�ò��뵽�������Ⱦ��.

���Ҫ�ڵ����в���ؼ�֡��λ��,��εĲ���,�ؼ�֡λ�õļ���

�����ѵ����frameMove����Ⱦ�Ĺ��ֿܷ�����.
*/

# define DEFAULT_GLOSSYTEX  "glossy"


osc_swordGlossy::osc_swordGlossy()
{
	m_iActNum = 0;


# if __GLOSSY_EDITOR__	
	// Ĭ�����еĵ�����ɫΪ��ɫ��
	for( int t_i=0;t_i<MAX_ACTGLOSSY;t_i ++ )
		m_vecGlossyColor[t_i] = 0x00000000;
# endif 



	m_fEffectPos = 0.5f;

	m_iMatrixIdx = -1;

	// River mod : 
	m_iGlossyMoveLength = MAX_SWORDPOS;// (MAX_SWORDPOS + MIN_SWORDPOS)/2;

	// Ĭ�ϵĵ����������ݡ�
	strcpy( m_szGlossyTex,DEFAULT_GLOSSYTEX );

	m_fOsaRotYAngle = 0.0f;

	m_fScaleVal[0] = 1.0f;
	m_fScaleVal[1] = 1.0f;
	m_fScaleVal[2] = 1.0f;

	m_fAddEffectPos[0] = 0.5f;
	m_fAddEffectPos[1] = 0.5f;

}



# if  __GLOSSY_EDITOR__



//! �ѵ����ص������ĵ��x����ת�����ڵ������⡣
void osc_swordGlossy::rot_swordGlossyX( float _radian )
{
	guard;

	osMatrix    t_mat;

	osMatrixTranslation( &t_mat,-m_vec3Center.x,
		-m_vec3Center.y,-m_vec3Center.z );
	osVec3Transform( &m_vec3Start,&m_vec3Start,&t_mat );
	osVec3Transform( &m_vec3End,&m_vec3End,&t_mat );

	osMatrixRotationX( &t_mat,_radian );
	osVec3Transform( &m_vec3Start,&m_vec3Start,&t_mat );
	osVec3Transform( &m_vec3End,&m_vec3End,&t_mat );

	osMatrixTranslation( &t_mat,m_vec3Center.x,
		m_vec3Center.y,m_vec3Center.z );
	osVec3Transform( &m_vec3Start,&m_vec3Start,&t_mat );
	osVec3Transform( &m_vec3End,&m_vec3End,&t_mat );

	unguard;
}

//! �ѵ����ص������ĵ��Y����ת���������⡣
void osc_swordGlossy::rot_swordGlossyY( float _radian )
{
	guard;

	osMatrix    t_mat;

	osMatrixTranslation( &t_mat,-m_vec3Center.x,
		-m_vec3Center.y,-m_vec3Center.z );
	osVec3Transform( &m_vec3Start,&m_vec3Start,&t_mat );
	osVec3Transform( &m_vec3End,&m_vec3End,&t_mat );

	osMatrixRotationY( &t_mat,_radian );
	osVec3Transform( &m_vec3Start,&m_vec3Start,&t_mat );
	osVec3Transform( &m_vec3End,&m_vec3End,&t_mat );

	osMatrixTranslation( &t_mat,m_vec3Center.x,
		m_vec3Center.y,m_vec3Center.z );
	osVec3Transform( &m_vec3Start,&m_vec3Start,&t_mat );
	osVec3Transform( &m_vec3End,&m_vec3End,&t_mat );

	unguard;
}

# endif 



////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ��ʼ�����ʵ�����ݡ�

osc_swordGlossyIns::osc_swordGlossyIns()
{

	m_iGlossyPos = 0;
	m_iPRSword = -1;
	m_bPosOverFlow = false;


	m_iGRNum = 0;

	m_ptrSwordGlossy = NULL;

	for( int t_i = 0;t_i<3;t_i ++ )
		m_iEffectId[t_i] = -1;

	m_iShaderId = -1;
	for( int t_i= 0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
		m_iEffectType[t_i] = 0;

	m_iWeaponEffIdx = 0;

	// ��ʼ��Ϊ����Ⱦ����.
	m_dwGlossyColor = 0;

	m_fScaleWeaponEffect = 1.0f;

	m_bReadyToDraw = false;

# if SKINMESH_DEBUG
	m_ptrmesh = NULL;
# endif 

}

osc_swordGlossyIns::~osc_swordGlossyIns()
{
	delete_devdepRes();
}

//! ��䵱ǰ�����õ���indexBuffer���ݡ�
void osc_swordGlossyIns::fill_indexBufferData( WORD* _idxBuf )
{
	guard;

	int      t_idx,t_iBufIdx,t_iStartIdx;

	//
	// ����ʹ�þ�̬�Ķ��㻺����.
	// ����ǰ��Index Buffer���ݡ�
	t_iBufIdx = 0;
	for( t_idx = 0;t_idx<MAX_RENDERSNUM-1;t_idx ++ )
	{
		t_iStartIdx = t_idx*2;

		_idxBuf[t_iBufIdx+0] = t_iStartIdx+0;
		_idxBuf[t_iBufIdx+1] = t_iStartIdx+1;
		_idxBuf[t_iBufIdx+2] = t_iStartIdx+2;

		_idxBuf[t_iBufIdx+3] = t_iStartIdx+2;
		_idxBuf[t_iBufIdx+4] = t_iStartIdx+1;
		_idxBuf[t_iBufIdx+5] = t_iStartIdx+3;

		t_iBufIdx += 6;
	}

	return;



	unguard;
}

// �Ƿ����������ĵ��⵭��
# define ALPHA_GLOSSY_BYPROGRAM  1


//! ��䵶��Ķ������ݣ���Ҫ���㵶���alpha���ݡ�
void osc_swordGlossyIns::fill_vertexBufferData( os_billBoardVertex* _verBuf )
{
	guard;

	int      t_idx,t_iLineIdx;
	BYTE     t_btAlphaAddE ;
	DWORD      t_btCurAlpha = 0;
	float     t_fU;


	// 
	// �����ߵ���Ŀ����ÿһ���ߵ�alphaֵ��
	t_btAlphaAddE = (BYTE)((0xff)/m_iGRNum);
	t_iLineIdx = 0;

# if ALPHA_GLOSSY_BYPROGRAM
	DWORD   t_dwCurColor = 0x00ffffff & m_dwGlossyColor ;
# else
	DWORD   t_dwCurColor = 0xff000000 | m_dwGlossyColor ;
# endif 

	// ����ÿһ���ߵ���ɫ����.�˴�������Ϊ����������,����Ⱦ��Ҫ���ݡ�
	for( int t_i=0;t_i<m_iGRNum;t_i ++ )
	{
		t_idx = t_i*2;

# if ALPHA_GLOSSY_BYPROGRAM
		t_btCurAlpha = t_iLineIdx*t_btAlphaAddE;
		_verBuf[t_idx].m_dwColor =   t_dwCurColor | (t_btCurAlpha<<24);	
		_verBuf[t_idx+1].m_dwColor = t_dwCurColor | (t_btCurAlpha<<24);
# else
		_verBuf[t_idx].m_dwColor =   t_dwCurColor ;	
		_verBuf[t_idx+1].m_dwColor = t_dwCurColor ;
# endif 


		t_fU = 1.0f/m_iGRNum*t_iLineIdx;

		_verBuf[t_idx].m_vec2Uv.x = t_fU;
		_verBuf[t_idx].m_vec2Uv.y = 1.0f;
		_verBuf[t_idx+1].m_vec2Uv.x = t_fU;
		_verBuf[t_idx+1].m_vec2Uv.y = 0.0f;

		t_iLineIdx ++;

		_verBuf[t_idx].m_vec3Pos = m_arrVec3Start[t_i];		
		_verBuf[t_idx+1].m_vec3Pos = m_arrVec3End[t_i];

	}

	return;

	unguard;
}




//! �洢��ǰ֡�ĵ����λ�á�
void osc_swordGlossyIns::add_swordPos( void )
{
	guard;

	m_arrVec3KeyS[m_iGlossyPos] = m_vec3TransStart;
	m_arrVec3KeyE[m_iGlossyPos] = m_vec3TransEnd;

	m_iGlossyPos ++;
	if( m_iGlossyPos == get_sgMoveLength() )
	{
		m_iGlossyPos = 0;
		m_bPosOverFlow = true;
	}

	// 
	// �����֡��ĵ������������һ������ֵ��������֮֡���ֵ��
	recal_sgData();

	unguard;
}


os_billBoardVertex     osc_swordGlossyIns::m_verSG[MAX_RENDERSNUM*2];


//! ��Ⱦ����ؼ�֡�õ�������. 	
osVec3D            osc_swordGlossyIns::m_arrVec3Start[MAX_RENDERSNUM];
osVec3D            osc_swordGlossyIns::m_arrVec3End[MAX_RENDERSNUM];

//! ������Ⱦ�����õ��Ķ��������������.
int                osc_swordGlossyIns::m_iIdxBufId = -1;
int                osc_swordGlossyIns::m_iVerBufId = -1;
osc_middlePipe*     osc_swordGlossyIns::m_ptrMidPipe = NULL;
os_polygon          osc_swordGlossyIns::m_sPolygon;




void osc_swordGlossyIns::init_swordGlossyIns( osc_middlePipe* _mp )
{
	guard;

	osassert( _mp );
	WORD*   t_ptrIdxBuf = new WORD[(MAX_RENDERSNUM-1)*6]; 



	osc_swordGlossyIns::m_ptrMidPipe = _mp;

	osc_swordGlossyIns::fill_indexBufferData( t_ptrIdxBuf );

	osc_swordGlossyIns::m_iIdxBufId = 
		_mp->create_staticIB( t_ptrIdxBuf,(MAX_RENDERSNUM-1)*6 );
	osassert( osc_swordGlossyIns::m_iIdxBufId >= 0 );

	// ʹ�ù�����fvf.
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	osc_swordGlossyIns::m_iVerBufId = _mp->create_dynamicVB( 
		t_dwFvf,sizeof( os_billBoardVertex)*MAX_RENDERSNUM*2 );
	osassert( osc_swordGlossyIns::m_iVerBufId>= 0 );

	SAFE_DELETE_ARRAY( t_ptrIdxBuf );




	//
	// ��ʼ����Ⱦpolygon
	//
	// ��ʼ�������������Ⱦpolygon.
	m_sPolygon.m_sBufData.m_iIbufId = osc_swordGlossyIns::m_iIdxBufId;
	m_sPolygon.m_sBufData.m_iVbufId = osc_swordGlossyIns::m_iVerBufId;
	m_sPolygon.m_bUseStaticVB = true;


	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_billBoardVertex );
	m_sPolygon.m_iVerNum = 0;
	m_sPolygon.m_iPriNum = 0;


	// ����ĵ���id��shader���豸��������д�����ÿһ�������Ӧһ������
	osassert( g_shaderMgr );

	unguard;
}


//! ���ݵ���ĳ��ȣ���ʼ�㣬�����㣬���µ���ؼ��߽������λ�á�ATTENTION TO OPP:Inline
void osc_swordGlossyIns::get_sgEndPt( osVec3D _spt,osVec3D& _ept,float _len )
{
	guard;

	osassertex( _len >= 0.0f,"���ⳤ�Ȳ���Ϊ��" );

	osVec3D t_vec3Len;

	t_vec3Len = _ept - _spt;
	osVec3Normalize( &t_vec3Len,&t_vec3Len );
	t_vec3Len *= _len;

	_ept = t_vec3Len + _spt;

	return;

	unguard;
}



//! ��InterPos�����յĵ���ؼ���֮���ֵ��
void osc_swordGlossyIns::Interplate_keyLine( int _is,int _ie )
{
	guard;

	osassert( _is>=0 );
	osassert( _ie>_is );

	osVec3D  t_vec3InterSVec,t_vec3InterEVec,t_vec3Tmp;
	int      t_iInterNum = _ie-_is;
	float    t_fInterLengthE,t_fInterLengthS,t_fLen;

	t_vec3InterEVec = m_arrVec3End[_ie] - m_arrVec3End[_is];
	t_vec3InterSVec = m_arrVec3Start[_ie] - m_arrVec3Start[_is];
	t_fInterLengthE = osVec3Length( &t_vec3InterEVec );
	t_fInterLengthS = osVec3Length( &t_vec3InterSVec );
	osVec3Normalize( &t_vec3InterSVec,&t_vec3InterSVec );
	osVec3Normalize( &t_vec3InterEVec,&t_vec3InterEVec );

	t_fInterLengthS = t_fInterLengthS/t_iInterNum;
	t_fInterLengthE = t_fInterLengthE/t_iInterNum;

	float t_fLengthS,t_fLengthE,t_fLengthSet;
	t_fLengthS = osVec3Length( &(m_arrVec3End[_is]-m_arrVec3Start[_is] ) );
	t_fLengthE = osVec3Length( &(m_arrVec3End[_ie]-m_arrVec3Start[_ie] ) );

	for( int t_i=_is+1;t_i < _ie;t_i ++ )
	{
		t_fLen = t_fInterLengthS * (t_i-_is);
		t_vec3Tmp = t_vec3InterSVec * t_fLen;
		m_arrVec3Start[t_i] = m_arrVec3Start[_is] + t_vec3Tmp;

		t_fLen = t_fInterLengthE * (t_i-_is);
		t_vec3Tmp = t_vec3InterEVec * t_fLen;
		m_arrVec3End[t_i] = m_arrVec3End[_is] + t_vec3Tmp;

		// river mod @ 2008-12-2:ԭ����m_ptrSwordGlossy->m_fSgLength
		// �����⣬�˴����ȱ�Ϊʵʱ����
		t_fLengthSet = osn_mathFunc::float_lerp( 
			t_fLengthS,t_fLengthE,float(t_i-_is)/(_ie-_is) );

		get_sgEndPt( m_arrVec3Start[t_i],
			m_arrVec3End[t_i],t_fLengthSet );
	}

	return ;

	unguard;
}


//  
//! ���ݻ������е����ݣ����²��õ�����ص�����.ATTENTION TO OPP: Ӧ���и��򵥵ķ���ʵ�֡�
bool osc_swordGlossyIns::recal_sgData()
{
	guard;

	int          t_idx,t_iDivNum;
	static BOOL   t_bNeedInter[MAX_SWORDPOS];
	static float  t_fKeyLength[MAX_SWORDPOS];
	static osVec3D t_arrVec3BkS[MAX_SWORDPOS];
	static osVec3D t_arrVec3BkE[MAX_SWORDPOS];
	float        t_fTotalLength,t_fLength;
	osVec3D       t_vec3Tmp;


	m_iGRNum = 0;


	t_idx = 0;
	for( int t_i=get_sgMoveLength()-1;t_i>=0;t_i -- )
	{
		t_arrVec3BkS[t_idx] = m_arrVec3KeyS[t_i];
		t_arrVec3BkE[t_idx] = m_arrVec3KeyE[t_i];

		t_idx ++;
	}


	memset( t_bNeedInter,0,sizeof( BOOL )*get_sgMoveLength() );
	memset( t_fKeyLength,0,sizeof( float )*get_sgMoveLength() );
	t_fTotalLength = 0.0f;
	t_idx = 0;

	for( int t_i=0;t_i<get_sgMoveLength()-1;t_i ++ )
	{
		t_vec3Tmp = t_arrVec3BkE[t_i+1] - t_arrVec3BkE[t_i];
		t_fLength = osVec3Length( &t_vec3Tmp );
		t_fKeyLength[t_idx] = t_fLength;
		if( t_fLength > MINDIS_INGRLINE )
		{
			t_fTotalLength += t_fLength;
			t_bNeedInter[t_idx] = TRUE;
		}
		t_idx ++;
	}

	t_iDivNum = float_div( t_fTotalLength,MINDIS_INGRLINE );
	t_fLength = 0.0f;

	// 
	// �����֮֡��ľ������,����Ҫ�ַ�����������Ⱦ�ؼ�������.
	t_iDivNum = (t_iDivNum+20)/MAX_RENDERSNUM;
	if( t_iDivNum == 0 )
		t_iDivNum = 1;
	t_fLength = MINDIS_INGRLINE*t_iDivNum;


	t_idx = 0;

	m_arrVec3Start[t_idx] = t_arrVec3BkS[0];
	m_arrVec3End[t_idx] = t_arrVec3BkE[0];
	t_idx ++;
	m_iGRNum = 1;

	for( int t_i=0;t_i<get_sgMoveLength()-1;t_i ++ )
	{
		if( !t_bNeedInter[t_i] )
		{
			m_arrVec3Start[t_idx] = t_arrVec3BkS[t_i+1];
			m_arrVec3End[t_idx] = t_arrVec3BkE[t_i+1];
			t_idx ++;
			m_iGRNum ++;
			continue;
		}

		// �����Ҫ��ֵ,�����ֵ����Ŀ.
		t_iDivNum = float_div( t_fKeyLength[t_i],t_fLength );

		/*
		// River added @ 2007-5-11:�ȴ��ؼ��ߵ���Ŀ����:
		osassertex( (t_idx+t_iDivNum)<MAX_RENDERSNUM,
			va( "����ؼ�����Ŀ����<%d,%d>..\n",t_idx,t_iDivNum ) );
		*/
		// River mod @ 2007-9-17:�����������,����ֵ���ؼ�����Ŀ����,��֡��������
		if( (t_idx + t_iDivNum)>=MAX_RENDERSNUM )
		{
			osDebugOut( "����ؼ�����Ŀ����<%d,%d><%f,%f>..\n",
				t_idx,t_iDivNum,t_fKeyLength[t_i],t_fLength );
			return false;
		}

		osassert( t_iDivNum >= 0 );

		m_arrVec3Start[t_idx+t_iDivNum] = t_arrVec3BkS[t_i+1];
		m_arrVec3End[t_idx+t_iDivNum] = t_arrVec3BkE[t_i+1];

		if( t_iDivNum > 0 )
			Interplate_keyLine( t_idx-1,t_idx+t_iDivNum );

		t_idx += (t_iDivNum+1);
		m_iGRNum += (t_iDivNum+1);
	}
	
	return true;

	unguard;
}




//! ���նԵ������Ⱦ.
void osc_swordGlossyIns::final_renderSG( int _lineNum )
{
	guard;

	// �����indexBlend,������Ⱦ����֮ǰ��Ҫ�ظ���Ⱦ״̬.
	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		m_ptrMidPipe->set_renderState( D3DRS_VERTEXBLEND,D3DVBF_DISABLE );
		m_ptrMidPipe->set_renderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		m_ptrMidPipe->set_worldMatrix( osc_skinMesh::m_smatIdentity );
	}


	m_sPolygon.m_iPriNum = (_lineNum-1)*2;
	m_sPolygon.m_iVerNum = _lineNum*2;

	m_sPolygon.m_sBufData.m_iVertexStart = 
		osc_swordGlossyIns::m_ptrMidPipe->fill_dnamicVB(
		osc_swordGlossyIns::m_iVerBufId,
		m_verSG,sizeof( os_billBoardVertex ),_lineNum*2 );

	osassert( m_sPolygon.m_sBufData.m_iVertexStart>= 0 );

	// ÿһ����Ⱦ����Ҫresetǰ�˹ܵ���VbufferCatch��ʹ�ÿ�����������Vbuffer.
	m_ptrMidPipe->set_nullStream();

	// ���ø���ǰalpha�õ�����ɫ��������ӵ���Ⱦ״̬��
	m_ptrMidPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_ptrMidPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE );
	m_ptrMidPipe->render_polygon( &m_sPolygon );

	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		m_ptrMidPipe->set_renderState( D3DRS_VERTEXBLEND,D3DVBF_2WEIGHTS );
		m_ptrMidPipe->set_renderState( D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE );
	}

	unguard;
}


//! ���������Ч�Ĵ���
void osc_swordGlossyIns::process_sgEffect(  bool _bindPos/* = true*/  )
{
	guard;

	// river mod @ 2006-6-21:���ڿ�����������Ч�Ƿ���֡�
	// ��������Ч��play_swordGlossyʹ��ͬ���Ľӿڡ�
	if( m_iEffectId[0] == -1 )
		return;

	osQuat     t_sOsaQuat;

	// 
	// �����osa��Ч,�������Ч��Ҫ��ת�ķ���.
	if( (m_iEffectType[0] == 3) ||
		(m_iEffectType[1] == 3) ||
		(m_iEffectType[2] == 3)   )
	{
		osQuat      t_sYRotQuat;
		osVec3D     t_vec3From( 0.0f,1.0f,0.0f );
		osVec3D     t_vec3To;

		// �����ķ�����ƥ�����ת.
		t_vec3To = m_arrVec3KeyE[0] - m_arrVec3KeyS[0];
		osn_mathFunc::osQuaternionAxisToAxis( &t_sOsaQuat,&t_vec3From,&t_vec3To );


		if( !float_equal( m_ptrSwordGlossy->m_fOsaRotYAngle,0.0f ) )
		{
			// ͷһ���������ת.
			osQuaternionRotationAxis( &t_sYRotQuat,
				&t_vec3From,m_ptrSwordGlossy->m_fOsaRotYAngle );

			// ������ת����.
			osQuaternionMultiply( &t_sOsaQuat,&t_sYRotQuat,&t_sOsaQuat );

		}

	}


	osc_TGManager*    t_ptrScene = (osc_TGManager*)::get_sceneMgr();
	osVec3D           t_vec3GAxis,t_vec3EffectPos;
	osassert( t_ptrScene );



	for( int t_i=0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		// River @ 2009-12-10:
		// �����λ��û����Ч��������һ��
		if( m_iEffectId[t_i] < 0 )
			continue;

		// 
		// River @ 2010-3-2: ÿһ���������Ч��Ӧһ��λ����Ϣ
		// ����õ������������Ч��λ�á�ATTENTION: �ںδ�������Щ��Ч��λ��?
		float t_fPos = m_ptrSwordGlossy->m_fEffectPos;
		if( t_i == 1 ) t_fPos = this->m_ptrSwordGlossy->m_fAddEffectPos[0];
		if( t_i == 2 ) t_fPos = this->m_ptrSwordGlossy->m_fAddEffectPos[1];
		osVec3Lerp( &t_vec3EffectPos,&m_arrVec3KeyS[0],&m_arrVec3KeyE[0],t_fPos );
		//@{
		//! River @ 2010-2-3:���ڴ������ڲ�����������ЧЧ��
		//                   �ŵ�ԭ�㣬�϶�������
		if( !_bindPos )
			t_vec3EffectPos = osVec3D( 0.0f,0.0f,0.0f );
		//@}

		switch( m_iEffectType[t_i] )
		{
		case 1:
			t_ptrScene->set_billBoardPos( m_iEffectId[t_i],t_vec3EffectPos );
			//@{
			// River added @ 2006-6-30:������������Ĺ����
			t_vec3GAxis = m_arrVec3KeyE[0] - m_arrVec3KeyS[0];
			osVec3Normalize( &t_vec3GAxis,&t_vec3GAxis );
			t_ptrScene->set_bbRotAxis( m_iEffectId[t_i],t_vec3GAxis );
			//@}

			t_ptrScene->scale_billBoard( m_iEffectId[t_i],m_fScaleWeaponEffect );
			t_ptrScene->update_bbPos( m_iEffectId[t_i],(I_camera*)g_ptrCamera );
			break;
		case 2:
			//
			t_ptrScene->set_particlePos( m_iEffectId[t_i],t_vec3EffectPos,true );

			//! River @ 2010-6-9: ȷ������������������Ч��Ⱦ��ȷ
			t_ptrScene->update_parRenderState( m_iEffectId[t_i] );
			
			// River added @ 2006-7-1:������������ɢ������Ч��
			osVec3Lerp( &t_vec3GAxis,&m_arrVec3KeyS[0],&m_arrVec3KeyE[0],t_fPos );
			t_ptrScene->set_particleLineSeg( m_iEffectId[t_i],t_vec3GAxis,m_arrVec3KeyE[0] );

			break;
			// ����osa��Ч,����osa��Ч����ת����.
		case 3:
			{			
				t_ptrScene->set_aniMeshPos( m_iEffectId[t_i],t_vec3EffectPos );


				osMatrix    t_mat,t_scaleMat;
				osMatrixRotationQuaternion( &t_mat,&t_sOsaQuat );

				// River added @ 2010-1-7:������Ӧ����������
				float   t_fScale = m_fScaleWeaponEffect * 
					         m_ptrSwordGlossy->m_fScaleVal[t_i];

				// �����������Ч������Ч��
				if( !float_equal( t_fScale,1.0f ) )		
				{
					osMatrixScaling( &t_scaleMat,t_fScale,t_fScale,t_fScale );
					t_mat = t_scaleMat * t_mat;
				}

				t_ptrScene->set_aniMeshTrans( m_iEffectId[t_i],t_mat );
				// ��ʱ������Ч��λ�ú���ת��Ϣ
				t_ptrScene->update_osaPosNRot( m_iEffectId[t_i] );

				//! ȷ��������������Ч��Ⱦ��ȷ
				t_ptrScene->update_osaRenderState( m_iEffectId[t_i] );
				break;
			}
		default:
			break;
		}
	}


	unguard;
}

//! ��ӵ���Ĺؼ�֡��
void osc_swordGlossyIns::add_swordGlossyKey( osMatrix& _rotMat )
{
	guard;

	
	osassert( m_iGlossyPos >= 0 );

	// 
	// �������Ŀ�ʼ�ͽ���λ��,�������˵���Ĺؼ�֡������
	osVec3Transform( &m_vec3TransStart,
		&m_ptrSwordGlossy->m_vec3Start,&_rotMat );
	osVec3Transform( &m_vec3TransEnd,
		&m_ptrSwordGlossy->m_vec3End,&_rotMat );

	m_arrVec3KeyS[m_iGlossyPos] = m_vec3TransStart;
	m_arrVec3KeyE[m_iGlossyPos] = m_vec3TransEnd;


	//osDebugOut( "Add the glossy pos is:<%d>..\n",m_iGlossyPos );


	m_iGlossyPos ++;
	if( m_iGlossyPos == get_sgMoveLength() )
	{
		m_iGlossyPos = 0;
		m_bPosOverFlow = true;
	}

	return;

	unguard;
}



//! �Ե��������Ⱦ
bool osc_swordGlossyIns::draw_swordGlossy( osVec3D& _spos,
			bool _glossyDraw,bool _ds/* = true*/,bool _bindEffPos/* = false*/ )
{
	guard;
	       
	osassert( m_ptrSwordGlossy );

	// 
	// �˱��������ⲿ�������ڵ��ϻ������Ϸ�����Ч��ͷһ��λ�ò��ǵ�����λ�ã�
	osVec3Lerp( &_spos,&m_arrVec3KeyS[0],
		&m_arrVec3KeyE[0],0.5f );


	//  
	//! River added @ 2011-2-25:���̵߳��뵶����ص�����,��С���̵߳�ѹ��
	if( !m_bReadyToDraw )
	{
		m_iGlossyPos = 0;
		return true;
	}

	// �����⸽�ӵ���Ч
	if( m_iEffectId >= 0 )
		process_sgEffect( _bindEffPos );


	// 
	// ���û�ж��嵶��༭���ã�����������
# if __GLOSSY_EDITOR__

# else

	// �������Ҫ�����⣬ҲҪ����
	if( (m_iPRSword == -1)||(!_ds)||(!_glossyDraw) )
	{
		// River added @ 2005-7-18: ������Ӵ˾䣬�����������Ч�����ܽ��������˶�����
		// ���赱ǰ�ĵ���λ��Ϊ0
		m_iGlossyPos = 0;

		return true;
	}
# endif 



	// 
	// River @ 2005-6-2:
	// ����������ɫalphaֵΪ�գ����Ϊ����ĵ��⣬����Ҫ��Ⱦ��
	// �����ʱ����ĵ������ݽ�����Ϊ�˼�������������Ч��λ��.
	if( (m_dwGlossyColor & 0xff000000) !=0 )
	{

		// �Զ�������ݽ��в�ֵ.
		if( !recal_sgData() )
		{
			m_iGlossyPos = 0;
			return true;
		}


		// ֻ��һ���߲��ܻ������Ρ�
		if( m_iGRNum == 0  )
			return true;

		// River mod @ 2011-3-23:ʹ�ÿͻ����ݲ�����.
		/*
		osassertex( m_iShaderId >= 0,
			va( "Cur SGI is<0x%x>...\n",DWORD(this) ) );
		*/
		if( m_iShaderId < 0 )
		{
			// ���赱ǰ�ĵ���λ��Ϊ0.
			m_iGlossyPos = 0;
			return true;
		}


		// �����Ⱦ����Ķ�������,��������,����Ⱦ.
		fill_vertexBufferData( m_verSG );

		m_sPolygon.m_iShaderId = m_iShaderId;

		final_renderSG( m_iGRNum );

	}

	// ���赱ǰ�ĵ���λ��Ϊ0.
	m_iGlossyPos = 0;

	return true;

	unguard;
}




# if __GLOSSY_EDITOR__
//! ���õ�ǰglossy��Ӧ������
void osc_swordGlossyIns::set_glossyTex( const char* _tex )
{
	guard;

	osassert( _tex&&(_tex[0]) );
	osassertex( strlen( _tex )<32,"���������Ӧ�����ֳ��ȱ���С��32���ֽ�.." );

	strcpy( m_ptrSwordGlossy->m_szGlossyTex,_tex );

	if( m_iShaderId >= 0 )
	{
		g_shaderMgr->release_shader( m_iShaderId );
		m_iShaderId = -1; 
	}

	m_iShaderId = g_shaderMgr->add_shader( m_ptrSwordGlossy->m_szGlossyTex, osc_engine::get_shaderFile( "billBoard" ).c_str() );
	if( m_iShaderId < 0 )
		m_iShaderId = g_shaderMgr->add_shadertomanager( m_ptrSwordGlossy->m_szGlossyTex,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
	osassert( m_iShaderId >= 0 );


	return ;

	unguard;
}

//! �õ����ӵ�addPos.
void osc_swordGlossyIns::get_sgEffectPos( float& _pos,int _idx )
{
	guard;

	if( _idx == 0 )
		_pos = m_ptrSwordGlossy->m_fEffectPos;
	if( _idx == 1 )
		_pos = m_ptrSwordGlossy->m_fAddEffectPos[0];
	if( _idx == 2 )
		_pos = m_ptrSwordGlossy->m_fAddEffectPos[1];

	return;

	unguard;
}


//! �õ���Ч��ص���Ϣ.
void osc_swordGlossyIns::get_sgEffect( float& _pos,I_weaponEffect& _effFName )
{
	osassert( m_ptrSwordGlossy );

	_pos = m_ptrSwordGlossy->m_fEffectPos;

	_effFName = m_ptrSwordGlossy->m_sEffectName[0];

	return;
}

//! ���õ�����β���ƶ�����
void osc_swordGlossyIns::set_sgMoveLength( int _len )
{
	if( _len < MIN_SWORDPOS )
		m_ptrSwordGlossy->m_iGlossyMoveLength = MIN_SWORDPOS;
	else if( _len > MAX_SWORDPOS )
		m_ptrSwordGlossy->m_iGlossyMoveLength = MAX_SWORDPOS;
	else
		m_ptrSwordGlossy->m_iGlossyMoveLength = _len;

}			   


//! ����osa��Ч�������ת.
void osc_swordGlossyIns::set_osaYRot( float _agl ) 
{ 
	guard;

	if( m_iEffectType[0] != 3 )
	{
		MessageBox( NULL,"��ǰ����Ч���Ͳ���Osa��Ч..","����",MB_OK );
		return ;
	} 

	m_ptrSwordGlossy->m_fOsaRotYAngle = _agl ; 

	unguard;
} 

//! ���ø��ӵ���Чλ��
void osc_swordGlossyIns::set_sgEffectPos( float _pos,int _idx )
{
	guard;

	switch( _idx )
	{
	case 0:
		this->m_ptrSwordGlossy->m_fEffectPos = _pos;
		break;
	case 1:
		this->m_ptrSwordGlossy->m_fAddEffectPos[0] = _pos;
		break;
	case 2:
		this->m_ptrSwordGlossy->m_fAddEffectPos[1] = _pos;
		break;
	default:
		osassert( false );
	}

	return;


	unguard;
}


//! ������Ч�����ű�����
float osc_swordGlossyIns::set_effScale( int _idx,float _scale )
{
	guard;

	float  t_fBack = 1.0f;

	// �������ֵ�����㣬�����Ŵ�������Ч�����򣬽����ص�ǰ��Ч������
	if( m_ptrSwordGlossy )
	{
		t_fBack = m_ptrSwordGlossy->m_fScaleVal[_idx];
		if( _scale > 0.001f )
		{
			m_ptrSwordGlossy->m_fScaleVal[_idx] = _scale;
		
			// ��Ч���Ÿı䣬ɾ�������´�����Ч
			delete_devdepRes();
			if( !create_devdepRes( m_iWeaponEffIdx ) )
				osassert( false );
		}
	}

	return t_fBack;

	unguard;
}


//! �õ�ʹ�õ���Ķ���������
void osc_swordGlossyIns::get_swordGlossyActIdx( int& _actNum,
								  int* _actIdx,DWORD* _color,char* _glossyTex )
{
	guard;

	_actNum = m_ptrSwordGlossy->m_iActNum;
	
	memcpy( _actIdx,m_ptrSwordGlossy->m_vecActIdx,sizeof( int )*_actNum );
	memcpy( _color,m_ptrSwordGlossy->m_vecGlossyColor,sizeof( DWORD )*_actNum );

	strcpy( _glossyTex,m_ptrSwordGlossy->m_szGlossyTex );

	return;

	unguard;
}

# endif 


//! �ڲ��ú���: ��ɫ����ʹ�õ���,���Ҳ���¼����״̬��
void osc_skinMesh::playSwordGlossyInner( bool _glossyEnable,bool _weaponEffEnable )
{
	guard;

	m_bPlaySGlossy = _glossyEnable;
	m_bPlayWeaponEffect = _weaponEffEnable;

	if( _weaponEffEnable )
	{
		// River @ 2011-2-26:���߳���������Ӧ������
		os_charLoadData   t_sCharLoad;
		t_sCharLoad.m_ptrSkinPtr = this;
		t_sCharLoad.m_bPlaySwordGlossy = true;
		// �����ٵĴ�����Ч.
		set_vipMTLoad();

		osc_skinMeshMgr*   t_ptrMgr = osc_skinMeshMgr::Instance();

		m_iMT_PlayWeaponEffect ++;
		t_ptrMgr->m_sCharLoadThread.push_loadChar( &t_sCharLoad );
	}

	if( m_iMT_PlayWeaponEffect <= 0 )
	{
		// ɾ��������Ч���
		if( m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns )
		{
			m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns->delete_devdepRes( 
				!m_bPlaySGlossy,!m_bPlayWeaponEffect );
		}
		if( m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns )
		{
			m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns->delete_devdepRes( 
				!m_bPlaySGlossy,!m_bPlayWeaponEffect );
		}
	}

	//! River @ 2010-1-7:ȷ�����������������Ч�ܹ���������ʾ
	if( m_bPlayWeaponEffect )
	{
		float  t_fScale = m_vScale.x/m_fBaseSkinScale;
		if( float_equal( 1.0f,t_fScale ) )
			return;

		if( m_ptrSGBpmIns )
			m_ptrSGBpmIns->scale_weaponEffect( t_fScale );
		if( m_ptrSGBpmIns8 )
			m_ptrSGBpmIns8->scale_weaponEffect( t_fScale );		
	}

	unguard;
}

//! ���̴߳���������ص�����.
void osc_skinMesh::playSwordGlossyThread( void )
{
	guard;

	// River mod @ 2011-3-4:��������ڵ���״̬���򲻴���
	if( m_bLoadLock )
		return;

	// 
	// River @ 2011-2-25: ���̴߳�����������
	if( m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns )
	{
		m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns->create_devdepRes(
			m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns->m_iWeaponEffIdx,
			m_bPlaySGlossy,m_bPlayWeaponEffect,m_bUseInInterface );
	}
	if( m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns )
	{
		m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns->create_devdepRes(
			m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns->m_iWeaponEffIdx,
			m_bPlaySGlossy,m_bPlayWeaponEffect,m_bUseInInterface );
	}

	m_bMustUpdateNextFrame = true;

	m_iMT_PlayWeaponEffect --;

	unguard;
}


//! ��ɫ����ʹ�õ���.
void osc_skinMesh::playSwordGlossy( bool _glossyEnable,bool _weaponEffEnable/* = true*/ )
{
	guard;

	//������ڵ���༭����ģʽ�£��򲻴�����Щ����
# if __GLOSSY_EDITOR__
	return;
# endif 

	m_bPlaySGlossy = _glossyEnable;
	m_bPlayWeaponEffect = _weaponEffEnable;

	//! river @ 2009-9-8:�ڲ���¼����״̬
	m_bPlaySGlossyBack = m_bPlaySGlossy;
	m_bPlayWeaponEffectBack = m_bPlayWeaponEffect;

	// River @ 2011-2-25:����Ǵ���������ص�����,���̴߳���
	if( _weaponEffEnable || _glossyEnable )
	{
		os_charLoadData   t_sCharLoad;
		t_sCharLoad.m_ptrSkinPtr = this;
		t_sCharLoad.m_bPlaySwordGlossy = true;
		
		// �����ٵĴ�����Ч.
		set_vipMTLoad();

		osc_skinMeshMgr*   t_ptrMgr = osc_skinMeshMgr::Instance();
		m_iMT_PlayWeaponEffect ++;
		t_ptrMgr->m_sCharLoadThread.push_loadChar( &t_sCharLoad );
		
	}

	//! River @ 2010-1-7:ȷ�����������������Ч�ܹ���������ʾ
	if( m_bPlayWeaponEffect )
	{
		float  t_fScale = m_vScale.x/m_fBaseSkinScale;
		if( float_equal( 1.0f,t_fScale ) )
			return;

		if( m_ptrSGBpmIns )
			m_ptrSGBpmIns->scale_weaponEffect( t_fScale );
		if( m_ptrSGBpmIns8 )
			m_ptrSGBpmIns8->scale_weaponEffect( t_fScale );		
	}

	if( m_iMT_PlayWeaponEffect <=0 )
	{
		// ɾ��������Ч���
		if( m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns )
		{
			m_vecBpmIns[WEAPON_PARTID7].m_ptrSGIns->delete_devdepRes( 
				!m_bPlaySGlossy,!m_bPlayWeaponEffect );
		}
		if( m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns )
		{
			m_vecBpmIns[WEAPON_PARTID8].m_ptrSGIns->delete_devdepRes( 
				!m_bPlaySGlossy,!m_bPlayWeaponEffect );
		}
	}

	unguard;
}

bool osc_swordGlossyIns::create_devdepRes( int _idx,
				bool _glossy/* = true*/,bool _effect/* = true*/,
				bool _topLayer/* = false*/ )
{
	guard;

	osassert( this->m_ptrSwordGlossy );

	osc_TGManager*    t_ptrScene = (osc_TGManager*)::get_sceneMgr();
	osassert( t_ptrScene );
 

	// ��ǰ�����Ӧ��������
	if( _glossy )
	{
		if( (m_iShaderId == -1)&&(NULL!=m_ptrSwordGlossy->m_szGlossyTex[0]) )
		{
			m_iShaderId = g_shaderMgr->add_shader( 
				m_ptrSwordGlossy->m_szGlossyTex,osc_engine::get_shaderFile( "billBoard" ).c_str() );
		}
		if( m_iShaderId < 0 )
		{
			m_iShaderId = g_shaderMgr->add_shadertomanager( 
				m_ptrSwordGlossy->m_szGlossyTex,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
		}
		osassert( m_iShaderId >= 0 );
	}

	// �������Ҫ�����Ͳ���������Ч
	if( !_effect )
		return true;


	//
	// ��Чû�д������򴴽�,��һ����Ч������������
	for( int t_i=0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		if( NULL == m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i][0] )
			m_iEffectType[t_i] = 0;
		else
		{
			m_iEffectType[t_i] = get_effTypeFromName( 
				m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] );
		}

		// 
		// River @ 2010-12-24:�޸Ĵ��룬���Ҫ��������Ч������ͬ������Ҫɾ��֮���ٴ����ˡ�
		if( m_iEffectId[t_i] >= 0 )
		{
			switch( m_iEffectType[t_i] )
			{
			case 1:
				{
					if( strcmp( t_ptrScene->get_bbNameFromId( m_iEffectId[t_i] ),
						m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ) == 0 )
						continue;
					else
						t_ptrScene->delete_billBoard( m_iEffectId[t_i] );
				}
				break;
			case 2:
				{
					if( strcmp( t_ptrScene->get_parNameFromId( m_iEffectId[t_i] ),
						m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ) == 0 )
						continue;
					else
						t_ptrScene->delete_particle( m_iEffectId[t_i] );
				}
				break;
			case 3:
				{
					if( strcmp( t_ptrScene->get_aniNameFromId( m_iEffectId[t_i] ),
						m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ) == 0 )
						continue;
					else
					{
						t_ptrScene->delete_animationMesh( m_iEffectId[t_i] );
						//osDebugOut( "The swordglossy delete aniid<%d><%d>...\n",
						//	m_iEffectId[t_i],m_ptrmesh );
					}
				}
				break;
			default:
				osDebugOut( "����Ҫ����Ĵ���ĵ��⸽����Ч��ʽ" );
			}
			m_iEffectId[t_i] = -1;
		}
	}



	// 
	// River added @ 2007-10-11:����,����Ѿ�����Ч,��ɾ��ԭ����Ч��
	for( int t_i=0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		if( (m_iEffectType[t_i]>0)&&( m_iEffectId[t_i] == -1) )
		{
			switch( m_iEffectType[t_i] )
			{
			case 1:
				{
					osVec3D  t_vec3Pos( 0.0f,0.0f,0.0f );

					osassert(m_ptrSwordGlossy);
					m_iEffectId[t_i] = t_ptrScene->create_billBoard( 
						m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i],
						t_vec3Pos,true,m_ptrSwordGlossy->m_fScaleVal[t_i] );

# if __GLOSSY_EDITOR__
					if( !osc_bbManager::Instance()->is_loopPlayBB( m_iEffectId[t_i] ) )
					{
						osassertex( false,va( "�����<%s>Ϊ���β��ŵĹ����.",
							m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ) );
					}
# endif 

					break;
				}
			case 2:
				{
					os_particleCreate  t_c;
					t_c.m_bAutoDelete = false;
					t_c.m_bCreateIns = true;
					strcpy( t_c.m_szParticleName,
						m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] );
					t_c.m_vec3Offset = osVec3D( 0.0f,0.0f,0.0f );
					osassert(m_ptrSwordGlossy);
					t_c.m_fSizeScale = m_ptrSwordGlossy->m_fScaleVal[t_i];

					// River @ 2010-6-8:�������Ĳ�����ȷ�Ͻ�����Ⱦ����������Ч����
					m_iEffectId[t_i] = t_ptrScene->create_particle( t_c,FALSE,_topLayer );
# if __GLOSSY_EDITOR__
					if( !t_ptrScene->is_loopPlayParticle( m_iEffectId[t_i] ) )
					{
						osassertex( false,va("����<%s>Ϊ���β�������...",
							m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i]) );
					}
# endif

					break;
				}
			case 3:
				{
					os_aniMeshCreate   t_c;
					t_c.m_bAutoDelete = false;
					t_c.m_bCreateIns = true;
					t_c.m_bEffectByAmbiL = false;
					t_c.m_dwAmbientL = 0xffffffff;
					t_c.m_fAngle = 0.0f;
					osassert(m_ptrSwordGlossy);
					t_c.m_fSizeScale = m_ptrSwordGlossy->m_fScaleVal[t_i];

					strcpy( t_c.m_szAMName,
						m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] );
					t_c.m_ePlayType = OSE_PLAYFOREVER;

					osassert( m_iEffectId[t_i] == -1 );
					m_iEffectId[t_i] = t_ptrScene->create_osaAmimation( &t_c,_topLayer ); 
					//osDebugOut( "The swordglossy create aniid<%d><%d>...\n",
					//	m_iEffectId[t_i],m_ptrmesh );

				}
				break;
			default:
				osassertex( false,"���ܴ���ĵ��⸽����Ч��ʽ" );
			}

			osassert( m_iEffectId[t_i] >= 0 );
			if( m_iEffectId[t_i] < 0 )
				return false;
		}
	}

	// 
	// River added @ 2011-2-25:���̵߳��뵶���������,��С���߳�ѹ��.
	m_bReadyToDraw = true;

	return true;

	unguard;
}


//! �����������ӵ���Ч.
void osc_swordGlossyIns::scale_weaponEffect( float _rate )
{
	guard;

	m_fScaleWeaponEffect = _rate;

	unguard;
}


//! ������ɾ�����������Ч: 
void osc_swordGlossyIns::delete_devdepRes( bool _glossy/* = true*/,bool _effect/* = true*/ )
{
	guard;

	m_bReadyToDraw = false;

	// ɾ�������Ӧ������shaderId.
	if( (m_iShaderId >= 0)&&_glossy )
	{
		g_shaderMgr->release_shader( m_iShaderId );
		m_iShaderId = -1;
	}

	// 
	// �������Ҫɾ���������ӵ���Ч
	if( !_effect )
		return;

	I_fdScene*    t_ptrScene = ::get_sceneMgr();
	
	if( NULL == t_ptrScene )
		return;

	// 
	// ɾ��ԭ����Ч��
	for( int t_i = 0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		if( m_iEffectId[t_i]>=0 )
		{
			switch( m_iEffectType[t_i] )
			{
			case 1:
				t_ptrScene->delete_billBoard( m_iEffectId[t_i] );
				break;
			case 2:
				t_ptrScene->delete_particle( m_iEffectId[t_i] );
				break;
			case 3:
				{
					t_ptrScene->delete_animationMesh( m_iEffectId[t_i] );
					//int  t_id = WORD(m_iEffectId[t_i]);
					//osDebugOut( "The swordglossy delete aniid<%d>...\n",t_id );
				}
				break;
			default:
				osDebugOut( "����Ҫ����Ĵ���ĵ��⸽����Ч��ʽ" );
			}

			m_iEffectId[t_i] = -1;
		}
	}


	unguard;
}


//! ���ļ�����չ���õ���Ч������
int osc_swordGlossyIns::get_effTypeFromName( const char* _name )
{
	guard;

	s_string    t_str = _name;

	//! river @ 2009-12-10:���Ϊ�գ�typeΪ��
	if( _name[0] == NULL )
		return 0;

	int  t_idx = t_str.rfind( "." );
	t_str = t_str.substr( t_idx+1,t_str.length() );
	std::transform( t_str.begin(),t_str.end(),t_str.begin(),tolower );

	if( t_str == "bb" )
		return 1;
	if( t_str == "fdp" )
		return 2;
	if( t_str == "osa" )
		return 3;

	return 0;

	unguard;
}



//! �л���������Ч��ָ��������
bool osc_swordGlossyIns::change_weaponEffect( int _idx )
{
	guard;

	osassert( _idx < 4 );
	
	osassertex( m_ptrSwordGlossy,"û�г�ʼ����������Ч...\n" );
	
# if !__GLOSSY_EDITOR__
	// River @ 2011-1-11: �༭������Ҫ�������л���
	// River @ 2010-12-24:�������ͬ���л���û�б�Ҫ�ٻ��˰ɣ����ܲ���bug,��β���
	if( m_iWeaponEffIdx == _idx )
		return true;
# endif 

	//
	m_iWeaponEffIdx = _idx;

	if( NULL == m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[0][0] )
	{
		// ������л�����ЧΪ�գ���ɾ����ǰ����Ч����ֱ�ӷ��ء�
		delete_devdepRes( false,true );
		return true;
	}

	// ����༭����ģʽ��,�и��ϸ������
# if __GLOSSY_EDITOR__
	for ( int t_i = 0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		if( m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i][0] )
		{
			osassertex( ::file_exist( (char*)m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ),
				va( "Ҫ�л���������Ч<%s>������...\n",m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ) );
		}
	}
# endif 

	// ɾ���ʹ����µ�������Ч�������Ĳ���
	delete_devdepRes( false,true );
	if( m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[0] )
		create_devdepRes( _idx,false,true );

	return true;

	unguard;
}



// River mod @ 2010-8-11:ƽʱ���Եõ�weaponEffect��ص�����
# if  __GLOSSY_EDITOR__

//! �����������õ���ǰweapon��Ч���ļ�����
const I_weaponEffect* osc_skinMesh::get_weaponEffect( 
	         int _idx,WEAPON_ID _weapid/* = WEAPON7*/  )
{
	guard;


	if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8 )
			return false;
		return m_ptrWeaponBpm8->get_weaponEffect( _idx );
	}

	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm )
			return false;
		return m_ptrWeaponBpm->get_weaponEffect( _idx );
	}

	return NULL;

	unguard;
}

# else
//! �����������õ���ǰweapon��Ч���ļ�����
const I_weaponEffect* osc_skinMesh::get_weaponEffect( 
	         int _idx,WEAPON_ID _weapid/* = WEAPON7*/  )
{
	guard;


	if( m_vecBpmIns[_weapid].m_ptrSGIns )
		if( m_vecBpmIns[_weapid].m_ptrSGIns->m_ptrSwordGlossy )
			return &m_vecBpmIns[_weapid].m_ptrSGIns->m_ptrSwordGlossy->m_sEffectName[_idx%4];;

	return	NULL;


	unguard;
}
# endif 


//! �õ�������Ч���ַ���.
const I_weaponEffect* osc_bodyPartMesh::get_weaponEffect( int _idx )
{
	guard;

	if( m_ptrSwordGlossy )
		return &m_ptrSwordGlossy->m_sEffectName[_idx%4];

	return NULL;

	unguard;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  BodyPartMesh�õ��ĵ������ݡ�
// 

# if  __GLOSSY_EDITOR__

//! ���õ����Ӧ�Ķ���������
void osc_bodyPartMesh::set_swordGlossyActIdx( int _actNum,int* _actIdx,DWORD* _color )
{
	guard;

	
	
	osassert( _actIdx );
	osassert( _actNum < 8 );

	osassert( m_ptrSwordGlossy );

	m_ptrSwordGlossy->m_iActNum = _actNum;
	memcpy( m_ptrSwordGlossy->m_vecActIdx,_actIdx,sizeof( int )*_actNum );
	memcpy( m_ptrSwordGlossy->m_vecGlossyColor,_color,sizeof( DWORD )*_actNum );

	unguard;
}


/** brief
*  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
*
*  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
*  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
*/
int  osc_bodyPartMesh::displaytri_bysel( int _triidx[24],int _trinum )
{
	guard;

	return g_smDevdepMgr->displaytri_bysel( m_iDevdepResId,_triidx,_trinum );


	unguard;
}


// ���õ����õ�����Ч��
bool osc_bodyPartMesh::set_sgEffect( float _pos,const I_weaponEffect& _effFName )
{
	guard;

	osassert( m_ptrSwordGlossy );

	m_ptrSwordGlossy->m_fEffectPos = _pos;
	m_ptrSwordGlossy->m_sEffectName[0] = _effFName;
		
	return true;

	unguard;
}




//! �洢������ص����ݡ�
void osc_bodyPartMesh::save_glossyData( FILE* _file )
{
	guard;

	osassert( _file );

	// �����Ƿ���ʾ���������ڶ�λ��Ч���������ݣ���Ҫ���ѡ�
	fwrite( &m_bDisplay,sizeof( BOOL ),1,_file );

	fwrite( &m_ptrSwordGlossy->m_vec3Start,sizeof( osVec3D ),1,_file );
	fwrite( &m_ptrSwordGlossy->m_vec3End,sizeof( osVec3D ),1,_file );
	fwrite( &m_ptrSwordGlossy->m_iActNum,sizeof( int ),1,_file );

	
	// ��صĶ�����������
	fwrite( m_ptrSwordGlossy->m_vecActIdx,
		sizeof( int ),m_ptrSwordGlossy->m_iActNum,_file );
	
	// ÿһ��������Ӧ�ĵ�����ɫ,��������˵õ����յ���ɫ.
	fwrite( m_ptrSwordGlossy->m_vecGlossyColor,
		sizeof( DWORD ),m_ptrSwordGlossy->m_iActNum,_file );


	//
	// �洢���⸽�ӵ���Ч���ݡ�
	fwrite( &m_ptrSwordGlossy->m_fEffectPos,sizeof( float ),1,_file );
	
	//! river @ 2010-3-6:�洢�¼������Чλ�á�
	fwrite( this->m_ptrSwordGlossy->m_fAddEffectPos,sizeof( float ),2,_file );

	fwrite( m_ptrSwordGlossy->m_sEffectName,sizeof( I_weaponEffect ),4,_file );	

	fwrite( m_ptrSwordGlossy->m_szGlossyTex,sizeof( char ),32,_file );

	fwrite( &m_ptrSwordGlossy->m_fOsaRotYAngle,sizeof( float ),1,_file );

	fwrite( &m_ptrSwordGlossy->m_iGlossyMoveLength,sizeof( int ),1,_file );

	// RIVER @ 2006-6-21:�����������Ч���ŵ�֧��,�汾����Ϊ105
	// RIVER @ 2009-12-10:106�汾������������Ч��Ӧ������
	fwrite( &m_ptrSwordGlossy->m_fScaleVal[0],sizeof( float )*3,1,_file );


	unguard;
}




/** \brief
*  ���õ�N��������Ч�ļ������4����
* 
*  \param int _idx ���õڼ���������Ч�����㿪ʼ�����ֵΪ3.����3�Ļ����ڲ����Ϊ��
*  \param char* _effName ������Ч���ļ���
*/
bool osc_bodyPartMesh::set_weaponEffect( int _idx,const I_weaponEffect& _effName )
{
	guard;

	// 
	//! �ٴ洢�ڲ���������Ч���ݣ���ʱ������ݣ�save_weaponFile��洢��Ӳ�̡�
	if( m_ptrSwordGlossy )
		m_ptrSwordGlossy->m_sEffectName[_idx%4] = _effName;

	return true;

	unguard;
}



//! �洢��ǰ��bpm���ݣ��������汾�ţ������µĵ������ݡ�
bool osc_bodyPartMesh::save_weaponFile( void ) 
{
	guard;



	int       t_iSize;
	s_string   t_szName;

	char   t_ch = m_strFname[ strlen(m_strFname)- WEAPON_PARTOFFSET ];

	// ȷ�ϵ�ǰ��bodyPartMesh�ǵ�7���ֵ�����mesh.ֻ������ĵڶ��͵��߲��ֿ����е��⡣
	osassert( (t_ch==WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR ) );

	t_szName = m_strFname ;
	osn_mathFunc::change_extName( t_szName,GLOSSY_FHEADER );

	t_iSize = GLOSSY_BPMFVERSION107;

	FILE*   t_file;
	t_file = fopen( t_szName.c_str(),"wb" );
	if( !t_file )
		osassert( false );

	fwrite( GLOSSY_FHEADER,sizeof( char ),4,t_file );
	fwrite( &t_iSize,sizeof( int ),1,t_file );

	// 
	// �洢��صĵ�������
	save_glossyData( t_file );

	fclose( t_file );

	return true;


	unguard;
}

//! ��X������ת���ǵĵ������ݡ�
void osc_bodyPartMesh::rot_swordGlossyX( float _radian )
{
	guard;

	m_ptrSwordGlossy->rot_swordGlossyX( _radian );

	unguard;
}

void osc_bodyPartMesh::rot_swordGlossyY( float _radian )
{
	guard;

	m_ptrSwordGlossy->rot_swordGlossyY( _radian );

	unguard;
}

//! ��ǰ�ĵ�����X���Y�����ƶ�һ����������
void osc_bodyPartMesh::move_swordGlossy( bool _x,float _dis )
{
	guard;

	osassert( m_ptrSwordGlossy );

	if( _x )
	{
		m_ptrSwordGlossy->m_vec3Start.x += _dis;
		m_ptrSwordGlossy->m_vec3End.x += _dis;
	}
	else
	{
		m_ptrSwordGlossy->m_vec3Start.y += _dis;
		m_ptrSwordGlossy->m_vec3End.y += _dis;
	}

	return;

	unguard;
}






//! ����ÿһ������ĳ��̡�
void osc_bodyPartMesh::adjust_sowrdGlossySize( bool _start,float _size )
{
	guard;

	osVec3D   t_vec3Dir;

	osassert( m_ptrSwordGlossy );

	if( _start )
	{
		t_vec3Dir = m_ptrSwordGlossy->m_vec3End - m_ptrSwordGlossy->m_vec3Start;
		osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

		t_vec3Dir *= _size;
		m_ptrSwordGlossy->m_vec3Start += t_vec3Dir;

	}
	else
	{
		t_vec3Dir = m_ptrSwordGlossy->m_vec3Start - m_ptrSwordGlossy->m_vec3End;
		osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

		t_vec3Dir *= _size;
		m_ptrSwordGlossy->m_vec3End += t_vec3Dir;

	}

	return;

	unguard;
}


//! ���ô��������ڶ�λ������ʾ��������
void osc_bodyPartMesh::set_displayWeapon( bool _dis )
{
	osassert( m_ptrSwordGlossy );

	m_bDisplay = _dis;

}

//! ���赶��ĸ�����ת�Ƕȣ�ʹ��ȫ�ص�����ǰ�ĵ���
void osc_bodyPartMesh::reset_swordGlossyPosRot( void )
{
	guard;

	osVec3D   t_vec3Max,t_vec3Min,t_vec3Center;

	D3DXComputeBoundingBox( (osVec3D*)m_ptrVerData,
		m_dwNumVertex,m_dwVertexSize,&t_vec3Min,&t_vec3Max );

	if( !m_ptrSwordGlossy )
		m_ptrSwordGlossy = new osc_swordGlossy;

	t_vec3Center = (t_vec3Max+t_vec3Min)/2.0f;
	m_ptrSwordGlossy->m_vec3Start = t_vec3Center;
	m_ptrSwordGlossy->m_vec3Start.z = t_vec3Max.z;
	m_ptrSwordGlossy->m_vec3End   = t_vec3Center;
	m_ptrSwordGlossy->m_vec3End.z = t_vec3Min.z;
	m_ptrSwordGlossy->m_vec3Center = t_vec3Center;
	m_ptrSwordGlossy->m_iActNum = 0;

	// ��Ӧ�ľ���������
	os_characterVer* t_ptrVer = 
		(os_characterVer*)m_ptrVerData;
	m_ptrSwordGlossy->m_iMatrixIdx = t_ptrVer->m_vecBoneIdx[0];

	unguard;
}



//! ������ʼ����Sword Glossy.
void osc_bodyPartMesh::create_defaultGlossy( void )
{
	guard;

	osVec3D   t_vec3Max,t_vec3Min,t_vec3Center;

	// ����Ѿ����ڵ������ݣ��򷵻ء�
	if( m_ptrSwordGlossy )
		return;

	D3DXComputeBoundingBox( (osVec3D*)m_ptrVerData,
		m_dwNumVertex,m_dwVertexSize,&t_vec3Min,&t_vec3Max );

	m_ptrSwordGlossy = new osc_swordGlossy;

	t_vec3Center = (t_vec3Max+t_vec3Min)/2.0f;
	m_ptrSwordGlossy->m_vec3Start = t_vec3Center;
	m_ptrSwordGlossy->m_vec3Start.z = t_vec3Max.z;
	m_ptrSwordGlossy->m_vec3End   = t_vec3Center;
	m_ptrSwordGlossy->m_vec3End.z = t_vec3Min.z;
	m_ptrSwordGlossy->m_vec3Center = t_vec3Center;
	m_ptrSwordGlossy->m_iActNum = 0;

	// ��Ӧ�ľ���������
	os_characterVer* t_ptrVer = 
		(os_characterVer*)m_ptrVerData;
	m_ptrSwordGlossy->m_iMatrixIdx = t_ptrVer->m_vecBoneIdx[0];

	return;

	unguard;
}


/** brief
*  ֻ��ʾĳ�����岿λ�Ĳ���������,�����ڵ���Ŀ��
*
*  \param int _paridx Ҫ��ʾ���������ε����岿λ����,���㿪ʼ��
*  \param int _trinum �������ʾĳ����λȫ���������������������ֵΪ�㼴�ɡ���
*/
int osc_bpmInstance::displaytri_bysel( int _triidx[24],int _trinum )
{
	guard;

	osassert( this->m_ptrBodyPartMesh );

	return m_ptrBodyPartMesh->displaytri_bysel( _triidx,_trinum );

	unguard;
}


//! �ڵ�����������ʱ,��ʼ��bpsInstance��ص����ݡ�
void osc_bpmInstance::init_swordGlossy( void )
{
	guard;

	m_ptrBodyPartMesh->create_defaultGlossy();

	// 
	// ����е�����ص����ݣ�����������ʵ����
	if( m_ptrBodyPartMesh->m_ptrSwordGlossy )
	{
		if( !m_ptrSGIns )
		{
			m_ptrSGIns = get_glossyInsPtr();//new osc_swordGlossyIns;

			m_ptrSGIns->set_swordGlossyPtr( m_ptrBodyPartMesh->m_ptrSwordGlossy );
			m_ptrSGIns->reset_swordGlossy();
			m_ptrSGIns->create_devdepRes( 0 );
		}
	}
	
	unguard;
}




/** \brief
*  ���õ�N��������Ч�ļ������4����
* 
*  \param int _idx ���õڼ���������Ч�����㿪ʼ�����ֵΪ3.����3�Ļ����ڲ����Ϊ��
*  \param char* _effName ������Ч���ļ���
*  \param WEAPON_ID _weapid ���õ��߸����ǵڰ˸�������λ����Ч��
*/
bool osc_skinMesh::set_weaponEffect( int _idx,
					const I_weaponEffect& _effName,WEAPON_ID _weapid/* = WEAPON7*/ )
{
	guard;
	
	if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8 )
			return false;
		m_ptrWeaponBpm8->set_weaponEffect( _idx,_effName );

		//! �л�Ч��
		m_ptrSGIns8->change_weaponEffect( _idx );

	}
	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm )
			return false;
		m_ptrWeaponBpm->set_weaponEffect( _idx,_effName );

		//! �л�Ч��
		m_ptrSGIns->change_weaponEffect( _idx );
	}
	return true;

	unguard;
}



//  ���º���ֻ�ڱ༭����ı༭����ʹ�á�
/** \brief
*  �ѵ�ǰsm�е������ļ����뵽�µ��ļ��С�
*/
bool osc_skinMesh::save_weaponFile( WEAPON_ID _weapid )
{
	guard;
	if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8 )
			return false;
		m_ptrWeaponBpm8->save_weaponFile();
	}
	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm )
		return false;
		m_ptrWeaponBpm->save_weaponFile();
		
	}
	return true;

	unguard;
}

//! ������ǰ�������е���Ĵ�С��
bool osc_skinMesh::adjust_swordGlossySize( bool _start,float _size, WEAPON_ID _weapid  )
{
	guard;

	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
			m_ptrWeaponBpm->adjust_sowrdGlossySize( _start,_size);
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
			m_ptrWeaponBpm8->adjust_sowrdGlossySize( _start,_size);
	}
	

	return true;

	unguard;
}

//! ��X��������ת���⡣
void osc_skinMesh::rot_swordGlossyX( float _radian, WEAPON_ID _weapid  ) 
{
	guard;

	
	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
			m_ptrWeaponBpm->rot_swordGlossyX( _radian);
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
			m_ptrWeaponBpm8->rot_swordGlossyX( _radian);
	}

	unguard;
}
//! ��Y��������ת���⡣
void osc_skinMesh::rot_swordGlossyY( float _radian, WEAPON_ID _weapid  )
{
	guard;
	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
			m_ptrWeaponBpm->rot_swordGlossyY( _radian );
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
			m_ptrWeaponBpm8->rot_swordGlossyY( _radian );

	}

	

	unguard;
}


//! ��X�����Y�����ƶ����⡣
void osc_skinMesh::move_swordGlossy( bool _x,float _dis, WEAPON_ID _weapid  )
{
	guard;
	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
			m_ptrWeaponBpm->move_swordGlossy( _x,_dis );
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
			m_ptrWeaponBpm8->move_swordGlossy( _x,_dis );
	}

	

	unguard;
}

//! ���赶��ĸ�����ת�Ƕȣ�ʹ��ȫ�ص�����ǰ�ĵ���
void osc_skinMesh::reset_swordGlossyPosRot( WEAPON_ID _weapid/* = WEAPON7*/ )
{
	guard;

	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
			m_ptrWeaponBpm->reset_swordGlossyPosRot();
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
			m_ptrWeaponBpm8->reset_swordGlossyPosRot();
	}

	unguard;
}


//! ���ô��������ڶ�λ������ʾ��������
void osc_skinMesh::set_displayWeapon( bool _dis , WEAPON_ID _weapid )
{
	guard;
	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
			m_ptrWeaponBpm->set_displayWeapon( _dis );
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
			m_ptrWeaponBpm8->set_displayWeapon( _dis );
	}

	

	unguard;
}




//! ���õ����Ӧ�Ķ���������
void osc_skinMesh::set_swordGlossyActIdx( int _actNum,int* _actIdx,
					DWORD* _color,const char* _glossyTex , WEAPON_ID _weapid )
{
	guard;
	osassert( _actNum >= 0 );
	osassert( _actIdx );
	osassert( _color );

	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
		{
			m_ptrWeaponBpm->set_swordGlossyActIdx( _actNum,_actIdx,_color );

			osassert( m_ptrSGIns );
			m_ptrSGIns->set_glossyTex( _glossyTex );
		}
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
		{
			m_ptrWeaponBpm8->set_swordGlossyActIdx( _actNum,_actIdx,_color );

			osassert( m_ptrSGIns8 );
			m_ptrSGIns8->set_glossyTex( _glossyTex );
		}
	}

	

	unguard;
}
/** \brief
*  �õ�����ĵ�ǰ������Ӧ�������ݡ�
*  
*  �����λ����Ϣ����Ҫ΢����
*  \param int& _actNum �����ж��ٸ�����ʹ���˵���.
*  \param int* _actIdx ����һ���������飬����������ʹ������12Ԫ�أ�������ʹ�õ����
*                      �Ķ���������
*  \param char* _glossyTex �ϲ㴫����ִ���������һ��64�ֽڵ�char�����飬�����˵�ǰ
*                          ����ʹ�õĵ����������֡�
*/
void osc_skinMesh::get_swordGlossyActIdx( int& _actNum,int* _actIdx,
											 DWORD* _color,char* _glossyTex, WEAPON_ID _weapid  )
{
	guard;
	if (_weapid == WEAPON7)
	{
		if( m_ptrWeaponBpm )
		{
			osassert( m_ptrSGIns );

			m_ptrSGIns->get_swordGlossyActIdx( _actNum,_actIdx,_color,_glossyTex );

		}
		else
		{
			_actNum = 0;
			_actIdx[0] = -1;
			_glossyTex[0] = NULL;
		}
	}
	else if (_weapid == WEAPON8)
	{
		if( m_ptrWeaponBpm8 )
		{
			osassert( m_ptrSGIns8 );

			m_ptrSGIns8->get_swordGlossyActIdx( _actNum,_actIdx,_color,_glossyTex );

		}
		else
		{
			_actNum = 0;
			_actIdx[0] = -1;
			_glossyTex[0] = NULL;
		}
	}
	unguard;
}




//! �Ե������ݽ��д�������༭��ר�ú�����
void osc_skinMesh::weapon_process( int _idx,osc_bodyPartMesh* _bpm )
{
	guard;

	osassert( _idx >= 0);
	osassert( _bpm );

	if( m_iNumParts == 2)
	{

	}
	else
	{
		if( _idx == WEAPON_PARTID7 )
		{
			m_ptrWeaponBpm = _bpm;
			m_vecBpmIns[_idx].init_swordGlossy();
			m_ptrSGIns = m_vecBpmIns[_idx].m_ptrSGIns;
		}
		else if (_idx == WEAPON_PARTID8)
		{
			m_ptrWeaponBpm8 = _bpm;
			m_vecBpmIns[_idx].init_swordGlossy();
			m_ptrSGIns8 = m_vecBpmIns[_idx].m_ptrSGIns;

		}
	}

	unguard;
}

//! river @ 2010-3-5:����effectPos.
void osc_skinMesh::set_sgEffectPos( float _pos,int _idx,WEAPON_ID _weapid/* = WEAPON7*/ )
{
	guard;

	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm  )
			return ;
		m_ptrSGIns->set_sgEffectPos( _pos,_idx );

	}	
	else if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8  )
			return ;
		m_ptrSGIns8->set_sgEffectPos( _pos,_idx );
	}



	unguard;
}


//! ����skinMesh��ǰ�������ӵĵ�����Ч��
void osc_skinMesh::set_sgEffect( 
		float _pos,const I_weaponEffect& _effFName,WEAPON_ID _weapid )
{
	guard;
	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm  )
			return ;

		osassert( m_ptrSGIns );

		osDebugOut( "delete devdepRes ..\n" );
		m_ptrSGIns->delete_devdepRes( false );
		osDebugOut( "������صĵ�����Ч����..\n" );
		m_ptrWeaponBpm->set_sgEffect( _pos,_effFName );
		m_ptrSGIns->change_weaponEffect( 0 );
	}
	else if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8  )
			return ;

		osassert( m_ptrSGIns8 );

		osDebugOut( "�ڰ˲���delete devdepRes ..\n" );
		m_ptrSGIns8->delete_devdepRes(  false  );
		osDebugOut( "�ڰ˲���������صĵ�����Ч����..\n" );
		m_ptrWeaponBpm8->set_sgEffect( _pos,_effFName );
		m_ptrSGIns8->change_weaponEffect( 0 );
	}
	unguard;
}

//! �õ����ӵ�addPos.
void osc_skinMesh::get_sgEffectPos( float& _pos,int _idx,WEAPON_ID _weapid/* = WEAPON7*/ )
{
	guard;

	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm )
			_pos = 0.0f;
		else
			m_ptrSGIns->get_sgEffectPos( _pos,_idx );
	}
	else if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8 )
			_pos = 0.0f;
		else
			m_ptrSGIns8->get_sgEffectPos( _pos,_idx );
	}
	return;

	unguard;
}


/** \brief
*  �õ�����������Ч��ص���Ϣ.
*
*  \param I_weaponEffect& _effFName �����Ӧ��Ӧ�����ݣ������ϲ�
*/
void osc_skinMesh::get_sgEffect( float& _pos,
				I_weaponEffect& _effFName, WEAPON_ID _weapid )
{
	guard;

	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm )
		{
			_pos = 0.0f;
		}
		else
			m_ptrSGIns->get_sgEffect( _pos,_effFName );
	}
	else if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8 )
		{
			_pos = 0.0f;
		}
		else
			m_ptrSGIns8->get_sgEffect( _pos,_effFName );
	}
	return;

	unguard;
}


/** \brief
*  ����������������תosa��Ч,ʹ��osa�ļ��������Ķ�λ���Ӿ�ȷ.
*
*  �����ǰ������Ӧ����Ч���Ͳ���osa,��˺���û��Ч��.
*/
void osc_skinMesh::rotate_osaEffect( float _agl, WEAPON_ID _weapid )
{
	guard;
	if (_weapid == WEAPON7)
	{
		osassert( m_ptrSGIns ); 
		m_ptrSGIns->set_osaYRot( _agl );

	}
	else if (_weapid == WEAPON8)
	{
		osassert( m_ptrSGIns8 ); 
		m_ptrSGIns8->set_osaYRot( _agl );
	}
	unguard;
}

/** \brief
*  ���������ϵ���ЧЧ��
*
*  \param _scale �����1.0���򱣳�ԭ��Ч��С���䣬2.0������Ϊԭ���Ķ���.0.2����СΪԭ����5��֮1
*                �����������ű���С���㣬���ڲ��ı������䣬�����ڲ���ǰ��Ч�����������
*  \return float �����˵�ǰ��Ч�����ű�����
*/
float osc_skinMesh::scale_weaponEffect( int _idx,float _scale,WEAPON_ID _weapin/* = WEAPON7*/ )
{
	guard;

	float   t_f = 1.0f;

	if (_weapin == WEAPON7)
	{
		if( m_ptrSGIns )
			t_f = m_ptrSGIns->set_effScale( _idx,_scale );

	}
	else if (_weapin == WEAPON8)
	{
		if( m_ptrSGIns8 )
			t_f = m_ptrSGIns8->set_effScale( _idx,_scale );
	}

	return t_f;

	unguard;
}



/** \brief 
*  ��ʾ�������һ������֡���ơ�
* 
*  ����Ķ������봦��suspend״̬����������������á�
*
*  \param bool _next ���Ϊtrue,��ʹ����֡�����ﶯ�������Ϊfalse,��ʹ����һ֡�����ơ�
*/
void osc_skinMesh::next_actFramePose( bool _next )
{
	guard;

	if( !m_bSuspendState )
		return;

	int            t_iMaxFrame;
	t_iMaxFrame = m_ptrBoneAni->m_vecBATable[m_iAniId].m_iNumFrame;


	m_fLastUsedTime = 0.0f;

	// ������ڶ����л�״̬�����õ�ǰ��actChangeStateΪfalse.
	if( m_bChangeActState )
	{
		m_bChangeActState = false;
		m_wAniIndex = 0;
	}

	if( _next )
	{
		m_wAniIndex ++;
		if( m_wAniIndex >= t_iMaxFrame )
			m_wAniIndex = 0;
	}
	else
	{
		m_wAniIndex --;
		if( m_wAniIndex < 0 )
			m_wAniIndex = t_iMaxFrame - 1;
	}

	return;

	unguard;
}

/** \brief
*  �õ�ĳһ�������ܹ��ж���֡��
*
*  \param _actName Ĭ�������ʹ�ö����������õ�֡����Frame��Ŀ��
*  \param _actIdx  �����ֵ���ڵ��㣬�򷵻���Ӧ������������Frame��Ŀ
*/
int osc_skinMesh::get_actFrameNum( const char* _actName,int _actIdx/* = -1*/ )
{
	guard;

	os_boneAction*  t_ptrBA;
	int            t_iNameIdx,t_idx;

	if( _actIdx == -1 )
	{
		t_idx = m_ptrBoneAni->get_actIdxFromName( _actName,t_iNameIdx );
		osassert( t_idx>=0 );
	}
	else
	{
		osassert( _actIdx < m_ptrBoneAni->m_iBANum );
		t_idx = _actIdx;
	}

	t_ptrBA = &m_ptrBoneAni->m_vecBATable[t_idx];

	return t_ptrBA->m_iNumFrame;

	unguard;
}

//! �������ﵶ����β�ĳ��ȡ���Ч������6-12��������ڻ���С�ڴ�ֵ��ʹ��Ĭ�ϵ���β����
void osc_skinMesh::set_sgMoveLength( int _len )
{
	guard;

	osassert( m_ptrSGIns );

	m_ptrSGIns->set_sgMoveLength( _len );

	unguard;
}

// �õ�������β�ĳ�������
int osc_skinMesh::get_sgMoveLength( void )
{
	guard;

	osassert( m_ptrSGIns );
	return 	m_ptrSGIns->get_sgMoveLength();

	unguard;
}

/** \brief
*  �õ����岿λ�������������ǰ���岿λ�����ڣ�����0
*
*  \param int _idx ���岿λ���������㿪ʼ��
*/
int osc_skinMesh::get_bodyTriNum( int _idx )
{
	guard;

	// ������ǰ�����岿λ��Ŀ
	if( (_idx + 1)>m_iNumParts )
		return 0;

	if( !m_vecBpmIns[_idx].m_ptrBodyPartMesh )
		return 0;

	return m_vecBpmIns[_idx].m_ptrBodyPartMesh->m_dwNumIndices/3;


	unguard;
}
# endif 





