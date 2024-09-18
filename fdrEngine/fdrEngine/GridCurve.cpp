//===============================================================================
/*! \file
*	Filename:	findpath.cpp	
*	Desc:		the CFindPath class' defining file
*	His:		tzz	2008-5-5
*/
//===============================================================================
#include "stdafx.h"
# include ".\gridCurve.h"

# include ".\galaEffect_common.h"


//! first difference of scaling to initializie 
//! m_fDiffScale variable
#define FIRST_DIFF_SCALE					0.05f

//! the interval time
#define INTERVAL_TIME						0.01f;

//! the z distance between two layer
#define	LAYER_FACTOR						10.0f

//! reference z value(initializing value)
#define REFERENCE_Z							(float)rand()

//! space speed
#define SPACE_HOLE_SPEED					2.0f

//! space hole time
#define SPACE_HOLE_TIME						1.5f

//! the disappearing velocity of  fog after throughing hole 
#define FOG_DISAPPEAR_VELOCITY				10



//====================================================================================================================
//
//	 CSpaceHole class 
//
//====================================================================================================================

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CSpaceHole::CSpaceHole(void):m_lpIndexBuf(NULL),
							m_lpVertBuf(NULL),
							m_lpVertBufBackup(NULL),
							m_fScale(1.0f),
							m_fCurrScale(0.1f),
							m_fOffset(0.0f),
							m_fCounter(0.0f),
							m_fRefZ(REFERENCE_Z),
							m_fDiffScale(FIRST_DIFF_SCALE),
							m_lpTex(NULL),
							m_lpTex2(NULL),
							m_dwLight(0),
							m_bHoleEnd(FALSE)
							
{
	
}
CSpaceHole::~CSpaceHole(void)
{
	DestroySpaceHole();
}

//--------------------------------------------------------------------------------------------------------------------
//Name: InitSpaceHole
//Desc: initialize the space hole(index buffer)
//--------------------------------------------------------------------------------------------------------------------
void CSpaceHole::InitSpaceHole(LPDIRECT3DDEVICE9 _lpd3dDev,D3DXVECTOR3& _vecWorldPos)
{
	guard;

	osassert(_lpd3dDev && !m_lpVertBuf);

	// set the world position
	m_vecWorldPos.x = 0.0f;
	m_vecWorldPos.y = 0.0f;
	m_vecWorldPos.z = 0.0f;

	//camera original position
	m_vecCamPosBackup.x = m_vecCamPos.x	= 0.0f;
	m_vecCamPosBackup.y = m_vecCamPos.y	= 0.0f;
	m_vecCamPosBackup.z = m_vecCamPos.z	= -1.0f;

	//camera original target position
	m_vecCamTargetBackup.x = m_vecCamTarget.x =0.0f;
	m_vecCamTargetBackup.y = m_vecCamTarget.y = 0.0f;
	m_vecCamTargetBackup.z = m_vecCamTarget.z = 2.0f;

	// get the random direction of space hole
	// because the fsincos instrucment to optimize the sin/cos function
	// write it in assembly
	float t_agl = (float)rand() ;
	__asm{
		mov		ecx,this
		fld		t_agl
		fsincos	
		fstp	[ecx]CSpaceHole.m_fRotAgl_cos
		fstp	[ecx]CSpaceHole.m_fRotAgl_sin
	}
	// make the sin value positive
	m_fRotAgl_sin = abs(m_fRotAgl_sin);

	// caution!!
	// the final vertex of every cycle is the same
	// as the first one, otherwise uv error will occur 
	// so we must increase a vertex in every cycle (check the value of scm_iTotalPoint)
	if(FAILED(_lpd3dDev->CreateVertexBuffer(scm_iTotalPoint * sizeof(MyVertex),
											0,
											e_myVertexUse,
											D3DPOOL_MANAGED,
											&m_lpVertBuf,
											NULL))){
		osassert(false);
	}
	if(FAILED(_lpd3dDev->CreateVertexBuffer(scm_iTotalPoint * sizeof(MyVertex),
											0,
											e_myVertexUse,
											D3DPOOL_MANAGED,
											&m_lpVertBufBackup,
											NULL))){
		osassert(false);
	}
	osassert(m_lpVertBuf);
	
	// fill the vertex buffer
	MyVertex* t_pBuffer;
	if(FAILED(m_lpVertBuf->Lock(0,scm_iTotalPoint * sizeof(MyVertex),(void**)&t_pBuffer,0))){
		osassert(false);
	}
	MyVertex* t_pBufferBackup;
	if(FAILED(m_lpVertBufBackup->Lock(0,scm_iTotalPoint * sizeof(MyVertex),(void**)&t_pBufferBackup,0))){
		osassert(false);
	}
	
	// create the vertexes of every layer
	for(int i= 0 ; i< scm_iLayer; i++){
		FillLayerVertex(i,t_pBuffer + (i * (scm_iCyclePoint + 1)));
	}
	memcpy(t_pBufferBackup,t_pBuffer,scm_iTotalPoint * sizeof(MyVertex));
	
	m_lpVertBufBackup->Unlock();
	m_lpVertBuf->Unlock();

	osassert(!m_lpIndexBuf);
	if(FAILED(_lpd3dDev->CreateIndexBuffer(scm_iTotalIndexNum * 2,
											0,
											D3DFMT_INDEX16,
											D3DPOOL_MANAGED,
											&m_lpIndexBuf,
											NULL))){
		osassert(false);
	}
	osassert(m_lpIndexBuf);

	WORD*	t_pIndexBuf = NULL;
	if(FAILED(m_lpIndexBuf->Lock(0,scm_iTotalIndexNum * 2 ,(void**)&t_pIndexBuf,NULL))){
		osassert(false);
	}

	//fill every cycle
	for(int layer = 0;layer <  scm_iLayer - 1;layer++){
		FillIndexTriangle(layer,t_pIndexBuf + (layer * (scm_iCyclePoint * 6) ));
	}

	m_lpIndexBuf->Unlock();

	//! read the file to create the texture
	if(D3DXCreateTextureFromFile(_lpd3dDev,"ui\\galaxy\\spaceHole_512_512.jpg",&m_lpTex)){
		osassert(false);
	}
	osassert(m_lpTex);
	if(D3DXCreateTextureFromFile(_lpd3dDev,"ui\\galaxy\\wrapLine_512_512.jpg",&m_lpTex2)){
		osassert(false);
	}
	osassert(m_lpTex2);

	unguard;

}

//--------------------------------------------------------------------------------------------------------------------
//Name: FillLayerVertex
//Desc: Create the vertex of every layer
//--------------------------------------------------------------------------------------------------------------------
void CSpaceHole::FillLayerVertex(int _iLayer,MyVertex* _pVertBuf)
{	
	
	// max Radius of cylinder
	const float t_fMaxRadius	= 3.0f;

	// radius factor to modify the radius
	const float t_fRadiusFactor = 0.25f;

	// layer factor to modify z value of vertexes
	const float t_fLayerFactor	= LAYER_FACTOR;

	// the z value;
	const float	t_fCurrZ		= _iLayer * t_fLayerFactor ;

	// uv coordinate ( bottom-left corner)
	const float t_u				= 0.0f;
	const float t_v				= 0.0f;

	//difference of uv mapping
	const float t_delta_u		= 1.0f / scm_iCyclePoint;
	const float t_delta_v		= 1.0f / (float)(scm_iLayer - 1) * (float)(_iLayer - 1);//pow(,4) * pow((float)(_iLayer - 1),4);

	//the max radius this layer
	const float t_fCurrRad = t_fMaxRadius - ((float)(_iLayer) * t_fRadiusFactor) ;

	float t_cos = 1.0f;
	float t_sin = 0.0f;
	const float t_theta_delta = 2 * OS_PI / scm_iCyclePoint;
	float t_theta = -t_theta_delta;
	
	for(int i = 0; i < scm_iCyclePoint;i++){
		t_theta += t_theta_delta;
		if( i != 0){
			__asm{
				fld		t_theta
				fsincos
				fstp	t_cos
				fstp	t_sin
			}
		}
		
		_pVertBuf[i].position.x = t_fCurrRad * t_cos;
		_pVertBuf[i].position.y = t_fCurrRad * t_sin;
		_pVertBuf[i].position.z = t_fCurrZ;
		_pVertBuf[i].color		= 0xffffffff;
		_pVertBuf[i].tu0		= t_u + t_delta_u * i ;
		_pVertBuf[i].tv0		= t_v + t_delta_v ;
    }

	//fill the final vertex which is same as first one
	_pVertBuf[scm_iCyclePoint].position.x	= _pVertBuf[0].position.x;
	_pVertBuf[scm_iCyclePoint].position.y	= _pVertBuf[0].position.y;
	_pVertBuf[scm_iCyclePoint].position.z	= t_fCurrZ;
	_pVertBuf[scm_iCyclePoint].color		= 0xffffffff;
	_pVertBuf[scm_iCyclePoint].tu0			= t_u +1.0f;
	_pVertBuf[scm_iCyclePoint].tv0			= t_v + t_delta_v ; 

}
//--------------------------------------------------------------------------------------------------------------------
//Name: FillIndexTriangle
//Desc: fill index triangle ring of cylinder
//--------------------------------------------------------------------------------------------------------------------
void CSpaceHole::FillIndexTriangle(int _iCycleRing,WORD* _wRingIndex)
{
	const int t_iCycleBegin		= _iCycleRing * (scm_iCyclePoint + 1);
	int	t_iVertIndex = 0;
	for(int i = 0 ;
		t_iVertIndex < scm_iCyclePoint ; 
		i+=6,t_iVertIndex++){

		// one triangle
		_wRingIndex[i]		= t_iCycleBegin + t_iVertIndex;
		_wRingIndex[i+2]	= t_iCycleBegin + t_iVertIndex + 1;
		_wRingIndex[i+1]	= t_iCycleBegin +	
								(scm_iCyclePoint + 1) + // the next cycle's vertex
								t_iVertIndex;

		// two triangle
		_wRingIndex[i+3]	= _wRingIndex[i+2];
		_wRingIndex[i+4]	= _wRingIndex[i+1];
		_wRingIndex[i+5]	= t_iCycleBegin + 1	+
								(scm_iCyclePoint + 1) + // the next cycle's vertex
								t_iVertIndex;
	}
}


//--------------------------------------------------------------------------------------------------------------------
//Name: RenderFrame
//Desc: render every frame
//--------------------------------------------------------------------------------------------------------------------
BOOL CSpaceHole::RenderFrame(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	SetCamera(_lpd3dDev,
			m_vecCamPosBackup,
			m_vecCamTargetBackup,
			20.0f,
			4.0f/3.0f,
			0.0f,
			10000.0f);
	
	/*const float t_scaleInterval = SPACE_HOLE_TIME;

	if(m_fCounter >= t_scaleInterval){
		m_fScale = 80.0f;
		m_fDiffScale = 2.0f;
	}

	if(m_fCurrScale <= m_fScale){
		D3DXMATRIX	t_scaleMat;
		D3DXMatrixIdentity(&t_scaleMat);
		D3DXMatrixScaling(&t_scaleMat,m_fCurrScale,m_fCurrScale,1.0f);

		m_fCurrScale += m_fDiffScale;
		_lpd3dDev->SetTransform(D3DTS_WORLD,&t_scaleMat);
	}
	if(m_fCurrScale > m_fScale && m_fCounter > t_scaleInterval){
		return FALSE;
	}*/
	if(!m_bHoleEnd){

		ProcessCylinder();
		
	}else{

		// we will move the cylinder to the far from
		// positive

		D3DXMATRIX	t_mat;
		D3DXMatrixIdentity(&t_mat);
		const float t_speed = 2.0f;
		m_vecWorldPos.z -= t_speed; 
		if(m_vecWorldPos.z < -100.0f){
			return FALSE;
		}
		D3DXMatrixTranslation(&t_mat,m_vecCamPosBackup.x,m_vecCamPosBackup.y,m_vecWorldPos.z);
		_lpd3dDev->SetTransform(D3DTS_WORLD,&t_mat);

		// change the clear color so that see the dawn light
		const DWORD	t_dwDiffColor = 5;
		
		m_dwLight	+= t_dwDiffColor;

		if(m_dwLight > 255){
			m_dwLight = 255;
		}

	}
	_lpd3dDev->SetStreamSource(0,m_lpVertBufBackup,0,sizeof(MyVertex));
	
	_lpd3dDev->SetFVF(e_myVertexUse);
	if(FAILED(_lpd3dDev->SetIndices(m_lpIndexBuf))){
		osassert(false);
	}
	
	if(FAILED(_lpd3dDev->Clear(0,NULL,D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,D3DCOLOR_XRGB(m_dwLight,m_dwLight,m_dwLight),0.0f,0))){
		osassert(false);
	}
	_lpd3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_GREATER);
	_lpd3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	if(FAILED(_lpd3dDev->SetRenderState(D3DRS_ZENABLE,TRUE))){
		osassert(false);
	}


	_lpd3dDev->SetTexture(0,m_lpTex2);

	_lpd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD);//( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    _lpd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	_lpd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	_lpd3dDev->SetTextureStageState( 0,D3DTSS_TEXCOORDINDEX,0);

	_lpd3dDev->SetTexture(1,m_lpTex);

	_lpd3dDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_ADD );//( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    _lpd3dDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	_lpd3dDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
	_lpd3dDev->SetTextureStageState( 1,D3DTSS_TEXCOORDINDEX,0);

	_lpd3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2 );
	_lpd3dDev->SetTextureStageState(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2 );


	//@{
	// velocity of rotate
	const float	t_fRotVelocity = 0.01f;

	m_fOffset += t_fRotVelocity;

	D3DXMATRIX t_mat;
	D3DXMatrixIdentity(&t_mat);
	D3DXMatrixIdentity(&t_mat);
	t_mat._31 = m_fOffset * (-0.5f);
	t_mat._32 = m_fOffset;

	_lpd3dDev->SetTransform(D3DTS_TEXTURE0,&t_mat);

	t_mat._31 = m_fOffset * (0.5f);

	_lpd3dDev->SetTransform(D3DTS_TEXTURE1,&t_mat);

	//@}

	if(FAILED(_lpd3dDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,scm_iTotalPoint,0,scm_iTriangleNum ))){
		osassert(false);
	}
	_lpd3dDev->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                 D3DTTFF_DISABLE );

	_lpd3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);

	D3DXMatrixIdentity(&t_mat);
	_lpd3dDev->SetTransform(D3DTS_WORLD,&t_mat);


	m_fCounter += INTERVAL_TIME;
	
	// the reference z value was going on moving
	// from the positive z to negative
	m_fRefZ -= SPACE_HOLE_SPEED;

	return TRUE;

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: ProcessCylinder
//Desc: process the vertex fo cylinder
//--------------------------------------------------------------------------------------------------------------------
BOOL CSpaceHole::ProcessCylinder(void)
{
	guard;

	osassert(m_lpVertBuf);

	MyVertex* t_buffer = NULL;
	if(FAILED(m_lpVertBuf->Lock(0,scm_iTotalPoint * sizeof(MyVertex),(void**)&t_buffer,0))){
		osassert(false);
	}
	MyVertex* t_bufferBackup = NULL;
	if(FAILED(m_lpVertBufBackup->Lock(0,scm_iTotalPoint * sizeof(MyVertex),(void**)&t_bufferBackup,0))){
		osassert(false);
	}
	//transform every layer's vertexes
	for(int layer = 0 ;layer < scm_iLayer;layer ++){
		TransformLayerVert(layer,t_buffer + layer * ( scm_iCyclePoint + 1),t_bufferBackup + layer* (scm_iCyclePoint +1));
	}
	
	m_lpVertBufBackup->Unlock();
	m_lpVertBuf->Unlock();
	
	return TRUE;

	unguard;
}


//--------------------------------------------------------------------------------------------------------------------
//Name: ProcessCylinder
//Desc: process the vertex fo cylinder
//--------------------------------------------------------------------------------------------------------------------
void CSpaceHole::TransformLayerVert(int _iLayer,MyVertex* _bufferScr,MyVertex* _bufferDest)
{
	//guard;
	
	const float	e				= 2.718281828f;

	// beta(damp)
	const float beta			= 2.5f;

	// min swing to clear the shaking state
	const float t_fMinSwing		= 0.5f;

	//the constant swing
	const float t_fSwing		= 60.0f * pow(e,-beta * (m_fCounter));

	if(t_fSwing < t_fMinSwing){
		m_bHoleEnd = TRUE;
		return;
	}

	//the frequence
	const float t_fFrequ		= OS_PI / 200.0f;

	// the z and time value;
	const float	t_fCurrZ		= (m_fRefZ - _iLayer * LAYER_FACTOR) *(t_fFrequ) ;//(m_fCounter + _iLayer) *  ;

	if(/*abs(t_fCurrZ) > t_fMinSwing */1){
		float t_sin  = sin(t_fCurrZ);

		//current y
		const float t_fCurrY	= t_sin * t_fSwing;
		
		if(/*t_fCurrY > 0*/1){
			for(int i = 0; i< (scm_iCyclePoint + 1);i++){
				_bufferDest[i].position.x = _bufferScr[i].position.x + t_fCurrY * m_fRotAgl_cos;
				_bufferDest[i].position.y = _bufferScr[i].position.y + t_fCurrY * m_fRotAgl_sin;
			}
		}else{
			for(int i = 0; i< (scm_iCyclePoint + 1);i++){
				_bufferDest[i].position.x = _bufferScr[i].position.x - t_fCurrY * m_fRotAgl_cos;
				_bufferDest[i].position.y = _bufferScr[i].position.y - t_fCurrY * m_fRotAgl_sin;
			}
		}
	
		//set the camera position and target when set the first layer's vertexes
		if(_iLayer == 0){
			if(/*t_fCurrY > 0*/1){
				m_vecCamPosBackup.x		= t_fCurrY * m_fRotAgl_cos;
				m_vecCamPosBackup.y		= t_fCurrY * m_fRotAgl_sin;
				
				m_vecCamTargetBackup.x	= t_fCurrY * m_fRotAgl_cos ;
				m_vecCamTargetBackup.y  = t_fCurrY * m_fRotAgl_sin;
			}else{
				m_vecCamPosBackup.x		= -t_fCurrY * m_fRotAgl_cos;
				m_vecCamPosBackup.y		= -t_fCurrY * m_fRotAgl_sin;

				m_vecCamTargetBackup.x	= -t_fCurrY * m_fRotAgl_cos;
				m_vecCamTargetBackup.y  = -t_fCurrY * m_fRotAgl_sin;
			}
		}
	}
	//unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: DestroySpaceHole
//Desc: Destroy the d3d resource
//--------------------------------------------------------------------------------------------------------------------
void CSpaceHole::DestroySpaceHole(void)
{
	//guard;

	SAFE_RELEASE(m_lpVertBuf);
	SAFE_RELEASE(m_lpVertBufBackup);

	SAFE_RELEASE(m_lpIndexBuf);

	SAFE_RELEASE(m_lpTex);
	SAFE_RELEASE(m_lpTex2);

	m_fDiffScale		= FIRST_DIFF_SCALE;
	m_fCurrScale		= 0.1f;
	m_fScale			= 1.0f;
	m_fCounter			= 0.0f;
	m_fOffset			= 0.0f;
	m_fRefZ				= REFERENCE_Z;

	m_bHoleEnd			= FALSE;
	m_dwLight			= 0;

	//unguard;
}

//====================================================================================================================
//
//	 CGridRect class 
//
//====================================================================================================================

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CGridRect::CGridRect(void):m_lpBgTexture(NULL),
							m_lpOldSurface(NULL),
							m_lppVertBuff(NULL),
							m_iWidthNum(0),
							m_iHeightNum(0),
							m_iWidth(0),
							m_iHeight(0),
							m_iGridWidth(0),
							m_iGridHeight(0),
							m_lpBgTexture2(NULL),
							m_lpFogVertexBuf(NULL),
							m_bReadyToRender(FALSE),
							m_bThroughHole(FALSE),
							m_bFirstRender(FALSE),
							m_bGridOver(FALSE),
							m_dwCurrFogLevel(255)
{
	//nothing to do now
}
CGridRect::~CGridRect(void)
{
	DestroyGridRes();
}
//--------------------------------------------------------------------------------------------------------------------
//Name: InitScreenWH
//Desc: initialize the width and height of whole screen
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::InitScreenWH(int _iScreenWidth,int _iScreenHeight)
{
	guard;

	osassert(_iScreenWidth > 0 && _iScreenHeight > 0);

	m_iWidth	= _iScreenWidth;
	m_iHeight	= _iScreenHeight;
    
	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: InitGridRes
//Desc: create all rectangle grid 
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::InitGridRes(LPDIRECT3D9 _lp3d,LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY)
{
	guard;

	//param test
	osassert(_lpd3dDev && (_iWidthNum >= 2) && (_iHeightNum >= 2));
	//precondition test
	osassert(m_iWidth > 0 && m_iHeight >0);
	//prevent memory leaking
	osassert(m_iWidthNum == 0);

	// initialize the space hole's data
	m_spaceHole.InitSpaceHole( _lpd3dDev,D3DXVECTOR3((float)_iStartX/(float)m_iWidth,(float)_iStartY/(float)m_iHeight,0.0f));
	
	m_iWidthNum		= _iWidthNum;
	m_iHeightNum	= _iHeightNum;
	m_iStartX		= _iStartX;
	m_iStartY		= _iStartY;

	//calculate the grid's width and height and amount
	m_iGridWidth	= m_iWidth / _iWidthNum;
	m_iGridHeight	= m_iHeight / _iHeightNum;
	m_iGridNum		= _iWidthNum * _iHeightNum;


	//create every grids
	m_lppVertBuff = new LPDIRECT3DVERTEXBUFFER9[m_iGridNum];
	for(int i = 0; i< m_iGridNum; i++){
		if(!CreateRect(_lpd3dDev,i)){
			osassert(false);
		}
	}
	
	D3DDISPLAYMODE t_d3ddm={0};
	
	//get the display mode to initializing
	if(FAILED(_lp3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&t_d3ddm))){
		osassert(false);
	}

	//create the second texture
	if(FAILED(D3DXCreateTextureFromFile(_lpd3dDev,"ui\\galaxy\\spaceHole_init_512_512.jpg",&m_lpBgTexture2))){
		osassert(false);
	}
	osassert(m_lpBgTexture2);
    
	//set the old and new surface 
	osassert(!m_lpOldSurface);
	if(FAILED(_lpd3dDev->GetRenderTarget(0,&m_lpOldSurface))){
		osassert(false);
	}

	// create the texture for render target
	if(FAILED(_lpd3dDev->CreateTexture(m_iWidth,m_iHeight,1,D3DUSAGE_RENDERTARGET,t_d3ddm.Format,D3DPOOL_DEFAULT,&m_lpBgTexture,NULL))){
		osassert(false);
	}
	osassert(m_lpBgTexture);
	LPDIRECT3DSURFACE9 t_texSurface = NULL;
	m_lpBgTexture->GetSurfaceLevel(0,&t_texSurface);

	if(FAILED(_lpd3dDev->SetRenderTarget(0,t_texSurface))){
		osassert(false);
	}
	
	SAFE_RELEASE(t_texSurface);

	m_bFirstRender = TRUE;

	// clear the counter
	m_dwCounter = 0;

	//create the fog rectangle to blend current scene
	CreateFogRect(_lpd3dDev);



	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: CreateRect
//Desc: create every rectangle in array and initialize them
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::RestoreRenderTarget(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	// restore the surface
	_lpd3dDev->SetRenderTarget(0,m_lpOldSurface);
	SAFE_RELEASE(m_lpOldSurface);

}
//--------------------------------------------------------------------------------------------------------------------
//Name: CreateRect
//Desc: create every rectangle in array and initialize them
//--------------------------------------------------------------------------------------------------------------------
BOOL CGridRect::CreateRect(LPDIRECT3DDEVICE9 _lpd3dDev,const int _index)
{
	guard;

	//precondition test
	osassert(m_iWidthNum && m_iWidth);

	//create a rectangle
	if(FAILED(_lpd3dDev->CreateVertexBuffer(4 * sizeof(MyRectVertex),	//4 vertexes for rectangle
											D3DUSAGE_DYNAMIC,
											e_myRectVertexUse,
											D3DPOOL_SYSTEMMEM,
											m_lppVertBuff + _index ,NULL))){
		return FALSE;
	}
	osassert(m_lppVertBuff[_index]);

	//
	MyRectVertex* t_pBuffer = NULL;
	if(FAILED(m_lppVertBuff[_index]->Lock(0,4 * sizeof(MyRectVertex),(void**)&t_pBuffer,0))){
		return FALSE;
	}
	
	//left-top corner position
	const float t_pos_x			= (float)((_index % m_iWidthNum) * m_iGridWidth) ;
	const float t_pos_y			= (float)m_iHeight - (float)((_index / m_iWidthNum) * m_iGridHeight) ;
	const float t_pos_u0		= t_pos_x / (float)(m_iWidth);
	const float t_pos_v0		= (float)((_index / m_iWidthNum) * m_iGridHeight) / (float)(m_iHeight);

	//delta of rectangle offset
	const float t_pos_delta_u	= (float)(m_iGridWidth) / (float)(m_iWidth);
	const float t_pos_delta_v	= (float)(m_iGridHeight) / (float)(m_iHeight);

	//top-left
	t_pBuffer[0].color		= e_rectColor;
	t_pBuffer[0].position.x = t_pos_x;
	t_pBuffer[0].position.y = t_pos_y;
	t_pBuffer[0].position.z = 0.5f;
	t_pBuffer[0].tu0		= t_pos_u0;
	t_pBuffer[0].tv0		= t_pos_v0;

	//top-right
	t_pBuffer[1].color		= e_rectColor;
	t_pBuffer[1].position.x = t_pos_x + m_iGridWidth;
	t_pBuffer[1].position.y = t_pos_y;
	t_pBuffer[1].position.z = 0.5f;
	t_pBuffer[1].tu0		= t_pos_u0 + t_pos_delta_u;
	t_pBuffer[1].tv0		= t_pos_v0;

	//bottom-left
	t_pBuffer[2].color		= e_rectColor;
	t_pBuffer[2].position.x = t_pos_x ;
	t_pBuffer[2].position.y = t_pos_y - m_iGridHeight;
	t_pBuffer[2].position.z = 0.5f;
	t_pBuffer[2].tu0		= t_pos_u0;
	t_pBuffer[2].tv0		= t_pos_v0 + t_pos_delta_v;

	//bottom-right
	t_pBuffer[3].color		= e_rectColor;
	t_pBuffer[3].position.x = t_pos_x + m_iGridWidth;
	t_pBuffer[3].position.y = t_pos_y - m_iGridHeight;
	t_pBuffer[3].position.z = 0.5f;
	t_pBuffer[3].tu0		= t_pos_u0 + t_pos_delta_u;
	t_pBuffer[3].tv0		= t_pos_v0 + t_pos_delta_v;

	m_lppVertBuff[_index]->Unlock();

	return TRUE;

	unguard;

}

//--------------------------------------------------------------------------------------------------------------------
//Name: RenderFrame
//Desc: render every frame
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::RenderFrame(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	if(m_bFirstRender){
		m_bFirstRender = FALSE;
	}
	//render the space hole first
	// if return FALSE , present render is over
	if(!m_spaceHole.RenderFrame(_lpd3dDev)){
		m_bThroughHole		= TRUE;
		m_bReadyToRender	= FALSE;
		return;
	}
	
	if(m_bGridOver){ //has been rendered over(disappeared)
		return;
	}

	//! add the counter
	m_dwCounter++;
	
	_lpd3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	//set the ortho camera to render
	const float	t_fWidth	= (float)m_iWidth;
	const float	t_fHeight	= (float)m_iHeight;
	
	SetOrthoCamera(_lpd3dDev,
					D3DXVECTOR3(t_fWidth / 2.0f,t_fHeight / 2.0f,-1.0f),
					D3DXVECTOR3(t_fWidth / 2.0f,t_fHeight / 2.0f,1.0f),
					t_fWidth,
					t_fHeight,
					-50.0f,
					5000.0f);
	
	// process the vertex buffer rectangles
	ProcessRect();
	
	
	_lpd3dDev->SetTexture(0,m_lpBgTexture);
	_lpd3dDev->SetSamplerState(0, D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	_lpd3dDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	_lpd3dDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	_lpd3dDev->SetTexture(1,m_lpBgTexture2);
	_lpd3dDev->SetSamplerState(1, D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	_lpd3dDev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	_lpd3dDev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);


	_lpd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );//( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    _lpd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    _lpd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	_lpd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	_lpd3dDev->SetTextureStageState( 0,D3DTSS_TEXCOORDINDEX,0);

	_lpd3dDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_ADD );//( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    _lpd3dDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	_lpd3dDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
    _lpd3dDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	_lpd3dDev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	_lpd3dDev->SetTextureStageState( 1,D3DTSS_TEXCOORDINDEX,0);

	
	_lpd3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	_lpd3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	_lpd3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	
	// render the rectangles
	osassert(m_iGridNum > 0);

	for(int i = 0;i< m_iGridNum; i++){
		_lpd3dDev->SetStreamSource(0,m_lppVertBuff[i],0,sizeof(MyRectVertex));

		_lpd3dDev->SetFVF(e_myRectVertexUse);

		_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);

	}

	_lpd3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

	_lpd3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);

	unguard;
}


//--------------------------------------------------------------------------------------------------------------------
//Name: DestroyGridRes
//Desc: destroy the grid resource
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::DestroyGridRes(void)
{
	guard;

	//osassert(m_iGridNum >0);

	for(int i =0;i<m_iGridNum;i++){
		SAFE_RELEASE(m_lppVertBuff[i]);
	}

	SAFE_RELEASE(m_lpBgTexture2);
	SAFE_RELEASE(m_lpBgTexture);

	SAFE_DELETE(m_lppVertBuff);

	SAFE_RELEASE(m_lpFogVertexBuf);

	SAFE_RELEASE(m_lpOldSurface);


	m_iWidthNum = m_iHeightNum = m_iWidth 
		= m_iHeight = m_iGridWidth = m_iGridHeight = m_iGridNum = 0;

	m_bGridOver = m_bFirstRender = m_bReadyToRender = m_bThroughHole = FALSE;

	m_dwCurrFogLevel	= 255;

	m_spaceHole.DestroySpaceHole();

	unguard;
}


//--------------------------------------------------------------------------------------------------------------------
//Name: ProcessRect
//Desc: process rectangle's vertex
//--------------------------------------------------------------------------------------------------------------------
BOOL CGridRect::ProcessRect(void)
{
	guard;

	osassert(m_iGridNum > 0);

	//render over ?
	//BOOL	t_bIsRenderOver = FALSE;

	//render the grids
	for(int i = 0; i < m_iGridNum;i++){
		MyRectVertex* t_pBuffer = NULL;
		if(FAILED(m_lppVertBuff[i]->Lock(0,4 * sizeof(MyRectVertex),(void**)&t_pBuffer,0))){
			osassert(false);
		}

		ProcessARect(i,t_pBuffer);

		m_lppVertBuff[i]->Unlock();
	}

	return TRUE;
	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: ProcessARect
//Desc: process a rectangle's vertex
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::ProcessARect(const int _index,MyRectVertex* _grid)
{
	const int t_iStartX		= m_iStartX;
	const int t_iStartY		= m_iStartY;
	const float t_const		= 0.001f;
	const float	t_linear	= 0.00000005f;
	const float t_sqrt		= 0.0f;

	for(int index = 0 ;index < 4; index++){
		
		const float t_dis		= ((_grid[index].position.x-t_iStartX) * (_grid[index].position.x-t_iStartX) + 
									(_grid[index].position.y-t_iStartY) * (_grid[index].position.y-t_iStartY));
		
		const float t_delta		= (t_const + t_dis * t_linear + t_sqrt * (t_dis * t_dis));

		float t_cos;
		float t_sin;
		// because the fsincos instrucment to optimize the sin/cos function
		// write it in assembly
		__asm{
			fld		t_delta
			fsincos
			fstp	t_cos
			fstp	t_sin

		}
		float t_fTmp = 0.0f;

		float t_dir_x		= t_iStartX - _grid[index].position.x ;
		float t_dir_y		= t_iStartY - _grid[index].position.y ;
		const float t_len	= sqrt(t_dir_x* t_dir_x + t_dir_y * t_dir_y);
 		t_dir_x /= t_len;
		t_dir_y /= t_len;
		
		DWORD t_alpha = 0;
		if(t_len > 10){
			t_alpha = ((_grid[index].color & 0xff000000) >>24) - (DWORD)((255.0f / 40000.0f) * t_len);
			if(t_alpha>255){
				t_alpha = 0;
			}

			// the top left corner vertex is go to the center
			// screen, these grids is rendered over
			if(_index == 0 && t_len <100.0f){
				m_bGridOver = TRUE;
			}
		}


		//go to the local coordinate?
		_grid[index].position.x -= t_iStartX;
		_grid[index].position.y -= t_iStartY;
		
		//rotate
		t_fTmp = _grid[index].position.x; 
		_grid[index].position.x = t_cos * _grid[index].position.x - t_sin * _grid[index].position.y;
		_grid[index].position.y = t_cos * _grid[index].position.y + t_sin * t_fTmp;

		//go to the world coordinate
		_grid[index].position.x += t_iStartX + (m_dwCounter >> 3) * pow(t_dir_x,3) ;
		_grid[index].position.y += t_iStartY + (m_dwCounter >> 3) * pow(t_dir_y,3) ;

		//set the alpha
		_grid[index].color = (_grid[index].color & 0x00ffffff) | (t_alpha << 24);
		
	}

}


//--------------------------------------------------------------------------------------------------------------------
//Name: CreateFogRect
//Desc: create fog rectangle 
//--------------------------------------------------------------------------------------------------------------------
BOOL CGridRect::CreateFogRect(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	osassert(m_iWidth && m_iHeight);

	const int t_iLen = 4 * sizeof(MyRectVertex);
	
	if(FAILED(_lpd3dDev->CreateVertexBuffer(4 * sizeof(MyRectVertex),
											D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY ,
											e_myRectVertexUse,
											D3DPOOL_DEFAULT,
											&m_lpFogVertexBuf,NULL))){
		return FALSE;
	}
	osassert(m_lpFogVertexBuf);

	MyRectVertex*	t_pBuffer = NULL;
	if(FAILED(m_lpFogVertexBuf->Lock(0,t_iLen,(void**)&t_pBuffer,0))){
		return FALSE;
	}
	//left-top corner position
	const float t_pos_x			= 0.0f;
	const float t_pos_y			= 0.0f;
	const float t_pos_u0		= 0.0f;
	const float t_pos_v0		= 0.0f;

	//delta of rectangle offset
	const float t_pos_delta_u	= 1.0f;
	const float t_pos_delta_v	= 1.0f;

	//top-left
	t_pBuffer[0].color		= e_rectColor;
	t_pBuffer[0].position.x = t_pos_x ;
	t_pBuffer[0].position.y = t_pos_y;
	t_pBuffer[0].position.z = 0.5f;
	//t_pBuffer[0].rhw		= 1.0f;
	t_pBuffer[0].tu0		= t_pos_u0;
	t_pBuffer[0].tv0		= t_pos_v0;
	
	//top-right
	t_pBuffer[1].color		= e_rectColor;
	t_pBuffer[1].position.x = t_pos_x; 
	t_pBuffer[1].position.y = t_pos_y + (float)m_iHeight;
	t_pBuffer[1].position.z = 0.5f;
	//t_pBuffer[1].rhw		= 1.0f;
	t_pBuffer[1].tu0		= t_pos_u0 + t_pos_delta_u;
	t_pBuffer[1].tv0		= t_pos_v0;
	

	//bottom-left
	t_pBuffer[2].color		= e_rectColor;
	t_pBuffer[2].position.x = t_pos_x + (float)m_iWidth;
	t_pBuffer[2].position.y = t_pos_y ;
	t_pBuffer[2].position.z = 0.5f;
	//t_pBuffer[2].rhw		= 1.0f;
	t_pBuffer[2].tu0		= t_pos_u0;
	t_pBuffer[2].tv0		= t_pos_v0 + t_pos_delta_v;
	

	//bottom-right
	t_pBuffer[3].color		= e_rectColor;
	t_pBuffer[3].position.x = t_pos_x + (float)m_iWidth;
	t_pBuffer[3].position.y = t_pos_y + (float)m_iHeight;
	t_pBuffer[3].position.z = 0.5f;
	//t_pBuffer[3].rhw		= 1.0f;
	t_pBuffer[3].tu0		= t_pos_u0 + t_pos_delta_u;
	t_pBuffer[3].tv0		= t_pos_v0 + t_pos_delta_v;
	
	if(FAILED(m_lpFogVertexBuf->Unlock())){
		osassert(false);
	}
	return TRUE;

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: RenderThroughFog
//Desc: render the fog rectangle 
//--------------------------------------------------------------------------------------------------------------------
void CGridRect::RenderThroughFog(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	// if has not through the space hole
	if(!m_bThroughHole){
		return;
	}

	
	// rendering the fog is over
	if(m_dwCurrFogLevel == 0 || m_dwCurrFogLevel > 255){
		DestroyGridRes();
		return;
	}

	// change the rectangle'a alpha 
	osassert(m_lpFogVertexBuf);

	const int t_iLen = 4 * sizeof(MyRectVertex);
	MyRectVertex*	t_pBuffer = NULL;

	if(FAILED(m_lpFogVertexBuf->Lock(0,t_iLen,(void**)&t_pBuffer,0))){
		osassert(false);
	}
	osassert(t_pBuffer);
	
	for(int i = 0;i < 4;i++){
		t_pBuffer[i].color = (t_pBuffer[i].color & 0x00ffffff) | (m_dwCurrFogLevel << 24);
	}

	//low the fog level
	m_dwCurrFogLevel -= FOG_DISAPPEAR_VELOCITY;

	m_lpFogVertexBuf->Unlock();

	//set the ortho camera to render

	const float	t_fWidth	= (float)m_iWidth;
	const float	t_fHeight	= (float)m_iHeight;
	
	SetOrthoCamera(_lpd3dDev,
					D3DXVECTOR3(t_fWidth / 2.0f,t_fHeight / 2.0f,-1.0f),
					D3DXVECTOR3(t_fWidth / 2.0f,t_fHeight / 2.0f,1.0f),
					t_fWidth,
					t_fHeight,
					-50.0f,
					5000.0f);

	//set the d3d's state
	// and the world matrix
	_lpd3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	_lpd3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	_lpd3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	_lpd3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	D3DXMATRIX t_mat;
	D3DXMatrixIdentity(&t_mat);
	_lpd3dDev->SetTransform(D3DTS_WORLD,&t_mat);

	//disable the texture
	_lpd3dDev->SetTexture(0,NULL);

	if(FAILED(_lpd3dDev->SetStreamSource(0,m_lpFogVertexBuf,0,sizeof(MyRectVertex)))){
		osassert(false);
	}

	if(FAILED(_lpd3dDev->SetFVF(e_myRectVertexUse))){
		osassert(false);
	}

	if(FAILED(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2))){
		osassert(false);
	}

	_lpd3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);


	unguard;
}
