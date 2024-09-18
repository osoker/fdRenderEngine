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
# pragma once

# include "../../interface/osInterface.h"





//! 淡入淡出状态的处理。
enum ose_fadeState
{
	//! 全显示状态。
	OSE_DISPLAY = 0,
	//! 处于淡出状态。
	OSE_FADEOUT,
	//! 消失状态。
	OSE_DISAPPEAR,
	//! 处于淡入状态。
	OSE_FADEIN,

	//! 半透明显示状态。
	OSE_HALFALPHA,

	//! River @ 2009-4-29: 半透明回复状态。
	OSE_HALFALPHAFADEIN,

};

# define HALFAHLPHA_VAL 0.5f


/** \brief
 *  对物品的淡入淡出状态进行管理的类。
 *   
 *  
 */
class  osc_fadeMgr
{
private:
	//! 当前是否处于face状态。
	ose_fadeState     m_eFadeState;


	//! 从改变状态到现在过去的时间。
	float            m_fFadeTime;

private:

	//! 淡出到达的时间状态。
	float            m_fFadeToTime;

public:
	osc_fadeMgr();

	//! 每一帧对物品的淡入淡出状态进行处理。
	void              frame_fade( float _time );

	//! 得到当前物品状态的alpha值。
	float             get_alpha( void );

	/** \brief
	 *  开始新的淡入淡出状态。
	 *
	 *  \param _fout 如果真，则开始淡出状态，如果false,则开始淡入状态。
	 *               如果当前处理淡入状态，就开始淡出，则从当前的alpha值开始淡出。
	 *               同理于淡出壮态。
	 *  \param _fv   淡出时，表示淡出到达的值，淡入时，表示淡入开始的值。
	 */
	void              start_fade( bool _fout,float _fv = 0.0f );

	/** \brief
	 *  得到当前物品是否处理fade状态。
	 */
	ose_fadeState      get_fadeState( void );

	//! 是否淡出淡入结束。
	bool              is_fadeOver( void );

	//! 设置当前的物品显示为半透明状态。
	void              half_alpha( void );

	//! River @ 2009-4-29: 设置当前的物品为半透明回复状态。
	void              half_alphaFadeIn( void );



};

//! fade管理器的vector. 
typedef std::vector<osc_fadeMgr>  VEC_fadeMgr;


inline osc_fadeMgr::osc_fadeMgr()
{
	m_eFadeState = OSE_DISAPPEAR;
	m_fFadeTime = 0.0f;
}


/** \brief
*  得到当前物品是否处理fade状态。
*/
inline ose_fadeState osc_fadeMgr::get_fadeState( void )
{
	return m_eFadeState;
}


//! 得到当前物品状态的alpha值。
inline float osc_fadeMgr::get_alpha( void )
{
	return m_fFadeTime/ALPHA_FADETIME;
}

//! 是否淡出淡入结束。
inline bool osc_fadeMgr::is_fadeOver( void )
{
	if( (m_eFadeState == OSE_DISAPPEAR)||
		(m_eFadeState == OSE_DISPLAY ) )
		return true;
	else
		return false;
}




