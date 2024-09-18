///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osChrSwordGlossy.cpp
 * 
 *  His:      River created @ 2004-8-13
 *
 *  Desc:     用于引擎中刀光相关的数据。
 * 
 *  "物有本末，事有始终，知所先后，则近道矣" 
 *
 *  ATTENTION TO FIX:
 *  目前刀光最大的问题是如果帧速度非常低,动作又比较大的情况下,两帧间的刀光插值可能
 *  会出现问题,如果要解决这个问题,最好是手工计算出刀光前几个关键帧所在的位置,这个
 *  跟时间无关,但计算量会增大很多.
 *  
 *  如果两个动作插值的时候，中间跳过了关键帧，这种情况需要特殊的处理。
 *  需要在人物的渲染过程中加入非常的复杂的渲染流程......
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/middlePipe.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"
# include "../../effect/include/fdBillboard.h"

/*
对使用刀光的mesh进行特殊的处理.

如果当前帧的位置A与上一帧的位置B之间出现了动作的关键帧,把关键的位置插入到刀光的渲染中.

如果要在刀光中插入关键帧的位置,如何的插入,关键帧位置的计算

尽量把刀光的frameMove和渲染的功能分开处理.
*/

# define DEFAULT_GLOSSYTEX  "glossy"


osc_swordGlossy::osc_swordGlossy()
{
	m_iActNum = 0;


# if __GLOSSY_EDITOR__	
	// 默认所有的刀光颜色为白色。
	for( int t_i=0;t_i<MAX_ACTGLOSSY;t_i ++ )
		m_vecGlossyColor[t_i] = 0x00000000;
# endif 



	m_fEffectPos = 0.5f;

	m_iMatrixIdx = -1;

	// River mod : 
	m_iGlossyMoveLength = MAX_SWORDPOS;// (MAX_SWORDPOS + MIN_SWORDPOS)/2;

	// 默认的刀光纹理数据。
	strcpy( m_szGlossyTex,DEFAULT_GLOSSYTEX );

	m_fOsaRotYAngle = 0.0f;

	m_fScaleVal[0] = 1.0f;
	m_fScaleVal[1] = 1.0f;
	m_fScaleVal[2] = 1.0f;

	m_fAddEffectPos[0] = 0.5f;
	m_fAddEffectPos[1] = 0.5f;

}



# if  __GLOSSY_EDITOR__



//! 把刀光沿刀光中心点的x轴旋转，用于调整刀光。
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

//! 把刀光沿刀光中心点的Y轴旋转，调整刀光。
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
// 开始刀光的实例数据。

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

	// 初始化为不渲染刀光.
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

//! 填充当前刀光用到的indexBuffer数据。
void osc_swordGlossyIns::fill_indexBufferData( WORD* _idxBuf )
{
	guard;

	int      t_idx,t_iBufIdx,t_iStartIdx;

	//
	// 可以使用静态的顶点缓冲区.
	// 处理当前的Index Buffer数据。
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

// 是否加入程序计算的刀光淡出
# define ALPHA_GLOSSY_BYPROGRAM  1


//! 填充刀光的顶点数据，主要计算刀光的alpha数据。
void osc_swordGlossyIns::fill_vertexBufferData( os_billBoardVertex* _verBuf )
{
	guard;

	int      t_idx,t_iLineIdx;
	BYTE     t_btAlphaAddE ;
	DWORD      t_btCurAlpha = 0;
	float     t_fU;


	// 
	// 根据线的数目分配每一条线的alpha值。
	t_btAlphaAddE = (BYTE)((0xff)/m_iGRNum);
	t_iLineIdx = 0;

# if ALPHA_GLOSSY_BYPROGRAM
	DWORD   t_dwCurColor = 0x00ffffff & m_dwGlossyColor ;
# else
	DWORD   t_dwCurColor = 0xff000000 | m_dwGlossyColor ;
# endif 

	// 处理每一条线的颜色数据.此处的数组为处理后的数据,纯渲染需要数据。
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




//! 存储当前帧的刀光的位置。
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
	// 如果两帧间的刀光相差矩离大于一定的数值，在这两帧之间插值。
	recal_sgData();

	unguard;
}


os_billBoardVertex     osc_swordGlossyIns::m_verSG[MAX_RENDERSNUM*2];


//! 渲染刀光关键帧用到的数据. 	
osVec3D            osc_swordGlossyIns::m_arrVec3Start[MAX_RENDERSNUM];
osVec3D            osc_swordGlossyIns::m_arrVec3End[MAX_RENDERSNUM];

//! 用于渲染刀光用到的顶点和索引缓冲区.
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

	// 使用公告板的fvf.
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	osc_swordGlossyIns::m_iVerBufId = _mp->create_dynamicVB( 
		t_dwFvf,sizeof( os_billBoardVertex)*MAX_RENDERSNUM*2 );
	osassert( osc_swordGlossyIns::m_iVerBufId>= 0 );

	SAFE_DELETE_ARRAY( t_ptrIdxBuf );




	//
	// 初始化渲染polygon
	//
	// 初始化并填充我们渲染polygon.
	m_sPolygon.m_sBufData.m_iIbufId = osc_swordGlossyIns::m_iIdxBufId;
	m_sPolygon.m_sBufData.m_iVbufId = osc_swordGlossyIns::m_iVerBufId;
	m_sPolygon.m_bUseStaticVB = true;


	m_sPolygon.m_iPriType = TRIANGLELIST;
	m_sPolygon.m_iVerSize = sizeof( os_billBoardVertex );
	m_sPolygon.m_iVerNum = 0;
	m_sPolygon.m_iPriNum = 0;


	// 人物的刀光id的shader在设备相关数据中创建，每一个刀光对应一个纹理。
	osassert( g_shaderMgr );

	unguard;
}


//! 根据刀光的长度，开始点，结束点，重新刀光关键线结束点的位置。ATTENTION TO OPP:Inline
void osc_swordGlossyIns::get_sgEndPt( osVec3D _spt,osVec3D& _ept,float _len )
{
	guard;

	osassertex( _len >= 0.0f,"刀光长度不能为负" );

	osVec3D t_vec3Len;

	t_vec3Len = _ept - _spt;
	osVec3Normalize( &t_vec3Len,&t_vec3Len );
	t_vec3Len *= _len;

	_ept = t_vec3Len + _spt;

	return;

	unguard;
}



//! 在InterPos与最终的刀光关键线之间插值。
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

		// river mod @ 2008-12-2:原数据m_ptrSwordGlossy->m_fSgLength
		// 有问题，此处长度变为实时计算
		t_fLengthSet = osn_mathFunc::float_lerp( 
			t_fLengthS,t_fLengthE,float(t_i-_is)/(_ie-_is) );

		get_sgEndPt( m_arrVec3Start[t_i],
			m_arrVec3End[t_i],t_fLengthSet );
	}

	return ;

	unguard;
}


//  
//! 根据缓冲区中的数据，重新布置刀光相关的数据.ATTENTION TO OPP: 应该有更简单的方法实现。
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
	// 如果关帧之间的矩离过长,则需要粗分任意两个渲染关键刀光线.
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

		// 如果需要插值,计算插值的数目.
		t_iDivNum = float_div( t_fKeyLength[t_i],t_fLength );

		/*
		// River added @ 2007-5-11:等待关键线的数目数据:
		osassertex( (t_idx+t_iDivNum)<MAX_RENDERSNUM,
			va( "刀光关键线数目过多<%d,%d>..\n",t_idx,t_iDivNum ) );
		*/
		// River mod @ 2007-9-17:在特殊情况下,会出现刀光关键线数目过多,此帧不画刀光
		if( (t_idx + t_iDivNum)>=MAX_RENDERSNUM )
		{
			osDebugOut( "刀光关键线数目过多<%d,%d><%f,%f>..\n",
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




//! 最终对刀光的渲染.
void osc_swordGlossyIns::final_renderSG( int _lineNum )
{
	guard;

	// 如果是indexBlend,则在渲染刀光之前需要回复渲染状态.
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

	// 每一次渲染刀光要reset前端管道的VbufferCatch，使用可以重新设置Vbuffer.
	m_ptrMidPipe->set_nullStream();

	// 设置跟当前alpha得到的颜色跟背景相加的渲染状态。
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


//! 对相关联特效的处理。
void osc_swordGlossyIns::process_sgEffect(  bool _bindPos/* = true*/  )
{
	guard;

	// river mod @ 2006-6-21:用于控制武器的特效是否出现。
	// 武器的特效跟play_swordGlossy使用同样的接口。
	if( m_iEffectId[0] == -1 )
		return;

	osQuat     t_sOsaQuat;

	// 
	// 如果是osa特效,则计算特效需要旋转的方向.
	if( (m_iEffectType[0] == 3) ||
		(m_iEffectType[1] == 3) ||
		(m_iEffectType[2] == 3)   )
	{
		osQuat      t_sYRotQuat;
		osVec3D     t_vec3From( 0.0f,1.0f,0.0f );
		osVec3D     t_vec3To;

		// 跟刀的方向相匹配的旋转.
		t_vec3To = m_arrVec3KeyE[0] - m_arrVec3KeyS[0];
		osn_mathFunc::osQuaternionAxisToAxis( &t_sOsaQuat,&t_vec3From,&t_vec3To );


		if( !float_equal( m_ptrSwordGlossy->m_fOsaRotYAngle,0.0f ) )
		{
			// 头一次自身的旋转.
			osQuaternionRotationAxis( &t_sYRotQuat,
				&t_vec3From,m_ptrSwordGlossy->m_fOsaRotYAngle );

			// 两个旋转叠加.
			osQuaternionMultiply( &t_sOsaQuat,&t_sYRotQuat,&t_sOsaQuat );

		}

	}


	osc_TGManager*    t_ptrScene = (osc_TGManager*)::get_sceneMgr();
	osVec3D           t_vec3GAxis,t_vec3EffectPos;
	osassert( t_ptrScene );



	for( int t_i=0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		// River @ 2009-12-10:
		// 如果此位置没有特效，进入下一个
		if( m_iEffectId[t_i] < 0 )
			continue;

		// 
		// River @ 2010-3-2: 每一个刀光的特效对应一个位置信息
		// 处理得到刀光相关联特效的位置。ATTENTION: 在何处设置这些特效的位置?
		float t_fPos = m_ptrSwordGlossy->m_fEffectPos;
		if( t_i == 1 ) t_fPos = this->m_ptrSwordGlossy->m_fAddEffectPos[0];
		if( t_i == 2 ) t_fPos = this->m_ptrSwordGlossy->m_fAddEffectPos[1];
		osVec3Lerp( &t_vec3EffectPos,&m_arrVec3KeyS[0],&m_arrVec3KeyE[0],t_fPos );
		//@{
		//! River @ 2010-2-3:用于处理场景内残留的武器特效效果
		//                   放到原点，肯定画不到
		if( !_bindPos )
			t_vec3EffectPos = osVec3D( 0.0f,0.0f,0.0f );
		//@}

		switch( m_iEffectType[t_i] )
		{
		case 1:
			t_ptrScene->set_billBoardPos( m_iEffectId[t_i],t_vec3EffectPos );
			//@{
			// River added @ 2006-6-30:加入沿武器轴的公告板
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

			//! River @ 2010-6-9: 确保界面上人物粒子特效渲染正确
			t_ptrScene->update_parRenderState( m_iEffectId[t_i] );
			
			// River added @ 2006-7-1:加入沿武器发散的粒子效果
			osVec3Lerp( &t_vec3GAxis,&m_arrVec3KeyS[0],&m_arrVec3KeyE[0],t_fPos );
			t_ptrScene->set_particleLineSeg( m_iEffectId[t_i],t_vec3GAxis,m_arrVec3KeyE[0] );

			break;
			// 对于osa特效,设置osa特效的旋转方向.
		case 3:
			{			
				t_ptrScene->set_aniMeshPos( m_iEffectId[t_i],t_vec3EffectPos );


				osMatrix    t_mat,t_scaleMat;
				osMatrixRotationQuaternion( &t_mat,&t_sOsaQuat );

				// River added @ 2010-1-7:处理相应的缩放数据
				float   t_fScale = m_fScaleWeaponEffect * 
					         m_ptrSwordGlossy->m_fScaleVal[t_i];

				// 加入此武器特效的缩放效果
				if( !float_equal( t_fScale,1.0f ) )		
				{
					osMatrixScaling( &t_scaleMat,t_fScale,t_fScale,t_fScale );
					t_mat = t_scaleMat * t_mat;
				}

				t_ptrScene->set_aniMeshTrans( m_iEffectId[t_i],t_mat );
				// 即时更新特效的位置和旋转信息
				t_ptrScene->update_osaPosNRot( m_iEffectId[t_i] );

				//! 确保界面上人物特效渲染正确
				t_ptrScene->update_osaRenderState( m_iEffectId[t_i] );
				break;
			}
		default:
			break;
		}
	}


	unguard;
}

//! 添加刀光的关键帧。
void osc_swordGlossyIns::add_swordGlossyKey( osMatrix& _rotMat )
{
	guard;

	
	osassert( m_iGlossyPos >= 0 );

	// 
	// 算出刀光的开始和结束位置,并加入了刀光的关键帧队列中
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



//! 对刀光进行渲染
bool osc_swordGlossyIns::draw_swordGlossy( osVec3D& _spos,
			bool _glossyDraw,bool _ds/* = true*/,bool _bindEffPos/* = false*/ )
{
	guard;
	       
	osassert( m_ptrSwordGlossy );

	// 
	// 此变量传给外部，用于在刀上或是手上放置特效。头一个位置才是刀所在位置？
	osVec3Lerp( &_spos,&m_arrVec3KeyS[0],
		&m_arrVec3KeyE[0],0.5f );


	//  
	//! River added @ 2011-2-25:多线程调入刀光相关的数据,减小主线程的压力
	if( !m_bReadyToDraw )
	{
		m_iGlossyPos = 0;
		return true;
	}

	// 处理刀光附加的特效
	if( m_iEffectId >= 0 )
		process_sgEffect( _bindEffPos );


	// 
	// 如果没有定义刀光编辑器用，则正常处理。
# if __GLOSSY_EDITOR__

# else

	// 如果不需要画刀光，也要返回
	if( (m_iPRSword == -1)||(!_ds)||(!_glossyDraw) )
	{
		// River added @ 2005-7-18: 如果不加此句，则武器跟随的效果不能紧跟武器运动。　
		// 重设当前的刀光位置为0
		m_iGlossyPos = 0;

		return true;
	}
# endif 



	// 
	// River @ 2005-6-2:
	// 如果刀光的颜色alpha值为空，则此为特殊的刀光，不需要渲染。
	// 比如此时计算的刀光数据仅仅是为了计算武器附加特效的位置.
	if( (m_dwGlossyColor & 0xff000000) !=0 )
	{

		// 对顶点的数据进行插值.
		if( !recal_sgData() )
		{
			m_iGlossyPos = 0;
			return true;
		}


		// 只有一条线不能画三角形。
		if( m_iGRNum == 0  )
			return true;

		// River mod @ 2011-3-23:使用客户端暂不出错.
		/*
		osassertex( m_iShaderId >= 0,
			va( "Cur SGI is<0x%x>...\n",DWORD(this) ) );
		*/
		if( m_iShaderId < 0 )
		{
			// 重设当前的刀光位置为0.
			m_iGlossyPos = 0;
			return true;
		}


		// 填充渲染刀光的顶点数据,索引数据,并渲染.
		fill_vertexBufferData( m_verSG );

		m_sPolygon.m_iShaderId = m_iShaderId;

		final_renderSG( m_iGRNum );

	}

	// 重设当前的刀光位置为0.
	m_iGlossyPos = 0;

	return true;

	unguard;
}




# if __GLOSSY_EDITOR__
//! 设置当前glossy对应的纹理。
void osc_swordGlossyIns::set_glossyTex( const char* _tex )
{
	guard;

	osassert( _tex&&(_tex[0]) );
	osassertex( strlen( _tex )<32,"刀光纹理对应的名字长度必须小于32个字节.." );

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

//! 得到附加的addPos.
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


//! 得到特效相关的信息.
void osc_swordGlossyIns::get_sgEffect( float& _pos,I_weaponEffect& _effFName )
{
	osassert( m_ptrSwordGlossy );

	_pos = m_ptrSwordGlossy->m_fEffectPos;

	_effFName = m_ptrSwordGlossy->m_sEffectName[0];

	return;
}

//! 设置刀光拖尾的移动矩离
void osc_swordGlossyIns::set_sgMoveLength( int _len )
{
	if( _len < MIN_SWORDPOS )
		m_ptrSwordGlossy->m_iGlossyMoveLength = MIN_SWORDPOS;
	else if( _len > MAX_SWORDPOS )
		m_ptrSwordGlossy->m_iGlossyMoveLength = MAX_SWORDPOS;
	else
		m_ptrSwordGlossy->m_iGlossyMoveLength = _len;

}			   


//! 设置osa特效自身的旋转.
void osc_swordGlossyIns::set_osaYRot( float _agl ) 
{ 
	guard;

	if( m_iEffectType[0] != 3 )
	{
		MessageBox( NULL,"当前的特效类型不是Osa特效..","警告",MB_OK );
		return ;
	} 

	m_ptrSwordGlossy->m_fOsaRotYAngle = _agl ; 

	unguard;
} 

//! 设置附加的特效位置
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


//! 设置特效的缩放比例。
float osc_swordGlossyIns::set_effScale( int _idx,float _scale )
{
	guard;

	float  t_fBack = 1.0f;

	// 如果缩放值大于零，则缩放此武器特效，否则，仅返回当前特效的缩放
	if( m_ptrSwordGlossy )
	{
		t_fBack = m_ptrSwordGlossy->m_fScaleVal[_idx];
		if( _scale > 0.001f )
		{
			m_ptrSwordGlossy->m_fScaleVal[_idx] = _scale;
		
			// 特效缩放改变，删除和重新创建特效
			delete_devdepRes();
			if( !create_devdepRes( m_iWeaponEffIdx ) )
				osassert( false );
		}
	}

	return t_fBack;

	unguard;
}


//! 得到使用刀光的动作索引。
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


//! 内部用函数: 角色可以使用刀光,并且不记录刀光状态．
void osc_skinMesh::playSwordGlossyInner( bool _glossyEnable,bool _weaponEffEnable )
{
	guard;

	m_bPlaySGlossy = _glossyEnable;
	m_bPlayWeaponEffect = _weaponEffEnable;

	if( _weaponEffEnable )
	{
		// River @ 2011-2-26:多线程来播放相应的数据
		os_charLoadData   t_sCharLoad;
		t_sCharLoad.m_ptrSkinPtr = this;
		t_sCharLoad.m_bPlaySwordGlossy = true;
		// 更快速的处理特效.
		set_vipMTLoad();

		osc_skinMeshMgr*   t_ptrMgr = osc_skinMeshMgr::Instance();

		m_iMT_PlayWeaponEffect ++;
		t_ptrMgr->m_sCharLoadThread.push_loadChar( &t_sCharLoad );
	}

	if( m_iMT_PlayWeaponEffect <= 0 )
	{
		// 删除武器特效相关
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

	//! River @ 2010-1-7:确保人物的武器附加特效能够正常的显示
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

//! 多线程创建刀光相关的数据.
void osc_skinMesh::playSwordGlossyThread( void )
{
	guard;

	// River mod @ 2011-3-4:如果正处于调入状态，则不处理。
	if( m_bLoadLock )
		return;

	// 
	// River @ 2011-2-25: 多线程创建刀光数据
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


//! 角色可以使用刀光.
void osc_skinMesh::playSwordGlossy( bool _glossyEnable,bool _weaponEffEnable/* = true*/ )
{
	guard;

	//如果处于刀光编辑器的模式下，则不处理这些参数
# if __GLOSSY_EDITOR__
	return;
# endif 

	m_bPlaySGlossy = _glossyEnable;
	m_bPlayWeaponEffect = _weaponEffEnable;

	//! river @ 2009-9-8:内部记录刀光状态
	m_bPlaySGlossyBack = m_bPlaySGlossy;
	m_bPlayWeaponEffectBack = m_bPlayWeaponEffect;

	// River @ 2011-2-25:如果是创建刀光相关的数据,多线程创建
	if( _weaponEffEnable || _glossyEnable )
	{
		os_charLoadData   t_sCharLoad;
		t_sCharLoad.m_ptrSkinPtr = this;
		t_sCharLoad.m_bPlaySwordGlossy = true;
		
		// 更快速的处理特效.
		set_vipMTLoad();

		osc_skinMeshMgr*   t_ptrMgr = osc_skinMeshMgr::Instance();
		m_iMT_PlayWeaponEffect ++;
		t_ptrMgr->m_sCharLoadThread.push_loadChar( &t_sCharLoad );
		
	}

	//! River @ 2010-1-7:确保人物的武器附加特效能够正常的显示
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
		// 删除武器特效相关
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
 

	// 当前刀光对应到的纹理
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

	// 如果不需要创建和播放武器特效
	if( !_effect )
		return true;


	//
	// 特效没有创建，则创建,哪一个特效索引？？？？
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
		// River @ 2010-12-24:修改代码，如果要创建的特效名字相同，则不需要删除之后再创建了。
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
				osDebugOut( "不需要处理的处理的刀光附加特效格式" );
			}
			m_iEffectId[t_i] = -1;
		}
	}



	// 
	// River added @ 2007-10-11:测试,如果已经有特效,则删除原来特效。
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
						osassertex( false,va( "公告板<%s>为单次播放的公告板.",
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

					// River @ 2010-6-8:加入更多的参数，确认界面渲染人物武器特效粒子
					m_iEffectId[t_i] = t_ptrScene->create_particle( t_c,FALSE,_topLayer );
# if __GLOSSY_EDITOR__
					if( !t_ptrScene->is_loopPlayParticle( m_iEffectId[t_i] ) )
					{
						osassertex( false,va("粒子<%s>为单次播放粒子...",
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
				osassertex( false,"不能处理的刀光附加特效格式" );
			}

			osassert( m_iEffectId[t_i] >= 0 );
			if( m_iEffectId[t_i] < 0 )
				return false;
		}
	}

	// 
	// River added @ 2011-2-25:多线程调入刀光相关数据,减小主线程压力.
	m_bReadyToDraw = true;

	return true;

	unguard;
}


//! 缩放武器附加的特效.
void osc_swordGlossyIns::scale_weaponEffect( float _rate )
{
	guard;

	m_fScaleWeaponEffect = _rate;

	unguard;
}


//! 创建和删除刀光关联特效: 
void osc_swordGlossyIns::delete_devdepRes( bool _glossy/* = true*/,bool _effect/* = true*/ )
{
	guard;

	m_bReadyToDraw = false;

	// 删除刀光对应的纹理shaderId.
	if( (m_iShaderId >= 0)&&_glossy )
	{
		g_shaderMgr->release_shader( m_iShaderId );
		m_iShaderId = -1;
	}

	// 
	// 如果不需要删除武器附加的特效
	if( !_effect )
		return;

	I_fdScene*    t_ptrScene = ::get_sceneMgr();
	
	if( NULL == t_ptrScene )
		return;

	// 
	// 删除原来特效。
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
				osDebugOut( "不需要处理的处理的刀光附加特效格式" );
			}

			m_iEffectId[t_i] = -1;
		}
	}


	unguard;
}


//! 从文件的扩展名得到特效的类型
int osc_swordGlossyIns::get_effTypeFromName( const char* _name )
{
	guard;

	s_string    t_str = _name;

	//! river @ 2009-12-10:如果为空，type为零
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



//! 切换此武器特效到指定的索引
bool osc_swordGlossyIns::change_weaponEffect( int _idx )
{
	guard;

	osassert( _idx < 4 );
	
	osassertex( m_ptrSwordGlossy,"没有初始化的武器特效...\n" );
	
# if !__GLOSSY_EDITOR__
	// River @ 2011-1-11: 编辑器内需要这样的切换。
	// River @ 2010-12-24:如果是相同的切换，没有必要再换了吧？可能产生bug,多次测试
	if( m_iWeaponEffIdx == _idx )
		return true;
# endif 

	//
	m_iWeaponEffIdx = _idx;

	if( NULL == m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[0][0] )
	{
		// 如果新切换的特效为空，则删除当前的特效，并直接返回。
		delete_devdepRes( false,true );
		return true;
	}

	// 刀光编辑器的模式下,有更严格的条件
# if __GLOSSY_EDITOR__
	for ( int t_i = 0;t_i<EFFECTNUM_PERWEAPON;t_i ++ )
	{
		if( m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i][0] )
		{
			osassertex( ::file_exist( (char*)m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ),
				va( "要切换的武器特效<%s>不存在...\n",m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[t_i] ) );
		}
	}
# endif 

	// 删除和创建新的武器特效，其它的不变
	delete_devdepRes( false,true );
	if( m_ptrSwordGlossy->m_sEffectName[_idx].m_szEffObjName[0] )
		create_devdepRes( _idx,false,true );

	return true;

	unguard;
}



// River mod @ 2010-8-11:平时可以得到weaponEffect相关的数据
# if  __GLOSSY_EDITOR__

//! 根据索引，得到当前weapon特效的文件名。
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
//! 根据索引，得到当前weapon特效的文件名。
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


//! 得到武器特效的字符串.
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
//  BodyPartMesh用到的刀光数据。
// 

# if  __GLOSSY_EDITOR__

//! 设置刀光对应的动作索引。
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
*  只显示某个身体部位的部分三角形,公用于调试目的
*
*  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
*  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
*/
int  osc_bodyPartMesh::displaytri_bysel( int _triidx[24],int _trinum )
{
	guard;

	return g_smDevdepMgr->displaytri_bysel( m_iDevdepResId,_triidx,_trinum );


	unguard;
}


// 设置刀光用到的特效。
bool osc_bodyPartMesh::set_sgEffect( float _pos,const I_weaponEffect& _effFName )
{
	guard;

	osassert( m_ptrSwordGlossy );

	m_ptrSwordGlossy->m_fEffectPos = _pos;
	m_ptrSwordGlossy->m_sEffectName[0] = _effFName;
		
	return true;

	unguard;
}




//! 存储刀光相关的数据。
void osc_bodyPartMesh::save_glossyData( FILE* _file )
{
	guard;

	osassert( _file );

	// 武器是否显示，仅仅用于定位特效的武器数据，需要这个选项。
	fwrite( &m_bDisplay,sizeof( BOOL ),1,_file );

	fwrite( &m_ptrSwordGlossy->m_vec3Start,sizeof( osVec3D ),1,_file );
	fwrite( &m_ptrSwordGlossy->m_vec3End,sizeof( osVec3D ),1,_file );
	fwrite( &m_ptrSwordGlossy->m_iActNum,sizeof( int ),1,_file );

	
	// 相关的动作索引数据
	fwrite( m_ptrSwordGlossy->m_vecActIdx,
		sizeof( int ),m_ptrSwordGlossy->m_iActNum,_file );
	
	// 每一个动作对应的刀光颜色,与纹理相乘得到最终的颜色.
	fwrite( m_ptrSwordGlossy->m_vecGlossyColor,
		sizeof( DWORD ),m_ptrSwordGlossy->m_iActNum,_file );


	//
	// 存储刀光附加的特效数据。
	fwrite( &m_ptrSwordGlossy->m_fEffectPos,sizeof( float ),1,_file );
	
	//! river @ 2010-3-6:存储新加入的特效位置。
	fwrite( this->m_ptrSwordGlossy->m_fAddEffectPos,sizeof( float ),2,_file );

	fwrite( m_ptrSwordGlossy->m_sEffectName,sizeof( I_weaponEffect ),4,_file );	

	fwrite( m_ptrSwordGlossy->m_szGlossyTex,sizeof( char ),32,_file );

	fwrite( &m_ptrSwordGlossy->m_fOsaRotYAngle,sizeof( float ),1,_file );

	fwrite( &m_ptrSwordGlossy->m_iGlossyMoveLength,sizeof( int ),1,_file );

	// RIVER @ 2006-6-21:加入对武器特效缩放的支持,版本升级为105
	// RIVER @ 2009-12-10:106版本，加入三个特效相应的缩放
	fwrite( &m_ptrSwordGlossy->m_fScaleVal[0],sizeof( float )*3,1,_file );


	unguard;
}




/** \brief
*  设置第N个武器特效文件，最多4个。
* 
*  \param int _idx 设置第几个武器特效，从零开始，最大值为3.超过3的话，内部会变为零
*  \param char* _effName 武器特效的文件名
*/
bool osc_bodyPartMesh::set_weaponEffect( int _idx,const I_weaponEffect& _effName )
{
	guard;

	// 
	//! 再存储内部的武器特效数据，临时存放数据，save_weaponFile后存储于硬盘。
	if( m_ptrSwordGlossy )
		m_ptrSwordGlossy->m_sEffectName[_idx%4] = _effName;

	return true;

	unguard;
}



//! 存储当前的bpm数据，并升级版本号，加入新的刀光数据。
bool osc_bodyPartMesh::save_weaponFile( void ) 
{
	guard;



	int       t_iSize;
	s_string   t_szName;

	char   t_ch = m_strFname[ strlen(m_strFname)- WEAPON_PARTOFFSET ];

	// 确认当前的bodyPartMesh是第7部分的身体mesh.只有身体的第二和第七部分可以有刀光。
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
	// 存储相关的刀光数据
	save_glossyData( t_file );

	fclose( t_file );

	return true;


	unguard;
}

//! 在X轴上旋转我们的刀光数据。
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

//! 当前的刀光在X轴或Y轴上移动一定的向量。
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






//! 调整每一个刀光的长短。
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


//! 设置此武器用于定位，不显示此武器。
void osc_bodyPartMesh::set_displayWeapon( bool _dis )
{
	osassert( m_ptrSwordGlossy );

	m_bDisplay = _dis;

}

//! 重设刀光的各种旋转角度，使完全回到调整前的刀光
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

	// 对应的矩阵索引。
	os_characterVer* t_ptrVer = 
		(os_characterVer*)m_ptrVerData;
	m_ptrSwordGlossy->m_iMatrixIdx = t_ptrVer->m_vecBoneIdx[0];

	unguard;
}



//! 创建初始化的Sword Glossy.
void osc_bodyPartMesh::create_defaultGlossy( void )
{
	guard;

	osVec3D   t_vec3Max,t_vec3Min,t_vec3Center;

	// 如果已经存在刀光数据，则返回。
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

	// 对应的矩阵索引。
	os_characterVer* t_ptrVer = 
		(os_characterVer*)m_ptrVerData;
	m_ptrSwordGlossy->m_iMatrixIdx = t_ptrVer->m_vecBoneIdx[0];

	return;

	unguard;
}


/** brief
*  只显示某个身体部位的部分三角形,公用于调试目的
*
*  \param int _paridx 要显示部分三角形的身体部位索引,从零开始。
*  \param int _trinum 如果想显示某个部位全部的三角形索引，则传入此值为零即可。　
*/
int osc_bpmInstance::displaytri_bysel( int _triidx[24],int _trinum )
{
	guard;

	osassert( this->m_ptrBodyPartMesh );

	return m_ptrBodyPartMesh->displaytri_bysel( _triidx,_trinum );

	unguard;
}


//! 在调入人物数据时,初始化bpsInstance相关的数据。
void osc_bpmInstance::init_swordGlossy( void )
{
	guard;

	m_ptrBodyPartMesh->create_defaultGlossy();

	// 
	// 如果有刀光相关的数据，处理刀光数据实例。
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
*  设置第N个武器特效文件，最多4个。
* 
*  \param int _idx 设置第几个武器特效，从零开始，最大值为3.超过3的话，内部会变为零
*  \param char* _effName 武器特效的文件名
*  \param WEAPON_ID _weapid 设置第七个还是第八个武器部位的特效。
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

		//! 切换效果
		m_ptrSGIns8->change_weaponEffect( _idx );

	}
	if (_weapid == WEAPON7)
	{
		if( !m_ptrWeaponBpm )
			return false;
		m_ptrWeaponBpm->set_weaponEffect( _idx,_effName );

		//! 切换效果
		m_ptrSGIns->change_weaponEffect( _idx );
	}
	return true;

	unguard;
}



//  以下函数只在编辑刀光的编辑器中使用。
/** \brief
*  把当前sm中的武器文件存入到新的文件中。
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

//! 调整当前人物手中刀光的大小。
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

//! 在X方向上旋转刀光。
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
//! 在Y方向上旋转刀光。
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


//! 在X轴或是Y轴上移动刀光。
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

//! 重设刀光的各种旋转角度，使完全回到调整前的刀光
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


//! 设置此武器用于定位，不显示此武器。
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




//! 设置刀光对应的动作索引。
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
*  得到人物的当前武器对应刀光数据。
*  
*  刀光的位置信息还需要微调。
*  \param int& _actNum 返回有多少个动作使用了刀光.
*  \param int* _actIdx 传入一个整型数组，这个数组必须使用至少12元素，传出了使用刀光的
*                      的动作索引。
*  \param char* _glossyTex 上层传入的字串，必须是一个64字节的char型数组，传回了当前
*                          武器使用的刀光纹理名字。
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




//! 对刀光数据进行处理。刀光编辑器专用函数。
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

//! river @ 2010-3-5:设置effectPos.
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


//! 设置skinMesh当前武器附加的刀光特效。
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
		osDebugOut( "设置相关的刀光特效数据..\n" );
		m_ptrWeaponBpm->set_sgEffect( _pos,_effFName );
		m_ptrSGIns->change_weaponEffect( 0 );
	}
	else if (_weapid == WEAPON8)
	{
		if( !m_ptrWeaponBpm8  )
			return ;

		osassert( m_ptrSGIns8 );

		osDebugOut( "第八部分delete devdepRes ..\n" );
		m_ptrSGIns8->delete_devdepRes(  false  );
		osDebugOut( "第八部分设置相关的刀光特效数据..\n" );
		m_ptrWeaponBpm8->set_sgEffect( _pos,_effFName );
		m_ptrSGIns8->change_weaponEffect( 0 );
	}
	unguard;
}

//! 得到附加的addPos.
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
*  得到武器附加特效相关的信息.
*
*  \param I_weaponEffect& _effFName 填充相应相应的数据，返回上层
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
*  绕武器的中心轴旋转osa特效,使用osa文件与武器的对位更加精确.
*
*  如果当前武器对应的特效类型不是osa,则此函数没有效果.
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
*  缩放武器上的特效效果
*
*  \param _scale 如果是1.0，则保持原特效大小不变，2.0则增大为原来的二倍.0.2则缩小为原来的5分之1
*                如果传入的缩放比例小于零，则内部的比例不变，返回内部当前特效的缩入比例。
*  \return float 返回了当前特效的缩放比例。
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
*  显示人物的下一个动作帧姿势。
* 
*  人物的动作必须处于suspend状态，这个函数才起作用。
*
*  \param bool _next 如果为true,则使用下帧的人物动作，如果为false,则使用上一帧的姿势。
*/
void osc_skinMesh::next_actFramePose( bool _next )
{
	guard;

	if( !m_bSuspendState )
		return;

	int            t_iMaxFrame;
	t_iMaxFrame = m_ptrBoneAni->m_vecBATable[m_iAniId].m_iNumFrame;


	m_fLastUsedTime = 0.0f;

	// 如果处于动作切换状态，设置当前的actChangeState为false.
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
*  得到某一个动作总共有多少帧。
*
*  \param _actName 默认情况下使用动作名字来得到帧的总Frame数目。
*  \param _actIdx  如果此值大于等零，则返回相应索引动作的总Frame数目
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

//! 设置人物刀光拖尾的长度。有效数据是6-12，如果大于或是小于此值，使用默认的拖尾长度
void osc_skinMesh::set_sgMoveLength( int _len )
{
	guard;

	osassert( m_ptrSGIns );

	m_ptrSGIns->set_sgMoveLength( _len );

	unguard;
}

// 得到刀光拖尾的长度数据
int osc_skinMesh::get_sgMoveLength( void )
{
	guard;

	osassert( m_ptrSGIns );
	return 	m_ptrSGIns->get_sgMoveLength();

	unguard;
}

/** \brief
*  得到身体部位的面数，如果当前身体部位不存在，返回0
*
*  \param int _idx 身体部位索引，从零开始。
*/
int osc_skinMesh::get_bodyTriNum( int _idx )
{
	guard;

	// 超出当前的身体部位数目
	if( (_idx + 1)>m_iNumParts )
		return 0;

	if( !m_vecBpmIns[_idx].m_ptrBodyPartMesh )
		return 0;

	return m_vecBpmIns[_idx].m_ptrBodyPartMesh->m_dwNumIndices/3;


	unguard;
}
# endif 





