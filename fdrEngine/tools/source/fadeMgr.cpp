//=======================================================================================================
/**  \file
 *   Filename: fadeMgr.h
 *
 *   Desc:     场景中远处的物品可淡入到视野，也可淡出视野，这个文件实现淡入淡出需要的数据。
 *
 *   His:      River created @ 2004-6-3
 *
 *   "大学之道，大明明德，在亲民，在至于至善。"
 *  
 */
//=======================================================================================================
# include "stdafx.h"
# include "../include/fadeMgr.h"




//! 每一帧对物品的淡入淡出状态进行处理。
void osc_fadeMgr::frame_fade( float _time )
{

	// 淡出时，fade时间加。
	if( m_eFadeState == OSE_FADEOUT )
	{
		m_fFadeTime -= _time;
		if( m_fFadeTime <= 0.0f )
		{
			m_eFadeState = OSE_DISPLAY;
			m_fFadeTime = 0.0f ;
		}
	}

	//  淡入时，fade时间减。
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
	// 如果想显示物品成半透明状态，则必须每一个帧进行设置。
	// 如果上一帧显示的是半透明的状态,则设置为不透明。
	if( m_eFadeState == OSE_HALFALPHA )
	{
		if( m_fFadeTime > HALF_FADETIME )
		{
			// 
			// 快速变为半透明
			m_fFadeTime -= (_time*3.0f);
			if( m_fFadeTime < HALF_FADETIME )
				m_fFadeTime = HALF_FADETIME;
		}
	}

	//
	// River added @ 2009-4-29:物品挡住相机离开后，fadein效果。
	// 淡入的速度慢一些，让人物离开物品折挡的时候，再fadeIn.
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



//! 设置当前的物品显示为半透明状态。
void osc_fadeMgr::half_alpha( void )
{
	// 四分之一透明。
	// River @ 2009-4-29:如果处于半透明回复状态，直接使用当前的Alpha.
	if( m_eFadeState != OSE_HALFALPHAFADEIN )
		m_fFadeTime = ALPHA_FADETIME;
	m_eFadeState = OSE_HALFALPHA;

}

//! River @ 2009-4-29: 设置当前的物品为半透明回复状态。
void osc_fadeMgr::half_alphaFadeIn( void )
{
	//! 从当前的透明度状态，回到不透明的状态。
	if( m_eFadeState == OSE_HALFALPHA )
		m_eFadeState = OSE_HALFALPHAFADEIN;
}



/** \brief
*  开始新的淡入淡出状态。
*
*  \param _fout 如果真，则开始淡出状态，如果false,则开始淡入状态。
*               如果当前处理淡入状态，就开始淡出，则从当前的alpha值开始淡出。
*               同理于淡出壮态。
*/
void osc_fadeMgr::start_fade( bool _fout,float _fv/* = 0.0f*/ )
{
	// 如果是开始淡出状态
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





