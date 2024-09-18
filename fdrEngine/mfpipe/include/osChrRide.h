//===================================================================
/** \file  
* Filename :   osChrRide.h
* Desc     :   
* His      :   Windy create @2005-8-27 18:55:25
*/
//===================================================================
#ifndef __OSCHRRIDE_H__
#define __OSCHRRIDE_H__


#endif //__OSCHRRIDE_H__
class osc_ChrRide
{
public:
	//! 对应的骨骼的索引。
	int         m_iMatrixIdx;
	//! 初始位置。
	osVec3D      m_vec3Center;
public:
	osc_ChrRide();
	~osc_ChrRide();

};
class osc_ChrRideInst
{
public:
	osc_ChrRide * m_ptrRide;
	//!当前位置
	osVec3D      m_vec3Pos;
public:
	void update_poskey( osMatrix& _rotMat );
public:
	osc_ChrRideInst();
	~osc_ChrRideInst();
};