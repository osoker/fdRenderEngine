/** \file SceneLight.cpp
\n π¶ƒ‹√Ë ˆ :   
\author         windy   
\date           2006-6-21 15:26:39*/
# include "stdafx.h"
#include "../include/SceneLight.h"

SceneLightMgr::SceneLightMgr()
{
	mBackUpDirectionColor.r = g_fDirectLR;
	mBackUpDirectionColor.g = g_fDirectLG;
	mBackUpDirectionColor.b = g_fDirectLB;
	mIsSceneLight = false;
}
SceneLightMgr::~SceneLightMgr()
{
}
void SceneLightMgr::start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color/*= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f)*/)
{
	mBackUpDirectionColor.r = g_fDirectLR;
	mBackUpDirectionColor.g = g_fDirectLG;
	mBackUpDirectionColor.b = g_fDirectLB;

	mSceneLightLengthTime = _times;
	mCurrentSceneLightTime = 0.0f;

	osassert(frequency);
	mInvFrequency = 1.0f/frequency;

	mFlashTime = flashTime;
	mIsSceneLight = true;
	mFlashColor = _color;
	mFlashType = flashtype;

}

void SceneLightMgr::frame_move(float _timeMs)
{
	static bool renderLight = false;
	static float renderTime = 0.0f;
	if (mIsSceneLight)
	{
		mCurrentSceneLightTime += _timeMs;
		if (mSceneLightLengthTime==-1.0f||mCurrentSceneLightTime<mSceneLightLengthTime)
		{
			
			float CurrentTime = fmod(mCurrentSceneLightTime,mInvFrequency);
			float factor = CurrentTime/mInvFrequency;

			float rate =  (float)rand()/(float)RAND_MAX;
		//	float FlashRate = 0.9f;
			
			
			if (rate < mFlashTime*0.3f)
			{
				if ( factor < 0.5f )
				{
					renderLight = true;
					renderTime = 0.0f;
				}
			}
			else
			{
		//		renderLight = false;
			}
			
		}
		else
		{
		//	renderLight = false;
			mIsSceneLight = false;
			g_fDirectLR = mBackUpDirectionColor.r;
			g_fDirectLG = mBackUpDirectionColor.g;
			g_fDirectLB = mBackUpDirectionColor.b;
		}
	}
	if (renderLight&&renderTime<0.1f)
	{
		g_fDirectLR = mFlashColor.r;
		g_fDirectLG = mFlashColor.g;
		g_fDirectLB = mFlashColor.b;
		renderTime+=_timeMs;
	}
	else
	{
		renderLight = false;
		g_fDirectLR = mBackUpDirectionColor.r;
		g_fDirectLG = mBackUpDirectionColor.g;
		g_fDirectLB = mBackUpDirectionColor.b;

	}

}

void SceneLightMgr::stop()
{
	if (mIsSceneLight)
	{
		mIsSceneLight = false;
		g_fDirectLR = mBackUpDirectionColor.r;
		g_fDirectLG = mBackUpDirectionColor.g;
		g_fDirectLB = mBackUpDirectionColor.b;
	}
}