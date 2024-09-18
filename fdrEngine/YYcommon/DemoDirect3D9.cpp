#include "stdafx.h"
#include "DemoDirect3D9.h"

CDemoDirect3D9 g_DemoDx9;


CDemoDirect3D9::CDemoDirect3D9(void)
{
    m_pD3D = NULL;//用来创建D3D设备
    m_pd3dDevice = NULL;  //D3D设备
}

CDemoDirect3D9::~CDemoDirect3D9(void)
{

}


void CDemoDirect3D9::CleanupDirect()
{
    if( m_pd3dDevice != NULL) 
        m_pd3dDevice->Release();

    if( m_pD3D != NULL) 
        m_pD3D->Release();
}

HRESULT CDemoDirect3D9::InitDirect( HWND hGameWnd )
{   // 创建D3D对象
    if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) ) return E_FAIL;

    // 获取当前的显示模式
    D3DDISPLAYMODE d3ddm;
    if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
        return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;						// 窗口模式
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// 设置交换模式
    d3dpp.BackBufferFormat = d3ddm.Format;		// 设置背景缓冲区格式为当前左面格式

	// 创建D3D设备
	// 第一个参数：使用默认的显卡适配器
	// 第二个参数：请求使用硬件抽象层（HAL）
	// 第三个参数：窗口句柄
	// 第四个参数：使用软件处理顶点
	// 第五个参数：创建的参数
	// 第六个参数：创建的D3D设备指针
    if( FAILED( m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hGameWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    return S_OK;
}


void CDemoDirect3D9::BeginDirectRender()
{
    if( NULL == m_pd3dDevice ) return;

    // 清除背景为兰色
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    // 开始绘制场景
    m_pd3dDevice->BeginScene();
    
    // 绘制游戏相关内容
    
    // 结束场景绘制
}

void CDemoDirect3D9::EndDirectPresent()
{    // 显示到屏幕上
    m_pd3dDevice->EndScene();
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


