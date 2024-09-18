//===============================================================================
/*! \file
*	Filename:	galaEff.cpp
*
*	Desc:		this file contain the base class CGalaEff 
*				CGalaEff manage all effect of Galaxy game using VS and PS 
*				it's main mechanism is rendering to texture and redraw it
*				by a rectangle fill up the screen
*
*	His:		tzz	2008-7-3
*/
//===============================================================================


# include "stdafx.h"
# include ".\galaeff.h"
# include "..\interface\osInterface.h"
# include ".\galaEffect_common.h"



//====================================================================================================================
//
//	 CGalaEff class 
//
//====================================================================================================================

//static variable
int CGalaEff::sm_iWidth = 0;
int CGalaEff::sm_iHeight = 0;
D3DFORMAT CGalaEff::sm_format = D3DFMT_A8R8G8B8;
BOOL CGalaEff::sm_bUseFSAA	= FALSE;

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------

CGalaEff::CGalaEff(void) :m_lpMainSurface(NULL),
						m_lpDecl(NULL),
						m_lpEffect(NULL),
						m_bEffective(FALSE),
						m_lpVertBuf(NULL),
						m_bInit(FALSE),
						m_lpBackupDepthStencil(NULL)						
{
	// noting to do now
}
CGalaEff::~CGalaEff(void)
{
	
	// because the derive class call the DestroyEffRes() function
	// which is virtual function and will call the derive one
	// so we needn't call DestroyEffRes() function here
	
	//DestroyEffRes();

}

//--------------------------------------------------------------------------------------------------------------------
//Name: DestroyGrowRes
//Desc: destroy the d3d resource
//--------------------------------------------------------------------------------------------------------------------
void CGalaEff::DestroyEffRes(void)
{
	SAFE_RELEASE(m_lpEffect);
	SAFE_RELEASE(m_lpMainSurface);
	SAFE_RELEASE(m_lpVertBuf);
	SAFE_RELEASE(m_lpDecl);
	SAFE_RELEASE(m_lpBackupDepthStencil);
}

//--------------------------------------------------------------------------------------------------------------------
//Name: InitEffScene
//Desc: initialize the base class' data
//--------------------------------------------------------------------------------------------------------------------
void CGalaEff::InitEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format /*=  D3DFMT_A8R8G8B8*/,BOOL _bUseFSAA /*= FALSE*/)
{
	guard;

	m_bInit = TRUE;

	// set the width and height of screen
	sm_iWidth		= _iWidth;
	sm_iHeight		= _iHeight;
	sm_format		= _format;
	sm_bUseFSAA		= _bUseFSAA;

	
	//store the main surface of render target 
	SAFE_RELEASE(m_lpMainSurface);
	
	I_deviceManager*   t_ptrMgr = ::get_deviceManagerPtr();

	t_ptrMgr->get_mainRenderTarget( m_lpMainSurface );

	/*
	if(FAILED(_lpd3dDev->GetRenderTarget(0,&m_lpMainSurface))){
		osassert(false);
	}
	*/

	osassert(_iWidth > 0 && _iHeight >0);

	unguard;
}


//--------------------------------------------------------------------------------------------------------------------
//Name: InitEffect
//Desc: initialize some shader interface
//--------------------------------------------------------------------------------------------------------------------
void CGalaEff::InitEffect(LPDIRECT3DDEVICE9 _lpd3dDev,LPCTSTR _filename)
{
	guard;
	
	DWORD dwShaderFlags = NULL ;// = D3DXFX_NOT_CLONEABLE  ;

    #if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DXSHADER_DEBUG;
    #endif

    #ifdef DEBUG_VS 
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

	LPD3DXBUFFER	t_error = NULL;

	// Read the Effect file into tmp buffer.
	//
	CReadFile t_file;
	if(!t_file.OpenFile(_filename)){
		osassert(false);
	}
	
	if(FAILED(D3DXCreateEffect(_lpd3dDev,t_file.GetFullBuffer(),t_file.GetFileSize(),NULL,NULL,NULL,NULL,&m_lpEffect,&t_error))){
		osassertex( false,va("Effect<%s> compile error<%s>..\n",_filename,(char*)(t_error->GetBufferPointer())));
	}

	t_file.CloseFile();

	//declaration
	D3DVERTEXELEMENT9 t_decl[] = 
	{
		//position
		{0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},	
		
		//diffuse
		{0,12,D3DDECLTYPE_D3DCOLOR,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_COLOR,0},	

		//texture coordinate
		{0,16,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TEXCOORD,0},	

		D3DDECL_END()
	};

	//create the declaration
	if(FAILED(_lpd3dDev->CreateVertexDeclaration(t_decl,&m_lpDecl))){
		osassert(false);
	}
	osassert(m_lpDecl);

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: CreateRect
//Desc: create the rectangle rendered finally in ortho camera
//--------------------------------------------------------------------------------------------------------------------
void CGalaEff::CreateRect(LPDIRECT3DDEVICE9 _lpd3dDev)
{
guard;

	osassert(sm_iWidth && sm_iHeight);

	const int t_iLen = 4 * sizeof(MyVertex);
	
	VALIDATE_D3D(_lpd3dDev->CreateVertexBuffer(4 * sizeof(MyVertex),
											D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
											D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_TEX2 ,
											D3DPOOL_DEFAULT,
											&m_lpVertBuf,NULL));

	osassert(m_lpVertBuf);

	MyVertex*	t_pBuffer = NULL;
	VALIDATE_D3D(m_lpVertBuf->Lock(0,t_iLen,(void**)&t_pBuffer,0));
	
	const float t_width			= (float)sm_iWidth - 0.5f;
	const float t_height		= (float)sm_iHeight + 0.5f;
	
	//left-bottom corner position
	const float t_pos_x			= 0.0f;
	const float t_pos_y			= t_height;
	const float t_pos_u0		= 0.0f;
	const float t_pos_v0		= 0.0f;

	

	//delta of rectangle offset
	const float t_pos_delta_u	= 1.0f;
	const float t_pos_delta_v	= 1.0f;

	//top-left
	t_pBuffer[0].color		= e_rectColor;
	t_pBuffer[0].position.x = t_pos_x ;
	t_pBuffer[0].position.y = t_pos_y;
	t_pBuffer[0].position.z = 1.0f;
	t_pBuffer[0].tu0		= t_pos_u0;
	t_pBuffer[0].tv0		= t_pos_v0 ;
	
	//top-right
	t_pBuffer[1].color		= e_rectColor;
	t_pBuffer[1].position.x = t_pos_x; 
	t_pBuffer[1].position.y = t_pos_y - t_height;
	t_pBuffer[1].position.z = 1.0f;
	t_pBuffer[1].tu0		= t_pos_u0;
	t_pBuffer[1].tv0		= t_pos_v0 + t_pos_delta_v;
	

	//bottom-left
	t_pBuffer[2].color		= e_rectColor;
	t_pBuffer[2].position.x = t_pos_x + t_width;
	t_pBuffer[2].position.y = t_pos_y ;
	t_pBuffer[2].position.z = 1.0f;
	t_pBuffer[2].tu0		= t_pos_u0 + t_pos_delta_u;
	t_pBuffer[2].tv0		= t_pos_v0 ;
	

	//bottom-right
	t_pBuffer[3].color		= e_rectColor;
	t_pBuffer[3].position.x = t_pos_x + t_width;
	t_pBuffer[3].position.y = t_pos_y - t_height;
	t_pBuffer[3].position.z = 1.0f;
	t_pBuffer[3].tu0		= t_pos_u0 + t_pos_delta_u;
	t_pBuffer[3].tv0		= t_pos_v0 + t_pos_delta_v;
	
	VALIDATE_D3D(m_lpVertBuf->Unlock());

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: PrepareEffScene
//Desc: prepare the effect scene ( view project matrix and viewport variable)
//--------------------------------------------------------------------------------------------------------------------
void CGalaEff::PrepareEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;
	
	osassert(sm_iWidth > 0);

	// set the device and vertex buffer and prepare to render it
	VALIDATE_D3D(_lpd3dDev->SetVertexDeclaration(m_lpDecl));
	//VALIDATE_D3D(_lpd3dDev->SetFVF(0));
	VALIDATE_D3D(_lpd3dDev->SetStreamSource(0,m_lpVertBuf,0,sizeof(MyVertex)));

	// set the camera
	D3DXMATRIX	t_view;
	D3DXMATRIX	t_project;

	const float	t_fWidth	= (float)sm_iWidth;
	const float	t_fHeight	= (float)sm_iHeight;
	
	VALIDATE_D3D(SetOrthoCamera(_lpd3dDev,
								D3DXVECTOR3(t_fWidth / 2.0f,t_fHeight / 2.0f,0.0f),
								D3DXVECTOR3(t_fWidth / 2.0f,t_fHeight / 2.0f,1.0f),
								t_fWidth,
								t_fHeight,
								0.0f,
								10000.0f,
								&t_view,
								&t_project));

	//D3DXMATRIX t_test = t_view * t_project ;
	VALIDATE_D3D(m_lpEffect->SetMatrix("g_mat",&( t_view * t_project )));

	//set the viewport size 
	const float t_viewport[2] = {(float)sm_iWidth,(float)sm_iHeight};
	VALIDATE_D3D(m_lpEffect->SetFloatArray("g_viewportSize",t_viewport,2));

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: RenderToTex
//Desc:  render to the texture...
//--------------------------------------------------------------------------------------------------------------------
void CGalaEff::RenderToTex(LPDIRECT3DDEVICE9 _lpd3dDev,LPDIRECT3DSURFACE9 _lpRenderTarget)
{
	
	VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,_lpRenderTarget));
	
	//render it 
	VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));

}