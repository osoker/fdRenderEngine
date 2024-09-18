///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterShadow.cpp
 * 
 *  His:      River created @ 2004-12-11
 *
 *  Desc:     引擎中人物阴影相关的函数。
 * 
 *  "你有多大的成就，就要受到多大的争议" 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"

//! 边列表文件对应的扩展名
# define EDGEFILE_EXTENT   ".edg"

//! River added @ 2008-11-19:用于控制人物阴影的透明度，0完全透明.1完全不透明
OSENGINE_API float                  g_fShadowChroma = 0.2f;

//! 设备重设时，阴影部分调入，使用的临界区
CRITICAL_SECTION    g_sShadowSec;


# define MAX_VERTEX_PERSKINMESH   4096

osc_fakeGlowData::osc_fakeGlowData()
{
	m_vec3Normal.resize( MAX_VERTEX_PERSKINMESH );
	m_iNormalUseNum = 0;
	m_fLength = 0.05f;
	m_fCurUseLength = m_fLength;
}

osVec3D* osc_fakeGlowData::get_unUseNormlDataStart( void )
{
	return &m_vec3Normal[m_iNormalUseNum];
}

void osc_fakeGlowData::set_useNormalNum( int _num )
{
	m_iNormalUseNum += _num;
	if( m_iNormalUseNum>MAX_VERTEX_PERSKINMESH )
		osassert( false );
}

void osc_fakeGlowData::resetNormal( void )
{
	m_iNormalUseNum = 0;
}

void osc_fakeGlowData::set_fakeGlowData( DWORD _color,float _length )
{
	m_dwColor = _color;
	m_fLength = _length;
}

# define MOVE_TIME   4.0f

void osc_fakeGlowData::frame_move( void )
{
	guard;

	float t_fEletime = sg_timer::Instance()->get_lastelatime();
	static bool  t_bUp = true;

	float  t_fCurFrameLength = t_fEletime*m_fLength/MOVE_TIME;

	if( t_bUp )
	{
		m_fCurUseLength += t_fCurFrameLength;
		if( m_fCurUseLength > ( this->m_fLength*1.5f ) )
		{
			m_fCurUseLength = m_fLength*1.5f;
			t_bUp = false;
		}
	}
	else
	{
		m_fCurUseLength -= t_fCurFrameLength;
		if( m_fCurUseLength < ( m_fLength*0.5f ) )
		{
			m_fCurUseLength = m_fLength*0.5f;
			t_bUp = true;
		}		
	} 

	unguard;
}


osc_fakeGlowData* osc_bpmShadow::m_ptrFakeGlowData = NULL;


/** 

阴影的使用描述：

目前场景主角人物的即时阴影使用Stencil的方式进行。刚开始的时候使用ZPass的方式，

2006-5-12修改为使用ZFail的方式。

ZPass的方式主要问题在于当近剪切面切掉部分ShadowVolume后，人物的阴影会大块的显示
在屏幕，出现一些错误

ZFail的错误在于远剪切面也会剪切掉人物的ShadowVolume,会在人物的阴影上显示一个小洞，
好在人物阴影的洞非常小，品质上比ZPass产生的错误要好很多。
ZFail的阴影投射矩离使用了一个非常大的值，为的是让ShadowVolume,
在远剪切面内都正确的写入StencilBuf.

ZFail的情况下，
必须渲染人物朝向方向光的面，不然会出现透过山可以看见人物阴影的情形

可能是因为人物的面不是封闭的面，所以就算渲染人物朝向方向光的面，



在山挡住人物的时候，仍然有少部分的阴影破面出现。这个问题暂时没有解决。
有两种解决方案：第一：通过相机，做人物位置和地形的可见性，如果此时看不见人
物，则不渲染人物的阴影，第二：设置相机，使不出现山体挡住人物的情形。
River @ 2006-5-13:
最终的解决方案：相机位置和焦点的射线之间如果有地形挡住，则不渲染人物的阴影。


为了使人物不产生自阴影，程序先画人物的阴影，然后再画人物。相当于人物把人物的自阴影
部分的阴影面片挡住了。

**/

struct SHADOWVERTEX
{
    osVec4D     m_vec4Pos;
    DWORD       m_dwDiffuse;
};

//! 用于shadowSquare的渲染
#define   SHADOWVERTEX_FVF     (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

//! 使用Zpass的阴影渲染
# define STENCIL_ZPASS    1


//! 初始化静态变量
LPDIRECT3DDEVICE9 osc_bpmShadow::m_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9  osc_bpmShadow::m_pVBufShadowSqu = NULL;


//@{
//! 渲染阴影时用到的渲染状态块索引
int osc_bpmShadow::m_iSHDrawState = -1;
int osc_bpmShadow::m_iSHDrawRestoreState = -1;
int osc_bpmShadow::m_iShadowSquareState = -1;
int osc_bpmShadow::m_iShadowSSRestore = -1;
/** \brief
*  设置渲染状态，使渲染的时候，只渲染几何信息到到zbuffer,不显示。
* 
*  有阴影的武器需要先渲染武器的zbuffer,用于使用武器上没有阴影的效果。
*/
int osc_bpmShadow::m_iOnlyZbufSB = -1;
int osc_bpmShadow::m_iRestoreOnlyZbufSB = -1;
//@}

//! 屏幕宽度，屏幕高度
float osc_bpmShadow::m_fScrWidth = 800.0f;
float osc_bpmShadow::m_fScrHeight = 600.0f;



//! 默认阴影投射的长度,使用大量的象素填充来减少远剪切面处的错误
# define DEFAULT_PROJSHADOWLENGTH   60.0f


osc_bpmShadow::osc_bpmShadow()
{
	m_ptrBpmVer = NULL;
	m_ptrBpmVerAT = NULL;
	this->m_iVerNum = 0;
	this->m_ptrBpmIdx = NULL;
	this->m_iIdxNum = 0;

	this->m_bFront = NULL;
	m_arrNeiList = NULL;

	//!  要渲染的边的列表。
	m_arrEdgeList = NULL;
	m_iEdgeNum = 0;

	//! 　
	m_pShadowVBuf = NULL;
	m_iShadowVerNum = 0;

	m_pSourceIdxBuf = NULL;
	m_iSourceTriNum = 0;

	//! 更新ShadowVolume.
	m_bUpdated = false;

	m_fShadowProjLength = DEFAULT_PROJSHADOWLENGTH;

	m_bFakeGlow = false;
	m_vec3Normal = NULL;

}

osc_bpmShadow::~osc_bpmShadow()
{
	release_shadowData();
}

//! 
# define  MAX_SHADERVERMULTI   6

//! 初始化阴影数据结构。　
bool osc_bpmShadow::init_shadowData( const char* _fname )
{
	guard;

	osassert( _fname&& (_fname[0]) );

	char      t_str[256];
	s_string   t_strName = _fname;
	int   t_size;

	m_bFront = new bool[m_iIdxNum/3];
	m_arrNeiList = new os_neighbour[this->m_iIdxNum/3];

	//@{
	// syq : 加大为2倍的缓冲区大小。　旧代码当游离的网格边缘多时，缓冲区会不足
	//m_arrEdgeList = new WORD[ m_iIdxNum ];
	m_arrEdgeList = new WORD[ m_iIdxNum * 2 ];
	//@}

	t_strName += EDGEFILE_EXTENT;
	// tzz add:
	// 使用CReadFile 来读取内容,
	// 防止在osassert 抛出异常之后没有 END_USEGBUF（有太多的osassert）
	// 导致程序不能正常终止,需要用资源管理器来终止
	//
	// 查看 CReadFile 获得更多的信息
	// 
	//t_fstart = START_USEGBUF( t_iGBufIdx );
	//int   t_size = read_fileToBuf( 
	//	(char*)t_strName.c_str(),t_fstart,TMP_BUFSIZE );
	//if( t_size <=0 )
	//{
	//	osassert( false );
	//	return false;
	//}

	CReadFile t_file;
	if(!t_file.OpenFile(t_strName.c_str())){
		return false;
	}
	
	//READ_MEM_OFF( t_str,t_fstart,sizeof( char )*3 );
	t_file.ReadBuffer(t_str,sizeof( char )*3);

	t_str[3] = NULL;
	if( strcmp( t_str,"edg" ) )
		osassert( false );

	// 读入主次版本号，
	//READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );
	//READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );
	t_file.ReadBuffer(&t_size,sizeof(int));
	t_file.ReadBuffer(&t_size,sizeof(int));


	// 读入每一个三角形对应的邻接信息。
	//READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );
	t_file.ReadBuffer(&t_size,sizeof(int));

	osassertex( t_size == (this->m_iIdxNum/3),
		va("出错的文件为:<%s> <%d %d>..\n",t_str,t_size,m_iIdxNum ) );
	
// 	READ_MEM_OFF( this->m_arrNeiList,
// 		t_fstart,sizeof( os_neighbour )*t_size );
	t_file.ReadBuffer(this->m_arrNeiList,sizeof( os_neighbour )*t_size);

	//END_USEGBUF( t_iGBufIdx );
	t_file.CloseFile();

	//
	// 初始化设备相关的数据，创建顶点和索引缓冲区，
	// River @ 2005-8-18:原来创建的阴影顶点缓冲区是原来idxNum的三倍，会出现问题。
	// River @ 2006-5-12: 使用ZFail的阴影渲染，需要把朝向方向光的面渲染了。
	osassert( m_pd3dDevice );
	HRESULT    t_hr;
	t_hr = m_pd3dDevice->CreateVertexBuffer( 
		sizeof( osVec3D )*(m_iIdxNum*MAX_SHADERVERMULTI + m_iVerNum),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		0, D3DPOOL_DEFAULT,&m_pShadowVBuf,NULL );
	osassert( !FAILED( t_hr ) );
	
	// 创建人物身体几何数据的索引幼冲区
	t_hr = m_pd3dDevice->CreateIndexBuffer( 
		sizeof( WORD )*m_iIdxNum,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_pSourceIdxBuf,NULL );
	osassert( !FAILED( t_hr ) );
	m_iSourceTriNum = 0;


	return true;

	unguard;
}

//! 设备的reset相关函数.
void osc_bpmShadow::shadow_onLostDevice( void )
{
	guard;

	SAFE_RELEASE( m_pShadowVBuf );
	SAFE_RELEASE( m_pSourceIdxBuf );

	unguard;
}
void osc_bpmShadow::shadow_onResetDevice( void )
{
	guard;

	//! restore过了.
	if( m_pShadowVBuf )
		return;

	HRESULT    t_hr;
	t_hr = m_pd3dDevice->CreateVertexBuffer( 
		sizeof( osVec3D )*( m_iIdxNum*MAX_SHADERVERMULTI + m_iVerNum),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		0, D3DPOOL_DEFAULT,&this->m_pShadowVBuf,NULL );
	if( FAILED( t_hr ) )
		osassertex( false,"阴影reset Device失败...\n" );

	// 创建人物身体几何数据的索引幼冲区
	t_hr = m_pd3dDevice->CreateIndexBuffer( 
		sizeof( WORD )*m_iIdxNum,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_pSourceIdxBuf,NULL );
	osassert( !FAILED( t_hr ) );


	//! 重新填充屏幕多边形的缓冲区，有可能修改屏幕大小
	if( !fill_screenSquareVB() )
		osassertex( false,"阴影的屏幕缓冲区填充失败...\n" );

	unguard;
}



//! 释放当前的阴影相关数据。
void osc_bpmShadow::release_shadowData( void )
{
	guard;

	// ATTENTION TO OPT @ 2009-4-30:
	// 最好使用全局的一个缓冲区，不然会非常的费时间。
	SAFE_DELETE_ARRAY( this->m_ptrBpmIdx );
	SAFE_DELETE_ARRAY( this->m_ptrBpmVerAT );
	SAFE_DELETE_ARRAY( this->m_ptrBpmVer );
	SAFE_DELETE_ARRAY( this->m_bFront );
	SAFE_DELETE_ARRAY( this->m_arrNeiList );
	m_iIdxNum = 0;
	m_iVerNum = 0;

	SAFE_DELETE_ARRAY( this->m_arrEdgeList );
	m_iEdgeNum = 0;


	SAFE_RELEASE( m_pShadowVBuf );
	m_iShadowVerNum = 0;

	SAFE_RELEASE( m_pSourceIdxBuf );
	m_iSourceTriNum = 0;

	m_bFakeGlow = false;
	m_vec3Normal = NULL;


	unguard;
}

//! 往边列表中加入一条边。
void osc_bpmShadow::push_edge( WORD _verIdxs,WORD _verIdxe )
{
	guard;
	
	// syq mod @ 05.3.4　超出有效索引，返回
	for( int t_i=0;t_i<this->m_iEdgeNum;t_i ++ )
	{

		if( ((m_arrEdgeList[t_i*2] == _verIdxs)&&
			(m_arrEdgeList[t_i*2+1] == _verIdxe)) ||
			((m_arrEdgeList[t_i*2] == _verIdxe)&&
			(m_arrEdgeList[t_i*2+1] == _verIdxs) ) )
		{
				osDebugOut( "此模型的阴影有问题，存在重复边:%d,%d\n", _verIdxs, _verIdxe );
				return;
		}
	}

	osassertex( (m_iEdgeNum < m_iIdxNum),"\n边的数量超出缓冲区的大小!\n" );

	m_arrEdgeList[this->m_iEdgeNum*2] = _verIdxs;
	m_arrEdgeList[this->m_iEdgeNum*2+1] = _verIdxe;
	m_iEdgeNum ++;


	return;

	unguard;
}

//! 每一帧开始时，重设当前的shadowVolume.
void osc_bpmShadow::reset_shadowVolume( void )
{
	guard;

	m_iEdgeNum = 0;
	m_iShadowVerNum = 0;
	
	memset( this->m_bFront,0,sizeof( bool )*m_iIdxNum/3 );


	unguard;
}

//! 处理得到产生阴影的模型边。
void osc_bpmShadow::get_shadowEdge( void )
{
	guard;

	for( int i=0; i<this->m_iIdxNum/3; i++ )
	{
		// 三角形的三个顶点在缓冲区内的索引
		WORD t_wVerIdx0 = m_ptrBpmIdx[3*i+0];
		WORD t_wVerIdx1 = m_ptrBpmIdx[3*i+1];
		WORD t_wVerIdx2 = m_ptrBpmIdx[3*i+2];

		if( !this->m_bFront[i] )
		{
			for (int n = 0; n<3; ++n)
			{
				int t_iEdgeNeiIdx = m_arrNeiList[i].m_wN[n];
				//遍历所有的面，生成面向光源的轮廓边，有如下两种边需要添加
				//把边界的边加入轮廓边列表中
				if( t_iEdgeNeiIdx==0xffff ) // no neighbor
				{
					if(n==0) push_edge( t_wVerIdx0, t_wVerIdx1 );
					if(n==1) push_edge( t_wVerIdx1, t_wVerIdx2 );
					if(n==2) push_edge( t_wVerIdx2, t_wVerIdx0 );
					continue;
				}

				//把面向光源和背向光源的夹边加入轮廓列表中
				if( m_bFront[t_iEdgeNeiIdx] )
				{
					if(n==0) this->push_edge( t_wVerIdx0, t_wVerIdx1 );
					if(n==1) this->push_edge( t_wVerIdx1, t_wVerIdx2 );
					if(n==2) this->push_edge( t_wVerIdx2, t_wVerIdx0 );
				}
			}
		}
	}

	//osassertex( m_iEdgeNum > 0,
	//	va( "EdegNum is:<%d>,Idx num is:<%d>..\n",m_iEdgeNum,m_iIdxNum ) );
	// River @ 2010-10-9:不管何种原因，此处不出错，最多不渲染阴影即可。

	unguard;
}




//! 得到模型中每一个三角形与方向光的关系，面对或是背对，
void osc_bpmShadow::cal_triToLight( osVec3D* _lightDir/* = NULL*/ )
{
	guard;

	osVec3D   t_vec3V0,t_vec3V1,t_vec3V2;

	osVec3D   t_vec3LightDir = g_vec3LPos;

	//! 如果传入数据，则使用非全局的光照方向
	if( _lightDir )
		t_vec3LightDir = *_lightDir;

	for( int t_i=0;t_i<m_iIdxNum/3;t_i ++ )
	{
		t_vec3V0 = 
			m_ptrBpmVerAT[m_ptrBpmIdx[t_i*3+2]] -
			m_ptrBpmVerAT[m_ptrBpmIdx[t_i*3+1]];

		t_vec3V1 =
			m_ptrBpmVerAT[m_ptrBpmIdx[t_i*3+1]] -
			m_ptrBpmVerAT[m_ptrBpmIdx[t_i*3]];

		osVec3Cross( &t_vec3V2,&t_vec3V0,&t_vec3V1 );


		//! 使用全局的方向光数据，
		if( osVec3Dot( &t_vec3V2,&t_vec3LightDir )>= 0.0f )
			m_bFront[t_i] = false;
		else
			m_bFront[t_i] = true;
	}

	return;

	unguard;
}



//! 超过如下的权重的骨骼才加入阴影顶点的运算。
# define MIN_WEIGHT   0.0001f
//! 根据传入的矩阵队列更新模型中每一个顶点的位置。
void osc_bpmShadow::update_verPos( osMatrix* _matArr,int _matNum )
{
	guard;
	
	osVec3D           t_vec3V0,t_vec3V1;
	os_characterVer    *t_ptrVer;
	float             t_fWei;

	for( int t_i=0;t_i<m_iVerNum;t_i ++ )
	{
		t_ptrVer = &m_ptrBpmVer[t_i];


		//　顶点和法向量一起运算
		osVec3TransformCoord( &t_vec3V0,
			&t_ptrVer->m_vec3Pos,&_matArr[t_ptrVer->m_vecBoneIdx[0]] );
		m_ptrBpmVerAT[t_i] = t_vec3V0 * t_ptrVer->m_vecWeight[0];

		//
		if( t_ptrVer->m_vecWeight[1] > MIN_WEIGHT )
		{
			osVec3TransformCoord( &t_vec3V0,
				&t_ptrVer->m_vec3Pos,&_matArr[t_ptrVer->m_vecBoneIdx[1]] );
			m_ptrBpmVerAT[t_i] += t_vec3V0 * t_ptrVer->m_vecWeight[1];
		}

		// 第二根骨骼，
		t_fWei = 1.0f - t_ptrVer->m_vecWeight[0] - t_ptrVer->m_vecWeight[1];
		if( t_fWei > MIN_WEIGHT )
		{
			osVec3TransformCoord( &t_vec3V0,
				&t_ptrVer->m_vec3Pos,&_matArr[t_ptrVer->m_vecBoneIdx[2]] );
			m_ptrBpmVerAT[t_i] += t_vec3V0 * t_fWei;
		}

	}

	if( !m_bFakeGlow )
		return;
	if( !m_vec3Normal )
		return;

	for( int t_i=0;t_i<m_iVerNum;t_i ++ )
	{
		t_ptrVer = &m_ptrBpmVer[t_i];

		//　顶点和法向量一起运算
		osVec3TransformNormal( &t_vec3V0,
			&t_ptrVer->m_vec3Normal,&_matArr[t_ptrVer->m_vecBoneIdx[0]] );
		m_vec3Normal[t_i] = t_vec3V0 * t_ptrVer->m_vecWeight[0];

		//
		if( t_ptrVer->m_vecWeight[1] > MIN_WEIGHT )
		{
			osVec3TransformNormal( &t_vec3V0,
				&t_ptrVer->m_vec3Normal,&_matArr[t_ptrVer->m_vecBoneIdx[1]] );
			m_vec3Normal[t_i] += t_vec3V0 * t_ptrVer->m_vecWeight[1];
		}

		// 第二根骨骼，
		t_fWei = 1.0f - t_ptrVer->m_vecWeight[0] - t_ptrVer->m_vecWeight[1];
		if( t_fWei > MIN_WEIGHT )
		{
			osVec3TransformNormal( &t_vec3V0,
				&t_ptrVer->m_vec3Normal,&_matArr[t_ptrVer->m_vecBoneIdx[2]] );
			m_vec3Normal[t_i] += t_vec3V0 * t_fWei;
		}

		osVec3Normalize( &m_vec3Normal[t_i],&m_vec3Normal[t_i] );
	}

	unguard;
}


//! fakeGlow相关，把人物的边数据填充到顶点缓冲区内。
void osc_bpmShadow::get_edgeVertexData( osVec3D& _vpVec )
{
	guard;

	int   t_iGBufIdx = -1;
	os_sceVerDiffuse*   t_ptrVStart  = (os_sceVerDiffuse*)START_USEGBUF( t_iGBufIdx );

	osassert( m_vec3Normal );

	// 
	// Extrude边对应的顶点数据
	osVec3D    t_vec3Pos[4];
	osVec3D    t_vec3DirExtrude;
	for( int t_i=0;t_i<m_iEdgeNum;t_i ++ )
	{
		t_vec3Pos[0] = m_ptrBpmVerAT[m_arrEdgeList[t_i*2+0]];
		t_vec3Pos[1] = m_ptrBpmVerAT[m_arrEdgeList[t_i*2+1]];

		t_vec3Pos[2] = t_vec3Pos[0] + 
			m_vec3Normal[m_arrEdgeList[t_i*2+0]]*m_ptrFakeGlowData->m_fCurUseLength;
		t_vec3Pos[3] = t_vec3Pos[1] + 
			m_vec3Normal[m_arrEdgeList[t_i*2+1]]*m_ptrFakeGlowData->m_fCurUseLength;

		t_ptrVStart[t_i*6+0].m_vecPos = t_vec3Pos[0];
		t_ptrVStart[t_i*6+0].m_color = 0xfffff000;
		t_ptrVStart[t_i*6+1].m_vecPos = t_vec3Pos[1];
		t_ptrVStart[t_i*6+1].m_color = 0xfffff000;
		t_ptrVStart[t_i*6+2].m_vecPos = t_vec3Pos[2];
		t_ptrVStart[t_i*6+2].m_color = 0x00fff000;

		t_ptrVStart[t_i*6+3].m_vecPos = t_vec3Pos[1];
		t_ptrVStart[t_i*6+3].m_color = 0xfffff000;
		t_ptrVStart[t_i*6+4].m_vecPos = t_vec3Pos[3];
		t_ptrVStart[t_i*6+4].m_color = 0x00fff000;
		t_ptrVStart[t_i*6+5].m_vecPos = t_vec3Pos[2];
		t_ptrVStart[t_i*6+5].m_color = 0x00fff000;
	}

	m_sRenderPolygon.m_sBufData.m_iVertexStart = m_ptrMiddlePipe->fill_dnamicVB( 
		m_sRenderPolygon.m_sBufData.m_iVbufId,
		t_ptrVStart,sizeof( os_sceVerDiffuse ),m_iEdgeNum*6 );

	END_USEGBUF( t_iGBufIdx );

	m_sRenderPolygon.m_iVerNum = m_iEdgeNum*6;
	m_sRenderPolygon.m_iPriNum = m_iEdgeNum*2;

	return ;

	unguard;
}


//! 把模型中的阴影边缘数据转化为顶点数据，并填充到顶点缓冲区中。
void osc_bpmShadow::get_shadowVertexData( void )
{
	guard;

	osVec3D*  t_vec3D;
	HRESULT   t_hr;

	osassert( m_pShadowVBuf );

	t_hr = m_pShadowVBuf->Lock( 0,0,(void**)&t_vec3D,D3DLOCK_DISCARD );
	if( FAILED( t_hr ) )
	{
		//osassert( false );
		// River @ 2011-3-19:如果lock失败，返回就OK。
		m_iShadowVerNum = 0;
		return;
	}

	osassertex( (MAX_SHADERVERMULTI*m_iIdxNum) >= (m_iEdgeNum*6),"阴影缓冲区小于顶点数目" );

	// 先是extrude边对应的顶点数据
	osVec3D    t_vec3Pos[4];
	for( int t_i=0;t_i<m_iEdgeNum;t_i ++ )
	{
		t_vec3Pos[0] = m_ptrBpmVerAT[m_arrEdgeList[t_i*2+0]];
		t_vec3Pos[1] = m_ptrBpmVerAT[m_arrEdgeList[t_i*2+1]];

		t_vec3Pos[2] = t_vec3Pos[0] - g_vec3LPos*m_fShadowProjLength;
		t_vec3Pos[3] = t_vec3Pos[1] - g_vec3LPos*m_fShadowProjLength;

		t_vec3D[t_i*6+0] = t_vec3Pos[0];
		t_vec3D[t_i*6+1] = t_vec3Pos[1];
		t_vec3D[t_i*6+2] = t_vec3Pos[2];

		t_vec3D[t_i*6+3] = t_vec3Pos[1];
		t_vec3D[t_i*6+4] = t_vec3Pos[3];
		t_vec3D[t_i*6+5] = t_vec3Pos[2];
	}

	// copy所有转化后的人物何顶点
	memcpy( &t_vec3D[m_iEdgeNum*6],m_ptrBpmVerAT,sizeof( osVec3D )*m_iVerNum );

	m_pShadowVBuf->Unlock();

	m_iShadowVerNum = m_iEdgeNum*6;

	// River @ 2010-12-21：去掉assert,直接返回。
	if( m_iShadowVerNum <= 0 )
		return;

	//
	// 填充面对方向光的面索引
	WORD*     t_ptrIdx;
	t_hr = m_pSourceIdxBuf->Lock( 0,0,(void**)&t_ptrIdx,D3DLOCK_DISCARD );
	osassert( !FAILED( t_hr ) );
	m_iSourceTriNum = 0;
	for( int t_i=0;t_i<m_iIdxNum/3;t_i ++ )
	{
		if( !m_bFront[t_i] )
			continue;

		t_ptrIdx[m_iSourceTriNum*3] = m_ptrBpmIdx[t_i*3];
		t_ptrIdx[m_iSourceTriNum*3+1] = m_ptrBpmIdx[t_i*3+1];
		t_ptrIdx[m_iSourceTriNum*3+2] = m_ptrBpmIdx[t_i*3+2];

		m_iSourceTriNum ++;
	}

	m_pSourceIdxBuf->Unlock();


	unguard;
}




//! 每一帧对阴影数据的更新
void osc_bpmShadow::update_shadow( osMatrix* _matArr,int _matNum )
{
	guard;

	//! 先重设当前的shadowVolume.
	reset_shadowVolume();

	//! 更新顶点位置
	update_verPos( _matArr,_matNum );

	//! 计算每一个模型中的面跟方向光的关系。
	cal_triToLight();

	//! 得到可以产生阴影的边列表。
	get_shadowEdge();

	//! 根据边列表来填充渲染阴影的顶点缓冲区，
	get_shadowVertexData();


	m_bUpdated = true;

	return;

	unguard;
}


/* \brief
*  画fadeGlow的接口，描边效果
* \param DWORD _color        要画的fadeGlow的颜色数据
* \param DWORD _colorNoise   颜色数据的随机数据值,在颜色的基础，加入此随项随机值
* \param osVec3D& _vpVec     从某一个方向描边，即阴影数据时，灯光的方向。
* \param float _length       描边数据得到后，往外扩散的边缘数据的长度。
* \param float _lenthNoise   边缘数据长度的随机值。
* \param bool _projParallel  如果此值为真，则顺着投射方向往前画描边数据，否则垂直画。
*/
bool osc_bpmShadow::draw_fakeGlow( DWORD _color,DWORD _colorNoise,
								  osVec3D& _vpVec,float _length,
								  float _lenthNoise,bool _projParallel )
{
	guard;


	//! 先重设当前的shadowVolume.
	reset_shadowVolume();

	//! 在阴影处理中，已经更新了顶点位置
	//  update_verPos( _matArr,_matNum );

	//! 计算每一个模型中的面跟方向光的关系。
	cal_triToLight( &_vpVec );

	//! 得到可以产生阴影的边列表。
	get_shadowEdge();

	//! 根据边列表来填充渲染阴影的顶点缓冲区，
	// get_shadowVertexData();
	//  
	// 填充中间管道的顶点缓冲区，然后DrawTriangleList
	get_edgeVertexData( _vpVec );

	m_ptrMiddlePipe->render_polygon( &m_sRenderPolygon );

	return true;

	unguard;
}


//! 开始画ShadowVolume.
void osc_bpmShadow::start_shadowVolumeDraw( void )
{
	guard;

	os_stateBlockMgr*   t_ptrSBM = os_stateBlockMgr::Instance();

	if( (m_iSHDrawState>=0)&&
		t_ptrSBM->validate_stateBlockId( m_iSHDrawState ) )
	{
		t_ptrSBM->apply_stateBlock( m_iSHDrawState );
	}
	else
	{
		// 如果一个stateBlock无效,则另一个肯定也会无效,所以两个一起record.
		record_ShadowVolumeStateBlock();
		t_ptrSBM->apply_stateBlock( m_iSHDrawState );
	}

	unguard;
}
void osc_bpmShadow::end_shadowVolumeDraw( void )
{
	guard;

	os_stateBlockMgr*   t_ptrSBM = os_stateBlockMgr::Instance();

	if( (m_iSHDrawRestoreState>=0)&&
		t_ptrSBM->validate_stateBlockId( m_iSHDrawRestoreState ) )
	{
		t_ptrSBM->apply_stateBlock( m_iSHDrawRestoreState );
	}
	else
	{
		// 如果一个stateBlock无效,则另一个肯定也会无效,所以两个一起record.
		record_ShadowVolumeStateBlock();
		t_ptrSBM->apply_stateBlock( m_iSHDrawRestoreState );
	}

	unguard;
}


//  
//! ATTENTION:7500LE的显卡，这个函数内如果没有DIP，则必花屏
//! Shadow渲染的接口。
bool osc_bpmShadow::draw_shadowVolume( void )
{
	guard;

	osassert( m_pd3dDevice );


	// 没有需要画的阴影顶点和边
	if( m_iShadowVerNum <= 0 )
		return true;
	
	//! river @ 2010-3-9:有可能出现为零的情形。
	if( m_iSourceTriNum == 0 )
		return true;


# if STENCIL_ZPASS
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
# endif 

	// Draw front-side of shadow volume in stencil/z only
	m_pd3dDevice->SetStreamSource( 0,m_pShadowVBuf,0,sizeof( osVec3D ) );
	// extrude triangle.
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,m_iShadowVerNum/3 );

# if !STENCIL_ZPASS
	m_pd3dDevice->SetIndices( m_pSourceIdxBuf );	
	// 人物身体上的几何数据
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		m_iShadowVerNum,0,m_iVerNum,0,this->m_iSourceTriNum );
# endif 

//
# if STENCIL_ZPASS
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
# else
	// zfail 方式
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT );
# endif 


	// Draw back-side of shadow volume in stencil/z only
	// extrude triangles.
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,m_iShadowVerNum/3 );

	// 
	// 人物身上原有的几何数据
	m_pd3dDevice->SetIndices( m_pSourceIdxBuf );
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		m_iShadowVerNum,0,m_iVerNum,0,this->m_iSourceTriNum );


	return true;

	unguard;
}


//! 录制渲染阴影需要的渲染状态块。
void osc_bpmShadow::record_ShadowVolumeStateBlock( void )
{
	guard;

	osassert( m_pd3dDevice );

	os_stateBlockMgr* t_ptrBM;

	t_ptrBM = os_stateBlockMgr::Instance();

	m_iSHDrawState = t_ptrBM->create_stateBlock();
	osassert( m_iSHDrawState>= 0 );
	os_stateBlockMgr::Instance()->start_stateBlock();
	// Block Start
	{
		// 
		// River added @ 2005-7-11: 阴影的alpha来源必须确认，不然弹出界面时会出
		//                          现阴影时隐时现的问题。
		// River @ 2006-5-13: 使用D3DRS_COLORWRITEENABLE可以不考虑纹理和状态的问题，
		//                    为了确认，暂不去除此处的状态。
		m_pd3dDevice->SetTexture( 0 , NULL );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,D3DTOP_DISABLE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

		// Disable z-buffer writes (note: z-testing still occurs), and enable the
		// stencil-buffer
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );

		// Zfunc设置的更严格一些
		m_pd3dDevice->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESS );

		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );
		// Dont bother with interpolating color
		m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE,  D3DSHADE_FLAT );

		// Set up stencil compare fuction, reference value, and masks.
		// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
		// Note: since we set up the stencil-test to always pass, the STENCILFAIL
		// renderstate is really not needed. 
		m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );	
		m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );

		// If ztest passes, inc/decrement stencil buffer value
		m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,       0x1 );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );


		// Make sure that no pixels get drawn to the frame buffer
		m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,FALSE );

		m_pd3dDevice->SetFVF( D3DFVF_XYZ );

# if STENCIL_ZPASS
		m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_INCR );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CW );
# else
		m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP );
		m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CW );
# endif 

		m_pd3dDevice->SetVertexShader( NULL );
		m_pd3dDevice->SetPixelShader( NULL );

	}
	t_ptrBM->end_stateBlock( m_iSHDrawState );
	m_iSHDrawRestoreState = t_ptrBM->create_stateBlock();
	osassert( m_iSHDrawRestoreState>=0 );
	t_ptrBM->start_stateBlock();
	// Block Start,
	{
		// Restore render states
		m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );

		// 重设可以使用颜色
		m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 
			D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_ALPHA| D3DCOLORWRITEENABLE_RED );
	}
	t_ptrBM->end_stateBlock( m_iSHDrawRestoreState );

	return;

	unguard;
}

void osc_bpmShadow::record_ShadowSquareStateBlock( void )
{
	guard;

	osassert( m_pd3dDevice );

	os_stateBlockMgr* t_ptrBM;
	t_ptrBM = os_stateBlockMgr::Instance();
	// 
	// Shadow Square相关的渲染状态。
	m_iShadowSquareState = t_ptrBM->create_stateBlock();
	osassert( m_iShadowSquareState>=0 );
	t_ptrBM->start_stateBlock();
	// Block Start
	{
		// Set renderstates (disable z-buffering, enable stencil, disable fog, and
		// turn on alphablending)
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );


		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE/* D3DBLEND_SRCALPHA*/ );//g_select_src );
		m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );//g_select );

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );

		// 用于解决在某些情况下，人物影子变黑的问题。
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );

		// Only write where stencil val >= 1 (count indicates # of shadows that
		// overlap that pixel)
		m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,  0x1 );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
		// Draw a big, gray square
		m_pd3dDevice->SetFVF( SHADOWVERTEX_FVF );
		m_pd3dDevice->SetStreamSource( 0, m_pVBufShadowSqu, 0, sizeof(SHADOWVERTEX) );

	}
	t_ptrBM->end_stateBlock( m_iShadowSquareState );
	m_iShadowSSRestore = t_ptrBM->create_stateBlock();
	osassert( m_iShadowSSRestore>=0 );

	t_ptrBM->start_stateBlock();
	// Block Start
	{
		// Restore render states
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}
	t_ptrBM->end_stateBlock( m_iShadowSSRestore );


	unguard;
}


//! 全局的释放阴影系统.
void osc_bpmShadow::release_shadowSys( bool _frel )
{
	guard;

	SAFE_RELEASE( m_pVBufShadowSqu );

	if( !_frel )
		return;


	os_stateBlockMgr*  t_ptrIns = os_stateBlockMgr::Instance();
	if( m_iSHDrawRestoreState>=0 )
		t_ptrIns->release_stateBlock( m_iSHDrawRestoreState );
	if( m_iSHDrawState>= 0 )
		t_ptrIns->release_stateBlock( m_iSHDrawState );
	if( m_iShadowSquareState >= 0 )
		t_ptrIns->release_stateBlock( m_iShadowSquareState );
	if( m_iShadowSSRestore>=0 )
		t_ptrIns->release_stateBlock( m_iShadowSSRestore );
	if( m_iOnlyZbufSB >= 0 )
		t_ptrIns->release_stateBlock( m_iOnlyZbufSB );
	if( m_iRestoreOnlyZbufSB >= 0 )
		t_ptrIns->release_stateBlock( m_iRestoreOnlyZbufSB );

	return;

	unguard;
}

//! 设置只渲染zBuffer的设备状态。
void osc_bpmShadow::set_onlyZbufStateBlock( void )
{
	guard;

	os_stateBlockMgr*  t_ptrIns = os_stateBlockMgr::Instance();

	if( (m_iOnlyZbufSB >= 0)&&
		t_ptrIns->validate_stateBlockId( m_iOnlyZbufSB ) )
	{
		t_ptrIns->apply_stateBlock( m_iOnlyZbufSB );
		return;
	}

	m_iOnlyZbufSB = t_ptrIns->create_stateBlock();
	osassert( m_iOnlyZbufSB >= 0 );
	t_ptrIns->start_stateBlock();
	{
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,FALSE );	
	}

	t_ptrIns->end_stateBlock( m_iOnlyZbufSB );
	t_ptrIns->apply_stateBlock( m_iOnlyZbufSB );

	return;

	unguard;
}


//! 重设fake glow
void osc_bpmShadow::set_fakeGlow( bool _glow )
{
	guard;

	if( _glow )
	{
		m_bFakeGlow = true;
		if( !m_ptrFakeGlowData )
			return;
		m_vec3Normal = m_ptrFakeGlowData->get_unUseNormlDataStart();
		m_ptrFakeGlowData->set_useNormalNum( m_iVerNum );
	}
	else
		m_vec3Normal = NULL;


	unguard;
}


//! 重设全局的阴影透明度
void osc_bpmShadow::reset_shadowColor( float _alpha )
{
	guard;

	HRESULT   t_hr;

	osassert( _alpha >= 0.0f );

	SHADOWVERTEX*   t_verS;

	// 填充顶点缓冲区，
	t_hr = m_pVBufShadowSqu->Lock( 0,0,(void**)&t_verS,0 );
	if( FAILED( t_hr ) )
		osassert( false );

	t_verS[0].m_vec4Pos = osVec4D(  0, m_fScrHeight, 0.0f, 1.0f );
	t_verS[1].m_vec4Pos = osVec4D(  0,  0, 0.0f, 1.0f );
	t_verS[2].m_vec4Pos = osVec4D( m_fScrWidth, m_fScrHeight, 0.0f, 1.0f );
	t_verS[3].m_vec4Pos = osVec4D( m_fScrWidth,  0, 0.0f, 1.0f );

	DWORD  t_dw = DWORD(g_fShadowChroma * _alpha * 0xff);

	t_verS[0].m_dwDiffuse = t_dw << 24;
	t_verS[1].m_dwDiffuse = t_dw << 24;
	t_verS[2].m_dwDiffuse = t_dw << 24;
	t_verS[3].m_dwDiffuse = t_dw << 24;

	m_pVBufShadowSqu->Unlock();

	unguard;
}

//! 全局的，用于填充描边数据的dynamic vb idx.
int osc_bpmShadow::m_iDynamicVBIdx = -1;
osc_middlePipe* osc_bpmShadow::m_ptrMiddlePipe = NULL;
os_polygon osc_bpmShadow::m_sRenderPolygon;


//! 填充屏幕多边形的几何数据
bool osc_bpmShadow::fill_screenSquareVB( void )
{
	guard;

	LPDIRECT3DSURFACE9     t_pBackBuffer;
 	D3DSURFACE_DESC        t_d3dsdBackBuffer;
	HRESULT                t_hr;
	SHADOWVERTEX*          t_verS = NULL;

	if( FAILED( m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &t_pBackBuffer ) ) )
		return false;

    if( FAILED( t_pBackBuffer->GetDesc( &t_d3dsdBackBuffer ) ) )
		return false;

    if( FAILED( t_pBackBuffer->Release() ) )
		return false;

    // Set the size of the big square shadow
    m_fScrWidth = (FLOAT)t_d3dsdBackBuffer.Width;
    m_fScrHeight = (FLOAT)t_d3dsdBackBuffer.Height;

	// 填充顶点缓冲区，
	t_hr = m_pVBufShadowSqu->Lock( 0,0,(void**)&t_verS,0 );
	if( FAILED( t_hr ) )
		osassert( false );

	t_verS[0].m_vec4Pos = osVec4D(  0, m_fScrHeight, 0.0f, 1.0f );
	t_verS[1].m_vec4Pos = osVec4D(  0,  0, 0.0f, 1.0f );
	t_verS[2].m_vec4Pos = osVec4D( m_fScrWidth, m_fScrHeight, 0.0f, 1.0f );
	t_verS[3].m_vec4Pos = osVec4D( m_fScrWidth,  0, 0.0f, 1.0f );

	DWORD  t_dw = DWORD(g_fShadowChroma * 0xff);

	t_verS[0].m_dwDiffuse = t_dw << 24;
	t_verS[1].m_dwDiffuse = t_dw << 24;
	t_verS[2].m_dwDiffuse = t_dw << 24;
	t_verS[3].m_dwDiffuse = t_dw << 24;

	m_pVBufShadowSqu->Unlock();

	return true;

	unguard;
}


//! 全局的初始化我们的阴影系统。
void osc_bpmShadow::init_shadowSys( osc_middlePipe* _mp )
{
	guard;

	osassert( _mp );

	HRESULT         t_hr;

	m_ptrMiddlePipe = _mp;
	m_pd3dDevice = _mp->get_device();

	t_hr = m_pd3dDevice->CreateVertexBuffer( 
		sizeof( SHADOWVERTEX )*4,D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED ,&m_pVBufShadowSqu,NULL );
	if( FAILED( t_hr ) )
		osassert( false );

	if( !fill_screenSquareVB() )
		osassertex( false,"填充阴影屏幕多边形失败...\n" );


	// 初始化渲染状态块。
	record_ShadowVolumeStateBlock();
	record_ShadowSquareStateBlock();


	//! 初始化描边数据的缓冲区
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	m_sRenderPolygon.m_sBufData.m_iVbufId = m_ptrMiddlePipe->create_dynamicVB( 
		t_dwFvf,MAX_EDGE_PERMESH*2*(sizeof(os_sceVerDiffuse)),true );

	// 创建Index buffer数据。
	int   t_iGBufIdx = -1;
	WORD*    t_idxData = (WORD*)START_USEGBUF( t_iGBufIdx );
	for( int t_i=0;t_i<MAX_EDGE_PERMESH;t_i ++ )
	{
		t_idxData[t_i*6+0] = t_i*6;
		t_idxData[t_i*6+1] = t_i*6+1;
		t_idxData[t_i*6+2] = t_i*6+2;

		t_idxData[t_i*6+3] = t_i*6+3;
		t_idxData[t_i*6+4] = t_i*6+4;
		t_idxData[t_i*6+5] = t_i*6+5;

	}
	m_sRenderPolygon.m_sBufData.m_iIbufId = 
		m_ptrMiddlePipe->create_staticIB( t_idxData,MAX_EDGE_PERMESH*6 );
	osassert( m_sRenderPolygon.m_sBufData.m_iIbufId>= 0 );
	END_USEGBUF( t_iGBufIdx );

	m_sRenderPolygon.m_iShaderId = g_shaderMgr->add_shader( 
		"rain", osc_engine::get_shaderFile( "line" ).c_str() );
	if( m_sRenderPolygon.m_iShaderId < 0 )
	{
		m_sRenderPolygon.m_iShaderId = 
			g_shaderMgr->add_shadertomanager( LINESHADER_NAME,LINESHADER_INI );
	}
	osassert( m_sRenderPolygon.m_iShaderId>=0 );

	m_sRenderPolygon.m_iVerSize = sizeof( os_sceVerDiffuse );
	m_sRenderPolygon.m_iPriType = TRIANGLELIST;
	m_sRenderPolygon.m_bUseStaticVB = true;

	m_ptrFakeGlowData = new osc_fakeGlowData();

	return ;

	unguard;
}



//! 在屏幕上画我们的StencilSquare.
void osc_bpmShadow::draw_shadowSquare( void )
{
	guard;


	os_stateBlockMgr*  t_ptrIns = os_stateBlockMgr::Instance();

	if( (m_iShadowSquareState>=0)&&
		t_ptrIns->validate_stateBlockId( m_iShadowSquareState ) )
	{
		t_ptrIns->apply_stateBlock( m_iShadowSquareState );
	}
	else
	{
		record_ShadowSquareStateBlock();
		t_ptrIns->apply_stateBlock( m_iShadowSquareState );
	}

    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	if( (m_iShadowSSRestore>=0)&&
		t_ptrIns->validate_stateBlockId( m_iShadowSSRestore ) )
	{
		t_ptrIns->apply_stateBlock( m_iShadowSSRestore );
	}
	else
	{
		record_ShadowSquareStateBlock();
		t_ptrIns->apply_stateBlock( m_iShadowSSRestore );
	}


	return;

	unguard;
}



//! 设置人物渲染fade glow.
void osc_skinMesh::set_fakeGlow( bool _glow,DWORD _color )
{
	guard;

	m_bFakeGlowDraw = _glow;
	m_dwFadeGlowColor = _color;

	if( osc_bpmShadow::m_ptrFakeGlowData )
		osc_bpmShadow::m_ptrFakeGlowData->resetNormal();

	if( m_bFakeGlowDraw )
	{
		osc_bpmShadow*  t_ptrShadow;
		for( int t_i=0;t_i<this->m_iNumParts;t_i ++ )
		{
			t_ptrShadow  = 
				m_vecBpmIns[t_i].get_shadowPtr();
			if( !t_ptrShadow )
				continue;
			t_ptrShadow->set_fakeGlow( _glow );
		}
	}

	return;

	unguard;
}



//! 推进要渲染的有即时阴影的skinMesh指针.
void osc_skinMeshMgr::push_rtShadowSmPtr( osc_skinMesh* _smPtr )
{
	guard;

	osassert( _smPtr->m_bShadowSupport );
	osassert( m_iRTShadowNum <= MAX_RTSHADOWNUM );


	m_ptrShadowSkinPtr[m_iRTShadowNum] = _smPtr;
	m_iRTShadowNum ++;
	osassertex( m_iRTShadowNum<=MAX_RTSHADOWNUM,"即时阴影的空间不够...\n" );
	
	unguard;
}


/** \brief
*  根据相机的位置和方向光的向量，来确认是否需要画人物的阴影
*
*  如果相机的位置在人物的阴影体内，则不需要画阴影，否则会出现错误的阴影。
*
*  算法描述：此检测是非精确检测,并且在相机离人物特别近的时候，可能会产生Bug.
*  
*  1：根据场景内方向光的方向和相机的位置，算出一条指向方向光方向的射线
*  2: 检测此射线是否跟人物的包围盒相交。
*  3: 如果不跟人物的包围盒相交，则相机所在的位置不在人物的阴影体内。
* 
*/
BOOL osc_skinMeshMgr::is_cameraInShadowVolume( void )
{
	guard;

	osVec3D        t_vec3RayStart;
	os_bbox*       t_ptrBox;
	float          t_fLen;

	g_ptrCamera->get_curpos( &t_vec3RayStart );

	// 如果相机在阴影体内，不需要渲染
	for( int t_idx=0;t_idx<m_iRTShadowNum;t_idx ++ )
	{
		t_ptrBox = m_ptrShadowSkinPtr[t_idx]->get_smBBox();

		if( t_ptrBox->ray_intersect( t_vec3RayStart,g_vec3LPos,t_fLen ) )
			return TRUE;
	}

	return FALSE;

	unguard;
}

/** \brief
 *  相机的近剪切面是否剪切到人物的投射阴影体，大致检测
 *
 *  使用人物的bbox按阴影投射长度投射的阴影被相机的近剪切面剪切，也不需要渲染阴影
 */
BOOL osc_skinMeshMgr::is_nearCamPlaneClipSV( float _length,int _shaIdx )
{
	guard;

	const osVec3D*     t_ptrVer;
	os_ViewFrustum*    t_ptrFrus = g_ptrCamera->get_frustum();

	t_ptrVer = m_ptrShadowSkinPtr[_shaIdx]->get_smBBox()->get_worldVertexPtr();
	osassert( t_ptrVer );


	osVec3D  t_vec3CenterPt;
	for( int t_i=0;t_i<8;t_i +=2 )
	{
		t_vec3CenterPt = t_ptrVer[t_i] + t_ptrVer[t_i+1];
		t_vec3CenterPt /= 2.0f;
		if( t_ptrFrus->ray_intersectNearClipPlane( t_vec3CenterPt,-g_vec3LPos ) )
			return TRUE;
	}
	t_vec3CenterPt = t_ptrVer[7] + t_ptrVer[0];
	t_vec3CenterPt /= 2.0f;
	if( t_ptrFrus->ray_intersectNearClipPlane( t_vec3CenterPt,-g_vec3LPos ) )
		return TRUE;
	
	for( int t_i=0;t_i<8;t_i ++ )
	{
		if( t_ptrFrus->ray_intersectNearClipPlane( (osVec3D&)t_ptrVer[t_i],-g_vec3LPos ) )
			return TRUE;
	}

	return FALSE;

	unguard;
}

//! 相机位置和焦点的射线是否被地形挡住
BOOL osc_skinMeshMgr::is_camFocusWardByTerr( void )
{
	guard;

	bool       t_bIntersect;
	osVec3D    t_vec3CamPos,t_vec3CamDir,t_vec3InterPos;
	osc_TGManager*  t_ptrTgMgr = (osc_TGManager*)::get_sceneMgr();


	osassert( t_ptrTgMgr );

	g_ptrCamera->get_curpos( &t_vec3CamPos );
	g_ptrCamera->get_godLookVec( t_vec3CamDir );


	t_bIntersect = t_ptrTgMgr->get_rayInterTerrPos( t_vec3CamPos,t_vec3CamDir,t_vec3InterPos );

	// 如果相交，查看交点是否在focus之前
	if( t_bIntersect )
	{
		float   t_fLength1,t_fLength2;
		g_ptrCamera->get_camFocus( t_vec3CamDir );
		t_fLength1 = osVec3Length( &(t_vec3CamDir-t_vec3CamPos ) );
		t_fLength2 = osVec3Length( &(t_vec3InterPos - t_vec3CamPos) );

		if( t_fLength1 > t_fLength2 )
			return TRUE;
	}


	return FALSE;

	unguard;
}



//! 使用ZPass阴影时用到的测试函数
void osc_skinMeshMgr::zpass_process( void )
{
	guard;

	// 计算是否需要画人物的阴影
	if( is_cameraInShadowVolume() )
		return;
	// 
	// 计算shadow应该投射的长度，跟当前人物周围的地形相关，使用tgMgr内的接口来处理
	// 使用第一个接口处理。对每一个阴影人物进行处理。
	for( int t_idx = 0;t_idx < m_iRTShadowNum;t_idx ++ )
	{
		osVec3D        t_vec3RayStart,t_vec3RayDir,t_vec3ResPos;
		const osVec3D  *t_ptrVec3Max,*t_ptrVec3Min;
		os_bbox*       t_ptrBox;
		osc_bpmShadow*  t_ptrShadow = NULL;

		//! river added @ 2009-8-26:如果需要隐藏实时的人物阴影,阴影投射长度为零.
		if( m_ptrShadowSkinPtr[t_idx]->m_bHideRealShadow )
		{
			for( int t_i = 0 ;t_i<m_ptrShadowSkinPtr[t_idx]->m_iNumParts;t_i ++ )
			{
				t_ptrShadow = m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_shadowPtr();
				if( t_ptrShadow )
					t_ptrShadow->set_shadowProjLength( 0.0f );
			}

			// 下一个处理.
			continue;
		}

		osc_TGManager*  t_ptrTgMgr = (osc_TGManager*)::get_sceneMgr();
		osassert( t_ptrTgMgr );


		t_ptrBox = m_ptrShadowSkinPtr[t_idx]->get_smBBox();
		t_ptrVec3Max = t_ptrBox->get_vecmax();
		t_ptrVec3Min = t_ptrBox->get_vecmin();
		t_vec3RayStart = *(t_ptrBox->get_bbPos());
		t_vec3RayStart.x += (t_ptrVec3Max->x+t_ptrVec3Min->x)/2.0f;
		t_vec3RayStart.z += (t_ptrVec3Max->z+t_ptrVec3Min->z)/2.0f;
		// 有时候人物的bbox为零，主要针对这种情况加入的处理
		if( t_ptrVec3Max->y > 0.1f ) 
			t_vec3RayStart.y += t_ptrVec3Max->y;
		else  
			t_vec3RayStart.y += 1.5f;

		// 
		// 如果找不到交点，就让影子使用默认的8.0f这个投影长度
		float   t_fLength = DEFAULT_PROJSHADOWLENGTH;
# if 0
		if( t_ptrTgMgr->get_rayInterTerrPos( t_vec3RayStart,-g_vec3LPos,t_vec3ResPos ) )
			t_fLength = osVec3Length( &(t_vec3RayStart-t_vec3ResPos) );
# else
		BOOL   t_bPickBridge;
		t_fLength = t_ptrTgMgr->get_pickRayLength( 
			t_bPickBridge,t_vec3RayStart,-g_vec3LPos,t_vec3ResPos );
		if( t_bPickBridge )
			t_fLength *= 1.05f;
		else
			t_fLength *= 1.2f;

		// River @ 2009-5-14:如果阴影投影过长，则不如不显示阴影.
		if( t_fLength >= 12.0f )
			t_fLength = 0.0f;
# endif 

		//
		// 检测如果阴影体投射与相机的近剪切面相交，则不渲染阴影体
		if( is_nearCamPlaneClipSV( t_fLength,t_idx ) )
			t_fLength = 0.0f;

		for( int t_i = 0 ;t_i<m_ptrShadowSkinPtr[t_idx]->m_iNumParts;t_i ++ )
		{
			t_ptrShadow = m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_shadowPtr();
			if( t_ptrShadow )
				t_ptrShadow->set_shadowProjLength( t_fLength );
		}

	}

	unguard;
}


/**  River @ 2009-5-10:
   人物的阴影是否淡入淡出，因为全局只画一次阴影的square,所以一个人物
   的阴影淡入淡出会影响所有的人物阴影淡入淡出，所以此功能仅仅用于强调
   单个人物的时候,使用完成后，尽快关掉此值。
*/
OSENGINE_API bool                   g_bShadowFade = false;


//! 渲染skinMesh中有阴影那一个
void osc_skinMeshMgr::render_shadowSkin( void )
{
	guard;

	m_pd3dDevice->SetTransform( 
		D3DTS_WORLD,&osc_skinMesh::m_smatIdentity );

# if STENCIL_ZPASS
	zpass_process();
# endif 

# if 0
	// ATTENTION: 
	// 在运行场景内，只有主角人物自己有阴影
	// 如果相机的位置和焦点射线被地形挡住，则不渲染人物的阴影
	if( is_camFocusWardByTerr() )
		return ;
# endif 

	// 
	// River @ 2009-7-20:统一状态块
	osc_bpmShadow::start_shadowVolumeDraw();

	//
	// 更新渲染每一个人物和部位的影子
	for( int t_idx = 0;t_idx < this->m_iRTShadowNum;t_idx ++ )
	{

		// 隐藏和透明状态下,不渲染人物的阴影
		if( m_ptrShadowSkinPtr[t_idx]->m_bHideSm )
			continue;

		// 
		// River @ 2009-5-10:
		// 如果处于非shadowFadeState,则需要判断不需要画阴影的情形
		if( !g_bShadowFade )
			if( m_ptrShadowSkinPtr[t_idx]->m_bFadeState )
				if( m_ptrShadowSkinPtr[t_idx]->m_fAlphaVal < 1.0f )
					continue;


		for( int t_i = 0 ;t_i<m_ptrShadowSkinPtr[t_idx]->m_iNumParts;t_i ++ )//
		{
			osc_bpmShadow*  t_ptrShadow  = 
				m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_shadowPtr();

			// River @ 2010-7-1:为了在某些动作下隐藏武器
			if( m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_hideState() )
				continue;

			if( !t_ptrShadow )
				continue;
			bool t_b = t_ptrShadow->draw_shadowVolume();
			osassertex( t_b,
				va( "The skinMesh name is:<%s>,Body part num<%d>,Current <%d> part....\n",
				m_ptrShadowSkinPtr[t_idx]->m_strSMDir,
				m_ptrShadowSkinPtr[t_idx]->m_iNumParts,t_i ) );
		}
	}

	osc_bpmShadow::end_shadowVolumeDraw();

	
	// 画屏幕阴影矩形
	if( m_iRTShadowNum>0 )
		osc_bpmShadow::draw_shadowSquare();
	

	unguard;
}


