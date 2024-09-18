//--------------------------------------------------------------------------------------------------------
/** \file
*  Filename: displayScrTexture.cpp
*  Desc:     显示屏幕多边形相关的函数。
*  His:      River created @ 2004-4-22
*
* “有做得到的事，也有做不到的事。”
* 
*/
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/deviceInit.h"
# include "../../exLib/SceneEx.h"
# include "../include/osStateBlock.h"


//! 鼠标光标动画的速度
OSENGINE_API float                    g_fMouseCursorFrameAniTime = 0.4f;


//! 是否使用动画光标.
OSENGINE_API BOOL                     g_bUseAniCursor = TRUE;


//! 屏幕多边形渲染用到的fvf.
# define SCRPIC_DISFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1   )
//! 定义一次可最多渲染的屏幕多边形顶点数目。
# define MAX_SCRVERTEX     MIN_TMPTEXTURE*6  

osc_scrPicRenderData::osc_scrPicRenderData()
{
	m_ptrScrVer = NULL;
	m_ptrVerBuf = NULL;
	m_ptrIdxBuf = NULL;
	m_pd3dDevice = NULL;
}

osc_scrPicRenderData::~osc_scrPicRenderData()
{
	SAFE_DELETE_ARRAY( m_ptrScrVer );
}

void osc_scrPicRenderData::init_scrPicRenderData( LPDIRECT3DDEVICE9 _dev )
{
	guard;

	HRESULT    t_hr;
	osassert( _dev );

	m_pd3dDevice = _dev;
	m_vecHaveRender.resize( MIN_TMPTEXTURE ); 
	m_vecScrPicPtr.resize( MIN_TMPTEXTURE ); 
	m_ptrScrVer = new os_screenVertex[MAX_SCRVERTEX];

	//! 创建动态的顶点缓冲区.
	t_hr = m_pd3dDevice->CreateVertexBuffer(
		sizeof( os_screenVertex )*MAX_SCRVERTEX,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		SCRPIC_DISFVF,D3DPOOL_DEFAULT,&m_ptrVerBuf,NULL );
	osassert( !FAILED( t_hr ) );


	//! 创建静态的索引缓冲区并填充数据。
	WORD*       t_istart;
	t_hr = m_pd3dDevice->CreateIndexBuffer( 
		sizeof( WORD )*MAX_SCRVERTEX/4*6,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED,
		&m_ptrIdxBuf,NULL );
	osassert( !FAILED( t_hr ) );

	t_hr = m_ptrIdxBuf->Lock( 0,0,(void**)&t_istart,0 );
	osassert( !FAILED( t_hr ) );

	for( int t_i=0;t_i<MAX_SCRVERTEX/4;t_i ++ )
	{
		t_istart[t_i*6+0] = t_i*4;
		t_istart[t_i*6+1] = t_i*4+1;
		t_istart[t_i*6+2] = t_i*4+2;

		t_istart[t_i*6+3] = t_i*4;
		t_istart[t_i*6+4] = t_i*4+2;
		t_istart[t_i*6+5] = t_i*4+3;
	}

	m_ptrIdxBuf->Unlock();

	m_iRenderVerNum = 0;

	unguard;
}


//! 设备管理相关.
void osc_scrPicRenderData::scrpic_onLostDevice( void )
{
	guard;

	SAFE_RELEASE( m_ptrVerBuf );

	unguard;
}
void osc_scrPicRenderData::scrpic_onResetDevice( void )
{
	guard;

	HRESULT   t_hr;

	//! 创建动态的顶点缓冲区.
	t_hr = m_pd3dDevice->CreateVertexBuffer(
		sizeof( os_screenVertex )*MAX_SCRVERTEX,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		SCRPIC_DISFVF,D3DPOOL_DEFAULT,&m_ptrVerBuf,NULL );
	osassert( !FAILED( t_hr ) );

	return;

	unguard;
}


void osc_scrPicRenderData::release_scrPicR( void )
{
	guard;

	m_vecHaveRender.clear();
	m_vecScrPicPtr.clear();

	SAFE_RELEASE( m_ptrVerBuf );
	SAFE_RELEASE( m_ptrIdxBuf );

	unguard;
}

void osc_scrPicRenderData::push_scrRenderVer( os_screenVertex* _ver )
{
	guard;

	osassert( m_iRenderVerNum < MAX_SCRVERTEX );

	memcpy( &m_ptrScrVer[m_iRenderVerNum],_ver,sizeof( os_screenVertex )*4 );
	m_iRenderVerNum += 4;

	unguard;
}

void osc_scrPicRenderData::draw_scrRenderVer( void )
{
	guard;

	BYTE*           t_bufStart;
	HRESULT         t_hr;	

	t_hr = m_ptrVerBuf->Lock(
		0,sizeof( os_screenVertex )*m_iRenderVerNum,
		(void**)&t_bufStart,D3DLOCK_DISCARD );
	memcpy( t_bufStart,m_ptrScrVer,sizeof( os_screenVertex )*m_iRenderVerNum );

	m_ptrVerBuf->Unlock();

	m_pd3dDevice->SetStreamSource( 0,m_ptrVerBuf,0,sizeof( os_screenVertex ) );
	m_pd3dDevice->SetIndices( m_ptrIdxBuf );

	t_hr = m_pd3dDevice->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,0,0,m_iRenderVerNum,0,m_iRenderVerNum/2 );
	osassert( !FAILED( t_hr ) );

	m_iRenderVerNum = 0;

	unguard;
}


//! 渲染一组屏幕多边形列表，列表中的数据应该有同样的纹理id.
bool osc_d3dManager::disp_scrPicArr( 
									os_screenPicDisp** _tdisp,int _disnum,int _texIdx )
{
	guard;

	osassert( _tdisp );
	osassert( _disnum>=0 );
	osassert( _texIdx >= 0 );

	HRESULT    hr;


	// 先设置纹理
	switch( _texIdx )
	{
	case 0:
		osassert( (_tdisp[0]->tex_id>=0)&&(_tdisp[0]->tex_id<=MIN_TMPTEXTURE) );
		osassert( m_vecTexture[_tdisp[0]->tex_id].inuse );
		m_pd3dDevice->SetTexture( 0,this->m_vecTexture[_tdisp[0]->tex_id].tex );
		break;
	case 1:
		osassert( (_tdisp[0]->tex_id>=0)&&(_tdisp[0]->tex_id<=MIN_TMPTEXTURE) );
		osassertex( m_vecSceneScrTexture[_tdisp[0]->tex_id].inuse,
			va( "Id is:<%d>..\n",_tdisp[0]->tex_id )  );

		hr = m_pd3dDevice->SetTexture( 0,m_vecSceneScrTexture[_tdisp[0]->tex_id].tex );


		osassert( !FAILED( hr ) );
		break;

		//syq_mask
	case 3:
		{
			m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
			m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );//TEST
			m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );//TEST

			m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,TRUE );
			m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
			m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ZERO );

			m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,TRUE );
			m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,0x1 );
			m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL );

			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

			m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		
			m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX,0 );


			m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );


			int id = _tdisp[0]->tex_id & 0x00003FFF;

			m_pd3dDevice->SetTexture( 0, get_renderTexture(id) );

			id = _tdisp[0]->tex_id & 0x3FFFC000;
			short int maskid = (short int)( id >> 14 );
			osassert( m_vecSceneScrTexture[maskid].tex );
			if( maskid >= 0 )
				m_pd3dDevice->SetTexture( 1, m_vecSceneScrTexture[maskid].tex );
			else
				m_pd3dDevice->SetTexture( 1, NULL );

		}
		break;


	default:
		osassert( false );
	}

	// River @ 2006-6-26:如果使用Zbuf.
	if( _tdisp[0]->m_bUseZBuf )
	{
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,TRUE );
	}

	// 
	// 然后渲染每一个图片
	// ATTENTION TO OPP: 可以使用一个动态的顶点缓冲区，填充顶点缓冲区后，只需调用一次DIP：
	if( _disnum == 1 )
	{
		// render the simple square.
		hr = m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,
			2,_tdisp[0]->m_scrVer,sizeof( os_screenVertex ) );
		if( FAILED( hr ) )
		{
			osassertex( false,va( "Draw temp screen square failed..<%s>..\n",
				osn_mathFunc::get_errorStr( hr ) )  );
			return false;
		}
	}
	else
	{
		for( int t_i=0;t_i<_disnum;t_i ++ )
			m_sPicRD.push_scrRenderVer( _tdisp[t_i]->m_scrVer );		
		m_sPicRD.draw_scrRenderVer();
	}

	// River @ 2006-6-26:Z缓冲相关.
	if( _tdisp[0]->m_bUseZBuf )
	{
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,FALSE );
	}


	//syq_mask
	m_pd3dDevice->SetTexture( 1, NULL );
	return true;

	unguard;
}


/** \brief
*  渲染和背景混合相加的图片
*/
bool osc_d3dManager::disp_alphaAddScrPic( 
	const os_screenPicDisp* _tdisp,int _idx,int _texIdx )
{
	guard;

	osassert( _tdisp );
	osassert( _tdisp[_idx].blend_mode == 5 );

	os_stateBlockMgr*  t_ptrStateBM = os_stateBlockMgr::Instance();
	if( (m_iScrTexRSAddId>=0)&&
		t_ptrStateBM->validate_stateBlockId( m_iScrTexRSAddId ) )
	{
		t_ptrStateBM->apply_stateBlock( m_iScrTexRSAddId );
	}
	else
	{
		record_scrAlphaAddBlock();
		t_ptrStateBM->apply_stateBlock( m_iScrTexRSAddId );
	}


	m_sPicRD.m_vecScrPicPtr[0] = (os_screenPicDisp*)&_tdisp[_idx];

	disp_scrPicArr( &m_sPicRD.m_vecScrPicPtr[0],1,_texIdx );


	m_sPicRD.m_vecHaveRender[_idx] = 1;

	// 恢复渲染状态
	if( (m_iScrTexRSId>=0)&&
		t_ptrStateBM->validate_stateBlockId( m_iScrTexRSId ) )
	{
		t_ptrStateBM->apply_stateBlock( m_iScrTexRSId );
	}
	else
	{
		record_scrTexRBlock();
		t_ptrStateBM->apply_stateBlock( m_iScrTexRSId );
	}

	return true;

	unguard;
}


/**
*  Display a temp picture in screen,such as load screen.
*
*  River: 2005-7-12:优化屏幕多边形图片的渲染。
*　算法描述：在函数中保存一个bool数组，用于记录
*  每一个要渲染的元素是否渲染。每一次渲染一个屏幕多边形的时候，
*  遍历整个渲染队列，把没有渲染的，又跟当前多边形同一个纹理的
*  的多边形全部渲染，并把渲染后的元素对应的是否渲染bool值修改为已渲染。
*  
*/
bool osc_d3dManager::disp_inscreen( const os_screenPicDisp* _tdisp,
								   int _disnum/* = 1*/,int _texIdx/* = 0*/,bool _LensPic )	
{
	guard;

	osassert( m_iScrTexRSId >= 0 );
	osassert( _disnum <= MIN_TMPTEXTURE );

	memset( &m_sPicRD.m_vecHaveRender[0],0,sizeof( BYTE )*_disnum );


	os_stateBlockMgr*  t_ptrStateBM = os_stateBlockMgr::Instance();
	// ATTENTION TO OPP: 必须每次推进多一些的多边形，才能使
	if (_LensPic)
	{

		if( (m_iLensFlareRSId>=0)&&
			t_ptrStateBM->validate_stateBlockId( m_iLensFlareRSId ) )
		{
			os_stateBlockMgr::Instance()->apply_stateBlock( m_iLensFlareRSId );
		}
		else
		{
			record_LensFlareRBlock();
			os_stateBlockMgr::Instance()->apply_stateBlock( m_iLensFlareRSId );
		}
	}
	else
	{
		//syq test code
		if( _tdisp->blend_mode == 3 )
		{
			if( (m_iScrTexRSId>=0)&&
				t_ptrStateBM->validate_stateBlockId( m_iScrTexRSId ) )
			{
				os_stateBlockMgr::Instance()->apply_stateBlock( m_iScrTexRSId );
			}
			else
			{
				record_scrTexRBlock();
				os_stateBlockMgr::Instance()->apply_stateBlock( m_iScrTexRSId );
			}
		}

		if( _tdisp->blend_mode == 4 )
		{
			os_stateBlockMgr::Instance()->apply_stateBlock( m_iScrTexRSId_rt );
		}
	}


	//
	// 开始渲染场景中的二维屏幕图片。
	int       t_iRenderPicNum = 0;
	int       t_iTexIdx;
	int       t_iMask = _texIdx;

	for( int t_i=0;t_i<_disnum;t_i++ )
	{
		// River mod @ 2006-8-9:对上层的特殊纹理ID进行处理
		t_iMask = (_tdisp[t_i].tex_id & 0xc0000000) >>30;
		if( t_iMask != 3 )
			t_iMask = _texIdx;

		if( (_tdisp[t_i].tex_id & 0x7FFF ) < 0 )
			continue;

		// 当前的屏幕元素已经渲染过
		if( m_sPicRD.m_vecHaveRender[t_i] != 0 )
			continue;

		// 如果当前的渲染模型是和背景混合相加，不渲染
		if( _tdisp[t_i].blend_mode == 5 )
		{
			disp_alphaAddScrPic( _tdisp,t_i,t_iMask );
			continue;
		}

		t_iTexIdx = _tdisp[t_i].tex_id;

		// 
		m_sPicRD.m_vecScrPicPtr[t_iRenderPicNum] = (os_screenPicDisp*)&_tdisp[t_i];
		t_iRenderPicNum++;
		m_sPicRD.m_vecHaveRender[t_i] = 1;

		for( int t_j=t_i+1;t_j<_disnum;t_j ++ )
		{
			if( (_tdisp[t_j].tex_id == t_iTexIdx)&&
				(_tdisp[t_j].blend_mode == _tdisp[t_i].blend_mode) )
			{
				m_sPicRD.m_vecScrPicPtr[t_iRenderPicNum] = (os_screenPicDisp*)&_tdisp[t_j];
				t_iRenderPicNum++;
				m_sPicRD.m_vecHaveRender[t_j] = 1;
			}
		}

		disp_scrPicArr( &m_sPicRD.m_vecScrPicPtr[0],t_iRenderPicNum,t_iMask );

		t_iRenderPicNum = 0;
	}


	// 
	// 
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE );

	return true;

	unguard;
}



//! 记录当前d3dMgr中需要的渲染状态块。
void osc_d3dManager::record_scrTexRBlock( void )
{
	guard;


	HRESULT          hr;


	m_iScrTexRSId = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iScrTexRSId>=0 );

	os_stateBlockMgr::Instance()->start_stateBlock();



	//set render state.
	m_pd3dDevice->SetFVF( SCRPIC_DISFVF );


	hr = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	osassert( !FAILED( hr ) );
 

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,FALSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLOROP,D3DTOP_MODULATE  );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG1,D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG2,D3DTA_TEXTURE );	
	m_pd3dDevice->SetTextureStageState( 1,D3DTSS_COLOROP,D3DTOP_DISABLE  );

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID   );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_POINT );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_POINT );

	//! 
	hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_NONE );
	osassert( !FAILED( hr ) );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );


	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );



	// 可以使用不透明的图做出透明的界面来,或是做出淡入淡出的界面。
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG2,D3DTA_TEXTURE );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iScrTexRSId  );



	return;

	unguard;
}

//! 记录当前d3dMgr中跟背景相加的渲染块
void osc_d3dManager::record_scrAlphaAddBlock( void )
{
	guard;

	HRESULT          hr;


	m_iScrTexRSAddId = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iScrTexRSAddId>=0 );

	os_stateBlockMgr::Instance()->start_stateBlock();



	//set render state.
	m_pd3dDevice->SetFVF( SCRPIC_DISFVF );


	hr = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	osassert( !FAILED( hr ) );
 
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,FALSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLOROP,D3DTOP_MODULATE  );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG1,D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG2,D3DTA_TEXTURE );	
	m_pd3dDevice->SetTextureStageState( 1,D3DTSS_COLOROP,D3DTOP_DISABLE  );

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID   );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_POINT );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_POINT );

	//! 
	hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_NONE );
	osassert( !FAILED( hr ) );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ONE );

	// 可以使用不透明的图做出透明的界面来,或是做出淡入淡出的界面。
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_ALPHAARG2,D3DTA_TEXTURE );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iScrTexRSAddId  );

	return;

	unguard;
}


//! 画renderTarget的状态
void osc_d3dManager::record_scrTexRBlock_rt( void )
{
	guard;

	m_iScrTexRSId_rt = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iScrTexRSId_rt>=0 );

	os_stateBlockMgr::Instance()->start_stateBlock();

	m_pd3dDevice->SetFVF( SCRPIC_DISFVF );


    m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);

    m_pd3dDevice->SetRenderState(D3DRS_ZFUNC,     D3DCMP_LESSEQUAL);
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,FALSE );

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA   );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1  );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE   );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iScrTexRSId_rt  );
	return;

	unguard;	
}

void osc_d3dManager::record_LensFlareRBlock( void )
{
	guard;
	HRESULT          hr;
	m_iLensFlareRSId = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iLensFlareRSId>=0 );

	os_stateBlockMgr::Instance()->start_stateBlock();
	//
	//set render state.
	//
	m_pd3dDevice->SetFVF( SCRPIC_DISFVF );

	hr = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	osassert( !FAILED( hr ) );

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,false );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLOROP,D3DTOP_MODULATE  );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG1,D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG2,D3DTA_TEXTURE );	

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID  /*D3DFILL_WIREFRAME*/ );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_POINT );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_POINT );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );
	m_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	os_stateBlockMgr::Instance()->end_stateBlock( m_iLensFlareRSId  );
	return;

	unguard;
}
//! 记录全屏alpha时,需要的渲染状态块.
void osc_d3dManager::record_fullScrFadeRBlock( void )
{
	guard;


	HRESULT          hr;


	m_iFullScrFadeRSId = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iFullScrFadeRSId>=0 );

	os_stateBlockMgr::Instance()->start_stateBlock();


	//
	//set render state.
	//
	m_pd3dDevice->SetFVF( SCRPIC_DISFVF );
	m_pd3dDevice->SetPixelShader( NULL );

	hr = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	osassert( !FAILED( hr ) );

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,false );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLOROP,D3DTOP_SELECTARG1  );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG1,D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );


	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_POINT );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_POINT );
	hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_NONE );
	osassert( !FAILED( hr ) );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,true );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,false );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetTextureStageState( 1,D3DTSS_COLOROP,D3DTOP_DISABLE  );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iFullScrFadeRSId );

	return;

	unguard;
}





//@{
//  程序中光标相关的接口。
/** \brief
*  使用文件名创建光标。
*
*  ATTENTION TO OPP:优化此处和上层的代码，使不需要渲染光标，我们使用了d3d的硬件光标。
*
*/
int osc_d3dManager::create_cursor( const char* _cursorName )
{
	guard;

	int   t_idx = 0;

	t_idx = m_vecCursor.get_freeNode();
	m_vecCursor[t_idx].create_cursor( m_pd3dDevice,_cursorName );
	return t_idx;

	unguard;
}


//! 设置引擎中显示的光标。
void osc_d3dManager::set_cursor( int _id, int _centerX/*=0*/, int _centerY/*=0*/ )
{
	guard;


	// 上层还没有准备好正确的光标ID
	if( _id < 0 )
		return;

	if( !m_vecCursor.validate_id( _id ) )
		return;

	//
	// 当前光标正确的情况下,不需要重设
	//
	if( m_iActiveCursorId == _id )
		return;

	m_vecCursor[_id].set_cursor( m_pd3dDevice,_centerX,_centerY );

	m_iActiveCursorId = _id;

	unguard;
}

//! 获得当前显示的光标id
int osc_d3dManager::get_cursor()
{
	guard;
	return m_iActiveCursorId;
	unguard;
}

//! 设置光标的位置。
void osc_d3dManager::set_cursorPos(  )
{ 
	POINT    t_pt;

	if( !m_pd3dDevice )
		return;

	::GetCursorPos( &t_pt );
	if( !m_bWindowed )
		::ScreenToClient( this->m_hWnd,&t_pt );
	m_pd3dDevice->SetCursorPosition( t_pt.x, t_pt.y, 0 );

}


//! 定义光标的大小。

/** \brief
*  渲染屏幕上的光标。
*
*  光标渲染必须在屏幕上所有的元素都渲染完成后才能渲染。
*/
void osc_d3dManager::render_cursor( void )
{
	guard;

	static    os_screenPicDisp    t_picDisp;

	// 
	// 只有设置了正确的光标后才能渲染光标。
	if( m_iActiveCursorId < 0 )
		return;

	return;
	unguard;
}

//@} 



//! 记录画全屏幕alpha的状态块渲染
void osc_d3dManager::record_fullScrAlpha( void )
{
	guard;

	HRESULT          hr;


	m_iFullscrAlphaId = os_stateBlockMgr::Instance()->create_stateBlock();
	osassert( m_iFullscrAlphaId>=0 );

	os_stateBlockMgr::Instance()->start_stateBlock();


	//
	//set render state.
	//
	m_pd3dDevice->SetFVF( SCRPIC_DISFVF );

	hr = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	osassert( !FAILED( hr ) );

	// River @ 2010-6-23:
	m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,false );

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,false );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLOROP,D3DTOP_SELECTARG1  );
	m_pd3dDevice->SetTextureStageState( 0,D3DTSS_COLORARG1,D3DTA_DIFFUSE );

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MAGFILTER,D3DTEXF_POINT );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MINFILTER,D3DTEXF_POINT );
	hr = m_pd3dDevice->SetSamplerState( 0,D3DSAMP_MIPFILTER,D3DTEXF_NONE );
	osassert( !FAILED( hr ) );

	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,false );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ZERO );
	m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALPHA );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

	m_pd3dDevice->SetTextureStageState( 1,D3DTSS_COLOROP,D3DTOP_DISABLE  );

	os_stateBlockMgr::Instance()->end_stateBlock( m_iFullscrAlphaId );

	return;

	unguard;
}


/** \brief
*  加入全屏幕画多边的形功能，用于画一个纯白的Alpha底图，在渲染
*  水面的时候，不会误fraction一部分的透明物品
*/
bool osc_d3dManager::draw_fullscrAlpha( float _alpha/* = 1.0f*/ )
{
	guard;

	osColor           t_colorSrc;
	os_screenVertex   t_disVer[4];

	t_colorSrc.a = _alpha;
	t_colorSrc.r = 0.0f;
	t_colorSrc.g = 0.0f;
	t_colorSrc.b = 0.0f;

	t_disVer[0].m_dwDiffuse = t_colorSrc;
	t_disVer[1].m_dwDiffuse = t_colorSrc;
	t_disVer[2].m_dwDiffuse = t_colorSrc;
	t_disVer[3].m_dwDiffuse = t_colorSrc;

	t_disVer[0].m_vecPos = osVec4D( 0,0,0.5f,0.5f );
	t_disVer[1].m_vecPos = osVec4D( float(m_rcClientBounds.right),0.0f,0.5f,0.5f );
	t_disVer[2].m_vecPos = osVec4D( float(m_rcClientBounds.right),
		float(m_rcClientBounds.bottom),0.5f,0.5f );
	t_disVer[3].m_vecPos = osVec4D( 0.0f,
		float(m_rcClientBounds.bottom),0.5f,0.5f );

	os_stateBlockMgr*   t_ptrSBM = os_stateBlockMgr::Instance();
	if( (m_iFullscrAlphaId>=0)&&
		t_ptrSBM->validate_stateBlockId(m_iFullscrAlphaId) )
	{
		t_ptrSBM->apply_stateBlock( m_iFullscrAlphaId );
	}
	else
	{
		record_fullScrAlpha();
		t_ptrSBM->apply_stateBlock( m_iFullscrAlphaId );
	}

	//
	// render the simple square.
	HRESULT    t_hr;
	t_hr = m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,
		2,t_disVer,sizeof( os_screenVertex ) );
	if( FAILED( t_hr ) )
	{
		osassertex( false,
			va( "Draw temp screen square failed,error<%s>...\n",
			osn_mathFunc::get_errorStr( t_hr ) ) );
		return false;
	}

	//! River @ 2010-6-29: 回复到可以全部使用颜色信息
	m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE |
		D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );


	return true;

	unguard;
}


void osc_d3dManager::draw_fullscrFade( void )
{
	guard;

	osassert( this->m_iFullScrFadeRSId>= 0 );

	if( !m_bFullScrFadeState )
		return;

	float   t_fLastEleTime = sg_timer::Instance()->get_lastelatime();

	//  River @ 2009-3-13:
	//! 如果某一帧的时间大于fadeTime的5分之一，则此帧不计fade状态。
	if( t_fLastEleTime < (m_fFadeTime/3.0f) )
		m_fElaFadeTime += t_fLastEleTime;
	else
		// 稍稍往前fade一点
		m_fElaFadeTime += m_fFadeTime/20.0f;

	if( m_fElaFadeTime > m_fFadeTime )
	{
		// 如果最终为全透明显示颜色.
		if( (m_dwEndColor>>24)==0 )
		{
			m_bFullScrFadeState = false;
			return;
		}

		m_fElaFadeTime = m_fFadeTime;
	}

	float    t_fLerp = m_fElaFadeTime / m_fFadeTime;

	osColor  t_colorSrc,t_colorDst;
	t_colorSrc = osColor( this->m_dwStartColor );
	t_colorDst = osColor( this->m_dwEndColor );
	osColorLerp( &t_colorDst,&t_colorSrc,&t_colorDst,t_fLerp );

	os_screenVertex   t_disVer[4];

	t_disVer[0].m_dwDiffuse = t_colorDst;
	t_disVer[1].m_dwDiffuse = t_disVer[0].m_dwDiffuse;
	t_disVer[2].m_dwDiffuse = t_disVer[0].m_dwDiffuse;
	t_disVer[3].m_dwDiffuse = t_disVer[0].m_dwDiffuse;

	t_disVer[0].m_vecPos = osVec4D( 0,0,0.5f,0.5f );
	t_disVer[1].m_vecPos = osVec4D( float(m_rcClientBounds.right),0.0f,0.5f,0.5f );
	t_disVer[2].m_vecPos = osVec4D( float(m_rcClientBounds.right),
		float(m_rcClientBounds.bottom),0.5f,0.5f );
	t_disVer[3].m_vecPos = osVec4D( 0.0f,
		float(m_rcClientBounds.bottom),0.5f,0.5f );

	os_stateBlockMgr*   t_ptrSBM = os_stateBlockMgr::Instance();
	if( (m_iFullScrFadeRSId>=0)&&
		t_ptrSBM->validate_stateBlockId(m_iFullScrFadeRSId) )
	{
		t_ptrSBM->apply_stateBlock( m_iFullScrFadeRSId );
	}
	else
	{
		record_fullScrFadeRBlock();
		t_ptrSBM->apply_stateBlock( m_iFullScrFadeRSId );
	}

	//
	// render the simple square.
	HRESULT    t_hr;
	t_hr = m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN,
		2,t_disVer,sizeof( os_screenVertex ) );
	if( FAILED( t_hr ) )
	{
		osDebugOut( "Draw temp screen square failed" );
		osDebugOut( osn_mathFunc::get_errorStr( t_hr ) );
		osassert( false );
		return ;
	}

	return;
	unguard;
}



osc_mouseCursor::osc_mouseCursor(void)
{
	reset();
}
//! 重设当前的cursor数据结构
void osc_mouseCursor::reset( void )
{
	m_vecCursorTex.clear();
	m_fFrameTime = 1.0f;
	m_iLastFrameCursorId = 0;
	m_cursorOffset_y = m_cursorOffset_x = 0;
}

//! 根据光标的名字，查找目录下对应的光标文件
bool osc_mouseCursor::process_cursorNum( const char* _name )
{
	guard;

	s_string  t_szSearchName = _name;

	os_tmpdisplayTex     t_vec3Tex;

	// 第一个必须存在
	strcpy( t_vec3Tex.m_szTexName,_name );
	m_vecCursorTex.push_back( t_vec3Tex );

	// 根据文件名,查找接下来存在目录
	int  t_iLidx = t_szSearchName.rfind( '.' );
	s_string   t_szTName;
	for( int t_i=1;t_i<=MAX_ANICURSORFRAME;t_i ++ )
	{
		t_szTName = t_szSearchName.substr( 0,t_iLidx );
		t_szTName += va("%d",t_i);
		t_szTName += ".hon";

		// 如果存在此纹理,继续查找,否则退出查找
		if( ::file_exist( (char*)t_szTName.c_str() ) )
		{
			strcpy( t_vec3Tex.m_szTexName,t_szTName.c_str() );
			m_vecCursorTex.push_back( t_vec3Tex );
		}
		else
			break;
	}

	return true;

	unguard;
}



//! 根据传入的文件名，来创建光标序列文件
BOOL osc_mouseCursor::create_cursor( LPDIRECT3DDEVICE9 _dev,const char* _name )
{
	guard;
	
	osassert( _dev );

	m_fFrameTime = g_fMouseCursorFrameAniTime;

	//
	// 得到纹理的信息,看是否符合我们的纹理格式.
	D3DXIMAGE_INFO   t_info;
	BOOL            t_bAlpha;
	HRESULT         t_hr;
	BYTE*           t_ptrStart;
	int             t_offset,t_fsize;
	BOOL			t_avoidMipMap = FALSE;

	s_string    t_strName = _name;
	osn_mathFunc::change_extName( t_strName,"hon" );

	// 得到所有的纹理名字
	process_cursorNum( t_strName.c_str() );

	for( int t_i=0;t_i<(int)m_vecCursorTex.size();t_i ++ )
	{
		int   t_iGBufIdx = -1;
		t_ptrStart = START_USEGBUF( t_iGBufIdx );

		t_fsize = load_tgaFile( m_vecCursorTex[t_i].m_szTexName,
			           t_bAlpha,t_ptrStart,TMP_BUFSIZE,t_offset ,t_avoidMipMap);
		osassert( t_fsize > 0 );
		t_ptrStart += t_offset;

		t_hr = D3DXGetImageInfoFromFileInMemory( 
			t_ptrStart,t_fsize,&t_info );
		if( FAILED( t_hr ) )
		{
			osassertex( false,(char*)m_vecCursorTex[t_i].m_szTexName );
			return -1;
		}
		if( (t_info.Width>32)||(t_info.Height>32 ) )
		{
			osassertex( false,va( "光标纹理不能超过32的限制大小<%s>,<%d,%d>..\n", 
				m_vecCursorTex[t_i].m_szTexName,t_info.Width,t_info.Height) );
		}

		//
		// 使用得到的纹理存储单元调入临时要显示的纹理.
		// 这些纹理使用的是d3d's Managed Texture,所以不需要担心纹理丢失的问题.
		//
		// Create texture from file's memory.
		t_hr = D3DXCreateTextureFromFileInMemoryEx( _dev,
			t_ptrStart,t_fsize,
			D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,
			0,D3DFMT_A8R8G8B8,
			D3DPOOL_MANAGED, 
			D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
			D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
			0, NULL, NULL, &m_vecCursorTex[t_i].tex  );

		END_USEGBUF( t_iGBufIdx );

		if( FAILED( t_hr ) )
		{
			osassertex( false,va( "The error is<%s>...\n",osn_mathFunc::get_errorStr( t_hr ) ) );
			return -1;
		}	

		m_vecCursorTex[t_i].add_ref();
		m_vecCursorTex[t_i].inuse = true;
	}

	return true;

	unguard;
}
//! 上层对光标的设置调用
void osc_mouseCursor::set_cursor( LPDIRECT3DDEVICE9 _dev,int _centerX/*=0*/, int _centerY/*=0*/ )
{
	guard;

	osassert( _dev );

	LPDIRECT3DSURFACE9   t_surface;
	::SetCursor( NULL );
	m_vecCursorTex[m_iLastFrameCursorId].tex->GetSurfaceLevel( 0,&t_surface );
	_dev->SetCursorProperties( _centerX,_centerY,t_surface );
	_dev->ShowCursor( TRUE );
	t_surface->Release();

	osassert(_centerX >= 0 && _centerY >= 0);

	m_cursorOffset_x = _centerX;
	m_cursorOffset_y = _centerY;

	m_iLastFrameCursorId = 0;
	m_fEvaSwitchTime = 0.0f;

	unguard;
}

//! 每一帧对设备相关光标的设置
void osc_mouseCursor::frame_setCursor( float _eletime,LPDIRECT3DDEVICE9 _dev )
{
	guard;

	m_fEvaSwitchTime += _eletime;

	// 在特定的条件下,动画光标会非常的占用时间,必须加入选项,可以全局关闭动画光标
	if( g_bUseAniCursor )
	{
		if( m_fEvaSwitchTime > g_fMouseCursorFrameAniTime )
		{

			m_iLastFrameCursorId += int(m_fEvaSwitchTime/g_fMouseCursorFrameAniTime);
			m_fEvaSwitchTime = float_mod( m_fEvaSwitchTime,g_fMouseCursorFrameAniTime );	
			if( m_iLastFrameCursorId >= (int)m_vecCursorTex.size() )
				m_iLastFrameCursorId = 0;

			// 重新设置光标
			LPDIRECT3DSURFACE9   t_surface;

			m_vecCursorTex[m_iLastFrameCursorId].tex->GetSurfaceLevel( 0,&t_surface );

			//PSTART( cursor ); 
			// River @ 2007-3-20:这个函数非常的慢,差不多要花费0.1秒
			_dev->SetCursorProperties( m_cursorOffset_x,m_cursorOffset_y,t_surface );
			//PEND( cursor );

			_dev->ShowCursor( TRUE );
			t_surface->Release();

		}
	}

	return;

	unguard;
}

