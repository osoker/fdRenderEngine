//================================================================================================
/** \file 
 *  Filename: sgdegine.h
 *
 *  His: River get it from sg engine file.
 *
 *  Des: ��õĳ�ͽṹ��һЩ����.����ļ�Ҳ������d3dx������װ��ͷ�ļ�.
 *
 */ 
//================================================================================================

#ifndef SG_DEFINE_H
#define SG_DEFINE_H


# include <iostream>
# include <string>
# include <list>
# include <map>
# include <vector>

# include "../common.h"

# pragma warning( disable : 4786 )
# pragma warning( disable : 4251 )
# pragma warning( disable : 4127 )
# pragma warning( disable : 4100 )

//! OSOK����汾�ĺ궨��.
#define OSOKENGINE_VERSION     0x0010

//! �Ƿ�ֻ�Ƕ�d3d���򵥵İ�װ
#define OS_WRAP_D3D     


//! Բ���ʵĶ���.
#define OS_PI    ((float)  3.141592654f)

/**
 *
 * ��Ԫ��color��Ķ���,ʹ��������Ը�����Ĳ���color.
 *
 */
class COMMON_API sg_ColorQuad
{
    unsigned int Color;

public:
    sg_ColorQuad( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
    sg_ColorQuad( unsigned char r, unsigned char g, unsigned char b );
    unsigned char Red(void);
    unsigned char Green(void);
    unsigned char Blue(void);
    unsigned char Alpha(void);
};





//--------------------------------------------------------------------------------------
//���õ�struct or type.
//--------------------------------------------------------------------------------------
typedef std::string            s_string;
typedef std::list<s_string>     LIST_string;
typedef std::vector<s_string>   VEC_string;
typedef std::list<int>          LIST_int;
typedef std::list<DWORD>        LIST_dword;
typedef std::vector<DWORD>      VEC_dword;
typedef std::vector<WORD>       VEC_word;
typedef std::vector<int>        VEC_int;
typedef std::vector<bool>       VEC_bool;
typedef std::vector<float>      VEC_float;
typedef std::vector<DWORD>      VEC_ptr;
typedef std::vector<BYTE>       VEC_BYTE;





//--------------------------------------------------------------------------------------
//��d3d primitive�ķ�װ
//--------------------------------------------------------------------------------------
# define         POINTLIST     1
# define         LINELIST          2 
# define         LINESTRIP         3 
# define         TRIANGLELIST      4 
# define         TRIANGLESTRIP     5 
# define         TRIANGLEFAN       6 


//--------------------------------------------------------------------------------------
//common used value.
//--------------------------------------------------------------------------------------
# define FLOATBOTTOM                  0.0001f
# define EPSILON                      0.003125f

# define SG_MAX_VALUE                     10000000000
# define SG_MIN_VALUE                    -10000000000



/********************************************************************************************/
//
// �������ݰ�װ����ͷ�ļ�,���ͷ�ļ���װ�˳��õ�d3dx math�⺯��.
#include "sgwrapmath.h"
//
/********************************************************************************************/
typedef std::vector< osMatrix >  VEC_matrix;
typedef std::vector< osVec3D >   VEC_vector3;
typedef std::vector< osVec2D >   VEC_vector2;


//--------------------------------------------------------------------------------------
//
// ���ıȽϺ���.
//
//--------------------------------------------------------------------------------------
/**
 * ����float��ֵ�ıȽ�.
 */
COMMON_API inline bool  float_equal( float _a,float _b )
{
	if( fabs(_a-_b)<EPSILON )
		return true;
	else
		return false;
}
/**
 * ����double��ֵ�ıȽ�.
 */
COMMON_API inline bool  double_equal( double _a,double _b )
{
	if( fabs( _a-_b ) < FLOATBOTTOM )
		return true;
	else
		return false;
}
/**
 * ����vec3������ֵ�ıȽ�.
 */
COMMON_API inline bool  vec3_equal( osVec3D& _a,osVec3D& _b )
{
	if( float_equal( _a.x,_b.x ) && float_equal( _a.y,_b.y ) &&
		float_equal( _a.z,_b.z ) )
		return true;
	else
		return false;
}
/**
 * ����vec2��ֵ�ıȽ�.
 */
COMMON_API inline bool  vec2_equal( osVec2D& _a,osVec2D& _b )
{
	if( float_equal( _a.x ,_b.x ) && float_equal( _a.y,_b.y ) )
		return true;
	else
		return false;
}
/**
 * ����plane��ֵ�ıȽ�.
 */
COMMON_API inline bool plane_equal( osPlane& _a,osPlane& _b )
{
	if( float_equal( _a.a,_b.a ) &&
		float_equal( _a.b,_b.b ) &&
		float_equal( _a.c,_b.c ) &&
		float_equal( _a.d,_b.d ) )
		return true;
	else
		return false;

}

/** \brief
 *  �õ�һ����������һ���Ķ��ٱ���
 *  
 *  �൱������ֵ��/�������
 */
COMMON_API inline int float_div( float _a,float _b )
{
	return int(_a/_b);
}

/** \brief
 *  ��һ��������������һ����������õ���������
 */
COMMON_API inline float float_mod( float _a,float _b )
{
	int   t_i;

	t_i = int(_a/_b);
	return _a-t_i*_b;
}

/** \brief
 *
 */
COMMON_API inline int   round_float( float _tf )
{
	int   t_i = int(_tf);

	if( (_tf - t_i)>= 0.5 )
		t_i ++;

	return t_i;
};



/** \brief
 *  ��һ��������Χ�ڵ��������
 */
COMMON_API inline int getrand_fromintscope( int _min,int _max )
{

	int tmpi;

	if( _max<_min )
		return -1;

	//should be 0 or min?
	if( _min==_max )
		return _min;

	tmpi = _max-_min+1;
	tmpi = rand()%tmpi+_min;

	return tmpi;

}


/** \brief
 *  ��һ����������Χ�ڵõ�һ�����������.
 *
 */
COMMON_API inline float getrand_fromfloatscope( float _min,float _max )
{
	float tmpf;

	if( _min==_max )
		return _min;

	tmpf = _max - _min;
	tmpf *= (rand()%10000+1)/10000.0f;
	tmpf += _min;

	return tmpf;
}


/** \brief
 *  ��һ��С��1�ĸ���������һ��0��1 ����,������ֵ�����С���ݾ���.
 */
COMMON_API inline int   getrand_fromfloat( float _rate )
{
	if( (rand()%1000)<(_rate*1000) )
		return 1;
	else
		return 0;
}


/** \brief
 *  �õ��ǻ������ֵ.
 */
COMMON_API inline BOOL getrand_yesorno( void )
{
	if( rand()%2 )
		return TRUE;
	else
		return FALSE;
}


//! ����һ��string��hashValue.
COMMON_API inline DWORD string_hash(const char* _lpszValue)
{
	DWORD h = 0;
	for(;*_lpszValue;_lpszValue++)
	{
		h = ((h<<5)-h)+(*(unsigned char*)_lpszValue);
	}
	return h;
}

//! ����һ��string��hashValue.
COMMON_API inline DWORD string_hash(const std::string& _szValue)
{
	DWORD h = 0;
	const char* s = _szValue.c_str();
	for(;*s;s++)
	{
		h = ((h<<5)-h)+(*(unsigned char*)s);
	}
	return h;
}







#endif