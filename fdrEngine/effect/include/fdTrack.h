//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdTrack.h
 *
 *  Desc:     ���������е�·������,������ʾ�����е���Ч.
 *
 *  His:      River created @ 2004-2-18
 *
 *  "���Ƕ���ѹ�����磬��������ѹ������"
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

# include "../../interface/osInterface.h"

/** \brief
 *  �洢��������ʹ�õ�·�����ݡ�
 *
 *  ʹ����Щ·�����ݣ����ǿ�������Ч��һ����·��ǰ��������Ҳ������������Ԫ�ذ������
 *  �ؼ�֡·��ǰ����
 */
struct os_track
{
	//! �ؼ��������ݡ�
	VEC_vector3     m_vec3Interpolate;

	//! ���������ؼ�����֮��ľ��롣
	VEC_float       m_vecDistance;

	//! �����йؼ������Ŀ��
	int            m_iPtNum;

	//! ��ǰ��track��Ӧ���ļ�����
	char           m_szFname[128];

public:
	os_track();

	/** \brief
	 *  ��һ���ļ��õ�һ���漣���������ݡ�
	 */
	bool           load_trackFromfile( const char* _fname );

};
typedef std::vector< os_track >  VEC_track;






/** \brief
 *  ����һ���漣������ʵ�����ݡ�
 *
 *  ÿһ��������ʵ�����ݱ����˵�ǰ������
 *  
 */
class osc_trackIns
{
private:
	//! ��ǰʵ���õ���track��ָ�롣
	os_track*      m_ptrTrack;

	//! ��ǰ��������Ĺؼ������������
	int           m_iPtIdx;

	//! ��ǰ�漣�������е������ܹ���ʱ�䡣
	float         m_fTtime;

	//! �����ؼ�����֮���ֵʱ�õ���ϵ����
	float          m_fInterVal;

	//! ��ǰ�����������ٶȡ�
	float          m_fMovSpeed;

	//! ��ǰ�漣������һ���ԵĻ���ѭ�����ŵġ�
	BOOL           m_bLoopAni;  

	//! ��ǰ�Ĺ漣ʵ�������Ƿ���ʹ���С�
	bool           m_bInuse;

public:

	osc_trackIns();

	//! ��ֹ��ǰ�Ĺ漣������
	void          disable_trackAni( void );

	//! ���õ�ǰ������Ӧ��trackָ�롣
	void          set_trackPtr( os_track* _tptr );


	/** \brief
	 *  ����һ��trackInstance.
	 * 
	 *  \param _loop �����Ƿ�ѭ�����ŵ�ǰ��ʵ�����ݡ�
	 *  \param _mspeed   ��ǰ������ʵ����ƶ��ٶȡ�
	 *  \param _randS    �Ƿ��������ʼλ��
	 */
	void          create_trackIns( BOOL _loop,float _mspeed,BOOL _randS );

	/** \brief
	 *  �õ���ǰ�����ﵽ�����ꡣ
	 *
	 *  \param _etime      ��һ֡��ȥ��ʱ�䡣
	 *  \param _resVec     �����˶�����������ꡣ
	 *  \param return bool ���������false,������˶����Ĺ��̣��Ժ�����ʹ�����
	 *                     �漣������ʵ��id�ˡ�
	 */
	bool          get_aniVec( float _etime,osVec3D& _resVec );

	/** \brief
	 *  �õ���ǰtrackIns�ϵ�����Ĺؼ��㡣
	 */
	void          get_trackRandKeyPos( osVec3D& _resVec );


	//! ��ǰ��ʵ���Ƿ���ʹ���С�
	bool          is_insInuse( void )             { return m_bInuse; }


};

//! trackʵ������ʹ�õ�vector.
typedef std::vector< osc_trackIns >   VEC_trackIns;






/** \brief
 *  �Գ����е�·�����ݽ��й���Ĺ�������
 *  
 *  
 */
class osc_trackMgr
{
private:

	//! �������еĹ漣�������ݡ�
	VEC_track        m_vecTrack;

	//! �������е�track ʵ���ࡣ
	VEC_trackIns     m_vecTrackIns;


public:
	osc_trackMgr();
	~osc_trackMgr(){release();}


	/** \brief 
	*	�ͷ���Դ,�ڹ��캯����vector<>::resize������ڴ棬���ܵ�MFC�ļ�飬��Ϊ���ڴ�й©
	*	��ʵ��û��й©���������������Ϣ����
	*/
	void         release(void)
	{
		m_vecTrackIns.clear();
		m_vecTrack.clear();
	}

	/** \brief
	 *  �ӹ������д���һ��Track.
	 *  
	 *  \return int �������Ǵ�����track��id,ʹ�����id���������ǵõ���Track.
	 *  \param      _fname     Ҫ������trackIns���ļ�����
	 *  \param      _movspeed  Ҫ������trackʵ���У�ʹ�ô�trackʵ����ƶ��ٶȡ�
	 *  \param      _look      �Ƿ�ѭ�����ŵ�ǰ��track������
	 *  \param      _randS     �Ƿ������ʼ��λ�ã����ڳ����ڵ�ʹ����ͬ�Ĺ�����ӣ��в�ͬ��Ч����
	 *  
	 */
	int          create_track( const char* _fname,float _movspeed,BOOL _loop,BOOL _randS = FALSE );

	/** \brief
	 *  �ӹ�������ʹһ��TrackʧЧ��
	 */
	void         delete_track( int _trackId );


	/** \brief
	 *  �ӵ�ǰ��Track�еĹؼ������У��õ�һ������Ķ��㡣
	 *
	 *  \param _trackId  Ҫ�õ���������track instance��id
	 *  \param _resVec   ���ص��ڵ�ǰTrack�е�����Ķ��㡣
	 */
	void         get_randomVec( int _trackId,osVec3D& _resVec );

	/** \brief
	 *  �õ���ǰtrack�Ķ�������.
	 *
	 *  \param _trackId   Ҫ������Trackʵ�����ݡ�
	 *  \param _etime     ��һ֡��ȥ��ʱ�䡣
	 *  \param _resVec    ���ص�ǰ��������Ķ��㡣
	 *  \return bool      �����һ���ԵĶ������������˵�ǰ�Ķ������̣��򷵻�false.
	 *                    ������Զ�����档
	 */
	bool         get_aniVec( int _trackId,float _etime,osVec3D& _resVec );

};


//! ȫ�ֵ�trackMgr.
extern     osc_trackMgr*    g_ptrTrackMgr;
















    