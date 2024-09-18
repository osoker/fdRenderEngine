//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdEleBolt.cpp
 *
 *  Desc:     场景中产生电弧效果的特交.
 *
 *  His:      River created @ 2005-4-18
 *
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdEleBolt.h"
# include "../../backpipe/include/mainEngine.h"

# define BOLT_TEXTURENAME    "bolt"

static  int             logBase2(int n)
{
	int     i = 0;

	osassert(n != 0);

	while   (!(n & 1))
	{
		n = n >> 1;
		i++;
	}

	assert((n & ~1) == 0);

	return i;
}


static  float GaussRand(void)
{
	int     i;
	int     r;

	r = 0;

	for(i = 0; i < 6; i++)
		r = r + OSRAND - OSRAND;

	return (float)r / ((float)RAND_MAX * 6.0f);
}


//! bolt用到的顶点缓冲区Id和索引缓冲区Id
int osc_eleBolt::m_iVerbufId = -1;
int osc_eleBolt::m_iIdxbufId = -1;

osc_eleBolt::osc_eleBolt()
{
	m_bReadyRender = false;

	m_vec3CurrentPoint = NULL;

}

osc_eleBolt::~osc_eleBolt()
{
}



int osc_eleBolt::get_dominantColor( osColor& _color )
{
	float    t_fMax;

	t_fMax = max( _color.r,_color.g );
	t_fMax = max( _color.b,t_fMax );

	if( float_equal( t_fMax,_color.r ) )
		return ELECTRIC_BOLT_REDDOMINANT;
	else if( float_equal( t_fMax,_color.g ) )
		return ELECTRIC_BOLT_GREENDOMINANT;
	else if( float_equal( t_fMax,_color.b ) )
		return ELECTRIC_BOLT_BLUEDOMINANT;
	else
		osassertex( false,"得到eleBolt的dominantColor失败" );

	return -1;

}


void osc_eleBolt::elebolt_setcolor( osColor& _color )
{
	guard;

	m_sCurrentColor = _color;

	m_arrBaseColors[0] = _color.r*255;
	m_arrBaseColors[1] = _color.g*255;
	m_arrBaseColors[2] = _color.b*255;

	m_vecCurrentColors[0] = _color.r*255;
	m_vecCurrentColors[1] = _color.g*255;
	m_vecCurrentColors[2] = _color.b*255;
	
	m_iDominantColor = get_dominantColor( _color );

	unguard;
}


BOOL osc_eleBolt::elebolt_init( os_boltInit* _init  )
{
	guard;

	osassert( _init );
	osColor   color;

	osassert( (_init->m_fWildness>=0)&&(_init->m_fWildness<=1.0f) );
	osassert( logBase2(_init->m_iNumPt) );
	osassert( !vec3_equal( _init->m_vec3Start,_init->m_vec3End )  );

	this->m_fLastChangeTime = 0;
	this->m_fLastBoltTime = 0;
	m_bInitialized = false;


	m_dwNumPoints = _init->m_iNumPt;
	if( m_dwNumPoints > MAXBOLT_POLY )
		m_dwNumPoints = MAXBOLT_POLY;

	m_fWildness = _init->m_fWildness;
	m_fWidth = _init->m_fWidth;

	color = _init->m_dwColor;
	elebolt_setcolor( color );

	m_vec3Start = _init->m_vec3Start;
	m_vec3End = _init->m_vec3End;

	m_fFreqTime = 1.0f/float(_init->m_iBoltFreq);

	// 
	// init the render poly data struct.
	if( m_sPoly.m_iShaderId < 0 )
	{
		m_sPoly.m_iShaderId = g_shaderMgr->add_shader( 
			BOLT_TEXTURENAME, osc_engine::get_shaderFile( "billBoard" ).c_str() );
		if( m_sPoly.m_iShaderId < 0 )
		{
			m_sPoly.m_iShaderId = g_shaderMgr->add_shadertomanager( 
				BOLT_TEXTURENAME,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
		}

		//m_sPoly.m_arrIdxData = new WORD[6*_init->m_iNumPt];
		//m_sPoly.m_arrVerData = new os_billBoardVertex[4*_init->m_iNumPt];
		m_sPoly.m_iPriType = TRIANGLELIST;
		m_sPoly.m_bSelfReleaseVI = true;
		m_sPoly.m_bUseStaticVB = true;
		m_sPoly.m_sBufData.m_iMiniIdx = 0;
		m_sPoly.m_iVerNum = 4*_init->m_iNumPt;
		m_sPoly.m_iPriNum = 2*_init->m_iNumPt;
		m_sPoly.m_iVerSize = sizeof( os_billBoardVertex );

		//! 创建索引顶点缓冲区.
		m_sPoly.m_sBufData.m_iVbufId = osc_eleBolt::m_iVerbufId;
		osassert( m_sPoly.m_sBufData.m_iVbufId>=0 );
		m_sPoly.m_sBufData.m_iIbufId = osc_eleBolt::m_iIdxbufId;
		osassert( m_sPoly.m_sBufData.m_iIbufId>=0 );
	}

	m_bReadyRender = false;

	return TRUE;

	unguard;
}


// 重设电弧相关的属性。
void osc_eleBolt::elebolt_reset( os_boltInit* _init )
{
	guard;

	osassert( !vec3_equal( _init->m_vec3End,_init->m_vec3Start ) );

	this->m_vec3Start = _init->m_vec3Start;
	m_vec3End = _init->m_vec3End;

	unguard;
}




//========================================================================
//从我们计算的顶点中填充我们要渲染的polygon结构.
//
//算法描述:
//1:从cam得到lookvec.
//2:从end-start得到一个坐标.
//3:cross_product得到ele_bolt的向上坐标.
//4:对elebolt中每两个动画点:
//  使用向上的坐标构建一个四边形.
//
//========================================================================
void osc_eleBolt::elebolt_fillpoly( I_camera* _camptr )
{
	guard;

	os_billBoardVertex  *verdata = NULL;
	WORD               *indexdata = NULL;
	osVec3D            lookvec,upvec;
	int                tmpi,ptindex;

	osassert( _camptr );

	//从cam得到look vec.
	_camptr->get_godLookVec( lookvec );
	upvec = m_vec3End-m_vec3Start;
	osVec3Cross( &upvec,&upvec,&lookvec );
	osVec3Normalize( &upvec,&upvec );
	osVec3Scale( &upvec,&upvec,m_fWidth/2.0f );

	//get the poly data pointer to fill data
	verdata = m_arrRVertex;

	m_sCurrentColor.r = m_vecCurrentColors[0]/255.0f;
	m_sCurrentColor.g = m_vecCurrentColors[1]/255.0f;
	m_sCurrentColor.b = m_vecCurrentColors[2]/255.0f;
	m_sCurrentColor.a = 1.0f;


	//对每一个顶点操作,使之产生一个面向cam的面.
	for( tmpi=0;tmpi<this->m_dwNumPoints-1;tmpi++ )
	{
		//fill vertex data.
		ptindex = tmpi*4;
		verdata[ptindex+0].m_vec3Pos = m_vec3CenterPoints[tmpi]+upvec;
		verdata[ptindex+0].m_vec2Uv = osVec2D( 0,0 );
		verdata[ptindex+0].m_dwColor = m_sCurrentColor;

		verdata[ptindex+1].m_vec3Pos = m_vec3CenterPoints[tmpi+1]+upvec;
		verdata[ptindex+1].m_vec2Uv = osVec2D( 1,0 );
		verdata[ptindex+1].m_dwColor = m_sCurrentColor;

		verdata[ptindex+2].m_vec3Pos = m_vec3CenterPoints[tmpi+1]-upvec;
		verdata[ptindex+2].m_vec2Uv = osVec2D( 1,1 );
		verdata[ptindex+2].m_dwColor = m_sCurrentColor;

		verdata[ptindex+3].m_vec3Pos = m_vec3CenterPoints[tmpi]-upvec;
		verdata[ptindex+3].m_vec2Uv = osVec2D( 0,1 );
		verdata[ptindex+3].m_dwColor = m_sCurrentColor;
	}

	//over.

	unguard;
}

void osc_eleBolt::subdivide( const osVec3D* start,
						   const osVec3D* end,float s,int n )
{
	guard;
	osVec3D  tmp;

	osassert( start );
	osassert( end );
	osassert( n>=0 );


	if	(n == 0)
	{
		m_vec3CurrentPoint++;
		*m_vec3CurrentPoint = *end;

		return;
	}
	
	tmp.x = (end->x + start->x) / 2 + s * GaussRand();
	tmp.y = (end->y + start->y) / 2 + s * GaussRand();
	tmp.z = (end->z + start->z) / 2 + s * GaussRand();

	subdivide( start, &tmp, s / 2, n - 1);
	subdivide( &tmp,    end, s / 2, n - 1);

	unguard;
}


void osc_eleBolt::genLightning( int RangeLow,int RangeHigh,const osVec3D* start,
					 const osVec3D* end )
{
	guard;
	float	length;

	osassert(start);
	osassert(end);
	osassert(RangeHigh > RangeLow);
	osassert( ((RangeHigh - RangeLow)%2) == 0);

	/* Manhattan length is good enough for this */
	length = (float)(fabs(start->x - end->x) +
		             fabs(start->y - end->y) +
		             fabs(start->z - end->z) );
	
	m_vec3CurrentPoint					= &m_vec3CenterPoints[RangeLow];
	m_vec3CenterPoints[RangeLow]		= *start;
	m_vec3CenterPoints[RangeHigh] 		= *end;
	
	//	be->m_vec3CenterPoints[be->m_dwNumPoints] = *end;
	//	subdivide(be, start, end, length * be->m_fWildness, logBase2(be->m_dwNumPoints));
	subdivide( start, end, length * m_fWildness, logBase2(RangeHigh - RangeLow));

	unguard;
}


void osc_eleBolt::elebolt_animate( const osVec3D* start,const osVec3D* end )
{
	guard;

	float       eletime;
	int			dominant;
	int			nonDominant1;
	int			nonDominant2;
	osVec3D		SubdivideStart;
	osVec3D		SubdivideEnd;
	int			LowIndex;
	int			HighIndex;
		
	eletime = m_fLastBoltTime;
    
	//===========================================================
	//color process .
	//===========================================================
	dominant = m_iDominantColor;
	nonDominant1 = (dominant + 1) % 3;
	nonDominant2 = (dominant + 2) % 3;
	if	(m_arrBaseColors[nonDominant1] == m_vecCurrentColors[nonDominant1])
	{
		int	DecayRate;
		int	Spike;
	
		DecayRate = OSRAND % 5;
		DecayRate = max(DecayRate, 5);
		m_iDecayRate = DecayRate;
		if	( m_arrBaseColors[nonDominant1] >= 1.0f)
			Spike = OSRAND % (int)(m_arrBaseColors[nonDominant1]);
		else
			Spike = 0;
		m_vecCurrentColors[nonDominant1] -= Spike;
		m_vecCurrentColors[nonDominant2] -= Spike;
	}
	else
	{
		m_vecCurrentColors[nonDominant1] += m_iDecayRate;
		m_vecCurrentColors[nonDominant2] += m_iDecayRate;
		if	(m_vecCurrentColors[nonDominant1] > m_arrBaseColors[nonDominant1])
		{
			m_vecCurrentColors[nonDominant1] = m_arrBaseColors[nonDominant1];
			m_vecCurrentColors[nonDominant2] = m_arrBaseColors[nonDominant2];
		}
	}
	
	//===========================================================
	//point process.
	//===========================================================
	if( m_bInitialized&&(m_dwNumPoints>16) )
	{
#if 1
		int		P1;
		int		P2;
		int		P3;
		int		P4;
		
		switch	(OSRAND % 7)
		{
		case	0:
			genLightning( 0, m_dwNumPoints, start, end);
			return;
			
		case	1:
		case	2:
		case	3:
			P1 = 0;
			P2 = m_dwNumPoints / 2;
			P3 = P2 + m_dwNumPoints / 4;
			P4 = m_dwNumPoints;
			break;
			
		case	4:
		case	5:
		case	6:
			P1 = 0;
			P3 = m_dwNumPoints / 2;
			P2 = P3 - m_dwNumPoints / 4;
			P4 = m_dwNumPoints;
			break;
		}

		SubdivideStart = m_vec3CenterPoints[P1];
		SubdivideEnd = m_vec3CenterPoints[P2];
		genLightning( P1, P2, &SubdivideStart, &SubdivideEnd);

		SubdivideStart = m_vec3CenterPoints[P2];
		SubdivideEnd = m_vec3CenterPoints[P3];
		genLightning( P2, P3, &SubdivideStart, &SubdivideEnd);

		SubdivideStart = m_vec3CenterPoints[P3];
		SubdivideEnd = m_vec3CenterPoints[P4];
		genLightning( P3, P4, &SubdivideStart, &SubdivideEnd);

#endif

	}
	else
	{
		m_bInitialized = true;
		LowIndex = 0;
		HighIndex = m_dwNumPoints;
		SubdivideStart = *start;
		SubdivideEnd   = *end;
		
		genLightning( LowIndex, HighIndex, &SubdivideStart, &SubdivideEnd);

	}

	unguard;
}


void osc_eleBolt::framemove( I_camera* _camptr,float eletime )
{
	guard;

	m_fLastChangeTime += eletime;
	m_fLastBoltTime = eletime;

	// 对频率的限制:
	if( m_fLastChangeTime > m_fFreqTime )
	{
		m_fLastChangeTime = float_mod( m_fLastChangeTime,m_fFreqTime );
		m_bReadyRender = true;
	}
	else
		return;

	elebolt_animate( &m_vec3Start,&m_vec3End );

	// fill the poly with point&cam data.
	elebolt_fillpoly( _camptr );


	unguard;
}


//render the elebolt.
BOOL osc_eleBolt::render( osc_middlePipe* _pipe )
{
	guard;

	if( m_bReadyRender )
	{
		m_sPoly.m_sBufData.m_iVertexStart = _pipe->fill_dnamicVB( 
			osc_eleBolt::m_iVerbufId,m_arrRVertex,
			sizeof( os_billBoardVertex ),m_dwNumPoints*4 );
		osassert( m_sPoly.m_sBufData.m_iVertexStart>= 0 );

		_pipe->render_polygon( &m_sPoly );
	}

	return TRUE;

	unguard;
}




//! 初始化静态变量
template<> osc_boltMgr* ost_Singleton<osc_boltMgr>::m_ptrSingleton = NULL;



/** 得到一个sg_timer的Instance指针.
*/
osc_boltMgr* osc_boltMgr::Instance( void )
{
	guard;

	if( !osc_boltMgr::m_ptrSingleton )
	{

		osc_boltMgr::m_ptrSingleton = new osc_boltMgr;
		
		osassert( osc_boltMgr::m_ptrSingleton );
	}
	
	return osc_boltMgr::m_ptrSingleton;

	unguard;
}

/** 删除一个sg_timer的Instance指针.
*/
void osc_boltMgr::DInstance( void )
{
	SAFE_DELETE( osc_boltMgr::m_ptrSingleton );
}



void osc_boltMgr::init_boltMgr( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	m_ptrMPipe = _pipe;

	//
	// 创建动态的顶点缓冲区.创建一个比较需要大很多倍的顶点缓冲区
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	osc_eleBolt::m_iVerbufId = m_ptrMPipe->create_dynamicVB( 
		t_dwFvf,MAXBOLT_POLY*100*sizeof( os_billBoardVertex ) );
	osassert( osc_eleBolt::m_iVerbufId>= 0 );

	// 
	// 创建静态的索引缓冲区。
	int t_iGBufIdx = -1;
	WORD*   t_idxBuf = (WORD*)START_USEGBUF( t_iGBufIdx );
	int     t_iIbufIdx,t_iVbufIdx;
	for( int t_i=0;t_i<MAXBOLT_POLY;t_i ++ )
	{
		t_iIbufIdx = t_i*6;
		t_iVbufIdx = t_i*4;

		t_idxBuf[t_iIbufIdx+0] = 0 + t_iVbufIdx;
		t_idxBuf[t_iIbufIdx+1] = 1 + t_iVbufIdx;
		t_idxBuf[t_iIbufIdx+2] = 2 + t_iVbufIdx;

		t_idxBuf[t_iIbufIdx+3] = 0 + t_iVbufIdx;
		t_idxBuf[t_iIbufIdx+4] = 2 + t_iVbufIdx;
		t_idxBuf[t_iIbufIdx+5] = 3 + t_iVbufIdx;
	}
	osc_eleBolt::m_iIdxbufId = m_ptrMPipe->create_staticIB( t_idxBuf,MAXBOLT_POLY*6 );
	osassert( osc_eleBolt::m_iIdxbufId >= 0 );
	END_USEGBUF( t_iGBufIdx );


	unguard;
}


int osc_boltMgr::create_bolt( os_boltInit* _binit,int _bnum )
{
	guard;

	int    t_idxBolt,t_i;

	osassert( _bnum <= MAX_ELEPERBOLT );

	t_idxBolt = m_vecBoltIdx.get_freeNode();

	for( t_i=0;t_i<_bnum;t_i ++ )
	{
		m_vecBoltIdx[t_idxBolt].m_iBoltIdx[t_i] = get_freeEleBolt();
		osassert( m_vecBoltIdx[t_idxBolt].m_iBoltIdx[t_i]>=0 );

		m_vecBolt[m_vecBoltIdx[t_idxBolt].m_iBoltIdx[t_i]].
			elebolt_init( &_binit[t_i] );
	}

	m_vecBoltIdx[t_idxBolt].m_iBotNum = _bnum;

	return t_idxBolt;

	unguard;
}

//! 重设当前的bolt.
void osc_boltMgr::reset_bolt( os_boltInit* _binit,int _idx )
{
	guard;
	osassert( m_vecBolt.validate_id( _idx ) );

	for( int t_i=0;t_i<m_vecBoltIdx[_idx].m_iBotNum;t_i ++ )
	{
		m_vecBolt[m_vecBoltIdx[_idx].m_iBoltIdx[t_i]].
			elebolt_reset( &_binit[t_i] );
	}

	return;

	unguard;
}

//! 删除一个bolt
void osc_boltMgr::delete_bolt( int _id )
{
	guard;
	
	osassert( m_vecBoltIdx.validate_id( _id ) );

	for( int t_i=0;t_i<m_vecBoltIdx[_id].m_iBotNum;t_i ++ )
		m_vecBolt.release_node( m_vecBoltIdx[_id].m_iBoltIdx[t_i] );
	m_vecBoltIdx.release_node( _id );

	unguard;
}



//!　ATTENTION TO OPP: 相机可见性？？？？
void osc_boltMgr::frame_move( I_camera* _camptr,float _etime )
{

	CSpaceMgr<osc_eleBolt>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecBolt.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecBolt.next_validNode(&t_ptrNode ) )
	{
		t_ptrNode.p->framemove( _camptr,_etime );
	}
}

void osc_boltMgr::render( void )
{
	CSpaceMgr<osc_eleBolt>::NODE  t_ptrNode;
	for( t_ptrNode = m_vecBolt.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecBolt.next_validNode(&t_ptrNode ) )
	{
		t_ptrNode.p->render( m_ptrMPipe );
	}
}