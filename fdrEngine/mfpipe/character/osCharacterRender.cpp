///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacter.h
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     ����os�����е����ﶯ�����á�
 * 
 *  ""
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"

# include "../../Terrain/include/fdFieldMgr.h"
# include "../include/deviceInit.h"
# include "../include/frontPipe.h"
# include "../../mfpipe/include/osStateBlock.h"
# include "../../mfpipe/include/osEffect.h"
# include "../../backPipe/include/osCamera.h"
# include "../../backpipe/include/mainEngine.h"

//! ��hlsl���õ��ľ�����.
# define  HLSL_VIEWPROJMAT    "mViewProj"
# define  HLSL_FOGVECTOR      "FogVector"
# define  HLSL_PALETTEMAT     "mWorldMatrixArray"
# define  HLSL_PALETTEQUATROT "mWorldRotArray"
# define  HLSL_PALETTEVEC3OFS "mWorldOfsArray"
# define  HLSL_CURBONES       "CurNumBones"
# define  HLSL_ALPHAVAL       "m_fAlphaFactor"
# define  HLSL_AMBIENT        "MaterialAmbient"
# define  HLSL_DIFFUSE        "lightDiffuse"
# define  HLSL_TEX2           "Texture1"
# define  HLSL_TEX3           "Texture2"

//! �����������ϸ�������Ч
# define  HLSL_TEX4           "Texture3"

# define  HLSL_TEXMAT          "MatrixTex"
# define  HLSL_LIGHTDIR        "lhtDir"


//!windy add
# define HLSL_VIEWPOSTION		"ViewPostion"
# define HLSL_VIEWMATRIX		"mViewMatrix"

//! ��Ұ�������Գ��ֵ�skinMesh��Ŀ��
OSENGINE_API int MAX_INVIEWSM_NUM = 96;


//! skinManager.
static osc_skinMeshMgr*  g_skinMeshMgr = NULL;

//! �Ƿ�ǿ����ʾskinMesh�϶�Ӧ�ĵڶ�����ͼ��
OSENGINE_API BOOL   g_bForceSecEff = FALSE;


// 
//! ��Ⱦ�����õ��Ķ�������.
LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration3;


//! River @ 2010-12-28:��Ϸ����������Ļ�������С����ֵ�����㣬С��2��
//  �����Ǹ���������ֵԽ��ռ���ڴ�Խ�࣬����������ʱ����Ӳ�̵Ľ���ԽС��
OSENGINE_API float           g_fCharacterTexBufScale = 0.7f;                  

//! ��������������skinMesh����Ŀ��// ATTENTION TO FIX:
//! ������У���ø����MAX_RTSHADOWNUM��ʹ����ͬ������
//  River @ 2011-2-25:�޸�Ϊȫ�ֵı���.
OSENGINE_API int             MAX_SMINVIWE = 128;



//! ����effect Shader��constant���ݡ�
void osc_bodyFrame::set_shaderConstant( LPD3DXEFFECT _eff )
{
	guard;

	osassert( _eff );

	osVec4D  t_vec4Col;

	t_vec4Col.w = 1.0f;

	// ��ʹ��ȫ�ֹ���Ϊ������skinMesh.
	if( m_pParentSkin->is_useDlAsAmbi() )
	{
		// ���������ȫ�ֹ⣬ȫ�ֹ�ʹ�������ķ���⡣
		t_vec4Col.x = g_fDirectLR;
		t_vec4Col.y = g_fDirectLG;
		t_vec4Col.z = g_fDirectLB;

		_eff->SetVector( HLSL_AMBIENT,
			(osVec4D*)&t_vec4Col );

		// ���÷����,�����Ϊ�գ��൱����ȫʹ�û�����.
		t_vec4Col.x = 0.0f;
		t_vec4Col.y = 0.0f;
		t_vec4Col.z = 0.0f;
		_eff->SetVector( HLSL_DIFFUSE,&t_vec4Col );
	}
	else
	{
		// ���������ȫ�ֹ�ͷ���⡣

		// ���е����ﶼ�ų�����
		//
		// tzz @2010-6-21 qtt suggest
		// ���е�������ͬ���Ĺ���Ӱ��
		//
//  		if(g_ptrLightMgr->IsEnableLight() 
// 			&& !m_pParentSkin->is_excludeLight()){
// 				
// 			t_vec4Col.x = m_pParentSkin->m_sCurSkinMeshAmbi.r * g_fAmbiR + 0.5f;
//  			t_vec4Col.y = m_pParentSkin->m_sCurSkinMeshAmbi.g * g_fAmbiG + 0.5f;
//  			t_vec4Col.z = m_pParentSkin->m_sCurSkinMeshAmbi.b * g_fAmbiB + 0.5f;
//  			t_vec4Col.w = 1.0f;
//  			_eff->SetVector( HLSL_AMBIENT,&t_vec4Col );
// 
//  		}else{

			t_vec4Col.x = m_pParentSkin->m_sCurSkinMeshAmbi.r;
			t_vec4Col.y = m_pParentSkin->m_sCurSkinMeshAmbi.g;
			t_vec4Col.z = m_pParentSkin->m_sCurSkinMeshAmbi.b;
			t_vec4Col *= 0.75f;

			t_vec4Col.w = 1.0f;
			_eff->SetVector( HLSL_AMBIENT,(osVec4D*)&t_vec4Col);
//		}

		// ���÷���⡣
		t_vec4Col.x = g_fDirectLR * m_pParentSkin->m_sCurSkinMeshAmbi.r;
		t_vec4Col.y = g_fDirectLG * m_pParentSkin->m_sCurSkinMeshAmbi.g;
		t_vec4Col.z = g_fDirectLB * m_pParentSkin->m_sCurSkinMeshAmbi.b;
		t_vec4Col.w = 1.0f;
		_eff->SetVector( HLSL_DIFFUSE,&t_vec4Col );
	}
	
	// ���õƹ�ķ���
	osVec3Normalize( &g_vec3LPos,&g_vec3LPos );
	t_vec4Col.x = g_vec3LPos.x;
	t_vec4Col.y = g_vec3LPos.y;
	t_vec4Col.z = g_vec3LPos.z;
	t_vec4Col.w = 1.0f;
	_eff->SetVector( HLSL_LIGHTDIR,&t_vec4Col );

	//@{
	// River @ 2009-1-29:���ǿ��shader�������������shader.
	if( g_bForceUseCustomShader )
	{
		t_vec4Col.x = 0.1f;
		t_vec4Col.y = 0.1f;
		t_vec4Col.z = 0.1f;
		_eff->SetVector( HLSL_AMBIENT,&t_vec4Col );
		t_vec4Col.x = 124.0f/255.0f;
		t_vec4Col.y = 157.0f/255.0f;
		t_vec4Col.z = 210.0f/255.0f;
		_eff->SetVector( HLSL_DIFFUSE,&t_vec4Col );
	}
	//@}


	//
	// ���Եƹ�Ч����
	osMatrix   t_mat;
	osMatrixMultiply( &t_mat,&g_matView,&g_matProj );
	_eff->SetMatrix( HLSL_VIEWPROJMAT,&t_mat );
	osVec4D  t_vec4;
	t_vec4.x = 1.0f/(g_fFogEnd-g_fFogStart);
	t_vec4.y = 0.0f;
	t_vec4.z = -g_fFogStart;
	t_vec4.w = 1.0f;
	_eff->SetVector( HLSL_FOGVECTOR,&t_vec4 );

	unguard;
}

//! ���뵶��Ĺؼ�֡.
int osc_bodyFrame::add_swordGlossyKey( int& _addNum )
{
	guard;

	osMatrix         t_matTmp,t_matNewView;;
	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	static osMatrix   t_matArray[MAX_BONEPERBPM];

	//
	// ��Ⱦ��ǰFrame�����е�body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		if (t_ptrPM == NULL) continue;
	
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		if( !t_ptrPM->m_ptrSwordGlossy )
			continue;

		for( int t_j=0;t_j<(int)t_ptrPM->m_dwNumPalette;t_j++ )
		{
			if (t_offsetMatPtr[t_j])
			{
				osMatrixMultiply( &t_matArray[t_j],
					&t_ptrPM->m_arrBoneMatrix[t_j],t_offsetMatPtr[t_j] );
			}
			
		}

		// ATTENTION TO FIX: ����˲�λ��Ӧ���mesh,��������:
		osassertex( m_vecPtrBpmIns[t_i]->m_ptrSGIns,"���ⲿλֻ��Ӧһ��mesh..." );

		m_vecPtrBpmIns[t_i]->m_ptrSGIns->add_swordGlossyKey( 
			t_matArray[t_ptrPM->m_ptrSwordGlossy->m_iMatrixIdx] );

		// ��Ӧ���˵����,����Ҫ�ٱ����������ӽ��,����.
		//return m_vecPtrBpmIns[t_i]->m_ptrSGIns->get_sgMoveLength();
		//! river @ 2009-12-11:�����ʱ�а˸����岿λ����������������Ҫ��������.
		if( m_pParentSkin->m_iNumParts != 8 )
			return m_vecPtrBpmIns[t_i]->m_ptrSGIns->get_sgMoveLength();
		else
		{
			_addNum ++;
			if( _addNum == 2 )
				return m_vecPtrBpmIns[t_i]->m_ptrSGIns->get_sgMoveLength();
		}
	}

	//
	// �ݹ���Ⱦ��ǰ�ṹ�����ֵܽṹ���ӽ��.
	int   t_iSgMoveLen = 0;
	if( m_pSibling )
	{
		t_iSgMoveLen = m_pSibling->add_swordGlossyKey( _addNum );
		if( t_iSgMoveLen > 0 )
			return t_iSgMoveLen;

	}

	if( m_pFirstChild )
		t_iSgMoveLen = m_pFirstChild->add_swordGlossyKey( _addNum );

	// �����������,��������һ��������.
	return t_iSgMoveLen;

	unguard;
}


//! �Ե������Ⱦ��ʹ�õ�����һ��������
bool osc_bodyFrame::render_swordGlossy( LPDIRECT3DDEVICE9 _dev,bool _gdraw )
{
	guard;

	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	static osMatrix   t_matArray[30];

	//
	// ��Ⱦ��ǰFrame�����е�body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		if( !t_ptrPM->m_ptrSwordGlossy )
			continue;

		//
		// ������Ⱦʹ��һ�������ĺ�����
		// �����ǰ��bodyPartMesh��Ӧ������������Ϣ��
		// ��ȡ��ǰ������Ϣ�õ��ľ�����Ⱦ�������ݡ�
		if( t_ptrPM->m_ptrSwordGlossy )
		{
			osVec3D   t_vec3PosMid;

			//@{
			//! River @ 2010-1-3:����ȥ�������ڲ�����������ЧЧ����
			bool      t_bUseBindEffectPos = true;
			if( (!m_pParentSkin->m_bInViewLastFrame)&&(!m_pParentSkin->m_bUseInInterface) )
				t_bUseBindEffectPos = false;
			//@}

			m_vecPtrBpmIns[t_i]->m_ptrSGIns->draw_swordGlossy( 
				t_vec3PosMid,_gdraw,t_ptrPM->m_bDisplay,t_bUseBindEffectPos );
			m_pParentSkin->set_handPos( t_vec3PosMid );
		}	
	}


	// River mod @ 2009-9-16: �޸ĵݹ��㷨,�����и����ε���������ṹ. 
	if( m_pFirstChild )
	{
		m_pFirstChild->render_swordGlossy( _dev,_gdraw );
		if( m_pFirstChild->m_pSibling )
		{
			osc_bodyFrame*   t_ptrFrame = m_pFirstChild->m_pSibling;
			while( t_ptrFrame )
			{
				if( (t_ptrFrame->m_iBMNum>0) || (t_ptrFrame->m_pFirstChild) )
					t_ptrFrame->render_swordGlossy( _dev,_gdraw );
				t_ptrFrame = t_ptrFrame->m_pSibling;
			}
		}
	}


	return true;


	unguard;
}



//! TEST CODE:���ڱ����㼶.
void osc_bodyFrame::through( int& _hier,os_boneSys* _boneSys,bool _sibling/* = true*/ )
{
	guard;

	_hier ++;

	if( _sibling )
	{
		if( m_iBMNum > 0 ) 
		{
		osDebugOut( "�ֵܵ���,The <%d> �㼶,<%d>����,����Ϊ<%s> bm num<%d>....\n",
			_hier,m_dwID,_boneSys->get_boneNameFromId( m_dwID ),m_iBMNum );
		}
		else
		{
		osDebugOut( "�ֵܵ���,The <%d> �㼶,<%d>����,����Ϊ<%s> �ӹ�ID<%d>����<%s>....\n",
			_hier,m_dwID,_boneSys->get_boneNameFromId( m_dwID ),
			m_pFirstChild->m_dwID,
			_boneSys->get_boneNameFromId( m_pFirstChild->m_dwID ) );
		}
	}
	else
	{
		osDebugOut( "���ӵ���,The <%d> �㼶,<%d>����,����Ϊ<%s>....\n",
			_hier,m_dwID,_boneSys->get_boneNameFromId( m_dwID ) );
	}

	// River mod @ 2009-9-16: �޸ĵݹ��㷨,�����и����ε���������ṹ. 
	if( m_pFirstChild )
	{
		m_pFirstChild->through( _hier,_boneSys,false );
		if( m_pFirstChild->m_pSibling )
		{
			osc_bodyFrame*   t_ptrFrame = m_pFirstChild->m_pSibling;
			while( t_ptrFrame )
			{
				if( (t_ptrFrame->m_iBMNum>0) || (t_ptrFrame->m_pFirstChild) )
					t_ptrFrame->through( _hier,_boneSys,true );
				t_ptrFrame = t_ptrFrame->m_pSibling;
			}
		}
	}

	_hier --;

	unguard;
}


bool osc_bodyFrame::RenderDepth( LPDIRECT3DDEVICE9 _dev )
{
	guard;
	LPD3DXEFFECT      t_eff;
	osc_effect*       pEff = 0;
	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	os_smMeshDevdep*  t_ptrRes;
//	HRESULT          t_hr;
	//! ����palette֧����Ŀ��
	static osMatrix   t_matArray[28];
	//float            t_fAlpha = _alphaVal;

	// ��Ⱦ��ǰFrame�����е�body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		if( !t_ptrPM->m_bDisplay )
			continue;

	
		t_ptrRes = g_smDevdepMgr->get_smMesh( t_ptrPM->get_devdepResId() );
		osassert( t_ptrRes->get_ref()>0 );


		osassert( t_ptrPM->m_dwNumPalette <= 28 );
		for( int t_j=0;t_j<(int)t_ptrPM->m_dwNumPalette;t_j++ )
		{
			osMatrixMultiply( &t_matArray[t_j],
				&t_ptrPM->m_arrBoneMatrix[t_j],t_offsetMatPtr[t_j] );
		}

		static int CharacterDepthShaderId = g_shaderMgr->add_shader( 
			"CharacterDepth", osc_engine::get_shaderFile( "character" ).c_str() );
	
		pEff = g_shaderMgr->get_effect( CharacterDepthShaderId );
		osassert( pEff );
		g_shaderMgr->set_EffectInfo( CharacterDepthShaderId );
		t_eff = pEff->m_peffect;
		osassert( t_eff );

		//! ����ȫ�ֵ�shader Constant.
		//set_shaderConstant( t_eff );
		osMatrix   t_mat;
		osMatrixMultiply( &t_mat,&g_matView,&g_matProj );
		t_eff->SetMatrix( HLSL_VIEWPROJMAT,&t_mat );


		//
		// ����effect��shader�õ��ľ���	
		t_eff->SetMatrixArray( HLSL_PALETTEMAT, t_matArray,t_ptrPM->m_dwNumPalette );

		
		//
		// ��ǰ�˹ܵ�ȥ��Ⱦ�����õ����豸������ݡ�
		// TEST CODE:
		//@{
		if( !t_ptrRes->m_pBuf )
		{		
			osDebugOut( "The vernum<%d>,versize<%d>..\n",
				t_ptrRes->m_iVerNum,t_ptrRes->m_iVerSize );

			if( t_ptrRes->m_strMFname[0] )
				osDebugOut( "The filename <%s>..\n",t_ptrRes->m_strMFname );

			if( t_ptrRes->m_bUsed )
				osDebugOut( "����ʹ����...\n" );
			else
				osDebugOut( "����ʹ����....\n" );

		    //osassert( t_ptrRes->m_pBuf );
			// River mod 2010-10-4:ȷ�ϴ�ʱ�ĳ�����.
			(*(char*)0) = NULL;
		}
		//@} 

		g_frender->prepare_andrender( t_ptrRes->m_pBuf,t_ptrRes->m_pIdxBuf,
			pEff,D3DPT_TRIANGLELIST,t_ptrRes->m_iVerSize,0,0,t_ptrRes->m_iIdxNum/3,
			t_ptrRes->m_iVerNum,true );

	}

	// River mod @ 2009-9-16: �޸ĵݹ��㷨,�����и����ε���������ṹ. 
	if( m_pFirstChild )
	{
		m_pFirstChild->RenderDepth( _dev);
		if( m_pFirstChild->m_pSibling )
		{
			osc_bodyFrame*   t_ptrFrame = m_pFirstChild->m_pSibling;
			while( t_ptrFrame )
			{
				if( (t_ptrFrame->m_iBMNum>0) || (t_ptrFrame->m_pFirstChild) )
					t_ptrFrame->RenderDepth( _dev );
				t_ptrFrame = t_ptrFrame->m_pSibling;
			}
		}
	}


	return true;

	unguard;
}
/** \brief
 *  ������ȾskinMesh�ĺ�����
 *
 *  ����������У���������ǰframe��Ӧ��mesh��Ϊ�գ���Ⱦ��ǰframe��Ӧ��mesh.
 *  
 *  \param _alpha �Ƿ��ǻ�͸����alphaƬ.
 *  \param _fade  �Ƿ��ڵ��뵭��״̬����Ⱦ������״̬�£���alpha�����Ҫ��
 *                alpha���һ���õ���Ⱦ��
 */
bool osc_bodyFrame::Render( LPDIRECT3DDEVICE9 _dev,
						   bool _alpha,bool _fade/* = false*/,float _alphaVal/* = 1.0f*/ )
{
	guard;

	LPD3DXEFFECT      t_eff;
	osc_effect*       pEff = 0;
	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	os_smMeshDevdep*  t_ptrRes;
	HRESULT          t_hr;
	//! ����palette֧����Ŀ��
	static osMatrix   t_matArray[28];
	float             t_fAlpha = _alphaVal;

	//
	// ��Ⱦ��ǰFrame�����е�body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		// River @ 2010-7-1:������������.
		if( m_vecPtrBpmIns[t_i]->get_hideState() )
			continue;

		if( !t_ptrPM->m_bDisplay )
			continue;

		//
		// �����ǰ��Ⱦ������alphaͨ����pm.
		if( _alpha )
		{
			// Alpha��Ⱦʱ�������fade״̬�£���û��alphaͨ���Ľ��ҲҪ��Ⱦ��
			if( (!_fade)
				&&(!m_vecPtrBpmIns[t_i]->has_alphaChannel()) )
				continue;
		}
		if( !_alpha )
		{
			if( m_vecPtrBpmIns[t_i]->has_alphaChannel() )
				continue;
		}

		t_ptrRes = g_smDevdepMgr->get_smMesh( t_ptrPM->get_devdepResId() );
		// River @ 2010-10-9:��ƽ���Ĵ�����ʽ��
		//osassert( t_ptrRes->get_ref()>0 );
		if( t_ptrRes->get_ref() <= 0 )
			continue;


		osassert( t_ptrPM->m_dwNumPalette <= 28 );
		for( int t_j=0;t_j<(int)t_ptrPM->m_dwNumPalette;t_j++ )
		{
			osMatrixMultiply( &t_matArray[t_j],
				&t_ptrPM->m_arrBoneMatrix[t_j],t_offsetMatPtr[t_j] );
		}

		// 
		// �����ǰframe��Ӧ��mesh��Ҫ��Ⱦ��Ӱ:
		osc_bpmShadow*  t_ptrSha;
		t_ptrSha = m_vecPtrBpmIns[t_i]->get_shadowPtr();
		if( t_ptrSha )
		{
			if( !t_ptrSha->is_updated() )
			{
				t_ptrSha->update_shadow( 
					t_matArray,t_ptrPM->m_dwNumPalette );

				continue;
			}
		}

		//
		// �����ǰ��skin������d3dIndexed,���ò�ͬ����Ⱦ����.
		if( (g_eSkinMethod == OSE_D3DINDEXED)&&
			(t_ptrPM->m_bDisplay) )
		{
			os_rsBodyFrame bf;
			osc_d3dManager*   t_ptrDevmgr;
			t_ptrDevmgr = (osc_d3dManager*)::get_deviceManagerPtr();
			bf.m_pd3dDevice = t_ptrDevmgr->get_d3ddevice();
			bf.m_bAlpha = _alpha;
			bf.m_ptrPMIns = m_vecPtrBpmIns[t_i];
			bf.m_ptrPM = t_ptrPM;
			bf.m_ptrRes = t_ptrRes;
			bf.m_ptrBoneMatrix = t_matArray;
			bf.m_fAlphaVal = t_fAlpha;

			RenderIndexBlend( bf );

			continue;
		}

		// River mod @ 2010-1-29: ����ǿ�Ƶ�Effect��Ⱦ������ǽ���������Ⱦ
		if( !g_bForceUseCustomShader )
		{
			pEff = g_shaderMgr->get_effect( m_vecPtrBpmIns[t_i]->get_shaderId() );
			g_shaderMgr->set_EffectInfo( m_vecPtrBpmIns[t_i]->get_shaderId() );
		}
		else
			pEff = g_shaderMgr->get_effect( g_iForceCustomShaderIdx );

		osassert( pEff );
		t_eff = pEff->m_peffect;
		osassert( t_eff );

		t_eff->SetFloat( HLSL_ALPHAVAL,t_fAlpha );

		// River @ 2010-1-29: ����ǿ��effect��Ⱦ�󣬲���Ҫ���õ��Դ
		if(/*!m_pParentSkin->is_excludeLight() && */  // tzz @ 2010-6-18 qtt ˵��Ҫ�����з�����Ӱ��,��ô���ԴҲ��Ҫ������ϣ�����ע�͵����
			 g_ptrCamera 
			&& (!g_bForceUseCustomShader) 
			&& g_ptrLightMgr->IsEnableLight())
		{
			g_ptrLightMgr->EffectSkinMesh(g_ptrCamera->get_camFocus(),28.0f,m_pParentSkin->get_smPos(),t_eff);
		}
		else
		{
			if( !g_bForceUseCustomShader )
				g_ptrLightMgr->UnEffectSkinMesh(t_eff);
		}

		//! ����ȫ�ֵ�shader Constant.
		set_shaderConstant( t_eff );


		//
		// ����effect��shader�õ��ľ���	
		t_eff->SetMatrixArray( HLSL_PALETTEMAT, t_matArray,t_ptrPM->m_dwNumPalette );
				
		// 
		//! River @ 2010-1-29:ǿ��shader����������ǽ�����Ⱦ
		if( (m_vecPtrBpmIns[t_i]->has_secondEffect())&&(!g_bForceUseCustomShader) )
		{
			D3DXVECTOR4 pos(g_vec3ViewPos.x,g_vec3ViewPos.y,g_vec3ViewPos.z,1);

			t_eff->SetVector(HLSL_VIEWPOSTION,&pos);
			t_eff->SetMatrix(HLSL_VIEWMATRIX,&g_matView);

			// ����ڶ����о���Ļ� 
			// River @ 2010-2-25:�����Է�������Ĵ���
			// River @ 2010-3-13:���������Ч������
			if( m_vecPtrBpmIns[t_i]->is_rottexEffect()   ||
				m_vecPtrBpmIns[t_i]->is_shineTexEffect() ||
				m_vecPtrBpmIns[t_i]->is_rotEnvEffect()   ||
				m_vecPtrBpmIns[t_i]->is_envShineEffect() ||
				m_vecPtrBpmIns[t_i]->is_rotShineEffect()   )
				t_eff->SetMatrix( HLSL_TEXMAT,m_vecPtrBpmIns[t_i]->get_texTransMat() );

			//  
			//! ���õڶ������......
			t_hr = t_eff->SetTexture( HLSL_TEX2,
				g_shaderMgr->get_texturePtr( m_vecPtrBpmIns[t_i]->get_secTexid() ) );
			osassertex( (!FAILED( t_hr )),(char*)osn_mathFunc::get_errorStr(t_hr) );
			
			//!���õ������������ڿ��Ƶڶ����ALPHA��
			int texid = m_vecPtrBpmIns[t_i]->get_ThdTexid();
			if (texid != -1){
				t_hr = t_eff->SetTexture( HLSL_TEX3,g_shaderMgr->get_texturePtr( texid));
			}
			osassertex( (!FAILED( t_hr )),(char*)osn_mathFunc::get_errorStr(t_hr) );

			//�� River @ 2010-3-11:����е��Ĳ��������õ��Ĳ�����
			texid = m_vecPtrBpmIns[t_i]->get_fourthTexid();
			if (texid != -1){
				t_hr = t_eff->SetTexture( HLSL_TEX4,g_shaderMgr->get_texturePtr( texid));
			}
			// River @ 2011-1-21:�˴����ܻ����Invalid call���������ʹ�ô�assert,�ٿ������
			//osassertex( (!FAILED( t_hr )),(char*)osn_mathFunc::get_errorStr(t_hr) );

		}

		//
		// ��ǰ�˹ܵ�ȥ��Ⱦ�����õ����豸������ݡ�
		// TEST CODE:
		//@{
		if( (!t_ptrRes->m_pBuf)||(!t_ptrRes->m_pIdxBuf ) )
		{		
			osDebugOut( "The vernum<%d>,versize<%d>..\n",
				t_ptrRes->m_iVerNum,t_ptrRes->m_iVerSize );

			if( t_ptrRes->m_strMFname[0] )
				osDebugOut( "The filename <%s>..\n",t_ptrRes->m_strMFname );

			if( t_ptrRes->m_bUsed )
				osDebugOut( "����ʹ����...\n" );
			else
				osDebugOut( "����ʹ����....\n" );

			//osassert( t_ptrRes->m_pBuf );
			// River mod @ 2011-6-15:
			// ����������ݴ���ֱ�ӷ��ز���Ⱦ�������Ⱦ�����⣬��ҿ����Լ��˳���Ϸ���ٽ��롣
			return true;

		}
		//@} 		
		g_frender->prepare_andrender( t_ptrRes->m_pBuf,t_ptrRes->m_pIdxBuf,
			pEff,D3DPT_TRIANGLELIST,t_ptrRes->m_iVerSize,0,0,t_ptrRes->m_iIdxNum/3,
			t_ptrRes->m_iVerNum,true );

	}

	// River mod @ 2009-9-16: �޸ĵݹ��㷨,�����и����ε���������ṹ. 
	if( m_pFirstChild )
	{
		m_pFirstChild->Render( _dev,_alpha,_fade,_alphaVal );
		if( m_pFirstChild->m_pSibling )
		{
			osc_bodyFrame*   t_ptrFrame = m_pFirstChild->m_pSibling;
			while( t_ptrFrame )
			{
				if( (t_ptrFrame->m_iBMNum>0) || (t_ptrFrame->m_pFirstChild) )
					t_ptrFrame->Render( _dev,_alpha,_fade,_alphaVal );
				t_ptrFrame = t_ptrFrame->m_pSibling;
			}
		}
	}

	return true;
	unguard;
}



/** \brief
*  ʹ��d3dIndexedBlend��ʱ��,ʹ�����������skinMesh������Ⱦ.
*
*  �Ժ��������������Ⱦ��ʽ,�Ը��Ӻ��������Ӳ������.
*  
*  ATTENTION TO OPP: ʹ��stateBlock.
*/
bool osc_bodyFrame::RenderIndexBlend( os_rsBodyFrame& _rbf )
{
	guard;

	int         t_i;
	osMatrix     matTemp;


	for ( t_i=0; t_i<int(_rbf.m_ptrPM->m_dwNumPalette); t_i++ )
 		_rbf.m_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(t_i), &_rbf.m_ptrBoneMatrix[t_i] );

	osassert(_rbf.m_ptrPMIns->m_ptrBpmTexture );
	_rbf.m_pd3dDevice->SetTexture ( 0, _rbf.m_ptrPMIns->m_ptrBpmTexture );
	

	_rbf.m_pd3dDevice->SetStreamSource( 0,_rbf.m_ptrRes->m_pBuf,0,_rbf.m_ptrRes->m_iVerSize);
	_rbf.m_pd3dDevice->SetIndices( _rbf.m_ptrRes->m_pIdxBuf );
	_rbf.m_pd3dDevice->SetFVF( IDXBLEND_FVF );

	D3DMATERIAL9   t_sMaterial;
	memset( &t_sMaterial,0,sizeof( D3DMATERIAL9 ) );
	t_sMaterial.Diffuse.r = 1.0f;
	t_sMaterial.Diffuse.g = 1.0f;
	t_sMaterial.Diffuse.b = 1.0f;
	t_sMaterial.Diffuse.a = _rbf.m_fAlphaVal;

	t_sMaterial.Ambient.r = 1.0f;
	t_sMaterial.Ambient.g = 1.0f;
	t_sMaterial.Ambient.b = 1.0f;
	t_sMaterial.Ambient.a = _rbf.m_fAlphaVal;

	_rbf.m_pd3dDevice->SetMaterial( &t_sMaterial );

	_rbf.m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,0,0,
			                _rbf.m_ptrRes->m_iVerNum,0,_rbf.m_ptrRes->m_iIdxNum/3	);


	return true;


	unguard;
}


//! River @ 2010-7-1: �����Ƿ�����������������أ�����Ⱦ��������
void osc_skinMesh::hide_weapon( bool _hide/* = true*/ )
{
	guard;

	bool  t_bUnHide = false;

	for( int t_i = 6;t_i < this->m_iNumParts;t_i ++ )
	{
		// ��������������ǲ���Ҫÿһ֡���ڲ�������
		if( (m_vecBpmIns[t_i].m_bHide == true) && 
			( _hide == false ) )
			t_bUnHide = true;

		m_vecBpmIns[t_i].m_bHide = _hide;
	}

	
	// River @ 2010-7-1:�˴�Ӧ�ü���������Ч�����أ���ȷ������û��������ټ���������Ч���.
	if( m_iNumParts > 6 )
	{
		if( _hide )
			playSwordGlossyInner( false,false );
		else
		{
			if( t_bUnHide )
				playSwordGlossyInner( m_bPlaySGlossyBack,m_bPlayWeaponEffectBack );
		}
	}

	unguard;
}



//! ����fade״̬��
void osc_skinMesh::set_fadeState( bool _fadeS,float _alphaV/* = 1.0f*/ )
{
	guard;


	// ������ﴦ������״̬��������fadeState����Ч��
	if( m_bHideSm )
	{
		//osDebugOut( "���������״̬�£������ٶ��������fade����....\n" );
		return;
	}

	// ����ϴ�����fade״̬Ϊfalse,����ǰ��fade״̬ҲΪfalse,�����κεĴ���
	if( (!m_bFadeState) && (!_fadeS ) )
		return;

	bool   t_bFadeSBack = m_bFadeState;
	

	m_bFadeState = _fadeS;
	m_fAlphaVal  = _alphaV;

	// 
	// River @ 2009-5-8:ȥ������fadeʱ,��Ӱ��͸����.
	// ������Ӱ��͸����
	if( _fadeS && g_bShadowFade )
		osc_bpmShadow::reset_shadowColor( _alphaV );



	//  
	//  River added @ 2009--3-13:
	//! ���fadeStateΪ�棬�Ҳ��ŵ�����Ч���ݲ����š�
	// �����Ҫfade,����Ҫɾ��������Ч,�����Ҫ��ʾ,����Ҫ����������Ч
	if( (!t_bFadeSBack) && m_bFadeState )
	{
		// River added @ 2010-8-18:��С���뿪��.
		if( m_iNumParts>6 )
			playSwordGlossyInner( false,false );
		if (mEffectId!=-1)
		{
			get_sceneMgr()->delete_effect(mEffectId);
			mEffectId = -1;
		}
	}
	// ������ʾ�����״̬
	else if( !m_bFadeState ) 
	{
		// River added @ 2010-8-18:��С���뿪��.
		if( m_iNumParts>6 )
			playSwordGlossyInner( m_bPlaySGlossyBack,m_bPlayWeaponEffectBack );

		if (mszSpeFileName.size()>0&&mEffectId==-1)
		{
			mEffectId = get_sceneMgr()->create_effect(
				mszSpeFileName.c_str(),m_vec3SMPos+mEffectOffset,m_fRotAgl);
		}
	}



	unguard;
}


/** \brief
*  ��Ⱦ��ǰ�����ĵ���
*
*  ��ʱ����ǰ�ƣ��������ǰ��֡�����λ�á��������Ӧ��Ҳ����ʵ�ֳ���������Ч������
*  �˵Ĳ�Ӱ�ȵȣ��Ժ���չ�������㷨�ģ���ռ�ô�����cpuʱ�䣬���ڵ����õĻ���,��Ҫ��
*  �ϲ�رյ�����Ч����صļ��㡣
*
*  1: �Ȱѵ�ǰ֡�����λ�ü��뵽���������.
*  2: ����ʱ�䣬���������ƣ���������������Ĺؼ�λ�á�����Щλ�����μ��뵽
*     ����������С�
*  3: ʹ�õ���������е�λ�ã����в�ֵ���õ���Բ���ĵ���
*/
bool osc_skinMesh::render_smSwordGlossy( LPDIRECT3DDEVICE9 _dev )
{
	guard;

	osassert( _dev );

	float    t_fMoveBackTime;
	int      t_i;
	int      t_iSGKeyNum = 0;

	//  �ȼ��뵱ǰ�����λ��:
	int   t_iTmp = 0;
	t_iSGKeyNum = m_pRoot->add_swordGlossyKey( t_iTmp );

	//  River mod @ 2007-9-29: ������ص�sgKeyNum����,����Ҫ��Ⱦ,����
	//! River mod @ 2010-6-5:  ����sgKeyNumΪ�������£��������������Ч��������Ⱦ��
	if( (t_iSGKeyNum == 0)&&(!m_bPlayWeaponEffect) )
		return false;

	//osassert( (t_iSGKeyNum>=MIN_SWORDPOS)&&(t_iSGKeyNum<=MAX_SWORDPOS) );

	//
	// ��ֻ��ʾ���⸽����Ч�������,ֻ��Ҫ�ƽ�һ������λ��.
	bool t_part7MultiPos = false,t_part8MultiPos = false;

	if (m_ptrSGBpmIns){
		t_part7MultiPos = m_ptrSGBpmIns->m_ptrSGIns->need_pushMultiSPos();
	}
	if (m_ptrSGBpmIns8){
		t_part8MultiPos = m_ptrSGBpmIns8->m_ptrSGIns->need_pushMultiSPos();
	}

# if __GLOSSY_EDITOR__

# else
	//! �����仯״̬ʱ�򵶹���ɫΪ��ʱ��Ҳ����Ҫ���µĴ���
	if( m_bPlaySGlossy && (m_dwFinalGlossyColor!=0) && 
		(t_part7MultiPos||t_part8MultiPos) && (!m_bChangeActState) )
# endif 
	{

		// ���κ��Ƶ����λ��.
		m_iNegAniIndex = m_wAniIndex;
		m_fNegLastUsedTime = m_fLastUsedTime;

		if( m_fNegLastUsedTime > m_fTPF/2.0f )
			t_fMoveBackTime = m_fNegLastUsedTime - m_fTPF/2.0f;
		else
			t_fMoveBackTime = m_fNegLastUsedTime/2.0f; 

		negative_frameMoveAction( -t_fMoveBackTime );
		t_iTmp = 0;
		m_pRoot->add_swordGlossyKey( t_iTmp );

		get_keyFramePose();
		t_iTmp = 0;
		m_pRoot->add_swordGlossyKey( t_iTmp );

		// ǰ���Ѿ������������ؼ�֡.
		t_i = 3;

		// ����ѭ��ֱ������������Ҫ�����ĵ����ؼ�֡
		while( 1 )
		{
			get_keyFramePose();
			t_iTmp = 0;
			m_pRoot->add_swordGlossyKey( t_iTmp );
			t_i ++;
			if( t_i == t_iSGKeyNum )
				break;

			// �õ���ǰ�Ĺؼ�֡����֮�Ϲؼ�֡��֡�м��ֵ��
			m_fNegLastUsedTime = 0.0f;
			negative_frameMoveAction( -m_fTPF/2.0f );
			t_iTmp = 0;
			m_pRoot->add_swordGlossyKey( t_iTmp );
			t_i ++;
			if( t_i == t_iSGKeyNum )
				break;

			if( m_iNegAniIndex < 0 )
			{
				os_boneAction*   t_ptrBA;
				t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];	
				m_iNegAniIndex = t_ptrBA->m_iNumFrame-1;
			}

		}


	}


	// ���ⲻдzbuffer.
	_dev->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
	m_pRoot->render_swordGlossy( _dev,m_bPlaySGlossy );

	return true;

	unguard;
}


/** \brief
*  ����Ļһ����ά��������Ⱦ��ǰ������,�����Ⱦʹ��OrthogonalͶӰ����
*
*  \param _rect  Ҫ���ĸ���Ļ��������Ⱦ���
*  \param _zoom  ���ڿ������������Զ��������,�ڲ��������Χ�е����ĵ����osVec3D( 0.0f,0.0f,0.0f)��
*                λ���ϣ��������osVec3D( 0.0f,0.0f,-_zoom)��λ���ϡ��ϲ���Ե�����һ�����ʵ�ֵ����
*                ʵʱ����Ⱦ��һֱʹ�����ֵ��
*  \param float _rotAgl ���ڶ�ά�������������ת����
*  \param _newChar �Ƿ����´��������������´���������ӿڣ����������Ķ�������ά�����ڵ�����û��
*                  �κεĹ�ϵ
*	\param _camera ������������Ч���Ͳ�����ȫ�ֵ����ָ�뾪����Ⱦ
*/
void osc_skinMesh::render_charInScrRect( RECT& _rect,float _rotAgl,
							float _zoom,BOOL _newChar/* = FALSE*/,I_camera* _camera /* = NULL */)
{
	guard;

	osMatrix   t_sMatView = g_matView,t_sMatProj = g_matProj;
	D3DVIEWPORT9  t_viewPort;
	D3DVIEWPORT9  t_sNewViewPort;

	// ����skinMeshMgr�ڵ���Ⱦ����
	osVec3D  t_vec3BackPos = m_vec3SMPos;
	float    t_fAngleBack = this->m_fRotAgl;
	float    t_fLastAngle = m_fLastRotAgl;

	if( m_bLoadLock )
		return ;

	if(_camera){
		((osc_camera*)_camera)->get_viewport( t_viewPort );
	}else{
		g_ptrCamera->get_viewport( t_viewPort );
	}
	
	// ����viewPort.
	t_sNewViewPort.Height = _rect.bottom - _rect.top;
	t_sNewViewPort.Width = _rect.right - _rect.left;
	t_sNewViewPort.MaxZ = 1.0f;
	t_sNewViewPort.MinZ = 0.0f;
	if( (DWORD)_rect.right > t_viewPort.Width )
		t_sNewViewPort.X = t_viewPort.Width - t_sNewViewPort.Width;
	else
	{
		if( _rect.left>0 )
			t_sNewViewPort.X = _rect.left;
		else
			t_sNewViewPort.X = 0;
	}
	if( (DWORD)_rect.bottom > t_viewPort.Height )
		t_sNewViewPort.Y = t_viewPort.Height - t_sNewViewPort.Height;
	else
	{
		if( _rect.top > 0 )
			t_sNewViewPort.Y = _rect.top;
		else
			t_sNewViewPort.Y = 0;
	}
	m_pd3dDevice->SetViewport( &t_sNewViewPort );


	// 
	// ���������õ�viewPort�Ƿ񳬳����ǵ���Ļ��С����������Ⱦ����ʱ�����λ�á�
	float    t_fRightOver = 0.0f;
	float    t_fBottomOver = 0.0f;
	if( (DWORD)_rect.right > t_viewPort.Width )
	{
		t_fRightOver = float((DWORD)_rect.right - t_viewPort.Width);
		t_fRightOver = t_fRightOver / t_sNewViewPort.Width;
	}
	else
	{
		if( _rect.left < 0 )
		{
			t_fRightOver = (float)_rect.left;
			t_fRightOver = t_fRightOver / t_sNewViewPort.Width;
		}
	}
	if( (DWORD)_rect.bottom > t_viewPort.Height )
	{
		t_fBottomOver = float((DWORD)_rect.bottom - t_viewPort.Height);
		t_fBottomOver = t_fBottomOver / t_sNewViewPort.Height;
	}
	else
	{
		if( _rect.top < 0 )
		{
			t_fBottomOver = (float)_rect.top;
			t_fBottomOver = t_fBottomOver / t_sNewViewPort.Height;
		}
	}



	// 
	// ��������ĵ�ŵ�0,0,0λ����,����Ҫ��Ⱦ���ӿ��Ƿ񳬳���Ļ����΢����
	// �����ö�ά��Ⱦʱ�������ת����
	float    t_fCharHeight,t_fx,t_fz,t_fZPos;

	// River mod @ 2009-4-14:���ڴ����Χ���ڿ��еĹ���������Ĺ��
	// �����ȺͿ�Ⱥܿ�Ĺ��
	t_fZPos = 1.0f;
	t_fCharHeight = m_sBBox.get_vecmax()->y;
	t_fx = m_sBBox.get_vecmax()->x - m_sBBox.get_vecmin()->x;
	t_fz = m_sBBox.get_vecmax()->z - m_sBBox.get_vecmin()->z;
	if( t_fCharHeight<t_fx ) t_fCharHeight = t_fx;
	if( t_fCharHeight<t_fz ) t_fCharHeight = t_fz;

	float  t_fMax = m_sBBox.get_vecmax()->z;
	if( t_fMax < m_sBBox.get_vecmax()->x )
		t_fMax = m_sBBox.get_vecmax()->x;
	if( t_fMax > t_fZPos )
		t_fZPos = t_fMax + 0.3f;

	//! River mod @ 2009-5-9:��ȷ��boundingBox,������ȷ����Ⱦ��
	t_fZPos *= 2.0f;

	if( t_fCharHeight > EPSILON )
		m_vec3SMPos = osVec3D( 0.0f,-t_fCharHeight/2.0f,0.0 );
	else
	{
		m_vec3SMPos = osVec3D( 0.0f,-1.0f,0.0 );
		t_fCharHeight = 2.0f;
	}

	m_fRotAgl = _rotAgl;
	m_fLastRotAgl = _rotAgl;

	// ���ﶯ����ʱ���ֵ
	float     t_fActTime = 0.0f;
	if( _newChar )
	{
		// River @ 2009-5-4:�����ϲ��֡����һ�δ˺���
		if( m_dwLastRenderTime == 0 )
		{
			t_fActTime = sg_timer::Instance()->get_lastelatime();
			m_dwLastRenderTime = ::timeGetTime();
		}
		else
		{
			DWORD  t_dwCurTime = ::timeGetTime();
			t_fActTime = (t_dwCurTime - m_dwLastRenderTime)/1000.0f;
			m_dwLastRenderTime = t_dwCurTime;
		}
	}


	//
	// �Ӿ����Orthogonal Prjection
	float   t_fWidth = _zoom*t_fCharHeight*(4.0f/3.0f);
	float   t_fHeight = _zoom*t_fCharHeight;
	float   t_fXPos = -t_fWidth * t_fRightOver;
	float   t_fYPos = t_fHeight * t_fBottomOver;
	osMatrixLookAtLH( &g_matView,&osVec3D( t_fXPos,t_fYPos,-t_fZPos ),
		&osVec3D( t_fXPos,t_fYPos,0.0f ),&osVec3D( 0.0f,1.0f,0.0f ) );
	osMatrixOrthoLH( &g_matProj,t_fWidth,t_fHeight,0.1f,30.0f );

	frame_moveAction( t_fActTime );
	osc_skinMeshMgr::Instance()->render_character( this );

	//  
	//! River @ 2009-12-29:�˴�Ӧ����Ⱦ������osa����Ч��.2010����ǰ������ɵĹ���
	//  ���������Ҫ�����Լ������Ӻ͹����Ķ�ά���涯��Ч��
	I_fdScene*  t_ptrScene = ::get_sceneMgr();
	osc_TGManager*  t_ptrRc = (osc_TGManager*)t_ptrScene;
	t_ptrRc->render_topLayer( _camera,OS_RENDER_TOPLAYER );


	// �ظ������λ������
	m_vec3SMPos =  t_vec3BackPos;
	m_fRotAgl = t_fAngleBack;
	m_fLastRotAgl = t_fLastAngle;
 
	// ��������лظ���ʹһ��֡����������ȷ����Ⱦ��
	m_pd3dDevice->SetViewport( &t_viewPort );
	g_matView = t_sMatView;
	g_matProj = t_sMatProj;


	unguard;
}

bool osc_skinMesh::render_skinDepth(LPDIRECT3DDEVICE9 _dev)
{
	guard;
	// River @ 2009-6-7:���������Load״̬�����ء�
	if( m_bLoadLock )
		return true;

	// 
	// �����ǰ��skinMesh��������״̬����ֱ�ӷ��ء�
	if( m_bHideSm  )
		return true;

	if( !m_pRoot->RenderDepth( _dev) )
		return false;

	return true;
	unguard;
}

/** \brief
*  �Ե�ǰ��skinMesh������Ⱦ��
*
*  
*  \param ��ǰ��Ⱦ��Ҫ����ȾskinMesh�е�alpha���ֻ��Ƿ�alpha���֡�
*/
bool osc_skinMesh::render_skinMesh( LPDIRECT3DDEVICE9 _dev,bool _alpha/* = false */ )
{
	guard;

	// River @ 2009-6-7:���������Load״̬�����ء�
	if( m_bLoadLock )
		return true;
	
	// 
	// �����ǰ��skinMesh��������״̬����ֱ�ӷ��ء�
	if( m_bHideSm  )
		return true;

	// 
	// ����fade״̬ʱ��ֻ��alpha��Ⱦ������Ⱦ��
	if( !_alpha )
	{
		if( m_bFadeState )			
			return true;
	}

	// ���õ�ǰskinMesh��Ⱦ�õ��Ļ����⡣
	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		if( m_bUseDirLAsAmbient )
		{
			osColor   t_color( g_fDirectLR,g_fDirectLG,g_fDirectLB,1.0f );
			_dev->SetRenderState( D3DRS_AMBIENT,(DWORD)t_color );
			
			// ATTENTION TO OPP: ���õ�һ���ƹ�Ϊ��
			_dev->LightEnable( 0,false );
		}
		else
		{
			_dev->SetRenderState( D3DRS_AMBIENT,((DWORD)m_sCurSkinMeshAmbi) );
			// ���õ�һ���ƹ�Ϊ���������е�diuffse�ƹ⡣
			_dev->LightEnable( 0,true );
		}

	}


	//
	// ATTENTION TO FIX�����ÿ�дZbuffer������
	if( _alpha )
	{
		_dev->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );
		_dev->SetRenderState( D3DRS_ALPHAREF, GLOBAL_MIDALPHAREF);
		_dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	}

	//
	// ������������Ⱦ״̬,�������effect�ļ�,
	if( g_bCharWireFrame )
		_dev->SetRenderState( D3DRS_FILLMODE,D3DFILL_WIREFRAME );
	else
		_dev->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );

	//
	// ����Ⱦû��alpha�Ĳ��֡�
	bool t_renderSucc = m_pRoot->Render( _dev,_alpha,m_bFadeState,m_fAlphaVal );
	if( !t_renderSucc )
		return false;

	//! River @ 2009-1-29:renderBehind Wall.
	if( g_bForceUseCustomShader )
		return true;


	// 
	// ��󻭵���.
	// 
	// _alpha ��Ⱦ�����alpha����ʱ,����Ⱦ����.
	// (!m_bRenderGlossy)���������֤������һ֡��ֻ��Ⱦһ�ε���
	// m_bPlaySGlossy ��ǰ���������ڲ��ŵ����״̬.
	// 
	// River mod @ 2009-2-18:����m_bPlaySGlossy���ж�
	if( _alpha && (!m_bRenderGlossy) && (m_bPlaySGlossy||m_bPlayWeaponEffect) )
	{
		// ���ڶ����仯״̬��Ҳ����Ҫ������
		// 
		// River @ 2006-6-27:�����仯״̬��������Ч��Ҫ��Ⱦ����ʱ�Ե������⴦��????
		//                   ���������ĸ�����Ч���Խ��д���
		//if( (m_ptrSGBpmIns||m_ptrSGBpmIns8) && (!m_bChangeActState) )
		if( (m_ptrSGBpmIns||m_ptrSGBpmIns8)&&(!m_bRenderGlossy) )
		{
			render_smSwordGlossy( _dev );
			m_bRenderGlossy = true;

			//@{  River mod @ 2007-5-11:�˴���Ҫ����������Ⱦ��Declaration��������һ��������Ⱦ��
			//    ��ʱ�򣬻���ַǷ���fvf������Ǵ���
			if( g_eSkinMethod == OSE_D3DINDEXEDVS )
			{
				HRESULT  t_hr;
				t_hr = m_pd3dDevice->SetVertexDeclaration( g_pVertexDeclaration3 );
				osassert( !FAILED( t_hr ) );
			}
			//@}

		}
	}

	//
	// 
	if( _alpha )
	{
		_dev->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		_dev->SetRenderState( D3DRS_ALPHATESTENABLE,FALSE );
	}

	return true;

	unguard;

}


//! Identity Matrix;
osMatrix osc_skinMesh::m_smatIdentity;





osc_skinMeshMgr::osc_skinMeshMgr()
{
	// ������Ҫ���зŵ�һ��Ȼ��sceneSm��������еĺ���õ��ڴ�ָ�롣
	m_vecDadded.resize( MAX_SMINVIWE );	
	m_vecInViewSM.resize( MAX_SMINVIWE );
	m_vecSceneSM.resize( MAX_SCENESMNUM/2 );

	m_vecRCBehindWall.resize( MAX_BEHINDWALLCHAR );
	for( int t_i = 0;t_i<MAX_BEHINDWALLCHAR;t_i ++ )
		m_vecRCBehindWall[t_i] = NULL;

	g_skinMeshMgr = this;

	// ��ʼ��criticalSection.
	::InitializeCriticalSection( &m_sChrCreateCS );
	m_sCharLoadThread.Begin();

}

osc_skinMeshMgr::~osc_skinMeshMgr()
{

	m_sCharLoadThread.end_thread();
	m_sCharLoadThread.End();

	release_smManager( true );


	osc_skinMesh::release_postTrail();


	// �ͷŵ�ǰ�Ķ���������
	SAFE_RELEASE( g_pVertexDeclaration3 );

	// ATTENTION TO FIX: �������ٽ���û���ͷš�
	::DeleteCriticalSection( &m_sChrCreateCS );

}


//! �豸������ݵ�ָ�롣
LPDIRECT3DDEVICE9 osc_skinMeshMgr::m_pd3dDevice = NULL;

//! ÿһʱ��mgrֻ�ܴ���һ��mesh.
CRITICAL_SECTION  osc_skinMeshMgr::m_sChrCreateCS;

//! �豸������ݵ�ָ�롣
LPDIRECT3DDEVICE9 osc_skinMesh::m_pd3dDevice = NULL;


//
//  ATTENTION OPP: ֻ��Ҫ����һ��Vertex deleration �Ϳ����ˡ�
//! ��ʼ����ǰ��skinMeshManager.
bool osc_skinMeshMgr::init_smManager(  osc_middlePipe* _mp  )
{
	guard;

	HRESULT   t_hr;

	osassert( _mp );

	m_pd3dDevice = _mp->get_device();
	m_devdepResMgr.init_smDevdepMgr( m_pd3dDevice );
	m_bpmMgr.init_bmpMgr( m_pd3dDevice );

	osc_bpmShadow::init_shadowSys( _mp );

	// 
	// ����������������Ҫ��������������Զ�Ӧ�����ܲ�ͬ������ĿӰ��Ķ���.
	// ֻһ��weight�����Ρ�
	// ������weight�����Ρ��޸������յĲ��,ʹ������ֻ��������������������Ķ���.
    D3DVERTEXELEMENT9 t_decl2[] =
    {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, 
			D3DDECLUSAGE_POSITION, 0 },
		
		{  0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
		D3DDECLUSAGE_BLENDWEIGHT, 0 },
		
		{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT,
		 D3DDECLUSAGE_BLENDINDICES, 0 },
		
		{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,
		 D3DDECLUSAGE_NORMAL, 0},

        { 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,
		D3DDECLUSAGE_TEXCOORD, 0 },
     
        D3DDECL_END()
    };

    if( FAILED( t_hr = m_pd3dDevice->CreateVertexDeclaration( 
		                 t_decl2, &g_pVertexDeclaration3 ) ) )
    {
		osassertex( false,"������������ʧ��!!!\n" );
        return false;
    }

	// 
	// �õ������Ժ����й�����ʹ��skinMesh�ķ���.
	init_skinMethod();

	if( g_eSkinMethod == OSE_D3DINDEXED )
		create_IndexedBlendSB();
	create_smMgrrsSB();
	create_smMgrEndrsSB();


	// flip Determinant
	osMatrixScaling( &osc_skinMesh::m_sFDMatrix,-1.0f,-1.0f,-1.0f );

	//!  IdentityMatrix.
	osMatrixIdentity( &osc_skinMesh::m_smatIdentity );
	osc_swordGlossyIns::init_swordGlossyIns( _mp );

	// skinMesh�ھ�̬�����ĸ�ֵ��
	osc_skinMesh::m_ptrVecDadded = &m_vecDadded;
	osc_skinMesh::m_ptrSceneSM = &m_vecSceneSM;
	osc_skinMesh::m_pd3dDevice = m_pd3dDevice;

	int t_m = sizeof( os_chaSecEffect );

	//! river @ 2010-12-22:Ԥ��ȡ�洢��Ϊ��Ч��.
	osc_skinMesh::m_vecSecEffect.resize( MAX_SMINVIWE*3 );
	osc_skinMesh::m_vecSecEffPreLoad.resize( MAX_SMINVIWE*3 );


	osc_skinMesh::init_postTrail();

	//! river @ 2010-12-22:Ԥ��ȡ�洢��Ϊ��Ч��.
	//! ��ʼ��glossyIns�õ����ڴ�
	osc_bpmInstance::m_vecDGlossyIns.resize( MAX_SMINVIWE );
	osc_bpmInstance::m_vecChrRideIns.resize( MAX_SMINVIWE*2 );

	//! ���ڿ�������
	osc_bpmInstance::m_vecChrShaderBuf.resize( int(MAX_SMINVIWE*g_fCharacterTexBufScale) );

	// River mod @ 2009-11-4:��ʼ����Ӱ������ٽ���.
	::InitializeCriticalSection( &g_sShadowSec );

	// River @ 2010-12-24:����Ĭ�ϵĵ�������.
	g_shaderMgr->add_shader( "glossy",osc_engine::get_shaderFile( "billBoard" ).c_str() );

	return true;

	unguard;
}



//! �ͷŵ�ǰskinMeshManager.
void osc_skinMeshMgr::release_smManager( bool _finalRelease/* = false*/ )
{
	guard;


	osDebugOut( "�ͷ����������..\n" );

	for( int t_i=0;t_i<MAX_RTSHADOWNUM;t_i ++ )
		m_ptrShadowSkinPtr[t_i] = NULL;

	// River mod @ 2009-11-4:��ʼ����Ӱ������ٽ���.
	if( _finalRelease )
		::DeleteCriticalSection( &g_sShadowSec );

	//! ���ͷų�����ÿһ��ʹ�õ����
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;

	if( !_finalRelease )
	{
		for( t_ptrNode = m_vecDadded.begin_usedNode();
			t_ptrNode.p != NULL ;
			t_ptrNode = m_vecDadded.next_validNode(&t_ptrNode ) )
		{
			osassert( t_ptrNode.p->m_bInUse );
			t_ptrNode.p->release_character();
		}
	}
	m_vecDadded.destroy_mgr();

	if( !_finalRelease )
	{
		for( t_ptrNode = m_vecSceneSM.begin_usedNode();
			t_ptrNode.p != NULL ;
			t_ptrNode = m_vecSceneSM.next_validNode(&t_ptrNode ) )
		{
			osassert( t_ptrNode.p->m_bInUse );
			t_ptrNode.p->release_character( TRUE );
		}
	}
	m_vecSceneSM.destroy_mgr();


	// ������������˳�����ֻҪ�ͷŵ������ok,�ڴ��������.
	if( !_finalRelease )		
		return;	

	m_devdepResMgr.release_smDevdepMgr( _finalRelease );
	m_boneSysMgr.release_boneSysMgr();
	m_boneAniMgr.release_boneAniMgr();
	m_bpmMgr.release_bpmMgr();

	// 
	osc_bpmShadow::release_shadowSys( _finalRelease );



	if( _finalRelease )
	{
		if( (m_iIndexedBlendSB>=0) )
		{
			os_stateBlockMgr::Instance()->
				release_stateBlock( m_iIndexedBlendSB );
		}
		if( m_iSetsmSB>= 0 )
		{
			os_stateBlockMgr::Instance()->
				release_stateBlock( m_iSetsmSB );
		}
		if( m_iEndSetsmSB>=0 )
		{
			os_stateBlockMgr::Instance()->
				release_stateBlock( m_iEndSetsmSB );
		}
# if GALAXY_VERSION 
		if( m_iProjectShadowSB >= 0 )
		{
			os_stateBlockMgr::Instance()->
				release_stateBlock( m_iProjectShadowSB );
		}
# endif 
	}

	osDebugOut( "����������ͷ����..\n" );

	unguard;
}


/** \brief
*  �ڳ�ʼ��skinMeshMgr��ʱ��,��ʼ�����ǵ�skinMethod.
*/
void osc_skinMeshMgr::init_skinMethod( void )
{
	guard;// osc_skinMeshMgr::init_skinMethod() );

	osc_d3dManager*   t_ptrDevmgr;

	t_ptrDevmgr = (osc_d3dManager*)::get_deviceManagerPtr();
	osassert( t_ptrDevmgr );
	
	// 
	// ATTENTION TO OPP: �鿴IB��Ӳ��֧�ֶȣ�������ע�͵�Indexed Blending�Ĵ��롣
	// 
	// ���Ӳ��֧��Indexed Blend ,ʹ�ã�����ʹ�������VertexShader.
	// �����ǰ�豸Ϊ�������ת������BlendMatrix��Ŀ����Ҫ�жϡ�
	// �ϲ��Ѿ��������жϡ�
	g_eSkinMethod = OSE_D3DINDEXEDVS;

	//
	// �������ʹ��Ӳ������,��ʹ��VertexShader,������Ҳ������Ⱦ��Ӱ��
	// �����Ⱦѡ��ǿ��ʹ�õڶ�����ͼ����ʹ��HLSL
	if( (!t_ptrDevmgr->is_softwareVS())|| g_bForceSecEff )
		g_eSkinMethod = OSE_D3DINDEXEDVS;

	// ��shaderDebugģʽ�£�ʹ��VS.
	if( g_bShaderDebug )
		g_eSkinMethod = OSE_D3DINDEXEDVS;

	// TEST CODE:��������ԭ���ϲ���֧��tnt������Կ��������������й�
	// OSE_D3DINDEXED�Ĵ��������û�����壬���Խ׶��Ե�ɾ����
	//g_eSkinMethod = OSE_D3DINDEXED;

	unguard;
}


//! d3dIndexedBlend��Ⱦʱ�õ�����Ⱦ״̬�顣
int osc_skinMeshMgr::m_iIndexedBlendSB = -1;
/** \brief
*  ����indexed Blend��Ⱦʱʹ�õ���Ⱦ״̬�顣
*/
void osc_skinMeshMgr::create_IndexedBlendSB( void )
{
	guard;

	osc_d3dManager*     t_ptrDevMgr = NULL;
	t_ptrDevMgr = (osc_d3dManager*)::get_deviceManagerPtr();


	m_iIndexedBlendSB = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iIndexedBlendSB>= 0 );
	os_stateBlockMgr::Instance()->start_stateBlock();


	D3DMATERIAL9   t_sMaterial;
	memset( &t_sMaterial,0,sizeof( D3DMATERIAL9 ) );
	t_sMaterial.Diffuse.r = 1.0f;
	t_sMaterial.Diffuse.g = 1.0f;
	t_sMaterial.Diffuse.b = 1.0f;
	t_sMaterial.Diffuse.a = 1.0f;

	t_sMaterial.Ambient.r = 1.0f;
	t_sMaterial.Ambient.g = 1.0f;
	t_sMaterial.Ambient.b = 1.0f;
	t_sMaterial.Ambient.a = 1.0f;

	m_pd3dDevice->SetMaterial( &t_sMaterial );
	m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0xffffffff );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,TRUE );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR ); 
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
	m_pd3dDevice->SetFVF( IDXBLEND_FVF );

	m_pd3dDevice->SetSoftwareVertexProcessing( t_ptrDevMgr->is_softwareVS() );
	m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,D3DVBF_2WEIGHTS );
	m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_CW );
	m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iIndexedBlendSB );


	unguard;
}


//! ��ȾsmMgrǰ����Ⱦ״̬�ı䡣
int osc_skinMeshMgr::m_iSetsmSB = -1;
//! ����smMgr��Ⱦʱ��״̬�ı䡣
int osc_skinMeshMgr::m_iEndSetsmSB = -1;


//! ����smMgrRState����Ⱦ�顣
void osc_skinMeshMgr::create_smMgrrsSB( void )
{
	guard;

	HRESULT            t_hr;
	os_stateBlockMgr*  t_ptrMgr;
	osc_d3dManager*     t_ptrDevMgr = NULL;	

	t_ptrDevMgr = (osc_d3dManager*)::get_deviceManagerPtr();
	t_ptrMgr = os_stateBlockMgr::Instance();
	osassert( t_ptrMgr);
	m_iSetsmSB = t_ptrMgr->create_stateBlock();
	osassert( m_iSetsmSB>=0 );
	t_ptrMgr->start_stateBlock();

	// 
	//@{ 
	// River @ 2004-6-30:Ϊ�˽�������沿�Ľӷ�����,������������skinMesh
	//                   ʹ�ó���1.0����С��0.0��u,v����.
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );
	//@} 

	// River @ 2005-6-20:alphaTest��صĴ��룬�����͸�����ֱ���ʹ��alphaTest.
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, GLOBAL_MAXALPHAREF );


# ifdef _DEBUG
	if( g_bCharWireFrame )
		m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_WIREFRAME );
	else
		m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );
# endif 
# ifdef NDEBUG
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );
# endif 


	switch( g_eSkinMethod )
	{
		// 
		// ʹ��Vertex Shader������Ⱦ.
	case OSE_D3DINDEXEDVS:
		{
			//
			//  ����ͳһ�Ķ������������е�skinMesh��ʹ���������������ݸ�ʽ��
			t_hr = m_pd3dDevice->SetVertexDeclaration( g_pVertexDeclaration3 );
			osassert( !FAILED( t_hr ) );

			if( t_ptrDevMgr->is_softwareVS() )
			{
				t_hr = m_pd3dDevice->SetSoftwareVertexProcessing( t_ptrDevMgr->is_softwareVS() );
				osassert( !FAILED( t_hr ) );
			}

		}
		break;

		// 
		// ʹ��indexed Blend������Ⱦ.
	case OSE_D3DINDEXED:
		break;
	default:
		osassert( false );
	}


	t_ptrMgr->end_stateBlock( m_iSetsmSB );

	return;

	unguard;
}

//! ��������smMgrRState����Ⱦ�顣
void osc_skinMeshMgr::create_smMgrEndrsSB( void )
{
	guard;

	os_stateBlockMgr*  t_ptrMgr;
	osc_d3dManager*     t_ptrDevMgr = NULL;	

	t_ptrDevMgr = (osc_d3dManager*)::get_deviceManagerPtr();
	t_ptrMgr = os_stateBlockMgr::Instance();
	osassert( t_ptrMgr);
	m_iEndSetsmSB = t_ptrMgr->create_stateBlock();
	osassert( m_iEndSetsmSB >= 0 );



	t_ptrMgr->start_stateBlock();

	if( (t_ptrDevMgr->is_hardwareTL())&&
		(t_ptrDevMgr->is_softwareVS() ) )
		m_pd3dDevice->SetSoftwareVertexProcessing( FALSE );

	// ʹ��Render State Block.
	// �����������Ⱦ����ص���Ⱦ״̬��
	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,D3DVBF_DISABLE );
		m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false );
	}


	//
	// ATTENTION TO FIX: ���ʹ��Effect�ļ�������Ⱦ�˵�ǰEffect
	//                   �ļ���Pass��D3D�ظ�����ԭ������Ⱦ״̬��
	// 
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_CCW );

	//! River @ 2010-1-29:����char behindWall����Ⱦ��״̬�ظ���
	m_pd3dDevice->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );

	// ���clamp��������Ⱦ״̬.
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );

	t_ptrMgr->end_stateBlock( m_iEndSetsmSB );

	unguard;
}



/** \brief
*  ����ȾskinMeshMgr֮ǰ�����豸��ͬ,������Ⱦ״̬.
*/
void osc_skinMeshMgr::set_smMgrRState( void )
{
	guard;


	if( g_eSkinMethod == OSE_D3DINDEXEDVS )
	{
		osMatrix           t_matrix;
		osc_d3dManager*     t_ptrDevMgr = NULL;

		t_ptrDevMgr = (osc_d3dManager*)::get_deviceManagerPtr();
		m_pd3dDevice->SetTransform( D3DTS_WORLD,&osc_skinMesh::m_smatIdentity );
	}

	os_stateBlockMgr* t_ptrSBM = os_stateBlockMgr::Instance();

	switch( g_eSkinMethod )
	{
		// 
		// ʹ��Vertex Shader������Ⱦ.
	case OSE_D3DINDEXEDVS:
		break;
		// 
		// ʹ��indexed Blend������Ⱦ.
	case OSE_D3DINDEXED:
		{
			//
			// ���������Ⱦ,����ֻʹ����������Ӱ��Ķ���.
			if( (m_iIndexedBlendSB>=0)&&
				t_ptrSBM->validate_stateBlockId( m_iIndexedBlendSB ) )
			{
				t_ptrSBM->apply_stateBlock( m_iIndexedBlendSB );
			}
			else
			{
				create_IndexedBlendSB();
				t_ptrSBM->apply_stateBlock( m_iIndexedBlendSB );
			}

			//
			// ȫ�ֹ����á�
// 			m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,
// 				osc_middlePipe::m_dwAmbientColor );

		}
		break;
	default:
		osassert( false );
	}

	// ȷ����Ⱦ״̬��,��ʹ�õ��豸.
	if( (m_iSetsmSB>=0)&&
		t_ptrSBM->validate_stateBlockId( m_iSetsmSB ) )
	{
		t_ptrSBM->apply_stateBlock( m_iSetsmSB );
	}
	else
	{
		create_smMgrrsSB();
		t_ptrSBM->apply_stateBlock( m_iSetsmSB );
	}


	return;

	unguard;
}



/** \brief
*  �ڽ���skinMeshMgr��Ⱦ��ʱ��,���õ���Ⱦ״̬.
*/
void osc_skinMeshMgr::endset_smMgrRState( void )
{
	guard;

	os_stateBlockMgr*   t_ptrSBM = os_stateBlockMgr::Instance();

	if( (m_iEndSetsmSB>=0)&&
		t_ptrSBM->validate_stateBlockId( m_iEndSetsmSB ) )
	{
		t_ptrSBM->apply_stateBlock( m_iEndSetsmSB );
	}
	else
	{
		create_smMgrEndrsSB();
		t_ptrSBM->apply_stateBlock( m_iEndSetsmSB );
	}

	unguard;
}


//! �ҵ��봫��λ����Զ��character����,���ڳ������������ʱ,ֻ��ʾ����������.
int osc_skinMeshMgr::get_farthestChrIdx( osVec3D& _pos,float& _resDis )
{
	guard;

	int     t_idx = 0;
	float   t_fFarthestDis = 0.0;
	float   t_fDis;
	osVec3D t_sSmPos;

	for( int t_i=0;t_i<m_iInviewSMNum;t_i ++ )
	{
		osassert( m_vecInViewSM[t_i] );
		m_vecInViewSM[t_i]->get_smPos( t_sSmPos );
		t_sSmPos -= _pos;
		t_fDis = osVec3Length( &t_sSmPos );
		if( t_fDis > t_fFarthestDis )
		{
			t_fFarthestDis = t_fDis;
			t_idx = t_i;
		}
	}

	_resDis = t_fFarthestDis;
	return t_idx;

	unguard;
}



//! ���������м���ɼ���skinMesh��Ʒ���Ѿ�ȷ����Ҫ�ڳ�������ʾ��
void osc_skinMeshMgr::add_sceneInviewSm( osc_skinMesh* _ptr )
{
	guard;

	// 
	osassert( _ptr );

	// ��������ڿ��ӵ�skinMesh���࣬��ֱ�ӷ��ء�
	if( m_iInviewSMNum >= MAX_SMINVIWE )
	{
		// River Added @ 2007-6-20:
		// �ҵ��������������Զ��sm��Ȼ��Ƚϵ�ǰҪ�����sm������ľ��룬ʹ�ý������Զ��
		float     t_fFarthestDis,t_fDis;
		osVec3D   t_vec3CamPos,t_vec3SmPos;
		int       t_iSmIdx;


		//! River @ 2011-2-14:����������Ϊ���ģ�ȷ����Χ������.���㼴��������ġ�
		g_ptrCamera->get_camFocus( t_vec3CamPos );

		t_iSmIdx = get_farthestChrIdx( t_vec3CamPos, t_fFarthestDis );
		_ptr->get_smPos( t_vec3SmPos );
		t_vec3SmPos -= t_vec3CamPos;
		t_fDis = osVec3Length( &t_vec3SmPos );
		if( t_fDis < t_fFarthestDis )
			m_vecInViewSM[t_iSmIdx] = _ptr;
		return;
	}

	m_vecInViewSM[m_iInviewSMNum] = _ptr;
	m_iInviewSMNum ++;

	unguard;
}



/** \brief
 *  ��qsort���õĺ���ָ�룬ʹ������������Ƚ�����SkinMesh�����������Զ����
 *  
 *  ������ɺ������ڵ�sm��Ʒ�ɽ���Զ�������С�
 */
int   alphaSM_compare( const void *_a, const void *_b ) 
{
	guard;

	osc_skinMesh     *t_ptra,*t_ptrb;
	osVec3D          t_camPos,t_tmpPos;
	float            t_fDisa,t_fDisb;

	osassert( _a&&_b );

	t_ptra = (osc_skinMesh*)(*(DWORD*)_a);
	t_ptrb = (osc_skinMesh*)(*(DWORD*)_b);


	g_ptrCamera->get_curpos( &t_camPos );
	t_ptra->get_smPos( t_tmpPos );
	t_camPos -= t_tmpPos;
	t_fDisa = osVec3Length( &t_camPos );

	g_ptrCamera->get_curpos( &t_camPos );
	t_ptrb->get_smPos( t_tmpPos );
	t_camPos -= t_tmpPos;
	t_fDisb = osVec3Length( &t_camPos );

	if( t_fDisa < t_fDisb )
		return -1;
	else
		return 1;
	
	unguard;
}




/** \brief
 *  �õ��������ڵ�Character���б�
 *  
 */
void osc_skinMeshMgr::get_inViewChrList( void )
{
	guard;

	osc_TGManager*     t_ptrSceMgr;

	t_ptrSceMgr = (osc_TGManager*)::get_sceneMgr();
	os_bsphere         t_bs;
	osVec3D            t_vec3Tmp;

	t_bs.radius = 3.0f;  // ÿһģ���ڵ�����ʱ��Ӧ�ü������sphere.

	//
	// ��Ⱦ�����õ���skinMesh
	CSpaceMgr<osc_skinMesh>::NODE  t_ptrNode;

	// 
	// ATTENTION TO OPP: �ڴ˴�����ʹ�ó������³��ֵ���������е���״̬��
	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		// River mod @ 2009-6-8:Ϊ�˼��������Ĵ���	
		if( t_ptrNode.p->m_bLoadLock )
			continue;

		// River @ 2009-11-15: TEST CODE�жϳ���ԭ��.
		osassertex( t_ptrNode.p->m_bInUse,
			va( "PTR<0x%x>...\n",DWORD( t_ptrNode.p )  ) );

		//@{
		//  River @ 2010-6-3:ȷ�������ڲ�����������������ƵĲ���
		//! �����ǰ��������skin,��ֱ�Ӽ��뵽InViewScene�ڣ�ȷ�������λ�ø���
		if( t_ptrNode.p->m_HumObj )
		{
			add_sceneInviewSm( t_ptrNode.p );
			continue;
		}
		//@} 

		//! �����֡�����˴�֡������µı���������¡�
		if( t_ptrNode.p->m_bMustUpdateNextFrame )
		{
			add_sceneInviewSm( t_ptrNode.p );
			continue;
		}


		t_bs.radius = t_ptrNode.p->get_smBSRadius();
		t_ptrNode.p->get_smPos( t_bs.veccen );

		// River added @ 2007-5-25:Ϊ�˴����ƫ�Ƶ����Ŀ���������
		t_ptrNode.p->get_adjPos( t_vec3Tmp );
		t_bs.veccen += t_vec3Tmp;

		// River mod @ 2008-8-16:
		// ���������bbox������������sphere radius
		os_bbox* t_ptrBox = t_ptrNode.p->get_smBBox();
		os_bsphere  t_s;
		t_ptrBox->to_bsphere( t_s );
		t_bs.radius = t_s.radius;


		// ֻҪ������������,ȫ����Ⱦ.
		if( !t_ptrSceMgr->is_sphereInView( t_bs,false ) )
		{
			// River added @ 2006-6-13:ʹ���������ڵ������������Բ���
			t_ptrNode.p->play_lastFrameStopAction();

			// River @ 2006-7-7:ֻ��������֡������������,�������Ĳ���Ⱦ��skinMesh,
			//                  Ϊ��֤������Ч�͵������ȷ��Ⱦ.
			if( t_ptrNode.p->m_bInViewLastFrame )
				t_ptrNode.p->m_bInViewLastFrame = false;
			else
				continue;

		}
		else
		{		
			// River mod @ 2008-8-16:��������ϸ�Ŀ����Բ���
			// Ŀǰ��⵱ǰ�Ķ����Ƿ��������ڡ�
			os_ViewFrustum*    t_frus = g_ptrCamera->get_frustum();
			if( !t_frus->objInVF( *t_ptrNode.p->get_smBBox() ) )
			{
				// River added @ 2006-6-13:ʹ���������ڵ������������Բ���
				t_ptrNode.p->play_lastFrameStopAction();

				// River @ 2006-7-7:ֻ��������֡������������,�������Ĳ���Ⱦ��skinMesh,
				//                  Ϊ��֤������Ч�͵������ȷ��Ⱦ.
				if( t_ptrNode.p->m_bInViewLastFrame )
					t_ptrNode.p->m_bInViewLastFrame = false;
				else
					continue;
			}

			// �˴��ŵ��������Ŀɼ�
			t_ptrNode.p->m_bInViewLastFrame = true;
		}

		add_sceneInviewSm( t_ptrNode.p );

	}

	// 
	// �õ����ǿɼ�sm�����б�󣬸�����Щ�����������Զ������������
	// ÿһ��Ԫ����һ��skinMesh��ָ�룬����ʹ��dword��Ϊsize.
	// �����õ�˳��Ϊ��Զ������
	qsort( &m_vecInViewSM[0],m_iInviewSMNum,sizeof( DWORD ),alphaSM_compare );

	//
	// ATTENTION TO FIX: 
	// �Ѿ����Զ��һ�������skinMesh��Ʒȥ����������Ⱦѹ����
	if( m_iInviewSMNum > MAX_INVIEWSM_NUM )
		m_iInviewSMNum = MAX_INVIEWSM_NUM;

	return;

	unguard;
}



//! ����ʵʱ��Ӱ����Ⱦ
void osc_skinMeshMgr::shadow_draw( BOOL _meshWithoutShadow )
{
	guard;

	//
	// �˴�����Ⱦ��������Ϊ����ȷ����Ⱦ��Ӱ������������޹�.
	// �˴�ֻ������������Ӱ����ȷλ�ã�û����Ⱦ����Ķ����
	for( int t_i=0;t_i<m_iRTShadowNum;t_i ++ )
	{
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice );
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice,true );
	}

 
	// �����������Ⱦ����ص���Ⱦ״̬��
	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,D3DVBF_DISABLE );
		m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false );
	}

	if(_meshWithoutShadow == FALSE)
		render_shadowSkin();
	
	set_smMgrRState();
	
	
	for( int t_i=0;t_i<this->m_iRTShadowNum;t_i ++ )
	{
		// ��������Ⱦ�����ס�������ϵ�����Ӱ֮ǰ����������Ķ���,�����ʱ����û�е��⣬����Ҫ����
		if( m_ptrShadowSkinPtr[t_i]->m_bRenderGlossy && m_ptrShadowSkinPtr[t_i]->m_dwFinalGlossyColor )
			m_ptrShadowSkinPtr[t_i]->cal_finalPose();
		
	    m_ptrShadowSkinPtr[t_i]->start_renderPostTrail(m_ptrShadowSkinPtr[t_i]);
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice );	

	}

	// ��alpha����ȷ��Ⱦ�������ϵ�͸����λ��
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );
	for( int t_i=0;t_i<m_iRTShadowNum;t_i ++ ){
		m_ptrShadowSkinPtr[t_i]->start_renderPostTrail( m_ptrShadowSkinPtr[t_i] );
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice,true );
	}

	

	unguard;
}


//! frameMove��ǰ��skin mesh Mgr.
bool osc_skinMeshMgr::framemove_smMgr( void )
{
	guard;

	float t_f = sg_timer::Instance()->get_lastelatime();

	// River @ 2010-9-28:������ϵͳ��Դ���,WORK START:�˴�ȷʵ���ܳ���....
	for( int t_i =0;t_i<MAX_RIDENUM;t_i ++ )
	{
		if( !m_vecRidePtrArray[t_i].m_ptrHuman )
			continue;
		if( m_vecRidePtrArray[t_i].m_ptrHuman->legal_operatePtr() &&
			m_vecRidePtrArray[t_i].m_ptrRide->legal_operatePtr() )
		{
			m_vecRidePtrArray[t_i].m_ptrRide->LinkHumObj(
				m_vecRidePtrArray[t_i].m_ptrHuman );
			m_vecRidePtrArray[t_i].m_ptrRide->m_bIsRide = true;

			m_vecRidePtrArray[t_i].m_ptrHuman->m_RideObj = 
				m_vecRidePtrArray[t_i].m_ptrRide;

			m_vecRidePtrArray[t_i].m_ptrRide->HideLastPart(true);

			m_vecRidePtrArray[t_i].m_ptrHuman = NULL;
			m_vecRidePtrArray[t_i].m_ptrRide = NULL;
		}
	}

	// 
	// ����һ���ԵĴ���
	// �õ��ɼ���sm�б�����
	get_inViewChrList();

	for( int t_i=0;t_i<m_iInviewSMNum;t_i++ )
	{
		// River @ 2010-9-1:���ﶯ��frame_move�����ڴ˴�����assert����.
		osassert( m_vecInViewSM[t_i] );
		m_vecInViewSM[t_i]->frame_moveAction( t_f );
		m_iRFaceNum += m_vecInViewSM[t_i]->get_smFaceNum();
	}

	return true;

	unguard;
}




//! River @ 2009-6-23:��ȾfakeGlowЧ��
void osc_skinMeshMgr::render_fakeGlow( void )
{
	guard;

	//! River @ 2009-6-23:�˴������������ڵ�����������ݺ���Ӱ����
	//!                   ��������Ӱ���ݣ����ܻ���Щ��������Ч��
	osVec3D   t_vec3Pos;
	g_ptrCamera->get_godLookVec( t_vec3Pos );
	osVec3Normalize( &t_vec3Pos,&t_vec3Pos );

	if( osc_bpmShadow::m_ptrFakeGlowData )
		osc_bpmShadow::m_ptrFakeGlowData->frame_move();

	for( int t_i=0;t_i<m_iRTShadowNum;t_i ++ )
	{
		if( !m_ptrShadowSkinPtr[t_i]->m_bFakeGlowDraw )
			continue;

		for( int t_idx = 0;t_idx < m_ptrShadowSkinPtr[t_i]->m_iNumParts;t_idx ++ )
		{
			osc_bpmShadow*  t_ptrShadow  = 
				m_ptrShadowSkinPtr[t_i]->m_vecBpmIns[t_idx].get_shadowPtr();
			if( !t_ptrShadow )
				continue;
			t_ptrShadow->draw_fakeGlow(
				0xffffffff,0xffffffff,t_vec3Pos,1.0f,1.0f,true );
		}

		break;
	}

	unguard;
}

bool osc_skinMeshMgr::render_smDepthMgr()
{
	guard;

	osassert( m_pd3dDevice );
	for( int t_i=0;t_i<m_iInviewSMNum;t_i++ )
	{
		m_vecInViewSM[t_i]->render_skinDepth( m_pd3dDevice );
	}
	
	return true;

	unguard;
}

void osc_skinMeshMgr::push_behindWallChar( osc_skinMesh* _ptrMesh )
{
	guard;

	osassert(_ptrMesh);

	for( int t_i=0;t_i<MAX_BEHINDWALLCHAR;t_i ++ )
	{
		if( m_vecRCBehindWall[t_i] == _ptrMesh)
		{
			// ֮ǰ�ƽ�������ֱ�ӷ���
			return ;
		}
	}

	for( int t_i=0;t_i<MAX_BEHINDWALLCHAR;t_i ++ )
	{
		if( m_vecRCBehindWall[t_i] == NULL )
		{
			m_vecRCBehindWall[t_i] = _ptrMesh;
			return;
		}
	}
	
	//osassertex( false,"û���㹻�Ŀռ������ɸ����behind WAll Char...\n" );

	unguard;
}
void osc_skinMeshMgr::delete_behindWallChar( osc_skinMesh* _ptrMesh )
{
	guard;

	osassert(_ptrMesh);

	for( int t_i=0;t_i<MAX_BEHINDWALLCHAR;t_i ++ )
	{
		if( m_vecRCBehindWall[t_i] == _ptrMesh )
		{
			m_vecRCBehindWall[t_i] = NULL;
			return;
		}
	}

	unguard;
}

//��  River @ 2010-1-29: ��Ⱦǽ�۵��������
bool osc_skinMeshMgr::renderBehindWallChar( void )
{
	guard;

	static int   t_iCharBehindWallEffectId = -1;

	if( t_iCharBehindWallEffectId < 0 )
	{
		t_iCharBehindWallEffectId = g_shaderMgr->add_shader( 
			"charBehindWall", osc_engine::get_shaderFile( "charBehindWall" ).c_str() );

		osassert( t_iCharBehindWallEffectId >= 0 );
	}

	set_smMgrRState();
	// ��ÿһ��ָ����д���
	g_bForceUseCustomShader = true;
	g_iForceCustomShaderIdx = t_iCharBehindWallEffectId;

	for( int t_i = 0;t_i<MAX_BEHINDWALLCHAR;t_i ++ )
	{
		if( m_vecRCBehindWall[t_i] != NULL )
		{
			if( !m_vecRCBehindWall[t_i]->legal_operatePtr() )
				continue;

			m_vecRCBehindWall[t_i]->render_skinMesh( m_pd3dDevice );
			m_vecRCBehindWall[t_i]->render_skinMesh( m_pd3dDevice,true );
		}		
	}
	g_iForceCustomShaderIdx = -1;
	g_bForceUseCustomShader = false;

	endset_smMgrRState();

	return true;

	unguard;
}

/** \brief
*   ��Ⱦ��ǰ��skinMeshManager.
*
*   \param _ralpha ��ǰ��Ⱦ�Ƿ���Ⱦ��alphaͨ����ani���֡�
*	\param _meshOrShadow �����0��ʱ����Ⱦ ģ�� + ��Ӱ��1��ʱ����Ⱦģ�ͣ�2��ʱ����Ⱦ��Ӱ
*
*   ��Ⱦ��ʱ����Ը���ÿһ��sm�����������Զ����������
*   ��Ⱦ��͸�����岿���ɽ���Զ����Ⱦ͸���������굽��������͸��������Ⱦ��ɺ�
*   ��Ҫ��Ⱦ͸�����ֵ�Zbuffer.
*   
*/
bool osc_skinMeshMgr::render_smMgr(  os_FightSceneRType _rtype/*= OS_COMMONRENDER*/,
			              BOOL _ralpha/* = false*/,BOOL _meshWithoutShadow /*= FALSE*/ )
{
	guard;

	osassert( m_pd3dDevice );

	int               t_iChrNum = 0;

	// River @ 2006-7-7:����Ĳ�͸�����ֺ�͸��������Ⱦ�ֿ�,���Դ˴���ÿ��������Ⱦ״̬.
	set_smMgrRState();

	if( !_ralpha )
	{
		for( int t_i=0;t_i<m_iInviewSMNum;t_i++ )
		{
			if( (!m_vecInViewSM[t_i]->m_bShadowSupport)||(!g_bHeroShadow)  ){
				m_vecInViewSM[t_i]->start_renderPostTrail((m_vecInViewSM[t_i]));
				m_vecInViewSM[t_i]->render_skinMesh( m_pd3dDevice );
			}
		}
		
		// ���ü�ʱ��Ӱ�Ķ���ԶԪ��ָ��Ϊ��
		for( int t_i=0;t_i<MAX_RTSHADOWNUM;t_i ++ )
			m_ptrShadowSkinPtr[t_i] = NULL;
		m_iRTShadowNum = 0;
	}

	

	if( _ralpha )
	{
		// 
		// ������Ⱦ״̬��
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );	

		for( int t_i=m_iInviewSMNum-1;t_i>=0;t_i-- )
		{
			if( (!m_vecInViewSM[t_i]->m_bShadowSupport)||(!g_bHeroShadow)  ){
				m_vecInViewSM[t_i]->start_renderPostTrail((m_vecInViewSM[t_i]));
				m_vecInViewSM[t_i]->render_skinMesh( m_pd3dDevice,true );
				
			}else
				push_rtShadowSmPtr( m_vecInViewSM[t_i] );
		}

		// �������е����뵽�ܵ��е�������Ӱ��Ⱦ.
		reset_rtShadow();
	}




	// ��Ⱦ�����������Ӱ��
	if( (_ralpha)&&(g_bHeroShadow))
	{
		if( (m_iRTShadowNum > 0) )
			shadow_draw( _meshWithoutShadow );
	}
	

	//
	// River @ 2006-7-7:����Ĳ�͸�����ֺ�͸��������Ⱦ�ֿ�,���Դ˴���ÿ��
	// �˳���Ⱦskeletal animation��ʱ��,����.
	// ���Ӳ������ת��,���vs.
	endset_smMgrRState();


	//! river: fake Glow����Ⱦ
	render_fakeGlow();

	return true;

	unguard;
}


//! ��ĳһ��Character���е�����Ⱦ����Ҫ���ڰ���ά������Ⱦ����ά����Ļ������
bool osc_skinMeshMgr::render_character( osc_skinMesh* _ptrMesh )
{
	guard;

	osassert( _ptrMesh );

	// ������Ⱦ״̬.
	set_smMgrRState();

	_ptrMesh->render_skinMesh( m_pd3dDevice );

	// 
	// ������Ⱦ״̬��
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );	
	_ptrMesh->render_skinMesh( m_pd3dDevice,true );

	endset_smMgrRState();

	return true;

	unguard;
}



//! ��������������еļ�ʱ��Ӱ����,ʹ�´���Ⱦ��ʱ��,������Ӱ.
void osc_skinMeshMgr::reset_rtShadow( void )
{
	guard;

	for( int t_idx = 0;t_idx<this->m_iRTShadowNum;t_idx ++ )
	{
		osassert( m_ptrShadowSkinPtr[t_idx] );
		for( int t_i=0;t_i<m_ptrShadowSkinPtr[t_idx]->m_iNumParts;t_i ++ )
		{
			osc_bpmShadow*  t_ptrShadow = 
				m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_shadowPtr();


# ifdef _DEBUG
			// ATTENTION TO FIX: River @ 2009-12-8:Ӧ�ò����ٳ��ִ��������ˡ�
			//                   �ϲ��ڴ���һ�������ͬʱ�����ͷ�������
			//                   loadLock�ڵײ�û������Ӧ�Ĵ������´�������ķ�����
			if( !t_ptrShadow )
			{
				// �ٴ�ȷ�ϣ��Ƿ��skinPtrΪҪ�ͷŵ�skinPtr.
				osassertex( (m_ptrShadowSkinPtr[t_idx]->m_Look.m_mesh[t_i] == 0),
					va( "��Ӱ�������� mesh idx<%d>..parts idx<%d>.skinName<%s>.SkinPtr<0x%x>..\n",
					m_ptrShadowSkinPtr[t_idx]->m_Look.m_mesh[t_i],t_idx,
					m_ptrShadowSkinPtr[t_idx]->m_strSMDir,DWORD( m_ptrShadowSkinPtr[t_idx] ) ) );
			}
# endif 
			if( t_ptrShadow )
				t_ptrShadow->frame_setShadow();
		}
	}


	unguard;
}


//! ÿһ֡��skinmeshMGR���á�
void osc_skinMeshMgr::frame_setSmMgr( void )
{
	guard;

	m_iInviewSMNum = 0;
	m_iRFaceNum = 0;

	//osDebugOut( "The skinmesh vector size is<%d>...\n",m_vecDadded.size() );


# if !__QTANG_VERSION__
	// �˴����룬���ڴ���ȫ3D������ʹ�á�
	// River @ 2009-4-16:��Ϊ�ͻ��˿�����������ĽǶȣ�����
	// �˴��Ĵ������ʡ��������������Ļ������Ľ���취
	//! �������Ӱ������ָ��,���������ļ�����bsphere��ײ,����ʾ
	for( int t_i=0;t_i<MAX_RTSHADOWNUM;t_i ++ )
	{
		if( !m_ptrShadowSkinPtr[t_i] )
			continue;

		os_bbox*     t_ptrBBox;
		os_bsphere   t_bs;
		
		osassertex( 
			_CrtIsValidPointer( m_ptrShadowSkinPtr[t_i],sizeof( osc_skinMesh ),true ),
			"�Ƿ���������Ӱָ��...\n" );

		t_ptrBBox = m_ptrShadowSkinPtr[t_i]->get_smBBox();

		g_ptrCamera->get_camNearClipBSphere( t_bs );
		if( t_ptrBBox->collision( t_bs ) )
			m_ptrShadowSkinPtr[t_i]->set_hideSkinMesh( true );
		else
		{
			// ���Ҫȫ����ʾ����,������и������ײ����ռ�.
			t_bs.radius *= 1.05f;		

			if( !t_ptrBBox->collision( t_bs ) )
				m_ptrShadowSkinPtr[t_i]->set_hideSkinMesh( false );
		}
	}
# endif 

	unguard;
}





