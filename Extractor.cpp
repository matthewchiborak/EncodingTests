#include "Extractor.h"

#include <codecvt>
#include <sstream>
#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <set>

Extractor::Extractor()
{
}

std::string Extractor::ExtractEntities()
{
	/*wchar_t aParams[32768] = {0};
	wcsncpy(aParams, sParams.data(), sParams.size());

	if (!CreateProcess(NULL, aParams, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, sNodePath.data(), &oStartupInfo, &m_oProcessInfo))
		return "Error";

	return std::string();*/

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    /*if (argc != 2)
    {
        printf("Usage: %s [cmdline]\n", argv[0]);
        return;
    }*/

    // Start the child process. 
    if (!CreateProcess(L"D:\\VisualStudioProjects\\EncodingTests\\OutputTheFile\\x64\\Release\\OutputTheFile.exe",   // No module name (use command line)
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return "Failed";
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return "Success";
}

int Extractor::ExtractEntities2(int argc, char* argv[])
{
    SECURITY_ATTRIBUTES saAttr;

    printf("\n->Start of parent execution.\n");

    // Set the bInheritHandle flag so pipe handles are inherited. 

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        return 6;// ErrorExit(TEXT("StdoutRd CreatePipe"));

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return 1;//ErrorExit(TEXT("Stdout SetHandleInformation"));

    // Create a pipe for the child process's STDIN. 

    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        return 2;// ErrorExit(TEXT("Stdin CreatePipe"));

    // Ensure the write handle to the pipe for STDIN is not inherited. 

    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return 3;// ErrorExit(TEXT("Stdin SetHandleInformation"));

    // Create the child process. 

    CreateChildProcess();

    // Get a handle to an input file for the parent. 
    // This example assumes a plain text file and uses string output to verify data flow. 

    //if (argc == 1)
       // return 4;// ErrorExit(TEXT("Please specify an input file.\n"));

    g_hInputFile = CreateFileW(
        L"C:\\Users\\mchiborak\\Desktop\\LtlocateContentsJune20\\EXTRACTOR_DATA\\data\\testSmall.txt",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY,
        NULL);

    if (g_hInputFile == INVALID_HANDLE_VALUE)
        return 5;// ErrorExit(TEXT("CreateFile"));

    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.

    WriteToPipe();
    printf("\n->Contents of %S written to child STDIN pipe.\n", L"C:\\Users\\mchiborak\\Desktop\\LtlocateContentsJune20\\EXTRACTOR_DATA\\data\\testSmall.txt");

    // Read from pipe that is the standard output for child process. 

    printf("\n->Contents of child process STDOUT:\n\n");
    ReadFromPipe();

    printf("\n->End of parent execution.\n");

    // The remaining open handles are cleaned up when this process terminates. 
    // To avoid resource leaks in a larger application, close handles explicitly. 

    return 0;
}

int Extractor::ExtractEntities3(int argc, char* argv[])
{
    SECURITY_ATTRIBUTES saAttr;

    printf("\n->Start of parent execution.\n");

    // Set the bInheritHandle flag so pipe handles are inherited. 

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        return 6;// ErrorExit(TEXT("StdoutRd CreatePipe"));

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return 1;//ErrorExit(TEXT("Stdout SetHandleInformation"));

    // Create a pipe for the child process's STDIN. 

    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        return 2;// ErrorExit(TEXT("Stdin CreatePipe"));

    // Ensure the write handle to the pipe for STDIN is not inherited. 

    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return 3;// ErrorExit(TEXT("Stdin SetHandleInformation"));

    // Create the child process. 

    CreateChildProcess();

    // Get a handle to an input file for the parent. 
    // This example assumes a plain text file and uses string output to verify data flow. 

    //if (argc == 1)
      //  return 4;// ErrorExit(TEXT("Please specify an input file.\n"));

    /*g_hInputFile = CreateFileW(
        L"C:\\Users\\mchiborak\\Desktop\\LtlocateContentsJune20\\EXTRACTOR_DATA\\data\\testSmall.txt",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY,
        NULL);

    if (g_hInputFile == INVALID_HANDLE_VALUE)
        return 5;// ErrorExit(TEXT("CreateFile"));
    */

    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.

    WriteToPipe();
    printf("\n->Contents of %S written to child STDIN pipe.\n", L"Something");

    // Read from pipe that is the standard output for child process. 

    printf("\n->Contents of child process STDOUT:\n\n");
    ReadFromPipe();

    printf("\n->End of parent execution.\n");

    // The remaining open handles are cleaned up when this process terminates. 
    // To avoid resource leaks in a larger application, close handles explicitly. 

    return 0;
}

bool Extractor::validUtf8(std::vector<int>& data)
{
    int cnt = 0;
    for (int i = 0; i < data.size(); i++) {
        int x = data[i];
        if (!cnt) {
            if ((x >> 5) == 0b110) {
                cnt = 1;
            }
            else if ((x >> 4) == 0b1110) {
                cnt = 2;
            }
            else if ((x >> 3) == 0b11110) {
                cnt = 3;
            }
            else if ((x >> 7) != 0) return false;
        }
        else {
            if ((x >> 6) != 0b10) return false;
            cnt--;
        }
    }
    return cnt == 0;
}

bool Extractor::theOldUtf8LenValidation(wchar_t byte)
{
    if ((byte & 0b10000000) == 0b00000000) { return 1; }
    if ((byte & 0b11100000) == 0b11000000) { return 2; }
    if ((byte & 0b11110000) == 0b11100000) { return 3; }
    if ((byte & 0b11111000) == 0b11110000) { return 4; }
    return -1;
}

int Extractor::MyUTF8lengthFinder(char byte)
{
    if (!isNthBitSet(byte, 7)) { return 1; }
    if (isNthBitSet(byte, 7) && isNthBitSet(byte, 6) && !isNthBitSet(byte, 5)) { return 2; }
    if (isNthBitSet(byte, 7) && isNthBitSet(byte, 6) && isNthBitSet(byte, 5) && !isNthBitSet(byte, 4)) { return 3; }
    if (isNthBitSet(byte, 7) && isNthBitSet(byte, 6) && isNthBitSet(byte, 5) && isNthBitSet(byte, 4) && !isNthBitSet(byte, 3)) { return 4; }
    return -1;
}

int Extractor::isNthBitSet(unsigned char c, int n)
{
    unsigned char mask[] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    return ((c & mask[n]) != 0);
}

std::string Extractor::wstring_to_utf8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

void Extractor::EncodingSeparateTest()
{
    std::wstring s = L"ø Roger Federer and also Tim Smith but also Jerry Sfren and also this character w/e it is";
    std::string sToWrite = wstring_to_utf8(s);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    std::wstring sOutput = myconv.from_bytes(sToWrite);

    std::wcout << sOutput << L"\n";
    //EncodingSeparateTest2(s)
}

void Extractor::EncodingSeparateTest2(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    std::wstring sOutput = myconv.from_bytes(s);

    std::wcout << sOutput << L"\n";
}

void Extractor::CreateChildProcess(void)
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

     // If an error occurs, exit the application. 
    //if (!bSuccess)
      //  ErrorExit(TEXT("CreateProcess"));
    //else
    if(bSuccess)
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

void Extractor::WriteToPipe(void)
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    /*std::wifstream infile("D:\\VisualStudioProjects\\EncodingTests\\FromWork\\ExampleOutput3.txt");

    std::wstring line;

    while (std::getline(infile, line))
    {
        //line = line + L"\n";
        std::string sToWrite = wstring_to_utf8(line);
        dwRead = sToWrite.size() * sizeof(char);

        bSuccess = WriteFile(g_hChildStd_IN_Wr, sToWrite.c_str(), dwRead, &dwWritten, NULL);
    }*/

    for (;;)
    {
        ///
        //std::wstring s = L"ø Roger Federer and also Tim Smith but also Jerry Sfren and also this character w/e it is\nT0	TERM	33	45	good article	good article	TERM	1011\nT1	TERM	84	93	Page semi	Page semi	TERM	1011";
        std::wstring s = L".O	This is a good article. Click here for more information.\nT0	TERM	33	45	good article	good article	TERM	1011\nT1	TERM	84	93	Page semi	Page semi	TERM	1011\n.O	Federer at the 2016 Wimbledon Championships";
        std::string sToWrite = wstring_to_utf8(s);
        dwRead = sToWrite.size() * sizeof(char);

        bSuccess = WriteFile(g_hChildStd_IN_Wr, sToWrite.c_str(), dwRead, &dwWritten, NULL);
        break; //The loop is if theres more data to be read from the file. But im not using that. So it loops forever
        ///
    }
    

    // Close the pipe handle so the child process stops reading. 

    CloseHandle(g_hChildStd_IN_Wr);
    //if (!CloseHandle(g_hChildStd_IN_Wr))
        //ErrorExit(TEXT("StdInWr CloseHandle"));
}

void Extractor::ReadFromPipe(void)
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;

    std::string sAllOut;
    std::wstring allOutput;
    std::wstring temp;

    //std::regex str_expr("T([0-9]*)\\t(.*)\\t([0-9]*)\\t");
    std::wregex str_expr(L"T([0-9]+)");

    std::unordered_map<std::wstring, std::set<std::wstring>> resultMap;

    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        std::string s(chBuf, dwRead);

        //sAllOut += s;

        std::wstring sOutput = myconv.from_bytes(s);

        std::wstringstream wss(sOutput);
        std::wstring line;
        std::wstring tabseg;
        std::wstring type;
        std::wstring value;

        while (std::getline(wss, line))
        {
            //std::wcout << L"LINE: " << line << L" !!!!!\n";
            //while (std::getline(wss, line))
            std::wstringstream sstabseg(line);
            std::getline(sstabseg, tabseg, L'\t');
            if (std::regex_match(tabseg, str_expr))
            {
                std::getline(sstabseg, tabseg, L'\t');
                //std::wcout << tabseg << L" ";
                type = tabseg;
                std::getline(sstabseg, tabseg, L'\t');
                std::getline(sstabseg, tabseg, L'\t');
                std::getline(sstabseg, tabseg, L'\t');
               // std::wcout << tabseg << L"\n";
                value = tabseg;

                if (resultMap.find(type) == resultMap.end())
                {
                    std::set<std::wstring> newValueMap;
                    resultMap.insert(std::pair<std::wstring, std::set<std::wstring>>(type, std::move(newValueMap)));
                }

                resultMap[type].insert(value);
            }
        }





        //std::wstringstream wss(sOutput);
        
        //int Tpos = sOutput.find(L"T");
        //wss.seekg(Tpos);

        //std::wstring focus = sOutput.substr(Tpos);



        //allOutput += sOutput;

        //std::wcout << sOutput << L"\n";

        /*std::wstringstream wss(sOutput);
        std::getline(wss, temp, L'\t');

        if (temp[0] == 'T')
        {
            std::getline(wss, temp, L'\t');
            std::wcout << temp << " ";
            std::getline(wss, temp, L'\t');
            std::getline(wss, temp, L'\t');
            std::getline(wss, temp, L'\t');
            std::wcout << temp << "\n";
        }*/

        //std::cout << s << "\n";

       // std::wstring wstr(reinterpret_cast<wchar_t*>(chBuf), dwRead / sizeof(wchar_t));
        //std::wcout << wstr << L"\n";
        ///////////
        //std::string sOutputs = chBuf;
        //std::wstring sOutput = myconv.from_bytes(chBuf);
        //std::vector<std::wstring> parts;
        //std::wstringstream wss(sOutput);
        //std::wcout << sOutput << L"\n";
        //std::cout << sOutputs << "\n";
        //////////////

        //bSuccess = WriteFile(hParentStdOut, chBuf,
          //  dwRead, &dwWritten, NULL);
        //if (!bSuccess) break;
    }


    for (std::unordered_map<std::wstring, std::set<std::wstring>>::iterator it = resultMap.begin(); it != resultMap.end(); it++)
    {
        std::wcout << L"Type:\n";
        std::wcout << it->first << L"\n";

        for (std::set<std::wstring>::iterator itset = it->second.begin(); itset != it->second.end(); itset++)
        {
            std::wcout << *itset << L"\n";
        }
    }

    //std::wstring sOutput = myconv.from_bytes(sAllOut);
    //std::wcout << sOutput;

    //std::wstring temp;
    //int mode = 0;

    //std::wstringstream wss(allOutput);

    //wss.
    //int index = allOutput.find(L"T");
    //int index2 = allOutput.find(L"T");

    /*while (std::getline(wss, temp, L'\t'))
    {
       // parts.push_back(temp);
        if (temp == L"T0")
        {
            mode = 1;
        }
    }*/
}

