///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: osCharacterShadow.cpp
 * 
 *  His:      River created @ 2004-12-11
 *
 *  Desc:     ������������Ӱ��صĺ�����
 * 
 *  "���ж��ĳɾͣ���Ҫ�ܵ���������" 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"

//! ���б��ļ���Ӧ����չ��
# define EDGEFILE_EXTENT   ".edg"

//! River added @ 2008-11-19:���ڿ���������Ӱ��͸���ȣ�0��ȫ͸��.1��ȫ��͸��
OSENGINE_API float                  g_fShadowChroma = 0.2f;

//! �豸����ʱ����Ӱ���ֵ��룬ʹ�õ��ٽ���
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

��Ӱ��ʹ��������

Ŀǰ������������ļ�ʱ��Ӱʹ��Stencil�ķ�ʽ���С��տ�ʼ��ʱ��ʹ��ZPass�ķ�ʽ��

2006-5-12�޸�Ϊʹ��ZFail�ķ�ʽ��

ZPass�ķ�ʽ��Ҫ�������ڵ����������е�����ShadowVolume���������Ӱ�������ʾ
����Ļ������һЩ����

ZFail�Ĵ�������Զ������Ҳ����е������ShadowVolume,�����������Ӱ����ʾһ��С����
����������Ӱ�Ķ��ǳ�С��Ʒ���ϱ�ZPass�����Ĵ���Ҫ�úܶࡣ
ZFail����ӰͶ�����ʹ����һ���ǳ����ֵ��Ϊ������ShadowVolume,
��Զ�������ڶ���ȷ��д��StencilBuf.

ZFail������£�
������Ⱦ���ﳯ�������棬��Ȼ�����͸��ɽ���Կ���������Ӱ������

��������Ϊ������治�Ƿ�յ��棬���Ծ�����Ⱦ���ﳯ�������棬



��ɽ��ס�����ʱ����Ȼ���ٲ��ֵ���Ӱ������֡����������ʱû�н����
�����ֽ����������һ��ͨ�������������λ�ú͵��εĿɼ��ԣ������ʱ��������
�����Ⱦ�������Ӱ���ڶ������������ʹ������ɽ�嵲ס��������Ρ�
River @ 2006-5-13:
���յĽ�����������λ�úͽ��������֮������е��ε�ס������Ⱦ�������Ӱ��


Ϊ��ʹ���ﲻ��������Ӱ�������Ȼ��������Ӱ��Ȼ���ٻ�����൱����������������Ӱ
���ֵ���Ӱ��Ƭ��ס�ˡ�

**/

struct SHADOWVERTEX
{
    osVec4D     m_vec4Pos;
    DWORD       m_dwDiffuse;
};

//! ����shadowSquare����Ⱦ
#define   SHADOWVERTEX_FVF     (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

//! ʹ��Zpass����Ӱ��Ⱦ
# define STENCIL_ZPASS    1


//! ��ʼ����̬����
LPDIRECT3DDEVICE9 osc_bpmShadow::m_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9  osc_bpmShadow::m_pVBufShadowSqu = NULL;


//@{
//! ��Ⱦ��Ӱʱ�õ�����Ⱦ״̬������
int osc_bpmShadow::m_iSHDrawState = -1;
int osc_bpmShadow::m_iSHDrawRestoreState = -1;
int osc_bpmShadow::m_iShadowSquareState = -1;
int osc_bpmShadow::m_iShadowSSRestore = -1;
/** \brief
*  ������Ⱦ״̬��ʹ��Ⱦ��ʱ��ֻ��Ⱦ������Ϣ����zbuffer,����ʾ��
* 
*  ����Ӱ��������Ҫ����Ⱦ������zbuffer,����ʹ��������û����Ӱ��Ч����
*/
int osc_bpmShadow::m_iOnlyZbufSB = -1;
int osc_bpmShadow::m_iRestoreOnlyZbufSB = -1;
//@}

//! ��Ļ��ȣ���Ļ�߶�
float osc_bpmShadow::m_fScrWidth = 800.0f;
float osc_bpmShadow::m_fScrHeight = 600.0f;



//! Ĭ����ӰͶ��ĳ���,ʹ�ô������������������Զ�����洦�Ĵ���
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

	//!  Ҫ��Ⱦ�ıߵ��б�
	m_arrEdgeList = NULL;
	m_iEdgeNum = 0;

	//! ��
	m_pShadowVBuf = NULL;
	m_iShadowVerNum = 0;

	m_pSourceIdxBuf = NULL;
	m_iSourceTriNum = 0;

	//! ����ShadowVolume.
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

//! ��ʼ����Ӱ���ݽṹ����
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
	// syq : �Ӵ�Ϊ2���Ļ�������С�����ɴ��뵱����������Ե��ʱ���������᲻��
	//m_arrEdgeList = new WORD[ m_iIdxNum ];
	m_arrEdgeList = new WORD[ m_iIdxNum * 2 ];
	//@}

	t_strName += EDGEFILE_EXTENT;
	// tzz add:
	// ʹ��CReadFile ����ȡ����,
	// ��ֹ��osassert �׳��쳣֮��û�� END_USEGBUF����̫���osassert��
	// ���³�����������ֹ,��Ҫ����Դ����������ֹ
	//
	// �鿴 CReadFile ��ø������Ϣ
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

	// �������ΰ汾�ţ�
	//READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );
	//READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );
	t_file.ReadBuffer(&t_size,sizeof(int));
	t_file.ReadBuffer(&t_size,sizeof(int));


	// ����ÿһ�������ζ�Ӧ���ڽ���Ϣ��
	//READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );
	t_file.ReadBuffer(&t_size,sizeof(int));

	osassertex( t_size == (this->m_iIdxNum/3),
		va("������ļ�Ϊ:<%s> <%d %d>..\n",t_str,t_size,m_iIdxNum ) );
	
// 	READ_MEM_OFF( this->m_arrNeiList,
// 		t_fstart,sizeof( os_neighbour )*t_size );
	t_file.ReadBuffer(this->m_arrNeiList,sizeof( os_neighbour )*t_size);

	//END_USEGBUF( t_iGBufIdx );
	t_file.CloseFile();

	//
	// ��ʼ���豸��ص����ݣ����������������������
	// River @ 2005-8-18:ԭ����������Ӱ���㻺������ԭ��idxNum����������������⡣
	// River @ 2006-5-12: ʹ��ZFail����Ӱ��Ⱦ����Ҫ�ѳ�����������Ⱦ�ˡ�
	osassert( m_pd3dDevice );
	HRESULT    t_hr;
	t_hr = m_pd3dDevice->CreateVertexBuffer( 
		sizeof( osVec3D )*(m_iIdxNum*MAX_SHADERVERMULTI + m_iVerNum),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		0, D3DPOOL_DEFAULT,&m_pShadowVBuf,NULL );
	osassert( !FAILED( t_hr ) );
	
	// �����������弸�����ݵ������׳���
	t_hr = m_pd3dDevice->CreateIndexBuffer( 
		sizeof( WORD )*m_iIdxNum,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_pSourceIdxBuf,NULL );
	osassert( !FAILED( t_hr ) );
	m_iSourceTriNum = 0;


	return true;

	unguard;
}

//! �豸��reset��غ���.
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

	//! restore����.
	if( m_pShadowVBuf )
		return;

	HRESULT    t_hr;
	t_hr = m_pd3dDevice->CreateVertexBuffer( 
		sizeof( osVec3D )*( m_iIdxNum*MAX_SHADERVERMULTI + m_iVerNum),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		0, D3DPOOL_DEFAULT,&this->m_pShadowVBuf,NULL );
	if( FAILED( t_hr ) )
		osassertex( false,"��Ӱreset Deviceʧ��...\n" );

	// �����������弸�����ݵ������׳���
	t_hr = m_pd3dDevice->CreateIndexBuffer( 
		sizeof( WORD )*m_iIdxNum,D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_pSourceIdxBuf,NULL );
	osassert( !FAILED( t_hr ) );


	//! ���������Ļ����εĻ��������п����޸���Ļ��С
	if( !fill_screenSquareVB() )
		osassertex( false,"��Ӱ����Ļ���������ʧ��...\n" );

	unguard;
}



//! �ͷŵ�ǰ����Ӱ������ݡ�
void osc_bpmShadow::release_shadowData( void )
{
	guard;

	// ATTENTION TO OPT @ 2009-4-30:
	// ���ʹ��ȫ�ֵ�һ������������Ȼ��ǳ��ķ�ʱ�䡣
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

//! �����б��м���һ���ߡ�
void osc_bpmShadow::push_edge( WORD _verIdxs,WORD _verIdxe )
{
	guard;
	
	// syq mod @ 05.3.4��������Ч����������
	for( int t_i=0;t_i<this->m_iEdgeNum;t_i ++ )
	{

		if( ((m_arrEdgeList[t_i*2] == _verIdxs)&&
			(m_arrEdgeList[t_i*2+1] == _verIdxe)) ||
			((m_arrEdgeList[t_i*2] == _verIdxe)&&
			(m_arrEdgeList[t_i*2+1] == _verIdxs) ) )
		{
				osDebugOut( "��ģ�͵���Ӱ�����⣬�����ظ���:%d,%d\n", _verIdxs, _verIdxe );
				return;
		}
	}

	osassertex( (m_iEdgeNum < m_iIdxNum),"\n�ߵ����������������Ĵ�С!\n" );

	m_arrEdgeList[this->m_iEdgeNum*2] = _verIdxs;
	m_arrEdgeList[this->m_iEdgeNum*2+1] = _verIdxe;
	m_iEdgeNum ++;


	return;

	unguard;
}

//! ÿһ֡��ʼʱ�����赱ǰ��shadowVolume.
void osc_bpmShadow::reset_shadowVolume( void )
{
	guard;

	m_iEdgeNum = 0;
	m_iShadowVerNum = 0;
	
	memset( this->m_bFront,0,sizeof( bool )*m_iIdxNum/3 );


	unguard;
}

//! ����õ�������Ӱ��ģ�ͱߡ�
void osc_bpmShadow::get_shadowEdge( void )
{
	guard;

	for( int i=0; i<this->m_iIdxNum/3; i++ )
	{
		// �����ε����������ڻ������ڵ�����
		WORD t_wVerIdx0 = m_ptrBpmIdx[3*i+0];
		WORD t_wVerIdx1 = m_ptrBpmIdx[3*i+1];
		WORD t_wVerIdx2 = m_ptrBpmIdx[3*i+2];

		if( !this->m_bFront[i] )
		{
			for (int n = 0; n<3; ++n)
			{
				int t_iEdgeNeiIdx = m_arrNeiList[i].m_wN[n];
				//�������е��棬���������Դ�������ߣ����������ֱ���Ҫ���
				//�ѱ߽�ı߼����������б���
				if( t_iEdgeNeiIdx==0xffff ) // no neighbor
				{
					if(n==0) push_edge( t_wVerIdx0, t_wVerIdx1 );
					if(n==1) push_edge( t_wVerIdx1, t_wVerIdx2 );
					if(n==2) push_edge( t_wVerIdx2, t_wVerIdx0 );
					continue;
				}

				//�������Դ�ͱ����Դ�ļб߼��������б���
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
	// River @ 2010-10-9:���ܺ���ԭ�򣬴˴���������಻��Ⱦ��Ӱ���ɡ�

	unguard;
}




//! �õ�ģ����ÿһ���������뷽���Ĺ�ϵ����Ի��Ǳ��ԣ�
void osc_bpmShadow::cal_triToLight( osVec3D* _lightDir/* = NULL*/ )
{
	guard;

	osVec3D   t_vec3V0,t_vec3V1,t_vec3V2;

	osVec3D   t_vec3LightDir = g_vec3LPos;

	//! ����������ݣ���ʹ�÷�ȫ�ֵĹ��շ���
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


		//! ʹ��ȫ�ֵķ�������ݣ�
		if( osVec3Dot( &t_vec3V2,&t_vec3LightDir )>= 0.0f )
			m_bFront[t_i] = false;
		else
			m_bFront[t_i] = true;
	}

	return;

	unguard;
}



//! �������µ�Ȩ�صĹ����ż�����Ӱ��������㡣
# define MIN_WEIGHT   0.0001f
//! ���ݴ���ľ�����и���ģ����ÿһ�������λ�á�
void osc_bpmShadow::update_verPos( osMatrix* _matArr,int _matNum )
{
	guard;
	
	osVec3D           t_vec3V0,t_vec3V1;
	os_characterVer    *t_ptrVer;
	float             t_fWei;

	for( int t_i=0;t_i<m_iVerNum;t_i ++ )
	{
		t_ptrVer = &m_ptrBpmVer[t_i];


		//������ͷ�����һ������
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

		// �ڶ���������
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

		//������ͷ�����һ������
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

		// �ڶ���������
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


//! fakeGlow��أ�������ı�������䵽���㻺�����ڡ�
void osc_bpmShadow::get_edgeVertexData( osVec3D& _vpVec )
{
	guard;

	int   t_iGBufIdx = -1;
	os_sceVerDiffuse*   t_ptrVStart  = (os_sceVerDiffuse*)START_USEGBUF( t_iGBufIdx );

	osassert( m_vec3Normal );

	// 
	// Extrude�߶�Ӧ�Ķ�������
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


//! ��ģ���е���Ӱ��Ե����ת��Ϊ�������ݣ�����䵽���㻺�����С�
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
		// River @ 2011-3-19:���lockʧ�ܣ����ؾ�OK��
		m_iShadowVerNum = 0;
		return;
	}

	osassertex( (MAX_SHADERVERMULTI*m_iIdxNum) >= (m_iEdgeNum*6),"��Ӱ������С�ڶ�����Ŀ" );

	// ����extrude�߶�Ӧ�Ķ�������
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

	// copy����ת���������ζ���
	memcpy( &t_vec3D[m_iEdgeNum*6],m_ptrBpmVerAT,sizeof( osVec3D )*m_iVerNum );

	m_pShadowVBuf->Unlock();

	m_iShadowVerNum = m_iEdgeNum*6;

	// River @ 2010-12-21��ȥ��assert,ֱ�ӷ��ء�
	if( m_iShadowVerNum <= 0 )
		return;

	//
	// �����Է�����������
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




//! ÿһ֡����Ӱ���ݵĸ���
void osc_bpmShadow::update_shadow( osMatrix* _matArr,int _matNum )
{
	guard;

	//! �����赱ǰ��shadowVolume.
	reset_shadowVolume();

	//! ���¶���λ��
	update_verPos( _matArr,_matNum );

	//! ����ÿһ��ģ���е���������Ĺ�ϵ��
	cal_triToLight();

	//! �õ����Բ�����Ӱ�ı��б�
	get_shadowEdge();

	//! ���ݱ��б��������Ⱦ��Ӱ�Ķ��㻺������
	get_shadowVertexData();


	m_bUpdated = true;

	return;

	unguard;
}


/* \brief
*  ��fadeGlow�Ľӿڣ����Ч��
* \param DWORD _color        Ҫ����fadeGlow����ɫ����
* \param DWORD _colorNoise   ��ɫ���ݵ��������ֵ,����ɫ�Ļ�����������������ֵ
* \param osVec3D& _vpVec     ��ĳһ��������ߣ�����Ӱ����ʱ���ƹ�ķ���
* \param float _length       ������ݵõ���������ɢ�ı�Ե���ݵĳ��ȡ�
* \param float _lenthNoise   ��Ե���ݳ��ȵ����ֵ��
* \param bool _projParallel  �����ֵΪ�棬��˳��Ͷ�䷽����ǰ��������ݣ�����ֱ����
*/
bool osc_bpmShadow::draw_fakeGlow( DWORD _color,DWORD _colorNoise,
								  osVec3D& _vpVec,float _length,
								  float _lenthNoise,bool _projParallel )
{
	guard;


	//! �����赱ǰ��shadowVolume.
	reset_shadowVolume();

	//! ����Ӱ�����У��Ѿ������˶���λ��
	//  update_verPos( _matArr,_matNum );

	//! ����ÿһ��ģ���е���������Ĺ�ϵ��
	cal_triToLight( &_vpVec );

	//! �õ����Բ�����Ӱ�ı��б�
	get_shadowEdge();

	//! ���ݱ��б��������Ⱦ��Ӱ�Ķ��㻺������
	// get_shadowVertexData();
	//  
	// ����м�ܵ��Ķ��㻺������Ȼ��DrawTriangleList
	get_edgeVertexData( _vpVec );

	m_ptrMiddlePipe->render_polygon( &m_sRenderPolygon );

	return true;

	unguard;
}


//! ��ʼ��ShadowVolume.
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
		// ���һ��stateBlock��Ч,����һ���϶�Ҳ����Ч,��������һ��record.
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
		// ���һ��stateBlock��Ч,����һ���϶�Ҳ����Ч,��������һ��record.
		record_ShadowVolumeStateBlock();
		t_ptrSBM->apply_stateBlock( m_iSHDrawRestoreState );
	}

	unguard;
}


//  
//! ATTENTION:7500LE���Կ���������������û��DIP����ػ���
//! Shadow��Ⱦ�Ľӿڡ�
bool osc_bpmShadow::draw_shadowVolume( void )
{
	guard;

	osassert( m_pd3dDevice );


	// û����Ҫ������Ӱ����ͱ�
	if( m_iShadowVerNum <= 0 )
		return true;
	
	//! river @ 2010-3-9:�п��ܳ���Ϊ������Ρ�
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
	// ���������ϵļ�������
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		m_iShadowVerNum,0,m_iVerNum,0,this->m_iSourceTriNum );
# endif 

//
# if STENCIL_ZPASS
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
# else
	// zfail ��ʽ
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
	m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT );
# endif 


	// Draw back-side of shadow volume in stencil/z only
	// extrude triangles.
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,0,m_iShadowVerNum/3 );

	// 
	// ��������ԭ�еļ�������
	m_pd3dDevice->SetIndices( m_pSourceIdxBuf );
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		m_iShadowVerNum,0,m_iVerNum,0,this->m_iSourceTriNum );


	return true;

	unguard;
}


//! ¼����Ⱦ��Ӱ��Ҫ����Ⱦ״̬�顣
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
		// River added @ 2005-7-11: ��Ӱ��alpha��Դ����ȷ�ϣ���Ȼ��������ʱ���
		//                          ����Ӱʱ��ʱ�ֵ����⡣
		// River @ 2006-5-13: ʹ��D3DRS_COLORWRITEENABLE���Բ����������״̬�����⣬
		//                    Ϊ��ȷ�ϣ��ݲ�ȥ���˴���״̬��
		m_pd3dDevice->SetTexture( 0 , NULL );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,D3DTOP_DISABLE );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

		// Disable z-buffer writes (note: z-testing still occurs), and enable the
		// stencil-buffer
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );

		// Zfunc���õĸ��ϸ�һЩ
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

		// �������ʹ����ɫ
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
	// Shadow Square��ص���Ⱦ״̬��
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

		// ���ڽ����ĳЩ����£�����Ӱ�ӱ�ڵ����⡣
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


//! ȫ�ֵ��ͷ���Ӱϵͳ.
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

//! ����ֻ��ȾzBuffer���豸״̬��
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


//! ����fake glow
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


//! ����ȫ�ֵ���Ӱ͸����
void osc_bpmShadow::reset_shadowColor( float _alpha )
{
	guard;

	HRESULT   t_hr;

	osassert( _alpha >= 0.0f );

	SHADOWVERTEX*   t_verS;

	// ��䶥�㻺������
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

//! ȫ�ֵģ��������������ݵ�dynamic vb idx.
int osc_bpmShadow::m_iDynamicVBIdx = -1;
osc_middlePipe* osc_bpmShadow::m_ptrMiddlePipe = NULL;
os_polygon osc_bpmShadow::m_sRenderPolygon;


//! �����Ļ����εļ�������
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

	// ��䶥�㻺������
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


//! ȫ�ֵĳ�ʼ�����ǵ���Ӱϵͳ��
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
		osassertex( false,"�����Ӱ��Ļ�����ʧ��...\n" );


	// ��ʼ����Ⱦ״̬�顣
	record_ShadowVolumeStateBlock();
	record_ShadowSquareStateBlock();


	//! ��ʼ��������ݵĻ�����
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	m_sRenderPolygon.m_sBufData.m_iVbufId = m_ptrMiddlePipe->create_dynamicVB( 
		t_dwFvf,MAX_EDGE_PERMESH*2*(sizeof(os_sceVerDiffuse)),true );

	// ����Index buffer���ݡ�
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



//! ����Ļ�ϻ����ǵ�StencilSquare.
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



//! ����������Ⱦfade glow.
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



//! �ƽ�Ҫ��Ⱦ���м�ʱ��Ӱ��skinMeshָ��.
void osc_skinMeshMgr::push_rtShadowSmPtr( osc_skinMesh* _smPtr )
{
	guard;

	osassert( _smPtr->m_bShadowSupport );
	osassert( m_iRTShadowNum <= MAX_RTSHADOWNUM );


	m_ptrShadowSkinPtr[m_iRTShadowNum] = _smPtr;
	m_iRTShadowNum ++;
	osassertex( m_iRTShadowNum<=MAX_RTSHADOWNUM,"��ʱ��Ӱ�Ŀռ䲻��...\n" );
	
	unguard;
}


/** \brief
*  ���������λ�úͷ�������������ȷ���Ƿ���Ҫ���������Ӱ
*
*  ��������λ�����������Ӱ���ڣ�����Ҫ����Ӱ���������ִ������Ӱ��
*
*  �㷨�������˼���ǷǾ�ȷ���,����������������ر����ʱ�򣬿��ܻ����Bug.
*  
*  1�����ݳ����ڷ����ķ���������λ�ã����һ��ָ����ⷽ�������
*  2: ���������Ƿ������İ�Χ���ཻ��
*  3: �����������İ�Χ���ཻ����������ڵ�λ�ò����������Ӱ���ڡ�
* 
*/
BOOL osc_skinMeshMgr::is_cameraInShadowVolume( void )
{
	guard;

	osVec3D        t_vec3RayStart;
	os_bbox*       t_ptrBox;
	float          t_fLen;

	g_ptrCamera->get_curpos( &t_vec3RayStart );

	// ����������Ӱ���ڣ�����Ҫ��Ⱦ
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
 *  ����Ľ��������Ƿ���е������Ͷ����Ӱ�壬���¼��
 *
 *  ʹ�������bbox����ӰͶ�䳤��Ͷ�����Ӱ������Ľ���������У�Ҳ����Ҫ��Ⱦ��Ӱ
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

//! ���λ�úͽ���������Ƿ񱻵��ε�ס
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

	// ����ཻ���鿴�����Ƿ���focus֮ǰ
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



//! ʹ��ZPass��Ӱʱ�õ��Ĳ��Ժ���
void osc_skinMeshMgr::zpass_process( void )
{
	guard;

	// �����Ƿ���Ҫ���������Ӱ
	if( is_cameraInShadowVolume() )
		return;
	// 
	// ����shadowӦ��Ͷ��ĳ��ȣ�����ǰ������Χ�ĵ�����أ�ʹ��tgMgr�ڵĽӿ�������
	// ʹ�õ�һ���ӿڴ�����ÿһ����Ӱ������д���
	for( int t_idx = 0;t_idx < m_iRTShadowNum;t_idx ++ )
	{
		osVec3D        t_vec3RayStart,t_vec3RayDir,t_vec3ResPos;
		const osVec3D  *t_ptrVec3Max,*t_ptrVec3Min;
		os_bbox*       t_ptrBox;
		osc_bpmShadow*  t_ptrShadow = NULL;

		//! river added @ 2009-8-26:�����Ҫ����ʵʱ��������Ӱ,��ӰͶ�䳤��Ϊ��.
		if( m_ptrShadowSkinPtr[t_idx]->m_bHideRealShadow )
		{
			for( int t_i = 0 ;t_i<m_ptrShadowSkinPtr[t_idx]->m_iNumParts;t_i ++ )
			{
				t_ptrShadow = m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_shadowPtr();
				if( t_ptrShadow )
					t_ptrShadow->set_shadowProjLength( 0.0f );
			}

			// ��һ������.
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
		// ��ʱ�������bboxΪ�㣬��Ҫ��������������Ĵ���
		if( t_ptrVec3Max->y > 0.1f ) 
			t_vec3RayStart.y += t_ptrVec3Max->y;
		else  
			t_vec3RayStart.y += 1.5f;

		// 
		// ����Ҳ������㣬����Ӱ��ʹ��Ĭ�ϵ�8.0f���ͶӰ����
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

		// River @ 2009-5-14:�����ӰͶӰ���������粻��ʾ��Ӱ.
		if( t_fLength >= 12.0f )
			t_fLength = 0.0f;
# endif 

		//
		// ��������Ӱ��Ͷ��������Ľ��������ཻ������Ⱦ��Ӱ��
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
   �������Ӱ�Ƿ��뵭������Ϊȫ��ֻ��һ����Ӱ��square,����һ������
   ����Ӱ���뵭����Ӱ�����е�������Ӱ���뵭�������Դ˹��ܽ�������ǿ��
   ���������ʱ��,ʹ����ɺ󣬾���ص���ֵ��
*/
OSENGINE_API bool                   g_bShadowFade = false;


//! ��ȾskinMesh������Ӱ��һ��
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
	// �����г����ڣ�ֻ�����������Լ�����Ӱ
	// ��������λ�úͽ������߱����ε�ס������Ⱦ�������Ӱ
	if( is_camFocusWardByTerr() )
		return ;
# endif 

	// 
	// River @ 2009-7-20:ͳһ״̬��
	osc_bpmShadow::start_shadowVolumeDraw();

	//
	// ������Ⱦÿһ������Ͳ�λ��Ӱ��
	for( int t_idx = 0;t_idx < this->m_iRTShadowNum;t_idx ++ )
	{

		// ���غ�͸��״̬��,����Ⱦ�������Ӱ
		if( m_ptrShadowSkinPtr[t_idx]->m_bHideSm )
			continue;

		// 
		// River @ 2009-5-10:
		// ������ڷ�shadowFadeState,����Ҫ�жϲ���Ҫ����Ӱ������
		if( !g_bShadowFade )
			if( m_ptrShadowSkinPtr[t_idx]->m_bFadeState )
				if( m_ptrShadowSkinPtr[t_idx]->m_fAlphaVal < 1.0f )
					continue;


		for( int t_i = 0 ;t_i<m_ptrShadowSkinPtr[t_idx]->m_iNumParts;t_i ++ )//
		{
			osc_bpmShadow*  t_ptrShadow  = 
				m_ptrShadowSkinPtr[t_idx]->m_vecBpmIns[t_i].get_shadowPtr();

			// River @ 2010-7-1:Ϊ����ĳЩ��������������
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

	
	// ����Ļ��Ӱ����
	if( m_iRTShadowNum>0 )
		osc_bpmShadow::draw_shadowSquare();
	

	unguard;
}


