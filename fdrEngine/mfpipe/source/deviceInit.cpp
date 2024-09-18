//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: deviceInit.cpp
 *  Desc:     ����3d Device�ĳ�ʼ��,�õ�һ�����Ա�Ӧ�ó���ʹ�õ�3d device.
 *  His:      River created @ 4/14 2003
 *
 * �������õ����£�Ҳ�����������¡���
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


// ���Ҫ�����Ļˢ����.
# define MIN_FRASHRATE    60


//! presentʱ�Ľṹ  // D3DPRESENT_DONOTWAIT //
# define PRESENT_INTERVAL     D3DPRESENT_INTERVAL_ONE 

//
// ���ܷ���,ʹ��nvperfhud,�Ժ����е�������Ȩ��ʹ������������������������
#define NVPERFHUD_ANALYSIS  0

//! 
//! ȫ���õ�����Ⱦ��ʽ:
OSENGINE_API ose_VertexProcessingType   g_vertexProType = SOFTWARE_VP;

//! �Ƿ�ʹ��ȫ�������,ʹ��fsaaʱ����˻���������ʹ��discard�ķ�ʽ
OSENGINE_API BOOL                   g_bUseFSAA = false;

//! ��¼���豸��shader�汾��.
OSENGINE_API DWORD                  g_iMajorVSVersion = 1;
OSENGINE_API DWORD                  g_iMinorVSVersion = 1; 

OSENGINE_API DWORD                  g_iMajorPSVersion = 1;
OSENGINE_API DWORD                  g_iMinorPSVersion = 4; 

# define NOT_FOUND_DEVICE      "�����豸ʧ��"
# define NOT_FOUND_DIVICE_TIPS "������������������ٴγ��ֵ�ǰ�Ի���˵�������Կ��汾���ϣ���֧�ֵ�ǰ����Ϸ���������Կ������Ի���ϵ�ͷ���Ա"


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
			// TEST CODE:����ȷ���ϲ�û��ɾ����Ļ����
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
*  ��ʼ�����еı���.
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
*  �õ���ǰ��d3d��ʽ��16λ����32λ.
*
*  �Ӳ�ͬ��d3d��ʽ�еõ������ʾģʽ��16λ����32λ.
*  \param _format Ҫ���ĸ�ʽ.
*  \return int ����16����32,����Ƿ���0,����.
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
*  �ҵ�һ��������Ҫ���device.
*
*  ���Ѿ��������豸�б����ҵ�һ����������Ҫ���Device,���ڲ�Ҫ�����,����SampleType
*  ����framerate,���ǲ���Ҫ��Ѱ��Init deviceʱ����.����ҵ��豸,
*  ������m_d3dSettings.
* 
*  \param _initdata   ҪѰ���豸��ָ������.
*  \return bool ����ҵ��豸����ڲ�����m_d3dSettings,������.
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

	//  ���泣�õ���Ļ����.
	if( _initdata->m_bFullScr )
		m_iScrBpp    = _initdata->m_dwColorBits;
	m_iScrWidth  = _initdata->m_dwScrWidth;
	m_iScrHeight = _initdata->m_dwScrHeight;

	//  �����Ҫ���Ǵ��ڵ��豸,���ú���find_windowedDevice...
	if( !_initdata->m_bFullScr )
	{
		return find_windowedDevice( _initdata );
	}


	for( UINT iai = _time; iai < this->m_Enumeration.m_pAdapterInfoList->Count(); iai++ )
	{

		// ���ÿһ��Adapter��ÿһ��device.�ȵõ�Adapter's Information.
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_Enumeration.m_pAdapterInfoList->GetPtr(iai);
        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
		{

			// ���ÿһ���豸,�ȵõ��豸��Ϣ.
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);

			// ��ʵ��Ӧ�ó����в���Ҫref ��Ⱦ��.
			if( (pDeviceInfo->DevType == D3DDEVTYPE_REF)&&(!g_bShaderDebug) )
				continue;

			// ���ÿһ��device combo ������,�ҵ����ŵ�device combo. 
            for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
			{
				// �ȵõ�device combo������.
                D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);

				if( pDeviceCombo->IsWindowed )
					continue;

				// �ҵ���Ҫ��ɫ�����.
				if( (formatlength( pDeviceCombo->BackBufferFormat ) == _initdata->m_dwColorBits)&&
					(formatlength( pDeviceCombo->AdapterFormat ) == _initdata->m_dwColorBits ) )
				{
                    pBestAdapterInfo = pAdapterInfo;
                    pBestDeviceInfo = pDeviceInfo;
                    pBestDeviceCombo = pDeviceCombo;

					goto EndFullscrDeviceComboSearch;
				} 
			} // ��ÿһ��device combo�ļ��.
		} // �豸���. 
	} // Adapter���.


EndFullscrDeviceComboSearch:

    // ������˻�û���ҵ������õ�device combo,���ش���.
	if (pBestDeviceCombo == NULL)
	{
		osassert( false );
        return false;
	}

    bestDisplayMode.Width = 0;
    bestDisplayMode.Height = 0;
    bestDisplayMode.Format = D3DFMT_UNKNOWN;
    bestDisplayMode.RefreshRate = 0;


	// ������Ҫ����ʾģʽ.
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

	// ����Ҳ�����Ҫ����ʾģʽ,����false.
	if( bestDisplayMode.Width == 0 )
	{
		osassert( false );
		return false;
	}

	//  ��д�̶���3d Setting ��Ϣ.
	m_d3dSettings.IsWindowed = false;
    m_d3dSettings.Fullscreen_MultisampleQuality = 0;
	m_d3dSettings.Fullscreen_PresentInterval = PRESENT_INTERVAL;

	//  ���������Ĳ���.
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
*  find_initdevice�����İ�������.
*
*  ���ҪѰ�ҵ��豸�Ǵ����豸,find_initdevice�����͵������������Ѱ���ʺϵĴ���
*  �豸.
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
	

	// ���ÿһ��Adapter.
    for( UINT iai = 0; iai < m_Enumeration.m_pAdapterInfoList->Count(); iai++ )
    {
		// ��ÿһ��Adapter,���ÿһ���豸.
        D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_Enumeration.m_pAdapterInfoList->GetPtr(iai);

        for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
        {
            D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
            
			// ���ǲ�ʹ��Ref���豸,�����Ref���͵��豸,pass.
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
		osDebugOut( "û���ҵ����õ�3D�豸......\n" );
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
*  �ڴ��ڷ����仯ʱ�������ڵ�style,�����ȫ���䵽������Ҫ��������style.
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
		
		// �����ȫ��ģʽ�������ô���˵�Ϊ��
		::SetMenu( m_hWnd,NULL );
    }


	unguard;
}



/** \brief 
*  Fill the D3DPRESENT_PARAMETERS struct.
*
*  ʹ������׼���õ�CD3DSettings�ṹȥ��䴴��3D�豸�����D3DPRESENT_PARAMETERS
*  �ṹ.Ҫ�����ı���Ϊ:m_d3dpp
*/
void osc_d3dManager::BuildPresentParamsFromSettings( void )
{
	guard;


	// ATTENTION TO FIX:
	// ��������: 
    m_d3dpp.Windowed               = m_d3dSettings.IsWindowed;
    m_d3dpp.BackBufferCount        = 2;

	//! ʹ��ȫ������ݵĻ���ǿ��ʹ�� 2 sample��sample��ʽ
	if( g_bUseFSAA )
		m_d3dpp.MultiSampleType    = D3DMULTISAMPLE_2_SAMPLES;
	else
		m_d3dpp.MultiSampleType        = m_d3dSettings.MultisampleType();
    m_d3dpp.MultiSampleQuality     = m_d3dSettings.MultisampleQuality();

	// ATTENTION : 
	// Ϊ��ʹ���ϴε���Ⱦ���ݣ�����ʹ��flip����copy������Discard
	// ��ȥ������BackBuf���ݡ���d3d���ĵ���������Ǵ���ģʽ����flip��ռ�ñȽ϶��
	// ���⴦��,copy��fullscreen�»�ռ����Դ��������Ҫʹ�ü�ȫ��������ʹ��copy.
	if( g_bUseFSAA )
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	else
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;


    m_d3dpp.EnableAutoDepthStencil = m_Enumeration.AppUsesDepthBuffer;
    m_d3dpp.hDeviceWindow          = m_hWnd;
    if( m_Enumeration.AppUsesDepthBuffer )
    {
		// ʹ��ȫ������ݵĻ�����������������
		if( g_bUseFSAA )
		{
			// ���ʹ��ȫ�������
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
*  ����3d��Ⱦ����.
*  
*  ���Ѿ�����CD3DSettings�ṹ�еõ�������Ҫ����3D device��Ҫ
*  �ĸ��ֲ���,����3d Device.
*
*  ��������:
*
*  1: ����� D3DPRESENT_PARAMETERS �ṹ.
* 
*  2: ����3d device.
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
	//  ��Ҫʱ����ı䴰�ڵĺ���.
    //  Prepare window for possible windowed/fullscreen change
    //  AdjustWindowForChange();
	//
	AdjustWindowForChange();

	//
    // Set up the presentation parameters
	//
    BuildPresentParamsFromSettings();

	//
	// �������ʹ�õ�device,���ش���.
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
		osDebugOut( "\nD3D������ѡ����������㴦��\n" );
		g_vertexProType = SOFTWARE_VP;
	}
    else if (m_d3dSettings.GetVertexProcessingType() == MIXED_VP)
	{
		behaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
		osDebugOut( "\nD3D������ѡ���϶��㴦��\n" );
		g_vertexProType = MIXED_VP;
	}
    else if (m_d3dSettings.GetVertexProcessingType() == HARDWARE_VP)
	{
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		osDebugOut( "\nD3D������ѡ��Ӳ�����㴦��\n" );
		g_vertexProType = HARDWARE_VP;
	}
    else if (m_d3dSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
	{
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		osDebugOut( "\nD3D������ѡ��Ӳ�����㴦��\n" );
		g_vertexProType = PURE_HARDWARE_VP;
	}
    else
	{
        behaviorFlags = 0; // TODO: throw exception
		osassert( false );
	}

	// 
	// ʹd3d���Զ��̰߳�ȫ�Ĺ�����
	behaviorFlags |= D3DCREATE_MULTITHREADED;
	

	//
	// Log ���Ǵ������豸����.

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

		// River @ 2010-4-22:ǿ��ʹ��query,ȥ���࿪ʱ�����Ļ�������
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
			// ��ĳЩ��������������豸ʱ�����ش��ڣ����ܻ��"Driver internal Error"�Ĵ���
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          SWP_SHOWWINDOW/*SWP_HIDEWINDOW*/ );
        }

       m_dwCreateFlags = behaviorFlags;
  
		//
		// �õ��豸��caps.
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
		// �Ѿ��ɹ������豸,���ҳɹ��������豸����Ҫ����������,
		// ����true.
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
		sprintf( d3dpp, "\nERROR�豸����ʧ��!\n����ԭ��:%s,backBufferWidth:%d, backBufferHeight:%d, format:%d, count:%d\nMultiSampleType:%d, MultiSampleQuality:%d\nSwapEffect:%d, Windowed:%d, EnableAutoDepthStencil:%d, AutoDepthStencilFormat:%d\nFlags:%d, RefreshRate:%d, PresentationInterval:%d\n",
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
		//MessageBox( NULL, buff, "ERROR�豸����ʧ��!", MB_OK );
	}

	// River mod @ 2008-5-23:�˴�����ViewPort.
	D3DVIEWPORT9 t_resetVp;
	t_resetVp.X      = 0;
	t_resetVp.Y      = 0;
	t_resetVp.Width  = m_d3dpp.BackBufferWidth;
	t_resetVp.Height = m_d3dpp.BackBufferHeight;
	t_resetVp.MinZ   = 0.0f;
	t_resetVp.MaxZ   = 1.0f;
	m_pd3dDevice->SetViewport( &t_resetVp );

	// ��������豸ʧ��,����Ҫ����Ref device,ֱ�ӷ��ش���.
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

		g_yyDemo.m_pfnLoadInGame("LY_LMQT|YY3D9|NONE");//���������Ҫ��YY������Ա����ȡ��

		const char* szYYFilePath = "Bin\\YY3804.zip";

		//����ǻ��л���Ŀ��ɾ�����е�ע��
		//g_yyDemo.m_pfnRunService( szYYFilePath );

		//�����OpenId��Ŀ��ɾ�����е�ע��
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
		{   //���д��������ã������޷���ʾ
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

//! ��������d3dDevice�Ŀ�ȣ��߶�,�����ϲ������Ϸ�ķֱ���
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

	// River mod @ 2008-5-23:�˴�����ViewPort.
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
*  Reset 3d ��Ⱦ����.
*
*  ��������3d ��Ⱦ����,����Ӵ���ģʽ��ȫ��ģʽ,����ȫ��ģʽ��ͬMode
*  ���л�.
*
*/
bool osc_d3dManager::reset_3denv( bool _forceReset )
{
	guard;

	HRESULT     t_hr;

	//! River @ 2009-11-4:����Reset�豸���ٽ���
	::EnterCriticalSection( &g_sShadowSec );

	// 
	// ���ͷ����е�user created state blocks���м�ܵ�����Դ
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

	// �ڲ���RenderTarget.
	for( int t_i=0;t_i<(int)m_vecRenderTarget.size();t_i ++ )
	{
		// River @ 2007-2-10:���ͷ�����Ļ�����ڵ�������Դ����
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

	//! River @ 2010-3-2:�ͷ�add swap chain.
	SAFE_RELEASE( m_ptrAddSwapChain );

	// Reset the 3d env.
	t_hr = m_pd3dDevice->TestCooperativeLevel();
	if( !_forceReset )
	{
		while( t_hr != D3DERR_DEVICENOTRESET )
		{
			// ���Դ�����������˳������ϲ��ִ��
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
		//! River mod @ 2011-4-2:��������豸���ٿ��Ƿ��ܹ��ɹ�.
		if( FAILED( t_hr ) )
		{
			int   t_iTimes = 0;
			while( t_iTimes < 5 )
			{
				t_hr = m_pd3dDevice->Reset( &m_d3dpp );
				if( FAILED( t_hr ) )
				{
					// �Ժ�����.
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
				va( "������Ⱦ�豸ʧ��,ʧ�ܴ���<D3DERR_OUTOFVIDEOMEMORY>" ) );
			break;
		case D3DERR_DEVICELOST:
			osassertex( false,
				va( "������Ⱦ�豸ʧ��,ʧ�ܴ���<D3DERR_DEVICELOST>" ) );
			break;
		case D3DERR_DRIVERINTERNALERROR:
			osassertex( false,
				va( "������Ⱦ�豸ʧ��,ʧ�ܴ���<D3DERR_DRIVERINTERNALERROR>" ) );
			break;
		case D3D_OK:
			break;
		default:
			osassertex( false,
				va( "��Ԥ֪����<%s>...\n",osn_mathFunc::get_errorStr( t_hr ) ) );
			break;
		}

		//! River @ 2009-11-4:�˳�Reset�豸���ٽ���
		::LeaveCriticalSection( &g_sShadowSec );

		if( FAILED( t_hr ) )
			return false;
	}
	//! River @ 2009-11-4:�˳�Reset�豸���ٽ���
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

		osassertex( false,va("�õ���˱���ʧ��<%s>..\n",osn_mathFunc::get_errorStr( t_hr ) ) );
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

		// �����豸�õ��Ĺ��
		set_cursor( m_iActiveCursorId );
	}



	// 
	// reset or recreat ���е� user created state blocks���м�ܵ�����Դ
	
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
	
	// River added @ 2007-4-2:�ر�������Ҫ�������
	MapTile::m_ptrMapTerrain->onRestoreMapTerrain();


	// ���¼���Queryϵͳ��
	if(t_d3dQuery){
		CreateD3DQuery();
	}

	// �ڲ���RenderTarget.
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

	// River @ 2010-3-2:���´���add swap chain.
	if( m_sAddSwapPara.BackBufferWidth > 0 )
	{
		t_hr = m_pd3dDevice->CreateAdditionalSwapChain( 
			&m_sAddSwapPara,&m_ptrAddSwapChain );
		if( FAILED( t_hr ) )
		{
			// River @ 2011-2-16:�����豸�ٴζ�ʧ��bug.
			if( D3DERR_DEVICELOST == t_hr )
				return false;
			osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
		}
	}


	// �ɹ�Reset 3d��Ⱦ����.
	return true;


	unguard;
}


//! ����һЩ�����Կ�������,ʹ���֪��Ϊʲô�������Ե.
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


//! ����Ǵ���ģʽ�������豸ǰ���ȵ������ڵĴ�С��λ�á�
void osc_d3dManager::adjust_windowSize( HWND _hwnd,int _width,int _height,bool _windowed)
{
	guard;

	BOOL t_seriousProcess = FALSE;

adjust_windowSize_flag:

	DWORD        t_winStyle;

	// �����ͻ�����С��������Ҫ�Ĵ�С.
	RECT   t_rect;
	t_rect.left = 0,t_rect.top = 0;
	t_rect.right = _width;
	t_rect.bottom = _height;
	

	t_winStyle = GetWindowLong( _hwnd,GWL_STYLE );

	// ������Ӵ��ڣ�SetWindowPos �����õĴ�С�� GetClientRect �ͻ��в��ȵ�
	// ���ڼ���ʹ��fdrEngine �������ı༭����ʹ�䲻������ʾ���ڡ�
	//
	const BOOL t_isChlidWnd = t_winStyle & WS_CHILDWINDOW;

	// ���ݵ�ǰ�����Ƿ���menu���д���.
	if( ::GetMenu( _hwnd ) )
		::AdjustWindowRectEx( &t_rect,t_winStyle,true,NULL );
	else
		::AdjustWindowRectEx( &t_rect,t_winStyle,false,NULL );

	// River added @ 2009-2-26:ʹ�����Ĵ���λ����Ļ�����м�
	int t_xOffset = _windowed?(GetSystemMetrics( SM_CXFULLSCREEN ) - (t_rect.right - t_rect.left))>> 1 : 0;
	int t_yOffset = _windowed?(GetSystemMetrics( SM_CYFULLSCREEN ) - (t_rect.bottom - t_rect.top))>> 1 : 0;
	if(t_yOffset < 0) t_yOffset = 0;
	if(t_xOffset < 0) t_xOffset = 0;
	

	// River @ 2006-4-1:ATTENTION TO FIX:
	// ��ĳЩ��������������豸ʱ�����ش��ڣ����ܻ��"Driver internal Error"�Ĵ���
	BOOL t_bSet = SetWindowPos( 
		m_hWnd,(t_seriousProcess?HWND_BOTTOM:HWND_TOP),t_xOffset,t_yOffset,t_rect.right-t_rect.left,
		t_rect.bottom-t_rect.top, SWP_SHOWWINDOW /*SWP_HIDEWINDOW*/ );

	::GetWindowRect( m_hWnd,&m_rcWindowBounds );
	::GetClientRect( m_hWnd,&m_rcClientBounds );

	if( (m_rcClientBounds.right - m_rcClientBounds.left != _width
		|| m_rcClientBounds.bottom - m_rcClientBounds.top != _height) && !t_isChlidWnd){

		// �����Ļ�ķֱ��ʱȽ�С���б������Ĵ��ھͻ���ֱַ��ʸ߿������õ����
		// ���Ǿ�Ҫȥ����������
		//
		if(!t_seriousProcess){

			//::MessageBoxA(m_hWnd,"���ĵ��Էֱ治�㣬ֻ���Էǳ�ģʽ������Ϸ����֤��Ϸ�������С�","������ʾ��",MB_ICONERROR);

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
			// ȥ���������Ĵ��ڣ������ܴﵽҪ�󣬾�ֻ�ܽ����ˡ�����
			//
			::MessageBoxA(m_hWnd,"�������ĵ�����Ȼ�޷�������Ϸ�������������ԡ�","����������ʾ��",MB_ICONERROR);
		}
	}

	// River added @ 2007-4-2:�����µ�dx��,�����˻�������С���ӿڲ�һ��,��
	// debug�汾��dx�ϻ��������
	// River mod 2008-4-13:ʹ��windows��client Bounds.
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


//! �ͷŵ�ǰ�豸��additional swap chain.
void osc_d3dManager::release_addSwapChain( void )
{
	guard;

	if( m_ptrAddSwapChain == NULL )
		return;

	SAFE_RELEASE( m_ptrAddSwapChain );

	unguard;
}


//! ���������swapChain,���ڶര����Ⱦ
bool osc_d3dManager::create_addSwapChain( int _width,int _height,HWND _hwnd )
{
	guard;

	if(_hwnd == NULL){
		// �ͷ� ������
		release_addSwapChain();
		m_sAddSwapPara.BackBufferHeight = 0;
		m_sAddSwapPara.BackBufferWidth = 0;
		return true;
	}

	if( (_width<=0)||(_height<=0) )
		return true;

	osassert( this->m_pd3dDevice );

	if(m_ptrAddSwapChain){
		// ���ͷ�ԭ���Ľ�����
		release_addSwapChain();
	}

	D3DPRESENT_PARAMETERS   t_para;
	HRESULT                 t_hr;

	//! �����ȫһ�£�ֱ�ӷ���
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
		// River @ 2011-10-18�����º͵Ĵ���
		return false;
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );
	}

	memcpy( &m_sAddSwapPara,&t_para,sizeof( D3DPRESENT_PARAMETERS ) );

	return true;

	unguard;
}



/** \brief 
*  ����������Ҫ��3d Device.
*
*  ���������I_deviceManager�еĽӿ�,ʵ�ֲ���:
*  
*  1: ��direct3d object.
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
	// ��������ʼ�����ǵ�3d��ʾ�豸.
	//
	this->m_hWnd = _hwnd;
	this->cfm_deviceptr = (CFMPTR*)_cfm_ptr;

	if( !_initdata->m_bFullScr )
		adjust_windowSize( _hwnd,_initdata->m_dwScrWidth,_initdata->m_dwScrHeight ,true);
	else
	{
		// ATTENTION TO FIX: ǿ��ʹ���ϲ�ָ���Ĵ�С����?
		m_rcWindowBounds.left = 0;
		m_rcWindowBounds.right = _initdata->m_dwScrWidth;
		m_rcWindowBounds.bottom = _initdata->m_dwScrHeight;
		m_rcWindowBounds.top = 0;

	}


	// ����ʹ��d3d��Ӳ�����.
	m_bShowCursorWhenFullscreen = _initdata->m_bHwCursor;

	//
	// ���һ��ʼ�����Ǵ���ģʽ,������Ҫ���洰�ڵ�style.
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
	// 1: ����direct3d object.
	//
    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	
    if( m_pD3D == NULL )
	{
		osassertex(false,"����d3d9ʧ��");
		//MessageBox( NULL, "����d3d9ʧ��","",MB_OK );
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
	// 3: Find the best device we need .ʹ��һ�������ͱ�׼���㷨��
	//
	memcpy( &t_init,_initdata,sizeof( os_deviceinit ) );
	t_init.m_dwColorBits = 32;
	t_init.m_dwDepthBits = 24;
	t_init.m_dwStencilBits = 8;
	m_iStencilBitsNum = 8;
	if( _initdata->m_dwColorBits == 16 )
		g_bUse16Color = TRUE;


	// ��_initdata�м���������device������.........
	if( _initdata->m_dwDepthBits>0 )
	{
		m_Enumeration.AppUsesDepthBuffer = true;
		m_Enumeration.AppMinDepthBits = _initdata->m_dwDepthBits;
	}
	m_Enumeration.AppMinStencilBits = m_iStencilBitsNum;
	if( FAILED( hr = this->m_Enumeration.Enumerate() ) )
	{
		MessageBox( NULL, "ö���豸ʧ��","",MB_OK );
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
			MessageBox( NULL, "�����豸ʧ��","",MB_OK );
			SAFE_RELEASE( m_pD3D );
			osassert( FALSE );
			return false;
		}

		// ���ҵڶ����Կ�����
		if( !find_initdevice( &t_init,1 ) )
		{
			t_init.m_dwStencilBits = 0;
			m_iStencilBitsNum = 0;

			m_Enumeration.AppMinStencilBits = m_iStencilBitsNum;
			if( FAILED( hr = this->m_Enumeration.Enumerate() ) )
			{
				MessageBox( NULL, "2��ö���豸ʧ��","",MB_OK );
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
		osDebugOut_tag("RenderDevice","ʹ��������Ⱦ�豸:<%s>",m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description);

		char t_desc[MAX_PATH];
		GetWindowsVerDesc(MAX_PATH,t_desc);
		osDebugOut_tag("System","����ϵͳ��Ϣ <%s>",t_desc);

		GetCpuInfoDesc(MAX_PATH,t_desc);
		osDebugOut_tag("CpuInfo","CPU ��Ϣ <%s>",t_desc);

		GetMemSizeInfoDesc(MAX_PATH,t_desc);
		osDebugOut_tag("MemInfo","�ڴ���Ϣ <%s>",t_desc);

		// �����Կ��ִ�,����Կ��Ƿ�Ϊ�����Կ�.
		if( is_onboardGraphicCard( 
			m_d3dSettings.pWindowed_AdapterInfo->AdapterIdentifier.Description ) )
		{
			char   t_sz[256];
			sprintf( t_sz,"�����Կ�<%s>�����弯���Կ����Ƿǳ����ϵ��Կ�,�������Կ���������Ϸ..",
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
		// River mod @ 2007-9-13:�������Ĭ�ϵ���Ļ��С,�򴴽�һ��Ĭ�ϴ�С����
		if(  (_initdata->m_dwScrWidth != 800) )
		{
			os_deviceinit   t_init;
			memcpy( &t_init,_initdata,sizeof( os_deviceinit ) );
			t_init.m_dwScrWidth = 800;
			t_init.m_dwScrHeight = 600;

			if( !init_device( _hwnd,&t_init,_cfm_ptr ) )
			{
				char  t_szErrorMsg[512];
				sprintf( t_szErrorMsg,"����3D�豸ʧ�ܣ������°�װ�Կ��������򲢼���Կ��Ƿ����..." );
				MessageBox( NULL,t_szErrorMsg,"ERROR",MB_OK );

				return false;
			}			
		}
		else
		{
			char  t_szErrorMsg[512];
			sprintf( t_szErrorMsg,"����3D�豸ʧ�ܣ������°�װ�Կ��������򲢼���Կ��Ƿ����..." );
			MessageBox( NULL,t_szErrorMsg,"ERROR",MB_OK );

			return false;
		}
		
	}
	

	// �����ʼ��
	init_miskRes();

	extern GFXCardProfiler* gCardProfiler ;
	gCardProfiler  = new GFXD3D9CardProfiler(m_pd3dDevice);
	gCardProfiler->init();
	// ���ɹ��Ĵ���3d Device,������.

	//! �ж��Կ��ȼ������ж����Ⱦ
	extern bool gUseTerrainPS;
	if (m_d3dCaps.PixelShaderVersion<D3DPS_VERSION(2,0)&&
		m_d3dCaps.MaxTextureBlendStages<6)
	{
		osDebugOut("\n����Mult-Pass��ʽ��Ⱦ�ر�\n");
		gUseTerrainPS = false;
		g_bWaterReflect = false;
	}
	else if (gUseTerrainPS)
	{
		osDebugOut("\n����Pixel Shader��ʽ��Ⱦ�ر�\n");
	}
	//! ����5200���Կ���ʹ�õ͵�ˮ��
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

	//! ���vertexshader&Ps�汾С��2,ʹ���޵�Ӱ��ˮ��
	if( (m_d3dCaps.PixelShaderVersion  < D3DPS_VERSION(2,0)) ||
		(m_d3dCaps.VertexShaderVersion < D3DVS_VERSION(2,0))   )
		::g_bWaterReflect = false;

	// River mod @ 2007-3-26: 
	hr = m_pd3dDevice->GetSwapChain( 0,&m_ptrSwapChain );
	if( FAILED( hr ) )
		osassertex( false,"�õ�SwapChain����..\n" );


	return true;

	unguard;
}


//! �Ƿ�֧���������ĵĶ����ͼ��ʽ.
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


//! ��ʼ��font������ݺ���������������,�ڳ�ʼ����ȫ�ֵ�StateBlockMgr����ô˺�����
void osc_d3dManager::init_miskRes( void )
{
	guard;

	// �ȳ�ʼ����Ⱦ״̬��Ĺ�������ʹ�����ĵط�����ʹ����Ⱦ״̬�顣
	os_stateBlockMgr::Instance()->init_sbMgr( m_pd3dDevice );

	
	//
	// �������ǵ�������Ϣ.
	char   t_str[] = { NULL,NULL,NULL,NULL };
	gFontManager.Initialize(m_pd3dDevice);
	//gFontManager.RegisterFont( "����", 32 );//����_GB2312 ����ϸ��һ_GBK

#if USE_PIXELFONT
	g_song12x12.OnCreateDevice( m_pd3dDevice );
	if( !g_song12x12.LoadFromFile( "ui\\gui\\fonts\\songti12x12.dat" ) )
		osassert( false );

	g_song14x14.OnCreateDevice( m_pd3dDevice );
	if( !g_song14x14.LoadFromFile( "ui\\gui\\fonts\\songti14x14.dat" ) )
		osassert( false );
#endif
	

	// 
	// ��ʼ�����֣�˳���¼һ�µ�ǰ��d3dMgr����Ҫ����Ⱦ״̬�顣
	record_scrTexRBlock();
	record_scrTexRBlock_rt();
	record_fullScrFadeRBlock();
	record_LensFlareRBlock();

	//
	// ��Ⱦ��Ļ�������Ҫ����Դ��ʼ��
	m_sPicRD.init_scrPicRenderData( m_pd3dDevice );


	unguard;
}



/** \brief 
*  �ͷ����Ǵ�����3d Device.
*
*  ���ͷ�3d Device֮ǰҪ�����е�3d��Դ���ͷ���.
*  
*/
void osc_d3dManager::release_device( void )
{
	guard;

	
	//
	// �ͷ����ǵ�fontMgr.
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



	// �˴�Ϊ������������������ֵ��
	osassertex( (D3D_OK == t_hr) || (D3DERR_DEVICELOST == t_hr),
		va( "����ķ���ֵ..%s <%d>",osn_mathFunc::get_errorStr( t_hr ),DWORD( t_hr ) )  );

	if( D3DERR_DEVICELOST == t_hr )
	{
		// 
		// River @ 2011-1-5:�����߳���Ⱦ������£��������豸���衣
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
*  ʹ��d3d��Ӳ�����.
*
*  ������������������ϢWM_MOUSEMOVE��Ϣ�е���,�����������ʹ��d3d��Ӳ�����.
*
*/
void osc_d3dManager::set_hwCursor( void )
{
	// ��ʼ����3d device����Ե����������.
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
	// �����ǰ�������������Ѿ��п���ʹ�õ�����,����id.
	for( t_i=0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( strcmp( m_vecTexture[t_i].m_szTexName,_filename ) == 0 )
		{
			m_vecTexture[t_i].add_ref();
			return t_i;
		}
	}

	//
	// �õ�һ�����õ�����洢��Ԫ.
	//
	for( t_i = 0;t_i<(int)m_vecTexture.size();t_i++ )
	{
		if( !m_vecTexture[t_i].inuse )
			break;
	}

	if( t_i == m_vecTexture.size() )
	{
		MessageBox( NULL,"û���㹻�Ŀռ䴴����ά��Ļ��ʾ����","Error",MB_OK );
		osassert( false );
	}

	//
	// ʹ�õõ�������洢��Ԫ������ʱҪ��ʾ������.
	// ��Щ����ʹ�õ���d3d's Managed Texture,���Բ���Ҫ��������ʧ������.
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
	// ���ص�����ʾ����ı�ʶID.
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
*  Ϊ��������Ҫ��ʾ�Ķ�άͼƬ��������
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
	// �����ǰ�������������Ѿ��п���ʹ�õ�����,����id.
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
	// �õ�һ�����õ�����洢��Ԫ.
	for( t_i = 0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( !m_vecSceneScrTexture[t_i].inuse )
			break;
	}

	if( t_i == m_vecSceneScrTexture.size() )
	{
//		MessageBox( NULL,"û���㹻�Ŀռ䴴����ά��Ļ��ʾ����","Error",MB_OK );
		osDebugOut( "û���㹻�Ŀռ䴴����ά��Ļ��ʾ����, maxsize:<%d>\n", t_i );
		osassert( false );
	}

	//osDebugOut( "����2d����:<%d><%s>\n", t_i, _filename ); 

	// 
	// �ȶ��������ڴ档
	int   t_iGBufIdx = -1;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int t_iSize = read_fileToBuf( (char*)_filename,t_ptrStart,TMP_BUFSIZE );
	osassertex( (t_iSize>0),(char*)_filename );

	//
	// �õ��������Ϣ,���Ƿ�������ǵ������ʽ.
	D3DXIMAGE_INFO   t_info;

	t_hr = D3DXGetImageInfoFromFileInMemory( t_ptrStart,t_iSize,&t_info );
	if( FAILED( t_hr ) )
	{
		MessageBox( NULL,_filename,"���ļ�����",MB_OK );
		osassertex( false,va( "���ļ�<%s>����...\n",_filename ) );
		return -1;
	}


# if __TEXTURE_SIZELIMIT__
	if( (t_info.Width>256)||(t_info.Height>256 ) )
	{
		MessageBox( NULL,"������256�����ƴ�С",_filename,MB_OK );
		osassert( false );
	}
# endif 


	//
	// ʹ�õõ�������洢��Ԫ������ʱҪ��ʾ������.
	// ��Щ����ʹ�õ���d3d's Managed Texture,���Բ���Ҫ��������ʧ������.
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
	// ���ص�����ʾ����ı�ʶID.
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

//! test ע��һ������,����id
int osc_d3dManager::register_texid( LPDIRECT3DTEXTURE9 _pTexture )
{
	osassert( _pTexture );
	int t_i = (int)m_vecSceneScrTexture.size();
	if( t_i==0 )
		m_vecSceneScrTexture.resize( MIN_TMPTEXTURE );
	// �õ�һ�����õ�����洢��Ԫ.
	int i;
	for( i = 0;i<(int)m_vecSceneScrTexture.size();i++ )
	{
		if( !m_vecSceneScrTexture[i].inuse )
			break;
	}
	if( i == m_vecSceneScrTexture.size() )
	{
		MessageBox( NULL,"û���㹻�Ŀռ䴴����ά��Ļ��ʾ����","Error",MB_OK );
		osassert( false );
	}
	m_vecSceneScrTexture[i].tex = _pTexture;

	// River added @ 2007-2-10:��Ϊ��renderTargetע�ᣬ���ͷ�ʱ����������Ļ����
	// ���ͷţ�Ҳ����target�б����ͷţ����ԼӴ�˴�renderTarget�����refֵ
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

//! ��˻�������renderTarget.
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

	// ATTENTION TO FIX: �˴����ɵ�m_pTargetSurface ��refCount����Ŀ����1,Ϊʲô???
	t_hr = pRenderTarget->GetSurfaceLevel( 0,&m_pTargetSurface );
	osassert( !FAILED( t_hr ) );


	if( NULL == m_pBackBufSurface )
		_dev->GetRenderTarget( 0, &m_pBackBufSurface );

	return;

	unguard;
}


//syq_mask ������ȾĿ��
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

//syq_mask �����ȾĿ��
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



//! ����һ������
int osc_d3dManager::create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture )
{
	int i;
	HRESULT hr;
	osassert( m_pd3dDevice );
	osassert( _pTexture );

	// �õ�һ�����õ�����洢��Ԫ.
	for( i = 0;i<(int)m_vecSceneScrTexture.size();i++ )
	{
		if( !m_vecSceneScrTexture[i].inuse )
			break;
	}
	if( i == m_vecSceneScrTexture.size() )
	{
		MessageBox( NULL,"û���㹻�Ŀռ䴴����ά��Ļ��ʾ����","Error",MB_OK );
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

//! �ͷų����еĶ�άͼƬ����
void osc_d3dManager::release_sceneScrTex( int _texid )
{
	guard;

	osassertex( (_texid>=0)&&(_texid<(int)m_vecSceneScrTexture.size()),
		va("The error tex is:<%d>..\n",_texid ) );

	m_vecSceneScrTexture[_texid].release_texture( _texid );

	unguard;
}

//! �ͷ����еĳ�����ʹ�ö�άͼƬ����
void osc_d3dManager::release_allSceScrTex( bool _realRse/* = true*/ )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecSceneScrTexture.size();t_i++ )
	{
		if( m_vecSceneScrTexture[t_i].inuse )
		{
			// 
			// ATTENTION OT OPP: �����ڴ˴����ͷ�����������һ�����������ٶȡ�
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

			// ����Ǽٵ��ͷţ���ֻ��Ҫ���õ�ǰ��inuseΪfalse.
			m_vecSceneScrTexture[t_i].inuse = false;
			m_vecSceneScrTexture[t_i].m_iRef = 0;
		}
	}

	//syq_mask release all render target...
	for( int i=0;i<(int)m_vecRenderTarget.size();++i )
	{
		//unregister_texid( m_vecRenderTarget.at(i).id
		// �ͷ��豸��ص�����
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
*  �õ��豸�ļ�������Ϣ.
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
	// �Ƿ�֧��dxT2����ѹ��,���Ӳ����֧��,ȫ�ֲ���ʹ������ѹ��.
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

	//! ��¼�豸��vertex shader��pixel shader�汾
	g_iMajorVSVersion = D3DSHADER_VERSION_MAJOR( m_d3dCaps.VertexShaderVersion );
	g_iMinorVSVersion = D3DSHADER_VERSION_MINOR( m_d3dCaps.VertexShaderVersion );

	g_iMajorPSVersion = D3DSHADER_VERSION_MAJOR( m_d3dCaps.PixelShaderVersion );
	g_iMinorPSVersion = D3DSHADER_VERSION_MINOR( m_d3dCaps.PixelShaderVersion );

	//
	// ����Ƿ�֧��None Filter.
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
*  ���������������ʾ������Ϣ����Ҫ�ĺ���,����֧��Unicode.
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


	//! ��Ⱦ�ַ����ı�����
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
		// ���ַ�������Ҫ��Ⱦ.
		if( (!_dstr[t_i].get_sdStr()[0])||(_dstr[t_i].m_dwBackColor==0) )
			continue;

		pFont->drawTextBG( (char*)_dstr[t_i].get_sdStr(),_dstr[t_i].x,
			_dstr[t_i].y,_dstr[t_i].z,_dstr[t_i].m_dwBackColor,1.0f );
	}

	pFont->Begin();

	for( t_i=0;t_i<_dnum;t_i++ )
	{
		// 
		// ���ַ�������Ҫ��Ⱦ.
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
 *  ��������Ļ����ʾ�ִ���������disp_string����һ�¡�
 *
 *  \param _cursor ��Ҫ������ʾ�Ĺ�����ݽṹ
 */
int osc_d3dManager::disp_stringImm( os_stringDisp* _dstr,int _fontType /*= EFT_DefaultType*/,
								   os_screenPicDisp* _cursor )
{
	guard;

	//! �����ǰʹ����ȫ������ݣ�����ʹ��������ʾ���ֵĹ���
	if( g_bUseFSAA )
		return 0;

	CWFont* pFont = gFontManager.GetFont(_fontType);
	osassert(pFont);
	
	// ���������Ⱦ״̬������������ʾ����.
	if( m_bInRender )
	{
		osassertex( false,"������Begin��endScene��������ʾ����" );
		return -1;
	}
	osassert( _dstr );

	// �մ�ֱ�ӷ��ء�
	if( _dstr->get_sdStr()[0] == NULL )
		return 0;

	// �˺����ڲ�ʹ��Bengin,endScene��Present.
	d3d_begindraw();
	
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );

	//! ��Ⱦ�ַ����ı�����
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

	// ��ʾ�����������
	if( _cursor )
		disp_inscreen( _cursor,1,1 );

	d3d_enddraw();

	d3d_present();


	return 1;

	unguard;
}



/** \brief
*  �Ե�ǰ����Ļ���н�ͼ.
*  ʹ�����ڼӵ�ǰ��ʱ����Ϊ��ͼ������.
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


	// �ļ��в����ڣ��򴴽��ļ���
	if( !PathIsDirectory( "screenshot" ) )
		CreateDirectory( "screenshot", 0 );

	t_hr = osSaveSurfaceToFile( t_str,D3DXIFF_BMP,t_backBuf,NULL,NULL );
	if( FAILED( t_hr ) )
		MessageBox( NULL,"���ڳ�������Ŀ¼�´���screenShotĿ¼","ע��",MB_OK );

	t_backBuf->Release();

	return t_str;

	unguard;
}

HWND  osc_d3dManager::get_hWnd()
{
	return m_hWnd;
}

//! �õ�������.
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

//! �õ��Կ�����Ϣ
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

	// ���������ص�����
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


//! ȫ��Ļ��fade.
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

