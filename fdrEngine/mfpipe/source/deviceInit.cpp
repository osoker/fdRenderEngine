//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: deviceInit.cpp
 *  Desc:     处理3d Device的初始化,得到一个可以被应用程序使用的3d device.
 *  His:      River created @ 4/14 2003
 *
 * “有做得到的事，也有做不到的事。”
 * 
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/deviceInit.h"
# include "../include/osStateBlock.h"
# include "../include/osStateBlock.h"
# include "../include/middlePipe.h"
# include "../include/osCharacter.h"
# include "../include/osTexture.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../terrain/geomipterrain/osMapTerrain.h"
# include "../include/PixelFont.h"
# include "../../fdrEngine/galaEff.h"
#include "../../effect/include/VolumeFog.h"
#include "../../CardProfile/gfxD3D9CardProfiler.h"
#include "../../fdrEngine/light.h"

CGfxFont   g_song12x12;
CGfxFont   g_song14x14;


// 最低要求的屏幕刷新率.
# define MIN_FRASHRATE    60


//! present时的结构  // D3DPRESENT_DONOTWAIT //
# define PRESENT_INTERVAL     D3DPRESENT_INTERVAL_ONE 

//
// 性能分析,使用nvperfhud,以后所有的引擎授权都使用这个工具来分析程序的性能
#define NVPERFHUD_ANALYSIS  0

//! 
//! 全局用到的渲染方式:
OSENGINE_API ose_VertexProcessingType   g_vertexProType = SOFTWARE_VP;

//! 是否使用全屏抗锯齿,使用fsaa时，后端缓冲区必须使用discard的方式
OSENGINE_API BOOL                   g_bUseFSAA = false;

//! 记录主设备的shader版本号.
OSENGINE_API DWORD                  g_iMajorVSVersion = 1;
OSENGINE_API DWORD                  g_iMinorVSVersion = 1; 

OSENGINE_API DWORD                  g_iMajorPSVersion = 1;
OSENGINE_API DWORD                  g_iMinorPSVersion = 4; 

# define NOT_FOUND_DEVICE      "查找设备失败"
# define NOT_FOUND_DIVICE_TIPS "请重新启动程序，如果再次出现当前对话框，说明您的显卡版本过老，不支持当前的游戏，请升级显卡后再试或联系客服人员"


CALLBACK_DEVICE callback_lostDevice =  0;
CALLBACK_DEVICE callback_resetDevice =  0;

void register_lostDeviceFunc( CALLBACK_DEVICE lpFunc )
{
	callback_lostDevice = lpFunc;
}

void register_resetDeviceFunc( CALLBACK_DEVICE lpFunc )
{
	callback_resetDevice = lpFunc;
}

void os_tmpdisplayTex::release_texture( int _curId )
{
	if( m_iRef > 0 )
	{
		m_iRef --;
		if( m_iRef == 0 )
		{
			SAFE_RELEASE( tex );
			inuse = false;
			m_szTexName[0] = NULL;

			// 
			// TEST CODE:用上确保上层没有删除屏幕纹理
			osDebugOut( "Cur id is:<%d>..\n",_curId );

		}
	}
	else
		return;
} 

os_screenVertex::os_screenVertex()
{
	m_vecPos.z = 0.5f;
	m_vecPos.w = 0.5f;
	m_dwDiffuse = 0xffffffff;
}






osc_d3dManager::osc_d3dManager()
{
	re_initvar();
}

osc_d3dManager::~osc_d3dManager()
{
	release_device();
}

/** \brief
*  初始化类中的变量.
*/
void osc_d3dManager::re_initvar( void )
{

    m_bWindowed = true;
    m_pD3D = NULL;              
    m_pd3dDevice = NULL;        
	m_dwCreateFlags = 0;     


	m_hWnd = NULL;
	m_iScrWidth = 800;
	m_iScrHeight = 600;  
	m_iScrBpp = 32;   

	m_iStencilBitsNum = 0;//8;
	m_iMaxClipPlane = 0;

	cfm_deviceptr = NULL;

	m_rcWindowBounds.left = 0;
	m_rcWindowBounds.right = 640;
	m_rcWindowBounds.top = 0;
	m_rcWindowBounds.bottom = 480;
    
	m_bShowCursorWhenFullscreen = true; 

	m_vecCursor.resize( MAX_CURSORNUM );
	m_iActiveCursorId = -1;

	m_iScrTexRSId = -1;
	m_iFullScrFadeRSId = -1;
	m_iLensFlareRSId = -1;
	m_iScrTexRSAddId = -1;

	m_iFullscrAlphaId = -1;

	m_bFullScrFadeState = false;

	m_bHideCursor = false;

	m_bInRender = false;

	m_bInDrawScene = FALSE;

	m_szBkInfo[0] = NULL;
	m_szWaitInfo[0] = NULL;
	m_dwBkInfoColor = 0xffffffff;
	m_dwWaitInfoColor = 0xffffffff;

	m_ptrSmMgr = NULL;
	m_ptrMiddlePipe = NULL;

	m_ptrSwapChain = NULL;
	m_ptrAddSwapChain = NULL;


	m_pEventQuery = NULL;
	m_sAddSwapPara.BackBufferWidth = 0;

}



/** \brief 
*  得到当前的d3d格式是16位还是32位.
*
*  从不同的d3d格式中得到这个显示模式是16位还是32位.
*  \param _format 要检测的格式.
*  \return int 返回16或是32,如果是返回0,出错.
*/
int osc_d3dManager::formatlength( D3DFORMAT _format )
{
	guard;// osc_d3dManager::formatlength() );

	switch( _format )
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_R8G8B8:
		return 32;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
		return 16;
	default:
		osassert( FALSE );
	}
	
	return 0;
	unguard;
}


/** \brief
*  找到一个可满足要求的device.
*
*  从已经建立的设备列表中找到一个可以满足要求的Device,对于不要求的项,比如SampleType
*  或是framerate,我们不需要在寻找Init device时考虑.如果找到设备,
*  填充变量m_d3dSettings.
* 
*  \param _initdata   要寻找设备的指标数据.
*  \return bool 如果找到设备填充内部变量m_d3dSettings,返回真.
* 
*/
bool osc_d3dManager::find_initdevice( const os_deviceinit* _initdata,int _time/* = 0*/ )
{
	guard;

	D3DDISPLAYMODE     bestDisplayMode;
    D3DAdapterInfo*    pBestAdapterInfo = NULL;
    D3DDeviceInfo*     pBestDeviceInfo = NULL;
    D3DDeviceCombo*    pBestDeviceCombo = NULL;

	osassert( _initdata );

	//  保存常用的屏幕数据.
	if( _initdata->m_bFullScr )
		m_iScrBpp    = _initdata->m_dwColorBits;
	m_iScrWidth  = _initdata->m_dwScrWidth;
	m_iScrHeight = _initdata->m_dwScrHeight;

	//  如果需要的是窗口的设备,调用函数find_windowedDevice...
	if( !_initdata->m_bFullScr )
	{
		return find_windowedDevice( _initdata );
	}


	for( UINT iai = _time; iai < this->m_Enumeration.m_pAdapterInfoList->Count(); iai++ )
	{

		// 检测每一个Adapter的每一个device.先得到Adapter's Information.
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_Enumeration.m_pAdapterInfoList->GetPtr(iai);
        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
		{

			// 检测每一个设备,先得到设备信息.
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);

			// 在实际应用程序中不需要ref 渲染器.
			if( (pDeviceInfo->DevType == D3DDEVTYPE_REF)&&(!g_bShaderDebug) )
				continue;

			// 检测每一个device combo 的数据,找到最优的device combo. 
            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
			{
				// 先得到device combo的数据.
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);

				if( pDeviceCombo->IsWindowed )
					continue;

				// 找到需要的色彩深度.
				if( (formatlength( pDeviceCombo->BackBufferFormat ) == _initdata->m_dwColorBits)&&
					(formatlength( pDeviceCombo->AdapterFormat ) == _initdata->m_dwColorBits ) )
				{
                    pBestAdapterInfo = pAdapterInfo;
                    pBestDeviceInfo = pDeviceInfo;
                    pBestDeviceCombo = pDeviceCombo;

					goto EndFullscrDeviceComboSearch;
				} 
			} // 以每一个device combo的检测.
		} // 设备检测. 
	} // Adapter检测.


EndFullscrDeviceComboSearch:

    // 如果到此还没有找到可以用的device combo,返回错误.
	if (pBestDeviceCombo == NULL)
	{
		osassert( false );
        return false;
	}

    bestDisplayMode.Width = 0;
    bestDisplayMode.Height = 0;
    bestDisplayMode.Format = D3DFMT_UNKNOWN;
    bestDisplayMode.RefreshRate = 0;


	// 查找需要的显示模式.
    for( UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++ )
    {
        D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
        if( pdm->Format != pBestDeviceCombo->AdapterFormat )
            continue;

		if( (pdm->Width == _initdata->m_dwScrWidth)&&
			(pdm->Height == _initdata->m_dwScrHeight)
			&&		(pdm->RefreshRate>=MIN_FRASHRATE )
			)
		{
			bestDisplayMode = *pdm;
			osDebugOut( "The Display width,height,frame is:<%d,%d,%d>..\n",
				pdm->Width,pdm->Height,pdm->RefreshRate );
			break;
		}
	}

	// 如果找不到需要的显示模式,返回false.
	if( bestDisplayMode.Width == 0 )
	{
		osassert( false );
		return false;
	}

	//  填写固定的3d Setting 信息.
	m_d3dSettings.IsWindowed = false;
    m_d3dSettings.Fullscreen_MultisampleQuality = 0;
	m_d3dSettings.Fullscreen_PresentInterval = PRESENT_INTERVAL;

	//  设置其它的参数.
	m_d3dSettings.pFullscreen_AdapterInfo = pBestAdapterInfo;
    m_d3dSettings.pFullscreen_DeviceInfo  = pBestDeviceInfo;
    m_d3dSettings.pFullscreen_DeviceCombo = pBestDeviceCombo;
    m_d3dSettings.Fullscreen_DisplayMode  = bestDisplayMode;
    if (m_Enumeration.AppUsesDepthBuffer)
        m_d3dSettings.Fullscreen_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);
    m_d3dSettings.Fullscreen_MultisampleType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
    m_d3dSettings.Fullscreen_VertexProcessingType = 
		*(ose_VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);


	return true;

	unguard;
}


/** \brief 
*  find_initdevice函数的帮助函数.
*
*  如果要寻找的设备是窗口设备,find_initdevice函数就调用这个函数来寻找适合的窗口
*  设备.
*/
bool osc_d3dManager::find_windowedDevice( const os_deviceinit* _initdata )
{
	guard;// osc_d3dManager::find_windowDevice() );

	D3DDISPLAYMODE  primaryDesktopDisplayMode;

    D3DAdapterInfo* pBestAdapterInfo = NULL;
    D3DDeviceInfo*  pBestDeviceInfo = NULL;
    D3DDeviceCombo* pBestDeviceCombo = NULL;


	osassert( _initdata );

	//
    // Get display mode of primary adapter (which is assumed to be where the window 
    // will appear)
	//
	m_pD3D->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);
	

	// 检测每一个Adapter.
    for( UINT iai = 0; iai < m_Enumeration.m_pAdapterInfoList->Count(); iai++ )
    {
		// 对每一个Adapter,检测每一个设备.
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_Enumeration.m_pAdapterInfoList->GetPtr(iai);

        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
        {
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
            
			// 我们不使用Ref的设备,如果是Ref类型的设备,pass.
			if ( (pDeviceInfo->DevType == D3DDEVTYPE_REF)&&(!g_bShaderDebug) )
                continue;

            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
            {
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
                bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
                if (!pDeviceCombo->IsWindowed)
				{
					continue;
				}

                if (pDeviceCombo->AdapterFormat != primaryDesktopDisplayMode.Format)
				{
					continue;
				}
				//
                // If we haven't found a compatible DeviceCombo yet, or if this set
                // is better (because it's a HAL, and/or because formats match better),
                // save it
				//
                if( pBestDeviceCombo == NULL || 
                    pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceCombo->DevType == D3DDEVTYPE_HAL ||
                    pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
                {
                    pBestAdapterInfo = pAdapterInfo;
                    pBestDeviceInfo = pDeviceInfo;
                    pBestDeviceCombo = pDeviceCombo;
                    if( pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
                    {
                        // This windowed device combo looks great -- take it
                        goto EndWindowedDeviceComboSearch;

                    }
                    // Otherwise keep looking for a better windowed device combo
                }
            }
        }
    }

EndWindowedDeviceComboSearch:
    
	if (pBestDeviceCombo == NULL )
	{
		osDebugOut( "没有找到可用的3D设备......\n" );
		return false;
	}

    m_d3dSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
    m_d3dSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
    m_d3dSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
    m_d3dSettings.IsWindowed = true;
    m_d3dSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;

	m_d3dSettings.Windowed_Width = m_rcWindowBounds.right-m_rcWindowBounds.left;
	m_d3dSettings.Windowed_Height = m_rcWindowBounds.bottom-m_rcWindowBounds.top;

    if (m_Enumeration.AppUsesDepthBuffer)
        m_d3dSettings.Windowed_DepthStencilBufferFormat = 
		*(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);
    m_d3dSettings.Windowed_MultisampleType = 
		*(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
    m_d3dSettings.Windowed_MultisampleQuality = 0;
    m_d3dSettings.Windowed_VertexProcessingType = 
		*(ose_VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
    m_d3dSettings.Windowed_PresentInterval = *(UINT*)pBestDeviceCombo->pPresentIntervalList->GetPtr(0);

	return true;

	unguard;
}




/** \brief
*  在窗口发生变化时调整窗口的style,比如从全屏变到局屏需要调整窗口style.
*
*/
void osc_d3dManager::AdjustWindowForChange( void )
{
	guard;

    if( m_bWindowed )
    {
        // Set windowed-mode style
        //SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle  );
			
    }
	else
	{
		// Set fullscreen-mode style
		SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|/*WS_SYSMENU|*/WS_VISIBLE );
		
		// 如果是全屏模式，则设置处理菜单为空
		::SetMenu( m_hWnd,NULL );
    }


	unguard;
}



/** \brief 
*  Fill the D3DPRESENT_PARAMETERS struct.
*
*  使用我们准备好的CD3DSettings结构去填充创建3D设备必须的D3DPRESENT_PARAMETERS
*  结构.要操作的变量为:m_d3dpp
*/
void osc_d3dManager::BuildPresentParamsFromSettings( void )
{
	guard;


	// ATTENTION TO FIX:
	// 性能问题: 
    m_d3dpp.Windowed               = m_d3dSettings.IsWindowed;
    m_d3dpp.BackBufferCount        = 2;

	//! 使用全屏抗锯齿的话，强制使用 2 sample的sample方式
	if( g_bUseFSAA )
		m_d3dpp.MultiSampleType    = D3DMULTISAMPLE_2_SAMPLES;
	else
		m_d3dpp.MultiSampleType        = m_d3dSettings.MultisampleType();
    m_d3dpp.MultiSampleQuality     = m_d3dSettings.MultisampleQuality();

	// ATTENTION : 
	// 为了使用上次的渲染内容，必须使用flip或是copy，否则Discard
	// 会去除所有BackBuf内容。从d3d的文档看，如果是窗口模式，则flip会占用比较多的
	// 额外处理,copy在fullscreen下会占用资源。我们主要使用假全屏，所以使用copy.
	if( g_bUseFSAA )
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	else
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;


    m_d3dpp.EnableAutoDepthStencil = m_Enumeration.AppUsesDepthBuffer;
    m_d3dpp.hDeviceWindow          = m_hWnd;
    if( m_Enumeration.AppUsesDepthBuffer )
    {
		// 使用全屏抗锯齿的话，缓冲区不能锁定
		if( g_bUseFSAA )
		{
			// 如果使用全屏抗锯齿
			m_d3dpp.Flags          =  0;//D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		}
		else
		{
			m_d3dpp.Flags = 0;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		}


        m_d3dpp.AutoDepthStencilFormat = m_d3dSettings.DepthStencilBufferFormat();
    }
    else
    {
        m_d3dpp.Flags              = 0;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    }


    if( m_bWindowed )
    {

		m_d3dpp.BackBufferWidth  = m_rcClientBounds.right - m_rcClientBounds.left;
		m_d3dpp.BackBufferHeight = m_rcClientBounds.bottom - m_rcClientBounds.top;

		// River @ 2009-7-1: TEST CODE:
        m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;//m_d3dSettings.PDeviceCombo()->BackBufferFormat;
        m_d3dpp.FullScreen_RefreshRateInHz = 0;



		m_d3dpp.PresentationInterval = PRESENT_INTERVAL;

    }
    else
    {
        m_d3dpp.BackBufferWidth  = m_d3dSettings.DisplayMode().Width;
        m_d3dpp.BackBufferHeight = m_d3dSettings.DisplayMode().Height;

		m_rcClientBounds.left = 0;
		m_rcClientBounds.top = 0;
		m_rcClientBounds.right = m_d3dSettings.DisplayMode().Width;
		m_rcClientBounds.bottom = m_d3dSettings.DisplayMode().Height;

		if( this->m_iScrBpp == 16 )
			m_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		else
			m_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

        m_d3dpp.FullScreen_RefreshRateInHz = m_d3dSettings.Fullscreen_DisplayMode.RefreshRate;


		m_d3dpp.PresentationInterval = PRESENT_INTERVAL;

    }


	m_d3dpp.PresentationInterval = PRESENT_INTERVAL;

	return;

	unguard;
}



/** \brief 
*  创建3d渲染环境.
*  
*  从已经填充的CD3DSettings结构中得到我们需要创建3D device需要
*  的各种参数,创建3d Device.
*
*  创建步骤:
*
*  1: 先填充 D3DPRESENT_PARAMETERS 结构.
* 
*  2: 创建3d device.
*  
*/
bool osc_d3dManager::create_3denv( BOOL _bStartQuery )
{
	guard;

	HRESULT      hr;
	DWORD        behaviorFlags;

    D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
    D3DDeviceInfo* pDeviceInfo = m_d3dSettings.PDeviceInfo();

    m_bWindowed = m_d3dSettings.IsWindowed;

	//
	//  需要时加入改变窗口的函数.
    //  Prepare window for possible windowed/fullscreen change
    //  AdjustWindowForChange();
	//
	AdjustWindowForChange();

	//
    // Set up the presentation parameters
	//
    BuildPresentParamsFromSettings();

	//
	// 如果不能使用的device,返回错误.
	//
    if( pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
    {
        // Warn user about null ref device that can't render anything
        osassert( false);
		return false;
    }


	m_eVPT = m_d3dSettings.GetVertexProcessingType();
    if (m_d3dSettings.GetVertexProcessingType() == SOFTWARE_VP)
	{
		behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		osDebugOut( "\nD3D创建：选择了软件顶点处理\n" );
		g_vertexProType = SOFTWARE_VP;
	}
    else if (m_d3dSettings.GetVertexProcessingType() == MIXED_VP)
	{
		behaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
		osDebugOut( "\nD3D创建：选择混合顶点处理\n" );
		g_vertexProType = MIXED_VP;
	}
    else if (m_d3dSettings.GetVertexProcessingType() == HARDWARE_VP)
	{
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		osDebugOut( "\nD3D创建：选择硬件顶点处理\n" );
		g_vertexProType = HARDWARE_VP;
	}
    else if (m_d3dSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
	{
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		osDebugOut( "\nD3D创建：选择纯硬件顶点处理\n" );
		g_vertexProType = PURE_HARDWARE_VP;
	}
    else
	{
        behaviorFlags = 0; // TODO: throw exception
		osassert( false );
	}

	// 
	// 使d3d可以多线程安全的工作。
	behaviorFlags |= D3DCREATE_MULTITHREADED;
	

	//
	// Log 我们创建的设备类型.

	// Create the device
#if NVPERFHUD_ANALYSIS

	UINT AdapterToUse=D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL;
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for (UINT Adapter=0;Adapter<m_pD3D->GetAdapterCount();Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res;
		Res = m_pD3D->GetAdapterIdentifier(Adapter,0,&Identifier);
		if (strstr(Identifier.Description,"PerfHUD") != 0)
		{
			AdapterToUse=Adapter;
			DeviceType=D3DDEVTYPE_REF;
			break;
		}
	}

	hr = m_pD3D->CreateDevice( AdapterToUse,DeviceType,
                               m_hWnd,
							   D3DCREATE_HARDWARE_VERTEXPROCESSING, 
							   &m_d3dpp,
                               &m_pd3dDevice );

#else
	
    hr = m_pD3D->CreateDevice( m_d3dSettings.AdapterOrdinal(), pDeviceInfo->DevType,
                               m_hWnd, behaviorFlags, &m_d3dpp,
                               &m_pd3dDevice );
#endif


    if( SUCCEEDED( hr) )
    {
		osassert( m_pd3dDevice );

		// River @ 2010-4-22:强制使用query,去除多开时产生的花屏现象。
		if(1)/*_bStartQuery)*/{
			CreateD3DQuery();
		}
		

		

        // When moving from fullscreen to windowed mode, it is important to
        // adjust the window size after recreating the device rather than
        // beforehand to ensure that you get the window size you want.  For
        // example, when switching from 640x480 fullscreen to windowed with
        // a 1000x600 window on a 1024x768 desktop, it is impossible to set
        // the window size to 1000x600 until after the display mode has
        // changed to 1024x768, because windows cannot be larger than the
        // desktop.
		
        if( m_bWindowed )
        {
			// River @ 2006-4-1:ATTENTION TO FIX:
			// 在某些机器，如果创建设备时，隐藏窗口，可能会出"Driver internal Error"的错误
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          SWP_SHOWWINDOW/*SWP_HIDEWINDOW*/ );
        }

       m_dwCreateFlags = behaviorFlags;
  
		//
		// 得到设备的caps.
		get_deviceCaps();


		// Confine cursor to fullscreen window
		if (!m_bWindowed )
		{
			RECT rcWindow;
			GetWindowRect( m_hWnd, &rcWindow );
			ClipCursor( &rcWindow );
			osDebugOut( "The window rect is:<%d,%d>..\n",
				rcWindow.right,rcWindow.bottom );
		}
		else
		{
			ClipCursor( NULL );
		}

#if __YY__
		if(!g_disableYY){
			yy_init();
		}		
#endif //__YY__


		//
		// 已经成功创建设备,并且成功的设置设备所需要的其它参数,
		// 返回true.
		//
		return true;
    }
	else
	{
		std::string msg = DXGetErrorDescription9(hr);
		char buff[256+512] = {0};
		strcpy( buff, "DevType:" );
		if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
			strcat( buff, "D3DDEVTYPE_HAL" );
		else if( pDeviceInfo->DevType == D3DDEVTYPE_REF )
			strcat( buff, "D3DDEVTYPE_REF" );
		else if( pDeviceInfo->DevType == D3DDEVTYPE_SW )
			strcat( buff, "D3DDEVTYPE_SW" );
		strcat( buff, "\n" );

		char d3dpp[512];
		sprintf( d3dpp, "\nERROR设备创建失败!\n出错原因:%s,backBufferWidth:%d, backBufferHeight:%d, format:%d, count:%d\nMultiSampleType:%d, MultiSampleQuality:%d\nSwapEffect:%d, Windowed:%d, EnableAutoDepthStencil:%d, AutoDepthStencilFormat:%d\nFlags:%d, RefreshRate:%d, PresentationInterval:%d\n",
			msg.c_str(),
			m_d3dpp.BackBufferWidth, 
			m_d3dpp.BackBufferHeight,
			m_d3dpp.BackBufferFormat,
			m_d3dpp.BackBufferCount,
			m_d3dpp.MultiSampleType,
			m_d3dpp.MultiSampleQuality,
			m_d3dpp.SwapEffect,
			m_d3dpp.Windowed,
			m_d3dpp.EnableAutoDepthStencil,
			m_d3dpp.AutoDepthStencilFormat,
			m_d3dpp.Flags,
			m_d3dpp.FullScreen_RefreshRateInHz,
			m_d3dpp.PresentationInterval );

		strcat( buff, d3dpp );

		osDebugOut( buff );

		return false;
		//MessageBox( NULL, buff, "ERROR设备创建失败!", MB_OK );
	}

	// River mod @ 2008-5-23:此处重设ViewPort.
	D3DVIEWPORT9 t_resetVp;
	t_resetVp.X      = 0;
	t_resetVp.Y      = 0;
	t_resetVp.Width  = m_d3dpp.BackBufferWidth;
	t_resetVp.Height = m_d3dpp.BackBufferHeight;
	t_resetVp.MinZ   = 0.0f;
	t_resetVp.MaxZ   = 1.0f;
	m_pd3dDevice->SetViewport( &t_resetVp );

	// 如果创建设备失败,不需要创建Ref device,直接返回错误.
	return false;

	unguard;
}

#if __YY__
#include "../../YYcommon/DemoInterface.h"
	bool osc_d3dManager::yy_init(void)
	{
		guard;

		if( g_yyDemo.InitFactory() == FALSE ){
			osDebugOut("yy can't be initialized error code<%d,%d>",::GetLastError(),g_yyDemo.GetErrorCode());
			return false; //can't init
		}

		g_yyDemo.m_pfnLoadInGame("LY_LMQT|YY3D9|NONE");//这个参数需要和YY技术人员重新取得

		const char* szYYFilePath = "Bin\\YY3804.zip";

		//如果是画中画项目就删除此行的注释
		//g_yyDemo.m_pfnRunService( szYYFilePath );

		//如果是OpenId项目就删除此行的注释
		g_yyDemo.m_pfnCheckClient( szYYFilePath, NULL );

		g_yyDemo.m_pfnSetMainWnd( m_hWnd );
		g_yyDemo.m_pfnCreateUI( m_pd3dDevice );

		return true;

		unguard;
	}

	int	osc_d3dManager::yy_msg_process(UINT _msg,WPARAM _wParam,LPARAM _lParam)
	{

		guard;

		if( g_yyDemo.IsInit())
		{   //此行代码必须调用，否则无法显示
			if( g_yyDemo.m_pfnGameWndMsg( _msg, _wParam,_lParam ) == 0 ) return 1;
		}

		return 0;

		unguard;
	}
	void osc_d3dManager::yy_show_openId(BOOL _show,int _x,int _y)
	{
		if(g_yyDemo.IsInit()){
			g_yyDemo.m_pfnShowOpid( _show, _x, _y );
		}
		
	}
	void osc_d3dManager::yy_set_callback(PFN_SETCALLBACK _callback)
	{
		guard;

		if(g_yyDemo.IsInit()){
			g_yyDemo.m_pfnSetCallBack( _callback );
		}		

		unguard;
	}
	void osc_d3dManager::yy_set_login_key(const char* _key)
	{
		guard;

		if(g_yyDemo.IsInit()){
			g_yyDemo.m_pfnSetLoginKey(_key);
		}

		unguard;
	}
	void osc_d3dManager::yy_set_login_failed(const char* _why)
	{
		guard;

		if(g_yyDemo.IsInit()){
			g_yyDemo.m_pfnLoginFailure( _why );
		}	

		unguard;
	}
	void osc_d3dManager::yy_set_login_succ(void)
	{
		guard;

		if(g_yyDemo.IsInit()){
			g_yyDemo.m_pfnLoginSucceed();
		}	

		unguard;
	}
#endif // __YY__

void osc_d3dManager::CreateD3DQuery(void)
{
	guard;

	SAFE_RELEASE(m_pEventQuery);

	// Create a query object
	//
	HRESULT hr = m_pd3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, NULL);

	if(hr == D3D_OK){
		m_pd3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_pEventQuery);
	}else{
		m_pEventQuery = NULL;
	}

	unguard;
}

//! 重新设置d3dDevice的宽度，高度,用于上层调整游戏的分辨率
bool osc_d3dManager::reset_device( int _width,
			int _height,bool _windowed/* = true*/ )
{
	guard;

	m_d3dpp.BackBufferWidth = _width;
	m_d3dpp.BackBufferHeight = _height;

	// set the galaEff width and height( grow effect)
	//
	CGalaEff::sm_iWidth = _width;
	CGalaEff::sm_iHeight = _height;

	m_bWindowed = _windowed;

	AdjustWindowForChange();
	adjust_windowSize( m_hWnd,_width,_height, _windowed);
	

	reset_3denv( true );

	// River mod @ 2008-5-23:此处重设ViewPort.
	D3DVIEWPORT9 t_resetVp;
	t_resetVp.X      = 0;
	t_resetVp.Y      = 0;
	t_resetVp.Width  = m_d3dpp.BackBufferWidth;
	t_resetVp.Height = m_d3dpp.BackBufferHeight;
	t_resetVp.MinZ   = 0.0f;
	t_resetVp.MaxZ   = 1.0f;
	m_pd3dDevice->SetViewport( &t_resetVp );

	return true;

	unguard;
}



/** \brief 
*  Reset 3d 渲染环境.
*
*  重新设置3d 渲染环境,比如从窗口模式到全屏模式,或是全屏模式不同Mode
*  的切换.
*
*/
bool osc_d3dManager::reset_3denv( bool _forceReset )
{
	guard;

	HRESULT     t_hr;

	//! River @ 2009-11-4:进入Reset设备的临界区
	::EnterCriticalSection( &g_sShadowSec );

	// 
	// 先释放所有的user created state blocks和中间管道的资源
	osassert( m_ptrMiddlePipe );
	osassert( m_ptrSmMgr );
	os_stateBlockMgr::Instance()->sbm_onLostDevice();
	osc_TGManager::on_deviceLost(m_pd3dDevice);
	m_ptrMiddlePipe->mdpipe_onLostDevice();
	m_ptrSmMgr->smMgr_onLostDevice();
	m_sPicRD.scrpic_onLostDevice();
	gFontManager.OnLostDevice();
	g_song12x12.OnLostDevice(m_pd3dDevice);
	g_song14x14.OnLostDevice(m_pd3dDevice);
	gVolumeFogPtr->OnLostDevice();

	// River @ 2010-4-8:
	g_shaderMgr->getTexMgr()->reset_textureMgr();		

	MapTile::m_ptrMapTerrain->onLostMapTerrain();

	// 内部的RenderTarget.
	for( int t_i=0;t_i<(int)m_vecRenderTarget.size();t_i ++ )
	{
		// River @ 2007-2-10:先释放在屏幕场景内的纹理资源计数
		int texid = m_vecRenderTarget[t_i].texid;
		if( texid >= 0 )
			m_vecSceneScrTexture[texid].release_texture( texid );

		m_vecRenderTarget[t_i].on_lostDevice();
	}

	if( callback_lostDevice )
		callback_lostDevice(0,0);

	
	// River mod @ 2008-8-7: release query
	const bool t_d3dQuery = (m_pEventQuery != NULL);
	SAFE_RELEASE(m_pEventQuery);

	//! River @ 2010-3-2:释放add swap chain.
	SAFE_RELEASE( m_ptrAddSwapChain );

	// Reset the 3d env.
	t_hr = m_pd3dDevice->TestCooperativeLevel();
	if( !_forceReset )
	{
		while( t_hr != D3DERR_DEVICENOTRESET )
		{
			// 尝试次数过多后，则退出进行上层的执行
			t_hr = m_pd3dDevice->TestCooperativeLevel();
		}
	}


    if( FAILED( t_hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
	{
		// sleep first and wait the device prepare
		//
		::Sleep(3000);

		if( t_hr == D3DERR_DEVICELOST )
		{
			while( D3DERR_DEVICELOST == t_hr )
				t_hr = m_pd3dDevice->Reset( &m_d3dpp );				
		}

		//@{ 
		//! River mod @ 2011-4-2:多次重设设备，再看是否能够成功.
		if( FAILED( t_hr ) )
		{
			int   t_iTimes = 0;
			while( t_iTimes < 5 )
			{
				t_hr = m_pd3dDevice->Reset( &m_d3dpp );
				if( FAILED( t_hr ) )
				{
					// 稍后再试.
					::Sleep( 10 );
					t_iTimes ++;
				}
				else
					break;
			}
		}
		//@} 

		switch( t_hr )
		{
		case D3DERR_OUTOFVIDEOMEMORY:
			osassertex( false,
				va( "重设渲染设备失败,失败代码<D3DERR_OUTOFVIDEOMEMORY>" ) );
			break;
		case D3DERR_DEVICELOST:
			osassertex( false,
				va( "重设渲染设备失败,失败代码<D3DERR_DEVICELOST>" ) );
			break;
		case D3DERR_DRIVERINTERNALERROR:
			osassertex( false,
				va( "重设渲染设备失败,失败代码<D3DERR_DRIVERINTERNALERROR>" ) );
			break;
		case D3D_OK:
			break;
		default:
			osassertex( false,
				va( "非预知错误<%s>...\n",osn_mathFunc::get_errorStr( t_hr ) ) );
			break;
		}

		//! River @ 2009-11-4:退出Reset设备的临界区
		::LeaveCriticalSection( &g_sShadowSec );

		if( FAILED( t_hr ) )
			return false;
	}
	//! River @ 2009-11-4:退出Reset设备的临界区
	::LeaveCriticalSection( &g_sShadowSec );

	if( FAILED( t_hr ) )
		osassertex( false,va("Reset device failed<%s>..\n",osn_mathFunc::get_errorStr( t_hr ) ) );

    // Store render target surface desc
    LPDIRECT3DSURFACE9 pBackBuffer;
    t_hr = m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	if( FAILED( t_hr ) )
	{

		if( D3DERR_DEVICELOST == t_hr )
			return false;

		osassertex( false,va("得到后端表面失败<%s>..\n",osn_mathFunc::get_errorStr( t_hr ) ) );
		return false;
	}
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();

	// Store device Caps
	m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
	// Get the max clip plane number.
	m_iMaxClipPlane = m_d3dCaps.MaxUserClipPlanes;

	// Confine cursor to fullscreen window
	if (!m_bWindowed )
	{
		RECT rcWindow;
		GetWindowRect( m_hWnd, &rcWindow );
		ClipCursor( &rcWindow );
	}
	else
	{
		ClipCursor( NULL );

		// 重设设备用到的光标
		set_cursor( m_iActiveCursorId );
	}



	// 
	// reset or recreat 所有的 user created state blocks和中间管道的资源
	
	m_ptrMiddlePipe->mdpipe_onResetDevice();
	
	m_ptrSmMgr->smMgr_onResetDevice();
	m_sPicRD.scrpic_onResetDevice();
	gFontManager.OnRestoreDevice();
	osc_TGManager::on_deviceRestor(m_pd3dDevice);
	gVolumeFogPtr->OnRestoreDevice(m_pd3dDevice);
	g_song12x12.OnResetDevice(m_pd3dDevice);
	g_song14x14.OnResetDevice(m_pd3dDevice);

	// River @ 2010-4-8:
	g_shaderMgr->getTexMgr()->restore_textureMgr();			
	
	// River added @ 2007-4-2:地表缓冲区需要重新填充
	MapTile::m_ptrMapTerrain->onRestoreMapTerrain();


	// 重新加入Query系统。
	if(t_d3dQuery){
		CreateD3DQuery();
	}

	// 内部的RenderTarget.
	for( int t_i=0;t_i<(int)m_vecRenderTarget.size();t_i ++ ){
		m_vecRenderTarget[t_i].on_restoreDevice( m_pd3dDevice );
			
		int texid = m_vecRenderTarget[t_i].texid;
		if( texid >= 0 )
		{
			m_vecSceneScrTexture[texid].tex = m_vecRenderTarget[t_i].pRenderTarget;
			m_vecSceneScrTexture[texid].tex->AddRef();
			m_vecSceneScrTexture[texid].add_ref();
			m_vecSceneScrTexture[texid].inuse = true;
		}
	}

	if( callback_resetDevice )
		callback_resetDevice(0,0);

	// River @ 2010-3-2:重新创建add swap chain.
	if( m_sAddSwapPara.BackBufferWidth > 0 )
	{
		t_hr = m_pd3dDevice->CreateAdditionalSwapChain( 
			&m_sAddSwapPara,&m_ptrAddSwapChain );
		if( FAILED( t_hr ) )
		{
			// River @ 2011-2-16:处理设备再次丢失的bug.
			if( D3DERR_DEVICELOST == t_hr )
				return false;
			osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		}
	}


	// 成功Reset 3d渲染环境.
	return true;


	unguard;
}


//! 处理一些集成显卡的名字,使玩家知道为什么不能玩边缘.
bool osc_d3dManager::is_onboardGraphicCard( const char* _des )
{
	guard;

	s_string     t_str = _des;
	s_string     t_sz[] = 
	{
		"S3 Graphics ProSavageDDR",
		"NVIDIA RIVA TNT2 Model 64",
	};

	s_string     t_szSub;
	for( int t_i=0;t_i<2;t_i ++ )
	{
		int t_iSize = t_sz[t_i].length();
		if( t_iSize > (int)t_str.length() )
			t_iSize = (int)t_str.length();

		t_szSub = t_str.substr( 0,t_sz[t_i].length() );
		if( t_szSub == t_sz[t_i] )
			return true;
	}

	return false;

	unguard;
}


//! 如果是窗口模式，创建设备前，先调整窗口的大小和位置。
void osc_d3dManager::adjust_windowSize( HWND _hwnd,int _width,int _height,bool _windowed)
{
	guard;

	BOOL t_seriousProcess = FALSE;

adjust_windowSize_flag:

	DWORD        t_winStyle;

	// 调整客户区大小到我们需要的大小.
	RECT   t_rect;
	t_rect.left = 0,t_rect.top = 0;
	t_rect.right = _width;
	t_rect.bottom = _height;
	

	t_winStyle = GetWindowLong( _hwnd,GWL_STYLE );

	// 如果是子窗口，SetWindowPos 所设置的大小和 GetClientRect 就会有不等的
	// 用于兼容使用fdrEngine 来建立的编辑器，使其不弹出提示窗口。
	//
	const BOOL t_isChlidWnd = t_winStyle & WS_CHILDWINDOW;

	// 根据当前窗口是否有menu进行处理.
	if( ::GetMenu( _hwnd ) )
		::AdjustWindowRectEx( &t_rect,t_winStyle,true,NULL );
	else
		::AdjustWindowRectEx( &t_rect,t_winStyle,false,NULL );

	// River added @ 2009-2-26:使创建的窗口位于屏幕的正中间
	int t_xOffset = _windowed?(GetSystemMetrics( SM_CXFULLSCREEN ) - (t_rect.right - t_rect.left))>> 1 : 0;
	int t_yOffset = _windowed?(GetSystemMetrics( SM_CYFULLSCREEN ) - (t_rect.bottom - t_rect.top))>> 1 : 0;
	if(t_yOffset < 0) t_yOffset = 0;
	if(t_xOffset < 0) t_xOffset = 0;
	

	// River @ 2006-4-1:ATTENTION TO FIX:
	// 在某些机器，如果创建设备时，隐藏窗口，可能会出"Driver internal Error"的错误
	BOOL t_bSet = SetWindowPos( 
		m_hWnd,(t_seriousProcess?HWND_BOTTOM:HWND_TOP),t_xOffset,t_yOffset,t_rect.right-t_rect.left,
		t_rect.bottom-t_rect.top, SWP_SHOWWINDOW /*SWP_HIDEWINDOW*/ );

	::GetWindowRect( m_hWnd,&m_rcWindowBounds );
	::GetClientRect( m_hWnd,&m_rcClientBounds );

	if( (m_rcClientBounds.right - m_rcClientBounds.left != _width
		|| m_rcClientBounds.bottom - m_rcClientBounds.top != _height) && !t_isChlidWnd){

		// 如果屏幕的分辨率比较小，有标题栏的窗口就会出现分辨率高宽不能设置的情况
		// 于是就要去掉标题栏。
		//
		if(!t_seriousProcess){

			//::MessageBoxA(m_hWnd,"您的电脑分辨不足，只能以非常模式运行游戏，保证游戏正常运行。","严重提示！",MB_ICONERROR);

			LONG t_dwWindowStyle = ::GetWindowLong(m_hWnd,GWL_STYLE);
			t_dwWindowStyle &= ~ (WS_MAXIMIZEBOX|WS_SIZEBOX| (WS_OVERLAPPED|WS_CAPTION|WS_GROUP|WS_SYSMENU));
			t_dwWindowStyle |= WS_POPUP;

			::SetWindowLong(m_hWnd,GWL_STYLE,t_dwWindowStyle);

			t_dwWindowStyle = ::GetWindowLong(m_hWnd,GWL_EXSTYLE);
			t_dwWindowStyle |= WS_EX_TOPMOST;

			::SetWindowLong(m_hWnd,GWL_EXSTYLE,t_dwWindowStyle);

			t_seriousProcess = TRUE;

			goto adjust_windowSize_flag;

		}else{
			// 去掉标题栏的窗口，都不能达到要求，就只能建议了。。。
			//
			::MessageBoxA(m_hWnd,"由于您的电脑仍然无法正常游戏，建议升级电脑。","严重严重提示！",MB_ICONERROR);
		}
	}

	// River added @ 2007-4-2:在最新的dx上,如果后端缓冲区大小和视口不一致,在
	// debug版本的dx上会出现问题
	// River mod 2008-4-13:使用windows的client Bounds.
	if( FALSE )
	{
		m_rcClientBounds.right = m_rcClientBounds.right;//_initdata->m_dwScrWidth;
		m_rcClientBounds.bottom = m_rcClientBounds.bottom;//_initdata->m_dwScrHeight;
	}
	else
	{
		m_rcClientBounds.right = _width;
		m_rcClientBounds.bottom = _height;
	}


	unguard;
}


//! 释放当前设备的additional swap chain.
void osc_d3dManager::release_addSwapChain( void )
{
	guard;

	if( m_ptrAddSwapChain == NULL )
		return;

	SAFE_RELEASE( m_ptrAddSwapChain );

	unguard;
}


//! 创建额外的swapChain,用于多窗口渲染
bool osc_d3dManager::create_addSwapChain( int _width,int _height,HWND _hwnd )
{
	guard;

	if(_hwnd == NULL){
		// 释放 交换链
		release_addSwapChain();
		m_sAddSwapPara.BackBufferHeight = 0;
		m_sAddSwapPara.BackBufferWidth = 0;
		return true;
	}

	if( (_width<=0)||(_height<=0) )
		return true;

	osassert( this->m_pd3dDevice );

	if(m_ptrAddSwapChain){
		// 先释放原来的交换链
		release_addSwapChain();
	}

	D3DPRESENT_PARAMETERS   t_para;
	HRESULT                 t_hr;

	//! 如果完全一致，直接返回
	if( m_ptrAddSwapChain &&
		(_width == m_sAddSwapPara.BackBufferWidth) &&
		(_height == m_sAddSwapPara.BackBufferHeight )&&
		( _hwnd == m_sAddSwapPara.hDeviceWindow ) )
		return true;

	memcpy( &t_para,&m_d3dpp,sizeof( D3DPRESENT_PARAMETERS ) );

	t_para.BackBufferCount = 2;
	t_para.BackBufferFormat = D3DFMT_A8R8G8B8;
	t_para.BackBufferWidth = _width;
	t_para.BackBufferHeight = _height;
	t_para.hDeviceWindow = _hwnd;
	t_para.SwapEffect = D3DSWAPEFFECT_DISCARD;
	t_para.EnableAutoDepthStencil = FALSE;

	t_para.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	t_hr = m_pd3dDevice->CreateAdditionalSwapChain( &t_para,&m_ptrAddSwapChain );
	if( FAILED( t_hr ) )
	{
		// River @ 2011-10-18：更温和的处理。
		return false;
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
	}

	memcpy( &m_sAddSwapPara,&t_para,sizeof( D3DPRESENT_PARAMETERS ) );

	return true;

	unguard;
}



/** \brief 
*  创建我们需要的3d Device.
*
*  这个函数是I_deviceManager中的接口,实现步履:
*  
*  1: 创direct3d object.
*  
*  2: Enumerate the device list.
*
*  3: Find the best device we need .
*
*  4: Create the device&return;
*
*/
bool osc_d3dManager::init_device( HWND _hwnd,const os_deviceinit* _initdata,
							  void* _cfm_ptr )
{
	guard;// osc_d3dManager::init_d3d() );

	HRESULT     hr;

	os_deviceinit t_init;

	osassert( _initdata );
	osassert( _cfm_ptr );


	//
	// 接下来初始化我们的3d显示设备.
	//
	this->m_hWnd = _hwnd;
	this->cfm_deviceptr = (CFMPTR*)_cfm_ptr;

	if( !_initdata->m_bFullScr )
		adjust_windowSize( _hwnd,_initdata->m_dwScrWidth,_initdata->m_dwScrHeight ,true);
	else
	{
		// ATTENTION TO FIX: 强制使用上层指定的大小窗口?
		m_rcWindowBounds.left = 0;
		m_rcWindowBounds.right = _initdata->m_dwScrWidth;
		m_rcWindowBounds.bottom = _initdata->m_dwScrHeight;
		m_rcWindowBounds.top = 0;

	}


	// 不是使用d3d的硬件光标.
	m_bShowCursorWhenFullscreen = _initdata->m_bHwCursor;

	//
	// 如果一开始程序是窗口模式,我们需要保存窗口的style.
	//
	if( !_initdata->m_bFullScr )
	{
		m_dwWindowStyle = GetWindowLong( m_hWnd,GWL_STYLE );
		SetWindowLong( m_hWnd,GWL_STYLE,m_dwWindowStyle );
    }
	else
	{
		m_dwWindowStyle = 
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
						  WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
	}


	//
	// 1: 创建direct3d object.
	//
    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	
    if( m_pD3D == NULL )
	{
		osassertex(false,"创建d3d9失败");
		//MessageBox( NULL, "创建d3d9失败","",MB_OK );
		osassert( FALSE );
		return false;
	}

	//
	// 2: Enumerate the device list.
	//
	this->m_Enumeration.SetD3D( m_pD3D );
	this->m_Enumeration.ConfirmDeviceCallback = (CONFIRMDEVICECALLBACK) cfm_deviceptr;
	m_Enumeration.AppRequiresFullscreen = false;//_initdata->m_bFullScr;
	m_Enumeration.AppRequiresWindowed = false;


	//
	// 3: Find the best device we need .使用一步步降低标准的算法。
	//
	memcpy( &t_init,_initdata,sizeof( os_deviceinit ) );
	t_init.m_dwColorBits = 32;
	t_init.m_dwDepthBits = 24;
	t_init.m_dwStencilBits = 8;
	m_iStencilBitsNum = 8;
	if( _initdata->m_dwColorBits == 16 )
		g_bUse16Color = TRUE;


	// 从_initdata中加入其它对device的限制.........
	if( _initdata->m_dwDepthBits>0 )
	{
		m_Enumeration.AppUsesDepthBuffer = true;
		m_Enumeration.AppMinDepthBits = _initdata->m_dwDepthBits;
	}
	m_Enumeration.AppMinStencilBits = m_iStencilBitsNum;
	if( FAILED( hr = this->m_Enumeration.Enumerate() ) )
	{
		MessageBox( NULL, "枚举设备失败","",MB_OK );
		SAFE_RELEASE( m_pD3D );
		osassert( FALSE );
		return false;
	}
	if( !find_initdevice( &t_init ) )
	{
		t_init.m_dwColorBits = 16;
		t_init.m_dwDepthBits = 15;
		t_init.m_dwStencilBits = 1;
		m_iStencilBitsNum = 1;

		m_Enumeration.AppMinStencilBits = m_iStencilBitsNum;
		if( FAILED( hr = this->m_Enumeration.Enumerate() ) )
		{
			MessageBox( NULL, "查找设备失败","",MB_OK );
			SAFE_RELEASE( m_pD3D );
			osassert( FALSE );
			return false;
		}

		// 查找第二个显卡驱动
		if( !find_initdevice( &t_init,1 ) )
		{
			t_init.m_dwStencilBits = 0;
			m_iStencilBitsNum = 0;

			m_Enumeration.AppMinStencilBits = m_iStencilBitsNum;
			if( FAILED( hr = this->m_Enumeration.Enumerate() ) )
			{
				MessageBox( NULL, "2次枚举设备失败","",MB_OK );
				SAFE_RELEASE( m_pD3D );
				osassert( FALSE );
				return false;
			}

			if( !find_initdevice( &t_init,1 ) )
			{
				MessageBox( NULL, 
					NOT_FOUND_DIVICE_TIPS,NOT_FOUND_DEVICE,MB_OK );
				return false;
			}
		}
	}

	if( !_initdata->m_bFullScr )
	{
		osDebugOut_tag("RenderDevice","使用以下渲染设备:<%s>",m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description);

		char t_desc[MAX_PATH];
		GetWindowsVerDesc(MAX_PATH,t_desc);
		osDebugOut_tag("System","操作系统信息 <%s>",t_desc);

		GetCpuInfoDesc(MAX_PATH,t_desc);
		osDebugOut_tag("CpuInfo","CPU 信息 <%s>",t_desc);

		GetMemSizeInfoDesc(MAX_PATH,t_desc);
		osDebugOut_tag("MemInfo","内存信息 <%s>",t_desc);

		// 根据显卡字串,检测显卡是否为集成显卡.
		if( is_onboardGraphicCard( 
			m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description ) )
		{
			char   t_sz[256];
			sprintf( t_sz,"您的显卡<%s>是主板集成显卡或是非常古老的显卡,请升级显卡后再起动游戏..",
				m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description );
			MessageBox( NULL,t_sz,"ERROR",MB_OK );

			return false;
		}
	}

	
	//
	// 4: Create the device&return;
	//
	if( !create_3denv(_initdata->m_bStartQuery) )
	{
		// River mod @ 2007-9-13:如果不是默认的屏幕大小,则创建一次默认大小尝试
		if(  (_initdata->m_dwScrWidth != 800) )
		{
			os_deviceinit   t_init;
			memcpy( &t_init,_initdata,sizeof( os_deviceinit ) );
			t_init.m_dwScrWidth = 800;
			t_init.m_dwScrHeight = 600;

			if( !init_device( _hwnd,&t_init,_cfm_ptr ) )
			{
				char  t_szErrorMsg[512];
				sprintf( t_szErrorMsg,"创建3D设备失败，请重新安装显卡驱动程序并检查显卡是否过老..." );
				MessageBox( NULL,t_szErrorMsg,"ERROR",MB_OK );

				return false;
			}			
		}
		else
		{
			char  t_szErrorMsg[512];
			sprintf( t_szErrorMsg,"创建3D设备失败，请重新安装显卡驱动程序并检查显卡是否过老..." );
			MessageBox( NULL,t_szErrorMsg,"ERROR",MB_OK );

			return false;
		}
		
	}
	

	// 杂项初始化
	init_miskRes();

	extern GFXCardProfiler* gCardProfiler ;
	gCardProfiler  = new GFXD3D9CardProfiler(m_pd3dDevice);
	gCardProfiler->init();
	// 最后成功的创建3d Device,返回真.

	//! 判断显卡等级，进行多层渲染
	extern bool gUseTerrainPS;
	if (m_d3dCaps.PixelShaderVersion<D3DPS_VERSION(2,0)&&
		m_d3dCaps.MaxTextureBlendStages<6)
	{
		osDebugOut("\n采用Mult-Pass方式渲染地表\n");
		gUseTerrainPS = false;
		g_bWaterReflect = false;
	}
	else if (gUseTerrainPS)
	{
		osDebugOut("\n采用Pixel Shader方式渲染地表\n");
	}
	//! 对于5200的显卡，使用低档水面
	if( !_initdata->m_bFullScr )
	{
		if(strstr(m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description,"5200")!=NULL)
		{
			gUseTerrainPS = false;
			g_bWaterReflect = false;
		}
	}
	static const char* t_RenderFog = StringTable->insert("RenderFog");
	g_bUseVolumeFog = gCardProfiler->getBooleanValue(t_RenderFog);

	//! 如果vertexshader&Ps版本小于2,使用无倒影的水面
	if( (m_d3dCaps.PixelShaderVersion  < D3DPS_VERSION(2,0)) ||
		(m_d3dCaps.VertexShaderVersion < D3DVS_VERSION(2,0))   )
		::g_bWaterReflect = false;

	// River mod @ 2007-3-26: 
	hr = m_pd3dDevice->GetSwapChain( 0,&m_ptrSwapChain );
	if( FAILED( hr ) )
		osassertex( false,"得到SwapChain出错..\n" );


	return true;

	unguard;
}


//! 是否支持我们旗帜的多层贴图方式.
bool osc_d3dManager::is_supportMlFlag( void )
{
	guard;

	//
	if( !(m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA) )
		return false;

	if( get_maxTextureNum() < 3 )
		return false;

	return true;

	unguard;
}


//! 初始化font相关数据和其它的杂项数据,在初始化完全局的StateBlockMgr后调用此函数。
void osc_d3dManager::init_miskRes( void )
{
	guard;

	// 先初始化渲染状态块的管理器，使其它的地方可以使用渲染状态块。
	os_stateBlockMgr::Instance()->init_sbMgr( m_pd3dDevice );

	
	//
	// 创建我们的字体信息.
	char   t_str[] = { NULL,NULL,NULL,NULL };
	gFontManager.Initialize(m_pd3dDevice);
	//gFontManager.RegisterFont( "黑体", 32 );//楷体_GB2312 方正细黑一_GBK

#if USE_PIXELFONT
	g_song12x12.OnCreateDevice( m_pd3dDevice );
	if( !g_song12x12.LoadFromFile( "ui\\gui\\fonts\\songti12x12.dat" ) )
		osassert( false );

	g_song14x14.OnCreateDevice( m_pd3dDevice );
	if( !g_song14x14.LoadFromFile( "ui\\gui\\fonts\\songti14x14.dat" ) )
		osassert( false );
#endif
	

	// 
	// 初始化文字，顺便记录一下当前的d3dMgr中需要的渲染状态块。
	record_scrTexRBlock();
	record_scrTexRBlock_rt();
	record_fullScrFadeRBlock();
	record_LensFlareRBlock();

	//
	// 渲染屏幕多边形需要的资源初始化
	m_sPicRD.init_scrPicRenderData( m_pd3dDevice );


	unguard;
}



/** \brief 
*  释放我们创建的3d Device.
*
*  在释放3d Device之前要把所有的3d资源都释放完.
*  
*/
void osc_d3dManager::release_device( void )
{
	guard;

	
	//
	// 释放我们的fontMgr.
	gFontManager.ReleaseFont();

	if( m_iScrTexRSId>=0 )
	{
		os_stateBlockMgr::Instance()->release_stateBlock( m_iScrTexRSId );
	}
	if( m_iScrTexRSAddId>= 0 )
	{
		os_stateBlockMgr::Instance()->release_stateBlock( m_iScrTexRSAddId );
	}
	if( m_iScrTexRSId_rt >= 0 )
	{
		os_stateBlockMgr::Instance()->release_stateBlock( m_iScrTexRSId_rt );
	}
	if( m_iFullScrFadeRSId>=0 )
	{
		os_stateBlockMgr::Instance()->release_stateBlock( m_iFullScrFadeRSId );
	}
	if( m_iFullscrAlphaId >= 0 )
		os_stateBlockMgr::Instance()->release_stateBlock( m_iFullscrAlphaId );


	if( m_iLensFlareRSId>=0 )
	{
		os_stateBlockMgr::Instance()->release_stateBlock( m_iLensFlareRSId );
	}
	g_song12x12.release();
	g_song14x14.release();
	
	
	//syq
	release_allSceScrTex( true );
	m_vecSceneScrTexture.clear();
	m_vecSPDisp.clear();
	m_vecTexture.clear();


	m_vecCursor.destroy_mgr();

	m_sPicRD.release_scrPicR();

	os_stateBlockMgr::DInstance();

	SAFE_RELEASE( m_ptrSwapChain );
	SAFE_RELEASE( m_ptrAddSwapChain );


	SAFE_RELEASE( m_pEventQuery );

	SAFE_RELEASE( m_pd3dDevice );
	SAFE_RELEASE( m_pD3D );

	unguard;
}





DWORD osc_d3dManager::d3d_present( const os_present* _present /*= NULL*/,bool _reset/* = true*/ )
{ 
	guard;

#if __YY__
	if(g_yyDemo.IsInit()){
		g_yyDemo.m_pfnRenderGUI();
	}	
#endif //__YY__


	HRESULT    t_hr;


	LPDIRECT3DSWAPCHAIN9   t_ptrSwap = NULL;

	if( _present )
		t_ptrSwap = this->m_ptrAddSwapChain;
	else
		t_ptrSwap = this->m_ptrSwapChain;

	if( t_ptrSwap == NULL )
		return 0;


# if 1
	if( m_pEventQuery ){

		// Add an end marker to the command buffer queue.
		m_pEventQuery->Issue(D3DISSUE_END);

		// Empty the command buffer and wait until the GPU is idle.
		while(S_FALSE == m_pEventQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ))
			::Sleep( 0 );
	}
# endif 
	
	DWORD t_dwS = timeGetTime();
	t_hr = t_ptrSwap->Present( NULL,NULL,NULL,NULL,0 );
	//DWORD t_dwE = timeGetTime();
	//osDebugOut( "The present time is:<%d>...\n",t_dwE - t_dwS );



	while( D3DERR_WASSTILLDRAWING == t_hr )
		t_hr = t_ptrSwap->Present( NULL,NULL,NULL,NULL,D3DPRESENT_DONOTWAIT );



	// 此处为会错，查出可能其它返回值。
	osassertex( (D3D_OK == t_hr) || (D3DERR_DEVICELOST == t_hr),
		va( "错误的返回值..%s <%d>",osn_mathFunc::get_errorStr( t_hr ),DWORD( t_hr ) )  );

	if( D3DERR_DEVICELOST == t_hr )
	{
		// 
		// River @ 2011-1-5:非主线程渲染的情况下，不处理设备重设。
		if( !_reset )
		{
			m_bInRender = false;
			return t_hr;
		}

		if( !reset_3denv() )
		{
			// River @ 2011-2-16: try again.
			if( !reset_3denv() )
				return E_FAIL;
		}
	}

	m_bInRender = false;

	return	t_hr;

	unguard;
}



/** \brief 
*  使用d3d的硬件光标.
*
*  这个函数在主程序的消息WM_MOUSEMOVE消息中调用,调用这个函数使用d3d的硬件光标.
*
*/
void osc_d3dManager::set_hwCursor( void )
{
	// 初始化完3d device后可以调用这个函数.
	if( m_pd3dDevice  )
	{
		::SetCursor( NULL );
		m_pd3dDevice->ShowCursor( TRUE );
	}
}



/**********************************************************************/
//@{
/** 
*  Function to display temp pictures.such as start screen or
*  load screen.
*/
int osc_d3dManager::load_texture( const char* _filename )
{
	guard;// osc_d3dManager::load_texture() );

	int              t_i;
	HRESULT           t_hr;

	osassert( _filename );

	t_i = (int)m_vecTexture.size();
	if( t_i==0 )
		m_vecTexture.resize( MIN_TMPTEXTURE );


	//
	// 如果当前场景的纹理中已经有可以使用的纹理,返回id.
	for( t_i=0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( strcmp( m_vecTexture[t_i].m_szTexName,_filename ) == 0 )
		{
			m_vecTexture[t_i].add_ref();
			return t_i;
		}
	}

	//
	// 得到一个可用的纹理存储单元.
	//
	for( t_i = 0;t_i<(int)m_vecTexture.size();t_i++ )
	{
		if( !m_vecTexture[t_i].inuse )
			break;
	}

	if( t_i == m_vecTexture.size() )
	{
		MessageBox( NULL,"没有足够的空间创建二维屏幕显示纹理","Error",MB_OK );
		osassert( false );
	}

	//
	// 使用得到的纹理存储单元调入临时要显示的纹理.
	// 这些纹理使用的是d3d's Managed Texture,所以不需要担心纹理丢失的问题.
	//
	int   t_iGBufIdx = -1;
	BYTE*   t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf( (char*)_filename,t_ptrStart,TMP_BUFSIZE );
	osassert( t_iSize>0 );
	osassert( m_pd3dDevice );
	t_hr = D3DXCreateTextureFromFileInMemoryEx( m_pd3dDevice,
		t_ptrStart,t_iSize,
		D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,
		0,D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED, 
		D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		0, NULL, NULL, &m_vecTexture[t_i].tex  );
	
	if( FAILED( t_hr ) )
	{
		char buff[256] = {0};
		if( t_hr == D3DERR_NOTAVAILABLE ) 
			strcpy(  buff, "This device does not support the queried technique" );
		if( t_hr == D3DERR_OUTOFVIDEOMEMORY )
			strcpy( buff,  
			"Microsoft Direct3D does not have enough display memory to perform the operation." );
		if( t_hr == D3DERR_INVALIDCALL )
			strcpy( buff, 
			"The method call is invalid. For example, a method's parameter may have an invalid value." );
		if( t_hr == D3DXERR_INVALIDDATA )
			strcpy( buff, "The data is invalid." );
		if( t_hr == E_OUTOFMEMORY )
			strcpy( buff, "Direct3D could not allocate sufficient memory to complete the call." );

		strcat( buff,"\n" );
		strcat( buff, _filename );
		MessageBox( NULL, buff, "call D3DXCreateTextureFromFileInMemoryEx() ERROR", MB_OK );
	}
	
	END_USEGBUF( t_iGBufIdx );


	m_vecTexture[t_i].inuse = true;
	m_vecTexture[t_i].add_ref();
	strcpy( m_vecTexture[t_i].m_szTexName,_filename );


	//
	// 返回调入显示纹理的标识ID.
	//
	return t_i;

	unguard;
}

void osc_d3dManager::unload_texture( int _texid )
{
	guard;

	osassert( (_texid>=0)&&(_texid<(int)m_vecTexture.size()) );

	SAFE_RELEASE( m_vecTexture[_texid].tex );
	m_vecTexture[_texid].inuse = false;
	m_vecTexture[_texid].m_iRef = 0;
	m_vecTexture[_texid].m_szTexName[0] = NULL;

	unguard;
}

/** \brief
*  为场景中需要显示的二维图片创建纹理。
*/
int osc_d3dManager::create_sceneScrTex( const char* _filename )
{
	guard;// osc_d3dManager::create_sceneScrTex() );

	int          t_i;
	HRESULT      t_hr;

	osassert( _filename );

	t_i = (int)m_vecSceneScrTexture.size();
	if( t_i==0 )
		m_vecSceneScrTexture.resize( MIN_TMPTEXTURE );


	//
	// 如果当前场景的纹理中已经有可以使用的纹理,返回id.
	for( t_i=0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( (strcmp( m_vecSceneScrTexture[t_i].m_szTexName,_filename ) == 0)&&
			(m_vecSceneScrTexture[t_i].inuse )     )
		{
			m_vecSceneScrTexture[t_i].add_ref();
			
			return t_i;
		}
	}

	//
	// 得到一个可用的纹理存储单元.
	for( t_i = 0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( !m_vecSceneScrTexture[t_i].inuse )
			break;
	}

	if( t_i == m_vecSceneScrTexture.size() )
	{
//		MessageBox( NULL,"没有足够的空间创建二维屏幕显示纹理","Error",MB_OK );
		osDebugOut( "没有足够的空间创建二维屏幕显示纹理, maxsize:<%d>\n", t_i );
		osassert( false );
	}

	//osDebugOut( "创建2d纹理:<%d><%s>\n", t_i, _filename ); 

	// 
	// 先读入纹理到内存。
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf( (char*)_filename,t_ptrStart,TMP_BUFSIZE );
	osassertex( (t_iSize>0),(char*)_filename );

	//
	// 得到纹理的信息,看是否符合我们的纹理格式.
	D3DXIMAGE_INFO   t_info;

	t_hr = D3DXGetImageInfoFromFileInMemory( t_ptrStart,t_iSize,&t_info );
	if( FAILED( t_hr ) )
	{
		MessageBox( NULL,_filename,"打开文件出错",MB_OK );
		osassertex( false,va( "打开文件<%s>出错...\n",_filename ) );
		return -1;
	}


# if __TEXTURE_SIZELIMIT__
	if( (t_info.Width>256)||(t_info.Height>256 ) )
	{
		MessageBox( NULL,"纹理超过256的限制大小",_filename,MB_OK );
		osassert( false );
	}
# endif 


	//
	// 使用得到的纹理存储单元调入临时要显示的纹理.
	// 这些纹理使用的是d3d's Managed Texture,所以不需要担心纹理丢失的问题.
	t_hr = D3DXCreateTextureFromFileInMemoryEx( m_pd3dDevice,
		t_ptrStart,t_iSize,
		D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,
		0,D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED, 
		D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		0, NULL, NULL, &m_vecSceneScrTexture[t_i].tex  );

	osassertex( !FAILED( t_hr ),va( "The Error des is:<%s>..fileName is:<%s>\n",
		osn_mathFunc::get_errorStr( t_hr),(char*)_filename ) );

	END_USEGBUF( t_iGBufIdx );



	m_vecSceneScrTexture[t_i].add_ref();
	m_vecSceneScrTexture[t_i].inuse = true;
	strcpy( m_vecSceneScrTexture[t_i].m_szTexName,_filename );


	//
	// 返回调入显示纹理的标识ID.
	//
	return t_i;

	unguard;
}

LPDIRECT3DTEXTURE9 osc_d3dManager::get_sceneScrTex( int _id )
{
	guard;
	if( _id < 0 || _id >= (int)m_vecSceneScrTexture.size() )
		return NULL;
	return m_vecSceneScrTexture[_id].tex;
	unguard;
}

//! test 注册一个纹理,返回id
int osc_d3dManager::register_texid( LPDIRECT3DTEXTURE9 _pTexture )
{
	osassert( _pTexture );
	int t_i = (int)m_vecSceneScrTexture.size();
	if( t_i==0 )
		m_vecSceneScrTexture.resize( MIN_TMPTEXTURE );
	// 得到一个可用的纹理存储单元.
	int i;
	for( i = 0;i<(int)m_vecSceneScrTexture.size();i++ )
	{
		if( !m_vecSceneScrTexture[i].inuse )
			break;
	}
	if( i == m_vecSceneScrTexture.size() )
	{
		MessageBox( NULL,"没有足够的空间创建二维屏幕显示纹理","Error",MB_OK );
		osassert( false );
	}
	m_vecSceneScrTexture[i].tex = _pTexture;

	// River added @ 2007-2-10:因为从renderTarget注册，在释放时，不光在屏幕纹理
	// 内释放，也会在target列表内释放，所以加大此处renderTarget纹理的ref值
	m_vecSceneScrTexture[i].tex->AddRef();

	m_vecSceneScrTexture[i].add_ref();
	m_vecSceneScrTexture[i].inuse = true;

	return i;
}

int osc_d3dManager::register_targetAsTex( int _targetIdx )
{
	osassert( _targetIdx >=0 &&  _targetIdx < (int)m_vecRenderTarget.size() );
	int texid = register_texid( m_vecRenderTarget[_targetIdx].pRenderTarget );
	m_vecRenderTarget[_targetIdx].texid = texid;
	return texid;
}

//! 
void osc_d3dManager::unregister_texid( int _texid )
{
	osassert( _texid >= 0 && _texid < (int)m_vecSceneScrTexture.size() );
	m_vecSceneScrTexture[_texid].inuse = false;
	m_vecSceneScrTexture[_texid].m_iRef = 0;
	m_vecSceneScrTexture[_texid].tex = 0;
	m_vecSceneScrTexture[_texid].m_szTexName[0] = 0;


}

//! 后端缓冲区的renderTarget.
LPDIRECT3DSURFACE9 RENDERTARGET::m_pBackBufSurface = NULL;


//! 
void RENDERTARGET::on_lostDevice( void )
{
	guard;

	SAFE_RELEASE( m_pTargetSurface );
	SAFE_RELEASE( pRenderTarget );
	SAFE_RELEASE( m_pBackBufSurface );

	unguard;
}
void RENDERTARGET::on_restoreDevice( LPDIRECT3DDEVICE9 _dev )
{
	guard;

	osassert( _dev );
	HRESULT t_hr;
	t_hr = _dev->CreateTexture( this->m_iWidth,this->m_iHeight, 
		0, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &pRenderTarget,NULL );
	osassert( !FAILED( t_hr ) );

	// ATTENTION TO FIX: 此处生成的m_pTargetSurface 的refCount的数目大于1,为什么???
	t_hr = pRenderTarget->GetSurfaceLevel( 0,&m_pTargetSurface );
	osassert( !FAILED( t_hr ) );


	if( NULL == m_pBackBufSurface )
		_dev->GetRenderTarget( 0, &m_pBackBufSurface );

	return;

	unguard;
}


//syq_mask 创建渲染目标
int osc_d3dManager::create_renderTarget( int _width, int _height, D3DFORMAT _format /*= D3DFMT_A8R8G8B8*/  )
{
	guard;

	RENDERTARGET r;
	r.id = (int)m_vecRenderTarget.size();
	r.m_iWidth = _width;
	r.m_iHeight = _height;
	r.format = _format;
	r.on_restoreDevice( m_pd3dDevice );

	m_vecRenderTarget.push_back( r );

	
	return r.id;

	unguard;
}

//syq_mask 获得渲染目标
LPDIRECT3DSURFACE9 osc_d3dManager::get_renderTarget( int _id )
{
	if( _id == -1 )
		return RENDERTARGET::m_pBackBufSurface;

	if( _id < 0 || _id >= (int)m_vecRenderTarget.size() )
		return NULL;
	return m_vecRenderTarget.at( _id ).m_pTargetSurface;
}

LPDIRECT3DTEXTURE9 osc_d3dManager::get_renderTexture( int _id )
{
	if( _id < 0 || _id >= (int)m_vecRenderTarget.size() )
		return NULL;
	return m_vecRenderTarget.at( _id ).pRenderTarget;
}



//! 创建一个纹理
int osc_d3dManager::create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture )
{
	int i;
	HRESULT hr;
	osassert( m_pd3dDevice );
	osassert( _pTexture );

	// 得到一个可用的纹理存储单元.
	for( i = 0;i<(int)m_vecSceneScrTexture.size();i++ )
	{
		if( !m_vecSceneScrTexture[i].inuse )
			break;
	}
	if( i == m_vecSceneScrTexture.size() )
	{
		MessageBox( NULL,"没有足够的空间创建二维屏幕显示纹理","Error",MB_OK );
		osassert( false );
	}

	IDirect3DTexture9* pTex = NULL;
	hr = m_pd3dDevice->CreateTexture( _width, _height, 1, D3DUSAGE_DYNAMIC, 
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTex, NULL );
	m_vecSceneScrTexture[i].tex = pTex;
	if( hr == D3DERR_INVALIDCALL )
	{
		osassert( false );
	}
	if( hr == D3DERR_OUTOFVIDEOMEMORY )
	{
		osassert( false );
	}
	if( hr == E_OUTOFMEMORY )
	{
		osassert( false );
	}
	osassert( m_vecSceneScrTexture[i].tex );

	m_vecSceneScrTexture[i].add_ref();
	m_vecSceneScrTexture[i].inuse = true;

	*_pTexture = m_vecSceneScrTexture[i].tex;
	return i;
}

//! 释放场景中的二维图片纹理。
void osc_d3dManager::release_sceneScrTex( int _texid )
{
	guard;

	osassertex( (_texid>=0)&&(_texid<(int)m_vecSceneScrTexture.size()),
		va("The error tex is:<%d>..\n",_texid ) );

	m_vecSceneScrTexture[_texid].release_texture( _texid );

	unguard;
}

//! 释放所有的场景中使用二维图片纹理。
void osc_d3dManager::release_allSceScrTex( bool _realRse/* = true*/ )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( m_vecSceneScrTexture[t_i].inuse )
		{
			// 
			// ATTENTION OT OPP: 可以在此处不释放纹理，增加下一个场景调入速度。
# if 0			
			if( _realRse )
			{
				SAFE_RELEASE( m_vecSceneScrTexture[t_i].tex );
				m_vecSceneScrTexture[t_i].m_szTexName[0] = NULL;
			}
# else
			SAFE_RELEASE( m_vecSceneScrTexture[t_i].tex );
			m_vecSceneScrTexture[t_i].m_szTexName[0] = NULL;
# endif 

			// 如果是假的释放，则只需要设置当前的inuse为false.
			m_vecSceneScrTexture[t_i].inuse = false;
			m_vecSceneScrTexture[t_i].m_iRef = 0;
		}
	}

	//syq_mask release all render target...
	for( int i=0;i<(int)m_vecRenderTarget.size();++i )
	{
		//unregister_texid( m_vecRenderTarget.at(i).id
		// 释放设备相关的数据
		m_vecRenderTarget[i].on_lostDevice();
	}
	SAFE_RELEASE( RENDERTARGET::m_pBackBufSurface );
	
	m_vecRenderTarget.clear();

	unguard;
}
//@}


int osc_d3dManager::get_availableTextureMem( void )
{
	guard;

	osassert( m_pd3dDevice );

	UINT   t_ut;

	t_ut = m_pd3dDevice->GetAvailableTextureMem();

	t_ut /= (1024*1024);

	return t_ut;

	unguard;
}


//syq
int osc_d3dManager::get_curUsedSceneScrTextureNum()
{
	guard;
	int n=0;
	for( int i=0;i<(int)m_vecSceneScrTexture.size();i++ )
	{
		if( m_vecSceneScrTexture[i].inuse )
			n++;
	}
	return n; 
	unguard;
}



/** \brief
*  得到设备的兼容性信息.
*/
void osc_d3dManager::get_deviceCaps( void )
{
	HRESULT       t_hr;


	t_hr = m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
	osassert( !FAILED( t_hr ) );

	//
	// FogTable.
	if( m_d3dCaps.RasterCaps&D3DPRASTERCAPS_FOGTABLE )
		m_bUseFogTable = true;
	else
		m_bUseFogTable = false;


	// Get the max clip plane number.
	m_iMaxClipPlane = m_d3dCaps.MaxUserClipPlanes;

	// Store render target surface desc
	LPDIRECT3DSURFACE9 pBackBuffer = NULL;
	m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
	pBackBuffer->Release();


	//
	// 是否支持dxT2纹理压缩,如果硬件不支持,全局不能使用纹理压缩.
    D3DDeviceInfo* pDeviceInfo = m_d3dSettings.PDeviceInfo();
	int t_m = pDeviceInfo->pDeviceComboList->Count();
	m_bSupportDXT2 = true;
	for( int t_i=0;t_i<t_m;t_i ++ )
	{
		D3DDeviceCombo*  t_ptrCom = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr( t_i );
		
		t_hr = m_pD3D->CheckDeviceFormat( 
			D3DADAPTER_DEFAULT,pDeviceInfo->DevType,
			t_ptrCom->AdapterFormat,
			0,D3DRTYPE_TEXTURE,D3DFORMAT( MAKEFOURCC('D', 'X', 'T', '2') ) );

		if( FAILED( t_hr ) )
			m_bSupportDXT2 = false;

		t_hr = m_pD3D->CheckDeviceFormat( 
			D3DADAPTER_DEFAULT,pDeviceInfo->DevType,
			t_ptrCom->AdapterFormat,
			0,D3DRTYPE_TEXTURE,D3DFMT_A4R4G4B4  );
		if( FAILED( t_hr ) )
			g_bUse16Color = FALSE;

	}
	if( !m_bSupportDXT2 )
		g_bUseDXTC = FALSE;

	//! 记录设备的vertex shader和pixel shader版本
	g_iMajorVSVersion = D3DSHADER_VERSION_MAJOR( m_d3dCaps.VertexShaderVersion );
	g_iMinorVSVersion = D3DSHADER_VERSION_MINOR( m_d3dCaps.VertexShaderVersion );

	g_iMajorPSVersion = D3DSHADER_VERSION_MAJOR( m_d3dCaps.PixelShaderVersion );
	g_iMinorPSVersion = D3DSHADER_VERSION_MINOR( m_d3dCaps.PixelShaderVersion );

	//
	// 检测是否支持None Filter.
	osDebugOut( "Max stage is:<%d>..,Max texture is:<%d>...\n",
		m_d3dCaps.MaxTextureBlendStages,m_d3dCaps.MaxSimultaneousTextures );

	return;

}





/**********************************************************************/


/**********************************************************************/
//
// Use following func to display string of Engine.
//
/**********************************************************************/
/** \brief
*  Draw text on the screen.  
*
*  这个函数是引擎显示文字信息的主要的函数,必须支持Unicode.
*/
int osc_d3dManager::disp_string( os_stringDisp* _dstr,int _dnum,int _fontType /*= EFT_DefaultType*/,bool _useZ/* = false*/, bool _bShadow, RECT* _clipRect )
{
	guard;

	//osassert( false );
#if USE_PIXELFONT
	if( _useZ )
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_TRUE );
	else
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );

	if( _fontType==0 )
	{
		for( int t_i=0;t_i<_dnum;t_i++ ) 
		{
			g_song12x12.ShowText( m_pd3dDevice, _dstr[t_i].x, _dstr[t_i].y, _dstr[t_i].z, 0xffffff, _dstr[t_i].color, _bShadow, ((_dstr->color)&0xff000000), _clipRect, (char*)_dstr[t_i].get_sdStr() );
		}
	}
	else
	{
		for( int t_i=0;t_i<_dnum;t_i++ ) 
		{
			g_song14x14.ShowText( m_pd3dDevice, _dstr[t_i].x, _dstr[t_i].y, _dstr[t_i].z, 0xffffff, _dstr[t_i].color, _bShadow, ((_dstr->color)&0xff000000), _clipRect, (char*)_dstr[t_i].get_sdStr() );
		}
	}
#else

	int         t_i;
	CWFont* pFont = gFontManager.GetFont(_fontType);
	osassert(pFont);
	if( _useZ )
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_TRUE );
	else
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );


	//! 渲染字符串的背景。
	for( t_i=0;t_i<_dnum;t_i ++ )
	{
		if( _dstr[t_i].m_dwBackColor > 0 )
		{
			pFont->begin_bgRender();
			break;
		}
	}
	for( t_i=0;t_i<_dnum;t_i++ )
	{
		// 
		// 空字符串不需要渲染.
		if( (!_dstr[t_i].get_sdStr()[0])||(_dstr[t_i].m_dwBackColor==0) )
			continue;

		pFont->drawTextBG( (char*)_dstr[t_i].get_sdStr(),_dstr[t_i].x,
			_dstr[t_i].y,_dstr[t_i].z,_dstr[t_i].m_dwBackColor,1.0f );
	}

	pFont->Begin();

	for( t_i=0;t_i<_dnum;t_i++ )
	{
		// 
		// 空字符串不需要渲染.
		if( !_dstr[t_i].get_sdStr()[0] )
			continue;

		//t_f = float()/float(DEVICE_CHAR_WIDTH);
		if( !pFont->DrawText( (char*)_dstr[t_i].get_sdStr(),
			 _dstr[t_i].x,_dstr[t_i].x,_dstr[t_i].y,_dstr[t_i].z,_dstr[t_i].color,_dstr[t_i].charHeight, _bShadow, 0xb465523F, _clipRect  ) )
			 return -1;
 
	}
	pFont->End();

#endif

	return _dnum;

	unguard;
}

/** \brief
 *  立即在屏幕上显示字串，参数跟disp_string参数一致。
 *
 *  \param _cursor 需要立即显示的光标数据结构
 */
int osc_d3dManager::disp_stringImm( os_stringDisp* _dstr,int _fontType /*= EFT_DefaultType*/,
								   os_screenPicDisp* _cursor )
{
	guard;

	//! 如果当前使用了全屏抗锯齿，则不能使用立即显示文字的功能
	if( g_bUseFSAA )
		return 0;

	CWFont* pFont = gFontManager.GetFont(_fontType);
	osassert(pFont);
	
	// 如果处于渲染状态，不能立即显示文字.
	if( m_bInRender )
	{
		osassertex( false,"不能在Begin和endScene中立即显示文字" );
		return -1;
	}
	osassert( _dstr );

	// 空串直接返回。
	if( _dstr->get_sdStr()[0] == NULL )
		return 0;

	// 此函数内部使用Bengin,endScene和Present.
	d3d_begindraw();
	
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );

	//! 渲染字符串的背景。
	if( _dstr->m_dwBackColor > 0 )
		pFont->begin_bgRender();

	pFont->drawTextBG( (char*)_dstr->get_sdStr(),_dstr->x,
		_dstr->y,_dstr->z,_dstr->m_dwBackColor,1.0f );


	pFont->Begin();

	//t_f = float()/float(DEVICE_CHAR_WIDTH);
	if( !pFont->DrawText( (char*)_dstr->get_sdStr(),_dstr->x,_dstr->x,
		_dstr->y,_dstr->z,_dstr->color,(float)_dstr->charHeight, false  ) )
		return -1;
 
	pFont->End();

	// 显示光标的相关数据
	if( _cursor )
		disp_inscreen( _cursor,1,1 );

	d3d_enddraw();

	d3d_present();


	return 1;

	unguard;
}



/** \brief
*  对当前的屏幕进行截图.
*  使用日期加当前的时间做为截图的名字.
*/
char* osc_d3dManager::catch_screen( void )
{
	guard;// osc_d3dManager::catch_screen() );

	static char        t_str[256];
	SYSTEMTIME   t_time;
	HRESULT      t_hr;

	LPDIRECT3DSURFACE9  t_backBuf;

	GetSystemTime( &t_time );
	sprintf( t_str,"screenshot\\pic%d_%d_%d_%d_%d_%d.bmp",
		t_time.wYear,t_time.wMonth,t_time.wDay,t_time.wHour,
		t_time.wMinute,t_time.wMilliseconds );

	m_pd3dDevice->GetBackBuffer( 0,0,D3DBACKBUFFER_TYPE_MONO,&t_backBuf );


	// 文件夹不存在，则创建文件夹
	if( !PathIsDirectory( "screenshot" ) )
		CreateDirectory( "screenshot", 0 );

	t_hr = osSaveSurfaceToFile( t_str,D3DXIFF_BMP,t_backBuf,NULL,NULL );
	if( FAILED( t_hr ) )
		MessageBox( NULL,"请在程序运行目录下创建screenShot目录","注意",MB_OK );

	t_backBuf->Release();

	return t_str;

	unguard;
}

HWND  osc_d3dManager::get_hWnd()
{
	return m_hWnd;
}

//! 得到主表面.
bool osc_d3dManager::get_mainRenderTarget( LPDIRECT3DSURFACE9& _target )
{
	guard;

	if( !m_ptrSwapChain )
		return false;

	HRESULT   t_hr;
	if( FAILED( t_hr = m_ptrSwapChain->GetBackBuffer( 0,
		D3DBACKBUFFER_TYPE_MONO,&_target ) ) )
	{
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		return false;
	}

	return true;

	unguard;
}

//! 得到显卡的信息
const char* osc_d3dManager::get_displayCardInfo(void)const
{
	guard;

	return m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description;

	unguard;
}

void osc_d3dManager::d3d_clear(  DWORD _Count,
							   CONST D3DRECT* _pRects,
							   DWORD _Flags,
							   D3DCOLOR _Color,
							   float _Z,
							   DWORD _Stencil )
{
	guard;

	HRESULT  t_hr;
	osassert( this->m_ptrSwapChain );

	LPDIRECT3DSURFACE9 t_pBack=NULL;

	if( FAILED( t_hr = m_ptrSwapChain->GetBackBuffer( 0,
		D3DBACKBUFFER_TYPE_MONO,&t_pBack ) ) )
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );

	if( FAILED( t_hr = m_pd3dDevice->SetRenderTarget( 0,t_pBack ) ) )
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );

	if( FAILED(  t_hr = m_pd3dDevice->Clear( _Count,_pRects,_Flags,
		            _Color,_Z,_Stencil ) ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
	}
	SAFE_RELEASE( t_pBack );
	m_bInRender = true;

	unguard;

}

void osc_d3dManager::d3d_begindraw( void )
{

	osassert( !m_bInDrawScene );
	m_bInDrawScene = true;

	// 动画光标相关的数据
	if( m_iActiveCursorId >= 0 )
	{
		if( m_vecCursor.validate_id( m_iActiveCursorId ) )
		{
			m_vecCursor[m_iActiveCursorId].frame_setCursor( 
				sg_timer::Instance()->get_lastelatime(),this->m_pd3dDevice );
		}
	}

	HRESULT t_hr = m_pd3dDevice->BeginScene();
	if( FAILED( t_hr ) ){
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );

	}

	if(m_pEventQuery){

		m_pEventQuery->Issue(D3DISSUE_BEGIN);
	}
}


//! 全屏幕的fade.
void osc_d3dManager::start_fullscrFade( 
	DWORD _scolor,DWORD _ecolor,float _fadetime )
{
	osassert( _fadetime > 0 );

	m_bFullScrFadeState = true;
	m_fFadeTime = _fadetime;
	m_dwStartColor = _scolor;
	m_dwEndColor = _ecolor;
	m_fElaFadeTime = 0.0f;

}

