//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: loadBGDis.cpp
 *  Desc:     调入一个场景时背景图片的显示。
 *  His:      River created @ 2004-4-22
 *
 * “有做得到的事，也有做不到的事。”
 * 
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/deviceInit.h"



//! 背景纹理所在的目录。
# define  BACKGROUND_DIR    "ui\\"

//! 渲染背景时我们用到的纹理的数目。
# define  BG_TEXNUM         8

//! 进度条的位置。
# define  PBAR_XSTART       0.1f
# define  PBAR_XEND         0.9f
# define  PBAR_YSTART       0.84f
# define  PBAR_YEND         0.90f
# define  PBAR_OFFSET       16
# define  PBAR_LENGTH       256

//! 调入地图时,引擎用到的背景图片集的名字.比如"back"表示ui\\back\\目录下按格式的调入图片.
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

	//! 背景提示信息以及信息的显示位置。
	m_iInfoX = _x,m_iInfoY = _y;
	if( _info )
	{
		osassertex( (strlen( _info )<MAX_BKINFOLEN),"提示信息过长...\n" );
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
	// 调入我们的背景纹理。
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
*  设置调入游戏或其它长时间任务的背景画面。 
* 
*  背景画面使用统一的图片规格，使用640*480的真实图片大小，把这张图片
*  切分成四张256*256的图加两张128*256的图。
*  并且使用一张256*64的调入框。
*  应该再加一张跟随进度条的图片,进度条到什么位置，则当前图片就放置到什么位置。
*  
*  \param _bgName 传入的是界面目录的名字，在这个目录下存入了背景图片的名字。
*  
*  \param _x,_y,_info 设置背景时，这些参数可以在背景的某个位置设置一个提示信息,
*                     这个提示信息在显示背景的过程中，一直存在.
*  \param _infoColor   设置背景的提示信息时，提示信息的字串颜色
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

	// 保存全局的地图调入背景图片.
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
	// 放置我们的进度条.
	t_ptrSpd = &m_vecSPDisp[6];
	t_ptrSpd->blend_mode = 3;
	t_ptrSpd->tex_id = 6;

	t_ptrSpd->m_scrVer[0].m_vecUv = osVec2D( 0,0 );
	t_ptrSpd->m_scrVer[1].m_vecUv = osVec2D( 1,0 );
	t_ptrSpd->m_scrVer[2].m_vecUv = osVec2D( 1,1 );
	t_ptrSpd->m_scrVer[3].m_vecUv = osVec2D( 0,1 );

	//
	// 根据屏幕的宽度调试当前度条的放置位置。
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
	// 处理进度改变的进度条。
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
*  设置符任务进展。
*
*  最大值为1.0,这个进展将拖动我们任务进展条。
*
*  每次我们设置进度条的时候，我们渲染屏幕，更新进度条，并
*  把渲染后的屏幕Present到前台进行处理。
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
	// 动态的处理进度条的大小和位置。
	os_screenPicDisp* t_ptrSpd;

	t_ptrSpd = &m_vecSPDisp[7];
	t_iWidth = m_iScrWidth;
	t_f = m_fCurProgress*PBAR_LENGTH;
	t_f += t_ptrSpd->m_scrVer[0].m_vecPos.x;
	t_ptrSpd->m_scrVer[1].m_vecPos.x = (float)round_float( t_f );
	t_ptrSpd->m_scrVer[2].m_vecPos.x = (float)round_float( t_f );

	// 
	// 设置这两个顶点正确的u方向值。
	t_ptrSpd->m_scrVer[1].m_vecUv.x = m_fCurProgress;
	t_ptrSpd->m_scrVer[2].m_vecUv.x = m_fCurProgress;

	// 
	// 确认当前的渲染状态，不能在beginScene和endScene之间调用beginScene.
	if( m_bInDrawScene )
		d3d_enddraw();

	//
	// 渲染我们进度条画面。
	d3d_clear( 0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
		                                0x00000000, 1.0f, 0L );

	d3d_begindraw();

	disp_inscreen( &m_vecSPDisp[0],8,0 );

	// 处理文字提示信息,如果等待信息为空，则试着使用上一次保存的信息。
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

	// 显示背景的帮助信息
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


