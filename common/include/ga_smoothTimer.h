///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: ga_smoothTimer.h
 *
 *  Desc:     �ѵײ�õ���timeʱ��ƽ��������
 * 
 *  His:      River created @ 2008-5-9
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once
# include "windows.h"
# include "mmsystem.h"
# include "stdio.h"
# include "../common.h"
# include <assert.h>

/** \brief
 *  �ѵײ�������ʱ��ƽ������һ�£�ʹ�ϲ���˶�����ƽ˳
 *
 */
class COMMON_API ga_smoothTimer
{
private:
	float    m_fSmoothTime;

public:
	ga_smoothTimer(void);
	~ga_smoothTimer(void);

	//! ÿһ֡���д���
	void     frame_move( void );
	//! �ϲ�õ�ƽ��ʱ��
	float    get_smoothTime( void ) { return m_fSmoothTime; }

	static ga_smoothTimer* Instance(void);

};

extern COMMON_API ga_smoothTimer*   g_ptrSmoothTimer;