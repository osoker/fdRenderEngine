//================================================================================================
/** \file 
 *  Filename: oswrapmath.h
 *
 *  His: River get it from os engine file.
 *
 *  Des: 对D3DX数学库的简单的包装,使用这些包装后的函数,以后如果使用新的类库,
 *       可以方便的替换这些类库.
 *
 */ 
//================================================================================================


#ifndef os_WRAPMATH_H
#define os_WRAPMATH_H

# include "d3dx9.h"
# include "d3dx9mesh.h"

typedef D3DXVECTOR2     osVec2D;
typedef D3DXVECTOR3     osVec3D;
typedef D3DXVECTOR4     osVec4D;
typedef D3DXMATRIX      osMatrix;
typedef D3DXQUATERNION  osQuat;
typedef D3DXCOLOR       osColor;
typedef D3DXPLANE       osPlane;
typedef D3DVIEWPORT9    osViewport;

//--------------------------
// 2D Vector
//--------------------------

// COMMON_API inline

COMMON_API inline FLOAT osVec2Length
    ( CONST osVec2D *pV )
{
    return D3DXVec2Length(pV);
}

COMMON_API  inline FLOAT osVec2LengthSq
    ( CONST osVec2D *pV )
{
    return D3DXVec2LengthSq(pV);
}

COMMON_API  inline FLOAT osVec2Dot
    ( CONST osVec2D *pV1, CONST osVec2D *pV2 )
{
    return D3DXVec2Dot(pV1,pV2);
}

// Z component of ((x1,y1,0) cross (x2,y2,0))
COMMON_API  inline FLOAT osVec2CCW
    ( CONST osVec2D *pV1, CONST osVec2D *pV2 )
{
    return D3DXVec2CCW(pV1, pV2);
}

COMMON_API  inline osVec2D* osVec2Add
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pV2 )
{
    return D3DXVec2Add(pOut, pV1, pV2 );
}

COMMON_API  inline osVec2D* osVec2Subtract
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pV2 )
{
    return D3DXVec2Subtract( pOut, pV1, pV2 );
}

// Minimize each component.  x = min(x1, x2), y = min(y1, y2)
COMMON_API  inline osVec2D* osVec2Minimize
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pV2 )
{
    return D3DXVec2Minimize( pOut, pV1, pV2 );
}

// Maximize each component.  x = max(x1, x2), y = max(y1, y2)
 COMMON_API inline osVec2D* osVec2Maximize
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pV2 )
{
    return D3DXVec2Maximize( pOut, pV1, pV2 );
}

 COMMON_API inline osVec2D* osVec2Scale
    ( osVec2D *pOut, CONST osVec2D *pV, FLOAT s )
{
    return D3DXVec2Scale( pOut, pV, s );
}

// Linear interpolation. V1 + s(V2-V1)
 COMMON_API inline osVec2D* osVec2Lerp
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pV2,
      FLOAT s )
{
    return D3DXVec2Lerp( pOut, pV1, pV2, s );
}

 COMMON_API inline osVec2D* WINAPI osVec2Normalize
    ( osVec2D *pOut, CONST osVec2D *pV )
{
    return D3DXVec2Normalize( pOut, pV );
}

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
 COMMON_API inline osVec2D* WINAPI osVec2Hermite
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pT1,
      CONST osVec2D *pV2, CONST osVec2D *pT2, FLOAT s )
{
    return D3DXVec2Hermite( pOut, pV1, pT1, pV2, pT2, s );
}

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
 COMMON_API inline osVec2D* WINAPI osVec2CatmullRom
    ( osVec2D *pOut, CONST osVec2D *pV0, CONST osVec2D *pV1,
      CONST osVec2D *pV2, CONST osVec2D *pV3, FLOAT s )
{
    return D3DXVec2CatmullRom( pOut, pV0, pV1, pV2, pV3, s );
}

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
 COMMON_API inline osVec2D* WINAPI osVec2BaryCentric
    ( osVec2D *pOut, CONST osVec2D *pV1, CONST osVec2D *pV2,
      osVec2D *pV3, FLOAT f, FLOAT g)
{
    return D3DXVec2BaryCentric( pOut, pV1, pV2, pV3, f, g );
}

// Transform (x, y, 0, 1) by matrix.
 COMMON_API inline osVec4D* WINAPI osVec2Transform
    ( osVec4D *pOut, CONST osVec2D *pV, CONST osMatrix *pM )
{
    return D3DXVec2Transform( pOut, pV, pM );
}

// Transform (x, y, 0, 1) by matrix, project result back into w=1.
 COMMON_API inline osVec2D* WINAPI osVec2TransformCoord
    ( osVec2D *pOut, CONST osVec2D *pV, CONST osMatrix *pM )
{
    return D3DXVec2TransformCoord( pOut, pV, pM );
}

// Transform (x, y, 0, 0) by matrix.
 COMMON_API inline osVec2D* WINAPI osVec2TransformNormal
    ( osVec2D *pOut, CONST osVec2D *pV, CONST osMatrix *pM )
{
    return D3DXVec2TransformNormal( pOut, pV, pM );
}


//--------------------------
// 3D Vector
//--------------------------

// COMMON_API inline

 COMMON_API inline FLOAT osVec3Length
    ( CONST osVec3D *pV )
{
    return D3DXVec3Length( pV );
}

 COMMON_API inline FLOAT osVec3LengthSq
    ( CONST osVec3D *pV )
{
    return D3DXVec3LengthSq( pV );
}

 COMMON_API inline FLOAT osVec3Dot
    ( CONST osVec3D *pV1, CONST osVec3D *pV2 )
{
    return D3DXVec3Dot( pV1, pV2 );
}

 COMMON_API inline osVec3D* osVec3Cross
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2 )
{
    return D3DXVec3Cross( pOut, pV1, pV2 );
}

 COMMON_API inline osVec3D* osVec3Add
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2 )
{
    return D3DXVec3Add( pOut, pV1, pV2 );
}

 COMMON_API inline osVec3D* osVec3Subtract
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2 )
{
    return D3DXVec3Subtract( pOut, pV1, pV2 );
}

// Minimize each component.  x = min(x1, x2), y = min(y1, y2), ...
 COMMON_API inline osVec3D* osVec3Minimize
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2 )
{
    return D3DXVec3Minimize( pOut, pV1, pV2 );
}

// Maximize each component.  x = max(x1, x2), y = max(y1, y2), ...
 COMMON_API inline osVec3D* osVec3Maximize
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2 )
{
    return D3DXVec3Maximize( pOut, pV1, pV2 );
}

 COMMON_API inline osVec3D* osVec3Scale
    ( osVec3D *pOut, CONST osVec3D *pV, FLOAT s)
{
    return D3DXVec3Scale( pOut, pV, s );
}

// Linear interpolation. V1 + s(V2-V1)
 COMMON_API inline osVec3D* osVec3Lerp
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2,
      FLOAT s )
{
    return D3DXVec3Lerp( pOut, pV1, pV2, s );
}


 COMMON_API inline osVec3D* WINAPI osVec3Normalize
    ( osVec3D *pOut, CONST osVec3D *pV )
{
    return D3DXVec3Normalize( pOut, pV );
}

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
 COMMON_API inline osVec3D* WINAPI osVec3Hermite
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pT1,
      CONST osVec3D *pV2, CONST osVec3D *pT2, FLOAT s )
{
    return D3DXVec3Hermite( pOut, pV1, pT1, pV2, pT2, s );
}

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
 COMMON_API inline osVec3D* WINAPI osVec3CatmullRom
    ( osVec3D *pOut, CONST osVec3D *pV0, CONST osVec3D *pV1,
      CONST osVec3D *pV2, CONST osVec3D *pV3, FLOAT s )
{
    return D3DXVec3CatmullRom( pOut, pV0, pV1, pV2, pV3, s );
}

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
 COMMON_API inline osVec3D* WINAPI osVec3BaryCentric
    ( osVec3D *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2,
      CONST osVec3D *pV3, FLOAT f, FLOAT g)
{
    return D3DXVec3BaryCentric( pOut, pV1, pV2, pV3, f, g );
}

// Transform (x, y, z, 1) by matrix.
 COMMON_API inline osVec4D* WINAPI osVec3Transform
    ( osVec4D *pOut, CONST osVec3D *pV, CONST osMatrix *pM )
{
    return D3DXVec3Transform( pOut, pV, pM );
}

//river added to expand d3dx lib.
 COMMON_API inline osVec3D* WINAPI osVec3Transform
     ( osVec3D *pOut,CONST osVec3D *pV, CONST osMatrix *pM )
{
	osVec4D      tmpv4;

	D3DXVec3Transform( &tmpv4,pV,pM );
	(*pOut).x = tmpv4.x;
	(*pOut).y = tmpv4.y;
	(*pOut).z = tmpv4.z;

	return pOut;
}

// Transform (x, y, z, 1) by matrix, project result back into w=1.
 COMMON_API inline osVec3D* WINAPI osVec3TransformCoord
    ( osVec3D *pOut, CONST osVec3D *pV, CONST osMatrix *pM )
{
    return D3DXVec3TransformCoord( pOut, pV, pM );
}

// Transform (x, y, z, 0) by matrix.  If you transforming a normal by a 
// non-affine matrix, the matrix you pass to this function should be the 
// transpose of the inverse of the matrix you would use to transform a coord.
 COMMON_API inline osVec3D* WINAPI osVec3TransformNormal
    ( osVec3D *pOut, CONST osVec3D *pV, CONST osMatrix *pM )
{
    return D3DXVec3TransformNormal( pOut, pV, pM );
}

// Project vector from object space into screen space
 COMMON_API inline osVec3D* WINAPI osVec3Project
    ( osVec3D *pOut, CONST osVec3D *pV, CONST D3DVIEWPORT9 *pViewport,
      CONST osMatrix *pProjection, CONST osMatrix *pView, CONST osMatrix *pWorld)
{
    return D3DXVec3Project( pOut, pV, pViewport, pProjection, pView, pWorld );
}

// Project vector from screen space into object space
 COMMON_API inline osVec3D* WINAPI osVec3Unproject
    ( osVec3D *pOut, CONST osVec3D *pV, CONST D3DVIEWPORT9 *pViewport,
      CONST osMatrix *pProjection, CONST osMatrix *pView, CONST osMatrix *pWorld)
{
    return D3DXVec3Unproject( pOut, pV, pViewport, pProjection, pView, pWorld );
}


//--------------------------
// 4D Vector
//--------------------------

// COMMON_API inline

 COMMON_API inline FLOAT osVec4Length
    ( CONST osVec4D *pV )
{
    return D3DXVec4Length( pV );
}

 COMMON_API inline FLOAT osVec4LengthSq
    ( CONST osVec4D *pV )
{
    return D3DXVec4LengthSq( pV );
}

 COMMON_API inline FLOAT osVec4Dot
    ( CONST osVec4D *pV1, CONST osVec4D *pV2 )
{
    return D3DXVec4Dot( pV1, pV2 );
}

 COMMON_API inline osVec4D* osVec4Add
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2)
{
    return D3DXVec4Add( pOut, pV1, pV2 );
}

 COMMON_API inline osVec4D* osVec4Subtract
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2)
{
    return D3DXVec4Subtract( pOut, pV1, pV2 );
}

// Minimize each component.  x = min(x1, x2), y = min(y1, y2), ...
 COMMON_API inline osVec4D* osVec4Minimize
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2)
{
    return D3DXVec4Minimize( pOut, pV1, pV2 );
}

// Maximize each component.  x = max(x1, x2), y = max(y1, y2), ...
 COMMON_API inline osVec4D* osVec4Maximize
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2)
{
    return D3DXVec4Maximize( pOut, pV1, pV2 );
}

 COMMON_API inline osVec4D* osVec4Scale
    ( osVec4D *pOut, CONST osVec4D *pV, FLOAT s)
{
    return D3DXVec4Scale( pOut, pV, s );
}

// Linear interpolation. V1 + s(V2-V1)
 COMMON_API inline osVec4D* osVec4Lerp
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2,
      FLOAT s )
{
    return D3DXVec4Lerp( pOut, pV1, pV2, s );
}

// Cross-product in 4 dimensions.
 COMMON_API inline osVec4D* WINAPI osVec4Cross
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2,
      CONST osVec4D *pV3)
{
    return D3DXVec4Cross( pOut, pV1, pV2, pV3 );
}

 COMMON_API inline osVec4D* WINAPI osVec4Normalize
    ( osVec4D *pOut, CONST osVec4D *pV )
{
    return D3DXVec4Normalize( pOut, pV );
}

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
 COMMON_API inline osVec4D* WINAPI osVec4Hermite
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pT1,
      CONST osVec4D *pV2, CONST osVec4D *pT2, FLOAT s )
{
    return D3DXVec4Hermite( pOut, pV1, pT1, pV2, pT2, s );
}

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
 COMMON_API inline osVec4D* WINAPI osVec4CatmullRom
    ( osVec4D *pOut, CONST osVec4D *pV0, CONST osVec4D *pV1,
      CONST osVec4D *pV2, CONST osVec4D *pV3, FLOAT s )
{
    return D3DXVec4CatmullRom( pOut, pV0, pV1, pV2, pV3, s );
}

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
 COMMON_API inline osVec4D* WINAPI osVec4BaryCentric
    ( osVec4D *pOut, CONST osVec4D *pV1, CONST osVec4D *pV2,
      CONST osVec4D *pV3, FLOAT f, FLOAT g)
{
    return D3DXVec4BaryCentric( pOut, pV1, pV2, pV3, f, g );
}

// Transform vector by matrix.
 COMMON_API inline osVec4D* WINAPI osVec4Transform
    ( osVec4D *pOut, CONST osVec4D *pV, CONST osMatrix *pM )
{
    return D3DXVec4Transform( pOut, pV, pM );
}


//--------------------------
// 4D Matrix
//--------------------------

// COMMON_API inline

 COMMON_API inline osMatrix* osMatrixIdentity
    ( osMatrix *pOut )
{
    return D3DXMatrixIdentity( pOut );
}

 COMMON_API inline BOOL osMatrixIsIdentity
    ( CONST osMatrix *pM )
{
    return D3DXMatrixIsIdentity( pM );
}


 COMMON_API inline FLOAT WINAPI osMatrixfDeterminant
    ( CONST osMatrix *pM )
{
    return D3DXMatrixDeterminant( pM );
}

// Matrix multiplication.  The result represents the transformation M2
// followed by the transformation M1.  (Out = M1 * M2)
 COMMON_API inline osMatrix* WINAPI osMatrixMultiply
    ( osMatrix *pOut, CONST osMatrix *pM1, CONST osMatrix *pM2 )
{
    return D3DXMatrixMultiply( pOut, pM1, pM2 );
}

 COMMON_API inline osMatrix* WINAPI osMatrixTranspose
    ( osMatrix *pOut, CONST osMatrix *pM )
{
    return D3DXMatrixTranspose( pOut, pM );
}

// Calculate inverse of matrix.  Inversion my fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
 COMMON_API inline osMatrix* WINAPI osMatrixInverse
    ( osMatrix *pOut, FLOAT *pDeterminant, CONST osMatrix *pM )
{
    return D3DXMatrixInverse( pOut, pDeterminant, pM );
}

// Build a matrix which scales by (sx, sy, sz)
 COMMON_API inline osMatrix* WINAPI osMatrixScaling
    ( osMatrix *pOut, FLOAT sx, FLOAT sy, FLOAT sz )
{
    return D3DXMatrixScaling( pOut, sx, sy, sz );
}

// Build a matrix which translates by (x, y, z)
 COMMON_API inline osMatrix* WINAPI osMatrixTranslation
    ( osMatrix *pOut, FLOAT x, FLOAT y, FLOAT z )
{
    return D3DXMatrixTranslation( pOut, x, y, z );
}

// Build a matrix which rotates around the X axis
 COMMON_API inline osMatrix* WINAPI osMatrixRotationX
    ( osMatrix *pOut, FLOAT Angle )
{
    return D3DXMatrixRotationX( pOut, Angle );
}

// Build a matrix which rotates around the Y axis
 COMMON_API inline osMatrix* WINAPI osMatrixRotationY
    ( osMatrix *pOut, FLOAT Angle )
{
    return D3DXMatrixRotationY( pOut, Angle );
}

// Build a matrix which rotates around the Z axis
 COMMON_API inline osMatrix* WINAPI osMatrixRotationZ
    ( osMatrix *pOut, FLOAT Angle )
{
    return D3DXMatrixRotationZ( pOut, Angle );
}

// Build a matrix which rotates around an arbitrary axis
 COMMON_API inline osMatrix* WINAPI osMatrixRotationAxis
    ( osMatrix *pOut, CONST osVec3D *pV, FLOAT Angle )
{
    return D3DXMatrixRotationAxis( pOut, pV, Angle );
}

// Build a matrix from a quaternion
 COMMON_API inline osMatrix* WINAPI osMatrixRotationQuaternion
    ( osMatrix *pOut, CONST osQuat *pQ)
{
    return D3DXMatrixRotationQuaternion( pOut, pQ );
}

// Yaw around the Y axis, a pitch around the X axis,
// and a roll around the Z axis.
 COMMON_API inline osMatrix* WINAPI osMatrixRotationYawPitchRoll
    ( osMatrix *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll )
{
    return D3DXMatrixRotationYawPitchRoll( pOut, Yaw, Pitch, Roll );
}


// Build transformation matrix.  NULL arguments are treated as identity.
// Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
 COMMON_API inline osMatrix* WINAPI osMatrixTransformation
    ( osMatrix *pOut, CONST osVec3D *pScalingCenter,
      CONST osQuat *pScalingRotation, CONST osVec3D *pScaling,
      CONST osVec3D *pRotationCenter, CONST osQuat *pRotation,
      CONST osVec3D *pTranslation)
{
    return D3DXMatrixTransformation( pOut, pScalingCenter, pScalingRotation, pScaling, 
        pRotationCenter, pRotation, pTranslation );
}

// Build affine transformation matrix.  NULL arguments are treated as identity.
// Mout = Ms * Mrc-1 * Mr * Mrc * Mt
 COMMON_API inline osMatrix* WINAPI osMatrixAffineTransformation
    ( osMatrix *pOut, FLOAT Scaling, CONST osVec3D *pRotationCenter,
      CONST osQuat *pRotation, CONST osVec3D *pTranslation)
{
    return D3DXMatrixAffineTransformation( pOut, Scaling, pRotationCenter, pRotation, pTranslation );
}

// Build a lookat matrix. (right-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixLookAtRH
    ( osMatrix *pOut, CONST osVec3D *pEye, CONST osVec3D *pAt,
      CONST osVec3D *pUp )
{
    return D3DXMatrixLookAtRH( pOut, pEye, pAt, pUp );
}

// Build a lookat matrix. (left-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixLookAtLH
    ( osMatrix *pOut, CONST osVec3D *pEye, CONST osVec3D *pAt,
      CONST osVec3D *pUp )
{
    return D3DXMatrixLookAtLH( pOut, pEye, pAt, pUp );
}

// Build a perspective projection matrix. (right-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixPerspectiveRH
    ( osMatrix *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf )
{
    return D3DXMatrixPerspectiveRH( pOut, w, h, zn, zf );
}

// Build a perspective projection matrix. (left-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixPerspectiveLH
    ( osMatrix *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf )
{
    return D3DXMatrixPerspectiveLH( pOut, w, h, zn, zf );
}

// Build a perspective projection matrix. (right-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixPerspectiveFovRH
    ( osMatrix *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf )
{
    return D3DXMatrixPerspectiveFovRH( pOut, fovy, Aspect, zn, zf );
}

// Build a perspective projection matrix. (left-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixPerspectiveFovLH
    ( osMatrix *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf )
{
    return D3DXMatrixPerspectiveFovLH( pOut, fovy, Aspect, zn, zf );
}

// Build a perspective projection matrix. (right-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixPerspectiveOffCenterRH
    ( osMatrix *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf )
{
    return D3DXMatrixPerspectiveOffCenterRH( pOut, l, r, b, t, zn, zf );
}

// Build a perspective projection matrix. (left-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixPerspectiveOffCenterLH
    ( osMatrix *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf )
{
    return D3DXMatrixPerspectiveOffCenterLH( pOut, l, r, b, t, zn, zf );
}

// Build an ortho projection matrix. (right-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixOrthoRH
    ( osMatrix *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf )
{
    return D3DXMatrixOrthoRH( pOut, w, h, zn, zf );
}

// Build an ortho projection matrix. (left-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixOrthoLH
    ( osMatrix *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf )
{
    return D3DXMatrixOrthoLH( pOut, w, h, zn, zf );
}

// Build an ortho projection matrix. (right-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixOrthoOffCenterRH
    ( osMatrix *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf )
{
    return D3DXMatrixOrthoOffCenterRH( pOut, l, r, b, t, zn, zf );
}

// Build an ortho projection matrix. (left-handed)
 COMMON_API inline osMatrix* WINAPI osMatrixOrthoOffCenterLH
    ( osMatrix *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf )
{
    return D3DXMatrixOrthoOffCenterLH( pOut, l, r, b, t, zn, zf );
}

// Build a matrix which flattens geometry into a plane, as if casting
// a shadow from a light.
 COMMON_API inline osMatrix* WINAPI osMatrixShadow
    ( osMatrix *pOut, CONST osVec4D *pLight,
      CONST osPlane *pPlane )
{
    return D3DXMatrixShadow( pOut, pLight, pPlane );
}

// Build a matrix which reflects the coordinate system about a plane
 COMMON_API inline osMatrix* WINAPI osMatrixReflect
    ( osMatrix *pOut, CONST osPlane *pPlane )
{
    return D3DXMatrixReflect( pOut, pPlane );
}


//--------------------------
// Quaternion
//--------------------------

// COMMON_API inline

 COMMON_API inline FLOAT osQuaternionLength
    ( CONST osQuat *pQ )
{
    return D3DXQuaternionLength( pQ );
}

// Length squared, or "norm"
 COMMON_API inline FLOAT osQuaternionLengthSq
    ( CONST osQuat *pQ )
{
    return D3DXQuaternionLengthSq( pQ );
}

 COMMON_API inline FLOAT osQuaternionDot
    ( CONST osQuat *pQ1, CONST osQuat *pQ2 )
{
    return D3DXQuaternionDot( pQ1, pQ2 );
}

// (0, 0, 0, 1)
 COMMON_API inline osQuat* osQuaternionIdentity
    ( osQuat *pOut )
{
    return D3DXQuaternionIdentity( pOut );
}

 COMMON_API inline BOOL osQuaternionIsIdentity
    ( CONST osQuat *pQ )
{
    return D3DXQuaternionIsIdentity( pQ );
}

// (-x, -y, -z, w)
 COMMON_API inline osQuat* osQuaternionConjugate
    ( osQuat *pOut, CONST osQuat *pQ )
{
    return D3DXQuaternionConjugate( pOut, pQ );
}


// Compute a quaternin's axis and angle of rotation. Expects unit quaternions.
 COMMON_API inline void WINAPI osQuaternionToAxisAngle
    ( CONST osQuat *pQ, osVec3D *pAxis, FLOAT *pAngle )
{
    D3DXQuaternionToAxisAngle( pQ, pAxis, pAngle );
}

// Build a quaternion from a rotation matrix.
 COMMON_API inline osQuat* WINAPI osQuaternionRotationMatrix
    ( osQuat *pOut, CONST osMatrix *pM)
{
    return D3DXQuaternionRotationMatrix( pOut, pM );
}

// Rotation about arbitrary axis.
 COMMON_API inline osQuat* WINAPI osQuaternionRotationAxis
    ( osQuat *pOut, CONST osVec3D *pV, FLOAT Angle )
{
    return D3DXQuaternionRotationAxis( pOut, pV, Angle );
}

// Yaw around the Y axis, a pitch around the X axis,
// and a roll around the Z axis.
 COMMON_API inline osQuat* WINAPI osQuaternionRotationYawPitchRoll
    ( osQuat *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll )
{
    return D3DXQuaternionRotationYawPitchRoll( pOut, Yaw, Pitch, Roll );
}

// Quaternion multiplication.  The result represents the rotation Q2
// followed by the rotation Q1.  (Out = Q2 * Q1)
 COMMON_API inline osQuat* WINAPI osQuaternionMultiply
    ( osQuat *pOut, CONST osQuat *pQ1,
      CONST osQuat *pQ2 )
{
    return D3DXQuaternionMultiply( pOut, pQ1, pQ2 );
}

 COMMON_API inline osQuat* WINAPI osQuaternionNormalize
    ( osQuat *pOut, CONST osQuat *pQ )
{
    return D3DXQuaternionNormalize( pOut, pQ );
}

// Conjugate and re-norm
 COMMON_API inline osQuat* WINAPI osQuaternionInverse
    ( osQuat *pOut, CONST osQuat *pQ )
{
    return D3DXQuaternionInverse( pOut, pQ );
}

// Expects unit quaternions.
// if q = (cos(theta), sin(theta) * v); ln(q) = (0, theta * v)
 COMMON_API inline osQuat* WINAPI osQuaternionLn
    ( osQuat *pOut, CONST osQuat *pQ )
{
    return D3DXQuaternionLn( pOut, pQ );
}

// Expects pure quaternions. (w == 0)  w is ignored in calculation.
// if q = (0, theta * v); exp(q) = (cos(theta), sin(theta) * v)
 COMMON_API inline osQuat* WINAPI osQuaternionExp
    ( osQuat *pOut, CONST osQuat *pQ )
{
    return D3DXQuaternionExp( pOut, pQ );
}

// Spherical linear interpolation between Q1 (s == 0) and Q2 (s == 1).
// Expects unit quaternions.
 COMMON_API inline osQuat* WINAPI osQuaternionSlerp
    ( osQuat *pOut, CONST osQuat *pQ1,
      CONST osQuat *pQ2, FLOAT t )
{
    return D3DXQuaternionSlerp( pOut, pQ1, pQ2, t );
}

// Spherical quadrangle interpolation.
// Slerp(Slerp(Q1, Q4, t), Slerp(Q2, Q3, t), 2t(1-t))
 COMMON_API inline osQuat* WINAPI osQuaternionSquad
    ( osQuat *pOut, CONST osQuat *pQ1,
      CONST osQuat *pQ2, CONST osQuat *pQ3,
      CONST osQuat *pQ4, FLOAT t )
{
    return D3DXQuaternionSquad( pOut, pQ1, pQ2, pQ3, pQ4, t );
}

// Slerp(Slerp(Q1, Q2, f+g), Slerp(Q1, Q3, f+g), g/(f+g))
 COMMON_API inline osQuat* WINAPI osQuaternionBaryCentric
    ( osQuat *pOut, CONST osQuat *pQ1,
      CONST osQuat *pQ2, CONST osQuat *pQ3,
      FLOAT f, FLOAT g )
{
    return D3DXQuaternionBaryCentric( pOut, pQ1, pQ2, pQ3, f, g );
}


//--------------------------
// Plane
//--------------------------

// COMMON_API inline

// ax + by + cz + dw
 COMMON_API inline FLOAT osPlaneDot
    ( CONST osPlane *pP, CONST osVec4D *pV)
{
    return D3DXPlaneDot( pP, pV );
}

// ax + by + cz + d
 COMMON_API inline FLOAT osPlaneDotCoord
    ( CONST osPlane *pP, CONST osVec3D *pV)
{
    return D3DXPlaneDotCoord( pP, pV );
}

// ax + by + cz
 COMMON_API inline FLOAT osPlaneDotNormal
    ( CONST osPlane *pP, CONST osVec3D *pV)
{
    return D3DXPlaneDotNormal( pP, pV );
}

// Normalize plane (so that |a,b,c| == 1)
 COMMON_API inline osPlane* WINAPI osPlaneNormalize
    ( osPlane *pOut, CONST osPlane *pP)
{
    return D3DXPlaneNormalize( pOut, pP );
}

// Find the intersection between a plane and a line.  If the line is
// parallel to the plane, NULL is returned.
 COMMON_API inline osVec3D* WINAPI osPlaneIntersectLine
    ( osVec3D *pOut, CONST osPlane *pP, CONST osVec3D *pV1,
      CONST osVec3D *pV2)
{
    return D3DXPlaneIntersectLine( pOut, pP, pV1, pV2 );
}

// Construct a plane from a point and a normal
 COMMON_API inline osPlane* WINAPI osPlaneFromPointNormal
    ( osPlane *pOut, CONST osVec3D *pPoint, CONST osVec3D *pNormal)
{
    return D3DXPlaneFromPointNormal( pOut, pPoint, pNormal );
}

// Construct a plane from 3 points
 COMMON_API inline osPlane* WINAPI osPlaneFromPoints
    ( osPlane *pOut, CONST osVec3D *pV1, CONST osVec3D *pV2,
      CONST osVec3D *pV3)
{
    return D3DXPlaneFromPoints( pOut, pV1, pV2, pV3 );
}

// Transform a plane by a matrix.  The vector (a,b,c) must be normal.
// M must be an affine transform.
 COMMON_API inline osPlane* WINAPI osPlaneTransform
    ( osPlane *pOut, CONST osPlane *pP, CONST osMatrix *pM )

{
    return D3DXPlaneTransform( pOut, pP, pM );
}

//--------------------------
// Color
//--------------------------

// COMMON_API inline

// (1-r, 1-g, 1-b, a)
 COMMON_API inline osColor* osColorNegative
    (osColor *pOut, CONST osColor *pC)
{
    return D3DXColorNegative( pOut, pC );
}

 COMMON_API inline osColor* osColorAdd
    (osColor *pOut, CONST osColor *pC1, CONST osColor *pC2)
{
    return D3DXColorAdd( pOut, pC1, pC2 );
}

 COMMON_API inline osColor* osColorSubtract
    (osColor *pOut, CONST osColor *pC1, CONST osColor *pC2)
{
    return D3DXColorSubtract( pOut, pC1, pC2 );
}

 COMMON_API inline osColor* osColorScale
    (osColor *pOut, CONST osColor *pC, FLOAT s)
{
    return D3DXColorScale( pOut, pC, s );
}

// (r1*r2, g1*g2, b1*b2, a1*a2)
 COMMON_API inline osColor* osColorModulate
    (osColor *pOut, CONST osColor *pC1, CONST osColor *pC2)
{
    return D3DXColorModulate( pOut, pC1, pC2 );
}

// Linear interpolation of r,g,b, and a. C1 + s(C2-C1)
 COMMON_API inline osColor* osColorLerp
    (osColor *pOut, CONST osColor *pC1, CONST osColor *pC2, FLOAT s)
{
    return D3DXColorLerp( pOut, pC1, pC2, s );
}

// Interpolate r,g,b between desaturated color and color.
// DesaturatedColor + s(Color - DesaturatedColor)
 COMMON_API inline osColor* WINAPI osColorAdjustSaturation
    (osColor *pOut, CONST osColor *pC, FLOAT s)
{
    return D3DXColorAdjustSaturation( pOut, pC, s );
}

// Interpolate r,g,b between 50% grey and color.  Grey + s(Color - Grey)
 COMMON_API inline osColor* WINAPI osColorAdjustContrast
    (osColor *pOut, CONST osColor *pC, FLOAT c)
{
    return D3DXColorAdjustContrast( pOut, pC, c );
}


//about save surface to file.
 COMMON_API inline HRESULT osSaveSurfaceToFile(LPCTSTR              pDestFile,
								   D3DXIMAGE_FILEFORMAT DestFormat,
								   LPDIRECT3DSURFACE9   pSrcSurface,
								   CONST PALETTEENTRY*  pSrcPalette,
								   CONST RECT*          pSrcRect )
{
	return D3DXSaveSurfaceToFile( pDestFile,DestFormat,pSrcSurface,pSrcPalette,pSrcRect );
}


#endif
