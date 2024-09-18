//-----------------------------------------------------------------------------
// Torque Shader Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "../mfpipe/include/frontPipe.h"
#include "gfxD3D9CardProfiler.h"
#include "wmiVideoInfo.h"


GFXD3D9CardProfiler::GFXD3D9CardProfiler(LPDIRECT3DDEVICE9 _pDevice) : GFXCardProfiler()
{
	mD3DDevice = _pDevice;
}

GFXD3D9CardProfiler::~GFXD3D9CardProfiler()
{

}

void GFXD3D9CardProfiler::init()
{
   //mD3DDevice = g_frender->get_d3dDevice();//dynamic_cast<GFXD3D9Device *>(GFX)->getDevice();
   osassertex( mD3DDevice, "GFXD3D9CardProfiler::init() - No D3D9 Device found!");

   // Grab the caps so we can get our adapter ordinal and look up our name.
   D3DCAPS9 caps;
   if(FAILED(mD3DDevice->GetDeviceCaps(&caps))){
	   osassertex(false,"GFXD3D9CardProfiler::init - failed to get device caps!");
   }
   

   WMIVideoInfo wmiVidInfo;
   if( wmiVidInfo.profileAdapters() )
   {
      const PlatformVideoInfo::PVIAdapter &adapter = wmiVidInfo.getAdapterInformation( caps.AdapterOrdinal );

      mCardDescription = adapter.description;
      mChipSet = adapter.chipSet;
      mVersionString = adapter.driverVersion;
      mVideoMemory = adapter.vram;
   }

   Parent::init();
}

void GFXD3D9CardProfiler::setupCardCapabilities()
{
   // Get the D3D device caps
   D3DCAPS9 caps;
   mD3DDevice->GetDeviceCaps(&caps);

   setCapability( "autoMipMapLevel", ( caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP ? 1 : 0 ) );

   setCapability( "maxTextureWidth", caps.MaxTextureWidth );
   setCapability( "maxTextureHeight", caps.MaxTextureHeight );
   setCapability( "maxTextureSize", min( (unsigned int)caps.MaxTextureWidth, (unsigned int)caps.MaxTextureHeight) );

   bool canDoLERPDetailBlend = ( caps.TextureOpCaps & D3DTEXOPCAPS_LERP ) && ( caps.MaxTextureBlendStages > 1 );

   bool canDoFourStageDetailBlend = ( caps.TextureOpCaps & D3DTEXOPCAPS_SUBTRACT ) &&
                                    ( caps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP ) &&
                                    ( caps.MaxTextureBlendStages > 3 );

   setCapability( "lerpDetailBlend", canDoLERPDetailBlend );
   setCapability( "fourStageDetailBlend", canDoFourStageDetailBlend );

   // Ask the device about R8G8B8 textures. Some devices will not like 24-bit RGB
   // and will insist on R8G8B8X8, 32-bit, RGB texture format.
   IDirect3D9 *pD3D = NULL;
   D3DDISPLAYMODE displayMode;
   mD3DDevice->GetDirect3D( &pD3D );
   mD3DDevice->GetDisplayMode( 0, &displayMode );
   HRESULT hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
                                    displayMode.Format, 0 /* usage flags = 0 */, 
                                    D3DRTYPE_TEXTURE, D3DFMT_R8G8B8 );

   setCapability( ( "allowRGB24BitTextures" ), SUCCEEDED( hr ) ); 

   SAFE_RELEASE( pD3D );
}

bool GFXD3D9CardProfiler::_queryCardCap(const char* query, unsigned int &foundResult)
{
   return 0;
}
//
//bool GFXD3D9CardProfiler::_queryFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips )
//{
//   // TODO: Store these as members? -Patw
//   IDirect3D9 *pD3D = NULL;
//   D3DDISPLAYMODE displayMode;
//   mD3DDevice->GetDirect3D( &pD3D );
//   mD3DDevice->GetDisplayMode( 0, &displayMode );
//
//   DWORD usage = 0;
//   D3DRESOURCETYPE rType = D3DRTYPE_TEXTURE;
//
//   if( profile->isRenderTarget() )
//      usage |= D3DUSAGE_RENDERTARGET;
//   else if( profile->isZTarget() )
//   {
//      usage |= D3DUSAGE_DEPTHSTENCIL;
//      rType = D3DRTYPE_SURFACE;
//   }
//   
//   if( inOutAutogenMips )
//      usage |= D3DUSAGE_AUTOGENMIPMAP;
//
//   HRESULT hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
//      displayMode.Format, usage, rType, GFXD3D9TextureFormat[fmt] );
//
//   bool retVal = SUCCEEDED( hr );
//
//   // If check device format failed, and auto gen mips were requested, try again
//   // without autogen mips.
//   if( !retVal && inOutAutogenMips )
//   {
//      usage ^= D3DUSAGE_AUTOGENMIPMAP;
//
//      hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
//         displayMode.Format, usage, D3DRTYPE_TEXTURE, GFXD3D9TextureFormat[fmt] );
//
//      retVal = SUCCEEDED( hr );
//
//      // If this one passed, auto gen mips are not supported with this format, 
//      // so set the variable properly
//      if( retVal )
//         inOutAutogenMips = false;
//   }
//
//   SAFE_RELEASE( pD3D );
//
//   return retVal;
//}