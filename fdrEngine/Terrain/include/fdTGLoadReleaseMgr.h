//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdTGLoadReleaseMgr.h
 *
 *  His:      River created @ 2006-1-10
 *
 *  Des:      用于管理当前整个场景内的地图,哪个应该调入,哪个应该退出内存等
 *   
 * “欲先取之,必先予之” 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# pragma warning( disable : 173 )

//! 包含地图管理器用到的相关的数据结构。
# include "fdTGQuadtree.h"
# include "../include/fdFieldStruct.h"
# include "fdNewFieldStruct.h"

# include "../../effect/include/fdRain.h"
# include "../../effect/include/fdlensflare.h"
# include "../GeoMipTerrain/CeoMipTerrainAll.h"
# include "../../effect/include/fdSpecialEffect.h"



typedef osc_newTileGround* osc_TileGroundPtr;


struct os_tgMTLoader
{
	s_string         m_szMapName;
	osc_TileGroundPtr   m_ptrTG;
	int             m_iMapIdx;

	//! 此地图调入完成后, 通知上层完成了地图调入.
	BOOL             m_bFinishNotify;
public:
	os_tgMTLoader()
	{
		m_bFinishNotify = FALSE;
	}
};


//! TG管理器中，最少必须保证的TG数目,此数目保证可以在场景中最远看到192米的矩离

# if __QTANG_VERSION__
  # define MINTG_INTGMGRMACRO       4
  # define MAXTG_INTGMGRMACRO       4
# else
  # define MINTG_INTGMGRMACRO       9
  # define MAXTG_INTGMGRMACRO       16
# endif 


/** \brief
 *  多线程调入地图块的数据封装处理。
 *
 *  算法描述: 每调入一个地图,处理一下这个类内部的数据,看是否需要做其它的操作.
 *  １: 主线程在每一帧中调用函数:frame_moveMLMgr
 *  ２: 如果地图列表中存在要调入的地图,开新线程,调入地图.
 *  ３: 调入完一个地图后,阻塞当前的线程,然后处理主线程中的事件.
 *  ４: 主线程可能会关闭当前的调入线程或是继续执行当前的线程.
 *  
 *  调入地图的线程在地图管理器初始化后一直存在,平时主要是Sleep,在有需要调入的地图时.
 *  开始调入地图,调入完成后,继续空闲的线程.
 *  
 */
class osc_mapLoadThread : public com_thread
{
private:
	//! 当前正在调入的地图列表。
	std::list< os_tgMTLoader >    m_sMapNeedLoad;

	//! 当前正调入的地图.
	os_tgMTLoader                m_sCurLoadMap;

	//! 地图管理器.
	osc_TGManager*               m_ptrTGMgr;

	//! 地图调入和释放管理器,以后用这个管理器替代之上的TGManager.
	osc_tgLoadReleaseMgr*         m_ptrLRMgr;

	//! 是否唤醒地图调入线程的event.
	static HANDLE                m_sEventStartMt;

	//! 地图调入在阻塞模式下,使用这个变量通知上层完成了地图调入.
	HANDLE                      m_sEventMapLoadFinish;

	//! 线程退出的事件，跟m_sEventStartMt配合，来确认让线程退出。
	HANDLE                      m_sEventExitThread;

private:
	
	//! 地图调入线程是否处于空闲状态，在等待线程被唤醒的消息
	static HANDLE               m_sEventThreadFree;

public:
	//@{
	//! 全局使用的critical_section.

	//! 在推进和读出要调入的地图名字和指针时,使用的临界区.
	static CRITICAL_SECTION   m_sMapNameCS;


	//! 临界创建粒子时用到的资源.
	static CRITICAL_SECTION   m_sParticleCS;

	//! 临界创建mesh时用到的资源.
	static CRITICAL_SECTION   m_sMeshCS;

	//! 临界创建osa时用到的资源.
	static CRITICAL_SECTION   m_sOsaCS;

	//! 临界创建billboard用到的资源.
	static CRITICAL_SECTION   m_sBbCS;

	//! 临界shader创建用到的资源.
	static CRITICAL_SECTION   m_sShaderCS;

	//! 创建Effect需要的CS.
	static CRITICAL_SECTION   m_sDxEffectCS;

	//! 临界shader创建用到的资源.
	static CRITICAL_SECTION   m_sShaderReleaseCS;


	//! 临界创建特效用到的资源
	static CRITICAL_SECTION   m_sEffectCS;

	//! 场景内活动地图列表更新时，用到的临界区资源，上层可能用到相关的信息
	static CRITICAL_SECTION   m_sActiveMapCS;


private:

	//! 从队列中弹出地图列表.
	void                    pop_loadMap( void );  

	//! 清空要调入地图的队列.
	void                    clear_loadMap( void );

	//! 得到最前端的地图.
	BOOL                    get_frontMap( os_tgMTLoader& _tgLoad ); 


public:
	osc_mapLoadThread();
	~osc_mapLoadThread();

	//! 传入内部使用的指针
	void                    init_mtLoadMapMgr( 
		                      osc_TGManager* _mgr,osc_tgLoadReleaseMgr* _lrMgr );


	//! 每一帧对多线程地图调入管理器进行处理。
	void                    frame_moveMLMgr( void );

	/** \brief
	 *  给管理器指定一个要调入的地图。
	 *
	 *  \param _mapName 要调地图的名字.
	 *  \param _mapIdx 调入地图时,使用的地图索引.ATTENTION TO FIX:
	 *  \param _waitForFinish 是否等待另外的线程完成地图的调入,
	 *                  此机制用于完成阻塞式的地图调入.
	 */
	void                    push_loadMap( char* _mapName,
		                      int _mapIdx,BOOL _waitForFinish = FALSE );

	//! 清空管理器要调入的地图,在释放整个地图时，可能有另外的线程正在调入地图，处理。
	void                    reset_loadMapMgr( void );

	/** \brief
	 *  是否是可操作的TG,即可使用的tg.
	 *
	 *  在地图调入线程的置换地图队列中没有当前tg.这个tg就是activeTg,
	 *  为上层渲染需要使用到的tg.
	 */
	bool                    is_activeTg( int _idx );

	//! 当前是否处于闲置状态,即等待主线程推进要调入的地图.
	bool                    is_freeStatus( void );


	//! 得到要调入的地图的名字.
	void                    get_loadMapName( int _idx,char* _name,int _maxname );

	/**
	 *  每一个派生类都必须重载的函数.
	 *
	 *  函数处理流程:
	 *  
	 */
	virtual DWORD            ThreadProc();
	virtual void ThreadExceptionProc(){}


	//! 结束线程的执行
	void                    end_thread( void );
	
};




/** \brief
 *  对地图进行管理的类,用于确认哪一块地图应该在内存中,下一次要交换哪一块地图出内存.
 *
 *  此管理器必须考虑所有的地图都调入到的内存中的情形.维护一个激活的内存列表,使用这
 *  个列表来让tgMgr来处理场景中的可见和渲染,使用这个列表来更新这个激活的列表.
 *  
 *  地图全部调入到内存是把硬盘上的地图文件调入到的内存中,但这些内存并不去创建设备相关
 *  的数据.
 *
 *  有关地图调入，释放，置换的工作全部由这个类来实现? ( 自由移动等等 )
 * 
 *  给上层提供出确认可以使用的TG列表: 在这个类的内部来处理相关的多线程调入和地图置换算法.
 * 
 *  ATTENTION : 此类只把握地图的地块数据和置换,整体地图的信息目前不受这个类管理.
 *  
 *  多线程调入地图在初始化地图的时候,是阻塞式的调入.只有在运行的过程中,才会在另一个线程中
 *  动态的调入
 *
 *  这个管理类目前尽量简单，把除地图管理外的所有工作都放到上层的osc_TGManager类中去做。
 *
 *  对于全部把地图调入到内存中的情形,可以只调入相应的地图硬盘数据,到需要显示地图的时候,
 *  在地图调入线程中激活地图,并创建此地图相关的设备相关数据.
 */
class osc_tgLoadReleaseMgr
{
	friend class    osc_mapLoadThread;

private:
	//! 整体地图的名字，比如map\vilg
	s_string            m_strMapName;
	//! 中间管道的指针.
	osc_middlePipe*      m_ptrMidPipe;

	//! 真正存储从硬盘上调入的tg数据的指针.
	osc_newTileGround*      m_ptrRealTg;
	//! 管理器中总共的tg数目。
	int                 m_iCurTgNum;

	/** \brief 
	 *  维护用于上层来计算可见性和渲染的tg列表.
	 *
	 *  数组内变量可以为空.在mgr正调入某个tg时,此处对应的tg就为空.
	 */
	osc_TileGroundPtr    m_ptrActTg[MAXTG_INTGMGRMACRO];

	/** \brief
	 *  需要动态调入的tg名字数据.
	 * 
	 *  当前相机的位置发生变化时,使用此结构做为临时变量,得到需要动态调入的地图列表.
	 */
	os_tileIdx          m_vecNeedLoadTile[MAXTG_INTGMGRMACRO];
	//! 需要重新处理的

	/** \brief
	 *  是否把地图全部的调入到内存中.
	 *
	 *  如果不是TotalLoad,则内部使用MAXTG_INTGMGR个tg做为内存管理的tg.
	 *
	 */
	bool                m_bTLoadInMem;

	//! 当前应该释放的tg指针列表.
	int                 m_vecTgNeedRelease[MAXTG_INTGMGRMACRO];
	int                 m_iNeedRelTgNum;

	//! 应该置换入内存的tg指针列表。
	os_tileIdx           m_vecTgNeedSwapIn[MAXTG_INTGMGRMACRO];
	int                 m_iNeedSwapInNum;
	//! 置换入内存的tg队列前面几个需要阻塞调入。以后可优化为使用不同的调入优化级别.
	int                 m_iBlockLoad;

	//! 用于调入地图数据的线程管理器。
	osc_mapLoadThread    m_sThreadMgr;


	/** \brief
	 *  X,Z方向上地图块的最大索引值,此值为显示格子的牵引值.
	 *  以64为基准,为64的倍数.
	 */
	int              m_iMaxX;
	int              m_iMaxZ;

	//! 整个地图中x,z方向最大的图块索引数目。
	int              m_iMaxXTgNum;
	int              m_iMaxZTgNum;

	//@{ 
	//  以下数据将根据相机的位置,来计算当前应该有哪些地图应该置换出内存或是
	//  设置为激活的地图.
	//! 上一帧相机在地图上的位置.
	osn_inTgPos       m_eLFTgPos;
	//! 上一帧相机所在的tg.
	os_tileIdx        m_sLFCamInIdx;

	//! 当前帧相机所在的x,z碰撞格子位置.
	int              m_iCamInX,m_iCamInZ;



private:
	//! 内部的相关多线程数据结构.

	//  得到地图管理器初始化的位置信息.
	void             init_insertInfo( os_sceneLoadStruct* _sl );

	/** \brief
	 *  根据相机的位置，计算出当前帧需要调入的图块列表.
	 *
	 *  \param return bool 返回值表示了是否需要对当前的显示地图块进行置换.
	 *  
	 */
	bool             get_tgNeedInMemList( osVec3D& _pos );


	//! 计算需要从内存中置换出的地图图块列表 
	void             process_needSwapOutTgList( void );

	/** \brief
	 *  根据传入的地图名字列表，来计算出需要释放的地图名字
	 *
	 *  此函数用于多线程状态下的人物自由移动.
	 */
	void             spacemove_swapOutMap( s_string* _mapList );

	//! 计算需要在内存中存在，目前在内存中又不存在的图块列表。 
	void             process_needSwapInTgList( void );

	//! 把应该存在于内存,又不在内存的tg推进给另外的线程,调入.
	bool             push_needSwapInMap( void );


	//! 根据当前的相机，对要置换入的地图列表排序
	bool             sort_needSwapInMapList( osc_camera* _cam );



	/** \brief
	 *  整理出上层可用的地图列表,即激活的地图列表.
	 *
	 */
	void                get_activeTgList( void );

	//@{

	//! 得到当前tg周围一圈的tg,共8个，这些tg肯定需要调入到内存。
	void                get_surroundTG( os_tileIdx* _mapIdx,int _my,int _mx );


	//! 根据当前在tg中的位置,来调入当前tg第二圈的数据.
	void                get_secSurTG( os_tileIdx* _mapIdx,int _my,int _mx );

	//@}

	/** \brief
	 *  从客户端填充的场景调入结构中得我们要调入的场景文件名。
	 *
	 *  \param  _nameArr 传入的4个长度的s_string字串，我们最多静态
	 *                   调入4张地图，在这个数据中返回我们要调入的地图名，
	 *                   如果地图名为空，则不需要调入这个地图。
	 *
	 * 
	 */
	bool                get_loadSceneName( 
		                 s_string* _nameArr,os_sceneLoadStruct* _sl );


	/** \brief
	 *  多线程安全的地图调入代码.
	 *
	 *  替换TGManager中的相关函数.
	 */
	bool      threadSafe_loadNMap( char* _nmapFname,osc_TileGroundPtr _tg );


	/** \brief
	 *  如果另外的线程正在调入地图,终止当前的地图调入状态.
	 *
	 */
	void                 stop_threadMapLoading( void );


public:
	osc_tgLoadReleaseMgr();
	~osc_tgLoadReleaseMgr();

	//! 
	void                init_tgLRMgr( osc_TGManager* _tgMgr );

	/** \brief
	 *  传入相机，根据相机来得到应该可见的地图列表, 激活的地图烈表.
	 *
	 *  每一帧需要调用这个函数，在后台来处理地图的调入.
	 *  传回上层当前可用的地图指针列表和可用的地图数目.
	 *
	 *  1: 根据相机的位置，计算出当前帧相机所在的Tg与Tg内的方位,
	 *     与上一帧存储的数据比较,算出是否需要置换地图.
	 *  
	 *  2: 处理置换的地图，推进到地图调入线程。
	 *
	 *  3: 算出上层可以使用的地图列表。 返回地图个数。
	 */
	int                 frame_getInViewTg( osc_camera* _cam,
		                  osc_TileGroundPtr* _tgList,int _maxNum );

	//! 初始化地图后，得到当前激活的地图列表，此时还没有相机加入，所以加入单独的函数。
	int                 init_getActMapList( osc_TileGroundPtr* _tgList,int _maxNum );

	/** \brief
	 *  初次调入地图的函数。
	 *
	 *  相当于接替了原来的osc_TGManager中的load_scene函数.
	 *
	 *  \param _sl 调入地图的结构。
	 *  \param _maxX,_maxZ 地图在x,z方向上的最大地图块数。
	 *  
	 */
	bool                load_scene( os_sceneLoadStruct* _sl,int _maxX,int _maxZ );

	//! 整体释放当前的地图内存管理器。
	void                release_tgLRMgr( void );


	//! 自由移动的处理函数跟此函数。
	bool                space_moveMapLoad( os_sceneLoadStruct* _sl,int _maxX,int _maxZ );


};

