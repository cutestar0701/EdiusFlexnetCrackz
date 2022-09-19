// EdiusCrackInstaller.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "EdiusCrackInstaller.h"
#include <stdio.h>
#include "resource.h"

int Step1_install_original()
{
    const size_t stringSize = 1000;
    char pchCurrentDir[1024] = { 0 };
    GetCurrentDirectory(1024, pchCurrentDir);
    char commandLine[stringSize] = { 0 };

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD exit_code;
    sprintf_s(commandLine,stringSize, "%s\\%s", pchCurrentDir, ORIGINAL_SETUP_FILE);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        commandLine,   // Command line
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
        return -1;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    GetExitCodeProcess(pi.hProcess, &exit_code);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

int Step2_copy_backup_dmp()
{
#if 0
    HRSRC myResource = ::FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
    unsigned int myResourceSize = ::SizeofResource(NULL, myResource);
    HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
    void* pMyBinaryData = ::LockResource(myResourceData);
#endif
    return 0;

}

int Step3_registerKey()
{
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    Step1_install_original();
    Step2_copy_backup_dmp();
    Step3_registerKey();
    return 0;
}

