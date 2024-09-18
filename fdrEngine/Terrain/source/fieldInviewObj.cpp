//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fieldInviewObj.cpp
 *
 *  His:      River created @ 2004-2-24
 *
 *  Des:      地图渲染时的处理，地图调试信息的函数等都在这个文件中。
 *   
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../tools/include/fadeMgr.h"
# include "../../mfpipe/include/osCharacter.h"


//! 全局变量，控制当前物品进入视野时的淡入淡出状态。
OSENGINE_API bool              g_bFadeEnable = false;


//@ {
//  River @ 2009-12-30:
//  这两个变量控制了场景内物品淡入淡出的时间,物品折挡相机后的
//  半透明度由HALF_FADETIME/ALPHA_FADETIME获得。
// 
//! 全局的控制淡入淡出物品时间的变量。
OSENGINE_API float             ALPHA_FADETIME = 1.5f;
OSENGINE_API float             HALF_FADETIME = 0.60f;
//@}



osc_inViewObjMgr::osc_inViewObjMgr()
{

	//! 可见物品管理器对应的淡入管理器.
	m_vecFadeInMgr.resize( MAX_ADDDELOBJ );
	m_vecFadeInObj.resize( MAX_ADDDELOBJ );

	//! 淡出物品相关的数据.
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




//! 初始化当前的InviewObjMgr.
void osc_inViewObjMgr::init_ivoMgr( int _objNum )
{
	guard;

	m_iMaxNodeNum = _objNum;

	osDebugOut( "The Tg's obj num is:<%d>...\n",_objNum );
	
	m_vecNormalDis.resize( _objNum );
	m_vecLfVs.resize( _objNum );

	m_vecIvOs.resize( _objNum );


	// 初始化场景中挡住相机的透明物品的烈表.
	m_vecHalfFadeMgr.resize( MAX_HALFALPHAOBJ );
	memset( &m_vecAccuHAObj[0],0xffffffff,sizeof( int )*MAX_HALFALPHAOBJ );
	memset( &m_vecHalfAlphaObj[0],0xffffffff,sizeof( int )*MAX_HALFALPHAOBJ );


	// 
	// 初始化为场景中所有的物品都不在视野内部。
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

//! 从正常显示的物品队列中删除一个物品.
void osc_inViewObjMgr::delete_normalDisObj( int _idx )
{
	guard;

	osassert( _idx >= 0 );

	if( m_iNDObjNum <= 0 )
		return;

	// 
	// 先从正常显示的队列中移除要变为半透明的物品。
	int     t_i;
	osassert( m_iNDObjNum <= (int)m_vecNormalDis.size() );
	for( t_i=0;t_i<m_iNDObjNum;t_i ++ )
	{
		if( m_vecNormalDis[t_i] == _idx )
		{
			// 正常渲染的物品数目变小
			m_iNDObjNum --;
			for( int t_j = t_i;t_j<m_iNDObjNum;t_j ++ )
				m_vecNormalDis[t_j] = m_vecNormalDis[t_j+1];

			break;
		}
	}

	return;

	unguard;
}


//! 把一个正常显示的物品变为半透明的物品。
void osc_inViewObjMgr::make_halfFade( int _objIdx )
{
	guard;

	int   t_i;

	//
	// 把要变为半透明的物品放到fadeIn队列。
	for( t_i=0;t_i<MAX_HALFALPHAOBJ;t_i ++ )
	{
		if( m_vecHalfAlphaObj[t_i] == -1 )
			break;
	}

	//! river mod @ 2009-4-29:无须过多的错误处理，物品不透明，最多玩家需根转动
	//  一下相机。更优化的做法是，找到正在fadein的物品，然后正常显示这些物品。
	if( t_i == MAX_HALFALPHAOBJ )
		return;

	m_vecHalfAlphaObj[t_i] = _objIdx;

	unguard;
}



//! 开始新一帧的处理.
void osc_inViewObjMgr::frame_setObjMgr( void )
{
	guard;

	int          t_i;
	float        t_fEtime = sg_timer::Instance()->get_lastelatime();

	m_iNDObjNum = 0;

	//
	// 对fade In 队列的数据进行处理.
	for( t_i =0;t_i<MAX_ADDDELOBJ;t_i ++ )
	{
		if( m_vecFadeInObj[t_i] == -1 )
			continue;

		if( m_vecFadeInMgr[t_i].is_fadeOver() )
		{
			if (m_vecFadeInObj[t_i]<0||m_vecFadeInObj[t_i]>=(int)m_vecIvOs.size())
			{
				osassertex(false,"待检查的错误!");
			}
			m_vecIvOs[m_vecFadeInObj[t_i]] = OSE_IVO_NORMALDIS;
			m_vecFadeInObj[t_i] = -1;
		}
		else
			m_vecFadeInMgr[t_i].frame_fade( t_fEtime );

	}

	//
	// 对fade Out队列的数据进行处理.
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
	// 对挡住相机的透明物品.
	for( int t_i=0;t_i<MAX_HALFALPHAOBJ;t_i ++ )
	{
		if( m_vecAccuHAObj[t_i] == -1 )
			continue;
		
		m_vecHalfFadeMgr[t_i].frame_fade( t_fEtime );
	}

	return;

	unguard;
}


//! 添加一个fadeInObject.
bool osc_inViewObjMgr::add_fadeInObj( int _idx )
{
	guard;

	int    t_i;

	// 如果相机中有旋转操作，则不淡入。
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
	// ATTENTION TO FIX:对离相机远的物品，直接退出渲染。　
	// 如果没有空间，进行特殊处理，
	if( t_i == MAX_ADDDELOBJ )
	{
		// 如果淡入物品的空间不够，则直接显示
		m_vecIvOs[_idx] = OSE_IVO_NORMALDIS;
		return false;
	}

	m_vecFadeInObj[t_i] = _idx;

	// 开始淡入。
	m_vecFadeInMgr[t_i].start_fade( false );

	return true;

	unguard;
}


//! 添加一个FadeOutObject
bool osc_inViewObjMgr::add_fadeOutObj( int _idx )
{
	guard;

	int    t_i;


	
	// 如果相机中有旋转操作，则不淡出。
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
	// 如果没有空间，进行特殊处理，
	if( t_i == MAX_ADDDELOBJ )
	{
		m_vecIvOs[_idx] = OSE_IVO_NOTINVIEW;
		return false;
	}

	m_vecFadeOutObj[t_i] = _idx;


	// 开始淡入。
	m_vecFadeOutMgr[t_i].start_fade( true );

	return true;

	unguard;
}


//! 每一帧去挡住相机射线的透明物品的处理.
void osc_inViewObjMgr::process_halfFadeObj( void )
{
	guard;

	static bool  t_bInSec[MAX_HALFALPHAOBJ];
	int         t_i,t_j;

	//! River added @ 2009-4-29:先处理fadeIn的物品，对于已经正常显示的物品，删除。
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

		// 从正常显示的队列中删除此物品索引.
		delete_normalDisObj( m_vecHalfAlphaObj[t_i] );

		// 如果此索引已经存在于目前场景中的半透明物品列表中, 下一个.
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

	// 对没有挡住射线,而又在半透明物品列表中的索引进行处理.
	for( t_i=0;t_i<MAX_HALFALPHAOBJ;t_i++ )
	{
		//
		// River @ 2009-4-29:应该增强此处的功能,由淡出变成淡入,不应该直接的变成可以显示.
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

	// 对半透明的物品队列进行处理。
	memset( &m_vecHalfAlphaObj[0],0xffffffff,sizeof( int )*MAX_HALFALPHAOBJ );

	unguard;
}



/** \brief
*  每一帧中得到当前TG内物品列表后，对inviewObjMgr内物品的处理。
*
*  River @ 2005-3-11: 如果是因为相机的旋转而淡入淡出物品，把淡入的物品
*                  　直接加入到可视队列，把淡出的物品直接从视野中删除。
*                    只有相机在移动时，才在最远处淡入淡出物品，增加三维场景
*                    的真实度。　
*
*  ATTENTION TO OPP:这个函数内的[]操作符占用了整个渲染的很大百分比，是很费时的一项操作。
*  如果g_bFadeEnable设为false,则帧速率能提高不少!!!!!
*/
void osc_inViewObjMgr::frame_processIvo( VEC_BYTE& _ivoList )
{
	guard;

	int    t_i;
	int    t_iSize = (int)_ivoList.size();

	osassert( t_iSize <= m_iMaxNodeNum );
	//
	// 如果全局的fade状态没有打开的话,简洁的处理。
	if( !g_bFadeEnable )
	{
		for( t_i=0;t_i<t_iSize;t_i ++ )
		{
			// 
			// 在可视范围内
			if( _ivoList[t_i] > MASK_OUTHALF_NOTVIEW )
			{
				m_vecNormalDis[m_iNDObjNum] = t_i;
				m_iNDObjNum ++;
			}
		}

		// 
		// 忘了执行这个步骤的话，会出一"过多物品挡住相机"的bug.
		// 对挡住相机的透明物品的处理.
		if( g_bAlphaShelter )
			process_halfFadeObj();

		return;
	}


	osassert( m_iNDObjNum >= 0 );

	// 
	// 对TG内的每一个物品进行处理。
	for( t_i =0;t_i<t_iSize;t_i ++ )
	{
		osassert( t_i < m_iMaxNodeNum );
		//
		// 上一帧和当前帧物品都不在视野内。
		if( (!m_vecLfVs[t_i])&&
			(_ivoList[t_i]<MASK_HALF_VIEW) )
		{
			// 两帧中物品都不在视野内，不需要处理

			continue;
		}

		//
		// 上一帧和当前帧此物品都在视野内部。
		if( m_vecLfVs[t_i]&&
			(_ivoList[t_i]>=MASK_HALF_VIEW) )
		{
			if( m_vecIvOs[t_i] == OSE_IVO_NORMALDIS )
			{
				// 不需要处理物品的状态。物品静止显示就可以了。
				m_vecNormalDis[m_iNDObjNum] = t_i;
				m_iNDObjNum ++;
			}

			continue;
		}

		//
		// 上一帧在视野内，当前帧不在视野内。
		if( (m_vecLfVs[t_i])&&
			(_ivoList[t_i]<MASK_HALF_VIEW) )
		{
			// 
			// 如果物品不在视野的一半矩离外部，则不需要淡出
			if( _ivoList[t_i]!=MASK_OUTHALF_NOTVIEW )
			{
				m_vecIvOs[t_i] = OSE_IVO_NOTINVIEW;
			}
			else
			{
				m_vecIvOs[t_i] = OSE_IVO_FADEOUT;

				// 往淡出队列中加入物品,
				add_fadeOutObj( t_i );
			}
			m_vecLfVs[t_i] = false;


			continue;
		}


		// 
		// 上一帧不在视野内，当前帧在视野内。
		if( (!m_vecLfVs[t_i])&&
			(_ivoList[t_i]>=MASK_HALF_VIEW) )
		{
			// 如果物品在视野的一半矩离内部，则不需要淡出.
			if( _ivoList[t_i]==MASK_HALF_VIEW )
			{
				m_vecIvOs[t_i] = OSE_IVO_NORMALDIS;
			}
			else
			{
				m_vecIvOs[t_i] = OSE_IVO_FADEIN;

				// 往淡入队列中加入新的物品。
				add_fadeInObj( t_i );
		
			}
			m_vecLfVs[t_i] = true;

		}

	}

	// 对挡住相机的透明物品的处理.
	if( g_bAlphaShelter )
		process_halfFadeObj();


	return;

	unguard;
}


/** \brief
*  设置场景中用到的Material值。
*
*  \param _alpha 当前material用到的alpha值。
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







