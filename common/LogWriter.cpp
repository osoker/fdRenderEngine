
#include "stdafx.h"
#include ".\logwriter.h"
#include "include/timecounter.h"
#include "include/Sgdebug.h"

CLogWriter::CLogWriter(void):m_logFile(NULL)
{
	ZeroMemory(&m_pushStringSection,sizeof(m_pushStringSection));
}

CLogWriter::~CLogWriter(void)
{
}
void CLogWriter::PushLogString(const char* _string)
{
	

	if(m_logFile){
		CCriticalSec t_criticalSec(&m_pushStringSection);

		fputs(_string,m_logFile);
		fflush(m_logFile);
	}

	
}
BOOL CLogWriter::InitLogWriter(const char* _logFilename)
{
	BOOL t_result = FALSE;

	if(m_logFile){
		fclose(m_logFile);
		m_logFile = NULL;
	}

	_mkdir("log");
	m_logFile = fopen(_logFilename,"wt+");

	if(m_logFile){

		time_t	t_timeVal = time(NULL);
		tm* t_time = localtime(&t_timeVal);

		fprintf( m_logFile,"\\---- Start log time : %d/%d/%d/ %d:%d ----/\n",
				t_time->tm_year + 1900,
				t_time->tm_mon + 1,
				t_time->tm_mday,
				t_time->tm_hour,
				t_time->tm_min);

		t_result = TRUE;

		::InitializeCriticalSection(&m_pushStringSection);
	}

	return t_result;
}

void CLogWriter::EndLogWriter(const char* _str)
{
	if(m_logFile){
		
		if(_str){
			fprintf( m_logFile,"%s\n\n\n",_str );
		}
		// 写入程序从开始到现在，总共运行了多长的时间。
		fprintf( m_logFile,"Total Run Time is:< %f > seconds...\n",
			sg_timer::Instance()->get_talelatime() );

		fclose(m_logFile);
		m_logFile = NULL;

		::DeleteCriticalSection(&m_pushStringSection);
	}
}
