//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdTrack.h
 *
 *  Desc:     处理引擎中的路径数据,用于显示场景中的特效.
 *
 *  His:      River created @ 2004-2-18
 *
 *  "不是东风压倒西风，就是西风压倒东风"
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"

/** \brief
 *  存储了引擎中使用的路径数据。
 *
 *  使用这些路径数据，我们可以让特效按一定的路径前进，我们也可以让其它的元素按照这个
 *  关键帧路径前进。
 */
struct os_track
{
	//! 关键顶点数据。
	VEC_vector3     m_vec3Interpolate;

	//! 任意两个关键顶点之间的距离。
	VEC_float       m_vecDistance;

	//! 场景中关键点的数目。
	int            m_iPtNum;

	//! 当前的track对应的文件名。
	char           m_szFname[128];

public:
	os_track();

	/** \brief
	 *  从一个文件得到一个规迹动画的数据。
	 */
	bool           load_trackFromfile( const char* _fname );

};
typedef std::vector< os_track >  VEC_track;






/** \brief
 *  创建一个规迹动画的实例数据。
 *
 *  每一个动画的实例数据保存了当前动画。
 *  
 */
class osc_trackIns
{
private:
	//! 当前实例用到的track的指针。
	os_track*      m_ptrTrack;

	//! 当前动画到达的关键顶点的索引。
	int           m_iPtIdx;

	//! 当前规迹动画运行到现在总共的时间。
	float         m_fTtime;

	//! 两个关键顶点之间插值时用到的系数。
	float          m_fInterVal;

	//! 当前动画的运行速度。
	float          m_fMovSpeed;

	//! 当前规迹动画是一次性的还是循环播放的。
	BOOL           m_bLoopAni;  

	//! 当前的规迹实例数据是否在使用中。
	bool           m_bInuse;

public:

	osc_trackIns();

	//! 终止当前的规迹动画。
	void          disable_trackAni( void );

	//! 设置当前动画对应的track指针。
	void          set_trackPtr( os_track* _tptr );


	/** \brief
	 *  创建一个trackInstance.
	 * 
	 *  \param _loop 　　是否循环播放当前的实例数据。
	 *  \param _mspeed   当前动画中实体的移动速度。
	 *  \param _randS    是否随机化初始位置
	 */
	void          create_trackIns( BOOL _loop,float _mspeed,BOOL _randS );

	/** \brief
	 *  得到当前动画达到的坐标。
	 *
	 *  \param _etime      上一帧过去的时间。
	 *  \param _resVec     返回了动画到达的坐标。
	 *  \param return bool 如果返回了false,则完成了动画的过程，以后不能再使用这个
	 *                     规迹动画的实例id了。
	 */
	bool          get_aniVec( float _etime,osVec3D& _resVec );

	/** \brief
	 *  得到当前trackIns上的随机的关键点。
	 */
	void          get_trackRandKeyPos( osVec3D& _resVec );


	//! 当前的实例是否在使用中。
	bool          is_insInuse( void )             { return m_bInuse; }


};

//! track实例数据使用到vector.
typedef std::vector< osc_trackIns >   VEC_trackIns;






/** \brief
 *  对场景中的路径数据进行管理的管理器。
 *  
 *  
 */
class osc_trackMgr
{
private:

	//! 管理器中的规迹动画数据。
	VEC_track        m_vecTrack;

	//! 管理器中的track 实例类。
	VEC_trackIns     m_vecTrackIns;


public:
	osc_trackMgr();
	~osc_trackMgr(){release();}


	/** \brief 
	*	释放资源,在构造函数中vector<>::resize分配的内存，会受到MFC的检查，认为是内存泄漏
	*	事实上没有泄漏，但会引起调试信息混乱
	*/
	void         release(void)
	{
		m_vecTrackIns.clear();
		m_vecTrack.clear();
	}

	/** \brief
	 *  从管理器中创建一个Track.
	 *  
	 *  \return int 返回我们创建的track的id,使用这个id来操作我们得到的Track.
	 *  \param      _fname     要创建的trackIns的文件名。
	 *  \param      _movspeed  要创建的track实例中，使用此track实体的移动速度。
	 *  \param      _look      是否循环播放当前的track动画。
	 *  \param      _randS     是否随机初始化位置，用于场景内的使用相同的规道粒子，有不同的效果。
	 *  
	 */
	int          create_track( const char* _fname,float _movspeed,BOOL _loop,BOOL _randS = FALSE );

	/** \brief
	 *  从管理器中使一个Track失效。
	 */
	void         delete_track( int _trackId );


	/** \brief
	 *  从当前的Track中的关键顶点中，得到一个随机的顶点。
	 *
	 *  \param _trackId  要得到随机顶点的track instance的id
	 *  \param _resVec   返回的在当前Track中的随机的顶点。
	 */
	void         get_randomVec( int _trackId,osVec3D& _resVec );

	/** \brief
	 *  得到当前track的动画顶点.
	 *
	 *  \param _trackId   要操作的Track实例数据。
	 *  \param _etime     上一帧过去的时间。
	 *  \param _resVec    返回当前动画到达的顶点。
	 *  \return bool      如果是一次性的动画，如果完成了当前的动画过程，则返回false.
	 *                    否则永远返回真。
	 */
	bool         get_aniVec( int _trackId,float _etime,osVec3D& _resVec );

};


//! 全局的trackMgr.
extern     osc_trackMgr*    g_ptrTrackMgr;
















    