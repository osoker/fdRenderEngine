//-----------------------------------------------------------------------------
/** \file
 *  Filename:  D3DEnumeration.h
 *
 * Desc:       Enumerates D3D adapters, devices, modes, etc.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * River Get it from Dx9.0 App Framework @ 4/14 2003.
 * 
 */
//-----------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"

class CArrayList;


/**
 * Name: struct D3DAdapterInfo
 * Desc: Info about a display adapter.
 */
struct D3DAdapterInfo
{
    int AdapterOrdinal;
    D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
    CArrayList* pDisplayModeList; // List of D3DDISPLAYMODEs
    CArrayList* pDeviceInfoList; // List of D3DDeviceInfo pointers
    ~D3DAdapterInfo( void );
};


/**
 * Name: struct D3DDeviceInfo
 * Desc: Info about a D3D device, including a list of D3DDeviceCombos (see below) 
 *       that work with the device.
 */
struct D3DDeviceInfo
{
    int AdapterOrdinal;
    D3DDEVTYPE DevType;
    D3DCAPS9 Caps;
    CArrayList* pDeviceComboList; // List of D3DDeviceCombo pointers
    ~D3DDeviceInfo( void );
};


/** 
 * Name: struct D3DDSMSConflict
 * Desc: A depth/stencil buffer format that is incompatible with a
 *       multisample type.
 */
struct D3DDSMSConflict
{
    D3DFORMAT DSFormat;
    D3DMULTISAMPLE_TYPE MSType;
};



/**
 * Name: struct D3DDeviceCombo
 *
 * Desc: A combination of adapter format, back buffer format, and windowed/fullscreen 
 *       that is compatible with a particular D3D device (and the app).
 *       每一种Device可以有多个D3DDeviceCombo.
 *
 */
struct D3DDeviceCombo
{
    int AdapterOrdinal;
    D3DDEVTYPE DevType;
    D3DFORMAT AdapterFormat;
    D3DFORMAT BackBufferFormat;
    bool IsWindowed;
    CArrayList* pDepthStencilFormatList; // List of D3DFORMATs
    CArrayList* pMultiSampleTypeList; // List of D3DMULTISAMPLE_TYPEs
    CArrayList* pMultiSampleQualityList; // List of DWORDs (number of quality 
                                         // levels for each multisample type)
    CArrayList* pDSMSConflictList; // List of D3DDSMSConflicts
    CArrayList* pVertexProcessingTypeList; // List of VertexProcessingTypes
    CArrayList* pPresentIntervalList; // List of D3DPRESENT_INTERVALs

    ~D3DDeviceCombo( void );
};


typedef bool(* CONFIRMDEVICECALLBACK)( IDirect3D9* _d3d9,D3DCAPS9* pCaps, ose_VertexProcessingType vertexProcessingType, 
									   D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );


//-----------------------------------------------------------------------------
// Name: class CD3DEnumeration
// Desc: Enumerates available D3D adapters, devices, modes, etc.
/** \brief 
 * CD3DEnumeration 取自Dx9.0的框架文件,列出所有符合我们要求的显示模式.
 * 
 * 使用方式: 
 * 1: 使用SetD3D函数设置3D Object.
 * 
 * 2: 设置ConfirmDeviceCallback函数.
 *
 * 3: 设置其它的公有变量.
 *
 * 4: 调用Enumerate函数完成列举.
 * 
 */
//-----------------------------------------------------------------------------
class CD3DEnumeration
{
private:
    IDirect3D9* m_pD3D;

private:
    HRESULT EnumerateDevices( D3DAdapterInfo* pAdapterInfo, CArrayList* pAdapterFormatList );
    HRESULT EnumerateDeviceCombos( D3DDeviceInfo* pDeviceInfo, CArrayList* pAdapterFormatList );
    void BuildDepthStencilFormatList( D3DDeviceCombo* pDeviceCombo );
    void BuildMultiSampleTypeList( D3DDeviceCombo* pDeviceCombo );
    void BuildDSMSConflictList( D3DDeviceCombo* pDeviceCombo );
    void BuildVertexProcessingTypeList( D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo );
    void BuildPresentIntervalList( D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo );

public:
    CD3DEnumeration();
    ~CD3DEnumeration();

    void SetD3D(IDirect3D9* pD3D) { m_pD3D = pD3D; }
    HRESULT Enumerate();

	//@{
	/**
     * The variables can be used to limit what modes, formats, 
     * etc. are enumerated.  Set them to the values you want before calling
     * Enumerate().
	 */
    CONFIRMDEVICECALLBACK ConfirmDeviceCallback;
    UINT AppMinFullscreenWidth;
    UINT AppMinFullscreenHeight;
    UINT AppMinColorChannelBits; // min color bits per channel in adapter format
    UINT AppMinAlphaChannelBits; // min alpha bits per pixel in back buffer format
    UINT AppMinDepthBits;
    UINT AppMinStencilBits;
    bool AppUsesDepthBuffer;
    bool AppUsesMixedVP; // whether app can take advantage of mixed vp mode
    bool AppRequiresWindowed;
    bool AppRequiresFullscreen;
    CArrayList* m_pAllowedAdapterFormatList; // list of D3DFORMATs
	//@}

    CArrayList* m_pAdapterInfoList;

};
