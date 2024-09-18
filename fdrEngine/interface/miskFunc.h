//=======================================================================================================
/**  \file
 *   Filename: miskFunc.h
 *   Desc:     Osok引擎中常用的杂项函数，每一个头文件都应该包含这个文件。
 *
 *   His:      River created @ 5/15 2003.
 *
 *   "故事的主角有很多机会他们可以放弃,但是他们并没有,他们决定勇往直前,
 *    因为他们抱着一种信念."
 * 
 */
//=======================================================================================================
# pragma    once
#include "osInterface.h"

/** \brief
 *  一个几何体相对一个面的位置.
 *
 *  用于bsp和常用的几何数据操作.
 */
enum ose_geopos 
{
	OSE_FRONT = 0,
	OSE_ONPLANE = 1,
	OSE_BACK = 2,
	
	OSE_INTERSECT = 3,
};

// 
// ATTENTION: 常用的结构，不使用虚函数，虚函数会使结构产生一个V-table的结构，
//            如果对结构使用memeset,会破坏V-table,调用出错.不使用虚函数，可以
//            直接对结构使用memset,也可以直接对结构进行赋值。

/** \brief
 *  Bounding sphere,用于sphere碰撞检测的类.用于bsp碰撞检测.
 *
 */
class OSENGINE_API os_bsphere
{
public:
	osVec3D     veccen;
	float       radius;
public:
	//! 顶点是否在当前sphere的内部。
	bool        pt_inSphere( osVec3D& _pos );

	//! 跟aabbox是否相交
	bool        collision( const os_aabbox& _box );

	/** \brief
	 *  跟三角形的碰撞
	 *
	 *  \param osVec3D* _vec1,2,3 传入三角形的三个顶点
	 *  \param osPlane* _plane    三角的面,节省内部的运算量,如果外部没有计算,则默认为空,内部计算
	 */
	bool        collision( const osVec3D* _vec1,const osVec3D* _vec2,
		            const osVec3D* _vec3,const osPlane* _plane = NULL );

	//! 根据传入的顶点来扩展bounding sphere.
	void        extendedBs( osVec3D& _newpt );

};
#define epsilon 0.00001f
static inline bool equal(float a, float b)
{
	const float d = a - b;
	if (d<epsilon && d>-epsilon) return true;
	else return false;
}

//! 纯粹的AABBox
class OSENGINE_API os_aabbox
{
public:
	osVec3D    m_vecMax;
	osVec3D    m_vecMin;
public:
	//! 有相交返回true.
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
	*  射线是否和当前的Bounding Box相交。
	*
	*  \param _res 如果射线和BoundingBox相交，则此值为返回的交点值。
	*  \param _dis 如果相交，此值返回开始点和交点之间的矩离。
	*  
	*/
	bool ray_intersect( osVec3D& _start,osVec3D& _dir,float& _dis );
	
	/** \brief
	 *  线段是否和bounding Box相交.
	 */
	bool ls_intersect( osVec3D& _ls,osVec3D& _le );


	//! 顶点是否在aabbox内
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
	//! 得到8个顶点
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

	//! 从一个矩阵转化当前的min和max,并得到一个新的aabbox.
	void    get_transNewAABBox( osMatrix& _inMat,os_aabbox& _res );

};

/** \brief 
 *  Oriented Bounding box,用于Box的碰撞检测.用于bsp的碰撞检测.
 * 
 *  BBox的最终结果由以下运算得到：
 *  vecmax和vecmin以原点为中心，绕Y轴旋转m_fBoxRot弧度，然后平移到m_vecBoxPos位置
 */
class OSENGINE_API os_bbox
{
private:
	osVec3D     vecmax;
	osVec3D     vecmin;

	//! 当前bounding box在世界空间中的位置。
	osVec3D     m_vec3BoxPos;

	//! 当前Bounding Box的旋转角度,绕Y轴旋转
	float       m_fBoxRot;

	//! bbox的位置旋转等信息是否改变
	BOOL        m_bChanged;

	//! 转化到世界空间的bbox顶点队列.
	osVec3D     m_vecWorldVer[8];


	//! 传出8个转化后的顶点
	void       to_worldVertex( void );

public:
	os_bbox();

	//! 设置最大点和最小点
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

	//! 转化为二维的包围盒
	void              to_vec2Box( osVec2D* _vec );


	//! 把aabbox赋给oriented bbox.
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
	*  线段是否和当前的Bounding Box相交。
	*
	*  \param _res 如果射线和BoundingBox相交，则此值为返回的交点值。
	*  \param _dis 如果相交，此值返回开始点和交点之间的矩离。
	*  
	*/
	bool      ray_intersect( osVec3D& _start,osVec3D& _dir,float& _dis );	

	/** \brief
	 *  判断一条有radius的射线是否与Bounding box相交。
	 *  
	 *  \param osVec3D _start,_end 射线的开始点和结束点。
	 *  \param float _radius 射线的半径
	 */
	bool      ray_intersectWithRadius( osVec3D& _start,osVec3D& _end,float _radius );


	//! 当前点是否在BoundingBox的内部。
	bool      pt_inBBox( osVec3D& _pos );
	//! 检测一组顶点，看是否有顶点在bbox内.
	bool      has_ptInBBox( osVec3D* _pt,int _num );

	void      to_bsphere( os_bsphere& _sphere );
	void      to_aabbox( os_aabbox& _aabbox );

	// 相交测试
	bool      collision( const os_aabbox& _aabbox );
	bool      collision( const os_bbox& _bbox );
	bool      collision( const os_bsphere& _sphere );

	//! 得到盒子的六个面,面的法向量朝里.
	void      get_boxPlane( osPlane _plane[6] );

	//! 当前bbox内的顶点是否全部都在aabbox内
	bool      all_inAABBox( const os_aabbox& _aabbox );


	//! 得到此box转化到世界空间后的8个顶点.
	const osVec3D*  get_worldVertexPtr( void ) 
	{ 
		if( m_bChanged ) 
			to_worldVertex();
		return m_vecWorldVer; 
	}

};

/** 表示3D空间的一条射线,由起点和方向组成 */
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

		/** 设置射线的起点 */
		void setOrigin(const osVec3D& origin) {mOrigin = origin;} 
		/** 得到射线的起点 */
		const osVec3D& getOrigin(void) const {return mOrigin;} 

		/** 设置射线的方向 */
		void setDirection(const osVec3D& dir) {mDirection = dir;} 
		/** 得到射线的方向 */
		const osVec3D& getDirection(void) const {return mDirection;} 

		/** 计算沿射线方向 t 长度的点坐标 */
		osVec3D getPoint(float t) const { 
			return osVec3D(mOrigin + (mDirection * t));
		}

		/** 计算沿射线方向 t 长度的点坐标 */
		osVec3D operator*(float t) const { 
			return getPoint(t);
		}

		/** 测试射线是否与给定的平面相交
		@returns 如果相交反回TRUE ,不相交反回FALSE
		*/
		//bool intersects(const Plane & p) const
		//{
		//	return true;
		//	//			return Math::intersects(*this, p);
		//}

		/** 测试射线是否与给定的轴对齐盒子相交
		@returns 如果相交反回TRUE ,不相交反回FALSE
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
	 *  把这个缓冲区中的大写字母变为小写字母.
	 */
	OSENGINE_API void convert_lowercase( char* _buf );


	/** \brief 
	 *  得到dx错误代码的函数，dx函数的封装。
	 */
	OSENGINE_API inline const char* get_errorStr( HRESULT _hr )
	{
		return DXGetErrorDescription9( _hr );
	}

	/** \brief
	 *  得到顶点到一个面的距离.
	 */
	OSENGINE_API inline float distance_V2P( const osVec3D& _v,const osPlane& _p )
	{
		return osVec3Dot( &_v,(osVec3D*)&_p ) + _p.d ;
	}


	/** \brief
	*  对两个盒子进行对齐.是_b2盒子移动到_np2的位置上，跟_b1对齐。
	*
	*  _snap1: 在旋转轴上矩离最近的面对齐。
	*  _snap2: 中心点对齐到旋转轴上，
	*  _snap3: 最后是高度对齐,在高度上把两个盒子的底线保持到一个位置上。
	*
	*  算法描述：
	*  1: 计算出两个bbox要对齐的轴线。
	*  2: 计算第二个盒子在轴线向量上的可以跟第一个子接触的向量长度。对齐_snap1
	*  3: 把第二个盒子的中心点移动到两个bbox要对齐的轴线上，对齐_snap2
	*  4: 移动第二个盒子的中心点，把两个的盒子的Y方向最小值放到相同的Y方向水平线上。
	*   
	*/
	OSENGINE_API bool snap_bbox( os_bbox& _b1,
		 os_bbox& _b2,osVec3D& _np2,bool _snap1,bool _snap2,bool _snap3 );

	/** \brief
	 *  得到一个顶点和另外两个顶点所成的直线的矩离。
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
	 *  把顶点数组中的顶点按顺时针方向进行排列.
	 */
	OSENGINE_API bool sort_Vertex2Cw( osVec3D* _pvert,int _num ,osPlane& _plane );


	/** \brief
	 *  得到顶点是不是在同一条直线上.
	 */
	OSENGINE_API bool get_coline( osVec3D* _vec,int _vecnum );


	/** \brief
	 *  得到一个bounding box的max extents.
	 */
	OSENGINE_API void get_bboxExtents( osVec3D& _res,osVec3D& _min,osVec3D& _max );

	/** \brief
	 *  得到一个Bounding box和一个面计算碰撞时需要的偏移距离.
	 */
	OSENGINE_API float distance_bboxOffset( osVec3D& _extents,osPlane& _plane );

	/** \brief
	 *  从多个Bsphere中得到一个Bsphere.
	 */
	OSENGINE_API void get_bsFromBsArray( os_bsphere& _res,os_bsphere* _barr,int _num );

	/** \brief
	 *  检测一条射线是否和一个三角形相交。
	 *
	 *  \param _start,_dir 开始顶点和这个顶点的方向向量。
	 *  \param _v0,_v1,_v2 三角形的三个顶点。
	 *  \param _t  　　　　返回开始点和交点间的矩离。
	 *  \param _u,_v       返回交点在三角形上的uv分量。
	 */
	OSENGINE_API bool intersect_tri( osVec3D& _orig,osVec3D& _dir,
		const osVec3D& _v0,const osVec3D& _v1,
		const osVec3D& _v2,float& _t,float& _u,float& _v );



	/** \brief
	 *  看一条射线是否会和一个多边形相交，如果相交，求出交点。
	 *  polydata必须是这个多边顶点数据的顺时针排列.
	 */
	OSENGINE_API bool intersect_Ray2Poly( const osVec3D& _ori, const osVec3D& _end,
		                               int _vernum,const osVec3D* _polydata,osVec3D& _result );

	/** \brief
	 *  看一条射线是是否和一个多边形相交,射线加入了bounding sphere.
	 */
	OSENGINE_API bool intersect_Ray2Poly( const osVec3D& _ori,const osVec3D& _end,float _radius,
		                               int _vernum,const osVec3D* _polydata,osVec3D& _result );
		                         


	/** \brief
	 *  判断第一条线段的两个点是否在第二条线段的两侧。
	 */
	OSENGINE_API bool intersect_seg2d( osVec2D& _s1,osVec2D& _e1,osVec2D& _s2,osVec2D& _e2 );

	/** \brief
	 * 判断两条三维的线段是否相交。
	 */


	/** \brief
	 *  查看一个顶点相对于一个面的位置.
	 *
	 */
	OSENGINE_API inline ose_geopos  classify_PointByPlane( const osPlane& _p,const osVec3D& _vec )
	{
		float    tmpf;

		// ATTENTION TO FIX:使用EPSILON会出错，不使用EPSILON也会出错????
		tmpf = distance_V2P( _vec,_p );
		if( tmpf > 0.0f )    //EPSILON )
			return OSE_FRONT;
		else if( tmpf< 0.0f )//-EPSILON) 
			return OSE_BACK;
		else
			return OSE_ONPLANE;
	}


	/** \brief
	 *  得到一条线段和一个平面相交的交点。
     */
	OSENGINE_API bool get_LsIntersectPlane( osVec3D& _result,osVec3D& _start,osVec3D& _end,
		                               const osPlane& _plane,float& _percent );



	/** \brief
	 *  查看一个几何体相对于一个面的位置.
	 *
	 *  在bsp树中大量用到这个函数.
	 */
	OSENGINE_API ose_geopos classify_GeoByPlane( const osPlane& _p,const osVec3D* _vec,int _vecnum );

	/** \brief
	 *  把一个多边形用面进行分割的函数.用于bsp树的编译.也可以用于decal的剪切.
	 *
	 *  \param osVec3D* _polyvec 传入的顶点必须顺时针排列。并且传入顶点组成的多边
	 *                           形必须是凸多边形，在一个平面上。
	 *  
	 *  算法描述：
	 *  1: 
	 */
	OSENGINE_API void split_PolyByPlane( const osPlane& _plane,
		const osVec3D* _polyvec,int& _vecnum,osVec3D* _resvec,int& _resvecnum );


	/** \brief
	 *  得到顶点投影到一个面上得到的顶点.
	 *
	 *  算法描述:
	 *
	 *  找出当前顶点和面之间的距离,使当前面的法向量乘以这个距离,
	 *  以当前的顶点加上乘后的向量,就得到这个顶点在面的投影点.
	 *
	 *  使用这个函数，也可以得到一个向量在一个面的投影向量。
	 *  
	 */
	OSENGINE_API void project_PtToPlane( osPlane& _plane,osVec3D& _srcpt,osVec3D& _respt );

	/** \brief
	 *  得到一个顶点在一条直线上的投影顶点
	 *
	 *  \param osVec3D& _v 要投影的顶点
	 *  \param osVec3D& _res 返回的投影顶点。
	 *  \param osVec3D& _s   直线的开始点顶点。
	 *  \param osVec3D& _e   直线的结速顶点。
	 *  
	 *  算法描述：
	 *  先找出和_v顶点“平行”的包含_s和_e的面。然后投影_v顶点到这个面上，即为我们要找到的顶点。
	 */
	OSENGINE_API void project_PtToLine( osVec3D& _res,osVec3D& _v,osVec3D& _s,osVec3D& _e );


	/** \brief
	 *  得到三个平面的交点。
	 *
	 *  如果其中有两个平面平行的话，返回错误。
	 *  目前不使用这个函数。
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
	 *  得到当前mesh的Bounding Box.
	 */
	OSENGINE_API bool get_meshBBox( LPD3DXMESH _mesh,osVec3D& _max,osVec3D& _min );

	/** \brief
	 *  更改当前string内的最后三个字母,用于更改文件扩展名.
	 */
	OSENGINE_API bool change_extName( s_string& _str,const char* _newExt );



	/** \brief
	 *  得到两个向量的夹角，返回的是弧度。
	 *
	 *  假设传入的两个参数都是法向量。
	 */
	OSENGINE_API float get_vectorAngle( osVec3D& _normalVec1,osVec3D& _normalVec2 );

	//! 得到两个二维向量之间的夹角.
	OSENGINE_API float get_vectorAngle( osVec2D& _normalVec1,osVec2D& _normalVec2 );


	/** \brief
	 *  转化3dsMax的右手坐标第为左手坐标系.并非完全的右手坐标转左手坐标。
	 *
	 *  从d3d的左手坐转化到3dsMax的右手坐标也可以使用这个函数。
	 */
	OSENGINE_API void convert_coorRhToLh( osMatrix& _res,osMatrix& _src );

	//! 从一串的vertex中，计算出当前vertex的aabbox.
	OSENGINE_API void cal_aabboxFromVArray( osVec3D* _vec,int _num,os_aabbox& _box );

	/** \brief
	 *   对两个浮点值进行插值运算。
	 *
	 *   \return float      返回的结果。
	 *   \param  _src,_dst  两个要插值的浮点数。
	 *   \param  _lv        插值系数。
	 */
	OSENGINE_API inline float   float_lerp( float _src,float _dst,float _lv )
	{
		return _lv*(_dst-_src)+_src;
	}

	
	//! 检查DWORD值的低位WORD是否相等。
	OSENGINE_API inline bool    low_equal( DWORD _a,WORD _b )
	{
		if( WORD(_a) == _b )
			return true;
		else
			return false;
	}

	//! 使用一个DWORD的高位WORD和一个DWROD的代位word构建一个dword.
	OSENGINE_API inline DWORD   syn_dword( DWORD _hi,DWORD _low )
	{
		DWORD   t_dw = (_hi&0x0000ffff)<<16;
		
		return (t_dw | (WORD)_low);
	}

	//! 得到一个dword值的高位值。
	OSENGINE_API inline int    get_hiword( DWORD _a )
	{
		return (_a&0xffff0000)>>16;
	}


	/** \brief
	 *  对使用一个插值系数对两个矩阵进行插值。
	 *  
	 *  \param _resmat  结果矩阵.
	 *  \param _src     源矩阵。
	 *  \param _dst     目标矩阵.
	 *  \param _lv      插值系数。
	 *  
	 */
	OSENGINE_API void matrix_lerp( osMatrix& _resmat,osMatrix& _src,osMatrix& _dst,float _lv );


	/** \brief
	 *  去除文件名中多余的分隔符信息
	 *
	 *  \param _isz 即是输入参数，也是输出结果。
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
	 *  给出两个向量,计算这两个向量的之间的quaternion. 
	 * 
	 *  目前主要用于刀光的计算.
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
    *   打开一个进程的帮助函数
	*/
   OSENGINE_API bool osCreateProcess( const char* _pname );
   //@{
   // Windy mod @ 2005-11-7 16:12:36
   //Desc: 插值模版函数
   template<class T>
	   inline T interpolate(const float r, const T &v1, const T &v2)
   {
	   return static_cast<T>(v1*(1.0f - r) + v2*r);
   }
   
   //!Hermite插值．已知若干离散点的位置和导数值．求相对位置点．
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
   //Desc: LOD 用于计算距离的，计算太慢要改进。
   inline const float Vec3Distance(osVec3D &a,osVec3D &b){
	   return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z)); 
   }
  
   //@}
   //@{
   // Windy mod @ 2006-1-14 9:54:16
   //Desc: 
   /** 射线 / 盒子 相交测试, 反回BOOL结果 TRUE为相交. */
   OSENGINE_API inline bool		Intersects(const os_Ray& ray, const os_bbox& box);
   /** 射线 / 三角片相交测试,返回TURE 相交,及交点*/
  
   OSENGINE_API inline osVec3D	IntersectionPoint(const os_Ray &ray, const Plane& plane)
   {
	   if (!equal(osVec3Length(&ray.getDirection()),1.0f))
	   {
		   assert(!"入射光线没有规一化");
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
	   //!计算两向量的点积
	   double dotProduct = osVec3Dot(&V1,&V2);
	   //!计算两个向量长度的积
	   double vectorsMagnitude = osVec3Length(&V1)*osVec3Length(&V2);
	   //!得到两个向量相交的弧度
	   double angle = acos( dotProduct / vectorsMagnitude );
	   if(_isnan(angle)){
		   //assert(!"-1.#IND0000000在计算反余弦时出现");
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
*  常用函数的集合。
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
从OGRE里提出来的,用于对一系列点的插值曲线生成
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

