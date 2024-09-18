//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdWater.h
 *
 *  Desc:     ������ˮ��ص���Ч�Ĵ���
 *
 *  His:      River created @ 2004-5-13.
 *
 *  "��������˵��������ױ������Լ��ĵ�λ"��( �����ά�� )
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"
//

//! River @ 2005-9-22: �Ժ������ڵ�ˮ��osa�ļ���ʵ�֣�
//                     �������ļ���ʱ���á�

# if 0  //���ô��ļ���

/*
ˮ��ص���Ʒ�Ĵ���ˮ��ص���Ʒ�����Ͽ����ù������ʵ�֣�����������ܵĻ���
ʹ�ù������ص����ݽṹ��

ʹ�ù�������ƵĴ��룬ˮ������һ���򵥵ı༭��,����ϰд�༭����ء�
*/


#define WATERFILETITLE   "wat"

//!�ļ�ͷ�ṹ
struct WATER_FILE_HEAD
{
	//!�ļ�����
	char			m_szFileName[4];

	//!�ļ��汾��100
	DWORD			m_dwFileVersion;
};


/** \brief
 *  ˮ�涥���õ��Ķ����ʽ��
 */
struct os_verWater
{
	osVec3D    m_vecPos;
	DWORD      m_dwColor;
	osVec2D    m_vecUv;
};

typedef std::vector< os_verWater > VEC_verWater;


//! ������ʹ�õ�ˮ��������Ŀ��
# define MAX_WATERFRAME        60

/** \brief
 *  ˮ��ص��ࡣ
 *
 */
class osc_dwater
{
	friend class   osc_dwaterMgr;
private:

	//! ��ǰ��ˮ���Ӧ��ͼƬ�����֡�
	char      m_strWTextureName[128];

	//! ֡���л���Ҫ��ʱ�䡣
	float     m_fMoveSpeed;


	//! x�᷽����ӵ���Ŀ��
	int       m_iXSize;

	//! z�᷽����ӵ���Ŀ��
	int       m_iZSize;

	/** \brief
	 *  ��ǰˮ���ķ���1:x 2:-y 3: -x 4:y
	 *
	 *  ͨ������ÿһ��ˮ�����ӵ�uv�����������������
	 */
	int      m_iWaterDir;

	//! ��ǰˮ�����ĵ��λ�á�
	osVec3D  m_vec3CPos;

	//! ��ǰˮ�����ת����
	float    m_fRotAgl;

	//! ��ǰˮ�濪ʼ���λ��,����x,z��Сֵ��λ�á�
	osVec3D  m_vec3WStart;


	//! ��ǰˮ����������Ķ���֡����
	int      m_iFrameNum;

	//! ��ǰˮ���õ�����������id,ֻ��Ҫ�洢��һ���Ϳ����ˡ�
	int      m_vecDevdepShaderId[MAX_WATERFRAME];

	//! ��ǰˮ���͸���ȡ�
	float    m_fAlpha;

	/** \brief
	 *  ��Ⱦ��ǰ��ˮ��ʹ�õ�����
	 *
	 *  ÿһ��ˮ����ӵĳ��Ⱥ͵ر����һ����3M.
	 *  ��ǰˮ��ɼ���ʱ��ÿһ֡����ÿһ��polygon��shaderid.
	 *  
	 */
	VEC_polygon    m_arrPolygon;
	VEC_verWater   m_arrWVertex;
	VEC_int        m_vecShaderId;


	// ���й����е�ǰ֡��ȥ��ʱ�䡣
	float         m_fCurFrameEleTime;

	//! ����ˮ���ʱ��Ӧ�õõ���ǰˮ���bounding Square.


	//! ��ǰˮ���Ƿ���ʹ���С�
	bool          m_bInuse;

private:

	//! ��ˮ������ĵ�õ�ˮ�����ʼ��λ�á�
	void     get_waterStartPos( void );

	//! ��ʼ��ˮ���shader id.
	int      get_polygonSId( int _x,int _z );

	//! �������ǵ����ݳ�ʼ�����ǵ�polygon.
	void     init_rPolygon( osc_middlePipe* _pipe );

	//! ���ݷ���Ĳ�ͬ�õ������õ���uv���ꡣ
	void     get_uvFromWaterDir( osVec2D* _uv );

	//! ������ˮ��Ķ��㡣
	void     fill_waterTileVer( int _x,int _y,os_verWater* _ver );

	//! ÿһ֡����ˮ�Ķ�����ͼ�仯��
	void     frame_moveWater( float _etime );

	//! ��ini�ļ��ж���ˮ��������á�
	bool     read_waterIni( const char* _iname );

	//! ��ˮ���ļ��ж������ݡ�
	bool     read_waterFile( const char* _wname );

public:
	osc_dwater();
	~osc_dwater();

	//  ATTENTION TO OPP:���հ汾��ȥ�����������
	//! ���ļ��е��뵥����ˮ�ļ���
	bool     load_waterFromfile( const char* _fname,
		        osVec3D& _pos,float _rgl,osc_middlePipe* _pipe  );


	//! ʹ�ô���Ľṹ������ˮ�����ݡ�
	bool     create_water( os_waterCreate& _wc,
		               osVec3D& _pos,float _rgl,osc_middlePipe* _pipe );


	//! ��Ⱦ��ǰ��ˮ�档
	bool     render_water( I_camera*  _camptr,float _etime,osc_middlePipe* _pipe );


	//! �ͷŵ�ǰ��ˮ��Դ��
	void     release_water( void );

};
//! 
typedef std::vector< osc_dwater > VEC_water;


// 
//! ATTENTION TO OPP: ��һʱ�̣�������ֻ����һ���ɼ���ˮ�档
//! ��ǰ�������������е�ˮ����Ŀ��
# define  MAX_WATERINSCE   4

/** \brief
 *  �����������е�ˮ��ص���Ч��
 *
 *  ʹ��ˮ��Ч������������ˮ��Ч��ֻ�����ڳ���С��Χ��ˮ�棬���ڳ����д�Χ��ˮ�棬
 *  ���纣��ȵȣ�����ʹ��max����ͼ�Ƶ����ã���Ȼʹ�ô��ַ�ʽ�����˷���Ⱦ��Դ��
 * 
 */
class osc_dwaterMgr
{
private:
	//!  �м�ܵ���ָ�롣
	osc_middlePipe*    m_ptrMPipe;

	//!  �洢��ǰ������ˮ������ݡ�
	VEC_water         m_vecWater;

	//!  ����֡��Ⱦ����ˮ���id.
	int               m_iLastRenderId;


public:
	osc_dwaterMgr();
	~osc_dwaterMgr();

	//! ��ʼ�����ǵ�ˮ��Ч��������
	void        init_dwaterMgr( osc_middlePipe* _pipe );


# if __WATER_EDITOR__
	//! �Ӳ�������һ��ˮ�����ݡ�
	int         create_dwater( os_waterCreate& _wc,osVec3D& _pos,float _rgl = 0.0f );
# endif 


	//! ����һ��ˮ�����ݡ�
	int         create_dwater( const char* _fname,osVec3D& _pos,float _rgl );

	//! ��Ⱦ��ǰ��ˮ��Ч��������
	bool        render_dWater( I_camera*  _camptr );

	//! ��ˮ���������ɾ��һ��ˮ�档
	void        delete_dWater( int _id );

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ˮ��غ�����inline������
// 

# endif //���ô��ļ���




/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*\
|*                 �µ�water��         snowwin create @ 2006-1-3                   *|
\*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/// ��������bumpˮ��Ľṹ
struct os_water
{
	char		m_szShaderIni[128];
	char		m_szShaderName[64];

	float		m_fSpeed;
	int			m_iDir;

	int			m_iXSize;
	int			m_iZSize;

	float		m_fAlpha;
};


/// ˮ��ص��ࡣ
class osc_water
{
	friend class   osc_waterMgr;

	/// ˮ�涥���õ��Ķ����ʽ
	struct os_verWater
	{
		osVec3D    m_vecPos;
		DWORD      m_dwColor;
		osVec2D    m_vecUv;
	};

	typedef std::vector< os_verWater > VEC_verWater;

private:
	char      m_szShaderIni[128];
	char      m_szShaderName[64];


	//! x�᷽����ӵ���Ŀ��
	int       m_iXSize;

	//! z�᷽����ӵ���Ŀ��
	int       m_iZSize;

	/** \brief
	 *  ��ǰˮ���ķ���1:x 2:-y 3: -x 4:y
	 *
	 *  ͨ������ÿһ��ˮ�����ӵ�uv�����������������
	 */
	int      m_iWaterDir;

	//! ��ǰˮ�����ĵ��λ�á�
	osVec3D  m_vec3CPos;

	//! ��ǰˮ�濪ʼ���λ��,����x,z��Сֵ��λ�á�
	osVec3D  m_vec3WStart;

	//! ��ǰˮ���õ�����������id,ֻ��Ҫ�洢��һ���Ϳ����ˡ�
	int      m_iShaderId;

	//! ��ǰˮ���͸���ȡ�
	float    m_fAlpha;

	/** \brief
	 *  ��Ⱦ��ǰ��ˮ��ʹ�õ�����
	 *
	 *  ÿһ��ˮ����ӵĳ��Ⱥ͵ر����һ����3M.
	 */
	VEC_polygon    m_arrPolygon;
	os_polygonSet	m_cPolySet;
	VEC_verWater   m_arrWVertex;

	//! ��ǰˮ���Ƿ���ʹ���С�
	bool          m_bInuse;

private:

	//! ��ˮ������ĵ�õ�ˮ�����ʼ��λ�á�
	void     get_waterStartPos( void );


	//! �������ǵ����ݳ�ʼ�����ǵ�polygon.
	void     init_rPolygon( osc_middlePipe* _pipe );

	//! ���ݷ���Ĳ�ͬ�õ������õ���uv���ꡣ
	void     get_uvFromWaterDir( osVec2D* _uv );

	//! ������ˮ��Ķ��㡣
	void     fill_waterTileVer( int _x,int _y,os_verWater* _ver );


public:
	osc_water();
	~osc_water();


	//! ʹ�ô���Ľṹ������ˮ�����ݡ�
	bool     create_water( os_water& _wc, osVec3D& _pos, osc_middlePipe* _pipe );


	//! ��Ⱦ��ǰ��ˮ�档
	bool     render_water( I_camera*  _camptr, osc_middlePipe* _pipe );


	//! �ͷŵ�ǰ��ˮ��Դ��
	void     release_water( void );
};

typedef std::vector< osc_water > VEC_water;

//! ��ǰ�������������е�ˮ����Ŀ��
# define  MAX_WATERINSCE   4

/** \brief
 *  �����������е�ˮ��ص���Ч��
 */
class osc_waterMgr
{
private:
	//!  �м�ܵ���ָ�롣
	osc_middlePipe*    m_ptrMPipe;

	//!  �洢��ǰ������ˮ������ݡ�
	VEC_water         m_vecWater;

	//!  ����֡��Ⱦ����ˮ���id.
	int               m_iLastRenderId;

public:
	osc_waterMgr();
	~osc_waterMgr();

	//! ��ʼ�����ǵ�ˮ��Ч��������
	void        init_waterMgr( osc_middlePipe* _pipe );


	//! �Ӳ�������һ��ˮ�����ݡ�
	int         create_water( os_water& _wc,osVec3D& _pos,float _rgl = 0.0f );

	//! ��Ⱦ��ǰ��ˮ��Ч��������
	bool        render_water( I_camera*  _camptr );

	//! ��ˮ���������ɾ��һ��ˮ�档
	void        delete_water( int _id );
};
