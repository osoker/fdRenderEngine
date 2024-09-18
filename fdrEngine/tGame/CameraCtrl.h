//===================================================================
/** \file  
*  Filename: CameraCtrl.h
*  Desc:   相机控制的实现文件
*  His:      王凯 @ 1/12 2004 16:18
*/
//================================================================
#pragma once

class CCameraCtrl
{
public:
	CCameraCtrl(void);
	~CCameraCtrl(void);

public:
	//！创建一个camera
	bool Create(HWND _hwnd);

	//!设置跟随的对象
	void SetFollowObject(I_Character   *_pChar, osVec3D* _pVerDist);

	//!更新相机的位置
	void Update();

	//!调整相机与对象之间的间距
	void ModifyDist(osVec3D *_pVerDist);
public:
	//!camera接口
	I_camera			*m_pCamera;
	//!视点跟随的对象
	I_Character			*m_pChar;
	//!初始化相机需要的数据
	os_cameraInit		m_sCameraData;
	//！相机和跟随物体的距离
	osVec3D				m_VerDist;
	//!眼睛看的位置
	osVec3D				m_VerLookat;
};
