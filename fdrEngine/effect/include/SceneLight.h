/** \file SceneLight.h
\n �������� :   
\author         windy   
\date           2006-6-21 15:26:34*/
#ifndef __SCENELIGHT_H__
#define __SCENELIGHT_H__
#include "../../interface/osinterface.h"

class SceneLightMgr
{
public:
	SceneLightMgr();
	~SceneLightMgr();

	//! ��ʼ��������,
	/*!\param _times ,����ʱ�䳤��
	\param frequency һ����������.
	\param flashTime ÿ������,��ռʱ�� [0-1]֮����
	\param _color ������ɫ*/
	void	start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f));
	//! 
	void	stop();
	void	frame_move(float _timeMs);

	//! ��ǰ�ĳ��������Ƿ��ڴ�״̬�������Ҫ�л������������������ص���������Ч��
	bool    is_sceneLightOn( void  ) { return mIsSceneLight; } 

protected:
private:
	bool	mIsSceneLight;
	// �Ժ���Ϊ��λ
	float	mCurrentSceneLightTime;
	float	mSceneLightLengthTime;
	//!  ��һ���ö���ʱ��.
	float	mInvFrequency;

	float	mFlashTime;

	osColor mFlashColor;
	osColor	mBackUpDirectionColor;
	int		mFlashType;

};

#endif //__SCENELIGHT_H__