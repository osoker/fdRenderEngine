///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacter.h
 * 
 *  His:      River created @ 2003-12-31
 *
 *  Desc:     用于os引擎中的人物动画调用。
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

//! 在hlsl中用到的距阵名.
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

//! 用于人物身上更多层的特效
# define  HLSL_TEX4           "Texture3"

# define  HLSL_TEXMAT          "MatrixTex"
# define  HLSL_LIGHTDIR        "lhtDir"


//!windy add
# define HLSL_VIEWPOSTION		"ViewPostion"
# define HLSL_VIEWMATRIX		"mViewMatrix"

//! 视野中最多可以出现的skinMesh数目。
OSENGINE_API int MAX_INVIEWSM_NUM = 96;


//! skinManager.
static osc_skinMeshMgr*  g_skinMeshMgr = NULL;

//! 是否强制显示skinMesh上对应的第二层贴图。
OSENGINE_API BOOL   g_bForceSecEff = FALSE;


// 
//! 渲染人物用到的顶点声明.
LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration3;


//! River @ 2010-12-28:游戏内人物纹理的缓冲区大小，此值大于零，小于2，
//  可以是浮点数，此值越大，占用内存越多，但调入人物时，与硬盘的交互越小。
OSENGINE_API float           g_fCharacterTexBufScale = 0.7f;                  

//! 定义视域内最多的skinMesh的数目。// ATTENTION TO FIX:
//! 如果可行，最好跟这个MAX_RTSHADOWNUM宏使用相同的数据
//  River @ 2011-2-25:修改为全局的变量.
OSENGINE_API int             MAX_SMINVIWE = 128;



//! 设置effect Shader的constant数据。
void osc_bodyFrame::set_shaderConstant( LPD3DXEFFECT _eff )
{
	guard;

	osassert( _eff );

	osVec4D  t_vec4Col;

	t_vec4Col.w = 1.0f;

	// 有使用全局光做为方向光的skinMesh.
	if( m_pParentSkin->is_useDlAsAmbi() )
	{
		// 设置人物的全局光，全局光使用最亮的方向光。
		t_vec4Col.x = g_fDirectLR;
		t_vec4Col.y = g_fDirectLG;
		t_vec4Col.z = g_fDirectLB;

		_eff->SetVector( HLSL_AMBIENT,
			(osVec4D*)&t_vec4Col );

		// 设置方向光,方向光为空，相当于完全使用环境光.
		t_vec4Col.x = 0.0f;
		t_vec4Col.y = 0.0f;
		t_vec4Col.z = 0.0f;
		_eff->SetVector( HLSL_DIFFUSE,&t_vec4Col );
	}
	else
	{
		// 设置人物的全局光和方向光。

		// 所有的人物都排除开。
		//
		// tzz @2010-6-21 qtt suggest
		// 所有的任务都受同样的光照影响
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

		// 设置方向光。
		t_vec4Col.x = g_fDirectLR * m_pParentSkin->m_sCurSkinMeshAmbi.r;
		t_vec4Col.y = g_fDirectLG * m_pParentSkin->m_sCurSkinMeshAmbi.g;
		t_vec4Col.z = g_fDirectLB * m_pParentSkin->m_sCurSkinMeshAmbi.b;
		t_vec4Col.w = 1.0f;
		_eff->SetVector( HLSL_DIFFUSE,&t_vec4Col );
	}
	
	// 设置灯光的方向，
	osVec3Normalize( &g_vec3LPos,&g_vec3LPos );
	t_vec4Col.x = g_vec3LPos.x;
	t_vec4Col.y = g_vec3LPos.y;
	t_vec4Col.z = g_vec3LPos.z;
	t_vec4Col.w = 1.0f;
	_eff->SetVector( HLSL_LIGHTDIR,&t_vec4Col );

	//@{
	// River @ 2009-1-29:如果强制shader，则设置特殊的shader.
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
	// 测试灯光效果。
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

//! 加入刀光的关键帧.
int osc_bodyFrame::add_swordGlossyKey( int& _addNum )
{
	guard;

	osMatrix         t_matTmp,t_matNewView;;
	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	static osMatrix   t_matArray[MAX_BONEPERBPM];

	//
	// 渲染当前Frame中所有的body part mesh.
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

		// ATTENTION TO FIX: 如果此部位对应多个mesh,会有问题:
		osassertex( m_vecPtrBpmIns[t_i]->m_ptrSGIns,"刀光部位只对应一个mesh..." );

		m_vecPtrBpmIns[t_i]->m_ptrSGIns->add_swordGlossyKey( 
			t_matArray[t_ptrPM->m_ptrSwordGlossy->m_iMatrixIdx] );

		// 对应完了刀光后,不需要再遍历其它的子结点,返回.
		//return m_vecPtrBpmIns[t_i]->m_ptrSGIns->get_sgMoveLength();
		//! river @ 2009-12-11:如果此时有八个身体部位，即两手武器，需要继续遍历.
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
	// 递归渲染当前结构的子兄弟结构和子结点.
	int   t_iSgMoveLen = 0;
	if( m_pSibling )
	{
		t_iSgMoveLen = m_pSibling->add_swordGlossyKey( _addNum );
		if( t_iSgMoveLen > 0 )
			return t_iSgMoveLen;

	}

	if( m_pFirstChild )
		t_iSgMoveLen = m_pFirstChild->add_swordGlossyKey( _addNum );

	// 如果返回是零,则由再上一层来处理.
	return t_iSgMoveLen;

	unguard;
}


//! 对刀光的渲染，使用单独的一个函数。
bool osc_bodyFrame::render_swordGlossy( LPDIRECT3DDEVICE9 _dev,bool _gdraw )
{
	guard;

	osc_bodyPartMesh* t_ptrPM;
	osMatrix**        t_offsetMatPtr;
	static osMatrix   t_matArray[30];

	//
	// 渲染当前Frame中所有的body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		if( !t_ptrPM->m_ptrSwordGlossy )
			continue;

		//
		// 刀光渲染使用一个单独的函数：
		// 如果当前的bodyPartMesh对应了武器刀光信息。
		// 提取当前刀光信息用到的矩阵并渲染刀光数据。
		if( t_ptrPM->m_ptrSwordGlossy )
		{
			osVec3D   t_vec3PosMid;

			//@{
			//! River @ 2010-1-3:用于去除场景内残留的武器特效效果。
			bool      t_bUseBindEffectPos = true;
			if( (!m_pParentSkin->m_bInViewLastFrame)&&(!m_pParentSkin->m_bUseInInterface) )
				t_bUseBindEffectPos = false;
			//@}

			m_vecPtrBpmIns[t_i]->m_ptrSGIns->draw_swordGlossy( 
				t_vec3PosMid,_gdraw,t_ptrPM->m_bDisplay,t_bUseBindEffectPos );
			m_pParentSkin->set_handPos( t_vec3PosMid );
		}	
	}


	// River mod @ 2009-9-16: 修改递归算法,可以有更多层次的人物骨骼结构. 
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



//! TEST CODE:用于遍历层级.
void osc_bodyFrame::through( int& _hier,os_boneSys* _boneSys,bool _sibling/* = true*/ )
{
	guard;

	_hier ++;

	if( _sibling )
	{
		if( m_iBMNum > 0 ) 
		{
		osDebugOut( "兄弟调用,The <%d> 层级,<%d>骨骼,名字为<%s> bm num<%d>....\n",
			_hier,m_dwID,_boneSys->get_boneNameFromId( m_dwID ),m_iBMNum );
		}
		else
		{
		osDebugOut( "兄弟调用,The <%d> 层级,<%d>骨骼,名字为<%s> 子骨ID<%d>名字<%s>....\n",
			_hier,m_dwID,_boneSys->get_boneNameFromId( m_dwID ),
			m_pFirstChild->m_dwID,
			_boneSys->get_boneNameFromId( m_pFirstChild->m_dwID ) );
		}
	}
	else
	{
		osDebugOut( "父子调用,The <%d> 层级,<%d>骨骼,名字为<%s>....\n",
			_hier,m_dwID,_boneSys->get_boneNameFromId( m_dwID ) );
	}

	// River mod @ 2009-9-16: 修改递归算法,可以有更多层次的人物骨骼结构. 
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
	//! 最大的palette支持数目。
	static osMatrix   t_matArray[28];
	//float            t_fAlpha = _alphaVal;

	// 渲染当前Frame中所有的body part mesh.
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

		//! 设置全局的shader Constant.
		//set_shaderConstant( t_eff );
		osMatrix   t_mat;
		osMatrixMultiply( &t_mat,&g_matView,&g_matProj );
		t_eff->SetMatrix( HLSL_VIEWPROJMAT,&t_mat );


		//
		// 设置effect中shader用到的矩阵	
		t_eff->SetMatrixArray( HLSL_PALETTEMAT, t_matArray,t_ptrPM->m_dwNumPalette );

		
		//
		// 由前端管道去渲染我们用到的设备相关数据。
		// TEST CODE:
		//@{
		if( !t_ptrRes->m_pBuf )
		{		
			osDebugOut( "The vernum<%d>,versize<%d>..\n",
				t_ptrRes->m_iVerNum,t_ptrRes->m_iVerSize );

			if( t_ptrRes->m_strMFname[0] )
				osDebugOut( "The filename <%s>..\n",t_ptrRes->m_strMFname );

			if( t_ptrRes->m_bUsed )
				osDebugOut( "正在使用中...\n" );
			else
				osDebugOut( "不在使用中....\n" );

		    //osassert( t_ptrRes->m_pBuf );
			// River mod 2010-10-4:确认此时的出错环境.
			(*(char*)0) = NULL;
		}
		//@} 

		g_frender->prepare_andrender( t_ptrRes->m_pBuf,t_ptrRes->m_pIdxBuf,
			pEff,D3DPT_TRIANGLELIST,t_ptrRes->m_iVerSize,0,0,t_ptrRes->m_iIdxNum/3,
			t_ptrRes->m_iVerNum,true );

	}

	// River mod @ 2009-9-16: 修改递归算法,可以有更多层次的人物骨骼结构. 
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
 *  真正渲染skinMesh的函数。
 *
 *  在这个函数中，检测如果当前frame对应的mesh不为空，渲染当前frame对应的mesh.
 *  
 *  \param _alpha 是否是画透明的alpha片.
 *  \param _fade  是否处于淡入淡出状态的渲染，这种状态下，非alpha结点需要象
 *                alpha结点一样得到渲染。
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
	//! 最大的palette支持数目。
	static osMatrix   t_matArray[28];
	float             t_fAlpha = _alphaVal;

	//
	// 渲染当前Frame中所有的body part mesh.
	for( int t_i=0;t_i<m_iBMNum;t_i++ )
	{
		osassert( this->m_vecPtrBpmIns[t_i] != NULL );
		t_offsetMatPtr = m_vecPtrBpmIns[t_i]->get_bpmBoneOffset();
		t_ptrPM = m_vecPtrBpmIns[t_i]->get_bpmPtr();
		osassert( t_ptrPM->m_dwNumInfluPV == VERTEX_INFLUENCE );

		// River @ 2010-7-1:用于隐藏武器.
		if( m_vecPtrBpmIns[t_i]->get_hideState() )
			continue;

		if( !t_ptrPM->m_bDisplay )
			continue;

		//
		// 如果当前渲染的是有alpha通道的pm.
		if( _alpha )
		{
			// Alpha渲染时，如果是fade状态下，则没有alpha通道的结点也要渲染。
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
		// River @ 2010-10-9:更平缓的错误处理方式。
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
		// 如果当前frame对应的mesh需要渲染阴影:
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
		// 如果当前的skin方法是d3dIndexed,调用不同的渲染函数.
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

		// River mod @ 2010-1-29: 加入强制的Effect渲染，用于墙后的人物渲染
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

		// River @ 2010-1-29: 加入强制effect渲染后，不需要设置点光源
		if(/*!m_pParentSkin->is_excludeLight() && */  // tzz @ 2010-6-18 qtt 说需要人物有方向光的影响,那么点光源也需要做出配合，于是注释掉这句
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

		//! 设置全局的shader Constant.
		set_shaderConstant( t_eff );


		//
		// 设置effect中shader用到的矩阵	
		t_eff->SetMatrixArray( HLSL_PALETTEMAT, t_matArray,t_ptrPM->m_dwNumPalette );
				
		// 
		//! River @ 2010-1-29:强制shader处理人物在墙后的渲染
		if( (m_vecPtrBpmIns[t_i]->has_secondEffect())&&(!g_bForceUseCustomShader) )
		{
			D3DXVECTOR4 pos(g_vec3ViewPos.x,g_vec3ViewPos.y,g_vec3ViewPos.z,1);

			t_eff->SetVector(HLSL_VIEWPOSTION,&pos);
			t_eff->SetMatrix(HLSL_VIEWMATRIX,&g_matView);

			// 如果第二层有矩阵的话 
			// River @ 2010-2-25:加入自发光纹理的处理
			// River @ 2010-3-13:更多的纹理效果加入
			if( m_vecPtrBpmIns[t_i]->is_rottexEffect()   ||
				m_vecPtrBpmIns[t_i]->is_shineTexEffect() ||
				m_vecPtrBpmIns[t_i]->is_rotEnvEffect()   ||
				m_vecPtrBpmIns[t_i]->is_envShineEffect() ||
				m_vecPtrBpmIns[t_i]->is_rotShineEffect()   )
				t_eff->SetMatrix( HLSL_TEXMAT,m_vecPtrBpmIns[t_i]->get_texTransMat() );

			//  
			//! 设置第二层控制......
			t_hr = t_eff->SetTexture( HLSL_TEX2,
				g_shaderMgr->get_texturePtr( m_vecPtrBpmIns[t_i]->get_secTexid() ) );
			osassertex( (!FAILED( t_hr )),(char*)osn_mathFunc::get_errorStr(t_hr) );
			
			//!设置第三层纹理，用于控制第二层的ALPHA，
			int texid = m_vecPtrBpmIns[t_i]->get_ThdTexid();
			if (texid != -1){
				t_hr = t_eff->SetTexture( HLSL_TEX3,g_shaderMgr->get_texturePtr( texid));
			}
			osassertex( (!FAILED( t_hr )),(char*)osn_mathFunc::get_errorStr(t_hr) );

			//！ River @ 2010-3-11:如果有第四层纹理，设置第四层纹理
			texid = m_vecPtrBpmIns[t_i]->get_fourthTexid();
			if (texid != -1){
				t_hr = t_eff->SetTexture( HLSL_TEX4,g_shaderMgr->get_texturePtr( texid));
			}
			// River @ 2011-1-21:此处可能会出现Invalid call的情况，不使用此assert,再看后果。
			//osassertex( (!FAILED( t_hr )),(char*)osn_mathFunc::get_errorStr(t_hr) );

		}

		//
		// 由前端管道去渲染我们用到的设备相关数据。
		// TEST CODE:
		//@{
		if( (!t_ptrRes->m_pBuf)||(!t_ptrRes->m_pIdxBuf ) )
		{		
			osDebugOut( "The vernum<%d>,versize<%d>..\n",
				t_ptrRes->m_iVerNum,t_ptrRes->m_iVerSize );

			if( t_ptrRes->m_strMFname[0] )
				osDebugOut( "The filename <%s>..\n",t_ptrRes->m_strMFname );

			if( t_ptrRes->m_bUsed )
				osDebugOut( "正在使用中...\n" );
			else
				osDebugOut( "不在使用中....\n" );

			//osassert( t_ptrRes->m_pBuf );
			// River mod @ 2011-6-15:
			// 如果出现数据错误，直接返回不渲染，如果渲染有问题，玩家可以自己退出游戏，再进入。
			return true;

		}
		//@} 		
		g_frender->prepare_andrender( t_ptrRes->m_pBuf,t_ptrRes->m_pIdxBuf,
			pEff,D3DPT_TRIANGLELIST,t_ptrRes->m_iVerSize,0,0,t_ptrRes->m_iIdxNum/3,
			t_ptrRes->m_iVerNum,true );

	}

	// River mod @ 2009-9-16: 修改递归算法,可以有更多层次的人物骨骼结构. 
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
*  使用d3dIndexedBlend的时候,使用这个函数对skinMesh进行渲染.
*
*  以后会增加其它的渲染方式,以更加合理的利用硬件功能.
*  
*  ATTENTION TO OPP: 使用stateBlock.
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


//! River @ 2010-7-1: 人物是否隐藏武器，如果隐藏，则不渲染武器部分
void osc_skinMesh::hide_weapon( bool _hide/* = true*/ )
{
	guard;

	bool  t_bUnHide = false;

	for( int t_i = 6;t_i < this->m_iNumParts;t_i ++ )
	{
		// 处理这个变量，是不需要每一帧都内部处理刀光
		if( (m_vecBpmIns[t_i].m_bHide == true) && 
			( _hide == false ) )
			t_bUnHide = true;

		m_vecBpmIns[t_i].m_bHide = _hide;
	}

	
	// River @ 2010-7-1:此处应该加入武器特效的隐藏，先确认武器没有问题后再加入武器特效相关.
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



//! 设置fade状态。
void osc_skinMesh::set_fadeState( bool _fadeS,float _alphaV/* = 1.0f*/ )
{
	guard;


	// 如果人物处于隐藏状态，则设置fadeState不起效果
	if( m_bHideSm )
	{
		//osDebugOut( "隐藏人物的状态下，不能再对人物进行fade操作....\n" );
		return;
	}

	// 如果上次设置fade状态为false,而当前的fade状态也为false,则不做任何的处理
	if( (!m_bFadeState) && (!_fadeS ) )
		return;

	bool   t_bFadeSBack = m_bFadeState;
	

	m_bFadeState = _fadeS;
	m_fAlphaVal  = _alphaV;

	// 
	// River @ 2009-5-8:去掉人物fade时,阴影的透明度.
	// 重设阴影的透明度
	if( _fadeS && g_bShadowFade )
		osc_bpmShadow::reset_shadowColor( _alphaV );



	//  
	//  River added @ 2009--3-13:
	//! 如果fadeState为真，且播放刀光特效，暂不播放。
	// 如果是要fade,则需要删除武器特效,如果是要显示,则需要创建武器特效
	if( (!t_bFadeSBack) && m_bFadeState )
	{
		// River added @ 2010-8-18:减小代码开销.
		if( m_iNumParts>6 )
			playSwordGlossyInner( false,false );
		if (mEffectId!=-1)
		{
			get_sceneMgr()->delete_effect(mEffectId);
			mEffectId = -1;
		}
	}
	// 正常显示人物的状态
	else if( !m_bFadeState ) 
	{
		// River added @ 2010-8-18:减小代码开销.
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
*  渲染当前动作的刀光
*
*  把时间往前推，依次算出前几帧刀光的位置。这个技术应该也可以实现出其它的特效，比如
*  人的残影等等，以后扩展。这种算法的，会占用大量的cpu时间，对于低配置的机器,需要在
*  上层关闭刀光特效和相关的计算。
*
*  1: 先把当前帧刀光的位置加入到刀光管理器.
*  2: 根据时间，依次往后推，再算出至少三个的关键位置。把这些位置依次加入到
*     刀光管理器中。
*  3: 使用刀光管理器中的位置，进行插值，得到更圆滑的刀光
*/
bool osc_skinMesh::render_smSwordGlossy( LPDIRECT3DDEVICE9 _dev )
{
	guard;

	osassert( _dev );

	float    t_fMoveBackTime;
	int      t_i;
	int      t_iSGKeyNum = 0;

	//  先加入当前刀光的位置:
	int   t_iTmp = 0;
	t_iSGKeyNum = m_pRoot->add_swordGlossyKey( t_iTmp );

	//  River mod @ 2007-9-29: 如果返回的sgKeyNum是零,则不需要渲染,返回
	//! River mod @ 2010-6-5:  刀光sgKeyNum为零的情况下，如果播放武器特效，继续渲染。
	if( (t_iSGKeyNum == 0)&&(!m_bPlayWeaponEffect) )
		return false;

	//osassert( (t_iSGKeyNum>=MIN_SWORDPOS)&&(t_iSGKeyNum<=MAX_SWORDPOS) );

	//
	// 在只显示刀光附加特效的情况下,只需要推进一个刀光位置.
	bool t_part7MultiPos = false,t_part8MultiPos = false;

	if (m_ptrSGBpmIns){
		t_part7MultiPos = m_ptrSGBpmIns->m_ptrSGIns->need_pushMultiSPos();
	}
	if (m_ptrSGBpmIns8){
		t_part8MultiPos = m_ptrSGBpmIns8->m_ptrSGIns->need_pushMultiSPos();
	}

# if __GLOSSY_EDITOR__

# else
	//! 动作变化状态时或刀光颜色为空时，也不需要如下的处理
	if( m_bPlaySGlossy && (m_dwFinalGlossyColor!=0) && 
		(t_part7MultiPos||t_part8MultiPos) && (!m_bChangeActState) )
# endif 
	{

		// 依次后推刀光的位置.
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

		// 前面已经加入了三个关键帧.
		t_i = 3;

		// 接下循环直到加入我们需要个数的刀光光关键帧
		while( 1 )
		{
			get_keyFramePose();
			t_iTmp = 0;
			m_pRoot->add_swordGlossyKey( t_iTmp );
			t_i ++;
			if( t_i == t_iSGKeyNum )
				break;

			// 得到当前的关键帧与他之上关键帧的帧中间插值。
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


	// 刀光不写zbuffer.
	_dev->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
	m_pRoot->render_swordGlossy( _dev,m_bPlaySGlossy );

	return true;

	unguard;
}


/** \brief
*  在屏幕一个二维区域上渲染当前的人物,这个渲染使用Orthogonal投影矩阵。
*
*  \param _rect  要在哪个屏幕区域内渲染人物。
*  \param _zoom  用于控制相机离人物远近的数据,内部把人物包围盒的中心点放在osVec3D( 0.0f,0.0f,0.0f)的
*                位置上，相机放在osVec3D( 0.0f,0.0f,-_zoom)的位置上。上层可以调整出一个合适的值，在
*                实时的渲染中一直使用这个值。
*  \param float _rotAgl 用于二维界面内人物的旋转方向。
*  \param _newChar 是否是新创建的人物，如果是新创建的人物接口，则这个人物的动作与三维场景内的人物没有
*                  任何的关系
*	\param _camera 如果这个参数有效，就不适用全局的相机指针惊醒渲染
*/
void osc_skinMesh::render_charInScrRect( RECT& _rect,float _rotAgl,
							float _zoom,BOOL _newChar/* = FALSE*/,I_camera* _camera /* = NULL */)
{
	guard;

	osMatrix   t_sMatView = g_matView,t_sMatProj = g_matProj;
	D3DVIEWPORT9  t_viewPort;
	D3DVIEWPORT9  t_sNewViewPort;

	// 调用skinMeshMgr内的渲染功能
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
	
	// 先是viewPort.
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
	// 根据新设置的viewPort是否超出我们的屏幕大小，来设置渲染人物时相机的位置。
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
	// 人物的中心点放到0,0,0位置上,根据要渲染的视口是否超出屏幕进行微调，
	// 并设置二维渲染时人物的旋转方向
	float    t_fCharHeight,t_fx,t_fz,t_fZPos;

	// River mod @ 2009-4-14:用于处理包围盒在空中的怪物，如飞行类的怪物。
	// 处理厚度和宽度很宽的怪物。
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

	//! River mod @ 2009-5-9:正确的boundingBox,尽量正确的渲染。
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

	// 人物动作的时间差值
	float     t_fActTime = 0.0f;
	if( _newChar )
	{
		// River @ 2009-5-4:用于上层多帧调用一次此函数
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
	// 视矩阵和Orthogonal Prjection
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
	//! River @ 2009-12-29:此处应该渲染单独的osa动画效果.2010年以前必须完成的功能
	//  如果后期需要，可以加入粒子和公告板的二维界面动画效果
	I_fdScene*  t_ptrScene = ::get_sceneMgr();
	osc_TGManager*  t_ptrRc = (osc_TGManager*)t_ptrScene;
	t_ptrRc->render_topLayer( _camera,OS_RENDER_TOPLAYER );


	// 回复人物的位置数据
	m_vec3SMPos =  t_vec3BackPos;
	m_fRotAgl = t_fAngleBack;
	m_fLastRotAgl = t_fLastAngle;
 
	// 对相机进行回复，使一下帧场景可以正确的渲染。
	m_pd3dDevice->SetViewport( &t_viewPort );
	g_matView = t_sMatView;
	g_matProj = t_sMatProj;


	unguard;
}

bool osc_skinMesh::render_skinDepth(LPDIRECT3DDEVICE9 _dev)
{
	guard;
	// River @ 2009-6-7:如果正处于Load状态，返回。
	if( m_bLoadLock )
		return true;

	// 
	// 如果当前的skinMesh处于隐藏状态，则直接返回。
	if( m_bHideSm  )
		return true;

	if( !m_pRoot->RenderDepth( _dev) )
		return false;

	return true;
	unguard;
}

/** \brief
*  对当前的skinMesh进行渲染。
*
*  
*  \param 当前渲染需要的渲染skinMesh中的alpha部分还是非alpha部分。
*/
bool osc_skinMesh::render_skinMesh( LPDIRECT3DDEVICE9 _dev,bool _alpha/* = false */ )
{
	guard;

	// River @ 2009-6-7:如果正处于Load状态，返回。
	if( m_bLoadLock )
		return true;
	
	// 
	// 如果当前的skinMesh处于隐藏状态，则直接返回。
	if( m_bHideSm  )
		return true;

	// 
	// 处于fade状态时，只有alpha渲染才能渲染。
	if( !_alpha )
	{
		if( m_bFadeState )			
			return true;
	}

	// 设置当前skinMesh渲染用到的环境光。
	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		if( m_bUseDirLAsAmbient )
		{
			osColor   t_color( g_fDirectLR,g_fDirectLG,g_fDirectLB,1.0f );
			_dev->SetRenderState( D3DRS_AMBIENT,(DWORD)t_color );
			
			// ATTENTION TO OPP: 设置第一个灯光为空
			_dev->LightEnable( 0,false );
		}
		else
		{
			_dev->SetRenderState( D3DRS_AMBIENT,((DWORD)m_sCurSkinMeshAmbi) );
			// 设置第一个灯光为正常场景中的diuffse灯光。
			_dev->LightEnable( 0,true );
		}

	}


	//
	// ATTENTION TO FIX：设置可写Zbuffer？？？
	if( _alpha )
	{
		_dev->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );
		_dev->SetRenderState( D3DRS_ALPHAREF, GLOBAL_MIDALPHAREF);
		_dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	}

	//
	// 设置其它的渲染状态,用于替代effect文件,
	if( g_bCharWireFrame )
		_dev->SetRenderState( D3DRS_FILLMODE,D3DFILL_WIREFRAME );
	else
		_dev->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );

	//
	// 先渲染没有alpha的部分。
	bool t_renderSucc = m_pRoot->Render( _dev,_alpha,m_bFadeState,m_fAlphaVal );
	if( !t_renderSucc )
		return false;

	//! River @ 2009-1-29:renderBehind Wall.
	if( g_bForceUseCustomShader )
		return true;


	// 
	// 最后画刀光.
	// 
	// _alpha 渲染人物的alpha部分时,才渲染刀光.
	// (!m_bRenderGlossy)这个条件保证人物在一帧中只渲染一次刀光
	// m_bPlaySGlossy 当前的人物是在播放刀光的状态.
	// 
	// River mod @ 2009-2-18:加入m_bPlaySGlossy的判断
	if( _alpha && (!m_bRenderGlossy) && (m_bPlaySGlossy||m_bPlayWeaponEffect) )
	{
		// 处于动作变化状态，也不需要处理刀光
		// 
		// River @ 2006-6-27:动作变化状态的武器特效需要渲染，此时对刀光特殊处理????
		//                   但对武器的附加特效可以进行处理
		//if( (m_ptrSGBpmIns||m_ptrSGBpmIns8) && (!m_bChangeActState) )
		if( (m_ptrSGBpmIns||m_ptrSGBpmIns8)&&(!m_bRenderGlossy) )
		{
			render_smSwordGlossy( _dev );
			m_bRenderGlossy = true;

			//@{  River mod @ 2007-5-11:此处需要重设人物渲染的Declaration，否则下一个人物渲染的
			//    的时候，会出现非法的fvf警告或是错误。
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
	// 两个主要队列放到一起，然后sceneSm从这个队列的后面得到内存指针。
	m_vecDadded.resize( MAX_SMINVIWE );	
	m_vecInViewSM.resize( MAX_SMINVIWE );
	m_vecSceneSM.resize( MAX_SCENESMNUM/2 );

	m_vecRCBehindWall.resize( MAX_BEHINDWALLCHAR );
	for( int t_i = 0;t_i<MAX_BEHINDWALLCHAR;t_i ++ )
		m_vecRCBehindWall[t_i] = NULL;

	g_skinMeshMgr = this;

	// 初始化criticalSection.
	::InitializeCriticalSection( &m_sChrCreateCS );
	m_sCharLoadThread.Begin();

}

osc_skinMeshMgr::~osc_skinMeshMgr()
{

	m_sCharLoadThread.end_thread();
	m_sCharLoadThread.End();

	release_smManager( true );


	osc_skinMesh::release_postTrail();


	// 释放当前的顶点声明。
	SAFE_RELEASE( g_pVertexDeclaration3 );

	// ATTENTION TO FIX: 其它的临界区没有释放。
	::DeleteCriticalSection( &m_sChrCreateCS );

}


//! 设备相关数据的指针。
LPDIRECT3DDEVICE9 osc_skinMeshMgr::m_pd3dDevice = NULL;

//! 每一时刻mgr只能创建一个mesh.
CRITICAL_SECTION  osc_skinMeshMgr::m_sChrCreateCS;

//! 设备相关数据的指针。
LPDIRECT3DDEVICE9 osc_skinMesh::m_pd3dDevice = NULL;


//
//  ATTENTION OPP: 只需要创建一个Vertex deleration 就可以了。
//! 初始化当前的skinMeshManager.
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
	// 创建顶点声明，需要多个顶点声明，以对应顶点受不同骨骼数目影响的动画.
	// 只一个weight的情形。
	// 有三个weight的情形。修改了最终的插件,使场景中只出现有三个骨骼相关联的动画.
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
		osassertex( false,"创建顶点声明失败!!!\n" );
        return false;
    }

	// 
	// 得到我们以后运行过程中使用skinMesh的方法.
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

	// skinMesh内静态变量的赋值。
	osc_skinMesh::m_ptrVecDadded = &m_vecDadded;
	osc_skinMesh::m_ptrSceneSM = &m_vecSceneSM;
	osc_skinMesh::m_pd3dDevice = m_pd3dDevice;

	int t_m = sizeof( os_chaSecEffect );

	//! river @ 2010-12-22:预读取存储，为了效率.
	osc_skinMesh::m_vecSecEffect.resize( MAX_SMINVIWE*3 );
	osc_skinMesh::m_vecSecEffPreLoad.resize( MAX_SMINVIWE*3 );


	osc_skinMesh::init_postTrail();

	//! river @ 2010-12-22:预读取存储，为了效率.
	//! 初始化glossyIns用到的内存
	osc_bpmInstance::m_vecDGlossyIns.resize( MAX_SMINVIWE );
	osc_bpmInstance::m_vecChrRideIns.resize( MAX_SMINVIWE*2 );

	//! 后期可以扩大
	osc_bpmInstance::m_vecChrShaderBuf.resize( int(MAX_SMINVIWE*g_fCharacterTexBufScale) );

	// River mod @ 2009-11-4:初始化阴影调入的临界区.
	::InitializeCriticalSection( &g_sShadowSec );

	// River @ 2010-12-24:创建默认的刀光纹理.
	g_shaderMgr->add_shader( "glossy",osc_engine::get_shaderFile( "billBoard" ).c_str() );

	return true;

	unguard;
}



//! 释放当前skinMeshManager.
void osc_skinMeshMgr::release_smManager( bool _finalRelease/* = false*/ )
{
	guard;


	osDebugOut( "释放人物管理器..\n" );

	for( int t_i=0;t_i<MAX_RTSHADOWNUM;t_i ++ )
		m_ptrShadowSkinPtr[t_i] = NULL;

	// River mod @ 2009-11-4:初始化阴影调入的临界区.
	if( _finalRelease )
		::DeleteCriticalSection( &g_sShadowSec );

	//! 先释放场景中每一个使用的人物。
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


	// 如果不是最终退出程序，只要释放掉人物就ok,内存继续保留.
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

	osDebugOut( "人物管理器释放完成..\n" );

	unguard;
}


/** \brief
*  在初始化skinMeshMgr的时候,初始化我们的skinMethod.
*/
void osc_skinMeshMgr::init_skinMethod( void )
{
	guard;// osc_skinMeshMgr::init_skinMethod() );

	osc_d3dManager*   t_ptrDevmgr;

	t_ptrDevmgr = (osc_d3dManager*)::get_deviceManagerPtr();
	osassert( t_ptrDevmgr );
	
	// 
	// ATTENTION TO OPP: 查看IB的硬件支持度，最后可以注释掉Indexed Blending的代码。
	// 
	// 如果硬件支持Indexed Blend ,使用，否则使用软件的VertexShader.
	// 如果当前设备为软件顶点转化，则BlendMatrix数目不需要判断。
	// 上层已经做足了判断。
	g_eSkinMethod = OSE_D3DINDEXEDVS;

	//
	// 如果可以使用硬件加速,则使用VertexShader,场景中也可以渲染阴影。
	// 如果渲染选项强制使用第二层贴图，则使用HLSL
	if( (!t_ptrDevmgr->is_softwareVS())|| g_bForceSecEff )
		g_eSkinMethod = OSE_D3DINDEXEDVS;

	// 在shaderDebug模式下，使用VS.
	if( g_bShaderDebug )
		g_eSkinMethod = OSE_D3DINDEXEDVS;

	// TEST CODE:整体引擎原则上不再支持tnt级别的显卡，所以引擎内有关
	// OSE_D3DINDEXED的代码基本上没有意义，可以阶段性的删除。
	//g_eSkinMethod = OSE_D3DINDEXED;

	unguard;
}


//! d3dIndexedBlend渲染时用到的渲染状态块。
int osc_skinMeshMgr::m_iIndexedBlendSB = -1;
/** \brief
*  创建indexed Blend渲染时使用的渲染状态块。
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


//! 渲染smMgr前的渲染状态改变。
int osc_skinMeshMgr::m_iSetsmSB = -1;
//! 结束smMgr渲染时的状态改变。
int osc_skinMeshMgr::m_iEndSetsmSB = -1;


//! 创建smMgrRState的渲染块。
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
	// River @ 2004-6-30:为了解决人物面部的接缝问题,这样就限制了skinMesh
	//                   使用超过1.0或是小于0.0的u,v坐标.
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );
	//@} 

	// River @ 2005-6-20:alphaTest相关的代码，人物的透明部分必须使用alphaTest.
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
		// 使用Vertex Shader进行渲染.
	case OSE_D3DINDEXEDVS:
		{
			//
			//  设置统一的顶点声明，所有的skinMesh都使用三根骨骼的数据格式。
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
		// 使用indexed Blend进行渲染.
	case OSE_D3DINDEXED:
		break;
	default:
		osassert( false );
	}


	t_ptrMgr->end_stateBlock( m_iSetsmSB );

	return;

	unguard;
}

//! 创建结束smMgrRState的渲染块。
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

	// 使用Render State Block.
	// 如果是索引渲染，则关掉渲染状态。
	if( g_eSkinMethod == OSE_D3DINDEXED )
	{
		m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,D3DVBF_DISABLE );
		m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false );
	}


	//
	// ATTENTION TO FIX: 如果使用Effect文件，则渲染了当前Effect
	//                   文件的Pass后，D3D回复到了原来的渲染状态吗？
	// 
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_CCW );

	//! River @ 2010-1-29:加入char behindWall的渲染后状态回复。
	m_pd3dDevice->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );

	// 结果clamp的纹理渲染状态.
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );

	t_ptrMgr->end_stateBlock( m_iEndSetsmSB );

	unguard;
}



/** \brief
*  在渲染skinMeshMgr之前根据设备不同,设置渲染状态.
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
		// 使用Vertex Shader进行渲染.
	case OSE_D3DINDEXEDVS:
		break;
		// 
		// 使用indexed Blend进行渲染.
	case OSE_D3DINDEXED:
		{
			//
			// 设置软件渲染,我们只使用三根骨骼影响的顶点.
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
			// 全局光设置。
// 			m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,
// 				osc_middlePipe::m_dwAmbientColor );

		}
		break;
	default:
		osassert( false );
	}

	// 确认渲染状态块,并使用到设备.
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
*  在结束skinMeshMgr渲染的时候,设置的渲染状态.
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


//! 找到离传入位置最远的character索引,用于场景内人物过多时,只显示近处的人物.
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



//! 往管理器中加入可见的skinMesh物品。已经确认需要在场景中显示。
void osc_skinMeshMgr::add_sceneInviewSm( osc_skinMesh* _ptr )
{
	guard;

	// 
	osassert( _ptr );

	// 如果场景内可视的skinMesh过多，则直接返回。
	if( m_iInviewSMNum >= MAX_SMINVIWE )
	{
		// River Added @ 2007-6-20:
		// 找到场景内离相机最远的sm，然后比较当前要插入的sm离相机的矩离，使用近的替代远的
		float     t_fFarthestDis,t_fDis;
		osVec3D   t_vec3CamPos,t_vec3SmPos;
		int       t_iSmIdx;


		//! River @ 2011-2-14:以主角人物为中心，确认周围的人物.焦点即人物的中心。
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
 *  由qsort调用的函数指针，使用这个函数，比较两个SkinMesh物体离相机的远近。
 *  
 *  排序完成后，数组内的sm物品由近到远进行排列。
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
 *  得到在视域内的Character的列表。
 *  
 */
void osc_skinMeshMgr::get_inViewChrList( void )
{
	guard;

	osc_TGManager*     t_ptrSceMgr;

	t_ptrSceMgr = (osc_TGManager*)::get_sceneMgr();
	os_bsphere         t_bs;
	osVec3D            t_vec3Tmp;

	t_bs.radius = 3.0f;  // 每一模型在导出的时候应该计算这个sphere.

	//
	// 渲染所有用到的skinMesh
	CSpaceMgr<osc_skinMesh>::NODE  t_ptrNode;

	// 
	// ATTENTION TO OPP: 在此处处理，使用场景中新出现的人物可以有淡出状态。
	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		// River mod @ 2009-6-8:为了加入其它的处理。	
		if( t_ptrNode.p->m_bLoadLock )
			continue;

		// River @ 2009-11-15: TEST CODE判断出错原因.
		osassertex( t_ptrNode.p->m_bInUse,
			va( "PTR<0x%x>...\n",DWORD( t_ptrNode.p )  ) );

		//@{
		//  River @ 2010-6-3:确保场景内不会出现人物骑马资势的残留
		//! 如果当前是骑乘类的skin,则直接加入到InViewScene内，确保人物的位置更新
		if( t_ptrNode.p->m_HumObj )
		{
			add_sceneInviewSm( t_ptrNode.p );
			continue;
		}
		//@} 

		//! 如果上帧设置了此帧必须更新的变量，则更新。
		if( t_ptrNode.p->m_bMustUpdateNextFrame )
		{
			add_sceneInviewSm( t_ptrNode.p );
			continue;
		}


		t_bs.radius = t_ptrNode.p->get_smBSRadius();
		t_ptrNode.p->get_smPos( t_bs.veccen );

		// River added @ 2007-5-25:为了处理高偏移的龙的可视性问题
		t_ptrNode.p->get_adjPos( t_vec3Tmp );
		t_bs.veccen += t_vec3Tmp;

		// River mod @ 2008-8-16:
		// 根据人物的bbox，计算此人物的sphere radius
		os_bbox* t_ptrBox = t_ptrNode.p->get_smBBox();
		os_bsphere  t_s;
		t_ptrBox->to_bsphere( t_s );
		t_bs.radius = t_s.radius;


		// 只要人物在视域内,全部渲染.
		if( !t_ptrSceMgr->is_sphereInView( t_bs,false ) )
		{
			// River added @ 2006-6-13:使不在视域内的死亡动作可以播放
			t_ptrNode.p->play_lastFrameStopAction();

			// River @ 2006-7-7:只有连续两帧都不在视域内,才真正的不渲染此skinMesh,
			//                  为保证武器特效和刀光的正确渲染.
			if( t_ptrNode.p->m_bInViewLastFrame )
				t_ptrNode.p->m_bInViewLastFrame = false;
			else
				continue;

		}
		else
		{		
			// River mod @ 2008-8-16:加入更加严格的可视性测试
			// 目前检测当前的顶点是否在视域内。
			os_ViewFrustum*    t_frus = g_ptrCamera->get_frustum();
			if( !t_frus->objInVF( *t_ptrNode.p->get_smBBox() ) )
			{
				// River added @ 2006-6-13:使不在视域内的死亡动作可以播放
				t_ptrNode.p->play_lastFrameStopAction();

				// River @ 2006-7-7:只有连续两帧都不在视域内,才真正的不渲染此skinMesh,
				//                  为保证武器特效和刀光的正确渲染.
				if( t_ptrNode.p->m_bInViewLastFrame )
					t_ptrNode.p->m_bInViewLastFrame = false;
				else
					continue;
			}

			// 此处才到达真正的可见
			t_ptrNode.p->m_bInViewLastFrame = true;
		}

		add_sceneInviewSm( t_ptrNode.p );

	}

	// 
	// 得到我们可见sm物体列表后，根据这些东西离相机的远近，进行排序。
	// 每一个元素是一个skinMesh的指针，所以使用dword做为size.
	// 排序后得到顺序为由远到近。
	qsort( &m_vecInViewSM[0],m_iInviewSMNum,sizeof( DWORD ),alphaSM_compare );

	//
	// ATTENTION TO FIX: 
	// 把矩相机远到一定程序的skinMesh物品去掉，减少渲染压力。
	if( m_iInviewSMNum > MAX_INVIEWSM_NUM )
		m_iInviewSMNum = MAX_INVIEWSM_NUM;

	return;

	unguard;
}



//! 人物实时阴影的渲染
void osc_skinMeshMgr::shadow_draw( BOOL _meshWithoutShadow )
{
	guard;

	//
	// 此处的渲染，仅仅是为了正确的渲染阴影，与人物外观无观.
	// 此处只更新了人物阴影的正确位置，没有渲染人物的多边形
	for( int t_i=0;t_i<m_iRTShadowNum;t_i ++ )
	{
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice );
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice,true );
	}

 
	// 如果是索引渲染，则关掉渲染状态。
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
		// 在重新渲染人物，挡住人物身上的自阴影之前，重设人物的动作,如果此时动作没有刀光，则不需要重设
		if( m_ptrShadowSkinPtr[t_i]->m_bRenderGlossy && m_ptrShadowSkinPtr[t_i]->m_dwFinalGlossyColor )
			m_ptrShadowSkinPtr[t_i]->cal_finalPose();
		
	    m_ptrShadowSkinPtr[t_i]->start_renderPostTrail(m_ptrShadowSkinPtr[t_i]);
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice );	

	}

	// 打开alpha，正确渲染人物身上的透明部位。
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );
	for( int t_i=0;t_i<m_iRTShadowNum;t_i ++ ){
		m_ptrShadowSkinPtr[t_i]->start_renderPostTrail( m_ptrShadowSkinPtr[t_i] );
		m_ptrShadowSkinPtr[t_i]->render_skinMesh( m_pd3dDevice,true );
	}

	

	unguard;
}


//! frameMove当前的skin mesh Mgr.
bool osc_skinMeshMgr::framemove_smMgr( void )
{
	guard;

	float t_f = sg_timer::Instance()->get_lastelatime();

	// River @ 2010-9-28:对坐骑系统配对处理,WORK START:此处确实可能出错....
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
	// 进行一次性的处理。
	// 得到可见的sm列表并排序。
	get_inViewChrList();

	for( int t_i=0;t_i<m_iInviewSMNum;t_i++ )
	{
		// River @ 2010-9-1:人物动作frame_move出错，在此处加入assert代码.
		osassert( m_vecInViewSM[t_i] );
		m_vecInViewSM[t_i]->frame_moveAction( t_f );
		m_iRFaceNum += m_vecInViewSM[t_i]->get_smFaceNum();
	}

	return true;

	unguard;
}




//! River @ 2009-6-23:渲染fakeGlow效果
void osc_skinMeshMgr::render_fakeGlow( void )
{
	guard;

	//! River @ 2009-6-23:此处画整个场景内的人物描边数据和拖影数据
	//!                   必须有阴影数据，才能画这些，否则无效。
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
			// 之前推进来过，直接返回
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
	
	//osassertex( false,"没有足够的空间来容纳更多的behind WAll Char...\n" );

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

//！  River @ 2010-1-29: 渲染墙折挡物后的人物。
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
	// 对每一个指针进行处理
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
*   渲染当前的skinMeshManager.
*
*   \param _ralpha 当前渲染是否渲染有alpha通道的ani部分。
*	\param _meshOrShadow 如果是0的时候渲染 模型 + 阴影，1的时候渲染模型，2的时候渲染阴影
*
*   渲染的时候可以根据每一个sm物体离相机的远近进行排序。
*   渲染非透明物体部分由近到远，渲染透明部分由完到近。可能透明部分渲染完成后，
*   需要渲染透明部分的Zbuffer.
*   
*/
bool osc_skinMeshMgr::render_smMgr(  os_FightSceneRType _rtype/*= OS_COMMONRENDER*/,
			              BOOL _ralpha/* = false*/,BOOL _meshWithoutShadow /*= FALSE*/ )
{
	guard;

	osassert( m_pd3dDevice );

	int               t_iChrNum = 0;

	// River @ 2006-7-7:人物的不透明部分和透明部分渲染分开,所以此处需每次设置渲染状态.
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
		
		// 设置即时阴影的队列远元素指针为空
		for( int t_i=0;t_i<MAX_RTSHADOWNUM;t_i ++ )
			m_ptrShadowSkinPtr[t_i] = NULL;
		m_iRTShadowNum = 0;
	}

	

	if( _ralpha )
	{
		// 
		// 设置渲染状态。
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );	

		for( int t_i=m_iInviewSMNum-1;t_i>=0;t_i-- )
		{
			if( (!m_vecInViewSM[t_i]->m_bShadowSupport)||(!g_bHeroShadow)  ){
				m_vecInViewSM[t_i]->start_renderPostTrail((m_vecInViewSM[t_i]));
				m_vecInViewSM[t_i]->render_skinMesh( m_pd3dDevice,true );
				
			}else
				push_rtShadowSmPtr( m_vecInViewSM[t_i] );
		}

		// 重设所有的推入到管道中的人物阴影渲染.
		reset_rtShadow();
	}




	// 渲染主角人物的阴影。
	if( (_ralpha)&&(g_bHeroShadow))
	{
		if( (m_iRTShadowNum > 0) )
			shadow_draw( _meshWithoutShadow );
	}
	

	//
	// River @ 2006-7-7:人物的不透明部分和透明部分渲染分开,所以此处需每次
	// 退出渲染skeletal animation的时候,设置.
	// 如果硬件顶点转换,软件vs.
	endset_smMgrRState();


	//! river: fake Glow的渲染
	render_fakeGlow();

	return true;

	unguard;
}


//! 对某一个Character进行单独渲染，主要用于把三维人物渲染到二维的屏幕界面内
bool osc_skinMeshMgr::render_character( osc_skinMesh* _ptrMesh )
{
	guard;

	osassert( _ptrMesh );

	// 设置渲染状态.
	set_smMgrRState();

	_ptrMesh->render_skinMesh( m_pd3dDevice );

	// 
	// 设置渲染状态。
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );	
	_ptrMesh->render_skinMesh( m_pd3dDevice,true );

	endset_smMgrRState();

	return true;

	unguard;
}



//! 重设管理器中所有的即时阴影数据,使下次渲染的时候,更新阴影.
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
			// ATTENTION TO FIX: River @ 2009-12-8:应该不会再出现此类问题了。
			//                   上层在创建一个人物的同时，又释放这个人物，
			//                   loadLock在底层没有做相应的处理，倒致此类现象的发生。
			if( !t_ptrShadow )
			{
				// 再次确认，是否此skinPtr为要释放的skinPtr.
				osassertex( (m_ptrShadowSkinPtr[t_idx]->m_Look.m_mesh[t_i] == 0),
					va( "阴影出现问题 mesh idx<%d>..parts idx<%d>.skinName<%s>.SkinPtr<0x%x>..\n",
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


//! 每一帧动skinmeshMGR设置。
void osc_skinMeshMgr::frame_setSmMgr( void )
{
	guard;

	m_iInviewSMNum = 0;
	m_iRFaceNum = 0;

	//osDebugOut( "The skinmesh vector size is<%d>...\n",m_vecDadded.size() );


# if !__QTANG_VERSION__
	// 此处代码，用于处理全3D的人物使用。
	// River @ 2009-4-16:因为客户端可以限制相机的角度，所以
	// 此处的处理可以省掉？测试有问题的话再想别的解决办法
	//! 检测有阴影的人物指针,如果跟相机的剪切面bsphere碰撞,则不显示
	for( int t_i=0;t_i<MAX_RTSHADOWNUM;t_i ++ )
	{
		if( !m_ptrShadowSkinPtr[t_i] )
			continue;

		os_bbox*     t_ptrBBox;
		os_bsphere   t_bs;
		
		osassertex( 
			_CrtIsValidPointer( m_ptrShadowSkinPtr[t_i],sizeof( osc_skinMesh ),true ),
			"非法的人物阴影指针...\n" );

		t_ptrBBox = m_ptrShadowSkinPtr[t_i]->get_smBBox();

		g_ptrCamera->get_camNearClipBSphere( t_bs );
		if( t_ptrBBox->collision( t_bs ) )
			m_ptrShadowSkinPtr[t_i]->set_hideSkinMesh( true );
		else
		{
			// 如果要全面显示人物,则必须有更大的碰撞缓冲空间.
			t_bs.radius *= 1.05f;		

			if( !t_ptrBBox->collision( t_bs ) )
				m_ptrShadowSkinPtr[t_i]->set_hideSkinMesh( false );
		}
	}
# endif 

	unguard;
}





