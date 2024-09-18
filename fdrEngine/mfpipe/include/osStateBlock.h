//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osStateBlock.h
 *
 *   Desc:     O.S.O.K 引擎的渲染块管理器
 *
 *   His:      River Created @ 2004-12-14
 *
 *  “在催人进步的压力中，你自己对你的压力远不如别人对你的压力，别人对你的
 *    压力远不如公众对你的压力”。
 */
//--------------------------------------------------------------------------------------------------------
# pragma    once

# include "../../interface/osInterface.h"
# include "../../interface/miskFunc.h"


//! 整个场景中最多可以有的block数目。
# define MAX_STATEBLOCK     36


/** \brief
 *  管理引擎内部使用的渲染状态块的管理器。
 *
 *  因为StateBlock全都是全局性的，在引擎开始到结束都需要使用，
 *  所以不需要有释放StateBlock的公开接口。
 *  
 *  全局只需要一个实例就可以了。
 *
 */
class os_stateBlockMgr
{
private:
	LPDIRECT3DDEVICE9       m_pd3dDevice;


	static os_stateBlockMgr* m_ptrInstance;

	//! DX内部渲染状态块指针的列表
	LPDIRECT3DSTATEBLOCK9*   m_vecStateBlock;
	VEC_bool                m_vecSBUsed;
	VEC_word                m_vecSBCreateTimes;

	os_stateBlockMgr();
	~os_stateBlockMgr();

	// TEST CODE:
	BOOL                    m_bRecordState;

public:

	/** 得到一个sg_timer的Instance指针.
	 */
	static os_stateBlockMgr*    Instance();
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void                DInstance();


	//! 初始化渲染状态块管理器。
	void       init_sbMgr( LPDIRECT3DDEVICE9 _dev );

	//! 得到一个渲染状态块。
	int        create_stateBlock( void );

	//! 开始录制状态块。
	void       start_stateBlock( void );

	//! 结束录制状态块。
	void       end_stateBlock( int _idx );

	/** \brief
	 *  当前的stateBlock ID 是否有效.
	 *
	 *  如果device lost ,则每一个id都无效,需要重新的创建每一个地方的state block.
	 *  为了使每一个state Block真正是当前的block,使用了创建次数和sb的id共同发给上
	 *  层,做为sb的id,不然Reset后会出现如下的问题:
	 *  对某一个stateId validate时,返回了正确的值.但底层的stateblock是别的地方创建
	 *  的stateBlock,这种情况下会出错.
	 */
	bool       validate_stateBlockId( int _id );


	//! 把渲染状态块使用到设备。
	void       apply_stateBlock( int _idx );

	//! 释放一个渲染状态块,要释放的地图块id变为-1.
	void       release_stateBlock( int& _idx );

	//! 在设备lost后,全部的释放当前的stateBlock资源.
	void       sbm_onLostDevice( void );

};

/** 得到一个sg_timer的Instance指针.
*/
inline os_stateBlockMgr* os_stateBlockMgr::Instance()
{
	if( m_ptrInstance )
		return m_ptrInstance;
	else
	{
		m_ptrInstance = new os_stateBlockMgr;
		return m_ptrInstance;
	}
}


//! 初始化渲染状态块管理器。
inline void os_stateBlockMgr::init_sbMgr( LPDIRECT3DDEVICE9 _dev )
{
	osassert( _dev );
	m_pd3dDevice = _dev;
}




//! 开始录制状态块。
inline void os_stateBlockMgr::start_stateBlock( void )
{
	m_bRecordState = TRUE;
	if( FAILED( m_pd3dDevice->BeginStateBlock() ) )
		osassert( false );
}


/** \brief
*  当前的stateBlock ID 是否有效.
*
*  如果device lost ,则每一个id都无效,需要重新的创建每一个地方的state block.
*/
inline bool os_stateBlockMgr::validate_stateBlockId( int _id )
{
	osassert( ( ((WORD)_id) >= 0)&&(((WORD)_id)<MAX_STATEBLOCK) );

	if( osn_mathFunc::get_hiword( _id ) == m_vecSBCreateTimes[(WORD)_id] )
		return m_vecSBUsed[(WORD)_id];
	else
		return false;
}
