//===================================================================
/** \file  
*  Filename: CameraCtrl.h
*  Desc:   ������Ƶ�ʵ���ļ�
*  His:      ���� @ 1/12 2004 16:18
*/
//================================================================
#pragma once

class CCameraCtrl
{
public:
	CCameraCtrl(void);
	~CCameraCtrl(void);

public:
	//������һ��camera
	bool Create(HWND _hwnd);

	//!���ø���Ķ���
	void SetFollowObject(I_Character   *_pChar, osVec3D* _pVerDist);

	//!���������λ��
	void Update();

	//!������������֮��ļ��
	void ModifyDist(osVec3D *_pVerDist);
public:
	//!camera�ӿ�
	I_camera			*m_pCamera;
	//!�ӵ����Ķ���
	I_Character			*m_pChar;
	//!��ʼ�������Ҫ������
	os_cameraInit		m_sCameraData;
	//������͸�������ľ���
	osVec3D				m_VerDist;
	//!�۾�����λ��
	osVec3D				m_VerLookat;
};
