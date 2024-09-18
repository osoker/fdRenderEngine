//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osStateBlock.h
 *
 *   Desc:     O.S.O.K �������Ⱦ�������
 *
 *   His:      River Created @ 2004-12-14
 *
 *  ���ڴ��˽�����ѹ���У����Լ������ѹ��Զ������˶����ѹ�������˶����
 *    ѹ��Զ���繫�ڶ����ѹ������
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/osStateBlock.h"
# include "../../interface/miskFunc.h"
# include "../../exLib/SceneEx.h"



os_stateBlockMgr* os_stateBlockMgr::m_ptrInstance = NULL;



os_stateBlockMgr::os_stateBlockMgr()
{
	m_pd3dDevice = NULL;
	m_ptrInstance = NULL;

	m_bRecordState = FALSE;

	m_vecStateBlock = new LPDIRECT3DSTATEBLOCK9[MAX_STATEBLOCK];
	m_vecSBUsed.resize( MAX_STATEBLOCK );
	m_vecSBCreateTimes.resize( MAX_STATEBLOCK );
	for( int t_i=0;t_i<MAX_STATEBLOCK;t_i ++ )
	{
		m_vecStateBlock[t_i] = NULL;
		m_vecSBUsed[t_i] = false;
		m_vecSBCreateTimes[t_i] = 0;
	}
	
}




os_stateBlockMgr::~os_stateBlockMgr()
{
	// river mod @ 2011-2-23:�˳�ʱ�ͷţ����岻��.
	/*
	for( int t_i=0;t_i<MAX_STATEBLOCK;t_i ++ )
	{
		osassertex( (!m_vecStateBlock[t_i]),
			"����û���ͷŵ��豸StateBlock!!!\n" );
	}
	SAFE_DELETE_ARRAY( m_vecStateBlock );
	*/ 

}

/** ɾ��һ��sg_timer��Instanceָ��.
*/
void os_stateBlockMgr::DInstance()
{
	SAFE_DELETE( m_ptrInstance );
}



//! �õ�һ����Ⱦ״̬�顣
int os_stateBlockMgr::create_stateBlock( void )
{
	guard;

	for( int t_i=0;t_i<MAX_STATEBLOCK;t_i ++ )
	{
		if( !m_vecSBUsed[t_i] )
		{
			m_vecSBUsed[t_i] = true;
			m_vecSBCreateTimes[t_i] ++;

			return osn_mathFunc::syn_dword( m_vecSBCreateTimes[t_i],t_i );
		}
	}
	
	return -1;

	unguard;
}

//! ����¼��״̬�顣
void os_stateBlockMgr::end_stateBlock( int _idx )
{
	guard;

	osassert( _idx >= 0 );
	osassert( m_pd3dDevice );
	HRESULT   t_hr;
	t_hr = m_pd3dDevice->EndStateBlock( &m_vecStateBlock[(WORD)_idx] );
	osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr ) );

	m_bRecordState = FALSE;

	unguard;
}

//! ����Ⱦ״̬��ʹ�õ��豸��
void os_stateBlockMgr::apply_stateBlock( int _idx )
{
	guard;

	osassert( ((WORD)_idx) >= 0 );
	HRESULT   t_hr;

	osassertex( (!m_bRecordState),"������¼��״̬���ʱ��apply...\n" );

	t_hr = m_vecStateBlock[(WORD)_idx]->Apply();

	// TEST CODE:
	static int t_iApplyTimes = 0;

	if( FAILED( t_hr ) )
	{
		osassertex( false,va( "Idx is:<%d><%s><%d>..\n",
			((WORD)_idx),osn_mathFunc::get_errorStr( t_hr ),t_iApplyTimes ) );
	}
	else
	{
		if( ((WORD)_idx) == 2 )
			t_iApplyTimes ++;
	}

	unguard;
}


//! �ͷ�һ����Ⱦ״̬��.
void os_stateBlockMgr::release_stateBlock( int& _idx )
{
	guard;

	osassert( (((WORD)_idx) >= 0)&&( ((WORD)_idx)<MAX_STATEBLOCK) );

	SAFE_RELEASE( m_vecStateBlock[(WORD)_idx] );
	m_vecSBUsed[(WORD)_idx] = false;
	_idx = -1;

	unguard;

}

//! ���豸lost��,ȫ�����ͷŵ�ǰ��stateBlock��Դ.
void os_stateBlockMgr::sbm_onLostDevice( void )
{
	guard;

	for( int t_i=0;t_i<MAX_STATEBLOCK;t_i ++ )
	{
		if( !m_vecSBUsed[t_i] )
			osassert( !m_vecStateBlock[t_i] );

		if( m_vecSBUsed[t_i] )
		{
			SAFE_RELEASE( m_vecStateBlock[t_i] );
			m_vecSBUsed[t_i] = false;
		}
	}

	unguard;
}

