//-----------------------------------------------------------------------------
// Torque Shader Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _GFXD3D9CARDPROFILER_H_
#define _GFXD3D9CARDPROFILER_H_

#ifndef _D3D9_H_
#include <d3d9.h>
#endif

#include "gfxCardProfile.h"


class GFXD3D9CardProfiler : public GFXCardProfiler
{
private:
   typedef GFXCardProfiler Parent;

   LPDIRECT3DDEVICE9 mD3DDevice;
   UINT mAdapterOrdinal;

public:
   GFXD3D9CardProfiler(LPDIRECT3DDEVICE9   _pDevice);
   ~GFXD3D9CardProfiler();
   void init();

protected:
   const char* getRendererString() const { static char* sRS = "D3D9"; return sRS; }

   void setupCardCapabilities();
   bool _queryCardCap(const char* query, unsigned int &foundResult);
   //bool _queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips);
};

#endif
