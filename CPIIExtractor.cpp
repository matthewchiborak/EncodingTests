#include "CPIIExtractor.h"

#include <codecvt>
#include <sstream>
#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <set>

PIIExtractor::PIIExtractor()
{
}

void PIIExtractor::ExtractEntities()
{
    SECURITY_ATTRIBUTES saAttr;

    // Set the bInheritHandle flag so pipe handles are inherited. 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        return;

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return;

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        return;

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return;

    // Create the child process. 
    CreateChildProcess();

    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.
    WriteToPipe();

    // Read from pipe that is the standard output for child process. 
    ReadFromPipe();
}

void PIIExtractor::CreateChildProcess(void)
{
    TCHAR szCmdline[] = TEXT("D:\\VisualStudioProjects\\EncodingTests\\OutputTheFile\\x64\\Release\\OutputTheFile.exe");
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure. 
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 
    bSuccess = CreateProcess(NULL,
        szCmdline,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    if (bSuccess)
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
}

void PIIExtractor::WriteToPipe(void)
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    std::wstring s = L".O	This is a good article. Click  here for more information.\nT0	TERM	33	45	good article	good article	TERM	1011\nT1	TERM	84	93	Page semi	Page semi	TERM	1011\nT2	PERSON	84	93	Test Person	Page semi	TERM	1011\n.O	Federer at the 2016 Wimbledon Championships";
    
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    std::string sToWrite = myconv.to_bytes(s);
    dwRead = sToWrite.size() * sizeof(char);

    bSuccess = WriteFile(g_hChildStd_IN_Wr, sToWrite.c_str(), dwRead, &dwWritten, NULL);

    // Close the pipe handle so the child process stops reading. 
    CloseHandle(g_hChildStd_IN_Wr);
}

void PIIExtractor::ReadFromPipe(void)
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> oConverter;

    std::wregex oEntityStartExpression(L"T([0-9]+)");
    std::wregex oLineStartExpression(L"^((\.O)|(T([0-9]+)))\\t");

    std::unordered_map<std::wstring, std::set<std::wstring>> mapExtractedEntities;

    std::wstring sLastLine = L"";
    std::wstring sTemp;

    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        std::string sOutputUTF8(chBuf, dwRead);

        std::wstring sOutput = oConverter.from_bytes(sOutputUTF8);

        std::wcout << L"The output: " << sOutput << L"\n";

        std::wstringstream ssOutput(sOutput);
        std::wstring sCurrentLine;
        std::wstring sTabSegment;
        std::wstring sType;
        std::wstring sValue;

        while (std::getline(ssOutput, sCurrentLine))
        {
            if (!std::regex_search(sCurrentLine, oLineStartExpression))
            {
                sLastLine += sCurrentLine;
                sCurrentLine = L"";
            }

            sTemp = sLastLine;
            sLastLine = sCurrentLine;
            sCurrentLine = sTemp;

            std::wstringstream sstabseg(sCurrentLine);
            std::getline(sstabseg, sTabSegment, L'\t');
            if (std::regex_match(sTabSegment, oEntityStartExpression))
            {
                std::getline(sstabseg, sTabSegment, L'\t');
                sType = sTabSegment;
                std::getline(sstabseg, sTabSegment, L'\t');
                std::getline(sstabseg, sTabSegment, L'\t');
                std::getline(sstabseg, sTabSegment, L'\t');
                sValue = sTabSegment;

                if (mapExtractedEntities.find(sType) == mapExtractedEntities.end())
                {
                    std::set<std::wstring> newValueMap;
                    mapExtractedEntities.insert(std::pair<std::wstring, std::set<std::wstring>>(sType, std::move(newValueMap)));
                }

                mapExtractedEntities[sType].insert(sValue);
            }
        }
    }

    for (std::unordered_map<std::wstring, std::set<std::wstring>>::iterator it = mapExtractedEntities.begin(); it != mapExtractedEntities.end(); it++)
    {
        std::wcout << L"Type:\n";
        std::wcout << it->first << L"\n";

        for (std::set<std::wstring>::iterator itset = it->second.begin(); itset != it->second.end(); itset++)
        {
            std::wcout << *itset << L"\n";
        }
    }
}
