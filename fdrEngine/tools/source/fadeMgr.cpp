//=======================================================================================================
/**  \file
 *   Filename: fadeMgr.h
 *
 *   Desc:     ������Զ������Ʒ�ɵ��뵽��Ұ��Ҳ�ɵ�����Ұ������ļ�ʵ�ֵ��뵭����Ҫ�����ݡ�
 *
 *   His:      River created @ 2004-6-3
 *
 *   "��ѧ֮�����������£����������������ơ�"
 *  
 */
//=======================================================================================================
# include "stdafx.h"
# include "../include/fadeMgr.h"




//! ÿһ֡����Ʒ�ĵ��뵭��״̬���д���
void osc_fadeMgr::frame_fade( float _time )
{

	// ����ʱ��fadeʱ��ӡ�
	if( m_eFadeState == OSE_FADEOUT )
	{
		m_fFadeTime -= _time;
		if( m_fFadeTime <= 0.0f )
		{
			m_eFadeState = OSE_DISPLAY;
			m_fFadeTime = 0.0f ;
		}
	}

	//  ����ʱ��fadeʱ�����
	if( m_eFadeState == OSE_FADEIN )
	{
		m_fFadeTime += _time;
		if( m_fFadeTime >= ALPHA_FADETIME )
		{
			m_eFadeState = OSE_DISAPPEAR ;
			m_fFadeTime = ALPHA_FADETIME;
		}
	}

	// 
	// �������ʾ��Ʒ�ɰ�͸��״̬�������ÿһ��֡�������á�
	// �����һ֡��ʾ���ǰ�͸����״̬,������Ϊ��͸����
	if( m_eFadeState == OSE_HALFALPHA )
	{
		if( m_fFadeTime > HALF_FADETIME )
		{
			// 
			// ���ٱ�Ϊ��͸��
			m_fFadeTime -= (_time*3.0f);
			if( m_fFadeTime < HALF_FADETIME )
				m_fFadeTime = HALF_FADETIME;
		}
	}

	//
	// River added @ 2009-4-29:��Ʒ��ס����뿪��fadeinЧ����
	// ������ٶ���һЩ���������뿪��Ʒ�۵���ʱ����fadeIn.
	if( m_eFadeState == OSE_HALFALPHAFADEIN )
	{
		m_fFadeTime += (_time*0.4f);
		if( m_fFadeTime >= ALPHA_FADETIME )
		{
			m_eFadeState = OSE_DISPLAY;
			m_fFadeTime = 0.0f ;
		}
	}

	return;

}



//! ���õ�ǰ����Ʒ��ʾΪ��͸��״̬��
void osc_fadeMgr::half_alpha( void )
{
	// �ķ�֮һ͸����
	// River @ 2009-4-29:������ڰ�͸���ظ�״̬��ֱ��ʹ�õ�ǰ��Alpha.
	if( m_eFadeState != OSE_HALFALPHAFADEIN )
		m_fFadeTime = ALPHA_FADETIME;
	m_eFadeState = OSE_HALFALPHA;

}

//! River @ 2009-4-29: ���õ�ǰ����ƷΪ��͸���ظ�״̬��
void osc_fadeMgr::half_alphaFadeIn( void )
{
	//! �ӵ�ǰ��͸����״̬���ص���͸����״̬��
	if( m_eFadeState == OSE_HALFALPHA )
		m_eFadeState = OSE_HALFALPHAFADEIN;
}



/** \brief
*  ��ʼ�µĵ��뵭��״̬��
*
*  \param _fout ����棬��ʼ����״̬�����false,��ʼ����״̬��
*               �����ǰ������״̬���Ϳ�ʼ��������ӵ�ǰ��alphaֵ��ʼ������
*               ͬ���ڵ���׳̬��
*/
void osc_fadeMgr::start_fade( bool _fout,float _fv/* = 0.0f*/ )
{
	// ����ǿ�ʼ����״̬
	if( _fout )
	{
		m_eFadeState = OSE_FADEOUT;

		m_fFadeTime = ALPHA_FADETIME*(1-_fv);

	}
	else
	{
		m_eFadeState = OSE_FADEIN;
		m_fFadeTime = 0.0f;

		m_fFadeTime = ALPHA_FADETIME*_fv;

	}
}





