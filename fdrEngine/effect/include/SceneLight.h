/** \file SceneLight.h
\n 功能描述 :   
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

	//! 开始播放闪电,
	/*!\param _times ,闪电时间长度
	\param frequency 一秒钟闪几次.
	\param flashTime 每次闪电,所占时间 [0-1]之间数
	\param _color 闪电颜色*/
	void	start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f));
	//! 
	void	stop();
	void	frame_move(float _timeMs);

	//! 当前的场景闪电是否处于打开状态，如果是要切换场景的天气，则必须关掉场景闪电效果
	bool    is_sceneLightOn( void  ) { return mIsSceneLight; } 

protected:
private:
	bool	mIsSceneLight;
	// 以毫秒为单位
	float	mCurrentSceneLightTime;
	float	mSceneLightLengthTime;
	//!  闪一次用多少时间.
	float	mInvFrequency;

	float	mFlashTime;

	osColor mFlashColor;
	osColor	mBackUpDirectionColor;
	int		mFlashType;

};

#endif //__SCENELIGHT_H__