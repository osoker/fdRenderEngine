//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdWater.h
 *
 *  Desc:     场景中水相关的特效的处理。
 *
 *  His:      River created @ 2004-5-13.
 *
 *  "最不依赖幸运的人最容易保持他自己的地位"　( 马基雅维里 )
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdWater.h"
# include "../../backpipe/include/osCamera.h"
# include "../../mfpipe/include/osShader.h"
# include "../../backpipe/include/mainEngine.h"

# if 0  //弃用此文件。

//! 定义一个水面格子的大小。
# define WATER_TILESIZE   6.0f


//@{
//! 定义水相关的ini.
# define WATER_SECTOR  "water"
# define WTEX_NAME     "tname"
# define MOVE_SPEED    "movespeed"
# define X_SIZE        "xsize"
# define Z_SIZE        "zsize"
# define WATER_DIR     "dir"
# define FRAME_NUM     "framenum"
# define WATER_ALPHA   "alpha"
//@}





//! 定义shader对应的文件。
//  水的shader定义文件和公告板使用同一个fx文件。

//  每一个水的实例都必须创建在原点，然后使用当前水实例的位置
//  和旋转方向，在场景中得到正确放置的水。

osc_dwater::osc_dwater()
{
	m_fCurFrameEleTime = 0.0f;
	m_strWTextureName[0] = NULL;
	m_fRotAgl = 0;
	m_vec3CPos.x = m_vec3CPos.y = m_vec3CPos.z = 0.0f;

	m_bInuse = false;
}

osc_dwater::~osc_dwater()
{
	
}



//! 根据方向的不同得到格子用到的uv坐标。
void osc_dwater::get_uvFromWaterDir( osVec2D* _uv )
{
	guard;

	osassert( _uv );

	// 按顺时针方向。
	switch( m_iWaterDir )
	{
	case 1:
		_uv[0] = osVec2D( 0.0f,0.0f );
		_uv[1] = osVec2D( 1.0f,0.0f );
		_uv[2] = osVec2D( 1.0f,1.0f );
		_uv[3] = osVec2D( 0.0f,1.0f );
		break;
	case 2:
		_uv[0] = osVec2D( 0.0f,1.0f );
		_uv[1] = osVec2D( 0.0f,0.0f );
		_uv[2] = osVec2D( 1.0f,0.0f );
		_uv[3] = osVec2D( 1.0f,1.0f );
		break;
	case 3:
		_uv[0] = osVec2D( 1.0f,1.0f );
		_uv[1] = osVec2D( 0.0f,1.0f );
		_uv[2] = osVec2D( 0.0f,0.0f );
		_uv[3] = osVec2D( 1.0f,0.0f );
		break;
	case 4:
		_uv[0] = osVec2D( 1.0f,0.0f );
		_uv[1] = osVec2D( 1.0f,1.0f );
		_uv[2] = osVec2D( 0.0f,1.0f );
		_uv[3] = osVec2D( 0.0f,0.0f );
		break;
	}
	return;

	unguard;
}



//! 填充格子水面的顶点。
void osc_dwater::fill_waterTileVer( int _x,int _y,os_verWater* _ver )
{
	guard;

	osassert( _ver );
	osassert( (_x >=0) &&(_y >=0) );

	// 顶点从左上角开始顺时针排列。


	_ver[0].m_vecPos.x =  _x   * WATER_TILESIZE  + m_vec3WStart.x;
	_ver[0].m_vecPos.z = (_y+1) * WATER_TILESIZE + m_vec3WStart.z;

	_ver[1].m_vecPos.x = (_x+1) * WATER_TILESIZE + m_vec3WStart.x;;
	_ver[1].m_vecPos.z = (_y+1) * WATER_TILESIZE + m_vec3WStart.z;;

	_ver[2].m_vecPos.x = (_x+1) * WATER_TILESIZE + m_vec3WStart.x;;
	_ver[2].m_vecPos.z = (_y)   * WATER_TILESIZE + m_vec3WStart.z;

	_ver[3].m_vecPos.x = _x * WATER_TILESIZE + m_vec3WStart.x;
	_ver[3].m_vecPos.z = _y * WATER_TILESIZE + m_vec3WStart.z;

	return;

	unguard;
}

//! 初始化水面的shader id.
int osc_dwater::get_polygonSId( int _x,int _z )
{
	guard;

	int   t_id;

	if( (_x == 0)&&(_z==0) )
		t_id = 0;
	else if( _x == 0 )
	{
		t_id = m_vecShaderId[(_z-1)*m_iXSize]+1;
	}
	else
		t_id = m_vecShaderId[_z*m_iXSize+_x-1]+1;

	if( t_id >= (m_iFrameNum) )
		t_id -= m_iFrameNum;

	osassert( (t_id>=0)&&
		     (t_id<(m_iFrameNum) ) );

	return t_id;

	unguard;
}

//! 每一帧处理水的动画贴图变化。
void osc_dwater::frame_moveWater( float _etime )
{
	guard;

	osassert( _etime>=0 );

	int   t_moveFrame;

	m_fCurFrameEleTime += _etime;

	t_moveFrame = 0;
	if( m_fCurFrameEleTime > m_fMoveSpeed )
	{
		t_moveFrame = float_div( m_fCurFrameEleTime,m_fMoveSpeed );
		m_fCurFrameEleTime = float_mod( m_fCurFrameEleTime,m_fMoveSpeed );
	}

	if( t_moveFrame == 0 )
		return;


	int   t_idx;
	for( int t_z = 0;t_z <this->m_iZSize;t_z ++ )
	{
		for( int t_x = 0;t_x <this->m_iXSize;t_x ++ )
		{
			t_idx = t_z*m_iXSize+t_x;

			m_vecShaderId[t_idx] += t_moveFrame;
			if( m_vecShaderId[t_idx]>=(m_iFrameNum) )
			{
				// 使用求余而非减。
				m_vecShaderId[t_idx] %= m_iFrameNum;
			}

		}
	}

	return;

	unguard;
}


//! 根据我们的数据初始化我们的polygon.
void osc_dwater::init_rPolygon( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	os_polygon*   t_ptrPolygon;
	int           t_idx;
	char          t_szShaName[256];

	// 先创建水面用到的shader和id
	// 使用billboard的fx文件，渲染状态需要图加调整。
	osassert( m_iFrameNum <= MAX_WATERFRAME );
	for( int t_i=0;t_i<this->m_iFrameNum;t_i ++ )
	{
		sprintf( t_szShaName,"%s%02d",m_strWTextureName,t_i );
		m_vecDevdepShaderId[t_i] = g_shaderMgr->add_shader( t_szShaName, osc_engine::get_shaderFile( "billBoard" ).c_str() );
		if( m_vecDevdepShaderId[t_i] < 0 )
			m_vecDevdepShaderId[t_i] = g_shaderMgr->add_shadertomanager( t_szShaName,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
	}

	// 使用顶点alpha，使边上水有alpha效果。
	// 填充每一个水格子用到的polygon.
	for( int t_y=0;t_y<m_iZSize;t_y++ )
	{
		for( int t_x=0;t_x<m_iXSize;t_x++ )
		{
			t_idx = t_y*m_iXSize+t_x;
			t_ptrPolygon = &m_arrPolygon[t_idx];
			
			// 指定每一个polygon的shaderID.
			m_vecShaderId[t_idx] = get_polygonSId( t_x,t_y );
	
			t_ptrPolygon->m_bUseLmp = false;
			t_ptrPolygon->m_arrVerData = &m_arrWVertex[t_idx*4];
			t_ptrPolygon->m_iIdxNum = 6;
			t_ptrPolygon->m_iIbufId = -1;
			t_ptrPolygon->m_bUseStaticVB = false;
			t_ptrPolygon->m_bUseStaticIB = false;
			t_ptrPolygon->m_iPriNum = 2;
			t_ptrPolygon->m_iPriType = TRIANGLELIST;
			t_ptrPolygon->m_iVerSize = sizeof( os_verWater );
			t_ptrPolygon->m_iVerNum = 4;
			t_ptrPolygon->m_iVertexStart = 0;

			// 填充顶点相关。
			fill_waterTileVer( t_x,t_y,
				(os_verWater *)t_ptrPolygon->m_arrVerData );

		}
	}

	return ;

	unguard;
}


//! 从水面的中心点得到水面的起始点位置。
void osc_dwater::get_waterStartPos( void )
{
	guard;

	float   t_f;

	t_f = m_iXSize/2.0f;
	t_f *= WATER_TILESIZE;
	m_vec3WStart.x = - t_f;

	t_f = m_iZSize/2.0f;
	t_f *= WATER_TILESIZE;
	m_vec3WStart.z = - t_f;

	m_vec3WStart.y = 0.0f;

	return;

	unguard;
}


//! 从ini文件中读入水的相关配置。
bool osc_dwater::read_waterIni( const char* _iname )
{
	guard;

	osassert( _iname&&(_iname[0]) );

	CIniFile  t_file;

	if( !t_file.OpenFileRead( _iname ) )
		osassert( false );

	if( !t_file.ReadSection( WATER_SECTOR ) )
		return false;

	t_file.ReadLine( WTEX_NAME,(char*)m_strWTextureName,128 );
	t_file.ReadLine( MOVE_SPEED,(float*)&m_fMoveSpeed );
	t_file.ReadLine( X_SIZE,(int*)&m_iXSize );
	t_file.ReadLine( Z_SIZE,(int*)&m_iZSize );
	t_file.ReadLine( WATER_DIR,(int*)&m_iWaterDir );   
	t_file.ReadLine( FRAME_NUM,(int*)&m_iFrameNum );
	t_file.ReadLine( WATER_ALPHA,(float*)&m_fAlpha ); 

	t_file.CloseFile();

	return true;

	unguard;
}



//! 从水面文件中读入数据。
bool osc_dwater::read_waterFile( const char* _wname )
{
	guard;

	BYTE*             t_start;
	int               t_size;
	os_waterCreate     t_wCreate;
	WATER_FILE_HEAD  t_header;

	osassert( _wname&&(_wname[0]) );

	// 读入文件到内存。
	int t_iGBufIdx = -1;
	t_start = START_USEGBUF( t_iGBufIdx );
	t_size = (DWORD)::read_fileToBuf( (char*)_wname,t_start,TMP_BUFSIZE );
	osassert( t_size > 0 );
	

	// 读入水面文件数据。
	READ_MEM_OFF( &t_header,t_start,sizeof( WATER_FILE_HEAD ) );
	READ_MEM_OFF( &t_wCreate,t_start,sizeof( os_waterCreate ) );

	// 给水面分配数据。
	strcpy( m_strWTextureName,t_wCreate.m_szWTexName );
	m_fMoveSpeed = t_wCreate.m_fMoveSpeed;
	m_iXSize = t_wCreate.m_iXSize;
	m_iZSize = t_wCreate.m_iZSize;
	m_iWaterDir = t_wCreate.m_iWaterDir;   
	m_iFrameNum = t_wCreate.m_iFrameNum;
	m_fAlpha = t_wCreate.m_fWAlpah; 

	END_USEGBUF( t_iGBufIdx );

	return true;

	unguard;
}



//  测试函数。
//! 从文件中调入单独的水文件。
bool osc_dwater::load_waterFromfile( const char* _fname,
									osVec3D& _pos,float _rgl,osc_middlePipe* _pipe  )
{
	guard;

	osassert( _fname&&(_fname[0]) );
	osassert( _pipe );


	// 
	// 读入水面文件。
	if( !read_waterFile( _fname ) )
		return false;


	m_vec3CPos = _pos;
	m_fRotAgl = _rgl;


	m_arrPolygon.resize( m_iXSize*m_iZSize );
	m_vecShaderId.resize( m_iXSize*m_iZSize );
	m_arrWVertex.resize( m_iXSize*m_iZSize*4 );

	// 得到水面的开始位置。
	get_waterStartPos();

	// 根据方向得到uv坐标。
	osVec2D      t_vec2Uv[4];
	get_uvFromWaterDir( t_vec2Uv );

	// 
	// 计算alpha值，填充默认的顶点数据。
	DWORD   t_dwAlpha;
	t_dwAlpha = 0xff;
	t_dwAlpha = DWORD(t_dwAlpha*m_fAlpha);
	t_dwAlpha <<= 24;
	t_dwAlpha |= 0x00ffffff;

	for( int t_i=0;t_i<m_iXSize*m_iZSize;t_i ++ )
	{
		m_arrWVertex[t_i*4+0].m_vecUv = t_vec2Uv[0];
		m_arrWVertex[t_i*4+1].m_vecUv = t_vec2Uv[1];
		m_arrWVertex[t_i*4+2].m_vecUv = t_vec2Uv[2];
		m_arrWVertex[t_i*4+3].m_vecUv = t_vec2Uv[3];

		for( int t_j=0;t_j<4;t_j++ )
		{
			m_arrWVertex[t_i*4+t_j].m_dwColor = t_dwAlpha;
			m_arrWVertex[t_i*4+t_j].m_vecPos.y = m_vec3WStart.y;
		}
	}

	init_rPolygon( _pipe );

	m_bInuse = true;

	return true;

	unguard;
}


//! 渲染当前的水面。
bool osc_dwater::render_water( I_camera*  _camptr,float _etime,osc_middlePipe* _pipe )
{
	guard;

	static WORD      t_wTriIdx[6] = { 0,1,2,0,2,3 };
	int          t_idx;
	os_polygon*   t_ptrPolygon;
	float        t_f;

	osassert( _pipe );
	osassert( _camptr );

	// 
	// 如果当前的水不在camera内，则不渲染。
	if( m_iXSize > m_iZSize )
		t_f = m_iXSize*WATER_TILESIZE;
	else
		t_f = m_iZSize*WATER_TILESIZE;
	os_ViewFrustum* t_frus = ((osc_camera*)_camptr)->get_frustum();
	if( !t_frus->objInVF( m_vec3CPos,t_f ) )
		return true;

	//
	// 设置水的世界坐标。　
	osMatrix   t_mat;
	osMatrixTranslation( &t_mat,m_vec3CPos.x,m_vec3CPos.y,m_vec3CPos.z );
	_pipe->set_worldMatrix( t_mat );

	// 
	// 先动画纹理。
	frame_moveWater( _etime );

	for( int t_z=0;t_z<m_iZSize;t_z ++ )
	{
		for( int t_x=0;t_x<m_iXSize;t_x ++ )
		{
			t_idx = t_z*m_iXSize+t_x;
			t_ptrPolygon = &m_arrPolygon[t_idx];

			//! 使用我们维护的数组中的ShaderId.
			osassert( m_vecShaderId[t_idx] < m_iFrameNum );
			t_ptrPolygon->m_iShaderId = 
				m_vecDevdepShaderId[m_vecShaderId[t_idx]];

			t_ptrPolygon->m_arrIdxData = t_wTriIdx;

			if( !_pipe->render_polygon( t_ptrPolygon ) )
				return false;
		}
	}

	return true;
	unguard;
}


//! 释放当前的水资源。
void osc_dwater::release_water( void )
{
	guard;

	//! 释放顶点数据。
	m_arrWVertex.clear();

	m_fCurFrameEleTime = 0.0f;
	m_strWTextureName[0] = NULL;
	m_fRotAgl = 0;
	m_vec3CPos.x = m_vec3CPos.y = m_vec3CPos.z = 0.0f;

	//! 释放设备相关的资源数据,即当前水面用到的纹理和shader资源。
	for( int t_i=0;t_i < m_iFrameNum;t_i ++ )
	{
		g_shaderMgr->release_shader( m_vecDevdepShaderId[t_i] );
		m_vecDevdepShaderId[t_i] = -1; 
	}

	m_bInuse =  false;
	

	unguard;
}


//! 使用传入的结构，创建水面数据。
bool osc_dwater::create_water( os_waterCreate& _wc,
					  osVec3D& _pos,float _rgl,osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	strcpy( m_strWTextureName,_wc.m_szWTexName );
	m_fMoveSpeed = _wc.m_fMoveSpeed;
	m_iXSize = _wc.m_iXSize;
	m_iZSize = _wc.m_iZSize;
	m_iWaterDir = _wc.m_iWaterDir;   
	m_iFrameNum = _wc.m_iFrameNum;
	m_fAlpha = _wc.m_fWAlpah; 

	m_vec3CPos = _pos;
	m_fRotAgl = _rgl;


	m_arrPolygon.resize( m_iXSize*m_iZSize );
	m_arrWVertex.resize( m_iXSize*m_iZSize*4 );

	// 得到水面的开始位置。
	get_waterStartPos();

	// 根据方向得到uv坐标。
	osVec2D      t_vec2Uv[4];
	get_uvFromWaterDir( t_vec2Uv );

	// 
	// 计算alpha值，填充默认的顶点数据。
	DWORD   t_dwAlpha;
	t_dwAlpha = 0xff;
	t_dwAlpha = DWORD(t_dwAlpha*m_fAlpha);
	t_dwAlpha <<= 24;
	t_dwAlpha |= 0x00ffffff;

	for( int t_i=0;t_i<m_iXSize*m_iZSize;t_i ++ )
	{
		m_arrWVertex[t_i*4+0].m_vecUv = t_vec2Uv[0];
		m_arrWVertex[t_i*4+1].m_vecUv = t_vec2Uv[1];
		m_arrWVertex[t_i*4+2].m_vecUv = t_vec2Uv[2];
		m_arrWVertex[t_i*4+3].m_vecUv = t_vec2Uv[3];

		for( int t_j=0;t_j<4;t_j++ )
		{
			m_arrWVertex[t_i*4+t_j].m_dwColor = t_dwAlpha;
			m_arrWVertex[t_i*4+t_j].m_vecPos.y = m_vec3WStart.y;
		}
	}

	init_rPolygon( _pipe );

	m_bInuse = true;


	return true;

	unguard;
}






////////////////////////////////////////////////////////////////////////////////////////////////////
//　　　场景中水资源相关的管理器。
// 
osc_dwaterMgr::osc_dwaterMgr()
{
	m_ptrMPipe = NULL;
	m_vecWater.resize( MAX_WATERINSCE );

	m_iLastRenderId = -1;
}

osc_dwaterMgr::~osc_dwaterMgr()
{

}


//! 初始化我们的水特效管理器。
void osc_dwaterMgr::init_dwaterMgr( osc_middlePipe* _pipe )
{
	guard;
	
	osassert( _pipe );
	m_ptrMPipe = _pipe;

	unguard;
}

# if __WATER_EDITOR__
//! 从参数创建一个水面数据。
int osc_dwaterMgr::create_dwater( os_waterCreate& _wc,osVec3D& _pos,float _rgl/* = 0.0f*/ )
{
	guard;

	int    t_idx = -1;

	for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
	{
		if( m_vecWater[t_i].m_bInuse )
			continue;

		t_idx = t_i;
		break;
	}

	// 
	// 找到上一帧没有渲染的水面id,释放这个水面，调入新的水面。
	if( t_idx == -1 )
	{
		for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
		{
			if( t_i != m_iLastRenderId )
			{
				t_idx = t_i;
				m_vecWater[t_i].release_water();
				break;
			}
		}
	}

	// 使用传入的结构，创建水面数据。
	if( !m_vecWater[t_idx].create_water( _wc,_pos,0.0f,m_ptrMPipe ) )
		osassert( false );

	return t_idx;	

	unguard;
}

# endif 



//! 创建一个水面数据。
int osc_dwaterMgr::create_dwater( const char* _fname,osVec3D& _pos,float _rgl )
{
	guard;

	int    t_idx = -1;

	for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
	{
		if( m_vecWater[t_i].m_bInuse )
			continue;

		t_idx = t_i;
		break;
	}

	// 
	// 找到上一帧没有渲染的水面id,释放这个水面，调入新的水面。
	if( t_idx == -1 )
	{
		for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
		{
			if( t_i != m_iLastRenderId )
			{
				t_idx = t_i;
				m_vecWater[t_i].release_water();
				break;
			}
		}
	}

	if( !m_vecWater[t_idx].load_waterFromfile( _fname,_pos,0.0f,m_ptrMPipe ) )
		osassert( false );

	return t_idx;

	unguard;
}


//! 渲染当前的水特效管理器。
bool osc_dwaterMgr::render_dWater( I_camera*  _camptr )
{
	guard;

	float   t_fEtime;

	osassert( _camptr );

	// 先设置顶点alpha和alpha的方式。
	m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1 );
	m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
	m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA  );
	m_ptrMPipe->set_renderState( D3DRS_ZWRITEENABLE,true );
	m_ptrMPipe->set_renderState( D3DRS_ZENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ZWRITEENABLE,FALSE );
	
	m_ptrMPipe->set_vertexShader( NULL );

	t_fEtime = sg_timer::Instance()->get_lastelatime();


	for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
	{
		if( !m_vecWater[t_i].m_bInuse )
			continue;

		m_vecWater[t_i].render_water( _camptr,t_fEtime,m_ptrMPipe );
		m_iLastRenderId  = t_i;
	}

	return true;

	unguard;
}

//! 从水面管理器中删除一个水面。
void osc_dwaterMgr::delete_dWater( int _id )
{
	guard;

	osassert( (_id>=0)&&(_id<MAX_WATERINSCE) );
	osassert( m_vecWater[_id].m_bInuse );

	m_vecWater[_id].release_water();

	unguard;
}

# endif //弃用此文件。




/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*\
|*                 新的water类         snowwin create @ 2006-1-3                   *|
\*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


//! 定义一个水面格子的大小。
# define WATER_TILESIZE   6.0f


//! 定义shader对应的文件。
//  水的shader定义文件和公告板使用同一个fx文件。

//  每一个水的实例都必须创建在原点，然后使用当前水实例的位置
//  和旋转方向，在场景中得到正确放置的水。

osc_water::osc_water()
{
	m_szShaderIni[0] = NULL;
	m_szShaderName[0] = NULL;
	m_vec3CPos.x = m_vec3CPos.y = m_vec3CPos.z = 0.0f;

	m_bInuse = false;
}

osc_water::~osc_water()
{
}



//! 根据方向的不同得到格子用到的uv坐标。
void osc_water::get_uvFromWaterDir( osVec2D* _uv )
{
	guard;

	osassert( _uv );

	// 按顺时针方向。
	switch( m_iWaterDir )
	{
	case 1:
		_uv[0] = osVec2D( 0.0f,0.0f );
		_uv[1] = osVec2D( 1.0f,0.0f );
		_uv[2] = osVec2D( 1.0f,1.0f );
		_uv[3] = osVec2D( 0.0f,1.0f );
		break;
	case 2:
		_uv[0] = osVec2D( 0.0f,1.0f );
		_uv[1] = osVec2D( 0.0f,0.0f );
		_uv[2] = osVec2D( 1.0f,0.0f );
		_uv[3] = osVec2D( 1.0f,1.0f );
		break;
	case 3:
		_uv[0] = osVec2D( 1.0f,1.0f );
		_uv[1] = osVec2D( 0.0f,1.0f );
		_uv[2] = osVec2D( 0.0f,0.0f );
		_uv[3] = osVec2D( 1.0f,0.0f );
		break;
	case 4:
		_uv[0] = osVec2D( 1.0f,0.0f );
		_uv[1] = osVec2D( 1.0f,1.0f );
		_uv[2] = osVec2D( 0.0f,1.0f );
		_uv[3] = osVec2D( 0.0f,0.0f );
		break;
	}
	return;

	unguard;
}



//! 填充格子水面的顶点。
void osc_water::fill_waterTileVer( int _x,int _y,os_verWater* _ver )
{
	guard;

	osassert( _ver );
	osassert( (_x >=0) &&(_y >=0) );

	// 顶点从左上角开始顺时针排列。


	_ver[0].m_vecPos.x =  _x   * WATER_TILESIZE  + m_vec3WStart.x;
	_ver[0].m_vecPos.z = (_y+1) * WATER_TILESIZE + m_vec3WStart.z;

	_ver[1].m_vecPos.x = (_x+1) * WATER_TILESIZE + m_vec3WStart.x;;
	_ver[1].m_vecPos.z = (_y+1) * WATER_TILESIZE + m_vec3WStart.z;;

	_ver[2].m_vecPos.x = (_x+1) * WATER_TILESIZE + m_vec3WStart.x;;
	_ver[2].m_vecPos.z = (_y)   * WATER_TILESIZE + m_vec3WStart.z;

	_ver[3].m_vecPos.x = _x * WATER_TILESIZE + m_vec3WStart.x;
	_ver[3].m_vecPos.z = _y * WATER_TILESIZE + m_vec3WStart.z;

	return;

	unguard;
}


//! 根据我们的数据初始化我们的polygon.
void osc_water::init_rPolygon( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	os_polygon*   t_ptrPolygon;
	int           t_idx;
	static WORD  t_wTriIdx[6] = { 0,1,2,0,2,3 };

	// 先创建水面用到的shader和id
	m_iShaderId = g_shaderMgr->add_shader( m_szShaderName, osc_engine::get_shaderFile( "object" ).c_str() );
	if( m_iShaderId < 0 )
		m_iShaderId = g_shaderMgr->add_shadertomanager( m_szShaderName, m_szShaderIni );


	// 使用顶点alpha，使边上水有alpha效果。
	// 填充每一个水格子用到的polygon.
	for( int t_y=0;t_y<m_iZSize;t_y++ )
	{
		for( int t_x=0;t_x<m_iXSize;t_x++ )
		{
			t_idx = t_y*m_iXSize+t_x;
			t_ptrPolygon = &m_arrPolygon[t_idx];

			//t_ptrPolygon->m_arrVerData = &m_arrWVertex[t_idx*4];
			//t_ptrPolygon->m_arrIdxData = t_wTriIdx;
			//t_ptrPolygon->m_iVertexStart = 0;
			//t_ptrPolygon->m_iIbufId = -1;
			//t_ptrPolygon->m_bUseStaticVB = false;
			t_ptrPolygon->set_rawData( &m_arrWVertex[t_idx*4], t_wTriIdx );

			t_ptrPolygon->m_iPriNum = 2;
			t_ptrPolygon->m_iPriType = TRIANGLELIST;
			t_ptrPolygon->m_iVerSize = sizeof( os_verWater );
			t_ptrPolygon->m_iVerNum = 4;
			t_ptrPolygon->m_iShaderId = m_iShaderId;

			// 填充顶点相关。
			fill_waterTileVer( t_x,t_y,	(os_verWater *)t_ptrPolygon->m_sRawData.m_arrVerData );

			m_cPolySet.add_polygon( t_ptrPolygon );
		}
	}

	return ;

	unguard;
}


//! 从水面的中心点得到水面的起始点位置。
void osc_water::get_waterStartPos( void )
{
	guard;

	float   t_f;

	t_f = m_iXSize/2.0f;
	t_f *= WATER_TILESIZE;
	m_vec3WStart.x = - t_f;

	t_f = m_iZSize/2.0f;
	t_f *= WATER_TILESIZE;
	m_vec3WStart.z = - t_f;

	m_vec3WStart.y = 0.0f;

	return;

	unguard;
}



DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

//! 渲染当前的水面。
bool osc_water::render_water( I_camera*  _camptr, osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );
	osassert( _camptr );

	float        t_f;

	// 如果当前的水不在camera内，则不渲染。
	if( m_iXSize > m_iZSize )
		t_f = m_iXSize*WATER_TILESIZE;
	else
		t_f = m_iZSize*WATER_TILESIZE;
	os_ViewFrustum* t_frus = ((osc_camera*)_camptr)->get_frustum();
	if( !t_frus->objInVF( m_vec3CPos,t_f ) )
		return true;

	// 设置水的世界坐标。　
	osMatrix   t_mat;
	osMatrixTranslation( &t_mat,m_vec3CPos.x,m_vec3CPos.y,m_vec3CPos.z );
	_pipe->set_worldMatrix( t_mat );

	//for( int t_z=0;t_z<m_iZSize;t_z ++ )
	//{
	//	for( int t_x=0;t_x<m_iXSize;t_x ++ )
	//	{
	//		if( !_pipe->render_polygon( &(m_arrPolygon[t_z*m_iXSize+t_x]) ) )
	//			return false;
	//	}
	//}
	if( !_pipe->render_andClearPolygonSet( &m_cPolySet ) )return false;

	return true;
	unguard;
}


//! 释放当前的水资源。
void osc_water::release_water( void )
{
	guard;

	//! 释放顶点数据。
	m_arrWVertex.clear();

	m_vec3CPos.x = m_vec3CPos.y = m_vec3CPos.z = 0.0f;

	//! 释放设备相关的资源数据,即当前水面用到的纹理和shader资源。
	g_shaderMgr->release_shader( m_iShaderId );
	m_iShaderId = -1;


	m_bInuse =  false;
	

	unguard;
}


//! 使用传入的结构，创建水面数据。
bool osc_water::create_water( os_water& _wc, osVec3D& _pos, osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	strcpy( this->m_szShaderIni, _wc.m_szShaderIni );
	strcpy( this->m_szShaderName, _wc.m_szShaderName );
	m_iXSize = _wc.m_iXSize;
	m_iZSize = _wc.m_iZSize;
	m_iWaterDir = _wc.m_iDir;   
	m_fAlpha = _wc.m_fAlpha; 

	m_vec3CPos = _pos;

	m_arrPolygon.resize( m_iXSize*m_iZSize );
	m_arrWVertex.resize( m_iXSize*m_iZSize*4 );

	// 得到水面的开始位置。
	get_waterStartPos();

	// 根据方向得到uv坐标。
	osVec2D      t_vec2Uv[4];
	get_uvFromWaterDir( t_vec2Uv );

	// 计算alpha值，填充默认的顶点数据。
	DWORD   t_dwAlpha;
	t_dwAlpha = 0xff;
	t_dwAlpha = DWORD(t_dwAlpha*m_fAlpha);
	t_dwAlpha <<= 24;
	t_dwAlpha |= 0x00ffffff;

	for( int t_i=0;t_i<m_iXSize*m_iZSize;t_i ++ )
	{
		m_arrWVertex[t_i*4+0].m_vecUv = t_vec2Uv[0];
		m_arrWVertex[t_i*4+1].m_vecUv = t_vec2Uv[1];
		m_arrWVertex[t_i*4+2].m_vecUv = t_vec2Uv[2];
		m_arrWVertex[t_i*4+3].m_vecUv = t_vec2Uv[3];

		for( int t_j=0;t_j<4;t_j++ )
		{
			m_arrWVertex[t_i*4+t_j].m_dwColor = t_dwAlpha;
			m_arrWVertex[t_i*4+t_j].m_vecPos.y = m_vec3WStart.y;
		}
	}

	init_rPolygon( _pipe );

	m_bInuse = true;

	return true;

	unguard;
}

//////////////////////////////////////////////////////////////////////

/// 场景中水资源相关的管理器。
osc_waterMgr::osc_waterMgr()
{
	m_ptrMPipe = NULL;
	m_vecWater.resize( MAX_WATERINSCE );

	m_iLastRenderId = -1;
}

osc_waterMgr::~osc_waterMgr()
{
}


//! 初始化我们的水特效管理器。
void osc_waterMgr::init_waterMgr( osc_middlePipe* _pipe )
{
	guard;
	
	osassert( _pipe );
	m_ptrMPipe = _pipe;

	unguard;
}

//! 从参数创建一个水面数据。
int osc_waterMgr::create_water( os_water& _wc,osVec3D& _pos,float _rgl/* = 0.0f*/ )
{
	guard;

	int    t_idx = -1;

	for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
	{
		if( m_vecWater[t_i].m_bInuse )
			continue;

		t_idx = t_i;
		break;
	}

	// 找到上一帧没有渲染的水面id,释放这个水面，调入新的水面。
	if( t_idx == -1 )
	{
		for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
		{
			if( t_i != m_iLastRenderId )
			{
				t_idx = t_i;
				m_vecWater[t_i].release_water();
				break;
			}
		}
	}

	// 使用传入的结构，创建水面数据。
	if( !m_vecWater[t_idx].create_water( _wc, _pos, m_ptrMPipe ) )
		osassert( false );

	return t_idx;	

	unguard;
}


//! 渲染当前的水特效管理器。
bool osc_waterMgr::render_water( I_camera*  _camptr )
{
	guard;

	osassert( _camptr );
	float   t_fTime = sg_timer::Instance()->get_talelatime();

	FLOAT r = 0.8f;
	static D3DXMATRIXA16 t_matBumpMat;
	t_matBumpMat._11 =  r * cosf( t_fTime * 0.1f );
	t_matBumpMat._12 = -r * sinf( t_fTime * 0.1f );
	t_matBumpMat._21 =  r * sinf( t_fTime * 0.1f );
	t_matBumpMat._22 =  r * cosf( t_fTime * 0.1f );
	m_ptrMPipe->set_textureStageState( 0, D3DTSS_BUMPENVMAT00,   F2DW( t_matBumpMat._11 ) );
	m_ptrMPipe->set_textureStageState( 0, D3DTSS_BUMPENVMAT01,   F2DW( t_matBumpMat._12 ) );
	m_ptrMPipe->set_textureStageState( 0, D3DTSS_BUMPENVMAT10,   F2DW( t_matBumpMat._21 ) );
	m_ptrMPipe->set_textureStageState( 0, D3DTSS_BUMPENVMAT11,   F2DW( t_matBumpMat._22 ) );
	m_ptrMPipe->set_textureStageState( 0, D3DTSS_BUMPENVLSCALE,  F2DW(1.0f) );
	m_ptrMPipe->set_textureStageState( 0, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f) );


	for( int t_i=0;t_i<MAX_WATERINSCE;t_i ++ )
	{
		if( !m_vecWater[t_i].m_bInuse )
			continue;

		m_vecWater[t_i].render_water( _camptr, m_ptrMPipe );
		m_iLastRenderId  = t_i;
	}

	m_ptrMPipe->set_textureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_ptrMPipe->set_textureStageState( 1, D3DTSS_ALPHAOP,D3DTOP_DISABLE );

	return true;

	unguard;
}

//! 从水面管理器中删除一个水面。
void osc_waterMgr::delete_water( int _id )
{
	guard;

	osassert( (_id>=0)&&(_id<MAX_WATERINSCE) );
	osassert( m_vecWater[_id].m_bInuse );

	m_vecWater[_id].release_water();

	unguard;
}

