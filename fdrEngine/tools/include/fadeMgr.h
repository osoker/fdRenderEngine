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
# pragma once

# include "../../interface/osInterface.h"





//! ���뵭��״̬�Ĵ���
enum ose_fadeState
{
	//! ȫ��ʾ״̬��
	OSE_DISPLAY = 0,
	//! ���ڵ���״̬��
	OSE_FADEOUT,
	//! ��ʧ״̬��
	OSE_DISAPPEAR,
	//! ���ڵ���״̬��
	OSE_FADEIN,

	//! ��͸����ʾ״̬��
	OSE_HALFALPHA,

	//! River @ 2009-4-29: ��͸���ظ�״̬��
	OSE_HALFALPHAFADEIN,

};

# define HALFAHLPHA_VAL 0.5f


/** \brief
 *  ����Ʒ�ĵ��뵭��״̬���й�����ࡣ
 *   
 *  
 */
class  osc_fadeMgr
{
private:
	//! ��ǰ�Ƿ���face״̬��
	ose_fadeState     m_eFadeState;


	//! �Ӹı�״̬�����ڹ�ȥ��ʱ�䡣
	float            m_fFadeTime;

private:

	//! ���������ʱ��״̬��
	float            m_fFadeToTime;

public:
	osc_fadeMgr();

	//! ÿһ֡����Ʒ�ĵ��뵭��״̬���д���
	void              frame_fade( float _time );

	//! �õ���ǰ��Ʒ״̬��alphaֵ��
	float             get_alpha( void );

	/** \brief
	 *  ��ʼ�µĵ��뵭��״̬��
	 *
	 *  \param _fout ����棬��ʼ����״̬�����false,��ʼ����״̬��
	 *               �����ǰ������״̬���Ϳ�ʼ��������ӵ�ǰ��alphaֵ��ʼ������
	 *               ͬ���ڵ���׳̬��
	 *  \param _fv   ����ʱ����ʾ���������ֵ������ʱ����ʾ���뿪ʼ��ֵ��
	 */
	void              start_fade( bool _fout,float _fv = 0.0f );

	/** \brief
	 *  �õ���ǰ��Ʒ�Ƿ���fade״̬��
	 */
	ose_fadeState      get_fadeState( void );

	//! �Ƿ񵭳����������
	bool              is_fadeOver( void );

	//! ���õ�ǰ����Ʒ��ʾΪ��͸��״̬��
	void              half_alpha( void );

	//! River @ 2009-4-29: ���õ�ǰ����ƷΪ��͸���ظ�״̬��
	void              half_alphaFadeIn( void );



};

//! fade��������vector. 
typedef std::vector<osc_fadeMgr>  VEC_fadeMgr;


inline osc_fadeMgr::osc_fadeMgr()
{
	m_eFadeState = OSE_DISAPPEAR;
	m_fFadeTime = 0.0f;
}


/** \brief
*  �õ���ǰ��Ʒ�Ƿ���fade״̬��
*/
inline ose_fadeState osc_fadeMgr::get_fadeState( void )
{
	return m_eFadeState;
}


//! �õ���ǰ��Ʒ״̬��alphaֵ��
inline float osc_fadeMgr::get_alpha( void )
{
	return m_fFadeTime/ALPHA_FADETIME;
}

//! �Ƿ񵭳����������
inline bool osc_fadeMgr::is_fadeOver( void )
{
	if( (m_eFadeState == OSE_DISAPPEAR)||
		(m_eFadeState == OSE_DISPLAY ) )
		return true;
	else
		return false;
}




