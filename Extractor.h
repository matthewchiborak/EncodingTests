#pragma once

#include <string>
#include <windows.h>
#include <vector>

#define BUFSIZE 100//4096 

struct Entity
{
	std::wstring type;
	std::wstring value;
};

class Extractor
{
public:
	Extractor();

	std::string ExtractEntities();
	int ExtractEntities2(int argc, char* argv[]);

	int ExtractEntities3(int argc, char* argv[]);

	bool validUtf8(std::vector<int>& data);
	bool theOldUtf8LenValidation(wchar_t byte);

	int MyUTF8lengthFinder(char byte);

	int isNthBitSet(unsigned char c, int n);

	std::string wstring_to_utf8(const std::wstring& str);

	void EncodingSeparateTest();
	void EncodingSeparateTest2(const std::string& s);

protected:
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;

	HANDLE g_hInputFile = NULL;

	void CreateChildProcess(void);
	void WriteToPipe(void);
	void ReadFromPipe(void);

	void ReadFromPipe2(void);

	//void ErrorExit(PTSTR error);
};

