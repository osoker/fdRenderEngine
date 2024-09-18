//=======================================================================================================
/**  \file
 *   Filename: miskFunc.h
 *   Desc:     Osok�����г��õ��������ÿһ��ͷ�ļ���Ӧ�ð�������ļ���
 *
 *   His:      River created @ 5/31 2003.
 *
 *   "���µ������кܶ�������ǿ��Է���,�������ǲ�û��,���Ǿ�������ֱǰ,
 *    ��Ϊ���Ǳ���һ������."
 * 
 */
//=======================================================================================================
# include "stdafx.h"
# include "../../interface/osInterface.h"


#define MAX_PVERNUM   64


static void reverse_array( int _num,osVec3D* _pvert )
{
	
	osVec3D tmpvec;
	int i;

	if( _num&1 )
	{
		for( i=1;i<_num/2+1;i++ )
		{
			tmpvec = _pvert[i];
			_pvert[i] = _pvert[_num-i];
			_pvert[_num-i] = tmpvec;
		}
	}
	else
	{
		for( i=1;i<_num/2;i++ )
		{

			tmpvec =   _pvert[i];
			_pvert[i] = _pvert[_num-i];
			_pvert[_num-i] = tmpvec;

		}
	}

}


/** \brief
*  ������������еĴ�д��ĸ��ΪСд��ĸ.
*/
OSENGINE_API void osn_mathFunc::convert_lowercase( char* _buf )
{
	guard;

	int    t_cnum,t_i;

	t_cnum = (int)strlen( _buf );
	for( t_i=0;t_i<t_cnum;t_i++ )
	{
		if( (_buf[t_i]>=65)&&(_buf[t_i]<=90) )
			_buf[t_i] += 32;
	}

	return;

	unguard;
}



/** \brief
*  �õ������ǲ�����ͬһ��ֱ����.
*/
OSENGINE_API bool osn_mathFunc::get_coline( osVec3D* _vec,int _vecnum )
{
	guard;

	osVec3D      tmpvec1,tmpvec2,tmpvec3;
	int          tmpi;

	osassert( (_vecnum>=3) );

	tmpvec1 = _vec[1] - _vec[0];
	osVec3Normalize( &tmpvec1,&tmpvec1 );

	for( tmpi=2;tmpi<_vecnum;tmpi++ )
	{
		tmpvec2 = _vec[tmpi]-_vec[tmpi-1];
		osVec3Normalize( &tmpvec2,&tmpvec2 );

		tmpvec3 = -tmpvec2;
		if( !(  ( vec3_equal( tmpvec1,tmpvec2 ) )||
			    ( vec3_equal( tmpvec1,tmpvec3 ) ) )  )
				return false;

	}

	return true;

	unguard;
}




/** \brief
*  �Ӷ��Bsphere�еõ�һ��Bsphere.
*/
OSENGINE_API void osn_mathFunc::get_bsFromBsArray( os_bsphere& _res,os_bsphere* _barr,int _num )
{
	guard;

	osassert( _barr );
	osassert( _num>1 );

	float     t_f = 0.0f;
	osVec3D   t_center(0,0,0);

	for( int t_i=0;t_i<_num;t_i++ )
		t_center += _barr[t_i].veccen;

	t_center /= (float)_num;
	_res.veccen = t_center;
	t_center = _res.veccen - _barr[0].veccen;
	_res.radius = osVec3Length( &t_center );
	_res.radius += _barr[0].radius;
	for( int t_i = 1;t_i<_num;t_i ++ )
	{
		t_f = osVec3Length( &(_res.veccen - _barr[t_i].veccen) );
		t_f += _barr[t_i].radius;

		if( _res.radius < t_f )
			_res.radius = t_f;
	}

	return;


	unguard;
}


/** \brief
*  �Ѷ��������еĶ��㰴˳ʱ�뷽���������.
*/
OSENGINE_API bool osn_mathFunc::sort_Vertex2Cw( osVec3D* _pvert,int _num ,osPlane& _plane )
{
	guard;

	osVec3D     origin_a,origin_b,origin_vec,tmpnormal,tmpvec,normal;
	bool        find = true;
	int         tmpk,tmpl;


	osassert( _pvert );
	osassert( (_num>=3) );

	normal.x = _plane.a;
	normal.y = _plane.b;
	normal.z = _plane.c;

	//�ҵ��������ε�һ����...	
	origin_a = _pvert[0];
	for( tmpk=1;tmpk<_num;tmpk++ )
	{
		find = TRUE;
		origin_b = _pvert[tmpk];
		tmpvec = origin_b-origin_a;

		osVec3Cross( &tmpnormal,&normal,&tmpvec );
		
		for( tmpl = 0;tmpl<_num;tmpl++ )
		{
			if( (!::vec3_equal(_pvert[tmpl],origin_a ))
				 &&(!vec3_equal( _pvert[tmpl],origin_b) ) )
			{
				tmpvec = _pvert[tmpl]-origin_a;
				
				if( osVec3Dot( &tmpvec,&tmpnormal )<0 )
				{
					find = false;
					break;
				}
			}
		}
		
		//see whether we've find the the one..
		if( find )
			break;

		find = false;
	}

	if( !find )
	{
		osassert( false );
		return false;
	}
	
	//swap the second vertex...
	tmpvec = _pvert[1];
	_pvert[1] = origin_b;
	_pvert[tmpk] = tmpvec;

	origin_vec = origin_b - origin_a;
	osVec3Normalize( &origin_vec,&origin_vec );
	
	//sort the vertex...
	for( tmpk=2;tmpk<_num;tmpk++ )
	{
		osVec3D b;
		double  smallest_angle;
		int     smallest;
		
		tmpvec = _pvert[tmpk]-origin_a;
		osVec3Normalize( &tmpvec,&tmpvec );
		smallest_angle = osVec3Dot( &tmpvec,&origin_vec );
		smallest = tmpk;
		
		for( tmpl = tmpk+1;tmpl<_num;tmpl++ )
		{
			double angle;
			
			b = _pvert[tmpl] -origin_a;
			osVec3Normalize( &b,&b );
			
			angle = osVec3Dot( &b,&origin_vec );
			if( angle > smallest_angle )
			{
				smallest_angle = angle;
				smallest = tmpl;
			}
		}
		
		//swap the vertex...
		tmpvec = _pvert[smallest];
		_pvert[smallest] = _pvert[tmpk];
		_pvert[tmpk] = tmpvec;
	}


	origin_a = _pvert[1] - _pvert[0];
	origin_b = _pvert[2] - _pvert[0];
	
	//if we sort vertex in cwc wise,reverse it....
	osVec3Cross( &tmpvec,&origin_a,&origin_b );
	osVec3Normalize( &tmpvec,&tmpvec );

	// �˴�ʹ��vec3_equal̫�ϸ�,�Լ�����.ATTENTION TO FIX: �˴����������⣺
# if 0
	if( !vec3_equal( tmpvec,normal ) )
# else
	if( (fabs(tmpvec.x-normal.x)>0.3f)||
		(fabs(tmpvec.y-normal.y)>0.3f)||
		(fabs(tmpvec.z-normal.z)>0.3f) )
# endif 
	{
		reverse_array( _num,_pvert );
	}

	return true;


	unguard;
}



/** \brief
*  �õ�һ��bounding box��max extents.
*/
OSENGINE_API void osn_mathFunc::get_bboxExtents( osVec3D& _res,osVec3D& _min,osVec3D& _max )
{
	guard;

	_res.x = -_min.x > _max.x ? -_min.x : _max.x;
	_res.y = -_min.y > _max.y ? -_min.y : _max.y;
	_res.z = -_min.z > _max.z ? -_min.z : _max.z;

	unguard;
}


/** \brief
*  �õ�һ��Bounding box��һ���������ײʱ��Ҫ��ƫ�ƾ���.
*/
OSENGINE_API float osn_mathFunc::distance_bboxOffset( osVec3D& _extents,osPlane& _plane )
{
	return  (float)(fabs( _extents.x * _plane.a ) +
		fabs( _extents.y * _plane.b ) +
		fabs( _extents.z * _plane.c ) );

}



/** \brief
*  ���һ�������Ƿ��һ���������ཻ��
*
*  \param _start,_dir ��ʼ������������ķ���������
*  \param _v0,_v1,_v2 �����ε��������㡣
*  \param _t  �����������ؿ�ʼ��ͽ����ľ��롣
*  \param _u,_v       ���ؽ������������ϵ�uv������
*/
OSENGINE_API bool osn_mathFunc::intersect_tri( osVec3D& _orig,
							osVec3D& _dir,const osVec3D& _v0,const osVec3D& _v1,
						   const osVec3D& _v2,float& _t,float& _u,float& _v )
{
	guard;

	// River @ 2006-2-18:
	// ��Normalize���ߵķ���,���򽻵������߿�ʼ��ľ����������
	osVec3Normalize( &_dir,&_dir );

    // Find vectors for two edges sharing vert0
    osVec3D edge1 = _v1 - _v0;
    osVec3D edge2 = _v2 - _v0;

    // Begin calculating determinant - also used to calculate U parameter
    osVec3D pvec;
    osVec3Cross( &pvec, &_dir, &edge2 );

    // If determinant is near zero, ray lies in plane of triangle
    FLOAT det = osVec3Dot( &edge1, &pvec );

    osVec3D  tvec;
    if( det > 0 )
    {
        tvec = _orig - _v0;
    }
    else
    {
        tvec = _v0 - _orig;
        det = -det;
    }

    if( det < 0.0001f )
        return false;

    // Calculate U parameter and test bounds
    _u = osVec3Dot( &tvec, &pvec );
    if( _u < 0.0f || _u > det )
        return false;

    // Prepare to test V parameter
    osVec3D qvec;
    osVec3Cross( &qvec, &tvec, &edge1 );

    // Calculate V parameter and test bounds
    _v = osVec3Dot( &_dir, &qvec );
    if( _v < 0.0f || _u + _v > det )
        return false;

    // Calculate t, scale parameters, ray intersects triangle
    _t = osVec3Dot( &edge2, &qvec );
    FLOAT fInvDet = 1.0f / det;
    _t *= fInvDet;
    _u *= fInvDet;
    _v *= fInvDet;

    return true;

	unguard;
}


/** \brief
*  ��һ�������Ƿ���һ��������ཻ������ཻ��������㡣
*  polydata�����������߶������ݵ�˳ʱ������.
*/
OSENGINE_API bool osn_mathFunc::intersect_Ray2Poly( const osVec3D& _ori, const osVec3D& _end,
								int _vernum,const osVec3D* _polydata,osVec3D& _result )
{
	guard;

	
	// �߶��Ƿ���ƽ���н���
	osVec3D     E[MAX_PVERNUM],NORMAL, TEMP;
	osVec3D     edge[MAX_PVERNUM];
	float       fTemp1,fTemp2;
	osPlane     poly_plane;
	
	
	osassert( _vernum<MAX_PVERNUM );
	osassert( _vernum>=3 );
	osassert( _polydata );
	
	//calculate the poly's plane.
	osPlaneFromPoints( &poly_plane,&_polydata[0],&_polydata[1],
		&_polydata[2] );
	
	//if two points all in poly's one side,return false.
	fTemp1 = distance_V2P( _ori,poly_plane );
	fTemp2 = distance_V2P( _end,poly_plane );
	
	if( ((fTemp1>0)&&(fTemp2>0))||
		((fTemp1<0)&&(fTemp2<0) )  )
		return false;
	
    // Find point of intersection between edge and face plane (if they're
    // parallel, edge does not intersect face and D3DXPlaneIntersectLine 
    // returns NULL)
    if( NULL == osPlaneIntersectLine( &_result, &poly_plane, &_ori, &_end ) )
        return false;
	
    // Project onto a 2D plane to make the pt-in-poly test easier
    FLOAT fAbsA = ( poly_plane.a > 0 ? poly_plane.a : -poly_plane.a);
    FLOAT fAbsB = ( poly_plane.b > 0 ? poly_plane.b : -poly_plane.b);
    FLOAT fAbsC = ( poly_plane.c > 0 ? poly_plane.c : -poly_plane.c);
    osVec2D facePoints[MAX_PVERNUM];
    osVec2D point;
	
    if( fAbsA > fAbsB && fAbsA > fAbsC )
    {
        // Plane is mainly pointing along X axis, so use Y and Z
        for( INT i = 0; i < _vernum; i++)
        {
            facePoints[i].x = _polydata[i].y;
            facePoints[i].y = _polydata[i].z;
        }
        point.x = _result.y;
        point.y = _result.z;
    }
    else if( fAbsB > fAbsA && fAbsB > fAbsC )
    {
        // Plane is mainly pointing along Y axis, so use X and Z
        for( INT i = 0; i < _vernum; i++)
        {
            facePoints[i].x = _polydata[i].x;
            facePoints[i].y = _polydata[i].z;
        }
        point.x = _result.x;
        point.y = _result.z;
    }
    else
    {
        // Plane is mainly pointing along Z axis, so use X and Y
        for( INT i = 0; i < _vernum; i++)
        {
            facePoints[i].x = _polydata[i].x;
            facePoints[i].y = _polydata[i].y;
        }
        point.x = _result.x;
        point.y = _result.y;
    }
	
	
	//
    // If point is on the outside of any of the face edges, it is
    // outside the face.  
    // We can do this by taking the determinant of the following matrix:
    // | x0 y0 1 |
    // | x1 y1 1 |
    // | x2 y2 1 |
    // where (x0,y0) and (x1,y1) are points on the face edge and (x2,y2) 
    // is our test point.  If this value is positive, the test point is
    // "to the left" of the line.  To determine whether a point needs to
    // be "to the right" or "to the left" of the four lines to qualify as
    // inside the face, we need to see if the faces are specified in 
    // clockwise or counter-clockwise order (it could be either, since the
    // edge could be penetrating from either side).  To determine this, we
    // do the same test to see if the third point is "to the right" or 
    // "to the left" of the line formed by the first two points.
    // See http://forum.swarthmore.edu/dr.math/problems/scott5.31.96.html
    float x0, x1, x2, y0, y1, y2;
    x0 = facePoints[0].x;
    y0 = facePoints[0].y;
    x1 = facePoints[1].x;
    y1 = facePoints[1].y;
    x2 = facePoints[2].x;
    y2 = facePoints[2].y;
    bool bClockwise = false;
    if( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 < 0 )
        bClockwise = true;
    x2 = point.x;
    y2 = point.y;
    for( INT i = 0; i < _vernum; i++ )
    {
        x0 = facePoints[i].x;
        y0 = facePoints[i].y;
        if( i < _vernum-1 )
        {
            x1 = facePoints[i+1].x;
            y1 = facePoints[i+1].y;
        }
        else
        {
            x1 = facePoints[0].x;
            y1 = facePoints[0].y;
        }
        if( ( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 > 0 ) == bClockwise )
            return false;
    }
	
    // If we get here, the point is inside all four face edges, 
    // so it's inside the face.
    return true;


	unguard;
}


/** \brief
*  �жϵ�һ���߶ε��������Ƿ��ڵڶ����߶ε����ࡣ
*/
OSENGINE_API bool osn_mathFunc::intersect_seg2d( osVec2D& _s1,osVec2D& _e1,osVec2D& _s2,osVec2D& _e2 )
{
	guard;

	float        t_f1,t_f2;
	osVec2D      t_vec2Ray1,t_vec2Ray2;

	// 
	// ���жϵ�һ���߶ε����������Ƿ��ڵڶ����߶ε����⡣
	t_vec2Ray1 = _e2-_s2;
	t_vec2Ray2 = _s1-_s2;
	t_f1 = osVec2CCW( &t_vec2Ray1,&t_vec2Ray2 );

	t_vec2Ray2 = _e1-_s2;
	t_f2 = osVec2CCW( &t_vec2Ray1,&t_vec2Ray2 );

	if( ((t_f1>=0)&&(t_f2<0))||((t_f1<=0)&&(t_f2>0)) )
		return true;
	else
		return false;

	unguard;
}



/** \brief
*  ��һ���������Ƿ��һ��������ཻ,���߼�����bounding sphere.
*/
OSENGINE_API bool osn_mathFunc::intersect_Ray2Poly( const osVec3D& _ori,const osVec3D& _end,float _radius,
								int _vernum,const osVec3D* _polydata,osVec3D& _result )
{
	guard;


	
	// �߶��Ƿ���ƽ���н���
	osVec3D     E[MAX_PVERNUM],NORMAL, TEMP;
	osVec3D     tstart,tend;
	osVec3D     edge[MAX_PVERNUM];
	float       fTemp1,fTemp2;
	osPlane     poly_plane;
	
	
	osassert( _vernum<MAX_PVERNUM );
	osassert( _vernum>=3 );
	osassert( _polydata );
	
	//calculate the poly's plane.
	osPlaneFromPoints( &poly_plane,&_polydata[0],&_polydata[1],
		&_polydata[2] );
	
	//if two points all in poly's one side,return false.
	fTemp1 = distance_V2P( _ori,poly_plane );
	fTemp2 = distance_V2P( _end,poly_plane );
	
	// ������bsphere�İ뾶�ж�.
	if( ((fTemp1>_radius)&&(fTemp2>_radius))||
		((fTemp1<-_radius)&&(fTemp2<-_radius) )  )
		return false;

	//
	// River add @ 7/1 2003
	// �����ʼ��ͽ���λ��poly������,��������.
	// �����ʼ��ͽ�����λ��poly��ͬ��,�ѿ�ʼ��ͽ�����
	// ��polygon normal�������ƶ�_radius����.
	//
	tstart = _ori;
	tend = _end;

	if( (fTemp1>0)&&(fTemp2>0) )
	{
		TEMP.x = -poly_plane.a;
		TEMP.y = -poly_plane.b;
		TEMP.z = -poly_plane.c;
		TEMP *= _radius;

		tstart = _ori + TEMP;
		tend  = _end + TEMP;
	}

	if( (fTemp1<0)&&(fTemp2<0) )
	{
		TEMP.x = poly_plane.a;
		TEMP.y = poly_plane.b;
		TEMP.z = poly_plane.c;
		TEMP *= _radius;

		tstart = _ori + TEMP;
		tend  = _end + TEMP;
	}


    // Find point of intersection between edge and face plane (if they're
    // parallel, edge does not intersect face and D3DXPlaneIntersectLine 
    // returns NULL)
    if( NULL == osPlaneIntersectLine( &_result, &poly_plane, &tstart, &tend) )
        return false;
	
    // Project onto a 2D plane to make the pt-in-poly test easier
    FLOAT fAbsA = ( poly_plane.a > 0 ? poly_plane.a : -poly_plane.a);
    FLOAT fAbsB = ( poly_plane.b > 0 ? poly_plane.b : -poly_plane.b);
    FLOAT fAbsC = ( poly_plane.c > 0 ? poly_plane.c : -poly_plane.c);
    osVec2D facePoints[MAX_PVERNUM];
    osVec2D point;
	
    if( fAbsA > fAbsB && fAbsA > fAbsC )
    {
        // Plane is mainly pointing along X axis, so use Y and Z
        for( INT i = 0; i < _vernum; i++)
        {
            facePoints[i].x = _polydata[i].y;
            facePoints[i].y = _polydata[i].z;
        }
        point.x = _result.y;
        point.y = _result.z;
    }
    else if( fAbsB > fAbsA && fAbsB > fAbsC )
    {
        // Plane is mainly pointing along Y axis, so use X and Z
        for( INT i = 0; i < _vernum; i++)
        {
            facePoints[i].x = _polydata[i].x;
            facePoints[i].y = _polydata[i].z;
        }
        point.x = _result.x;
        point.y = _result.z;
    }
    else
    {
        // Plane is mainly pointing along Z axis, so use X and Y
        for( INT i = 0; i < _vernum; i++)
        {
            facePoints[i].x = _polydata[i].x;
            facePoints[i].y = _polydata[i].y;
        }
        point.x = _result.x;
        point.y = _result.y;
    }
	
	
	//
    // If point is on the outside of any of the face edges, it is
    // outside the face.  
    // We can do this by taking the determinant of the following matrix:
    // | x0 y0 1 |
    // | x1 y1 1 |
    // | x2 y2 1 |
    // where (x0,y0) and (x1,y1) are points on the face edge and (x2,y2) 
    // is our test point.  If this value is positive, the test point is
    // "to the left" of the line.  To determine whether a point needs to
    // be "to the right" or "to the left" of the four lines to qualify as
    // inside the face, we need to see if the faces are specified in 
    // clockwise or counter-clockwise order (it could be either, since the
    // edge could be penetrating from either side).  To determine this, we
    // do the same test to see if the third point is "to the right" or 
    // "to the left" of the line formed by the first two points.
    // See http://forum.swarthmore.edu/dr.math/problems/scott5.31.96.html
    float x0, x1, x2, y0, y1, y2;
	float a,b,c,d;                // ����ǰ���жϵ���.
    x0 = facePoints[0].x;
    y0 = facePoints[0].y;
    x1 = facePoints[1].x;
    y1 = facePoints[1].y;
    x2 = facePoints[2].x;
    y2 = facePoints[2].y;
    bool bClockwise = false;
    if( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 < 0 )
        bClockwise = true;
    x2 = point.x;
    y2 = point.y;
    for( INT i = 0; i < _vernum; i++ )
    {
        x0 = facePoints[i].x;
        y0 = facePoints[i].y;
        if( i < _vernum-1 )
        {
            x1 = facePoints[i+1].x;
            y1 = facePoints[i+1].y;
        }
        else
        {
            x1 = facePoints[0].x;
            y1 = facePoints[0].y;
        }

        if( ( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 > 0 ) == bClockwise )
		{
			// �㵽ֱ�ߵľ���,�������������bsphere�İ뾶,�Ͳ�������������ཻ.
			a = (y1-y0)/(x1-x0);
			b = -1.0f;
			c = y1 - a*x1;
			d = float((a*x2+b*y2+c)/sqrt( a*a+b*b ));

			if( d>_radius )
				return false;
		}
    }
	
    // If we get here, the point is inside all four face edges, 
    // so it's inside the face.
	return true;

	unguard;
}



/** \brief
*  �õ�һ���߶κ�һ��ƽ���ཻ�Ľ��㡣
*/
OSENGINE_API bool osn_mathFunc::get_LsIntersectPlane( osVec3D& _result,osVec3D& _start,osVec3D& _end,
								 const osPlane& _plane,float& _percent )
{
	guard;

	osVec3D direction;
	osVec3D tmpvec;
	double	num, denom;


	// ��ȷ��ȵ��ж�
	osassert( (!( _start == _end ) ) );

	direction = _end - _start;
	tmpvec = direction;
	osVec3Normalize( &direction,&direction );
	
	denom = osVec3Dot( &direction,(osVec3D*)&_plane );
	
	// The line is parallel to the plane...
	if( fabs ( denom ) < EPSILON*0.01 )
	{
		return false;
	}
	
	num				= -osn_mathFunc::distance_V2P( _start,_plane );
	_percent        = float(num/denom) ;
	_result     	= _start + ( direction * _percent );
	_percent 		= _percent / osVec3Length( &tmpvec );
	
	return true;

	unguard;
}



/** \brief
*  �鿴һ�������������һ�����λ��.
*
*  ��bsp���д����õ��������.
*/
OSENGINE_API ose_geopos osn_mathFunc::classify_GeoByPlane( 
	              const osPlane& _p,const osVec3D* _vec,int _vecnum )
{
	guard;

	int          tmpi;
	int          frontnum,backnum,onpnum;
	ose_geopos   pos;

	osassert( _vec );
	osassert( _vecnum>0 );

	frontnum = 0;
	backnum = 0;
	onpnum = 0;

	for( tmpi=0;tmpi<_vecnum;tmpi++ )
	{
		pos = osn_mathFunc::classify_PointByPlane( _p,_vec[tmpi] );
		switch (pos)
		{
		case OSE_FRONT:
			frontnum++;
			break;
		case OSE_BACK:
			backnum++;
			break;
		case OSE_ONPLANE:
			onpnum++;
			break;
		default:
			break;
		}
	}
	if( onpnum == _vecnum )
		return OSE_ONPLANE;
	if( (frontnum+onpnum)==_vecnum )
		return OSE_FRONT;
	if( (backnum+onpnum)==_vecnum )
		return OSE_BACK;

	return OSE_INTERSECT;

	unguard;
}

/** \brief
*  ��һ�������������зָ�ĺ���.����bsp���ı���.Ҳ��������decal�ļ���.
*
*  River @ 2006-3-7: �Ż����������׼���������������decal�ļ��С�
*  
*  �㷨������
*  1: �Դ����ÿһ����¼�˶�������з�ƽ���λ�á�
*  2: �Դ������ε�ÿһ���߽��д���
*       a:��������ߵ��������㶼�ڼ���������棬�������ߵ�����������뵽
*         ���صĶ����б��С�
*       b:��������߸��������ཻ���ѽ�����뵽���صĶ����б��ڡ���������һ��
*         �ཻ�ıߡ�
*       c:��������ߵ��������㶼�ڼ�����ĺ��棬�Թ������ߵĴ���
*       d:����ҵ�����һ���ཻ�ıߣ��ѽ�����뵽���ض����б��ڡ�
*         ���Ѵ˽����Ժ��ԭ����ζ��㶼���뵽���ض����б��ڡ�
*  3: ��Ϊһ��ƽ���һ��ƽ��͹������ཻ�Ļ���ֻ�ܸ����߱��ཻ�������ҵ��ཻ���Ժ�
*     �Ϳ����˳�����
*  
*/
OSENGINE_API void osn_mathFunc::split_PolyByPlane( const osPlane& _plane,
				const osVec3D* _polyvec,int& _vecnum,osVec3D* _resvec,int& _resvecnum )
{
	guard;

	osVec3D            t_vec3Inter;
	// ���ڼ�¼ÿһ������������з����λ�á�
	static ose_geopos   t_vecPos[MAX_SPLITNUM]; 
	float              tmpf;
	bool               t_b,t_bBack;
	
	osassert( _polyvec );
	osassert( (_vecnum>=3) );
	osassert( (_vecnum<=MAX_SPLITNUM) );

	_resvecnum = 0;
	
	// 1: �Դ����ÿһ�������¼�˶�������з�ƽ���λ�á�
	t_b = true;
	t_bBack = true;
	for( int t_i=0;t_i<_vecnum;t_i ++ )
	{
		t_vecPos[t_i] = osn_mathFunc::classify_PointByPlane( _plane,_polyvec[t_i] );
		if( OSE_BACK == t_vecPos[t_i] )
			t_b = false;
		if( OSE_FRONT == t_vecPos[t_i] )
			t_bBack = false;
	}
	
	// ����ȫ������֮ǰ����ԭ����εĶ��㣬ȫ������֮���򷵻�������㡣
	if( t_b  )
	{
		_resvecnum = _vecnum;
		memcpy( _resvec,_polyvec,sizeof( osVec3D )*_vecnum );
		return;
	}
	if( t_bBack )
		return;

	// *  2: �Զ���ε�ÿһ���߽��д���
	//       a:��������ߵ��������㶼�ڼ���������棬�������ߵ�����������뵽
	//         ���صĶ����б��С�
	//       b:��������߸��������ཻ���ѽ�����뵽���صĶ����б��ڡ���������һ��
	//         �ཻ�ıߡ�
	//       c:��������ߵ��������㶼�ڼ�����ĺ��棬�Թ������ߵĴ���
	//       d:����ҵ�����һ���ཻ�ıߣ��ѽ�����뵽���ض����б��ڡ�
	//         ���Ѵ˽����Ժ��ԭ����ζ��㶼���뵽���ض����б��ڡ�
	// 
	for( int t_i=0;t_i<_vecnum;t_i ++ )
	{
		// �ߵĿ�ʼ�ͽ�����������
		int t_iVsIdx,t_iVeIdx;
		if( t_i == (_vecnum-1) )
		{
			t_iVsIdx = t_i;
			t_iVeIdx = 0;
		}
		else
		{
			t_iVsIdx = t_i;
			t_iVeIdx = t_i+1;
		}

		// �����ߵ��������㶼����֮��
		if( (OSE_BACK == t_vecPos[t_iVsIdx])&&
			(OSE_BACK == t_vecPos[t_iVeIdx]) )
			continue;

		// �����ߵ��������㶼����֮�ϻ�֮ǰ
		if( (OSE_BACK != t_vecPos[t_iVsIdx])&&
			(OSE_BACK != t_vecPos[t_iVeIdx]) )
		{
			_resvec[_resvecnum] = _polyvec[t_iVsIdx];
			_resvecnum ++;
			osassert( _resvecnum < MAX_SPLITNUM );
			continue;
		}

		// �ཻ������
		bool   t_bIntersect;
		t_bIntersect = osn_mathFunc::get_LsIntersectPlane( t_vec3Inter,
			(osVec3D&)_polyvec[t_iVsIdx],(osVec3D&)_polyvec[t_iVeIdx],_plane,tmpf );


		//@{
		// River mod @ 2007-4-26: ����ʹ�˴��������ཻ������п��ܷ�����
		//osassert( t_bIntersect );
		if( !t_bIntersect )
		{
			// River @ 2011-10-18:ȥ����Ӧ��assert,ֱ�ӷ��أ�������decal.
			/*
			osassertex( false,
				va( "Start<%f,%f,%f>,End<%f,%f,%f>..Plane<%f,%f,%f,%f>..\n",
				_polyvec[t_iVsIdx].x,_polyvec[t_iVsIdx].y,_polyvec[t_iVsIdx].z,
				_polyvec[t_iVeIdx].x,_polyvec[t_iVeIdx].y,_polyvec[t_iVeIdx].z,
				_plane.a,_plane.b,_plane.c,_plane.d ) );
			*/

			_resvecnum = 0;
			return ;
		}
		//@}

		// �߿�ʼ���������֮ǰ�����뷵�ص㡣
		if( OSE_BACK != t_vecPos[t_iVsIdx] )
		{
			_resvec[_resvecnum] = _polyvec[t_iVsIdx];
			_resvecnum ++;
			osassert( _resvecnum < MAX_SPLITNUM );
		}


		_resvec[_resvecnum] = t_vec3Inter;
		_resvecnum ++;
		osassert( _resvecnum < MAX_SPLITNUM );

	}

	osassertex( _resvecnum >= 3,"�зֺ�Ķ���Ӧ���������һ��������...\n" );

	return;

	unguard;
}

/** \brief
*  �õ�����ͶӰ��һ�����ϵõ��Ķ���.
*
*  �㷨����:
*
*  �ҳ���ǰ�������֮��ľ���,ʹ��ǰ��ķ����������������,
*  �Ե�ǰ�Ķ�����ϳ˺������,�͵õ�������������ͶӰ��.
*
*/
OSENGINE_API void osn_mathFunc::project_PtToPlane( 
	          osPlane& _plane,osVec3D& _srcpt,osVec3D& _respt )
{
	guard;

	osVec3D      t_vec;
	float        t_f;

	t_f = osn_mathFunc::distance_V2P( _srcpt,_plane );

	t_vec.x = -_plane.a*t_f;t_vec.y = -_plane.b*t_f;t_vec.z = -_plane.c*t_f;

	_respt = _srcpt + t_vec;

	return;

	unguard;
}

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
OSENGINE_API void osn_mathFunc::project_PtToLine( osVec3D& _res,osVec3D& _v,osVec3D& _s,osVec3D& _e )
{
	guard;

	osVec3D   t_vec3Tmp1,t_vec3Tmp2,t_vec3Right;
	osPlane   t_sPlane;

	osVec3D   t_vec3Tmp[3];

	t_vec3Tmp[0] = _v;
	t_vec3Tmp[1] = _s;
	t_vec3Tmp[2] = _e;
	if( osn_mathFunc::get_coline( t_vec3Tmp,3 ) )
	{
		_res = _v;
		return;
	}

	t_vec3Tmp1 = _v - _s;
	t_vec3Tmp2 = _e - _s;
	osVec3Cross( &t_vec3Right,&t_vec3Tmp1,&t_vec3Tmp2 );
	osVec3Normalize( &t_vec3Right,&t_vec3Right );
	t_vec3Right += _s;
	osPlaneFromPoints( &t_sPlane,&_s,&_e,&t_vec3Right );

	osn_mathFunc::project_PtToPlane( t_sPlane,_v,_res );

	return;

	unguard;
}


/** \brief
*  �õ���ǰmesh��Bounding Box.
*/
OSENGINE_API bool osn_mathFunc::get_meshBBox( LPD3DXMESH _mesh,osVec3D& _max,osVec3D& _min )
{
	guard;

	BYTE*    t_btVerStart;
	int      t_iNumVer;
	int      t_iStride;
	DWORD    t_dwFVF;
	HRESULT  t_hr;


	osassert( _mesh );

	t_dwFVF = _mesh->GetFVF();
	t_iStride = D3DXGetFVFVertexSize( t_dwFVF );
	t_iNumVer = _mesh->GetNumVertices();
	t_hr = _mesh->LockVertexBuffer( D3DLOCK_READONLY,(void**)&t_btVerStart );
	if( FAILED( t_hr ) )
		return false;

	t_hr = D3DXComputeBoundingBox( (D3DXVECTOR3*)t_btVerStart,t_iNumVer,t_iStride,&_max,&_min );
	if( FAILED( t_hr ) )
		return false;

	_mesh->UnlockVertexBuffer();

	return true;

	unguard;
}


/** \brief
*  ���ĵ�ǰstring�ڵ����������ĸ,���ڸ����ļ���չ��.
*/
OSENGINE_API bool osn_mathFunc::change_extName( s_string& _str,const char* _newExt )
{
	guard;

	int   t_i;
	
	t_i = (int)_str.length();
	if( t_i<3 )
		osassert( false );

	_str = _str.substr( 0,t_i-3 );
	_str += _newExt;

	return true;

	unguard;
}

/** \brief
*  ת��3dsMax�����������Ϊ��������ϵ.
*/
OSENGINE_API void osn_mathFunc::convert_coorRhToLh( osMatrix& _res,osMatrix& _src )
{
	osMatrix   t_matRot,t_matInv,t_matRef;
	
	osPlane    t_plane( 0,1,0,0 );
	osMatrixReflect( &t_matRef,&t_plane );

	// 
	// �õ���ԭ������б任�ľ���.
	osMatrixRotationX( &t_matRot,-OS_PI/2 );
	osMatrixMultiply( &t_matRot,&t_matRef,&t_matRot );


	//
	// �õ�Ҫת������������
	osMatrixInverse( &t_matInv,NULL,&t_matRot );

	//
	// 
	osMatrixMultiply( &_res,&t_matRot,&_src );

	//
	// �õ����վ���
	osMatrixMultiply( &_res,&_res,&t_matInv );

}

//! ��һ����vertex�У��������ǰvertex��aabbox.
OSENGINE_API void osn_mathFunc::cal_aabboxFromVArray( osVec3D* _vec,int _num,os_aabbox& _box )
{
	_box.m_vecMax = _vec[0];
	_box.m_vecMin = _vec[0];
	
	for( int t_i=1;t_i<_num;t_i ++ )
		_box.extend( _vec[t_i] );

	return;
}



/** \brief
*  �õ����������ļнǣ����ص��ǻ��ȡ�
*
*  ���贫���������������Normalize��������
*/
OSENGINE_API float osn_mathFunc::get_vectorAngle( 
	             osVec3D& _normalVec1,osVec3D& _normalVec2 )
{
	guard;

	float   t_dot;

	t_dot = osVec3Dot( &_normalVec1,&_normalVec2 );
	if( t_dot <= -1.0f )
		t_dot = -1.0f;
	if( t_dot > 1.0f )
		t_dot = 1.0f;
	t_dot = (float)acos( t_dot );

	// ���ڼ��:-1.#00000������
	osassert( !_isnan( t_dot ) );

	return t_dot;
	unguard;
}

//! �õ�������ά����֮��ļн�.
OSENGINE_API float osn_mathFunc::get_vectorAngle( osVec2D& _normalVec1,osVec2D& _normalVec2 )
{
	guard;

	float   t_dot;
	t_dot = osVec2Dot( &_normalVec1,&_normalVec2 );
	if( t_dot <= -1.0f )
		t_dot = -1.0f;
	if( t_dot > 1.0f )
		t_dot = 1.0f;
	t_dot = (float)acos( t_dot );

	/*
	// ���ڼ��:-1.#00000������
	osassert( !_isnan( t_dot ) );
	*/
	// River @ 2011-10-14:����˽Ƕȳ�������һ���̶��ĽǶ�.
	if( _isnan( t_dot ) )
		return OS_PI/4.0f;

	return t_dot;

	unguard;
}



/** \brief
*  ��ʹ��һ����ֵϵ��������������в�ֵ��
*
*
*  \param _resmat  �������.
*  \param _src     Դ����
*  \param _dst     Ŀ�����.
*  \param _lv      ��ֵϵ����
*  
*  Ŀǰʹ�ü򵥵����Բ�ֵ������
*  
*/
OSENGINE_API void osn_mathFunc::matrix_lerp( 
	osMatrix& _resmat,osMatrix& _src,osMatrix& _dst,float _lv )
{
	_resmat._11 = float_lerp( _src._11,_dst._11,_lv );
	_resmat._12 = float_lerp( _src._12,_dst._12,_lv );
	_resmat._13 = float_lerp( _src._13,_dst._13,_lv );
	_resmat._14 = float_lerp( _src._14,_dst._14,_lv );

	_resmat._21 = float_lerp( _src._21,_dst._21,_lv );
	_resmat._22 = float_lerp( _src._22,_dst._22,_lv );
	_resmat._23 = float_lerp( _src._23,_dst._23,_lv );
	_resmat._24 = float_lerp( _src._24,_dst._24,_lv );

	_resmat._31 = float_lerp( _src._31,_dst._31,_lv );
	_resmat._32 = float_lerp( _src._32,_dst._32,_lv );
	_resmat._33 = float_lerp( _src._33,_dst._33,_lv );
	_resmat._34 = float_lerp( _src._34,_dst._34,_lv );

	_resmat._41 = float_lerp( _src._41,_dst._41,_lv );
	_resmat._42 = float_lerp( _src._42,_dst._42,_lv );
	_resmat._43 = float_lerp( _src._43,_dst._43,_lv );
	_resmat._44 = float_lerp( _src._44,_dst._44,_lv );

	return;
}


//======================================================
// from Eric's oblique view frustum optimal code
//======================================================
OSENGINE_API void osn_mathFunc::clipProjectionMatrix(
	     const osPlane& _clipPlane, osMatrix& _matProj )
{
	osVec4D	q;

	// Calculate the clip-space corner point opposite the clipping plane
	// as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
	// transform it into camera space by multiplying it
	// by the inverse of the projection matrix

	q.x = sgn(_clipPlane.a) / _matProj.m[0][0];
	q.y = sgn(_clipPlane.b) / _matProj.m[1][1];
	q.z = 1.0F;
	q.w = (1.0F - _matProj.m[2][2]) / _matProj.m[3][2];

	// Calculate the scaled plane vector
	osVec4D  c = _clipPlane * (1.0F / osPlaneDot(&_clipPlane, &q));

	// Replace the third column of the projection matrix
	_matProj.m[0][2] = c.x;
	_matProj.m[1][2] = c.y;
	_matProj.m[2][2] = c.z;
	_matProj.m[3][2] = c.w;
}



/** \brief
*  �õ�����ƽ��Ľ��㡣
*
*  �������������ƽ��ƽ�еĻ������ش���
*/
OSENGINE_API bool osn_mathFunc::plane_intersect( 
	osVec3D& _ipt,osPlane& _a,osPlane& _b,osPlane& _c )
{
	guard;

	osassert( false );
	return true;
	
	unguard;
}

//! ȥ���ļ����ж���ķָ�����Ϣ��
OSENGINE_API void osn_mathFunc::delete_surplusSymbol( char* _isz )
{
	guard;

	s_string   t_outStr = _isz;

	std::basic_string <char>::size_type    t_iIdx;
	while( 1 )
	{
		t_iIdx = t_outStr.find( "\\\\" );
		if( t_iIdx == 0xffffffff )
			break;
		t_outStr.erase( t_iIdx,1 );
	}

	strcpy( _isz,t_outStr.c_str() );

	return;

	unguard;
}


/** \brief
*   ��һ�����̵İ�������
*/
OSENGINE_API bool osn_mathFunc::osCreateProcess( const char* _pname )
{
	guard;

	osassert( _pname&&(_pname[0]) );

	BOOL                t_b;
	STARTUPINFO          si;
	PROCESS_INFORMATION   pi;
	char                t_szCmd[256];

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	strcpy( t_szCmd,_pname );

	t_b = ::CreateProcess( NULL,t_szCmd,NULL,NULL,TRUE,
		   CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi );
	if( !t_b )
		MessageBox( NULL,_pname,"��������ʧ��",MB_OK );

	return t_b;


	unguard;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Bounding Box��Bounding Sphere�����������
// 
//////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AABB_Sphere_Collision(const os_bsphere &_sphere, const os_aabbox& _box)
{
	float fDistance = 0.0f;

	// x
	if( _sphere.veccen.x < _box.m_vecMin.x )
	{
		float fValue = (_sphere.veccen.x - _box.m_vecMin.x );
		fDistance += (fValue * fValue);
	}
	else if( _sphere.veccen.x > _box.m_vecMax.x )
	{
		float fValue = (_sphere.veccen.x - _box.m_vecMax.x );
		fDistance += (fValue * fValue);
	}

	// y
	if( _sphere.veccen.y < _box.m_vecMin.y )
	{
		float fValue = (_sphere.veccen.y - _box.m_vecMin.y );
		fDistance += (fValue * fValue);
	}
	else if( _sphere.veccen.y > _box.m_vecMax.y )
	{
		float fValue = (_sphere.veccen.y - _box.m_vecMax.y );
		fDistance += (fValue * fValue);
	}

	// z
	if( _sphere.veccen.z < _box.m_vecMin.z )
	{
		float fValue = (_sphere.veccen.z - _box.m_vecMin.z );
		fDistance += (fValue * fValue);
	}
	else if( _sphere.veccen.z > _box.m_vecMax.z )
	{
		float fValue = (_sphere.veccen.z - _box.m_vecMax.z );
		fDistance += (fValue * fValue);
	}


	if( fDistance <= (_sphere.radius*_sphere.radius) ) 
		return true;

	return false;
}


//! �����Ƿ��ڵ�ǰsphere���ڲ���
bool os_bsphere::pt_inSphere( osVec3D& _pos )
{
	float   t_f = osVec3Length( &(veccen - _pos) );
	if( t_f < radius )
		return true;
	else
		return false;
}


//! ���ݴ���Ķ�������չbounding sphere.�Ӷ�������ڹ�����С��bsphereҲ��һ����������,�Ժ��Ż�.
void os_bsphere::extendedBs( osVec3D& _newpt )
{
	if( pt_inSphere( _newpt ) )
		return;

	// ʹ����򵥵ķ���
	radius = osVec3Length( &(_newpt-veccen) );

}


//! ��aabbox�Ƿ��ཻ
bool os_bsphere::collision( const os_aabbox& _box )
{
	return AABB_Sphere_Collision( *this,_box );
}

/** \brief
*  �������ε���ײ
*
*  \param osVec3D* _vec1,2,3 ���������ε���������
*  \param osPlane* _plane    ���ǵ���,��ʡ�ڲ���������,����ⲿû�м���,��Ĭ��Ϊ��,�ڲ�����
*/
bool os_bsphere::collision( const osVec3D* _vec1,const osVec3D* _vec2,
					  const osVec3D* _vec3,const osPlane* _plane/* = NULL*/ )
{
	guard;

	osassert( _vec1&&_vec2&&_vec3 );

	osPlane    t_sPlane;

	if( _plane )
	{
		// sphere���ĵ㵽������ľ���������ڵ���sphere�İ뾶,��϶����ཻ
		if( osn_mathFunc::distance_V2P( veccen,*_plane ) >= radius )
			return false;
	}
	else
	{
		osPlaneFromPoints( &t_sPlane,_vec1,_vec2,_vec3 );
		if( osn_mathFunc::distance_V2P( veccen,t_sPlane ) >= radius )
			return false;
	}

	// ��������������bspher���ĵ�����ľ���,����˶�����bsphere��,����ײ,���򷵻�false.
	osVec3D    t_vec3Edge0 = *_vec2 - *_vec1;
	osVec3D    t_vec3Edge1 = *_vec3 - *_vec1;
	osVec3D    t_vec3Diff  = *_vec1 - veccen;

	float      t_fA00 = t_vec3Edge0.x*t_vec3Edge0.x + 
		                t_vec3Edge0.y*t_vec3Edge0.y + t_vec3Edge0.z*t_vec3Edge0.z;
	float      t_fA01 = osVec3Dot( &t_vec3Edge0,&t_vec3Edge1 );
	float      t_fA11 = t_vec3Edge1.x*t_vec3Edge1.x + 
		                t_vec3Edge1.y*t_vec3Edge1.y + t_vec3Edge1.z*t_vec3Edge1.z;

	float      t_fB0  = osVec3Dot( &t_vec3Diff,&t_vec3Edge0 );
	float      t_fB1  = osVec3Dot( &t_vec3Diff,&t_vec3Edge1 );
	float      t_fC   = t_vec3Diff.x*t_vec3Diff.x + 
		                t_vec3Diff.y*t_vec3Diff.y + t_vec3Diff.z*t_vec3Diff.z;

	float      t_fDet = fabs( t_fA00*t_fA11 - t_fA01*t_fA01 );
	float      t_fS   = t_fA01*t_fB1 - t_fA11*t_fB0;
	float      t_fT   = t_fA01*t_fB0 - t_fA00*t_fB1;

	float      t_fSqrDistance;

    if ( t_fS + t_fT <= t_fDet)
    {
        if (t_fS < 0.0f )
        {
            if (t_fT < 0.0f )  // region 4
            {
                if (t_fB0 < 0.0f)
                {
                    t_fT = 0.0f;
                    if (-t_fB0 >= t_fA00)
                    {
                        t_fS = 1.0f;
                        t_fSqrDistance = t_fA00+(2.0f)*t_fB0+t_fC;
                    }
                    else
                    {
                        t_fS = -t_fB0/t_fA00;
                        t_fSqrDistance = t_fB0*t_fS+t_fC;
                    }
                }
                else
                {
                    t_fS = 0.0f;
                    if (t_fB1 >= 0.0f)
                    {
                        t_fT = 0.0f;
                        t_fSqrDistance = t_fC;
                    }
                    else if (-t_fB1 >= t_fA11)
                    {
                        t_fT = 1.0f;
                        t_fSqrDistance = t_fA11+(2.0f)*t_fB1+t_fC;
                    }
                    else
                    {
                        t_fT = -t_fB1/t_fA11;
                        t_fSqrDistance = t_fB1*t_fT+t_fC;
                    }
                }
            }
            else  // region 3
            {
                t_fS = 0.0f;
                if (t_fB1 >= 0.0f)
                {
                    t_fT = 0.0f;
                    t_fSqrDistance = t_fC;
                }
                else if (-t_fB1 >= t_fA11)
                {
                    t_fT = 1.0f;
                    t_fSqrDistance = t_fA11+(2.0f)*t_fB1+t_fC;
                }
                else
                {
                    t_fT = -t_fB1/t_fA11;
                    t_fSqrDistance = t_fB1*t_fT+t_fC;
                }
            }
        }
        else if (t_fT < 0.0f)  // region 5
        {
            t_fT = 0.0f;
            if (t_fB0 >= 0.0f)
            {
                t_fS = 0.0f;
                t_fSqrDistance = t_fC;
            }
            else if (-t_fB0 >= t_fA00)
            {
                t_fS = 1.0f;
                t_fSqrDistance = t_fA00+(2.0f)*t_fB0+t_fC;
            }
            else
            {
                t_fS = -t_fB0/t_fA00;
                t_fSqrDistance = t_fB0*t_fS+t_fC;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            float t_fInvDet = (1.0f)/t_fDet;
            t_fS *= t_fInvDet;
            t_fT *= t_fInvDet;
            t_fSqrDistance = t_fS*(t_fA00*t_fS+t_fA01*t_fT+(2.0f)*t_fB0) +
                t_fT*(t_fA01*t_fS+t_fA11*t_fT+(2.0f)*t_fB1)+t_fC;
        }
    }
    else
    {
        float t_fTmp0, t_fTmp1, t_fNumer, t_fDenom;

        if (t_fS < 0.0f)  // region 2
        {
            t_fTmp0 = t_fA01 + t_fB0;
            t_fTmp1 = t_fA11 + t_fB1;
            if (t_fTmp1 > t_fTmp0)
            {
                t_fNumer = t_fTmp1 - t_fTmp0;
                t_fDenom = t_fA00-2.0f*t_fA01+t_fA11;
                if (t_fNumer >= t_fDenom)
                {
                    t_fS = 1.0f;
                    t_fT = 0.0f;
                    t_fSqrDistance = t_fA00+(2.0f)*t_fB0+t_fC;
                }
                else
                {
                    t_fS = t_fNumer/t_fDenom;
                    t_fT = 1.0f - t_fS;
                    t_fSqrDistance = t_fS*(t_fA00*t_fS+t_fA01*t_fT+2.0f*t_fB0) +
                        t_fT*(t_fA01*t_fS+t_fA11*t_fT+(2.0f)*t_fB1)+t_fC;
                }
            }
            else
            {
                t_fS = 0.0f;
                if (t_fTmp1 <= 0.0f)
                {
                    t_fT = 1.0f;
                    t_fSqrDistance = t_fA11+(2.0f)*t_fB1+t_fC;
                }
                else if (t_fB1 >= 0.0f)
                {
                    t_fT = 0.0f;
                    t_fSqrDistance = t_fC;
                }
                else
                {
                    t_fT = -t_fB1/t_fA11;
                    t_fSqrDistance = t_fB1*t_fT+t_fC;
                }
            }
        }
        else if (t_fT < 0.0f)  // region 6
        {
            t_fTmp0 = t_fA01 + t_fB1;
            t_fTmp1 = t_fA00 + t_fB0;
            if (t_fTmp1 > t_fTmp0)
            {
                t_fNumer = t_fTmp1 - t_fTmp0;
                t_fDenom = t_fA00-(2.0f)*t_fA01+t_fA11;
                if (t_fNumer >= t_fDenom)
                {
                    t_fT = 1.0f;
                    t_fS = 0.0f;
                    t_fSqrDistance = t_fA11+(2.0f)*t_fB1+t_fC;
                }
                else
                {
                    t_fT = t_fNumer/t_fDenom;
                    t_fS = 1.0f - t_fT;
                    t_fSqrDistance = t_fS*(t_fA00*t_fS+t_fA01*t_fT+(2.0f)*t_fB0) +
                        t_fT*(t_fA01*t_fS+t_fA11*t_fT+(2.0f)*t_fB1)+t_fC;
                }
            }
            else
            {
                t_fT = 0.0f;
                if (t_fTmp1 <= 0.0f)
                {
                    t_fS = 1.0f;
                    t_fSqrDistance = t_fA00+(2.0f)*t_fB0+t_fC;
                }
                else if (t_fB0 >= 0.0f)
                {
                    t_fS = 0.0f;
                    t_fSqrDistance = t_fC;
                }
                else
                {
                    t_fS = -t_fB0/t_fA00;
                    t_fSqrDistance = t_fB0*t_fS+t_fC;
                }
            }
        }
        else  // region 1
        {
            t_fNumer = t_fA11 + t_fB1 - t_fA01 - t_fB0;
            if (t_fNumer <= 0.0f)
            {
                t_fS = 0.0f;
                t_fT = 1.0f;
                t_fSqrDistance = t_fA11+(2.0f)*t_fB1+t_fC;
            }
            else
            {
                t_fDenom = t_fA00-2.0f*t_fA01+t_fA11;
                if (t_fNumer >= t_fDenom)
                {
                    t_fS = 1.0f;
                    t_fT = 0.0f;
                    t_fSqrDistance = t_fA00+(2.0f)*t_fB0+t_fC;
                }
                else
                {
                    t_fS = t_fNumer/t_fDenom;
                    t_fT = 1.0f - t_fS;
                    t_fSqrDistance = t_fS*(t_fA00*t_fS+t_fA01*t_fT+(2.0f)*t_fB0) +
                        t_fT*(t_fA01*t_fS+t_fA11*t_fT+(2.0f)*t_fB1)+t_fC;
                }
            }
        }
    }

	// account for numerical round-off error
	if( t_fSqrDistance < 0.0f )
		t_fSqrDistance = 0.0f;

	// ��������ε�Բ�ĵ���С����С��bsphere�İ뾶,���ཻ,���ز��ཻ
	t_fSqrDistance = sqrt( t_fSqrDistance );
	if( t_fSqrDistance < radius )
		return true;
	else
		return false;

	unguard;
}




///////////////////////////////////////////////////////////////////////////////////////////
//
// Bounding Box��ص����ݡ�
//
///////////////////////////////////////////////////////////////////////////////////////////
os_bbox::os_bbox()
{
	m_vec3BoxPos = osVec3D( 0.0f,0.0f,0.0f );
	m_fBoxRot = 0.0f;
}


//! ���һ�鶥�㣬���Ƿ��ж�����bbox��.
bool os_bbox::has_ptInBBox( osVec3D* _pt,int _num )
{
	guard;


	osVec3D         t_center,t_end;
	os_bsphere       t_sphere;
	static BOOL     t_bInBs[8];
	bool            t_bHasInBs = false;

	osassert( _pt );
	osassertex( _num < 8,"��Ҫ�Ӵ󻺳���..\n" );

	// 
	// ���λ�õ㲻�ڵ�ǰBoundingBox���ɵ�BoundingSphere���ڲ����򷵻ء�
	to_bsphere( t_sphere );
	for( int t_i=0;t_i<_num;t_i ++ )
	{
		t_bInBs[t_i] = t_sphere.pt_inSphere( _pt[t_i] );
		if( t_bInBs[t_i] )
			t_bHasInBs = true;
	}
	if( !t_bHasInBs )
		return false;

	//
	// ���̵����Ժ����ߺ�Bounding Box�ཻ�Ŀ����Ժܴ�
	// �����������ÿһ�������ε��ཻ����
	osPlane    t_vecPlane[6];

	get_boxPlane( t_vecPlane );

	for( int t_iVerIdx = 0;t_iVerIdx<_num;t_iVerIdx ++ )
	{
		if( !t_bInBs[t_iVerIdx] )
			continue;

		// 
		int  t_i;
		for( t_i=0;t_i<6;t_i ++ )
		{
			if( OSE_BACK == 
				osn_mathFunc::classify_PointByPlane( t_vecPlane[t_i],_pt[t_iVerIdx] ) )
				break;
		}

		// ֻҪ��һ��������bbox�ڣ��ͷ�����
		if( t_i == 6 )
			return true;

	}

	// �ұ����еĶ����û����bbox�ڵĶ���
	return false;

	unguard;
}



//! ��ǰ���Ƿ���BoundingBox���ڲ���
bool os_bbox::pt_inBBox( osVec3D& _pos )
{
	guard;

	osVec3D         t_center,t_end;
	os_bsphere       t_sphere;

	// 
	// ���λ�õ㲻�ڵ�ǰBoundingBox���ɵ�BoundingSphere���ڲ����򷵻ء�
	to_bsphere( t_sphere );
	if( !t_sphere.pt_inSphere( _pos ) )
		return false;

	//
	// ���̵����Ժ����ߺ�Bounding Box�ཻ�Ŀ����Ժܴ�
	// �����������ÿһ�������ε��ཻ����
	osPlane    t_vecPlane[6];

	get_boxPlane( t_vecPlane );
	for( int t_i=0;t_i<6;t_i ++ )
	{
		if( OSE_BACK == 
			osn_mathFunc::classify_PointByPlane( t_vecPlane[t_i],_pos ) )
			return false;
	}

	return true;

	unguard;
}

// River @ 2006-3-10:��ǰ�˶εĴ��룬���ش������еĹ�����������һ������Ļ���֮�ϡ�
/*
    osVec3D  t_vec3;
	t_vec3 = vecmax - vecmin;
	t_vec3 /= 2.0f;
	_sphere.veccen = t_vec3 + m_vec3BoxPos;
	_sphere.radius = osVec3Length( &t_vec3 );
*/
void os_bbox::to_bsphere( os_bsphere& _sphere )
{
	osVec3D   t_vec3;
	t_vec3 = vecmax + vecmin;
	t_vec3 /= 2.0f;
	_sphere.veccen = t_vec3 + m_vec3BoxPos;
	_sphere.radius = osVec3Length( &(vecmax-t_vec3) );

	return;
}


/** \brief
 *  ����8��ת����Ķ���
 *
 *  Y
 *  |    v7-----v6
 *  |   /      / |
 *  |  v4-----v5 |
 *  |   |v3-----v2 
 *  |   |/     /
 *  |  v0-----v1
 *  |------------------->X
 */
void os_bbox::to_worldVertex( void )
{

	m_vecWorldVer[0] = vecmin;
	m_vecWorldVer[1] = vecmin;m_vecWorldVer[1].x = vecmax.x;
	m_vecWorldVer[2] = vecmax;m_vecWorldVer[2].y = vecmin.y;
	m_vecWorldVer[3] = vecmin;m_vecWorldVer[3].z = vecmax.z;

	m_vecWorldVer[4] = vecmin;m_vecWorldVer[4].y = vecmax.y;
	m_vecWorldVer[5] = vecmax;m_vecWorldVer[5].z = vecmin.z;
	m_vecWorldVer[6] = vecmax;
	m_vecWorldVer[7] = vecmax;m_vecWorldVer[7].x = vecmin.x;

	// ������ת�Ƕȡ�
	osMatrix   t_rotMat;
	osMatrixRotationY( &t_rotMat,m_fBoxRot );
	for( int t_i=0;t_i<8;t_i ++ )
	{
		osVec3Transform( &m_vecWorldVer[t_i],&m_vecWorldVer[t_i],&t_rotMat );
		m_vecWorldVer[t_i] += m_vec3BoxPos;
	}

	// ������ʱʹ��ת����Ķ���.ֱ����һ�ιؼ����ݸı�.
	m_bChanged = FALSE;

}

//! �õ����ӵ�������,��ķ�������������ڲ���
void os_bbox::get_boxPlane( osPlane _plane[6] )
{
	const osVec3D*     t_pt;
	t_pt = get_worldVertexPtr();

	// top,bottom
	osPlaneFromPoints( &_plane[0],&t_pt[6],&t_pt[7],&t_pt[4] );
	osPlaneFromPoints( &_plane[1],&t_pt[2],&t_pt[1],&t_pt[0] );

	// left,right
	osPlaneFromPoints( &_plane[2],&t_pt[0],&t_pt[4],&t_pt[7] );
	osPlaneFromPoints( &_plane[3],&t_pt[6],&t_pt[5],&t_pt[1] );
	
	// front ,back.
	osPlaneFromPoints( &_plane[4],&t_pt[1],&t_pt[5],&t_pt[4] );
	osPlaneFromPoints( &_plane[5],&t_pt[3],&t_pt[7],&t_pt[6] );


	return ;
}




void os_bbox::to_aabbox( os_aabbox& _aabbox )
{
	const osVec3D*   t_vec3;
	t_vec3 = get_worldVertexPtr();
	
	_aabbox.m_vecMax = osVec3D( float(SG_MIN_VALUE),float(SG_MIN_VALUE),float(SG_MIN_VALUE) );
	_aabbox.m_vecMin = osVec3D( float(SG_MAX_VALUE),float(SG_MAX_VALUE),float(SG_MAX_VALUE) );

	for( int t_i = 0;t_i<8;t_i ++ )
	{
		// max vec.
		if( _aabbox.m_vecMax.x < t_vec3[t_i].x )
			_aabbox.m_vecMax.x = t_vec3[t_i].x;
		if( _aabbox.m_vecMax.y < t_vec3[t_i].y )
			_aabbox.m_vecMax.y = t_vec3[t_i].y;
		if( _aabbox.m_vecMax.z < t_vec3[t_i].z )
			_aabbox.m_vecMax.z = t_vec3[t_i].z;

		// min vec.
		if( _aabbox.m_vecMin.x > t_vec3[t_i].x )
			_aabbox.m_vecMin.x = t_vec3[t_i].x;
		if( _aabbox.m_vecMin.y > t_vec3[t_i].y )
			_aabbox.m_vecMin.y = t_vec3[t_i].y;
		if( _aabbox.m_vecMin.z > t_vec3[t_i].z )
			_aabbox.m_vecMin.z = t_vec3[t_i].z;
	}

	return;
}

//! ��ǰbbox�ڵĶ����Ƿ�ȫ������aabbox��
bool os_bbox::all_inAABBox( const os_aabbox& _aabbox )
{
	guard;

	if( m_bChanged ) 
		to_worldVertex();

	for( int t_i=0;t_i<8;t_i ++ )
	{
		if( !((os_aabbox&)_aabbox).pt_inBox( m_vecWorldVer[t_i] ) )
			return false;
	}

	return true;

	unguard;
}


// �ཻ����
bool os_bbox::collision( const os_aabbox& _aabbox ) 
{
	guard;

	os_aabbox  t_aabbox;

	// ��bbox��⣬�Ƿ��ཻ		
	to_aabbox( t_aabbox );
	if( !t_aabbox.collision( t_aabbox ) )
		return false;

	osPlane    t_vecPlane[6];
	osVec3D    t_vec3;

	get_boxPlane( t_vecPlane );
	for( int t_i=0;t_i<6;t_i ++ )
	{
		// �ҵ���������������ڵĶ���
		t_vec3.x = ((t_vecPlane[t_i].a < 0.0f) ? _aabbox.m_vecMin.x : _aabbox.m_vecMax.x );
		t_vec3.y = ((t_vecPlane[t_i].b < 0.0f) ? _aabbox.m_vecMin.y : _aabbox.m_vecMax.y );
		t_vec3.z = ((t_vecPlane[t_i].c < 0.0f) ? _aabbox.m_vecMin.z : _aabbox.m_vecMax.z );

		if( osn_mathFunc::distance_V2P( t_vec3,t_vecPlane[t_i] ) < 0.0f )
		{
			// ���_aabboxû����ȫ�İ�Χ��ǰ��os_bbox���򷵻�false.
			if( !all_inAABBox( _aabbox ) )
				return false;
			else
				return true;
		}
	}

	return true;

	unguard;
}

bool os_bbox::collision( const os_bbox& _bbox ) 
{
	guard;

	//osassert( false );
	// ��bbox�Ĳ���.
	
	// just test the 2D (x,z)
	//
	//osassert(m_fBoxRot == 0.0f);

	//// compute the width and length of to rectangle
	//const float t_AWidth	= m_vecWorldVer[1].x - m_vecWorldVer[0].x;
	//const float t_ALen		= m_vecWorldVer[3].z - m_vecWorldVer[0].z;
	//
	//const float t_BWidth	= _bbox.m_vecWorldVer[1].x - _bbox.m_vecWorldVer[0].x;
	//const float t_BLen		= _bbox.m_vecWorldVer[3].z - _bbox.m_vecWorldVer[0].z;

	//// compute the center of two rectangle
	////
	//const float t_ACenter_x = m_vecWorldVer[0].x + t_AWidth / 2.0f;
	//const float t_ACenter_z = m_vecWorldVer[0].z + t_ALen / 2.0f;

	//const float t_BCenter_x = _bbox.m_vecWorldVer[0].x + t_BWidth / 2.0f;
	//const float t_BCenter_z = _bbox.m_vecWorldVer[0].z + t_BLen / 2.0f;

	//return abs(t_ACenter_x - t_BCenter_x) < (t_BWidth + t_AWidth) 
	//		&& abs(t_ACenter_z - t_BCenter_z) < (t_BLen + t_ALen);


	//just using distance to test
	//
	const float t_fDis = 2.25f; // sqrt(2.25) == 1.5;

	const osVec3D& t_dir = m_vec3BoxPos - _bbox.m_vec3BoxPos;

	return osVec3Length(&t_dir) < t_fDis;



	// �پ�ϸ����.

	//return true;
	unguard;
}

bool os_bbox::collision( const os_bsphere& _sphere ) 
{
	guard;

	os_aabbox  t_aabbox;

	// ��bbox��⣬�Ƿ��ཻ		
	to_aabbox( t_aabbox );
	if( !t_aabbox.collision( _sphere ) )
		return false;

	osPlane    t_vecPlane[6];
	float      t_fdis;

	get_boxPlane( t_vecPlane );
	for( int t_i=0;t_i<6;t_i ++ )
	{
		t_fdis = osn_mathFunc::distance_V2P( _sphere.veccen,t_vecPlane[t_i] );
		t_fdis += _sphere.radius;
		if( t_fdis < 0.0f )
			return false;
	}

	return true;

	unguard;
}



bool os_aabbox::collision( const os_bsphere& _sphere )
{
	return AABB_Sphere_Collision( _sphere,*this );

}


void os_bbox::reset_bbox( void )
{
	vecmax = osVec3D( 0.0f,0.0f,0.0f );
	vecmin = vecmax;
	m_vec3BoxPos = vecmax;
	m_fBoxRot = 0.0f;
	m_bChanged = false;

	memset( m_vecWorldVer,0,sizeof( osVec3D )*8 );
}

/** \brief
*  �߶��Ƿ�͵�ǰ��Bounding Box�ཻ��
*
*  \param _res ������ߺ�BoundingBox�ཻ�����ֵΪ���صĽ���ֵ��
*  \param _dis ����ཻ����ֵ���ؿ�ʼ��ͽ���֮��ľ��롣
*  
*/
bool os_bbox::ray_intersect( osVec3D& _start,osVec3D& _dir,float& _dis )
{
	guard;


	osVec3D         t_center,t_end;
	float           t_dis,t_fRealDis;

	osMatrix        t_rotMat;

	osMatrixRotationY( &t_rotMat,m_fBoxRot );

	// 
	// ���bbox�����ĵ㵽ֱ�ߵľ���С��bbox��boundsphere�뾶��С�����ཻ��
	// ���bbox��ת��������ĵ������ת
	t_center = (vecmax+vecmin)/2.0f;
	osVec3Transform( &t_center,&t_center,&t_rotMat );
	t_center += m_vec3BoxPos;

	t_dis = osVec3Length( &(vecmax-vecmin) )/2.0f;
	t_end = _start + _dir*100000.0f;
	t_fRealDis = osn_mathFunc::distance_V2L( t_center,_start,t_end );
	if( t_fRealDis >= t_dis )
		return false;

	//
	// ���̵����Ժ����ߺ�Bounding Box�ཻ�Ŀ����Ժܴ�
	// �����������ÿһ�������ε��ཻ����
	const osVec3D*     t_pt;
	t_pt = get_worldVertexPtr();


	// 
	// ����ʱ����������η����������ߵ������Ƕ�С��180�ȣ�
	// ����Բ��ཻ��
	
	// ��bounding Box�±ߵ����������ν��д���
	float      t_f,t_u,t_v;
	osVec3D    t_normal( 0.0f,-1.0f,0.0f );

	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[1],t_pt[2],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[2],t_pt[3],_dis,t_u,t_v ) )
			return true;
	}

	//
	// y ����������档
	t_normal = osVec3D( 0.0f,1.0f,0.0f );

	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[4],t_pt[7],t_pt[6],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[4],t_pt[6],t_pt[5],_dis,t_u,t_v ) )
			return true;
	}
	
	// 
	// -z����������档
	t_normal = osVec3D( 0.0f,0.0f,-1.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[4],t_pt[5],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[5],t_pt[1],_dis,t_u,t_v ) )
			return true;
	}

	// 
	// z����������档
	t_normal = osVec3D( 0.0f,0.0f,1.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[2],t_pt[6],t_pt[7],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[2],t_pt[7],t_pt[3],_dis,t_u,t_v ) )
			return true;
	}


	// 
	// -x����������档
	t_normal = osVec3D( -1.0f,0.0f,0.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[3],t_pt[7],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[7],t_pt[4],_dis,t_u,t_v ) )
			return true;
	}

	// 
	// x����������档
	t_normal = osVec3D( 1.0f,0.0f,0.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[1],t_pt[5],t_pt[6],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[1],t_pt[6],t_pt[2],_dis,t_u,t_v ) )
			return true;
	}

	//
	// ������ص����ݡ�
	return false;

	unguard;
}

/** \brief
*  �ж�һ����radius�������Ƿ���Bounding box�ཻ��
*  
*  \param osVec3D _start,_end ���ߵĿ�ʼ��ͽ����㡣
*  \param float _radius ���ߵİ뾶
*/
bool os_bbox::ray_intersectWithRadius( osVec3D& _start,osVec3D& _end,float _radius )
{
	guard;

	// �����ݲ�ʵ�֣���Ҫ��ƽ���bounding Box���ཻ�ж����ϲ�����صĴ���

	return true;

	unguard;
}

//! ת��Ϊ��ά�İ�Χ��
void os_bbox::to_vec2Box( osVec2D* _vec )
{
	guard;

	if( m_bChanged ) 
		to_worldVertex();

	osassert( _vec );

	_vec[0].x = m_vecWorldVer[0].x;
	_vec[0].y = m_vecWorldVer[0].z;

	_vec[1].x = m_vecWorldVer[3].x;
	_vec[1].y = m_vecWorldVer[3].z;

	_vec[2].x = m_vecWorldVer[2].x;
	_vec[2].y = m_vecWorldVer[2].z;

	_vec[3].x = m_vecWorldVer[1].x;
	_vec[3].y = m_vecWorldVer[1].z;

	unguard;
}


//! ��һ������ת����ǰ��min��max,���õ�һ���µ�aabbox.
void os_aabbox::get_transNewAABBox( osMatrix& _inMat,os_aabbox& _res )
{
	guard;

	osVec3D   t_vec3Min,t_vec3Max;

	osVec3Transform( &t_vec3Min,&m_vecMin,&_inMat );
	osVec3Transform( &t_vec3Max,&m_vecMax,&_inMat );

	_res.m_vecMax.x = t_vec3Min.x;
	if( _res.m_vecMax.x < t_vec3Max.x )
		_res.m_vecMax.x = t_vec3Max.x;
	_res.m_vecMax.y = t_vec3Min.y;
	if( _res.m_vecMax.y < t_vec3Max.y )
		_res.m_vecMax.y = t_vec3Max.y;
	_res.m_vecMax.z = t_vec3Min.z;
	if( _res.m_vecMax.z < t_vec3Max.z )
		_res.m_vecMax.z = t_vec3Max.z;


	_res.m_vecMin.z = t_vec3Min.z;
	if( _res.m_vecMin.z > t_vec3Max.z )
		_res.m_vecMin.z = t_vec3Max.z;
	_res.m_vecMin.y = t_vec3Min.y;
	if( _res.m_vecMin.y > t_vec3Max.y )
		_res.m_vecMin.y = t_vec3Max.y;
	_res.m_vecMin.x = t_vec3Min.x;
	if( _res.m_vecMin.x > t_vec3Max.x )
		_res.m_vecMin.x = t_vec3Max.x;


	return;

	unguard;
}



/** \brief
*  �߶��Ƿ��bounding Box�ཻ.
*/
bool os_aabbox::ls_intersect( osVec3D& _ls,osVec3D& _le )
{
	guard;

	osVec3D   t_vec3Dir;
	float     t_fDisLs;
	
	t_vec3Dir = _le - _ls;
	t_fDisLs = osVec3Length( &t_vec3Dir );
	osVec3Normalize( &t_vec3Dir,&t_vec3Dir );

	float   t_fDis;
	
	if( !ray_intersect( _ls,t_vec3Dir,t_fDis ) )
		return false;

	// ����Զ���߶εĳ���,Ҳ���ཻ.
	if( t_fDis > t_fDisLs )
		return false;

	// ���������ཻ,��ʼ��Ҳ����aabbox��,���߶β��ཻ.
	if( (t_fDis < 0.0f)&&
		(!pt_inBox( _ls ) ) )
		return false;


	return true;

	unguard;
}


/** \brief
*  �߶��Ƿ�͵�ǰ��Bounding Box�ཻ��
*
*  \param _res ������ߺ�BoundingBox�ཻ�����ֵΪ���صĽ���ֵ��
*  \param _dis ����ཻ����ֵ���ؿ�ʼ��ͽ���֮��ľ��롣
*  
*/
bool os_aabbox::ray_intersect( osVec3D& _start,osVec3D& _dir,float& _dis )
{
	guard;


	osVec3D         t_center,t_end;
	float           t_dis,t_fRealDis;

	// 
	// ���bbox�����ĵ㵽ֱ�ߵľ���С��bbox��boundsphere�뾶��С�����ཻ��
	t_center = (m_vecMax+m_vecMin)/2.0f;
	t_dis = osVec3Length( &(m_vecMax-m_vecMin) )/2.0f;
	t_end = _start + _dir*100000.0f;
	t_fRealDis = osn_mathFunc::distance_V2L( t_center,_start,t_end );
	if( t_fRealDis >= t_dis )
		return false;

	osVec3D     m_vecWorldVer[8];
	m_vecWorldVer[0] = m_vecMin;
	m_vecWorldVer[1] = m_vecMin;m_vecWorldVer[1].x = m_vecMax.x;
	m_vecWorldVer[2] = m_vecMax;m_vecWorldVer[2].y = m_vecMin.y;
	m_vecWorldVer[3] = m_vecMin;m_vecWorldVer[3].z = m_vecMax.z;

	m_vecWorldVer[4] = m_vecMin;m_vecWorldVer[4].y = m_vecMax.y;
	m_vecWorldVer[5] = m_vecMax;m_vecWorldVer[5].z = m_vecMin.z;
	m_vecWorldVer[6] = m_vecMax;
	m_vecWorldVer[7] = m_vecMax;m_vecWorldVer[7].x = m_vecMin.x;

	//
	// ���̵����Ժ����ߺ�Bounding Box�ཻ�Ŀ����Ժܴ�
	// �����������ÿһ�������ε��ཻ����
	const osVec3D*     t_pt;
	osMatrix    t_rotMat;

	t_pt = m_vecWorldVer;
	osMatrixRotationY( &t_rotMat,0.0f );


	// 
	// ����ʱ����������η����������ߵ������Ƕ�С��180�ȣ�
	// ����Բ��ཻ��

	// ��bounding Box�±ߵ����������ν��д���
	float      t_f,t_u,t_v;
	osVec3D    t_normal( 0.0f,-1.0f,0.0f );

	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[1],t_pt[2],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[2],t_pt[3],_dis,t_u,t_v ) )
			return true;
	}

	//
	// y ����������档
	t_normal = osVec3D( 0.0f,1.0f,0.0f );

	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[4],t_pt[7],t_pt[6],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[4],t_pt[6],t_pt[5],_dis,t_u,t_v ) )
			return true;
	}

	// 
	// -z����������档
	t_normal = osVec3D( 0.0f,0.0f,-1.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[4],t_pt[5],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[5],t_pt[1],_dis,t_u,t_v ) )
			return true;
	}

	// 
	// z����������档
	t_normal = osVec3D( 0.0f,0.0f,1.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[2],t_pt[6],t_pt[7],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[2],t_pt[7],t_pt[3],_dis,t_u,t_v ) )
			return true;
	}


	// 
	// -x����������档
	t_normal = osVec3D( -1.0f,0.0f,0.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[3],t_pt[7],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[0],t_pt[7],t_pt[4],_dis,t_u,t_v ) )
			return true;
	}

	// 
	// x����������档
	t_normal = osVec3D( 1.0f,0.0f,0.0f );
	osVec3Transform( &t_normal,&t_normal,&t_rotMat );
	t_f = osVec3Dot( &t_normal,&_dir );
	if( t_f < 0.0f )
	{
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[1],t_pt[5],t_pt[6],_dis,t_u,t_v ) )
			return true;
		if( osn_mathFunc::intersect_tri( _start,_dir,
			t_pt[1],t_pt[6],t_pt[2],_dis,t_u,t_v ) )
			return true;
	}

	//
	// ������ص����ݡ�
	return false;

	unguard;

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
OSENGINE_API bool osn_mathFunc::snap_bbox( os_bbox& _b1,
							os_bbox& _b2,osVec3D& _np2,bool _snap1,bool _snap2,bool _snap3 )
{
	guard;

	osVec3D   t_vec3SnapAxis( 1.0f,0.0f,0.0f );
	osVec3D   t_vec3Box2ToBox1;
	osMatrix  t_sRotMat;
	osVec3D   t_vec3Box1Center,t_vec3Box2Center,t_vec3Box2COffset;
	osVec3D   t_vec3Box1Max,t_vec3Box1Min;
	osVec3D   t_vec3Box2Max,t_vec3Box2Min;

	osassertex( float_equal( _b1.get_bbYRot(),_b2.get_bbYRot() ),
		va( "�������ӵ���ת����һ��,_b1<%f>,_b2<%f>..\n",_b1.get_bbYRot(),_b2.get_bbYRot() ) );
	osMatrixRotationY( &t_sRotMat,_b1.get_bbYRot() );

	//
	// �����������Ӷ����ƺ��ӵ�λ�ý�����ת���ȼ������������ӵ����ĵ�λ�á�
	t_vec3Box1Max = *_b1.get_vecmax();
	t_vec3Box1Min = *_b1.get_vecmin();
	t_vec3Box2Max = *_b2.get_vecmax();
	t_vec3Box2Min = *_b2.get_vecmin();

	t_sRotMat._41 = _b1.get_bbPos()->x;
	t_sRotMat._42 = _b1.get_bbPos()->y;
	t_sRotMat._43 = _b1.get_bbPos()->z;
	osVec3Transform( &t_vec3Box1Max,&t_vec3Box1Max,&t_sRotMat );
	osVec3Transform( &t_vec3Box1Min,&t_vec3Box1Min,&t_sRotMat );

	t_sRotMat._41 = _b2.get_bbPos()->x;
	t_sRotMat._42 = _b2.get_bbPos()->y;
	t_sRotMat._43 = _b2.get_bbPos()->z;
	osVec3Transform( &t_vec3Box2Max,&t_vec3Box2Max,&t_sRotMat );
	osVec3Transform( &t_vec3Box2Min,&t_vec3Box2Min,&t_sRotMat );

	t_vec3Box1Center = (t_vec3Box1Max + t_vec3Box1Min)/2.0f;
	t_vec3Box2Center = (t_vec3Box2Max + t_vec3Box2Min)/2.0f;


	// 
	// ���������������ֵ,���ڽ������ĳ��ȵ���ؼ���
	t_vec3Box1Max = *_b1.get_vecmax();
	t_vec3Box1Min = *_b1.get_vecmin();
	t_vec3Box2Max = *_b2.get_vecmax();
	t_vec3Box2Min = *_b2.get_vecmin();

	t_sRotMat._41 = t_sRotMat._42 = t_sRotMat._43 = 0.0f;
	osVec3Transform( &t_vec3SnapAxis,&t_vec3SnapAxis,&t_sRotMat );

	// �����Ҫ�ƶ����ӵĶ���λ�ú�����λ�õ�ƫ����
	t_vec3Box2COffset = t_vec3Box2Center - *_b2.get_bbPos();


	t_vec3Box2ToBox1 = t_vec3Box2Center - t_vec3Box1Center;
	osVec3Normalize( &t_vec3Box2ToBox1,&t_vec3Box2ToBox1 );
	if( osVec3Dot( &t_vec3SnapAxis,&t_vec3Box2ToBox1 ) < 0.0f )
		t_vec3SnapAxis = -t_vec3SnapAxis;


	// ��һ����εĶ���
	osassertex( _snap1,"������������Ķ���.." );

	float   t_fDis1 = (t_vec3Box1Max.x - t_vec3Box1Min.x)/2.0f;
	float   t_fDis2 = (t_vec3Box2Max.x - t_vec3Box2Min.x)/2.0f;

	osVec3D   t_vec3SnapAxisV1,t_vec3SnapAxisV2,t_vec3Tmp;
	t_vec3SnapAxisV1 = t_vec3Box1Center;
	t_vec3SnapAxisV2 = t_vec3SnapAxis + t_vec3SnapAxisV1;

	osn_mathFunc::project_PtToLine( t_vec3Tmp,t_vec3Box2Center,t_vec3SnapAxisV1,t_vec3SnapAxisV2 );
	_np2 = t_vec3Box1Center + t_vec3SnapAxis*(t_fDis1+t_fDis2);
	_np2 -= t_vec3Tmp;
	_np2 = t_vec3Box2Center + _np2;

	// 
	// �ڶ�����εĶ���:
	if( _snap2 )
	{
		osn_mathFunc::project_PtToLine( t_vec3Tmp,_np2,t_vec3SnapAxisV1,t_vec3SnapAxisV2 );
		_np2 = t_vec3Tmp;
	}

	// ���ĸ߶ȶ���
	if( _snap3 )
	{
		t_fDis1 = (t_vec3Box1Max.y - t_vec3Box1Min.y)/2.0f;
		t_fDis2 = (t_vec3Box2Max.y - t_vec3Box2Min.y)/2.0f;

		t_fDis2 -= t_fDis1;
	
		// ���û�еڶ����Ķ��룬����Ҫ��λ�õĸ߶�Ҳ����.ATTENTION TO FIX:
		if( !_snap2 )
		{
			t_fDis1 = t_vec3Box2Center.y - t_vec3Box1Center.y;
			t_fDis2 += t_fDis1;
		}

		_np2.y += t_fDis2;
	}

	// �������ĵ㵽λ�õ����
	_np2 -= t_vec3Box2COffset;

	return true;

	unguard;
}





OSENGINE_API int FindEnumTalbe(const EnumTable& _table,const char* _name)
{
	guard;
	for (int j =0 ;j < _table.size;j++)
	{
		if (stricmp(_table.table[j].label,_name)==0)
		{
			return _table.table[j].index;
		}
	}
	//osassert(false);
	return -1;
	unguard;
}