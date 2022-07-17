#pragma once

#include <windows.h>

#define BUFSIZE 4096 

class PIIExtractor
{
public:
	PIIExtractor();

	void ExtractEntities();

protected:
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;

	void CreateChildProcess(void);
	void WriteToPipe(void);
	void ReadFromPipe(void);
};

