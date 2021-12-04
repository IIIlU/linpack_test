#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <string>
#include <iostream>
#include "process_library.h"

#define BUFSIZE 4096 

uint64_t ProcessLib::systemAffinity = 0;

void ErrorExit(std::string message) 
// Format a readable error message, display a message box, 
// and exit from the application.
{ 
    std::cout << "ERROR: " << message;
    ExitProcess(1);
}

struct ProcessLibData
{
    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    SECURITY_ATTRIBUTES saAttr; 

    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;

    ~ProcessLibData()
    {
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_OUT_Rd);
        CloseHandle(g_hChildStd_IN_Rd);
        CloseHandle(g_hChildStd_IN_Wr);
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    };
};

ProcessLib::ProcessLib(ProcessLib &&src)
{
    data = src.data;
    src.data = nullptr;
}

TCHAR *getEnvString(const std::vector<std::string> &env)
{
    const TCHAR *front = TEXT("PATH=");
    TCHAR *oldPath = new TCHAR[BUFSIZE];
    GetEnvironmentVariable(TEXT("PATH"), oldPath, BUFSIZE);
    int size = 0;
    while(oldPath[size++]);
    size += 6;

    for(std::string s : env)
    {
        size += s.length() + 1;
    }

    TCHAR *result = new TCHAR[size];
    TCHAR *dst = result;
    const TCHAR *src = oldPath;
    for(int i = 0; i < 5; ++i) *(dst++) = front[i];
    while(*(dst++) = *(src++)) ;
    delete oldPath;
    for(std::string s : env)
    {
        src = s.c_str();
        while(*(dst++) = *(src++));
    }
    *dst = '\0';
    return result;
}

ProcessLib::ProcessLib(const std::string &cmd, const std::vector<std::string> &env, uint64_t affinity)
{
    uint64_t processAffinity;
    data = new ProcessLibData();
    data->saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    data->saAttr.bInheritHandle = TRUE; 
    data->saAttr.lpSecurityDescriptor = NULL; 
        
    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&data->g_hChildStd_OUT_Rd, &data->g_hChildStd_OUT_Wr, &data->saAttr, 0)) 
    ErrorExit("StdoutRd CreatePipe");

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(data->g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    ErrorExit("Stdout SetHandleInformation"); 

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&data->g_hChildStd_IN_Rd, &data->g_hChildStd_IN_Wr, &data->saAttr, 0)) 
    ErrorExit("Stdin CreatePipe"); 

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(data->g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
    ErrorExit("Stdin SetHandleInformation");

    // Create Child Process Infomation
    TCHAR *szCmdline = const_cast<TCHAR*>(cmd.c_str());
    TCHAR *envLine = getEnvString(env);
    BOOL bSuccess = FALSE; 

    // Set up members of the PROCESS_INFORMATION structure. 
    ZeroMemory(&data->piProcInfo, sizeof(PROCESS_INFORMATION));
 
    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.
 
    ZeroMemory(&data->siStartInfo, sizeof(STARTUPINFO));
    data->siStartInfo.cb = sizeof(STARTUPINFO); 
    data->siStartInfo.hStdError = data->g_hChildStd_OUT_Wr;
    data->siStartInfo.hStdOutput = data->g_hChildStd_OUT_Wr;
    data->siStartInfo.hStdInput = data->g_hChildStd_IN_Rd;
    data->siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 
    bSuccess = CreateProcess(NULL, 
    szCmdline,     // command line 
    NULL,          // process security attributes 
    NULL,          // primary thread security attributes 
    TRUE,          // handles are inherited 
    CREATE_SUSPENDED,             // creation flags 
    envLine,          // use parent's environment 
    NULL,          // use parent's current directory 
    &data->siStartInfo,  // STARTUPINFO pointer 
    &data->piProcInfo);  // receives PROCESS_INFORMATION 

    delete envLine;
    // If an error occurs, exit the application. 
    if (!bSuccess) ErrorExit("CreateProcess");
    else 
    {
        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.
        CloseHandle(data->g_hChildStd_OUT_Wr);
        CloseHandle(data->g_hChildStd_IN_Rd);
        if(!systemAffinity) GetProcessAffinityMask(data->piProcInfo.hProcess, &processAffinity, &systemAffinity);
        changeAffinity(affinity);
        ResumeThread(data->piProcInfo.hThread);
    }
};

bool ProcessLib::WriteToPipe(const std::string &input)
// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{ 
    DWORD dwWritten; 
    const CHAR* chBuf = input.c_str();
    BOOL bSuccess = FALSE;
    int leftString = input.length();

    while(true)
    {
        bSuccess = WriteFile(data->g_hChildStd_IN_Wr, chBuf, leftString, &dwWritten, NULL);
        if (!bSuccess) return false;
        leftString -= dwWritten;
        chBuf += dwWritten;
        if(leftString < 1) break;
    } 
    return true;
};

std::string ProcessLib::ReadFromPipe(void) 
// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
    DWORD dwRead; 
    CHAR chBuf[BUFSIZE]; 
    BOOL bSuccess = FALSE;
    std::string result;
    DWORD dw;
    int trys = 3;

    while(PeekNamedPipe(data->g_hChildStd_OUT_Rd, NULL, 0, NULL, &dw, NULL)) 
    { 
        if(dw <= 0 && trys-- >= 0) break;
        bSuccess = ReadFile(data->g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if(!bSuccess || dwRead == 0) break; 
        chBuf[dwRead] = '\0';
        result += chBuf;
    }

    return std::move(result);
}; 

bool ProcessLib::is_running()
{
    DWORD ret;
    GetExitCodeProcess(data->piProcInfo.hProcess, &ret);
    return ret == STILL_ACTIVE;
}

bool ProcessLib::terminate()
{
    return TerminateProcess(data->piProcInfo.hProcess, 1);
}

ProcessLib::~ProcessLib()
{
    if(data == nullptr) return;
    if(this->is_running()) this->terminate();
    delete data;
};

bool ProcessLib::changeAffinity(uint64_t affinity)
{
    if(affinity == 0) return false;
    return SetProcessAffinityMask(data->piProcInfo.hProcess, affinity & systemAffinity);
}

void help_sleep(int ms)
{
    Sleep(ms);
}
