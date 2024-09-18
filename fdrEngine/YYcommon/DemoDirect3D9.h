#pragma once
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

class CDemoDirect3D9
{
private:
    LPDIRECT3D9         m_pD3D; //用来创建D3D设备
    LPDIRECT3DDEVICE9   m_pd3dDevice; //D3D设备

public:
    CDemoDirect3D9(void);
    ~CDemoDirect3D9(void);

    void    CleanupDirect();
    HRESULT InitDirect( HWND hGameWnd );

    void    BeginDirectRender();
    void    EndDirectPresent();
    LPDIRECT3DDEVICE9 GetDirectDevice() { return m_pd3dDevice; }
};

extern CDemoDirect3D9 g_DemoDx9;
