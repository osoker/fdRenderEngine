//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdBillboard.cpp
 *
 *  Desc:     ���������е�billBoard,������ʾ�����е���Ч.
 *
 *  His:      River created @ 2004-2-10
 *
 *  "The people who are crazy enough to think
 *   they can change the world, are the ones who do."
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/fdBillboard.h"
# include "../../backpipe/include/osCamera.h"
# include "../../mfpipe/include/osShader.h"
# include "../include/fdTrack.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../include/fdRain.h"
# include "../../mfPipe/include/osCharacter.h"
# include "../../backpipe/include/mainEngine.h"

# define  TRACKFILE_DIR       "csf\\"

# define  MAX_BILLBOARDNUM    256
# define  MAX_HUGEBBNUM       16

//! billBoard��Ҫ�ͷŵļ�������
# define  DISCARD_BBTIMES     2048
//! ÿ���೤ʱ�䴦��һ��billBoard���ݵ��ͷ�
# define  PROCESS_DISCARDTIME  1200.0f

//! billBoard�ļ����ļ�ͷ��
# define BBMAGIC_HEADER   "BB"

# define CURBBF_VERSION   3
// River @ 2010-6-5:����ѡ���������zEnableΪfalse.
# define CURBBF_VERSION4  4
// River @ 2013-4-28:����ѡ����Զ�ȡuvNum.�������е����ݽṹ
# define CURBBF_VERSION5  5





osc_billBoard::osc_billBoard()
{
	reset_bb();
}


//! ��ʼ����ǰ�Ĺ��������
void osc_billBoard::reset_bb( void )
{
	m_bInuse = false;
	m_bUseRunTrack = false;

	m_bCanSwap = true;

	m_vecBBSizeAdj = osVec2D( 0.0f,0.0f );

	m_fTotalPlayTime = 0.0f;
	m_dwHash = 0;

	m_bRotOrNot = FALSE;

	m_bZEnable = TRUE;

}




/** \brief
*  ���ļ��е���һ��billBoard.
*  
*  �ȵ���billBoard��Ӳ�����ݣ�Ȼ�󴴽���ǰbillBoard��ص��豸������ݡ�
*  ʹ��ȫ�ֵ�shaderMgrָ�봴��ȫ�ֵ�shader���ݡ�
*  
*/ 
bool osc_billBoard::load_billBoardFromFile( const char* _fname )
{  
	guard;

	int               t_size,t_i;
	BYTE*             t_ptrBuf;
	char              t_szShaName[128];
	os_bbHeader        t_fHeader;

	osassert( (_fname)&&(_fname[0]!=NULL) );
	int t_iGBufIdx = -1;
	t_ptrBuf = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)_fname,t_ptrBuf,TMP_BUFSIZE );
	osassert( t_size>=0 );

	//
	// �����ļ�ͷ�Ľṹ��
	READ_MEM_OFF( &t_fHeader,t_ptrBuf,sizeof( os_bbHeader ) );
	if( strcmp( t_fHeader.m_chFlag,BBMAGIC_HEADER ) )
	{
		END_USEGBUF( t_iGBufIdx );
		return false;
	}

	// River mod @ 2013-4-2�� ֧�ְ汾5.
	if( t_fHeader.m_dwVersion > CURBBF_VERSION5 )
	{
		END_USEGBUF( t_iGBufIdx );
		char temp[250];
		sprintf(temp,"%s��֧�ֵ��ļ��汾��",_fname);
		osassertex(false,temp);
		//MessageBox( NULL,"��֧�ֵ��ļ��汾��","ERROR",MB_OK );
		return false;
	}

	//
	// ���빫�����ص����ݡ�
	READ_MEM_OFF( &m_billBoardType,t_ptrBuf,sizeof( int ) );

	READ_MEM_OFF( &m_bLoopBB,t_ptrBuf,sizeof( int ) );

	//
	// ATTENTION: Ϊʲôʹ�ó���������
	READ_MEM_OFF( &this->m_fFrameTime,t_ptrBuf,sizeof( float ) );
	if( m_fFrameTime < 0.001f )
		m_fFrameTime = 0.001f;

	READ_MEM_OFF( &this->m_iTotalFrame,t_ptrBuf,sizeof( int ) );

	READ_MEM_OFF( &this->m_iTotalTexNum,t_ptrBuf,sizeof( int ) );	
	READ_MEM_OFF( &this->m_strTextureName,t_ptrBuf,sizeof( char )*128 );

	READ_MEM_OFF( &t_i,t_ptrBuf,sizeof( int ) );
	this->m_vecTexId.resize( this->m_iTotalFrame );
	READ_MEM_OFF( &m_vecTexId[0],t_ptrBuf,sizeof( int )*m_iTotalFrame );

	READ_MEM_OFF( &t_i,t_ptrBuf,sizeof( int ) );
	this->m_vecBBSize.resize( this->m_iTotalFrame );
	READ_MEM_OFF( &this->m_vecBBSize[0],t_ptrBuf,sizeof( float )*m_iTotalFrame );

	READ_MEM_OFF( &t_i,t_ptrBuf,sizeof( int ) );
	this->m_vecBBColor.resize( this->m_iTotalFrame );
	READ_MEM_OFF( &this->m_vecBBColor[0],t_ptrBuf,sizeof( DWORD )*m_iTotalFrame );

	READ_MEM_OFF( &this->m_wAlphaType,t_ptrBuf,sizeof( WORD ) );
	READ_MEM_OFF( &this->m_fTexScale,t_ptrBuf,sizeof( float ) );

	READ_MEM_OFF( &this->m_bUseRunTrack,t_ptrBuf,sizeof( float ) );
	READ_MEM_OFF( this->m_strTrackFname,t_ptrBuf,sizeof( char )*128 );
	READ_MEM_OFF( &this->m_fTrackMoveSpeed,t_ptrBuf,sizeof( float ) );

	//
	// ����ļ��İ汾����1���ϡ�
	if( t_fHeader.m_dwVersion > 1 )
		READ_MEM_OFF( &this->m_vecBBSizeAdj,t_ptrBuf,sizeof( osVec2D ) );

	//
	// �����˹������ת�İ汾���޹�ΰ�ṩ��
	if( t_fHeader.m_dwVersion > 2 )
	{
		READ_MEM_OFF( &this->m_bRotOrNot,t_ptrBuf,sizeof( int ) );
		READ_MEM_OFF( &this->m_fRotSpeed,t_ptrBuf,sizeof( float ) );
		READ_MEM_OFF( &this->m_iRotDir,t_ptrBuf,sizeof( int ) ) ;
	}

	if( t_fHeader.m_dwVersion > 3 )
		READ_MEM_OFF( &m_bZEnable,t_ptrBuf,sizeof( int) );

	// River @ 2008-4-18:���µİ汾��4,�޸Ĺ����ģʽ������ʹ�ó���10�����ϵĹ��������

	END_USEGBUF( t_iGBufIdx );



	//
	// �ӵ���������д����豸��ص����ݡ�
	//osassert( m_iTotalFrame <= MAX_BBTEXNUM );���ܶ�֡��Ӧһ����������20ֻ֡��Ҫһ������
	osassert( m_iTotalTexNum <= MAX_BBTEXNUM );
	for( t_i=0;t_i< m_iTotalTexNum;t_i++ )	
	{
		// River mod @ 2010-6-7:ʹ����򵥵İ汾��֮ǰ��֪����ô�����޸��˸�ʽ��
		sprintf( t_szShaName,"%s%d",m_strTextureName,t_i+1 );
		
		m_iShaderId[t_i] = g_shaderMgr->add_shader( t_szShaName, 
			osc_engine::get_shaderFile( "billBoard" ).c_str() );
		if( m_iShaderId[t_i] < 0 )
		{
			m_iShaderId[t_i] = g_shaderMgr->add_shadertomanager( 
				t_szShaName,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
		}
	}


	// ���㲥�ŵ�ǰ�Ĺ�����ܹ���Ҫ��ʱ��.
	m_fTotalPlayTime = m_iTotalFrame*m_fFrameTime;

	//
	// ����Ϊ���ã����������ǵĵ����ļ����ļ�����
	m_bInuse = true;
	strcpy( m_szBFilename,_fname );
	m_dwHash = string_hash( _fname );

	return true;

	unguard;
}

//! �ͷ�һ��������ļ�.
void osc_billBoard::release_billBoard( void )
{
	guard;

	for( int t_i=0;t_i<this->m_iTotalTexNum;t_i ++ )
	{
		if( m_iShaderId[t_i]>=0 )
		{
			g_shaderMgr->release_shader( m_iShaderId[t_i] );
			m_iShaderId[t_i] = -1;
		}
	}

	m_bInuse = false;

	reset_bb();

	unguard;
}





osc_billBoardIns::osc_billBoardIns()
{
	m_ptrBBData = NULL;
	m_iCurFrame = 0;
	m_bInuse = false;

	m_bUseCustomColor = false;

	m_bInview = false;

	m_fTimeScale = 1.0f;

	m_vec3BBRotAxis = osVec3D( 0.0f,1.0f,0.0f );

	m_bHide = false;

	m_bInReleaseState = false;

}




//! ��ʼ��billBoard��instance�Ķ������ݡ�
void osc_billBoardIns::init_bbAniData( void )
{
	m_bInuse = true;
	m_fInteValue = 0.0f;
	m_fAccumulateTime = 0.0f;
	m_iCurFrame = 0;
	
	// 
	// River @ 2006-7-21:�ڵ�һ��FrameMoveʱ����ֵ��������Ϊ����Ǽ�
	m_bInview = false;

	m_fRotAgl = 0.0f;

	m_fSizeScale = 1.0f;

	// ��ʼ��hide����false.
	m_bHide = false;

	m_fBaseSizeScale = 1.0f;

	m_bInReleaseState = false;

}



/** \brief
 *  �Ե�ǰ��billBoard�������д���
 *
 *  \return bool �������false,���ʾ��ǰ��bb�Ѿ��ͷ�
 */
bool osc_billBoardIns::frame_moveBBIns( float _etime,bool _canReleaseIns/* = false*/ )
{
	guard;

	float           t_f,t_fTimeAfterScale;
	osVec3D         t_trackVec;

	//
	// River @ 2011-1-30:�������releasestate,�ͷţ���ô����Ϊ�˼���CS.������Ϸƽ���ȡ�
	if( m_bInReleaseState && _canReleaseIns )
	{
		release_billBoardInsInner();
		return false;
	}

	t_fTimeAfterScale = _etime*m_fTimeScale;
	t_f = t_fTimeAfterScale + m_fAccumulateTime;
	if( t_f>m_ptrBBData->m_fFrameTime )
	{
		m_iCurFrame += float_div( t_f,m_ptrBBData->m_fFrameTime ) ;


		if( m_iCurFrame >= (m_ptrBBData->m_iTotalFrame-1) )
		{
			// 
			// �����ѭ�����ŵ�BillBoard.
			if( !m_ptrBBData->m_bLoopBB )
			{
				release_billBoardInsInner();
				return false;
			}
			else
				m_iCurFrame %= m_ptrBBData->m_iTotalFrame;

		}

		m_fAccumulateTime = float_mod( t_f,m_ptrBBData->m_fFrameTime );

	}
	else
	{
		this->m_fAccumulateTime = t_f;
	}

	// �������Բ�ֵ�Ĳ�ֵϵ����
	m_fInteValue = m_fAccumulateTime/m_ptrBBData->m_fFrameTime;


	osassert( m_iCurFrame < m_ptrBBData->m_iTotalFrame );

	//
	// ATTENTION:
	// ����billBoard���˶�·������,BillBoard����ʹ���˶�·�����ж�����
	if( m_ptrBBData->m_bUseRunTrack )
	{
		osassert( t_fTimeAfterScale>=0.0f );
		g_ptrTrackMgr->get_aniVec( m_iTrackId,t_fTimeAfterScale,t_trackVec );
		m_vec3Fpos = m_vec3Pos + t_trackVec;
	}
	else
		m_vec3Fpos = m_vec3Pos;


	//
	// �����ǰ�Ĺ����Ins����ת��
	if( m_ptrBBData->m_bRotOrNot )
	{
		if( m_ptrBBData->m_iRotDir )
			m_fRotAgl -= t_fTimeAfterScale * m_ptrBBData->m_fRotSpeed;
		else
			m_fRotAgl += t_fTimeAfterScale * m_ptrBBData->m_fRotSpeed;
	}


	return true;

	unguard;
}


//! �õ������ʵ�����ݵ�UP&Right Vector.
void osc_billBoardIns::get_bbUpRightVec( 
				I_camera* _camptr,osVec3D& _up,osVec3D& _right )
{
	guard;

	osc_camera*          t_ptrCam;
	osPlane             t_plane = osPlane( 0,1,0,0 );
	osVec3D             t_upVec,t_rightVec;
	osVec3D             t_camPos;
	
	
	


	t_ptrCam = (osc_camera*)_camptr;

	t_ptrCam->get_curpos( &t_camPos );

	t_ptrCam->get_rightvec( t_rightVec );
	//t_ptrCam->get_godLookVec( t_upVec );
	t_upVec = m_vec3Fpos - t_camPos;
	osVec3Normalize( &t_upVec,&t_upVec );
	
	osVec3Cross( &t_upVec,&t_upVec,&t_rightVec );

	// 
	// ���ݲ�ͬ�Ĺ����������������ӵĶ��㡣
	switch( m_ptrBBData->m_billBoardType )
	{
	case BILLBOARD_XZ:
		osn_mathFunc::project_PtToPlane( t_plane,t_upVec,t_upVec );
		osVec3Normalize( &t_upVec,&t_upVec );
		osVec3Normalize( &t_rightVec,&t_rightVec );
		break;
	case BILLBOARD_Y:
		t_upVec = osVec3D( 0,1,0 );
		break;
	case BILLBOARD_AXIS:
		t_upVec = m_vec3BBRotAxis;
		break;
	case BILLBOARD_ALL:
		osVec3Normalize( &t_upVec,&t_upVec );
		break;
	case BILLBOARD_CROSS:
		// Ŀǰ�ݲ�֧��ʮ����Ƭ��billBoard.
		osassert( false );
		break;

	}

	// 
	// �õ���������ת���ݡ�
	if( (m_ptrBBData->m_bRotOrNot)&&
		(m_ptrBBData->m_billBoardType != BILLBOARD_Y) )
	{
		osMatrix   t_matRot;
		osVec3D    t_rotAxis;

		t_ptrCam->get_curpos( &t_rotAxis );
		t_rotAxis = m_vec3Pos - t_rotAxis;
		if( m_ptrBBData->m_billBoardType == BILLBOARD_XZ )
			t_rotAxis = osVec3D( 0.0f,1.0f,0.0f );

		osMatrixRotationAxis( &t_matRot,&t_rotAxis,m_fRotAgl );
		osVec3TransformNormal( &t_upVec,&t_upVec,&t_matRot );
		osVec3TransformNormal( &t_rightVec,&t_rightVec,&t_matRot );
	}


	//
	// ���¼��㵱ǰbillBoard�����ҵ�����
	// ʹ��up������˴������billBoard���ĵ��������
	if( (m_ptrBBData->m_billBoardType == BILLBOARD_Y)||
		(m_ptrBBData->m_billBoardType == BILLBOARD_ALL)||
		(m_ptrBBData->m_billBoardType == BILLBOARD_AXIS) )
	{
		osVec3D    t_camPos;
		t_ptrCam->get_curpos( &t_camPos );
		t_rightVec = m_vec3Fpos - t_camPos;
		osVec3Cross( &t_rightVec,&t_upVec,&t_rightVec );
		osVec3Normalize( &t_rightVec,&t_rightVec );
	}

	_up = t_upVec;
	_right = t_rightVec;

	return;

	unguard;
}



/** \brief
 *  ����billBoard��instance���������������Ⱦ�õĶ������ݡ�
 *
 *  ���ݲ�ͬ��billBoard������ʹ�ò�ͬ�ķ�ʽ��䶥�����ݡ�
 */
bool osc_billBoardIns::fill_renderVerData( I_camera* _camptr )
{
	guard;

	float               t_fSize;
	osColor             t_srcC,t_dstC,t_resC;
	int                 t_iNextFrame,t_i;
	osVec3D             t_upVec,t_rightVec;
	float               t_fWidth,t_fHeight;
	DWORD               t_dwFBBColor;
	static DWORD         t_sdwAlphaBlack = osColor( 0.0f,0.0f,0.0f,1.0f );


	osassert( _camptr );

	osassert( this->m_bInuse );

	//
	// �����ǰ�����һ֡����͵���֡���в�ֵ��
	if( m_iCurFrame == (m_ptrBBData->m_iTotalFrame-1) )
		t_iNextFrame = 0;
	else
		t_iNextFrame = m_iCurFrame+1;

	// 
	// �õ��������Ĵ�С��
	t_fSize = m_ptrBBData->m_vecBBSize[t_iNextFrame] - 
		             m_ptrBBData->m_vecBBSize[m_iCurFrame];
	t_fSize = t_fSize*m_fInteValue + m_ptrBBData->m_vecBBSize[m_iCurFrame];
	
	// 
	// �õ���ǰ��������ɫ��
	t_srcC = m_ptrBBData->m_vecBBColor[m_iCurFrame];
	t_dstC = m_ptrBBData->m_vecBBColor[t_iNextFrame];
	osColorLerp( &t_resC,&t_srcC,&t_dstC,m_fInteValue );
	t_dwFBBColor = t_resC;
	if( m_bUseCustomColor )
	{
		t_dwFBBColor &= t_sdwAlphaBlack;
		t_dwFBBColor |= m_dwCustomColor;
	}


	for( t_i=0;t_i<4;t_i++ )
	{
		m_arrRVertex[t_i].m_dwColor = t_dwFBBColor;
		m_arrRVertex[t_i].m_vec3Pos = m_vec3Fpos;
	}


	// 
	// ��䶥���uv.
	m_arrRVertex[0].m_vec2Uv = osVec2D( 0,0 );
	m_arrRVertex[1].m_vec2Uv = osVec2D( 1,0 );
	m_arrRVertex[2].m_vec2Uv = osVec2D( 1,1 );
	m_arrRVertex[3].m_vec2Uv = osVec2D( 0,1 );


	get_bbUpRightVec( _camptr,t_upVec,t_rightVec );


	t_fWidth = t_fSize + m_ptrBBData->m_vecBBSizeAdj[1]/2.0f;
	t_fHeight = t_fSize + m_ptrBBData->m_vecBBSizeAdj[0]/2.0f;
	// ʹ�����Ǵ�����bbInsʱ�Ĳ�����scale��������Ĵ�С
	t_fWidth *= m_fSizeScale;
	t_fHeight *= m_fSizeScale;
	t_upVec *= t_fWidth;
	t_rightVec *= t_fHeight;


	m_arrRVertex[0].m_vec3Pos -= t_rightVec;
	m_arrRVertex[0].m_vec3Pos += t_upVec;

	m_arrRVertex[1].m_vec3Pos += t_rightVec;
	m_arrRVertex[1].m_vec3Pos += t_upVec;

	m_arrRVertex[2].m_vec3Pos += t_rightVec;
	m_arrRVertex[2].m_vec3Pos -= t_upVec;

	m_arrRVertex[3].m_vec3Pos -= t_rightVec;
	m_arrRVertex[3].m_vec3Pos -= t_upVec;

	// 
	// ��������Bounding Sphere
	if( t_fWidth>t_fHeight )
		m_bsphere.radius = t_fWidth;
	else
		m_bsphere.radius = t_fHeight;

	return true;

	unguard;
}



/** \brief
*  ��Ⱦ��ǰ��billBoard Instance.
*
*  ����billBoard�Ĳ�ͬ���ͣ��ó���ͬ��billBoard��Ⱦ���ݡ��ο�Character����Ⱦ��
*
*  \param _pipe    ��Ⱦ��ǰ��billBoardʱʹ�õ��м�ܵ���
*  
*/
bool osc_billBoardIns::render_billBoardIns( osc_middlePipe* _pipe )
{
	guard;

	os_polygon       t_rPolygon;
	static WORD      t_idx[6] = { 0,1,2,0,2,3 };

	osassert( m_iCurFrame < m_ptrBBData->m_iTotalFrame );

	//
	// ��Ⱦ��ǰ��billBoard.
	// ���Ը��ݵ�ǰ��ͬ��֡�����õ���ǰ֡Ӧ��ʹ�õ�shaderId��
	t_rPolygon.m_iShaderId = 
		m_ptrBBData->m_iShaderId[m_ptrBBData->m_vecTexId[m_iCurFrame]];  
	if( t_rPolygon.m_iShaderId<0 )
	{
		osassertex( m_bInuse,"��ǰ�Ĺ����ʵ������ʹ����" );
		osassertex( m_ptrBBData->m_bInuse,"��Ӧ�Ĺ�������ݲ���ʹ����" );
		osassertex( false,"���ԭ��...\n" );
	}

	t_rPolygon.set_rawData( m_arrRVertex, t_idx );

	t_rPolygon.m_iPriNum = 2;
	t_rPolygon.m_iPriType = TRIANGLELIST;
	t_rPolygon.m_iVerSize = sizeof( os_billBoardVertex );
	t_rPolygon.m_iVerNum = 4;

	//
	// 
	if( !_pipe->render_polygon( &t_rPolygon  ) )
		return false;

	return true;

	unguard;
}








/** \brief
 *  ��ʼ����ǰ��billboard Instance.
 *
 *  ��������ǰ��billBoard�У�û������Ҫ������billboard,�������billBoard
 *  �ϵõ�һ�����е�billBoard,��������ǰ��billBoard
 *  
 */
WORD osc_billBoardIns::init_billBoardIns( osc_billBoard* _bbdata,osVec3D& _pos,bool _createIns/* = true*/,
			DWORD _cusColor/* = 0*/,float _sizeScale/* = 1.0f*/,bool _topLayer/* = false*/ )
{
	guard;
	
	osassert( _bbdata );
	


	m_ptrBBData = _bbdata;
	if( !_createIns )
	{
		m_ptrBBData->m_bCanSwap = false;

		return 0;
	}

	m_bTopLayer = _topLayer;

	//
	// ����·�����ݵ�ʵ����
	if( m_ptrBBData->m_bUseRunTrack )
	{
		s_string   t_szTrackName = TRACKFILE_DIR;

		t_szTrackName += m_ptrBBData->m_strTrackFname;
		m_iTrackId = g_ptrTrackMgr->create_track( t_szTrackName.c_str(),
			m_ptrBBData->m_fTrackMoveSpeed,m_ptrBBData->m_bLoopBB );
		
		// ATTENTION TO FIX:River mod @ 2007-4-6:�������trackʧ��,��ʹ��track
		if( m_iTrackId < 0 )
			m_ptrBBData->m_bUseRunTrack = FALSE;
	}
	
	//
	// ��ʼ��bbInstance�Ķ������ݡ�
	init_bbAniData();
	osassert( _sizeScale >= 0.0f );
	m_fSizeScale = _sizeScale;
	m_fBaseSizeScale = m_fSizeScale;
	set_billBoardPos( _pos );

	if( _cusColor > 0 )
	{
		m_bUseCustomColor = true;
		m_dwCustomColor = _cusColor;
	}

	m_bInReleaseState = false;

	// 
	// River mod @ 2008-8-16: ��Χ��Ĵ�С����
	m_bsphere.radius *= this->m_fSizeScale;

	increase_val();
	set_insInit();

	return get_val();

	unguard;
}


//! �ͷŵ�ǰ��billboard,ʹ��ǰ��billBoardIns��Ϊ���õ�Instance.
void osc_billBoardIns::release_billBoardInsInner( void )
{
	guard;

	m_ptrBBData = NULL; 
	m_bInuse = false;   
	
	m_bInview = false;

	// River added @ 2011-1-30:Ϊ��ʹBB��������ƽ����
	m_bInReleaseState = false;

	m_fTimeScale = 1.0f;
	m_fBaseSizeScale = 1.0f;
	reset_ins();

	unguard;
}


//! �ͷŵ�ǰ��billboard,ʹ��ǰ��billBoardIns��Ϊ���õ�Instance.
void osc_billBoardIns::release_billBoardIns( bool _finalRelease/* = false*/ )
{ 
	guard;

	// River added @ 2010-3-20:
	if( _finalRelease )
	{
		::EnterCriticalSection( &osc_mapLoadThread::m_sBbCS );

		if( m_ptrBBData )
			m_ptrBBData->release_billBoard();

		// �ͷ���Ӧ��node.
		osc_bbManager*  t_ptrBBmgr = osc_bbManager::Instance();

		CSpaceMgr<osc_billBoard>::NODE t_ptrNode;
		for( t_ptrNode = t_ptrBBmgr->m_vecBillboard.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = t_ptrBBmgr->m_vecBillboard.next_validNode( &t_ptrNode ) )
		{
			if( t_ptrNode.p == m_ptrBBData )
				t_ptrBBmgr->m_vecBillboard.release_node( t_ptrNode.idx );
		}
		::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

	}

	// River added @ 2011-1-30:Ϊ��ʹBB��������ƽ����
	m_bInReleaseState = true;

	unguard;
} 



//! ��ʼ����̬����
template<> osc_bbManager* ost_Singleton<osc_bbManager>::m_ptrSingleton = NULL;


osc_bbManager::osc_bbManager()
{
	m_vecBillboard.resize( MAX_BILLBOARDNUM/2 );
	m_vecBBIns.resize( MAX_BILLBOARDNUM );
	m_vecHugeBBIns.resize( MAX_HUGEBBNUM );

	m_vecRainEffect.resize( MAX_RAINNUM/4 );
	
	m_ptrMPipe = NULL;

	m_iIdxBuf = -1;
	m_iVerBuf = -1;

	m_bFirstRainEff = true;
	m_bFristShadow = true;

	m_fDiscardBBTime = 0.0f;

	m_ptrGlobalCam = NULL;

}


//! ��ʼ��billBoardManager.
void osc_bbManager::init_bbManager( osc_middlePipe* _pipe )
{
	guard;

	osassert( _pipe );

	m_ptrMPipe = _pipe;
	osMatrixIdentity( &m_smatMatrix );

	// 
	// ������̬��������������
	int  t_iGBufIdx = -1;
	WORD*   t_idxBuf = (WORD*)START_USEGBUF( t_iGBufIdx );
	int     t_iIbufIdx,t_iVbufIdx;
	for( int t_i=0;t_i<MAX_BILLBOARDNUM;t_i ++ )
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
	m_iIdxBuf = m_ptrMPipe->create_staticIB( t_idxBuf,MAX_BILLBOARDNUM*6 );
	osassert( m_iIdxBuf >= 0 );
	END_USEGBUF( t_iGBufIdx );

	//
	// ������̬�Ķ��㻺����.
	DWORD   t_dwFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	m_iVerBuf = m_ptrMPipe->create_dynamicVB( 
		t_dwFvf,MAX_BILLBOARDNUM*4*sizeof( os_billBoardVertex ) );
	osassert( m_iVerBuf );


	//
	// ��������ļ���Ӱbb.


	unguard;
}


// River @ 2010-12-24:Ϊ���ϲ��ٽ���ɾ�������Ĳ�����
const char* osc_bbManager::get_bbNameFromId( int _id )
{
	guard;

	DWORD  t_dwId = (WORD)_id;
	static char  t_sz[4];

	if( m_vecBBIns.validate_id( t_dwId ) )
	{
		if( m_vecBBIns[t_dwId].validate_cval( _id ) )
			return m_vecBBIns[t_dwId].m_ptrBBData->m_szBFilename;
	}

	t_sz[0] = NULL;
	return t_sz;

	unguard;
}


/** \brief
*  ɾ��billBoard.
*
*  \param _id ɾ��ָ��id��billBoard.
*/
void osc_bbManager::delete_billBoard( int _id,bool _finalRelease/* = false*/ )
{	
	DWORD  t_dwId = (WORD)_id;

	osassert( (t_dwId>=0) );

	// HUGE bb
	if( t_dwId >= HUGEBB_IDSTART )
	{
		::EnterCriticalSection( &osc_mapLoadThread::m_sBbCS );

		t_dwId -= HUGEBB_IDSTART;

		if( m_vecHugeBBIns.validate_id( t_dwId ) )
		{
			if( m_vecHugeBBIns[t_dwId].validate_cval( _id ) )
				m_vecHugeBBIns[t_dwId].release_billBoardIns();
		}
		
		::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

		return;
	}

	osassert( m_vecBBIns.validate_id( t_dwId ) );

	if( m_vecBBIns[t_dwId].validate_cval( _id ) )
	{
		osassert( m_vecBBIns[t_dwId].is_billBoardInUsed() );

		m_vecBBIns[t_dwId].release_billBoardIns( _finalRelease );
	}

}

//
/** \brief
 *  ����һ��billBoardInstance.
 *
 *  �ҳ���ǰmgr�п��е�bbInstance,ʹ�ÿ��еĿռ䴴��bbInstance
 */
int osc_bbManager::create_billBoard( const char *_bfile,osVec3D& _pos,
	bool _createIns/* = true*/,float _sizeScale/* = 1.0f*/,bool _topLayer/* = false*/ )
{
	guard;

	int               t_id;
	BOOL               t_bHugeBB;
	WORD               t_wCreateVal = 0;
	osc_billBoardIns*  t_ptrBBIns;


	osc_billBoard*   t_ptrBBData;
	t_ptrBBData = get_billBoardPtr( _bfile );
	osassertex( t_ptrBBData,(char*)_bfile );

	t_bHugeBB = t_ptrBBData->is_hugeBillBoard();
	if( t_bHugeBB )
	{
		//! River @ 2011-1-30:ȷ��������̹߳�ͬ�õ�freeNode.
		::EnterCriticalSection( &osc_mapLoadThread::m_sBbCS );
		t_id = m_vecHugeBBIns.get_freeNodeAUse( &t_ptrBBIns );
		::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

		if( t_ptrBBIns == NULL )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

			osassertex( false,"There's Not enough size to Create bbIns!!!\n" );
			return -1;
		}
	}
	else
	{
		//! River @ 2011-1-30:ȷ��������̹߳�ͬ�õ�freeNode.
		::EnterCriticalSection( &osc_mapLoadThread::m_sBbCS );

		t_id = m_vecBBIns.get_freeNodeAUse( &t_ptrBBIns );

		::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

		if( t_ptrBBIns == NULL )
		{
			// River @ 2011-10-18: �޸ĳɸ��º͵ĳ���ʽ��
			//osassertex( false,"There's Not enough size to Create bbIns!!!\n" );
			osDebugOut( "There's Not enough size to Create bbIns!!!\n" );
			return -1;
		}
	}

	t_wCreateVal = t_ptrBBIns->init_billBoardIns( t_ptrBBData,_pos,_createIns,0,_sizeScale );

	// River Mod @ 2006-3-8: ������Ǵ���ʵ������bb���뵽�ڴ������ɾ��ʵ��
	if( !_createIns )
	{
		m_vecBBIns.release_node( t_id );
		t_id = -1;
	}

	

	// ���������࿪�Ĺ����id
	if( t_bHugeBB )
		t_id += HUGEBB_IDSTART;

	return osn_mathFunc::syn_dword( t_wCreateVal,WORD(t_id ) );

	unguard;
}

/** \brief
*  ����һ�����ܼ������Ӱ���billBoard
*
*  �������͵Ĺ������Ҫ����ȫ�ֵĳ�����Ч��������������Ч��������
*/
int osc_bbManager::create_hugeBillBoard( const char *_bfile,osVec3D& _pos,
									  bool _createIns/* = true*/,float _sizeScale/* = 1.0f*/ )
{
	guard;

	int               t_id;
	WORD               t_wCreateVal = 0;
	osc_billBoardIns*  t_ptrBBIns;


	osc_billBoard*   t_ptrBBData;
	t_ptrBBData = get_billBoardPtr( _bfile );
	osassertex( t_ptrBBData,(char*)_bfile );

	::EnterCriticalSection( &osc_mapLoadThread::m_sBbCS );

	t_id = m_vecHugeBBIns.get_freeNodeAUse( &t_ptrBBIns );
	if( t_ptrBBIns == NULL )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

		osassertex( false,"There's Not enough size to Create bbIns!!!\n" );
		return -1;
	}

	t_wCreateVal = t_ptrBBIns->init_billBoardIns( t_ptrBBData,_pos,_createIns,0,_sizeScale );

	// River Mod @ 2006-3-8: ������Ǵ���ʵ������bb���뵽�ڴ������ɾ��ʵ��
	if( !_createIns )
	{
		m_vecBBIns.release_node( t_id );
		t_id = -1;
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );


	t_id += HUGEBB_IDSTART;

	return osn_mathFunc::syn_dword( t_wCreateVal,WORD(t_id ) );

	unguard;
}




osMatrix   osc_bbManager::m_smatMatrix;



/** \brief
*  ����billBoard��λ�á�һ�������еĹ���壬���ܿ���λ�á�
*
*  \param _id ��Ҫ���õĹ�����id��
*  \param _pos  Ҫ���õĹ�����λ�á�
*/
void osc_bbManager::set_billBoardPos( int _id,osVec3D& _pos,bool _updateImm/* = false*/ )
{	
	guard;

	DWORD  t_dwId = (WORD)_id;
	osassert( (t_dwId>=0) );

	// HUGE bb
	if( t_dwId >= HUGEBB_IDSTART )
	{
		t_dwId -= HUGEBB_IDSTART;
		if( m_vecHugeBBIns.validate_id( t_dwId ) )
		{
			if( m_vecHugeBBIns[t_dwId].validate_cval( _id ) )
			{
				osassert( m_vecHugeBBIns[t_dwId].is_billBoardInUsed() );
				m_vecHugeBBIns[t_dwId].set_billBoardPos( _pos );
			}
		}
		return;
	}

	if( m_vecBBIns.validate_id( t_dwId ) )
	{
		if( m_vecBBIns[t_dwId].validate_cval( _id ) )
		{
			osassert( m_vecBBIns[t_dwId].is_billBoardInUsed() );
			m_vecBBIns[t_dwId].set_billBoardPos( _pos );
			
			// 
			// River added @ 2009-5-7:
			// Ϊʹ��Ч���������ӵ�˳������ʱ����bbpos.
			if( _updateImm )
				update_bbPos( _id,m_ptrGlobalCam );
		}
	}

	return;

	unguard;
}


/** \brief
*  ��ʱ���µ�ǰid�����ʵ����λ��
*
*  ���ڸ����ڳ��������FrameMove������׼ȷ�õ������λ�õ������
*  �����������õ��Ĺ���塣
*/
void osc_bbManager::update_bbPos( int _id,I_camera* _camptr )
{
	guard;

	DWORD  t_dwId = (WORD)_id;
	osassert( t_dwId >= 0 );

	// HUGE bb
	if( t_dwId >= HUGEBB_IDSTART )
	{
		t_dwId -= HUGEBB_IDSTART;
		if( m_vecHugeBBIns.validate_id( t_dwId ) )
		{
			if( m_vecHugeBBIns[t_dwId].validate_cval( _id ) )
			{
				if( !m_vecHugeBBIns[t_dwId].frame_moveBBIns( 0.0f ) )
					m_vecHugeBBIns.release_node( t_dwId );
				else
					if( !m_vecHugeBBIns[t_dwId].fill_renderVerData( _camptr ) )
						osassert( false );
			}
		}
		return;
	}



	if( m_vecBBIns.validate_id( t_dwId ) )
	{
		if( m_vecBBIns[t_dwId].validate_cval( _id ) )
		{
			if( !m_vecBBIns[t_dwId].frame_moveBBIns( 0.0f ) )
				m_vecBBIns.release_node( t_dwId );
			else
				if( !m_vecBBIns[t_dwId].fill_renderVerData( _camptr ) )
					osassert( false );
		}
	}

	return;


	unguard;
}


/** \brief
*  ������billboard����Ҫ�����billBoard����Ϊ��Ⱦ׼�����ݡ�
*  
*  ��������ڰ�����shadow���bb��������䴦��
*/
void osc_bbManager::frameMove_bbMgr( I_camera* _camptr )
{
	guard;

	float             t_eleTime = sg_timer::Instance()->get_lastelatime();
	CSpaceMgr<osc_billBoardIns>*   t_arrBBIns[3];

	osassert( this->m_ptrMPipe );
	osassert( _camptr );

	if( !m_ptrScePtr )
		m_ptrScePtr = ::get_sceneMgr();
	m_ptrGlobalCam = _camptr;

	// 
	// �����������ʹ�õ���Դ��
	set_bbMgrLru();

	//
	// �ȶ�������billBoard���д���
	t_arrBBIns[0] = &m_vecBBIns;
	t_arrBBIns[1] = &m_vecRainEffect;
	t_arrBBIns[2] = &m_vecHugeBBIns;

	CSpaceMgr<osc_billBoardIns>::NODE t_ptrNode;
	for( int t_j = 0;t_j<3;t_j ++ )
	{

		for( t_ptrNode = t_arrBBIns[t_j]->begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = t_arrBBIns[t_j]->next_validNode(&t_ptrNode) )
		{
			//osassert( t_ptrNode.p->m_bInuse );
			
			// 
			// ���ڱ�����߳���صĳ�ʼ������
			if( !t_ptrNode.p->get_insInit() )
				continue;

			// River Added @ 2011-3-19:��С���߳�Э������
			if( t_ptrNode.p->m_ptrBBData == NULL )
				continue;

			//! River mod @ 2009-7-31:������еĹ����У������ͷŵ�bb,��asser.
			if( !t_ptrNode.p->m_ptrBBData->m_bInuse )
			{
				//osassertex( false,"�˴���Ӧ�������ͷŵ�bb����....\n" );
				t_arrBBIns[t_j]->release_node( t_ptrNode.idx );
				continue;
			}


			// ֻҪ�����ã�billBoard�Ͳ����ͷš�
			t_ptrNode.p->m_ptrBBData->set_used();

			//
			// �ȴ���billBoard�Ķ�������,����ڲ����ͷ�״̬�����ڴ˴��ͷš�
			t_ptrNode.p->frame_moveBBIns( t_eleTime,true );

			if( !t_ptrNode.p->m_bInuse )
			{
				t_arrBBIns[t_j]->release_node( t_ptrNode.idx );
				continue;
			}

			// ����Ƿ�ǰ��billBoard���������ڡ�	
			t_ptrNode.p->m_bInview = ((osc_TGManager*)m_ptrScePtr)->
				             is_sphereInView( t_ptrNode.p->m_bsphere,false );
			
			// ���͹���岻���ɼ���
			if( (!t_ptrNode.p->m_bInview)&&(t_j!=2) )
				continue;
				
			//
			// ����billBoard�����ͣ����billBoard�����ݡ�
			if( !t_ptrNode.p->fill_renderVerData( _camptr ) )
				osassert( false );

		}
	}

	//! River @ 2009-7-31:�ƶ��������frameMove֮�������frameMove�Ĺ����У�
	//                    ������lruBB,��Ӧ���ڵ�ǰ֡�����á�
	m_fDiscardBBTime += t_eleTime;
	if( m_fDiscardBBTime >= PROCESS_DISCARDTIME )
	{
		m_fDiscardBBTime = 0.0f;
		discard_lruBB();
	}


	return;

	unguard;
}


//! ��Ⱦ���͹���壬�������Ⱦ�Ľ׶���Ⱦ
bool osc_bbManager::render_mgrHugeBB( I_camera* _camptr )
{
	guard;

    bool             t_b;
	float             t_eleTime = sg_timer::Instance()->get_lastelatime();
	osMatrix          t_worldMat;


	osassert( _camptr );

	osassert( m_ptrMPipe );

	osassert( m_ptrScePtr );

	m_iRenderTriNum = 0;

	// 
	// ��Ⱦ������ʱ�򣬲�ʹ��AlphaTest
	m_ptrMPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_CCW );
	m_ptrMPipe->set_renderState( D3DRS_ZENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ALPHAREF,0x1 );
	// River @ 2006-6-22:��������Ⱦ�ر�ȫ�ֵ���Ч
	m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,FALSE );
	m_ptrMPipe->set_worldMatrix( m_smatMatrix );


	for( int t_k = 1;t_k<4;t_k ++ )
	{
		switch( t_k )
		{
		case 1:
			{
				//
				// ����Ⱦ�ͱ�����ϵĹ���塣
				m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
				m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
				m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA  );

				break;
			}
		case 2:
			{
				//
				//����Ⱦ�ͱ�����ӵĹ���塣����ͱ�����ӣ��򲻿��������Alpha״̬
				m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1 );
				m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
				m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );

				break;
			}
		case 3:
			{
				//
				// �ͱ��������ӵĹ����
				m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
				m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
				m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );
				break;
			}
		default:
			osassert( false );
		}

		//
		// ���ݲ�ͬ����Ⱦ��ʽ����Ⱦ��ͬ�Ĺ�������ݡ�
		CSpaceMgr<osc_billBoardIns>::NODE  t_ptrNode;
		for( t_ptrNode = m_vecHugeBBIns.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = m_vecHugeBBIns.next_validNode( &t_ptrNode ) )
		{
			osassert( t_ptrNode.p->m_bInuse );

			if( t_ptrNode.p->m_ptrBBData->m_wAlphaType != t_k )
				continue;

			// River @ 2010-6-5:����ZEnableΪfalse�Ĺ����.
			if( !t_ptrNode.p->m_ptrBBData->m_bZEnable )
				m_ptrMPipe->set_renderState( D3DRS_ZENABLE,FALSE );

			t_b = t_ptrNode.p->render_billBoardIns( m_ptrMPipe );
			if( !t_b )
				return false;

			m_iRenderTriNum += 2;

			// River @ 2010-6-5:����ZEnableΪfalse�Ĺ����.
			if( !t_ptrNode.p->m_ptrBBData->m_bZEnable )
				m_ptrMPipe->set_renderState( D3DRS_ZENABLE,TRUE );

		}

	}

	return true;

	unguard;
}


/** \brief
*  ��Ⱦ�����������пɼ���billBoard.
*
*  \param _camptr    �����м���������
*  
*/
bool osc_bbManager::render_bbManager( 
	I_camera*  _camptr,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	guard;

    bool             t_b;
	float             t_eleTime = sg_timer::Instance()->get_lastelatime();
	osMatrix          t_worldMat;


	osassert( _camptr );
	osassert( m_ptrMPipe );

	osassert( m_ptrScePtr );

	m_iRenderTriNum = 0;

	// 
	// ��Ⱦ������ʱ�򣬲�ʹ��AlphaTest
	m_ptrMPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_CCW );
	m_ptrMPipe->set_renderState( D3DRS_ZENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ALPHATESTENABLE,TRUE );
	m_ptrMPipe->set_renderState( D3DRS_ALPHAREF,0x1 );
	// River @ 2006-6-22:��������Ⱦ�ر�ȫ�ֵ���Ч
	m_ptrMPipe->set_renderState( D3DRS_FOGENABLE,FALSE );
	m_ptrMPipe->set_worldMatrix( m_smatMatrix );

	for( int t_k = 1;t_k<4;t_k ++ )
	{
		switch( t_k )
		{
		case 1:
			{
				//
				// ����Ⱦ�ͱ�����ϵĹ���塣
				m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
				m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
				m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA  );

				break;
			}
		case 2:
			{
				//
				//����Ⱦ�ͱ�����ӵĹ���塣����ͱ�����ӣ��򲻿��������Alpha״̬
				m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1 );
				m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
				m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );

				break;
			}
		case 3:
			{
				//
				// �ͱ��������ӵĹ����
				m_ptrMPipe->set_textureStageState( 0,D3DTSS_ALPHAOP,D3DTOP_MODULATE );
				m_ptrMPipe->set_renderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
				m_ptrMPipe->set_renderState( D3DRS_DESTBLEND,D3DBLEND_ONE  );
				break;
			}
		default:
			osassert( false );
		}

		//
		// ���ݲ�ͬ����Ⱦ��ʽ����Ⱦ��ͬ�Ĺ�������ݡ�
		CSpaceMgr<osc_billBoardIns>::NODE  t_ptrNode;
		for( t_ptrNode = m_vecBBIns.begin_usedNode();
			t_ptrNode.p != NULL;
			t_ptrNode = m_vecBBIns.next_validNode( &t_ptrNode ) )
		{
			if( !t_ptrNode.p->get_insInit() )
				continue;

			//osassert( t_ptrNode.p->m_bInuse );
			// River @ 2011-3-19:ȥ����assert��������
			if( !t_ptrNode.p->m_bInuse )
				continue;

			// River Added @ 2011-3-19:��С���߳�Э������
			if( t_ptrNode.p->m_ptrBBData == NULL )
				continue;

			if( t_ptrNode.p->m_ptrBBData->m_wAlphaType != t_k )
				continue;
			if( !t_ptrNode.p->m_bInview )
				continue;

			// ����״̬
			if( t_ptrNode.p->m_bHide )
				continue;

			// top zlayer��AniMesh���������Ⱦ
			if( _rtype != OS_COMMONRENDER )
			{
				if( _rtype == OS_RENDER_LAYER )
				{
					if( t_ptrNode.p->m_bTopLayer )
						continue;
				}
				else
				{
					if( !t_ptrNode.p->m_bTopLayer )
						continue;
				}
			}

			// River @ 2010-6-5:����ZEnableΪfalse�Ĺ����.
			if( !t_ptrNode.p->m_ptrBBData->m_bZEnable )
				m_ptrMPipe->set_renderState( D3DRS_ZENABLE,FALSE );

			t_b = t_ptrNode.p->render_billBoardIns( m_ptrMPipe );
			
			// River @ 2010-6-5:����ZEnableΪfalse�Ĺ����.
			if( !t_ptrNode.p->m_ptrBBData->m_bZEnable )
				m_ptrMPipe->set_renderState( D3DRS_ZENABLE,TRUE );

			if( !t_b )
				return false;

			m_iRenderTriNum += 2;
		}

	}

	return true;

	unguard;
}

//! ��һ��bb�ļ���,�õ�billBoard��Ӧ�ڴ�ṹ��ָ��.
osc_billBoard* osc_bbManager::get_billBoardPtr( const char* _fname )
{
	guard;

	osc_billBoard*    t_ptrBB;
	DWORD             t_dwHash = string_hash( _fname );

	CSpaceMgr<osc_billBoard>::NODE t_ptrNode;
	for( t_ptrNode = m_vecBillboard.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecBillboard.next_validNode( &t_ptrNode ) )
	{
		//osassert( t_ptrNode.p->m_bInuse );
		// River @ 2011-2-28:��ͬһ�߳̿������ڵ���bb���ݣ�����ʱ��û����ȫ����.
		if( !t_ptrNode.p->m_bInuse )
			continue;

		if( t_ptrNode.p->m_dwHash == t_dwHash )
		{
			osassert( 0 == strcmp(t_ptrNode.p->m_szBFilename,_fname) );
			return t_ptrNode.p;
		}
	}


	//
	// ������Ѿ����ڵ�billBoard�б�����������Ҫ������billboard��
	//! River @ 2011-1-30:ȷ��������̹߳�ͬ�õ�freeNode.
	::EnterCriticalSection( &osc_mapLoadThread::m_sBbCS );

	m_vecBillboard.get_freeNodeAUse( &t_ptrBB );

	::LeaveCriticalSection( &osc_mapLoadThread::m_sBbCS );

	if( !t_ptrBB->load_billBoardFromFile( _fname ) )
		return NULL;
	else
		return t_ptrBB;

	unguard;
}


/** \brief
*  ��Ⱦ��ε�����²�����Ч����
*
*  ATTENTION TO OPP: shaderBB��Rainʹ��ͬһ��������Ҫ��Ⱦ�������Ƚϴ�
*                    �����Ż�����
*/
bool osc_bbManager::render_raindropBB( I_camera* _camptr )
{
	guard;

	float             t_eleTime = sg_timer::Instance()->get_lastelatime();
	osMatrix          t_worldMat;
	int               t_verIdx;

	osassert( _camptr );
	osassert( m_ptrMPipe );

	m_iRenderTriNum = 0;

	// ATTENTION TO OPP: ʹ��һ��СһЩ��buffer,������Ⱦʱ����仺����.
	int t_iGBufIdx = -1;
	os_billBoardVertex*  t_ptrBBver = (os_billBoardVertex*)START_USEGBUF( t_iGBufIdx );
	CSpaceMgr<osc_billBoardIns>::NODE  t_ptrNode;

	// 
	// ˳����Ⱦ���Ч���Ĺ���塣
	t_verIdx = 0;
	for( t_ptrNode = m_vecRainEffect.begin_usedNode();
		t_ptrNode.p != NULL&&t_verIdx<MAX_BILLBOARDNUM*4;
		t_ptrNode = m_vecRainEffect.next_validNode( &t_ptrNode ) )
	{
		osassert( t_ptrNode.p->m_bInuse );
		osassert( t_ptrNode.p->m_ptrBBData->m_wAlphaType == 1 );

		//! �����β�������Χ�ڣ�����Ҫ�Դ˹������Ⱦ
		if( !t_ptrNode.p->m_bInview )
			continue;

		m_sRainEffPoly.m_iShaderId = t_ptrNode.p->get_bbInsRShader();

		// �õ���Ⱦ��Ҫ�Ķ������ݡ�
		memcpy( &t_ptrBBver[t_verIdx],t_ptrNode.p->m_arrRVertex,
			sizeof( os_billBoardVertex )*4 );
		t_verIdx += 4;
		m_iRenderTriNum += 2;

	}


	// ��䶥�㻺����������Ⱦ��
	if( t_verIdx >=4 )
	{
		osassert( m_iVerBuf >= 0 );
		m_sRainEffPoly.m_sBufData.m_iVertexStart = m_ptrMPipe->fill_dnamicVB(
			m_iVerBuf,t_ptrBBver,sizeof( os_billBoardVertex ),t_verIdx );
		osassert( m_sRainEffPoly.m_sBufData.m_iVertexStart>= 0 );
		m_sRainEffPoly.m_iPriNum = t_verIdx/2;
		m_sRainEffPoly.m_iVerNum = t_verIdx;
		m_ptrMPipe->set_worldMatrix( m_smatMatrix );
		m_ptrMPipe->render_polygon( &m_sRainEffPoly );
	}

	END_USEGBUF( t_iGBufIdx );


	return true;

	unguard;
}

//@} 


//@{
//  ���Ч���䵽�����Ч����


//! ��һ�γ�ʼ��RainEff��ص����ݡ�
void osc_bbManager::first_initRainEff( void )
{
	guard;

	osassert( m_iVerBuf >= 0 );
	osassert( this->m_iIdxBuf >= 0 );

	m_sRainEffPoly.m_sBufData.m_iIbufId = m_iIdxBuf;
	m_sRainEffPoly.m_sBufData.m_iVbufId = m_iVerBuf;
	m_sRainEffPoly.m_bUseStaticVB = true;
	
	m_sRainEffPoly.m_iPriType = TRIANGLELIST;
	m_sRainEffPoly.m_iVerSize = sizeof( os_billBoardVertex );

	
	unguard;
}

//! ���ڴ��������Чʱ�Ĺ����λ��
I_fdScene* osc_bbManager::m_ptrScePtr = NULL;

//! �������Ч���õ��Ĺ�����ļ�����
# define   RAIN_EFFBB   "efftexture\\rainEff.bb"
# define   RAIN_SIZESCALE   0.8f
# define   RAIN_COLORADD    0.3f
# define   RAIN_BBOFFSET    0.2f


//! �������Ч����
void osc_bbManager::create_rainEff( osVec3D& _vec )
{
	guard;

	int               t_id;
	osc_billBoardIns*  t_ptrBBIns;

	t_ptrBBIns = NULL;
	t_id = m_vecRainEffect.get_freeNodeAUse( &t_ptrBBIns,TRUE );
	// ���û�пռ䣬��ֱ�ӷ��ء�
	if( t_ptrBBIns == NULL )
		return ;

	if( !m_ptrScePtr )
		m_ptrScePtr = ::get_sceneMgr();

	osColor    t_ambiColor; 
	osVec3D    t_vec3RPos;

	static float t_fDetailSize = TILE_WIDTH/(DETAIL_TILENUM*2);

	// �õ�׼ȷ�ĸ߶ȣ�������ƫ��ֵ
	osc_TGManager* pTGMgr = (osc_TGManager*)m_ptrScePtr;
	pTGMgr->get_detailTilePos( 
		int(_vec.x/t_fDetailSize),int(_vec.z/t_fDetailSize),t_vec3RPos );
	float t_fWaterHeight = pTGMgr->get_waterSurfaceHeight();
	if( t_vec3RPos.y < t_fWaterHeight )
		t_vec3RPos.y = t_fWaterHeight;

	t_vec3RPos.y += RAIN_BBOFFSET;

	t_ambiColor = osColor( g_fAmbiR+RAIN_COLORADD,
		         g_fAmbiG+RAIN_COLORADD,g_fAmbiB+RAIN_COLORADD,0.0f );
	osc_billBoard* t_ptrBBData = this->get_billBoardPtr( RAIN_EFFBB );
	osassert( t_ptrBBData );

	t_ptrBBIns->init_billBoardIns(
		t_ptrBBData,t_vec3RPos,true,DWORD(t_ambiColor),RAIN_SIZESCALE );

	t_ptrBBIns->set_insInit();

	// ��һ�����п����ص�polygon���ݡ�
	if( m_bFirstRainEff )
	{
		m_bFirstRainEff = false;
		m_sRainEffPoly.m_iShaderId = 
			t_ptrBBIns->get_bbInsRShader();
		first_initRainEff();
	}

	return;

	unguard;
}

//@} 



osc_bbManager::~osc_bbManager()
{

	m_iIdxBuf = -1;
	m_iVerBuf = -1;

}

/** �õ�һ��sg_timer��Instanceָ��.
*/
osc_bbManager* osc_bbManager::Instance( void )
{
	guard;

	if( !osc_bbManager::m_ptrSingleton )
	{

		osc_bbManager::m_ptrSingleton = new osc_bbManager;
		
		osassert( osc_bbManager::m_ptrSingleton );
	}
	
	return osc_bbManager::m_ptrSingleton;

	unguard;
}

/** ɾ��һ��sg_timer��Instanceָ��.
*/
void osc_bbManager::DInstance( void )
{
	SAFE_DELETE( osc_bbManager::m_ptrSingleton );
}

//! �ͷ����һ��ʱ��û��ʹ�õ�billboard.
void osc_bbManager::discard_lruBB( void )
{
	CSpaceMgr<osc_billBoard>::NODE t_ptrNode;
	for( t_ptrNode = m_vecBillboard.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecBillboard.next_validNode( &t_ptrNode ) )
	{
		if( (t_ptrNode.p->get_nu() > DISCARD_BBTIMES )&&
			(t_ptrNode.p->m_bCanSwap) )
		{
			t_ptrNode.p->release_billBoard();
			m_vecBillboard.release_node( t_ptrNode.idx );
		}
	}

}

//! ��ǰ�Ĺ�����Ƿ���ѭ�����ŵĹ����,����������Ч,����ʹ��ѭ�����ŵĹ����.
bool osc_bbManager::is_loopPlayBB( int _id )
{
	guard;

	DWORD  t_dwId = (WORD)_id;
	osassert( t_dwId >= 0 );

	// HUGE bb
	if( t_dwId >= HUGEBB_IDSTART )
	{
		t_dwId -= HUGEBB_IDSTART;

		if( m_vecHugeBBIns.validate_id( t_dwId ) )
		{
			if( m_vecHugeBBIns[t_dwId].validate_cval( _id ) )
				return m_vecHugeBBIns[t_dwId].is_loopPlayBB();
		}		

		return false;
	}


	if( m_vecBBIns[t_dwId].validate_cval( _id ) )
	{
		osassert( m_vecBBIns[t_dwId].is_billBoardInUsed() );
		return m_vecBBIns[t_dwId].is_loopPlayBB();
	}

	return false;

	unguard;
}















