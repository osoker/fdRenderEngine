//=======================================================================================================
/**  \file
 *   Filename: miskFunc.h
 *   Desc:     Osok�����г��õ��������ÿһ��ͷ�ļ���Ӧ�ð�������ļ���
 *
 *   His:      River created @ 5/15 2003.
 *
 *   "���µ������кܶ�������ǿ��Է���,�������ǲ�û��,���Ǿ�������ֱǰ,
 *    ��Ϊ���Ǳ���һ������."
 * 
 */
//=======================================================================================================
# pragma    once
#include "osInterface.h"

/** \brief
 *  һ�����������һ�����λ��.
 *
 *  ����bsp�ͳ��õļ������ݲ���.
 */
enum ose_geopos 
{
	OSE_FRONT = 0,
	OSE_ONPLANE = 1,
	OSE_BACK = 2,
	
	OSE_INTERSECT = 3,
};

// 
// ATTENTION: ���õĽṹ����ʹ���麯�����麯����ʹ�ṹ����һ��V-table�Ľṹ��
//            ����Խṹʹ��memeset,���ƻ�V-table,���ó���.��ʹ���麯��������
//            ֱ�ӶԽṹʹ��memset,Ҳ����ֱ�ӶԽṹ���и�ֵ��

/** \brief
 *  Bounding sphere,����sphere��ײ������.����bsp��ײ���.
 *
 */
class OSENGINE_API os_bsphere
{
public:
	osVec3D     veccen;
	float       radius;
public:
	//! �����Ƿ��ڵ�ǰsphere���ڲ���
	bool        pt_inSphere( osVec3D& _pos );

	//! ��aabbox�Ƿ��ཻ
	bool        collision( const os_aabbox& _box );

	/** \brief
	 *  �������ε���ײ
	 *
	 *  \param osVec3D* _vec1,2,3 ���������ε���������
	 *  \param osPlane* _plane    ���ǵ���,��ʡ�ڲ���������,����ⲿû�м���,��Ĭ��Ϊ��,�ڲ�����
	 */
	bool        collision( const osVec3D* _vec1,const osVec3D* _vec2,
		            const osVec3D* _vec3,const osPlane* _plane = NULL );

	//! ���ݴ���Ķ�������չbounding sphere.
	void        extendedBs( osVec3D& _newpt );

};
#define epsilon 0.00001f
static inline bool equal(float a, float b)
{
	const float d = a - b;
	if (d<epsilon && d>-epsilon) return true;
	else return false;
}

//! �����AABBox
class OSENGINE_API os_aabbox
{
public:
	osVec3D    m_vecMax;
	osVec3D    m_vecMin;
public:
	//! ���ཻ����true.
	bool       collision( const os_bsphere& _sphere );
	bool       collision( const os_aabbox& _box )
	{
		if( (m_vecMax.x<_box.m_vecMin.x)||
			(m_vecMax.y<_box.m_vecMin.y)||
			(m_vecMax.z<_box.m_vecMin.z)  )
			return false;
		if( (m_vecMin.x>_box.m_vecMax.x)||
			(m_vecMin.y>_box.m_vecMax.y)||
			(m_vecMin.z>_box.m_vecMax.z)  )
			return false;
		return true;
	}
	/** \brief
	*  �����Ƿ�͵�ǰ��Bounding Box�ཻ��
	*
	*  \param _res ������ߺ�BoundingBox�ཻ�����ֵΪ���صĽ���ֵ��
	*  \param _dis ����ཻ����ֵ���ؿ�ʼ��ͽ���֮��ľ��롣
	*  
	*/
	bool ray_intersect( osVec3D& _start,osVec3D& _dir,float& _dis );
	
	/** \brief
	 *  �߶��Ƿ��bounding Box�ཻ.
	 */
	bool ls_intersect( osVec3D& _ls,osVec3D& _le );


	//! �����Ƿ���aabbox��
	bool              pt_inBox( const osVec3D& _pt )
	{
		if( (_pt.x<=m_vecMax.x)&&(_pt.x>=m_vecMin.x)&&
			(_pt.y<=m_vecMax.y)&&(_pt.y>=m_vecMin.y)&&
			(_pt.z<=m_vecMax.z)&&(_pt.z>=m_vecMin.z) )
			return true;
		else
			return false;
	}

	void       extend( const osVec3D& _pt )
	{
		if( _pt.x > m_vecMax.x )
			m_vecMax.x = _pt.x;
		if( _pt.y > m_vecMax.y )
			m_vecMax.y = _pt.y;
		if( _pt.z > m_vecMax.z )
			m_vecMax.z = _pt.z;

		if( _pt.x < m_vecMin.x )
			m_vecMin.x = _pt.x;
		if( _pt.y < m_vecMin.y )
			m_vecMin.y = _pt.y;
		if( _pt.z < m_vecMin.z )
			m_vecMin.z = _pt.z;
	}
	osVec3D     get_center( void ) const
	{ 
		return (m_vecMax+m_vecMin)/2.0f; 
	}
	//! �õ�8������
	void getEdges(osVec3D *edges) const
	{
		osVec3D  middle = get_center();
		osVec3D  diag = middle - m_vecMax;

		/*
		  /5--------/7
		 /  |      / |
		/   |     /  |
		1---------3  |
		|   4- - -| -6
		|  /      |  /
		|/        | /
		0---------2/ 
		*/

		edges[0].x = (middle.x + diag.x);
		edges[0].y = (middle.y + diag.y);
		edges[0].z = (middle.z + diag.z);

		edges[1].x = (middle.x + diag.x);
		edges[1].y = (middle.y - diag.y);
		edges[1].z = (middle.z + diag.z);

		edges[2].x = (middle.x - diag.x);
		edges[2].y = (middle.y + diag.y);
		edges[2].z = (middle.z + diag.z);

		edges[3].x = (middle.x - diag.x);
		edges[3].y = (middle.y - diag.y);
		edges[3].z = (middle.z + diag.z);

		edges[4].x = (middle.x + diag.x);
		edges[4].y = (middle.y + diag.y);
		edges[4].z = (middle.z - diag.z);

		edges[5].x = (middle.x + diag.x);
		edges[5].y = (middle.y - diag.y);
		edges[5].z = (middle.z - diag.z);

		edges[6].x = (middle.x - diag.x);
		edges[6].y = (middle.y + diag.y);
		edges[6].z = (middle.z - diag.z);

		edges[7].x = (middle.x - diag.x);
		edges[7].y = (middle.y - diag.y);
		edges[7].z = (middle.z - diag.z);
	}

	void	recalc_aabbox()
	{
		osVec3D vecMax = osVec3D( float(SG_MIN_VALUE),float(SG_MIN_VALUE),float(SG_MIN_VALUE) );
		osVec3D vecMin = osVec3D( float(SG_MAX_VALUE),float(SG_MAX_VALUE),float(SG_MAX_VALUE) );


		osVec3D edges[8];
		getEdges(edges);

		for( int t_i = 0;t_i<8;t_i ++ )
		{
			// max vec.
			if( vecMax.x < edges[t_i].x )
				vecMax.x = edges[t_i].x;
			if( vecMax.y < edges[t_i].y )
				vecMax.y = edges[t_i].y;
			if( vecMax.z < edges[t_i].z )
				vecMax.z = edges[t_i].z;

			// min vec.
			if( vecMin.x > edges[t_i].x )
				vecMin.x = edges[t_i].x;
			if( vecMin.y > edges[t_i].y )
				vecMin.y = edges[t_i].y;
			if( vecMin.z > edges[t_i].z )
				vecMin.z = edges[t_i].z;
		}
		m_vecMax = vecMax;
		m_vecMin = vecMin;
	}

	//! ��һ������ת����ǰ��min��max,���õ�һ���µ�aabbox.
	void    get_transNewAABBox( osMatrix& _inMat,os_aabbox& _res );

};

/** \brief 
 *  Oriented Bounding box,����Box����ײ���.����bsp����ײ���.
 * 
 *  BBox�����ս������������õ���
 *  vecmax��vecmin��ԭ��Ϊ���ģ���Y����תm_fBoxRot���ȣ�Ȼ��ƽ�Ƶ�m_vecBoxPosλ��
 */
class OSENGINE_API os_bbox
{
private:
	osVec3D     vecmax;
	osVec3D     vecmin;

	//! ��ǰbounding box������ռ��е�λ�á�
	osVec3D     m_vec3BoxPos;

	//! ��ǰBounding Box����ת�Ƕ�,��Y����ת
	float       m_fBoxRot;

	//! bbox��λ����ת����Ϣ�Ƿ�ı�
	BOOL        m_bChanged;

	//! ת��������ռ��bbox�������.
	osVec3D     m_vecWorldVer[8];


	//! ����8��ת����Ķ���
	void       to_worldVertex( void );

public:
	os_bbox();

	//! �����������С��
	void             reset_bbox( void ); 
	void             set_bbMaxVec( const osVec3D& _max ){ vecmax = _max;m_bChanged = TRUE; }
	void             set_bbMinVec( const osVec3D& _min ){ vecmin = _min;m_bChanged = TRUE; }
	void             set_bbPos( const osVec3D& _pos ){ m_vec3BoxPos = _pos;m_bChanged = TRUE; }
	void             set_bbYRot( float _rot ){ m_fBoxRot = _rot;m_bChanged = TRUE; } 
	const osVec3D*    get_vecmax( void ) const { return &vecmax; } 
	const osVec3D*    get_vecmin( void ) const { return &vecmin; }
	const osVec3D*    get_bbPos( void ) const  { return &m_vec3BoxPos; } 
	float            get_bbYRot( void )const  { return m_fBoxRot; } 
	void             scale_bb( const osVec3D& _scale )
	{
		vecmax.x *= _scale.x;
		vecmax.y *= _scale.y;
		vecmax.z *= _scale.z;

		vecmin.x *= _scale.x;
		vecmin.y *= _scale.y;
		vecmin.z *= _scale.z;

		m_bChanged = TRUE;
	}

	//! ת��Ϊ��ά�İ�Χ��
	void              to_vec2Box( osVec2D* _vec );


	//! ��aabbox����oriented bbox.
	os_bbox& operator = ( const os_aabbox& _aabbox )
	{
		vecmax = _aabbox.m_vecMax;
		vecmin = _aabbox.m_vecMin;
		m_bChanged = TRUE;

		return *this;
	}
	os_bbox& operator = ( const os_bbox& _box )
	{
		vecmax = _box.vecmax;
		vecmin = _box.vecmin;
		m_vec3BoxPos = _box.m_vec3BoxPos;
		m_fBoxRot = _box.m_fBoxRot;
		m_bChanged = _box.m_bChanged;
		memcpy( m_vecWorldVer,_box.m_vecWorldVer,sizeof( osVec3D )*8 );

		return *this;
	}



	/** \brief
	*  �߶��Ƿ�͵�ǰ��Bounding Box�ཻ��
	*
	*  \param _res ������ߺ�BoundingBox�ཻ�����ֵΪ���صĽ���ֵ��
	*  \param _dis ����ཻ����ֵ���ؿ�ʼ��ͽ���֮��ľ��롣
	*  
	*/
	bool      ray_intersect( osVec3D& _start,osVec3D& _dir,float& _dis );	

	/** \brief
	 *  �ж�һ����radius�������Ƿ���Bounding box�ཻ��
	 *  
	 *  \param osVec3D _start,_end ���ߵĿ�ʼ��ͽ����㡣
	 *  \param float _radius ���ߵİ뾶
	 */
	bool      ray_intersectWithRadius( osVec3D& _start,osVec3D& _end,float _radius );


	//! ��ǰ���Ƿ���BoundingBox���ڲ���
	bool      pt_inBBox( osVec3D& _pos );
	//! ���һ�鶥�㣬���Ƿ��ж�����bbox��.
	bool      has_ptInBBox( osVec3D* _pt,int _num );

	void      to_bsphere( os_bsphere& _sphere );
	void      to_aabbox( os_aabbox& _aabbox );

	// �ཻ����
	bool      collision( const os_aabbox& _aabbox );
	bool      collision( const os_bbox& _bbox );
	bool      collision( const os_bsphere& _sphere );

	//! �õ����ӵ�������,��ķ���������.
	void      get_boxPlane( osPlane _plane[6] );

	//! ��ǰbbox�ڵĶ����Ƿ�ȫ������aabbox��
	bool      all_inAABBox( const os_aabbox& _aabbox );


	//! �õ���boxת��������ռ���8������.
	const osVec3D*  get_worldVertexPtr( void ) 
	{ 
		if( m_bChanged ) 
			to_worldVertex();
		return m_vecWorldVer; 
	}

};

/** ��ʾ3D�ռ��һ������,�����ͷ������ */
class OSENGINE_API os_Ray
{
public:
	osVec3D mOrigin;
	osVec3D mDirection;
public:
	//	Ray():mOrigin(CVector::ZERO), mDirection(CVector::UNIT_Z) {}
	os_Ray(const osVec3D& origin, const osVec3D& direction)
		:mOrigin(origin), mDirection(direction) {}
		virtual ~os_Ray() {}

		/** �������ߵ���� */
		void setOrigin(const osVec3D& origin) {mOrigin = origin;} 
		/** �õ����ߵ���� */
		const osVec3D& getOrigin(void) const {return mOrigin;} 

		/** �������ߵķ��� */
		void setDirection(const osVec3D& dir) {mDirection = dir;} 
		/** �õ����ߵķ��� */
		const osVec3D& getDirection(void) const {return mDirection;} 

		/** ���������߷��� t ���ȵĵ����� */
		osVec3D getPoint(float t) const { 
			return osVec3D(mOrigin + (mDirection * t));
		}

		/** ���������߷��� t ���ȵĵ����� */
		osVec3D operator*(float t) const { 
			return getPoint(t);
		}

		/** ���������Ƿ��������ƽ���ཻ
		@returns ����ཻ����TRUE ,���ཻ����FALSE
		*/
		//bool intersects(const Plane & p) const
		//{
		//	return true;
		//	//			return Math::intersects(*this, p);
		//}

		/** ���������Ƿ�����������������ཻ
		@returns ����ཻ����TRUE ,���ཻ����FALSE
		*/
		bool intersects( os_bbox & box) 
		{
			float dist;
			return box.ray_intersect(mOrigin,mDirection,dist);
			//return osn_mathFunc::Intersects(*this, box);
		}
		bool intersects( os_aabbox & box) 
		{
			float dist;
			return box.ray_intersect(mOrigin,mDirection,dist);
			//return osn_mathFunc::Intersects(*this, box);
		}
};

class OSENGINE_API Plane 
{
public:
	osVec3D m_Normal;
	float	m_Distance;

	Plane():m_Normal(0.0f,0.0f,0.0f){
		m_Distance = 0;
	}
	Plane(const osVec3D &v0,const osVec3D &v1,const osVec3D &v2){
		osVec3D edg0 = v0-v1;
		osVec3D edg1 = v0-v2;

		osVec3Cross(&m_Normal,&edg0,&edg1);
		//	m_Normal = edg0.cross(edg1);
		osVec3Normalize(&m_Normal,&m_Normal);
		//m_Normal.normalize();

		m_Distance = - ((m_Normal.x * v0.x) + (m_Normal.y * v0.y) + (m_Normal.z * v0.z));
	}
	float getDistance (const osVec3D &rkPoint) const{ 
		return m_Normal.x*rkPoint.x + m_Normal.y*rkPoint.y + m_Normal.z*rkPoint.z + m_Distance ;
	}
	~Plane(){}
	void normalize()
	{
		float oneOverLen = 1.0f /osVec3Length(&m_Normal);// m_Normal.magnitude();

		m_Normal.x *= oneOverLen;
		m_Normal.y *= oneOverLen;
		m_Normal.z *= oneOverLen;
		m_Distance *= oneOverLen;
	}
};
class os_Ray;

namespace osn_mathFunc
{

	/** \brief
	 *  ������������еĴ�д��ĸ��ΪСд��ĸ.
	 */
	OSENGINE_API void convert_lowercase( char* _buf );


	/** \brief 
	 *  �õ�dx�������ĺ�����dx�����ķ�װ��
	 */
	OSENGINE_API inline const char* get_errorStr( HRESULT _hr )
	{
		return DXGetErrorDescription9( _hr );
	}

	/** \brief
	 *  �õ����㵽һ����ľ���.
	 */
	OSENGINE_API inline float distance_V2P( const osVec3D& _v,const osPlane& _p )
	{
		return osVec3Dot( &_v,(osVec3D*)&_p ) + _p.d ;
	}


	/** \brief
	*  ���������ӽ��ж���.��_b2�����ƶ���_np2��λ���ϣ���_b1���롣
	*
	*  _snap1: ����ת���Ͼ������������롣
	*  _snap2: ���ĵ���뵽��ת���ϣ�
	*  _snap3: ����Ǹ߶ȶ���,�ڸ߶��ϰ��������ӵĵ��߱��ֵ�һ��λ���ϡ�
	*
	*  �㷨������
	*  1: ���������bboxҪ��������ߡ�
	*  2: ����ڶ������������������ϵĿ��Ը���һ���ӽӴ����������ȡ�����_snap1
	*  3: �ѵڶ������ӵ����ĵ��ƶ�������bboxҪ����������ϣ�����_snap2
	*  4: �ƶ��ڶ������ӵ����ĵ㣬�������ĺ��ӵ�Y������Сֵ�ŵ���ͬ��Y����ˮƽ���ϡ�
	*   
	*/
	OSENGINE_API bool snap_bbox( os_bbox& _b1,
		 os_bbox& _b2,osVec3D& _np2,bool _snap1,bool _snap2,bool _snap3 );

	/** \brief
	 *  �õ�һ����������������������ɵ�ֱ�ߵľ��롣
	 */
	OSENGINE_API inline float distance_V2L( const osVec3D& _v,const osVec3D& _s,const osVec3D& _e )
	{
		osVec3D  t_res,t_vec3Tmp1,t_vec3Tmp2;
		t_vec3Tmp1 = _v-_s;
		t_vec3Tmp2 = _e-_s;
		osVec3Cross( &t_res,&t_vec3Tmp1,&t_vec3Tmp2 );
		t_res /= osVec3Length( &t_vec3Tmp2 );
		return osVec3Length( &t_res );
	}


	/** \brief
	 *  �Ѷ��������еĶ��㰴˳ʱ�뷽���������.
	 */
	OSENGINE_API bool sort_Vertex2Cw( osVec3D* _pvert,int _num ,osPlane& _plane );


	/** \brief
	 *  �õ������ǲ�����ͬһ��ֱ����.
	 */
	OSENGINE_API bool get_coline( osVec3D* _vec,int _vecnum );


	/** \brief
	 *  �õ�һ��bounding box��max extents.
	 */
	OSENGINE_API void get_bboxExtents( osVec3D& _res,osVec3D& _min,osVec3D& _max );

	/** \brief
	 *  �õ�һ��Bounding box��һ���������ײʱ��Ҫ��ƫ�ƾ���.
	 */
	OSENGINE_API float distance_bboxOffset( osVec3D& _extents,osPlane& _plane );

	/** \brief
	 *  �Ӷ��Bsphere�еõ�һ��Bsphere.
	 */
	OSENGINE_API void get_bsFromBsArray( os_bsphere& _res,os_bsphere* _barr,int _num );

	/** \brief
	 *  ���һ�������Ƿ��һ���������ཻ��
	 *
	 *  \param _start,_dir ��ʼ������������ķ���������
	 *  \param _v0,_v1,_v2 �����ε��������㡣
	 *  \param _t  �����������ؿ�ʼ��ͽ����ľ��롣
	 *  \param _u,_v       ���ؽ������������ϵ�uv������
	 */
	OSENGINE_API bool intersect_tri( osVec3D& _orig,osVec3D& _dir,
		const osVec3D& _v0,const osVec3D& _v1,
		const osVec3D& _v2,float& _t,float& _u,float& _v );



	/** \brief
	 *  ��һ�������Ƿ���һ��������ཻ������ཻ��������㡣
	 *  polydata�����������߶������ݵ�˳ʱ������.
	 */
	OSENGINE_API bool intersect_Ray2Poly( const osVec3D& _ori, const osVec3D& _end,
		                               int _vernum,const osVec3D* _polydata,osVec3D& _result );

	/** \brief
	 *  ��һ���������Ƿ��һ��������ཻ,���߼�����bounding sphere.
	 */
	OSENGINE_API bool intersect_Ray2Poly( const osVec3D& _ori,const osVec3D& _end,float _radius,
		                               int _vernum,const osVec3D* _polydata,osVec3D& _result );
		                         


	/** \brief
	 *  �жϵ�һ���߶ε��������Ƿ��ڵڶ����߶ε����ࡣ
	 */
	OSENGINE_API bool intersect_seg2d( osVec2D& _s1,osVec2D& _e1,osVec2D& _s2,osVec2D& _e2 );

	/** \brief
	 * �ж�������ά���߶��Ƿ��ཻ��
	 */


	/** \brief
	 *  �鿴һ�����������һ�����λ��.
	 *
	 */
	OSENGINE_API inline ose_geopos  classify_PointByPlane( const osPlane& _p,const osVec3D& _vec )
	{
		float    tmpf;

		// ATTENTION TO FIX:ʹ��EPSILON�������ʹ��EPSILONҲ�����????
		tmpf = distance_V2P( _vec,_p );
		if( tmpf > 0.0f )    //EPSILON )
			return OSE_FRONT;
		else if( tmpf< 0.0f )//-EPSILON) 
			return OSE_BACK;
		else
			return OSE_ONPLANE;
	}


	/** \brief
	 *  �õ�һ���߶κ�һ��ƽ���ཻ�Ľ��㡣
     */
	OSENGINE_API bool get_LsIntersectPlane( osVec3D& _result,osVec3D& _start,osVec3D& _end,
		                               const osPlane& _plane,float& _percent );



	/** \brief
	 *  �鿴һ�������������һ�����λ��.
	 *
	 *  ��bsp���д����õ��������.
	 */
	OSENGINE_API ose_geopos classify_GeoByPlane( const osPlane& _p,const osVec3D* _vec,int _vecnum );

	/** \brief
	 *  ��һ�������������зָ�ĺ���.����bsp���ı���.Ҳ��������decal�ļ���.
	 *
	 *  \param osVec3D* _polyvec ����Ķ������˳ʱ�����С����Ҵ��붥����ɵĶ��
	 *                           �α�����͹����Σ���һ��ƽ���ϡ�
	 *  
	 *  �㷨������
	 *  1: 
	 */
	OSENGINE_API void split_PolyByPlane( const osPlane& _plane,
		const osVec3D* _polyvec,int& _vecnum,osVec3D* _resvec,int& _resvecnum );


	/** \brief
	 *  �õ�����ͶӰ��һ�����ϵõ��Ķ���.
	 *
	 *  �㷨����:
	 *
	 *  �ҳ���ǰ�������֮��ľ���,ʹ��ǰ��ķ����������������,
	 *  �Ե�ǰ�Ķ�����ϳ˺������,�͵õ�������������ͶӰ��.
	 *
	 *  ʹ�����������Ҳ���Եõ�һ��������һ�����ͶӰ������
	 *  
	 */
	OSENGINE_API void project_PtToPlane( osPlane& _plane,osVec3D& _srcpt,osVec3D& _respt );

	/** \brief
	 *  �õ�һ��������һ��ֱ���ϵ�ͶӰ����
	 *
	 *  \param osVec3D& _v ҪͶӰ�Ķ���
	 *  \param osVec3D& _res ���ص�ͶӰ���㡣
	 *  \param osVec3D& _s   ֱ�ߵĿ�ʼ�㶥�㡣
	 *  \param osVec3D& _e   ֱ�ߵĽ��ٶ��㡣
	 *  
	 *  �㷨������
	 *  ���ҳ���_v���㡰ƽ�С��İ���_s��_e���档Ȼ��ͶӰ_v���㵽������ϣ���Ϊ����Ҫ�ҵ��Ķ��㡣
	 */
	OSENGINE_API void project_PtToLine( osVec3D& _res,osVec3D& _v,osVec3D& _s,osVec3D& _e );


	/** \brief
	 *  �õ�����ƽ��Ľ��㡣
	 *
	 *  �������������ƽ��ƽ�еĻ������ش���
	 *  Ŀǰ��ʹ�����������
	 *  
	 */
	OSENGINE_API bool plane_intersect( osVec3D& _ipt,osPlane& _a,osPlane& _b,osPlane& _c );


	OSENGINE_API inline float sgn( float _a )
	{
		if (_a > 0.0F) return (1.0F);
		if (_a < 0.0F) return (-1.0F);
		return (0.0F );
	}

	//======================================================
	// from Eric's oblique view frustum optimal code
	//======================================================
	OSENGINE_API void  clipProjectionMatrix(const osPlane& _clipPlane, osMatrix& _matProj );


	/** \brief
	 *  �õ���ǰmesh��Bounding Box.
	 */
	OSENGINE_API bool get_meshBBox( LPD3DXMESH _mesh,osVec3D& _max,osVec3D& _min );

	/** \brief
	 *  ���ĵ�ǰstring�ڵ����������ĸ,���ڸ����ļ���չ��.
	 */
	OSENGINE_API bool change_extName( s_string& _str,const char* _newExt );



	/** \brief
	 *  �õ����������ļнǣ����ص��ǻ��ȡ�
	 *
	 *  ���贫��������������Ƿ�������
	 */
	OSENGINE_API float get_vectorAngle( osVec3D& _normalVec1,osVec3D& _normalVec2 );

	//! �õ�������ά����֮��ļн�.
	OSENGINE_API float get_vectorAngle( osVec2D& _normalVec1,osVec2D& _normalVec2 );


	/** \brief
	 *  ת��3dsMax�����������Ϊ��������ϵ.������ȫ����������ת�������ꡣ
	 *
	 *  ��d3d��������ת����3dsMax����������Ҳ����ʹ�����������
	 */
	OSENGINE_API void convert_coorRhToLh( osMatrix& _res,osMatrix& _src );

	//! ��һ����vertex�У��������ǰvertex��aabbox.
	OSENGINE_API void cal_aabboxFromVArray( osVec3D* _vec,int _num,os_aabbox& _box );

	/** \brief
	 *   ����������ֵ���в�ֵ���㡣
	 *
	 *   \return float      ���صĽ����
	 *   \param  _src,_dst  ����Ҫ��ֵ�ĸ�������
	 *   \param  _lv        ��ֵϵ����
	 */
	OSENGINE_API inline float   float_lerp( float _src,float _dst,float _lv )
	{
		return _lv*(_dst-_src)+_src;
	}

	
	//! ���DWORDֵ�ĵ�λWORD�Ƿ���ȡ�
	OSENGINE_API inline bool    low_equal( DWORD _a,WORD _b )
	{
		if( WORD(_a) == _b )
			return true;
		else
			return false;
	}

	//! ʹ��һ��DWORD�ĸ�λWORD��һ��DWROD�Ĵ�λword����һ��dword.
	OSENGINE_API inline DWORD   syn_dword( DWORD _hi,DWORD _low )
	{
		DWORD   t_dw = (_hi&0x0000ffff)<<16;
		
		return (t_dw | (WORD)_low);
	}

	//! �õ�һ��dwordֵ�ĸ�λֵ��
	OSENGINE_API inline int    get_hiword( DWORD _a )
	{
		return (_a&0xffff0000)>>16;
	}


	/** \brief
	 *  ��ʹ��һ����ֵϵ��������������в�ֵ��
	 *  
	 *  \param _resmat  �������.
	 *  \param _src     Դ����
	 *  \param _dst     Ŀ�����.
	 *  \param _lv      ��ֵϵ����
	 *  
	 */
	OSENGINE_API void matrix_lerp( osMatrix& _resmat,osMatrix& _src,osMatrix& _dst,float _lv );


	/** \brief
	 *  ȥ���ļ����ж���ķָ�����Ϣ
	 *
	 *  \param _isz �������������Ҳ����������
	 *  
	 */
	OSENGINE_API void delete_surplusSymbol( char* _isz );




	OSENGINE_API inline osQuat* WINAPI osQuaternionUnitAxisToUnitAxis2( osQuat *_pOut, 
		const osVec3D *_pvFrom, const osVec3D *_pvTo)
	{
		osVec3D vAxis;
		osVec3Cross(&vAxis, _pvFrom, _pvTo);    // proportional to sin(theta)
		_pOut->x = vAxis.x;
		_pOut->y = vAxis.y;
		_pOut->z = vAxis.z;
		_pOut->w = osVec3Dot( _pvFrom, _pvTo );
		return _pOut;
	}

	/** \brief
	 *  ������������,����������������֮���quaternion. 
	 * 
	 *  Ŀǰ��Ҫ���ڵ���ļ���.
	 */
    OSENGINE_API inline osQuat* WINAPI osQuaternionAxisToAxis( osQuat *_pOut, 
													 const osVec3D *_pvFrom, const osVec3D *_pvTo)
   {
	   osVec3D vA, vB;
	   osVec3Normalize(&vA, _pvFrom);
	   osVec3Normalize(&vB, _pvTo);
	   osVec3D vHalf(vA + vB);
	   osVec3Normalize(&vHalf, &vHalf);
	   
	   return osQuaternionUnitAxisToUnitAxis2(_pOut, &vA, &vHalf);
   }


   /** \brief
    *   ��һ�����̵İ�������
	*/
   OSENGINE_API bool osCreateProcess( const char* _pname );
   //@{
   // Windy mod @ 2005-11-7 16:12:36
   //Desc: ��ֵģ�溯��
   template<class T>
	   inline T interpolate(const float r, const T &v1, const T &v2)
   {
	   return static_cast<T>(v1*(1.0f - r) + v2*r);
   }
   
   //!Hermite��ֵ����֪������ɢ���λ�ú͵���ֵ�������λ�õ㣮
   template<class T>
	   inline T interpolateHermite(const float r, const T &v1, const T &v2, const T &in, const T &out)
   {
	   float h1 = 2.0f*r*r*r - 3.0f*r*r + 1.0f;
	   float h2 = -2.0f*r*r*r + 3.0f*r*r;
	   float h3 = r*r*r - 2.0f*r*r + r;
	   float h4 = r*r*r - r*r;
	   
	   return static_cast<T>(v1*h1 + v2*h2 + in*h3 + out*h4);
   }
   //@}
   //@{
   // Windy mod @ 2005-11-21 15:17:15
   //Desc: LOD ���ڼ������ģ�����̫��Ҫ�Ľ���
   inline const float Vec3Distance(osVec3D &a,osVec3D &b){
	   return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z)); 
   }
  
   //@}
   //@{
   // Windy mod @ 2006-1-14 9:54:16
   //Desc: 
   /** ���� / ���� �ཻ����, ����BOOL��� TRUEΪ�ཻ. */
   OSENGINE_API inline bool		Intersects(const os_Ray& ray, const os_bbox& box);
   /** ���� / ����Ƭ�ཻ����,����TURE �ཻ,������*/
  
   OSENGINE_API inline osVec3D	IntersectionPoint(const os_Ray &ray, const Plane& plane)
   {
	   if (!equal(osVec3Length(&ray.getDirection()),1.0f))
	   {
		   assert(!"�������û�й�һ��");
	   }

	   float Numerator = 0.0, Denominator = 0.0, dist = 0.0;
	   Numerator = - (osVec3Dot(&plane.m_Normal,&ray.getOrigin())+plane.m_Distance);// Use the plane equation with the normal and the line

	   Denominator = osVec3Dot(&plane.m_Normal,&ray.getDirection());//vNormal, vLineDir);		// Get the dot product of the line's vector and the normal of the plane
	   if( equal(Denominator ,0.0f))						// Check so we don't divide by zero
		   return ray.getOrigin();

	   dist = Numerator / Denominator;				// Divide to get the multiplying (percentage) factor
	   osVec3D vPoint;
	   vPoint = ray.getOrigin() + ray.getDirection() * dist;

	   return vPoint;
	
   }
   OSENGINE_API inline double	AngleBetweenVectors(const osVec3D &V1, const osVec3D &V2)
   {
	   //!�����������ĵ��
	   double dotProduct = osVec3Dot(&V1,&V2);
	   //!���������������ȵĻ�
	   double vectorsMagnitude = osVec3Length(&V1)*osVec3Length(&V2);
	   //!�õ����������ཻ�Ļ���
	   double angle = acos( dotProduct / vectorsMagnitude );
	   if(_isnan(angle)){
		   //assert(!"-1.#IND0000000�ڼ��㷴����ʱ����");
		   return 0.0f;
	   }
	   return angle;
   }
   OSENGINE_API inline bool		IntersectTriangle(const os_Ray &ray, const osVec3D& v0,const osVec3D& v1, const osVec3D& v2,osVec3D &_interpos)
   {
	   const double MATCH_FACTOR = 0.9999;	
	   Plane TriPlane(v0,v1,v2);
	   _interpos = IntersectionPoint(ray,TriPlane);
	   osVec3D InterEdg0(v0.x-_interpos.x,v0.y-_interpos.y,v0.z-_interpos.z);
	   osVec3D InterEdg1(v1.x-_interpos.x,v1.y-_interpos.y,v1.z-_interpos.z);
	   osVec3D InterEdg2(v2.x-_interpos.x,v2.y-_interpos.y,v2.z-_interpos.z);
	   double angle =0.0;
	   angle+=AngleBetweenVectors(InterEdg0,InterEdg1);
	   angle+=AngleBetweenVectors(InterEdg1,InterEdg2);
	   angle+=AngleBetweenVectors(InterEdg2,InterEdg0);
	   if (angle>=(MATCH_FACTOR * (2.0 * OS_PI))){
		   return true;
	   }
	   return false;
   }

   //@}


   inline int  get_indexCount( int _priType, int _priNum )
   {
	   switch( _priType )
	   {
	   case POINTLIST:		return _priNum;
	   case LINELIST:		return _priNum * 2;
	   case LINESTRIP:		return _priNum + 1;
	   case TRIANGLELIST:	return _priNum * 3;

	   case TRIANGLESTRIP:	return _priNum + 2;
	   case TRIANGLEFAN:	return _priNum + 2;
	   };
	   return 0;
   }

};




# define MAX_SPLITNUM      64
inline osVec4D operator *(const osVec4D& vec,const osMatrix& mat)
{
	osVec4D tmp;
	tmp.x = vec.x*mat._11 + vec.y*mat._21 + vec.z*mat._31 + vec.w*mat._41;
	tmp.y = vec.x*mat._12 + vec.y*mat._22 + vec.z*mat._32 + vec.w*mat._42;
	tmp.z = vec.x*mat._13 + vec.y*mat._23 + vec.z*mat._33 + vec.w*mat._43;
	tmp.w = vec.x*mat._14 + vec.y*mat._24 + vec.z*mat._34 + vec.w*mat._44;
	return tmp;
}
/** \brief
*  ���ú����ļ��ϡ�
*
*  
*/

/** A very simple spline class which implements the Catmull-Rom class of splines.
@remarks
Splines are bendy lines. You define a series of points, and the spline forms
a smoother line between the points to eliminate the sharp angles.
@par
Catmull-Rom splines are a specialisation of the general Hermite spline. With
a Hermite spline, you define the start and end point of the line, and 2 tangents,
one at the start of the line and one at the end. The Catmull-Rom spline simplifies
this by just asking you to define a series of points, and the tangents are 
created for you. 
@note
��OGRE���������,���ڶ�һϵ�е�Ĳ�ֵ��������
*/
class OSENGINE_API SimpleSpline
{
public:
	SimpleSpline()
	{
		// Set up matrix
		// Hermite polynomial
		/*
		2 -2 1 1
		-3 3 -2 -1
		0 0 1 0 
		1 0 0 0 
		*/
		mCoeffs._11 = 2;
		mCoeffs._12 = -2;
		mCoeffs._13 = 1;
		mCoeffs._14 = 1;
		mCoeffs._21 = -3;
		mCoeffs._22 = 3;
		mCoeffs._23 = -2;
		mCoeffs._24 = -1;
		mCoeffs._31 = 0;
		mCoeffs._32 = 0;
		mCoeffs._33 = 1;
		mCoeffs._34 = 0;
		mCoeffs._41 = 1;
		mCoeffs._42 = 0;
		mCoeffs._43 = 0;
		mCoeffs._44 = 0;


		mAutoCalc = true;
	}
	~SimpleSpline()
	{
	}

	/** Adds a control point to the end of the spline. */
	void addPoint(const osVec3D& p)
	{
		mPoints.push_back(p);
		if (mAutoCalc)
		{
			recalcTangents();
		}
	}

	/** Gets the detail of one of the control points of the spline. */
	const osVec3D& getPoint(unsigned short index) const
	{
		assert (index < mPoints.size() && "Point index is out of bounds!!");

		return mPoints[index];
	}

	/** Gets the number of control points in the spline. */
	unsigned short getNumPoints(void) const
	{
		 return (unsigned short)mPoints.size();
	}

	/** Clears all the points in the spline. */
	void clear(void)
	{
		mPoints.clear();
		mTangents.clear();
	}

	/** Updates a single point in the spline. 
	@remarks
	This point must already exist in the spline.
	*/
	void updatePoint(unsigned short index, const osVec3D& value)
	{
		assert (index < mPoints.size() && "Point index is out of bounds!!");

		mPoints[index] = value;
		if (mAutoCalc)
		{
			recalcTangents();
		}
	}

	/** Returns an interpolated point based on a parametric value over the whole series.
	@remarks
	Given a t value between 0 and 1 representing the parametric distance along the
	whole length of the spline, this method returns an interpolated point.
	@param t Parametric value.
	*/
	osVec3D interpolate(float t)
	{

		// Currently assumes points are evenly spaced, will cause velocity
		// change where this is not the case
		// TODO: base on arclength?


		// Work out which segment this is in
		float fSeg = t * mPoints.size();
		unsigned int segIdx = (unsigned int)fSeg;
		// Apportion t 
		t = fSeg - segIdx;

		return interpolate(segIdx, t);
	}

	/** Interpolates a single segment of the spline given a parametric value.
	@param fromIndex The point index to treat as t=0. fromIndex + 1 is deemed to be t=1
	@param t Parametric value
	*/
	osVec3D interpolate(unsigned int fromIndex, float t)
	{
		// Bounds check
		assert (fromIndex >= 0 && fromIndex < mPoints.size() &&
			"fromIndex out of bounds");

		if ((fromIndex + 1) == mPoints.size())
		{
			// Duff request, cannot blend to nothing
			// Just return source
			return mPoints[fromIndex];

		}

		// Fast special cases
		if (t == 0.0f)
		{
			return mPoints[fromIndex];
		}
		else if(t == 1.0f)
		{
			return mPoints[fromIndex + 1];
		}

		// Real interpolation
		// Form a vector of powers of t
		float t2, t3;
		t2 = t * t;
		t3 = t2 * t;
		osVec4D powers(t3, t2, t, 1);


		// Algorithm is ret = powers * mCoeffs * Matrix4(point1, point2, tangent1, tangent2)
		osVec3D& point1 = mPoints[fromIndex];
		osVec3D& point2 = mPoints[fromIndex+1];
		osVec3D& tan1 = mTangents[fromIndex];
		osVec3D& tan2 = mTangents[fromIndex+1];
		osMatrix pt;

		pt._11= point1.x;
		pt._12 = point1.y;
		pt._13 = point1.z;
		pt._14 = 1.0f;
		pt._21 = point2.x;
		pt._22 = point2.y;
		pt._23 = point2.z;
		pt._24 = 1.0f;
		pt._31 = tan1.x;
		pt._32 = tan1.y;
		pt._33 = tan1.z;
		pt._34 = 1.0f;
		pt._41 = tan2.x;
		pt._42 = tan2.y;
		pt._43 = tan2.z;
		pt._44 = 1.0f;

		osVec4D ret = powers * mCoeffs * pt;


		return osVec3D(ret.x, ret.y, ret.z);
	}


	/** Tells the spline whether it should automatically calculate tangents on demand
	as points are added.
	@remarks
	The spline calculates tangents at each point automatically based on the input points.
	Normally it does this every time a point changes. However, if you have a lot of points
	to add in one go, you probably don't want to incur this overhead and would prefer to 
	defer the calculation until you are finished setting all the points. You can do this
	by calling this method with a parameter of 'false'. Just remember to manually call 
	the recalcTangents method when you are done.
	@param autoCalc If true, tangents are calculated for you whenever a point changes. If false, 
	you must call reclacTangents to recalculate them when it best suits.
	*/
	void setAutoCalculate(bool autoCalc)
	{
		mAutoCalc = autoCalc;
	}

	/** Recalculates the tangents associated with this spline. 
	@remarks
	If you tell the spline not to update on demand by calling setAutoCalculate(false)
	then you must call this after completing your updates to the spline points.
	*/
	void recalcTangents(void)
	{
		// Catmull-Rom approach
		// 
		// tangent[i] = 0.5 * (point[i+1] - point[i-1])
		//
		// Assume endpoint tangents are parallel with line with neighbour

		unsigned int i, numPoints;
		bool isClosed;

		numPoints = (unsigned int)mPoints.size();
		if (numPoints < 2)
		{
			// Can't do anything yet
			return;
		}

		// Closed or open?
		if (mPoints[0] == mPoints[numPoints-1])
		{
			isClosed = true;
		}
		else
		{
			isClosed = false;
		}

		mTangents.resize(numPoints);



		for(i = 0; i < numPoints; ++i)
		{
			if (i ==0)
			{
				// Special case start
				if (isClosed)
				{
					// Use numPoints-2 since numPoints-1 is the last point and == [0]
					mTangents[i] = 0.5 * (mPoints[1] - mPoints[numPoints-2]);
				}
				else
				{
					mTangents[i] = 0.5 * (mPoints[1] - mPoints[0]);
				}
			}
			else if (i == numPoints-1)
			{
				// Special case end
				if (isClosed)
				{
					// Use same tangent as already calculated for [0]
					mTangents[i] = mTangents[0];
				}
				else
				{
					mTangents[i] = 0.5 * (mPoints[i] - mPoints[i-1]);
				}
			}
			else
			{
				mTangents[i] = 0.5 * (mPoints[i+1] - mPoints[i-1]);
			}

		}

	}

protected:

	bool mAutoCalc;

	std::vector<osVec3D> mPoints;
	std::vector<osVec3D> mTangents;

	/// Matrix of coefficients 
	osMatrix mCoeffs;



};
/// engine representation of an enum.
struct OSENGINE_API EnumTable
{
	/// Number of enumerated items in the table.
	int size;
	/// This represents a specific item in the enumeration.
	struct Enums
	{
		int         index;   ///< Index label maps to.
		const char* label;   ///< Label for this index.
	};

	const Enums *table;

	/// Constructor.
	///
	/// This sets up the EnumTable with predefined data.
	///
	/// @param sSize  Size of the table.
	/// @param sTable Pointer to table of Enums.
	///
	/// @see gCharInfoRefreshEnums
	EnumTable(int sSize, const Enums *sTable)
	{
		size = sSize;
		table = sTable;
	}

};

OSENGINE_API int FindEnumTalbe(const EnumTable& _table,const char* _name);

