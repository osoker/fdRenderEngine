// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>
// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ�ļ�

# include "../fdrEngine/interface/osInterface.h"
# include "direct.h"



//! ʹ�����ǵ����ӿ�
#pragma comment ( lib,"winmm" )
#pragma comment ( lib,"d3dx9" )

#ifdef _DEBUG

#pragma comment ( lib,"../lib/common" )
#pragma comment ( lib,"../lib/fdREngine" )
#pragma comment ( lib,"../lib/fileOperate" )
#pragma comment ( lib,"../lib/smManager" )

# else

#pragma comment ( lib,"../lib/commonR" )
#pragma comment ( lib,"../lib/fdREngineR" )
#pragma comment ( lib,"../lib/fileOperateR" )
#pragma comment ( lib,"../lib/smManagerR" )

# endif 

//! �Ƿ����޸ĵر���ײ��Ϣ�İ汾,���Ҫ�޸ĳɿ����޸���ײ��Ϣ�İ汾�Ļ�����
//! �Ѻ�__COLL_DISGRID__�������޸ĳ�5.
# define   __COLL_MOD__   0

//! ���ĵ���Χ��ʾ���ٲ���ײ��Ϣ,����������
# define   __COLL_DISGRID__   5


//! ����ˢ����Ϣ��صĲ���������
# define  __DEPLOY_MONSTERMOD__ 0
//! ˢ���������Ķ�����Ŀ.
# define MAX_POINTS   100

# if 0
#ifdef _DEBUG

//!�����ڴ����
#pragma comment( lib,"../../lib/strmbasd.lib")

//!ý��������
#pragma comment( lib,"../../lib/asynbase.lib")


#endif 

#ifdef NDEBUG

//!�����ڴ����
#pragma comment( lib,"../../lib/strmbaseR.lib")

//!ý��������
#pragma comment( lib,"../../lib/asynbaseR.lib")

#endif
# endif 