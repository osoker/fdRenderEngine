//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdWater.h
 *
 *  Desc:     ������ˮ��ص���Ч�Ĵ���
 *
 *  His:      River created @ 2004-5-13.
 *
 *  "��������˵��������ױ������Լ��ĵ�λ"��( �����ά�� )
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdWater.h"
# include "../../backpipe/include/osCamera.h"
# include "../../mfpipe/include/osShader.h"
# include "../../backpipe/include/mainEngine.h"

# if 0  //���ô��ļ���

//! ����һ��ˮ����ӵĴ�С��
# define WATER_TILESIZE   6.0f


//@{
//! ����ˮ��ص�ini.
# define WATER_SECTOR  "water"
# define WTEX_NAME     "tname"
# define MOVE_SPEED    "movespeed"
# define X_SIZE        "xsize"
# define Z_SIZE        "zsize"
# define WATER_DIR     "dir"
# define FRAME_NUM     "framenum"
# define WATER_ALPHA   "alpha"
//@}





//! ����shader��Ӧ���ļ���
//  ˮ��shader�����ļ��͹����ʹ��ͬһ��fx�ļ���

//  ÿһ��ˮ��ʵ�������봴����ԭ�㣬Ȼ��ʹ�õ�ǰˮʵ����λ��
//  ����ת�����ڳ����еõ���ȷ���õ�ˮ��

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



//! ���ݷ���Ĳ�ͬ�õ������õ���uv���ꡣ
void osc_dwater::get_uvFromWaterDir( osVec2D* _uv )
{
	guard;

	osassert( _uv );

	// ��˳ʱ�뷽��
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



//! ������ˮ��Ķ��㡣
void osc_dwater::fill_waterTileVer( int _x,int _y,os_verWater* _ver )
{
	guard;

	osassert( _ver );
	osassert( (_x >=0) &&(_y >=0) );

	// ��������Ͻǿ�ʼ˳ʱ�����С�


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

//! ��ʼ��ˮ���shader id.
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

//! ÿһ֡����ˮ�Ķ�����ͼ�仯��
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
				// ʹ��������Ǽ���
				m_vecShaderId[t_idx] %= m_iFrameNum;
			}

		}
	}

	return;

	unguard;
}


//! �������ǵ����ݳ�ʼ�����ǵ�polygon.
void osc_dwater::init_rPolygon( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	os_polygon*   t_ptrPolygon;
	int           t_idx;
	char          t_szShaName[256];

	// �ȴ���ˮ���õ���shader��id
	// ʹ��billboard��fx�ļ�����Ⱦ״̬��Ҫͼ�ӵ�����
	osassert( m_iFrameNum <= MAX_WATERFRAME );
	for( int t_i=0;t_i<this->m_iFrameNum;t_i ++ )
	{
		sprintf( t_szShaName,"%s%02d",m_strWTextureName,t_i );
		m_vecDevdepShaderId[t_i] = g_shaderMgr->add_shader( t_szShaName, osc_engine::get_shaderFile( "billBoard" ).c_str() );
		if( m_vecDevdepShaderId[t_i] < 0 )
			m_vecDevdepShaderId[t_i] = g_shaderMgr->add_shadertomanager( t_szShaName,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
	}

	// ʹ�ö���alpha��ʹ����ˮ��alphaЧ����
	// ���ÿһ��ˮ�����õ���polygon.
	for( int t_y=0;t_y<m_iZSize;t_y++ )
	{
		for( int t_x=0;t_x<m_iXSize;t_x++ )
		{
			t_idx = t_y*m_iXSize+t_x;
			t_ptrPolygon = &m_arrPolygon[t_idx];
			
			// ָ��ÿһ��polygon��shaderID.
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

			// ��䶥����ء�
			fill_waterTileVer( t_x,t_y,
				(os_verWater *)t_ptrPolygon->m_arrVerData );

		}
	}

	return ;

	unguard;
}


//! ��ˮ������ĵ�õ�ˮ�����ʼ��λ�á�
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


//! ��ini�ļ��ж���ˮ��������á�
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



//! ��ˮ���ļ��ж������ݡ�
bool osc_dwater::read_waterFile( const char* _wname )
{
	guard;

	BYTE*             t_start;
	int               t_size;
	os_waterCreate     t_wCreate;
	WATER_FILE_HEAD  t_header;

	osassert( _wname&&(_wname[0]) );

	// �����ļ����ڴ档
	int t_iGBufIdx = -1;
	t_start = START_USEGBUF( t_iGBufIdx );
	t_size = (DWORD)::read_fileToBuf( (char*)_wname,t_start,TMP_BUFSIZE );
	osassert( t_size > 0 );
	

	// ����ˮ���ļ����ݡ�
	READ_MEM_OFF( &t_header,t_start,sizeof( WATER_FILE_HEAD ) );
	READ_MEM_OFF( &t_wCreate,t_start,sizeof( os_waterCreate ) );

	// ��ˮ��������ݡ�
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



//  ���Ժ�����
//! ���ļ��е��뵥����ˮ�ļ���
bool osc_dwater::load_waterFromfile( const char* _fname,
									osVec3D& _pos,float _rgl,osc_middlePipe* _pipe  )
{
	guard;

	osassert( _fname&&(_fname[0]) );
	osassert( _pipe );


	// 
	// ����ˮ���ļ���
	if( !read_waterFile( _fname ) )
		return false;


	m_vec3CPos = _pos;
	m_fRotAgl = _rgl;


	m_arrPolygon.resize( m_iXSize*m_iZSize );
	m_vecShaderId.resize( m_iXSize*m_iZSize );
	m_arrWVertex.resize( m_iXSize*m_iZSize*4 );

	// �õ�ˮ��Ŀ�ʼλ�á�
	get_waterStartPos();

	// ���ݷ���õ�uv���ꡣ
	osVec2D      t_vec2Uv[4];
	get_uvFromWaterDir( t_vec2Uv );

	// 
	// ����alphaֵ�����Ĭ�ϵĶ������ݡ�
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


//! ��Ⱦ��ǰ��ˮ�档
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
	// �����ǰ��ˮ����camera�ڣ�����Ⱦ��
	if( m_iXSize > m_iZSize )
		t_f = m_iXSize*WATER_TILESIZE;
	else
		t_f = m_iZSize*WATER_TILESIZE;
	os_ViewFrustum* t_frus = ((osc_camera*)_camptr)->get_frustum();
	if( !t_frus->objInVF( m_vec3CPos,t_f ) )
		return true;

	//
	// ����ˮ���������ꡣ��
	osMatrix   t_mat;
	osMatrixTranslation( &t_mat,m_vec3CPos.x,m_vec3CPos.y,m_vec3CPos.z );
	_pipe->set_worldMatrix( t_mat );

	// 
	// �ȶ�������
	frame_moveWater( _etime );

	for( int t_z=0;t_z<m_iZSize;t_z ++ )
	{
		for( int t_x=0;t_x<m_iXSize;t_x ++ )
		{
			t_idx = t_z*m_iXSize+t_x;
			t_ptrPolygon = &m_arrPolygon[t_idx];

			//! ʹ������ά���������е�ShaderId.
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


//! �ͷŵ�ǰ��ˮ��Դ��
void osc_dwater::release_water( void )
{
	guard;

	//! �ͷŶ������ݡ�
	m_arrWVertex.clear();

	m_fCurFrameEleTime = 0.0f;
	m_strWTextureName[0] = NULL;
	m_fRotAgl = 0;
	m_vec3CPos.x = m_vec3CPos.y = m_vec3CPos.z = 0.0f;

	//! �ͷ��豸��ص���Դ����,����ǰˮ���õ��������shader��Դ��
	for( int t_i=0;t_i < m_iFrameNum;t_i ++ )
	{
		g_shaderMgr->release_shader( m_vecDevdepShaderId[t_i] );
		m_vecDevdepShaderId[t_i] = -1; 
	}

	m_bInuse =  false;
	

	unguard;
}


//! ʹ�ô���Ľṹ������ˮ�����ݡ�
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

	// �õ�ˮ��Ŀ�ʼλ�á�
	get_waterStartPos();

	// ���ݷ���õ�uv���ꡣ
	osVec2D      t_vec2Uv[4];
	get_uvFromWaterDir( t_vec2Uv );

	// 
	// ����alphaֵ�����Ĭ�ϵĶ������ݡ�
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
//������������ˮ��Դ��صĹ�������
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


//! ��ʼ�����ǵ�ˮ��Ч��������
void osc_dwaterMgr::init_dwaterMgr( osc_middlePipe* _pipe )
{
	guard;
	
	osassert( _pipe );
	m_ptrMPipe = _pipe;

	unguard;
}

# if __WATER_EDITOR__
//! �Ӳ�������һ��ˮ�����ݡ�
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
	// �ҵ���һ֡û����Ⱦ��ˮ��id,�ͷ����ˮ�棬�����µ�ˮ�档
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

	// ʹ�ô���Ľṹ������ˮ�����ݡ�
	if( !m_vecWater[t_idx].create_water( _wc,_pos,0.0f,m_ptrMPipe ) )
		osassert( false );

	return t_idx;	

	unguard;
}

# endif 



//! ����һ��ˮ�����ݡ�
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
	// �ҵ���һ֡û����Ⱦ��ˮ��id,�ͷ����ˮ�棬�����µ�ˮ�档
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


//! ��Ⱦ��ǰ��ˮ��Ч��������
bool osc_dwaterMgr::render_dWater( I_camera*  _camptr )
{
	guard;

	float   t_fEtime;

	osassert( _camptr );

	// �����ö���alpha��alpha�ķ�ʽ��
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

//! ��ˮ���������ɾ��һ��ˮ�档
void osc_dwaterMgr::delete_dWater( int _id )
{
	guard;

	osassert( (_id>=0)&&(_id<MAX_WATERINSCE) );
	osassert( m_vecWater[_id].m_bInuse );

	m_vecWater[_id].release_water();

	unguard;
}

# endif //���ô��ļ���




/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*\
|*                 �µ�water��         snowwin create @ 2006-1-3                   *|
\*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


//! ����һ��ˮ����ӵĴ�С��
# define WATER_TILESIZE   6.0f


//! ����shader��Ӧ���ļ���
//  ˮ��shader�����ļ��͹����ʹ��ͬһ��fx�ļ���

//  ÿһ��ˮ��ʵ�������봴����ԭ�㣬Ȼ��ʹ�õ�ǰˮʵ����λ��
//  ����ת�����ڳ����еõ���ȷ���õ�ˮ��

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



//! ���ݷ���Ĳ�ͬ�õ������õ���uv���ꡣ
void osc_water::get_uvFromWaterDir( osVec2D* _uv )
{
	guard;

	osassert( _uv );

	// ��˳ʱ�뷽��
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



//! ������ˮ��Ķ��㡣
void osc_water::fill_waterTileVer( int _x,int _y,os_verWater* _ver )
{
	guard;

	osassert( _ver );
	osassert( (_x >=0) &&(_y >=0) );

	// ��������Ͻǿ�ʼ˳ʱ�����С�


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


//! �������ǵ����ݳ�ʼ�����ǵ�polygon.
void osc_water::init_rPolygon( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	os_polygon*   t_ptrPolygon;
	int           t_idx;
	static WORD  t_wTriIdx[6] = { 0,1,2,0,2,3 };

	// �ȴ���ˮ���õ���shader��id
	m_iShaderId = g_shaderMgr->add_shader( m_szShaderName, osc_engine::get_shaderFile( "object" ).c_str() );
	if( m_iShaderId < 0 )
		m_iShaderId = g_shaderMgr->add_shadertomanager( m_szShaderName, m_szShaderIni );


	// ʹ�ö���alpha��ʹ����ˮ��alphaЧ����
	// ���ÿһ��ˮ�����õ���polygon.
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

			// ��䶥����ء�
			fill_waterTileVer( t_x,t_y,	(os_verWater *)t_ptrPolygon->m_sRawData.m_arrVerData );

			m_cPolySet.add_polygon( t_ptrPolygon );
		}
	}

	return ;

	unguard;
}


//! ��ˮ������ĵ�õ�ˮ�����ʼ��λ�á�
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

//! ��Ⱦ��ǰ��ˮ�档
bool osc_water::render_water( I_camera*  _camptr, osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );
	osassert( _camptr );

	float        t_f;

	// �����ǰ��ˮ����camera�ڣ�����Ⱦ��
	if( m_iXSize > m_iZSize )
		t_f = m_iXSize*WATER_TILESIZE;
	else
		t_f = m_iZSize*WATER_TILESIZE;
	os_ViewFrustum* t_frus = ((osc_camera*)_camptr)->get_frustum();
	if( !t_frus->objInVF( m_vec3CPos,t_f ) )
		return true;

	// ����ˮ���������ꡣ��
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


//! �ͷŵ�ǰ��ˮ��Դ��
void osc_water::release_water( void )
{
	guard;

	//! �ͷŶ������ݡ�
	m_arrWVertex.clear();

	m_vec3CPos.x = m_vec3CPos.y = m_vec3CPos.z = 0.0f;

	//! �ͷ��豸��ص���Դ����,����ǰˮ���õ��������shader��Դ��
	g_shaderMgr->release_shader( m_iShaderId );
	m_iShaderId = -1;


	m_bInuse =  false;
	

	unguard;
}


//! ʹ�ô���Ľṹ������ˮ�����ݡ�
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

	// �õ�ˮ��Ŀ�ʼλ�á�
	get_waterStartPos();

	// ���ݷ���õ�uv���ꡣ
	osVec2D      t_vec2Uv[4];
	get_uvFromWaterDir( t_vec2Uv );

	// ����alphaֵ�����Ĭ�ϵĶ������ݡ�
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

/// ������ˮ��Դ��صĹ�������
osc_waterMgr::osc_waterMgr()
{
	m_ptrMPipe = NULL;
	m_vecWater.resize( MAX_WATERINSCE );

	m_iLastRenderId = -1;
}

osc_waterMgr::~osc_waterMgr()
{
}


//! ��ʼ�����ǵ�ˮ��Ч��������
void osc_waterMgr::init_waterMgr( osc_middlePipe* _pipe )
{
	guard;
	
	osassert( _pipe );
	m_ptrMPipe = _pipe;

	unguard;
}

//! �Ӳ�������һ��ˮ�����ݡ�
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

	// �ҵ���һ֡û����Ⱦ��ˮ��id,�ͷ����ˮ�棬�����µ�ˮ�档
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

	// ʹ�ô���Ľṹ������ˮ�����ݡ�
	if( !m_vecWater[t_idx].create_water( _wc, _pos, m_ptrMPipe ) )
		osassert( false );

	return t_idx;	

	unguard;
}


//! ��Ⱦ��ǰ��ˮ��Ч��������
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

//! ��ˮ���������ɾ��һ��ˮ�档
void osc_waterMgr::delete_water( int _id )
{
	guard;

	osassert( (_id>=0)&&(_id<MAX_WATERINSCE) );
	osassert( m_vecWater[_id].m_bInuse );

	m_vecWater[_id].release_water();

	unguard;
}

