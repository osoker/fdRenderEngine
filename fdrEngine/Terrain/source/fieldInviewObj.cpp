//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fieldInviewObj.cpp
 *
 *  His:      River created @ 2004-2-24
 *
 *  Des:      ��ͼ��Ⱦʱ�Ĵ�����ͼ������Ϣ�ĺ����ȶ�������ļ��С�
 *   
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../tools/include/fadeMgr.h"
# include "../../mfpipe/include/osCharacter.h"


//! ȫ�ֱ��������Ƶ�ǰ��Ʒ������Ұʱ�ĵ��뵭��״̬��
OSENGINE_API bool              g_bFadeEnable = false;


//@ {
//  River @ 2009-12-30:
//  ���������������˳�������Ʒ���뵭����ʱ��,��Ʒ�۵�������
//  ��͸������HALF_FADETIME/ALPHA_FADETIME��á�
// 
//! ȫ�ֵĿ��Ƶ��뵭����Ʒʱ��ı�����
OSENGINE_API float             ALPHA_FADETIME = 1.5f;
OSENGINE_API float             HALF_FADETIME = 0.60f;
//@}



osc_inViewObjMgr::osc_inViewObjMgr()
{

	//! �ɼ���Ʒ��������Ӧ�ĵ��������.
	m_vecFadeInMgr.resize( MAX_ADDDELOBJ );
	m_vecFadeInObj.resize( MAX_ADDDELOBJ );

	//! ������Ʒ��ص�����.
	m_vecFadeOutMgr.resize( MAX_ADDDELOBJ );
	m_vecFadeOutObj.resize( MAX_ADDDELOBJ );

	m_iMaxNodeNum = 0;


}

osc_inViewObjMgr::~osc_inViewObjMgr()
{
	m_vecFadeInMgr.clear();
	m_vecFadeInObj.clear();
	m_vecFadeOutMgr.clear();
	m_vecFadeOutObj.clear();
}




//! ��ʼ����ǰ��InviewObjMgr.
void osc_inViewObjMgr::init_ivoMgr( int _objNum )
{
	guard;

	m_iMaxNodeNum = _objNum;

	osDebugOut( "The Tg's obj num is:<%d>...\n",_objNum );
	
	m_vecNormalDis.resize( _objNum );
	m_vecLfVs.resize( _objNum );

	m_vecIvOs.resize( _objNum );


	// ��ʼ�������е�ס�����͸����Ʒ���ұ�.
	m_vecHalfFadeMgr.resize( MAX_HALFALPHAOBJ );
	memset( &m_vecAccuHAObj[0],0xffffffff,sizeof( int )*MAX_HALFALPHAOBJ );
	memset( &m_vecHalfAlphaObj[0],0xffffffff,sizeof( int )*MAX_HALFALPHAOBJ );


	// 
	// ��ʼ��Ϊ���������е���Ʒ��������Ұ�ڲ���
	for( int t_i =0;t_i<_objNum;t_i ++ )
	{
		m_vecIvOs[t_i] = OSE_IVO_NOTINVIEW;
		m_vecLfVs[t_i] = false;
	} 


	// 
	for( int t_i =0;t_i<MAX_ADDDELOBJ;t_i ++ )
	{
		m_vecFadeInObj[t_i] = -1;
		m_vecFadeOutObj[t_i] = -1;
	}


	m_iNDObjNum = 0;

	return;

	unguard;
}

//! ��������ʾ����Ʒ������ɾ��һ����Ʒ.
void osc_inViewObjMgr::delete_normalDisObj( int _idx )
{
	guard;

	osassert( _idx >= 0 );

	if( m_iNDObjNum <= 0 )
		return;

	// 
	// �ȴ�������ʾ�Ķ������Ƴ�Ҫ��Ϊ��͸������Ʒ��
	int     t_i;
	osassert( m_iNDObjNum <= (int)m_vecNormalDis.size() );
	for( t_i=0;t_i<m_iNDObjNum;t_i ++ )
	{
		if( m_vecNormalDis[t_i] == _idx )
		{
			// ������Ⱦ����Ʒ��Ŀ��С
			m_iNDObjNum --;
			for( int t_j = t_i;t_j<m_iNDObjNum;t_j ++ )
				m_vecNormalDis[t_j] = m_vecNormalDis[t_j+1];

			break;
		}
	}

	return;

	unguard;
}


//! ��һ��������ʾ����Ʒ��Ϊ��͸������Ʒ��
void osc_inViewObjMgr::make_halfFade( int _objIdx )
{
	guard;

	int   t_i;

	//
	// ��Ҫ��Ϊ��͸������Ʒ�ŵ�fadeIn���С�
	for( t_i=0;t_i<MAX_HALFALPHAOBJ;t_i ++ )
	{
		if( m_vecHalfAlphaObj[t_i] == -1 )
			break;
	}

	//! river mod @ 2009-4-29:�������Ĵ�������Ʒ��͸�������������ת��
	//  һ����������Ż��������ǣ��ҵ�����fadein����Ʒ��Ȼ��������ʾ��Щ��Ʒ��
	if( t_i == MAX_HALFALPHAOBJ )
		return;

	m_vecHalfAlphaObj[t_i] = _objIdx;

	unguard;
}



//! ��ʼ��һ֡�Ĵ���.
void osc_inViewObjMgr::frame_setObjMgr( void )
{
	guard;

	int          t_i;
	float        t_fEtime = sg_timer::Instance()->get_lastelatime();

	m_iNDObjNum = 0;

	//
	// ��fade In ���е����ݽ��д���.
	for( t_i =0;t_i<MAX_ADDDELOBJ;t_i ++ )
	{
		if( m_vecFadeInObj[t_i] == -1 )
			continue;

		if( m_vecFadeInMgr[t_i].is_fadeOver() )
		{
			if (m_vecFadeInObj[t_i]<0||m_vecFadeInObj[t_i]>=(int)m_vecIvOs.size())
			{
				osassertex(false,"�����Ĵ���!");
			}
			m_vecIvOs[m_vecFadeInObj[t_i]] = OSE_IVO_NORMALDIS;
			m_vecFadeInObj[t_i] = -1;
		}
		else
			m_vecFadeInMgr[t_i].frame_fade( t_fEtime );

	}

	//
	// ��fade Out���е����ݽ��д���.
	for( t_i =0;t_i<MAX_ADDDELOBJ;t_i ++ )
	{
		if( m_vecFadeOutObj[t_i] == -1 )
			continue;
		if( m_vecFadeOutMgr[t_i].is_fadeOver() )
		{
			m_vecIvOs[m_vecFadeOutObj[t_i]] = OSE_IVO_NOTINVIEW;
			m_vecFadeOutObj[t_i] = -1;
		}
		else
		{
			m_vecFadeOutMgr[t_i].frame_fade( t_fEtime );
		}
	}

	//
	// �Ե�ס�����͸����Ʒ.
	for( int t_i=0;t_i<MAX_HALFALPHAOBJ;t_i ++ )
	{
		if( m_vecAccuHAObj[t_i] == -1 )
			continue;
		
		m_vecHalfFadeMgr[t_i].frame_fade( t_fEtime );
	}

	return;

	unguard;
}


//! ���һ��fadeInObject.
bool osc_inViewObjMgr::add_fadeInObj( int _idx )
{
	guard;

	int    t_i;

	// ������������ת�������򲻵��롣
	if( g_ptrCamera->is_rotIncurFrame() )
	{

		m_vecIvOs[_idx] = OSE_IVO_NORMALDIS;
		m_vecNormalDis[m_iNDObjNum] = _idx;
		m_iNDObjNum ++;
		return true;
	}


	for( t_i=0;t_i<MAX_ADDDELOBJ;t_i ++ )
	{
		if( m_vecFadeInObj[t_i] == -1 )
			break;
	}

	// 
	// ATTENTION TO FIX:�������Զ����Ʒ��ֱ���˳���Ⱦ����
	// ���û�пռ䣬�������⴦��
	if( t_i == MAX_ADDDELOBJ )
	{
		// ���������Ʒ�Ŀռ䲻������ֱ����ʾ
		m_vecIvOs[_idx] = OSE_IVO_NORMALDIS;
		return false;
	}

	m_vecFadeInObj[t_i] = _idx;

	// ��ʼ���롣
	m_vecFadeInMgr[t_i].start_fade( false );

	return true;

	unguard;
}


//! ���һ��FadeOutObject
bool osc_inViewObjMgr::add_fadeOutObj( int _idx )
{
	guard;

	int    t_i;


	
	// ������������ת�������򲻵�����
	if( g_ptrCamera->is_rotIncurFrame() )
	{
		m_vecIvOs[_idx] = OSE_IVO_NOTINVIEW;
		m_vecNormalDis[m_iNDObjNum] = _idx;
		m_iNDObjNum ++;
		return true;
	}

	for( t_i=0;t_i<MAX_ADDDELOBJ;t_i ++ )
	{
		if( m_vecFadeOutObj[t_i] == -1 )
			break;
	}

	// 
	// ���û�пռ䣬�������⴦��
	if( t_i == MAX_ADDDELOBJ )
	{
		m_vecIvOs[_idx] = OSE_IVO_NOTINVIEW;
		return false;
	}

	m_vecFadeOutObj[t_i] = _idx;


	// ��ʼ���롣
	m_vecFadeOutMgr[t_i].start_fade( true );

	return true;

	unguard;
}


//! ÿһ֡ȥ��ס������ߵ�͸����Ʒ�Ĵ���.
void osc_inViewObjMgr::process_halfFadeObj( void )
{
	guard;

	static bool  t_bInSec[MAX_HALFALPHAOBJ];
	int         t_i,t_j;

	//! River added @ 2009-4-29:�ȴ���fadeIn����Ʒ�������Ѿ�������ʾ����Ʒ��ɾ����
	for( t_i = 0;t_i<MAX_HALFALPHAOBJ;t_i ++ )
	{
		if( m_vecAccuHAObj[t_i] == -1 )
			continue;
		if( m_vecHalfFadeMgr[t_i].get_fadeState() == OSE_DISPLAY )
			m_vecAccuHAObj[t_i] = -1;
	}


	memset( t_bInSec,0,sizeof( bool )*MAX_HALFALPHAOBJ );
	for( t_i=0;t_i<MAX_HALFALPHAOBJ;t_i ++ )
	{
		if( m_vecHalfAlphaObj[t_i] == -1 )
			break;

		// ��������ʾ�Ķ�����ɾ������Ʒ����.
		delete_normalDisObj( m_vecHalfAlphaObj[t_i] );

		// ����������Ѿ�������Ŀǰ�����еİ�͸����Ʒ�б���, ��һ��.
		for( t_j=0;t_j<MAX_HALFALPHAOBJ;t_j ++ )
		{
			if( m_vecAccuHAObj[t_j] == m_vecHalfAlphaObj[t_i] )
			{
				t_bInSec[t_j] = true;
				break;
			}
		}

		if( t_j < MAX_HALFALPHAOBJ )
			continue;

		for( t_j=0;t_j<MAX_HALFALPHAOBJ;t_j ++ )
		{
			if( m_vecAccuHAObj[t_j] >= 0 )
				continue;

			m_vecAccuHAObj[t_j] = m_vecHalfAlphaObj[t_i];
			m_vecHalfFadeMgr[t_j].half_alpha();
			t_bInSec[t_j] = true;
			break;
		}
	}

	// ��û�е�ס����,�����ڰ�͸����Ʒ�б��е��������д���.
	for( t_i=0;t_i<MAX_HALFALPHAOBJ;t_i++ )
	{
		//
		// River @ 2009-4-29:Ӧ����ǿ�˴��Ĺ���,�ɵ�����ɵ���,��Ӧ��ֱ�ӵı�ɿ�����ʾ.
		//if( !t_bInSec[t_i] )
		//	m_vecAccuHAObj[t_i] = -1;
		if( !t_bInSec[t_i] )
		{
			if( m_vecAccuHAObj[t_i] != -1 )
			{
				m_vecHalfFadeMgr[t_i].half_alphaFadeIn();
				delete_normalDisObj( m_vecAccuHAObj[t_i] );
			}
		}

	}

	// �԰�͸������Ʒ���н��д���
	memset( &m_vecHalfAlphaObj[0],0xffffffff,sizeof( int )*MAX_HALFALPHAOBJ );

	unguard;
}



/** \brief
*  ÿһ֡�еõ���ǰTG����Ʒ�б�󣬶�inviewObjMgr����Ʒ�Ĵ���
*
*  River @ 2005-3-11: �������Ϊ�������ת�����뵭����Ʒ���ѵ������Ʒ
*                  ��ֱ�Ӽ��뵽���Ӷ��У��ѵ�������Ʒֱ�Ӵ���Ұ��ɾ����
*                    ֻ��������ƶ�ʱ��������Զ�����뵭����Ʒ��������ά����
*                    ����ʵ�ȡ���
*
*  ATTENTION TO OPP:��������ڵ�[]������ռ����������Ⱦ�ĺܴ�ٷֱȣ��Ǻܷ�ʱ��һ�������
*  ���g_bFadeEnable��Ϊfalse,��֡��������߲���!!!!!
*/
void osc_inViewObjMgr::frame_processIvo( VEC_BYTE& _ivoList )
{
	guard;

	int    t_i;
	int    t_iSize = (int)_ivoList.size();

	osassert( t_iSize <= m_iMaxNodeNum );
	//
	// ���ȫ�ֵ�fade״̬û�д򿪵Ļ�,���Ĵ���
	if( !g_bFadeEnable )
	{
		for( t_i=0;t_i<t_iSize;t_i ++ )
		{
			// 
			// �ڿ��ӷ�Χ��
			if( _ivoList[t_i] > MASK_OUTHALF_NOTVIEW )
			{
				m_vecNormalDis[m_iNDObjNum] = t_i;
				m_iNDObjNum ++;
			}
		}

		// 
		// ����ִ���������Ļ������һ"������Ʒ��ס���"��bug.
		// �Ե�ס�����͸����Ʒ�Ĵ���.
		if( g_bAlphaShelter )
			process_halfFadeObj();

		return;
	}


	osassert( m_iNDObjNum >= 0 );

	// 
	// ��TG�ڵ�ÿһ����Ʒ���д���
	for( t_i =0;t_i<t_iSize;t_i ++ )
	{
		osassert( t_i < m_iMaxNodeNum );
		//
		// ��һ֡�͵�ǰ֡��Ʒ��������Ұ�ڡ�
		if( (!m_vecLfVs[t_i])&&
			(_ivoList[t_i]<MASK_HALF_VIEW) )
		{
			// ��֡����Ʒ��������Ұ�ڣ�����Ҫ����

			continue;
		}

		//
		// ��һ֡�͵�ǰ֡����Ʒ������Ұ�ڲ���
		if( m_vecLfVs[t_i]&&
			(_ivoList[t_i]>=MASK_HALF_VIEW) )
		{
			if( m_vecIvOs[t_i] == OSE_IVO_NORMALDIS )
			{
				// ����Ҫ������Ʒ��״̬����Ʒ��ֹ��ʾ�Ϳ����ˡ�
				m_vecNormalDis[m_iNDObjNum] = t_i;
				m_iNDObjNum ++;
			}

			continue;
		}

		//
		// ��һ֡����Ұ�ڣ���ǰ֡������Ұ�ڡ�
		if( (m_vecLfVs[t_i])&&
			(_ivoList[t_i]<MASK_HALF_VIEW) )
		{
			// 
			// �����Ʒ������Ұ��һ������ⲿ������Ҫ����
			if( _ivoList[t_i]!=MASK_OUTHALF_NOTVIEW )
			{
				m_vecIvOs[t_i] = OSE_IVO_NOTINVIEW;
			}
			else
			{
				m_vecIvOs[t_i] = OSE_IVO_FADEOUT;

				// �����������м�����Ʒ,
				add_fadeOutObj( t_i );
			}
			m_vecLfVs[t_i] = false;


			continue;
		}


		// 
		// ��һ֡������Ұ�ڣ���ǰ֡����Ұ�ڡ�
		if( (!m_vecLfVs[t_i])&&
			(_ivoList[t_i]>=MASK_HALF_VIEW) )
		{
			// �����Ʒ����Ұ��һ������ڲ�������Ҫ����.
			if( _ivoList[t_i]==MASK_HALF_VIEW )
			{
				m_vecIvOs[t_i] = OSE_IVO_NORMALDIS;
			}
			else
			{
				m_vecIvOs[t_i] = OSE_IVO_FADEIN;

				// ����������м����µ���Ʒ��
				add_fadeInObj( t_i );
		
			}
			m_vecLfVs[t_i] = true;

		}

	}

	// �Ե�ס�����͸����Ʒ�Ĵ���.
	if( g_bAlphaShelter )
		process_halfFadeObj();


	return;

	unguard;
}


/** \brief
*  ���ó������õ���Materialֵ��
*
*  \param _alpha ��ǰmaterial�õ���alphaֵ��
*/
void osc_TGManager::set_material( float _alpha )
{
	D3DMATERIAL9   t_mtl;

	if( _alpha>1.0f )
		_alpha = 1.0f;
	if( _alpha<0.0f )
		_alpha = 0.0f;

	memset( &t_mtl,0,sizeof( D3DMATERIAL9 ) );

	t_mtl.Ambient.r = 1.0f;
	t_mtl.Ambient.g = 1.0f;
	t_mtl.Ambient.b = 1.0f;
	t_mtl.Diffuse.r = 1.0f;
	t_mtl.Diffuse.g = 1.0f;
	t_mtl.Diffuse.b = 1.0f;

	t_mtl.Ambient.a = _alpha;
	t_mtl.Diffuse.a = _alpha;

	m_middlePipe.set_material( t_mtl );

}







