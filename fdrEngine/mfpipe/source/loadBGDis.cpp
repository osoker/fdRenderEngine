//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: loadBGDis.cpp
 *  Desc:     ����һ������ʱ����ͼƬ����ʾ��
 *  His:      River created @ 2004-4-22
 *
 * �������õ����£�Ҳ�����������¡���
 * 
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/deviceInit.h"



//! �����������ڵ�Ŀ¼��
# define  BACKGROUND_DIR    "ui\\"

//! ��Ⱦ����ʱ�����õ����������Ŀ��
# define  BG_TEXNUM         8

//! ��������λ�á�
# define  PBAR_XSTART       0.1f
# define  PBAR_XEND         0.9f
# define  PBAR_YSTART       0.84f
# define  PBAR_YEND         0.90f
# define  PBAR_OFFSET       16
# define  PBAR_LENGTH       256

//! �����ͼʱ,�����õ��ı���ͼƬ��������.����"back"��ʾui\\back\\Ŀ¼�°���ʽ�ĵ���ͼƬ.
OSENGINE_API char   g_szMapLoadBack[16] = { 'b','a','c','k',0 };



//! 
void osc_d3dManager::bkset_process( const char* _bgName,
				int _x,int _y,const char* _info,DWORD _infoColor  )
{
	guard;

	s_string        t_szTexName,t_szComName;
	char            t_str[32];
	os_screenPicDisp* t_ptrSpd;

	osassert( _bgName );
	osassert( _bgName[0] );

	m_vecSPDisp.resize( BG_TEXNUM );

	for( int t_i=0;t_i<BG_TEXNUM;t_i ++ )
	{
		t_ptrSpd = &m_vecSPDisp[t_i];
		for( int t_j =0;t_j<4;t_j ++ )
			t_ptrSpd->m_scrVer[t_j].m_dwDiffuse = 0xffffffff;
	}

	//! ������ʾ��Ϣ�Լ���Ϣ����ʾλ�á�
	m_iInfoX = _x,m_iInfoY = _y;
	if( _info )
	{
		osassertex( (strlen( _info )<MAX_BKINFOLEN),"��ʾ��Ϣ����...\n" );
		strcpy( m_szBkInfo,_info );
		m_dwBkInfoColor = _infoColor;
	}
	else
		m_szBkInfo[0] = NULL;



	t_szComName = BACKGROUND_DIR;
	t_szComName += _bgName;
	t_szComName += "\\";
	t_szComName += _bgName;

	//
	// �������ǵı�������
	for( int t_i=0;t_i<BG_TEXNUM;t_i++ )
	{
		sprintf( t_str,"%02d.tga",t_i+1 );
		t_szTexName = t_szComName + t_str;
		if( -1 == load_texture( t_szTexName.c_str() ) )
		{
			osassert( false );
			return ;
		}
	}


	unguard;
}


/** \brief
*  ���õ�����Ϸ��������ʱ������ı������档 
* 
*  ��������ʹ��ͳһ��ͼƬ���ʹ��640*480����ʵͼƬ��С��������ͼƬ
*  �зֳ�����256*256��ͼ������128*256��ͼ��
*  ����ʹ��һ��256*64�ĵ����
*  Ӧ���ټ�һ�Ÿ����������ͼƬ,��������ʲôλ�ã���ǰͼƬ�ͷ��õ�ʲôλ�á�
*  
*  \param _bgName ������ǽ���Ŀ¼�����֣������Ŀ¼�´����˱���ͼƬ�����֡�
*  
*  \param _x,_y,_info ���ñ���ʱ����Щ���������ڱ�����ĳ��λ������һ����ʾ��Ϣ,
*                     �����ʾ��Ϣ����ʾ�����Ĺ����У�һֱ����.
*  \param _infoColor   ���ñ�������ʾ��Ϣʱ����ʾ��Ϣ���ִ���ɫ
* 
*/
bool osc_d3dManager::set_backGround( const char* _bgName,
				int _x/* = 0*/,int _y/* = 0*/,
				const char* _info/* = NULL*/,DWORD _infoColor/* = 0xffffffff*/ )
{
	guard;

	s_string        t_szTexName,t_szComName;
	int             t_iWidth,t_iHeight;
	os_screenPicDisp* t_ptrSpd;

	osassert( _bgName );
	osassert( _bgName[0] );

	// ����ȫ�ֵĵ�ͼ���뱳��ͼƬ.
	strcpy( g_szMapLoadBack,_bgName );

	bkset_process( _bgName,_x,_y,_info,_infoColor );
	

	t_iWidth = m_iScrWidth;
	t_iHeight = m_iScrHeight;

	//
	// 
	float   t_fW,t_fH;
	t_fW = float(256)/float(640);
	t_fH = float(256)/float(480);

	for( int t_i=0;t_i<3;t_i++ )
	{
		t_ptrSpd = &m_vecSPDisp[t_i];
		t_ptrSpd->blend_mode = 3;	

		t_ptrSpd->m_scrVer[0].m_vecUv = osVec2D( 0,0 );
		t_ptrSpd->m_scrVer[1].m_vecUv = osVec2D( 1,0 );
		t_ptrSpd->m_scrVer[2].m_vecUv = osVec2D( 1,1 );
		t_ptrSpd->m_scrVer[3].m_vecUv = osVec2D( 0,1 );
		
		t_ptrSpd->m_scrVer[0].m_vecPos.x = (float)round_float( t_fW*t_i*t_iWidth );
		t_ptrSpd->m_scrVer[0].m_vecPos.y = 0;

		t_ptrSpd->m_scrVer[1].m_vecPos.x = (float)round_float( t_fW*(t_i+1)*t_iWidth );
		if( t_ptrSpd->m_scrVer[1].m_vecPos.x > (float)t_iWidth )
			t_ptrSpd->m_scrVer[1].m_vecPos.x = (float)t_iWidth;
		t_ptrSpd->m_scrVer[1].m_vecPos.y = 0;

		t_ptrSpd->m_scrVer[2].m_vecPos.x = (float)round_float( t_fW*(t_i+1)*t_iWidth );
		if( t_ptrSpd->m_scrVer[2].m_vecPos.x > (float)t_iWidth )
			t_ptrSpd->m_scrVer[2].m_vecPos.x = (float)t_iWidth;
		t_ptrSpd->m_scrVer[2].m_vecPos.y = (float)round_float( t_fH*t_iHeight );

		t_ptrSpd->m_scrVer[3].m_vecPos.x = (float)round_float( t_fW*t_i*t_iWidth );
		t_ptrSpd->m_scrVer[3].m_vecPos.y = (float)round_float( t_fH*t_iHeight );

		t_ptrSpd->tex_id = t_i;
	}

	for( int t_i=0;t_i<3;t_i++ )
	{
		t_ptrSpd = &m_vecSPDisp[t_i+3];
		t_ptrSpd->blend_mode = 3;

		t_ptrSpd->m_scrVer[0].m_vecUv = osVec2D( 0,0 );
		t_ptrSpd->m_scrVer[1].m_vecUv = osVec2D( 1,0 );
		t_ptrSpd->m_scrVer[2].m_vecUv = osVec2D( 1,0.875 );
		t_ptrSpd->m_scrVer[3].m_vecUv = osVec2D( 0,0.875 );

		t_ptrSpd->m_scrVer[0].m_vecPos.x = (float)round_float( t_fW*t_i*t_iWidth );
		t_ptrSpd->m_scrVer[0].m_vecPos.y = (float)round_float( t_fH*t_iHeight );

		t_ptrSpd->m_scrVer[1].m_vecPos.x = (float)round_float( t_fW*(t_i+1)*t_iWidth );
		if( t_ptrSpd->m_scrVer[1].m_vecPos.x > (float)t_iWidth )
			t_ptrSpd->m_scrVer[1].m_vecPos.x = (float)t_iWidth;
		t_ptrSpd->m_scrVer[1].m_vecPos.y = (float)round_float( t_fH*t_iHeight );

		t_ptrSpd->m_scrVer[2].m_vecPos.x = (float)round_float( t_fW*(t_i+1)*t_iWidth );
		if( t_ptrSpd->m_scrVer[2].m_vecPos.x > (float)t_iWidth )
			t_ptrSpd->m_scrVer[2].m_vecPos.x = (float)t_iWidth;
		t_ptrSpd->m_scrVer[2].m_vecPos.y = (float)t_iHeight;

		t_ptrSpd->m_scrVer[3].m_vecPos.x = (float)round_float( t_fW*t_i*t_iWidth );
		t_ptrSpd->m_scrVer[3].m_vecPos.y = (float)t_iHeight;

		t_ptrSpd->tex_id = t_i + 3;
	}

	//
	// �������ǵĽ�����.
	t_ptrSpd = &m_vecSPDisp[6];
	t_ptrSpd->blend_mode = 3;
	t_ptrSpd->tex_id = 6;

	t_ptrSpd->m_scrVer[0].m_vecUv = osVec2D( 0,0 );
	t_ptrSpd->m_scrVer[1].m_vecUv = osVec2D( 1,0 );
	t_ptrSpd->m_scrVer[2].m_vecUv = osVec2D( 1,1 );
	t_ptrSpd->m_scrVer[3].m_vecUv = osVec2D( 0,1 );

	//
	// ������Ļ�Ŀ�ȵ��Ե�ǰ�����ķ���λ�á�
	int      t_iXstart;
	t_iXstart = int((t_iWidth - PBAR_LENGTH)/2.0f);
	t_ptrSpd->m_scrVer[0].m_vecPos.x = float(t_iXstart);
	t_ptrSpd->m_scrVer[0].m_vecPos.y = PBAR_YSTART*t_iHeight;

	t_ptrSpd->m_scrVer[1].m_vecPos.x = float(t_iXstart+PBAR_LENGTH);
	t_ptrSpd->m_scrVer[1].m_vecPos.y = PBAR_YSTART*t_iHeight;

	t_ptrSpd->m_scrVer[2].m_vecPos.x = float(t_iXstart+PBAR_LENGTH);
	t_ptrSpd->m_scrVer[2].m_vecPos.y = /*PBAR_YEND*t_iHeight;*/
		t_ptrSpd->m_scrVer[0].m_vecPos.y + PBAR_OFFSET;

	t_ptrSpd->m_scrVer[3].m_vecPos.x = float(t_iXstart);
	t_ptrSpd->m_scrVer[3].m_vecPos.y = /*PBAR_YEND*t_iHeight;*/
		t_ptrSpd->m_scrVer[0].m_vecPos.y + PBAR_OFFSET;


	//
	// ������ȸı�Ľ�������
	t_ptrSpd = &m_vecSPDisp[7];
	t_ptrSpd->blend_mode = 3;
	t_ptrSpd->tex_id = 7;

	t_ptrSpd->m_scrVer[0].m_vecUv = osVec2D( 0,0 );
	t_ptrSpd->m_scrVer[1].m_vecUv = osVec2D( 1,0 );
	t_ptrSpd->m_scrVer[2].m_vecUv = osVec2D( 1,1 );
	t_ptrSpd->m_scrVer[3].m_vecUv = osVec2D( 0,1 );

	t_ptrSpd->m_scrVer[0].m_vecPos.y = PBAR_YSTART*t_iHeight;
	t_ptrSpd->m_scrVer[1].m_vecPos.y = PBAR_YSTART*t_iHeight;
	t_ptrSpd->m_scrVer[0].m_vecPos.x = float(t_iXstart);
	t_ptrSpd->m_scrVer[3].m_vecPos.x = float(t_iXstart);

	t_ptrSpd->m_scrVer[2].m_vecPos.y = /*PBAR_YEND*t_iHeight;*/
		t_ptrSpd->m_scrVer[0].m_vecPos.y + PBAR_OFFSET;
	t_ptrSpd->m_scrVer[3].m_vecPos.y = /*PBAR_YEND*t_iHeight;*/
		t_ptrSpd->m_scrVer[0].m_vecPos.y + PBAR_OFFSET;

	m_fCurProgress = 0.0f;


	return true;

	unguard;
}

/** \brief
*  ���÷������չ��
*
*  ���ֵΪ1.0,�����չ���϶����������չ����
*
*  ÿ���������ý�������ʱ��������Ⱦ��Ļ�����½���������
*  ����Ⱦ�����ĻPresent��ǰ̨���д���
*  
*/
void osc_d3dManager::set_progress( float _addps,int _x,int _y,
					const char* _info/* = NULL*/,DWORD _infoColor/* = 0xffffffff*/ )
{
	guard;

	os_tmpdisplayTex* t_ptrTex;
	int   t_iWidth;
	float t_f;

	osassert( m_pd3dDevice );
	osassert( _addps >= 0 );

	t_ptrTex = &m_vecTexture[0];
	if( !t_ptrTex )
		return;
	if( t_ptrTex->tex == NULL )
		return;

	m_fCurProgress += _addps;
	if( m_fCurProgress>1.0f )
		m_fCurProgress = 1.0f;

	// 
	// ��̬�Ĵ���������Ĵ�С��λ�á�
	os_screenPicDisp* t_ptrSpd;

	t_ptrSpd = &m_vecSPDisp[7];
	t_iWidth = m_iScrWidth;
	t_f = m_fCurProgress*PBAR_LENGTH;
	t_f += t_ptrSpd->m_scrVer[0].m_vecPos.x;
	t_ptrSpd->m_scrVer[1].m_vecPos.x = (float)round_float( t_f );
	t_ptrSpd->m_scrVer[2].m_vecPos.x = (float)round_float( t_f );

	// 
	// ����������������ȷ��u����ֵ��
	t_ptrSpd->m_scrVer[1].m_vecUv.x = m_fCurProgress;
	t_ptrSpd->m_scrVer[2].m_vecUv.x = m_fCurProgress;

	// 
	// ȷ�ϵ�ǰ����Ⱦ״̬��������beginScene��endScene֮�����beginScene.
	if( m_bInDrawScene )
		d3d_enddraw();

	//
	// ��Ⱦ���ǽ��������档
	d3d_clear( 0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
		                                0x00000000, 1.0f, 0L );

	d3d_begindraw();

	disp_inscreen( &m_vecSPDisp[0],8,0 );

	// ����������ʾ��Ϣ,����ȴ���ϢΪ�գ�������ʹ����һ�α������Ϣ��
	if( _info )
	{
		os_stringDisp   t_disp;
		t_disp.charHeight = 12;
		t_disp.color = _infoColor;
		t_disp = _info;
		t_disp.x = _x;
		t_disp.y = _y;

		osDebugOut( "Start Draw bk String...\n" );
		disp_string( &t_disp,1 );

		m_iWaitInfoX = _x;
		m_iWaitInfoY = _y;
		strcpy( m_szWaitInfo,_info );
		m_dwWaitInfoColor = _infoColor;
	}
	else
	{
		if( m_szWaitInfo[0] )
		{
			os_stringDisp   t_disp;
			t_disp.charHeight = 12;
			t_disp.color = m_dwWaitInfoColor;
			t_disp = m_szWaitInfo;
			t_disp.x = m_iWaitInfoX;
			t_disp.y = m_iWaitInfoY;

			disp_string( &t_disp,1 );
		}
	}

	// ��ʾ�����İ�����Ϣ
	if( m_szBkInfo[0] != NULL )
	{
		os_stringDisp   t_disp;
		t_disp.charHeight = 12;
		t_disp.color = m_dwBkInfoColor;
		t_disp = m_szBkInfo;
		t_disp.x = m_iInfoX;
		t_disp.y = m_iInfoY;

		disp_string( &t_disp,1 );
	}

	d3d_enddraw();

	d3d_present();


	return;

	unguard;
}


