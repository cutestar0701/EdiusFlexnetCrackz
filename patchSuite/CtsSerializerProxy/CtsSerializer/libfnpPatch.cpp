#include "pch.h"
#include <comdef.h>
#include <Wbemidl.h>
#include "mhook-lib/mhook.h"
#include <atlconv.h>

char g_requestNum[16];
char g_fullfillmentID[129];

void PatchBytes(__int64 pAddress, char* pByte, int nlen)
{
    DWORD old;
    VirtualProtect((LPVOID)pAddress, nlen, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void*)pAddress, pByte, nlen);
    VirtualProtect((LPVOID)pAddress, nlen, old, &old);
}

BOOL PatchAppMaintainer_libFNP_dll()
{
//AppMaintainer_libFNP base = 00007FFE81220000

//checkSignatureHash
//.text:00007FFE813099C8 32 C0 xor al, al = > 
                                             // B0 01 | mov al, 1 
//checkRequestHash
//.text : 00007FFE812AE435 E8 46 18 30 00  call    memcmp ==>
                                                            //00007FFE812AE435 | 33C0 | xor eax, eax | 
                                                            //00007FFE812AE437 | 90 | nop |
                                                            //00007FFE812AE438 | 90 | nop |
                                                            //00007FFE812AE439 | 90 | nop |

//check RequestVerifier
//.text:00007FFE812DC239 0F 83 FB 00 00 00                                jnb     loc_7FFE812DC33A  ==> jmp E9 FC 00 00 00

    Sleep(5000);

    __int64 baseLibFnp = (__int64)LoadLibraryA("AppMaintainer_libFNP.dll");
    if (baseLibFnp == NULL)
    {
        kgh_log("$$$$$$$$$$$$$ patched failed : AppMaintainer_libFNP $$$$$$$$$$$$$\n");
        return FALSE;
    }

    __int64 checkSignatureHashOffset = 0xE99C8;
    PatchBytes(baseLibFnp + checkSignatureHashOffset, "\xb0\x01", 2);

    __int64 checkRequestHashOffset = 0x8E435;
    PatchBytes(baseLibFnp + checkRequestHashOffset, "\x33\xC0\x90\x90\x90", 5);

    __int64 checkUniqueMachineNumber = 0xBC239;
    PatchBytes(baseLibFnp + checkUniqueMachineNumber, "\xE9\xFC\x00\x00\x00", 5);

    kgh_log("============ patched okay : AppMaintainer_libFNP ================\n");
    return TRUE;
}

void PatchPayload()
{
    char pchFileName[1024] = { 0 };
    char pchCurrentDir[1024] = { 0 };
    GetCurrentDirectory(1024, pchCurrentDir);
    sprintf(pchFileName, "%s\\log\\%s.dmp", pchCurrentDir, GVKK_LICENSEKEY);

    FILE* fp = fopen(pchFileName, "rb");
    if (!fp)
        return;
    char oriByte[16] = { 0 };
    fseek(fp, 0, 2);
    int nfilesize = ftell(fp);
    fseek(fp, 0, 0);
    char* pchBuf = new char[nfilesize + 1];
    fread(pchBuf, nfilesize, 1, fp);
    fclose(fp);
    pchBuf[nfilesize] = 0;
    memcpy(oriByte, pchBuf, 4);
    fp = fopen(pchFileName, "wb");

    #define PATYLOAD_PATCH "Could not open named pipe"

    char payload[64] = PATYLOAD_PATCH;

    int payloadlen = strlen(PATYLOAD_PATCH);

    for (int i = 0; i < nfilesize; i++)
        pchBuf[i] ^= payload[i % payloadlen];
    
    fwrite(pchBuf, nfilesize, 1, fp);

    delete pchBuf;
    fclose(fp);

    kgh_log("============ patched payload : %s ================\n", oriByte);
}

void FetchRequestNumberFromID_Key(LPCWSTR pchFileName)
{
    FILE* fp = _wfopen((wchar_t*)pchFileName, L"rb");
    if (!fp)
        return;
    fseek(fp, 0, 2);
    int nfilesize = ftell(fp);
    fseek(fp, 0, 0);
    char* pchBuf = new char[nfilesize + 1];
    fread(pchBuf, nfilesize, 1, fp);
    fclose(fp);
    pchBuf[nfilesize] = 0;

    #define MARK_SEQUENCENUMBER "<SequenceNumber>"
    char* pchStartMark = strstr(pchBuf, MARK_SEQUENCENUMBER);
    if (pchStartMark)
    {
        pchStartMark += strlen(MARK_SEQUENCENUMBER);
        char pchSequenceNumber[16] = { 0 };

        int nlen = 0;
        for (int i = 0; i < 4; i++)
        {
            if (!isdigit(pchStartMark[i]))
                break;
            pchSequenceNumber[nlen] = pchStartMark[i];
            nlen++;
        }

        int nVal = atoi(pchSequenceNumber);
        if (nVal > 0 && nVal < 100)
        {
            strcpy(g_requestNum, pchSequenceNumber);
            kgh_log("Fetched RequestNumber from id.key : %s\n", g_requestNum);
        }
    }

    delete pchBuf;
    return;
}

void FetchFullfillmentID_FromID_Key(LPCWSTR pchFileName)
{
    FILE* fp = _wfopen((wchar_t*)pchFileName, L"rb");
    if (!fp)
        return;
    fseek(fp, 0, 2);
    int nfilesize = ftell(fp);
    fseek(fp, 0, 0);
    char* pchBuf = new char[nfilesize + 1];
    fread(pchBuf, nfilesize, 1, fp);
    fclose(fp);
    pchBuf[nfilesize] = 0;

#define MARK_FULLFILLMENT_ID "<FulfillmentId>"
    char* pchStartMark = strstr(pchBuf, MARK_SEQUENCENUMBER);
    if (pchStartMark)
    {
        pchStartMark += strlen(MARK_SEQUENCENUMBER);
        char pchFullfillID[128] = { 0 };

        int nlen = 0;
        for (int i = 0; i < 64; i++)
        {
            if (pchStartMark[i] == '<')
                break;
            pchFullfillID[nlen] = pchStartMark[i];
            nlen++;
        }

        int nVal = strlen(pchFullfillID);
        if (nVal == 40)
        {
            strcpy(g_fullfillmentID, pchFullfillID);
            kgh_log("Fetched FullfillmentID from id.key : %s\n", g_fullfillmentID);
        }
    }

    delete pchBuf;
    return;
}
typedef BOOL (* pfn_DeviceIoControl_T)(
    HANDLE       hDevice,
    DWORD        dwIoControlCode,
    LPVOID       lpInBuffer,
    DWORD        nInBufferSize,
    LPVOID       lpOutBuffer,
    DWORD        nOutBufferSize,
    LPDWORD      lpBytesReturned,
    LPOVERLAPPED lpOverlapped
);
pfn_DeviceIoControl_T g_ori_DeviceIoControl;

BOOL myDeviceIoControl(
    HANDLE       hDevice,
    DWORD        dwIoControlCode,
    LPVOID       lpInBuffer,
    DWORD        nInBufferSize,
    LPVOID       lpOutBuffer,
    DWORD        nOutBufferSize,
    LPDWORD      lpBytesReturned,
    LPOVERLAPPED lpOverlapped
)
{
BOOL bres = g_ori_DeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
if (dwIoControlCode == 0x7C088 || dwIoControlCode == 0x4d008)
{
#if 1
    strcpy((char*)lpOutBuffer + 0x30, "==KGHTEST==");
#else
    memset((char*)lpOutBuffer, 0x30, nOutBufferSize);
#endif
}
if (dwIoControlCode == 0x74080)
{
    //memset((char*)lpOutBuffer, 0x30, 0x44);
}
kgh_log("====== myDeviceIoControl : hDevice=0x%x , code=0x%x =======\n", hDevice, dwIoControlCode);
return bres;
}


typedef BOOL(__stdcall* pfn_DeleteFileW_T)(LPCWSTR lpFileName);
pfn_DeleteFileW_T g_ori_DeleteFileW;

BOOL __stdcall myDeleteFileW(LPCWSTR lpFileName)
{
    int nMode = 0;
    if (lpFileName && wcsstr(lpFileName, L"ID.key"))
    {
        nMode = 1;
    }

    if (lpFileName && wcsstr(lpFileName, L"Response.key"))
    {
        nMode = 2;
    }
    if (nMode)
    {
#if KGH_LOG_ENABLE
        kgh_log("====== v1.1 , Backup Key file : Mode:%d =======\n", nMode);

        SYSTEMTIME st;
        GetSystemTime(&st);

        wchar_t backupPath[1024] = { 0 };
        wsprintfW(backupPath, L"D:\\%04d%02d%02d_%02d%02d%02d.key%d.txt",
            st.wYear, st.wMonth, st.wDay, st.wHour + 9, st.wMinute,st.wSecond, nMode);

        CopyFileW(lpFileName, backupPath, 0);
#endif
        if (nMode == 1)
            FetchRequestNumberFromID_Key(lpFileName);
    }
    return g_ori_DeleteFileW(lpFileName);
}

void PatchRequestSequenceNumber(char* pBuffer, int nNumberOfBytesToRead)
{
    if (!pBuffer)
        return;
    if (nNumberOfBytesToRead < 0x1000 || nNumberOfBytesToRead > 0x2000)
        return;

    //kgh_log("====== ReadFile : nLen=%d =======\n", nNumberOfBytesToRead);

    if (strstr(pBuffer,"<Response xmlns=\"FLEXnetPublisher_01EFFF13-92D7-469e-B7CC-35E13C2E8D4A\">"))
    {
        //><RequestSequenceNumber>63</RequestSequenceNumber>
        #define MARK_REQUESTSEQUENCENUMBER "<RequestSequenceNumber>"
        char* pchRequestSequenceNumber = strstr(pBuffer, MARK_REQUESTSEQUENCENUMBER);
        if (!pchRequestSequenceNumber)
            return;

        pchRequestSequenceNumber += strlen(MARK_REQUESTSEQUENCENUMBER);

        int nLen = 0;
        for (int i = 0; i < 3; i++)
        {
            if (!isdigit(pchRequestSequenceNumber[i]))
                break;
            nLen++;
        }

        char sequenceNumber[32] = { 0 };
        memcpy(sequenceNumber, pchRequestSequenceNumber, nLen);
        int nReqSeqNum = atoi(sequenceNumber);
        
        if ((nLen == 1 || nLen == 2) && g_requestNum[0] != 0 && strlen(sequenceNumber) == strlen(g_requestNum))
        {
            memset(pchRequestSequenceNumber, 0x20, nLen);
            memcpy(pchRequestSequenceNumber, g_requestNum, nLen);
            //*(pchRequestSequenceNumber + nLen - 1) = *(pchRequestSequenceNumber + nLen - 1) + 1;
            kgh_log("====== ReadFile Patch {Success} : nLen=%d, original ReqSeqNum:%d , with  %s =======\n", nLen, nReqSeqNum, g_requestNum);
        }
        else
        {
            kgh_log("====== ReadFile Patch {Failed} : nLen=%d, original ReqSeqNum:%d , requestNum  %s =======\n", nLen, nReqSeqNum, g_requestNum);
        }
#if 0
        char* pchUMN1 = strstr(pBuffer, "4005761DF3AAAD78B447796314AA9AE97DF7C766");
        if(pchUMN1)
        {
            *pchUMN1 = 0x33;
        }
        char* pchUMN2 = strstr(pBuffer, "4CF59000F6EFA7B805A25FEBCCCD3DD0F655DE0E");
        if (pchUMN2)
        {
            *pchUMN2 = 0x33;
        }
        kgh_log("Patched UMN with invalid value for test\n");
#endif
    }
}


typedef BOOL (* pfn_WriteFile_T)(
    HANDLE       hFile,
    LPCVOID      lpBuffer,
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped);
pfn_WriteFile_T g_oriWriteFile;

BOOL myWriteFile(
    HANDLE       hFile,
    LPCVOID      lpBuffer,
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
)
{
    BOOL bres = g_oriWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    return bres;
}

typedef BOOL (* pfn_ReadFile_T)(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped);
pfn_ReadFile_T g_oriReadFile;

BOOL myReadFile(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
)
{
    BOOL bRes = g_oriReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    PatchRequestSequenceNumber((char*)lpBuffer, nNumberOfBytesToRead);
    return bRes;
}

void wmi_install_patch()
{
    if (!GetModuleHandle("AppMaintainer.exe"))
    {
        return;
    }

    HMODULE hmod = GetModuleHandle("kernelbase.dll");
    if (hmod == NULL)
        return;

    //g_ori_DeviceIoControl = (pfn_DeviceIoControl_T)GetProcAddress(hmod, "DeviceIoControl");
    //Mhook_SetHook((PVOID*)&g_ori_DeviceIoControl, myDeviceIoControl);

    g_ori_DeleteFileW = (pfn_DeleteFileW_T)GetProcAddress(hmod, "DeleteFileW");
    Mhook_SetHook((PVOID*)&g_ori_DeleteFileW, myDeleteFileW);

    //g_oriWriteFile = (pfn_WriteFile_T)GetProcAddress(hmod, "WriteFile");
    //Mhook_SetHook((PVOID*)&g_oriWriteFile, myWriteFile);

    g_oriReadFile = (pfn_ReadFile_T)GetProcAddress(hmod, "ReadFile");
    Mhook_SetHook((PVOID*)&g_oriReadFile, myReadFile);

    PatchAppMaintainer_libFNP_dll();
    //PatchPayload();
}

void wmi_uninstall()
{
    Mhook_Unhook((PVOID*)&g_ori_DeviceIoControl);
}