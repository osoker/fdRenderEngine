#pragma once

// multi-thread write the log file 
// to prevent disk op too frequently
//
class CLogWriter
{
	//! the log file
	FILE*				m_logFile;

	//! the Critical section for reading string
	CRITICAL_SECTION	m_pushStringSection;
	
public:

	CLogWriter(void);
	~CLogWriter(void);

public:

	//! init the log writer
	BOOL InitLogWriter(const char* _logFilename);

	//! end log writer
	void EndLogWriter(const char* _str);

	//! push a string to write
	void PushLogString(const char* _string);

	//! main thread procedure...
	static void LogWriterProc(void* _param);
};
