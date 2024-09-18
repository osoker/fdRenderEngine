#include "stdafx.h"
#include "DemoDirect3D9.h"

CDemoDirect3D9 g_DemoDx9;


CDemoDirect3D9::CDemoDirect3D9(void)
{
    m_pD3D = NULL;//��������D3D�豸
    m_pd3dDevice = NULL;  //D3D�豸
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
{   // ����D3D����
    if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) ) return E_FAIL;

    // ��ȡ��ǰ����ʾģʽ
    D3DDISPLAYMODE d3ddm;
    if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
        return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;						// ����ģʽ
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// ���ý���ģʽ
    d3dpp.BackBufferFormat = d3ddm.Format;		// ���ñ�����������ʽΪ��ǰ�����ʽ

	// ����D3D�豸
	// ��һ��������ʹ��Ĭ�ϵ��Կ�������
	// �ڶ�������������ʹ��Ӳ������㣨HAL��
	// ���������������ھ��
	// ���ĸ�������ʹ�����������
	// ����������������Ĳ���
	// ������������������D3D�豸ָ��
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

    // �������Ϊ��ɫ
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );
    
    // ��ʼ���Ƴ���
    m_pd3dDevice->BeginScene();
    
    // ������Ϸ�������
    
    // ������������
}

void CDemoDirect3D9::EndDirectPresent()
{    // ��ʾ����Ļ��
    m_pd3dDevice->EndScene();
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


