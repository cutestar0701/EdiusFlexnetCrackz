#include "pch.h"
#include "mhook-lib/mhook.h"
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")


HRESULT InitializeCom()
{
    HRESULT result = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(result))
        return result;

    result = CoInitializeSecurity(
        NULL,                           // pSecDesc
        -1,                             // cAuthSvc (COM authentication)
        NULL,                           // asAuthSvc
        NULL,                           // pReserved1
        RPC_C_AUTHN_LEVEL_DEFAULT,      // dwAuthnLevel
        RPC_C_IMP_LEVEL_IMPERSONATE,    // dwImpLevel
        NULL,                           // pAuthList
        EOAC_NONE,                      // dwCapabilities
        NULL                            // Reserved
    );

    if (FAILED(result) && result != RPC_E_TOO_LATE)
    {
        CoUninitialize();

        return result;
    }

    return NOERROR;
}

HRESULT GetWbemService(IWbemLocator** pLocator, IWbemServices** pService)
{
    HRESULT result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, reinterpret_cast<LPVOID*>(pLocator));

    if (FAILED(result))
    {
        return result;
    }

    result = (*pLocator)->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),    // strNetworkResource
        NULL,                       // strUser  
        NULL,                       // strPassword
        NULL,                       // strLocale
        0,                          // lSecurityFlags
        NULL,                       // strAuthority
        NULL,                       // pCtx
        pService                    // ppNamespace
    );

    if (FAILED(result))
    {
        (*pLocator)->Release();

        return result;
    }

    result = CoSetProxyBlanket(
        *pService,                      // pProxy
        RPC_C_AUTHN_WINNT,              // dwAuthnSvc
        RPC_C_AUTHZ_NONE,               // dwAuthzSvc
        NULL,                           // pServerPrincName
        RPC_C_AUTHN_LEVEL_CALL,         // dwAuthnLevel
        RPC_C_IMP_LEVEL_IMPERSONATE,    // dwImpLevel
        NULL,                           // pAuthInfo
        EOAC_NONE                       // dwCapabilities
    );

    if (FAILED(result))
    {
        (*pService)->Release();
        (*pLocator)->Release();

        return result;
    }

    return NOERROR;
}



HRESULT QueryValue(IWbemServices* pService, const wchar_t* query, const wchar_t* propertyName, char* propertyValue, int maximumPropertyValueLength)
{
    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT result = pService->ExecQuery(
        bstr_t(L"WQL"),                                         // strQueryLanguage
        bstr_t(query),                                          // strQuery
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,  // lFlags
        NULL,                                                   // pCtx
        &pEnumerator                                            // ppEnum
    );

    if (FAILED(result))
        return result;

    IWbemClassObject* pQueryObject = NULL;
    while (pEnumerator)
    {
        try
        {
            ULONG returnedObjectCount = 0;
            result = pEnumerator->Next(WBEM_INFINITE, 1, &pQueryObject, &returnedObjectCount);

            if (returnedObjectCount == 0)
                break;

            VARIANT objectProperty;
            result = pQueryObject->Get(propertyName, 0, &objectProperty, 0, 0);
            if (FAILED(result))
            {
                if (pEnumerator != NULL)
                    pEnumerator->Release();

                if (pQueryObject != NULL)
                    pQueryObject->Release();

                return result;
            }

            if ((objectProperty.vt & VT_BSTR) == VT_BSTR)
            {
                //strcpy_s(propertyValue, maximumPropertyValueLength, OLE2A(objectProperty.bstrVal));
                break;
            }

            VariantClear(&objectProperty);
        }
        catch (...)
        {
            if (pEnumerator != NULL)
                pEnumerator->Release();

            if (pQueryObject != NULL)
                pQueryObject->Release();

            return NOERROR;
        }
    }

    if (pEnumerator != NULL)
        pEnumerator->Release();

    if (pQueryObject != NULL)
        pQueryObject->Release();

    return NOERROR;
}

HRESULT GetCpuId(char* cpuId, int bufferLength)
{
    HRESULT result = InitializeCom();
    if (FAILED(result))
        return result;

    IWbemLocator* pLocator = NULL;
    IWbemServices* pService = NULL;
    result = GetWbemService(&pLocator, &pService);
    if (FAILED(result))
    {
        CoUninitialize();
        return result;
    }

    memset(cpuId, 0, bufferLength);
    result = QueryValue(pService,
        L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId",
        cpuId, bufferLength);

    if (FAILED(result))
    {
        pService->Release();
        pLocator->Release();
        CoUninitialize();

        return result;
    }

    pService->Release();
    pLocator->Release();
    CoUninitialize();

    return NOERROR;
}

void* Get_IWbemServices_ExecQuery_funcAddress()
{
    HRESULT result = InitializeCom();
    if (FAILED(result))
        return NULL;

    IWbemLocator* pLocator = NULL;
    IWbemServices* pService = NULL;
    result = GetWbemService(&pLocator, &pService);
    if (FAILED(result))
    {
        CoUninitialize();
        return NULL;
    }
    __int64 vft = *(__int64*)pService;
    void* pFunc = *(void**)(vft + 0xA0);

    pService->Release();
    pLocator->Release();
    CoUninitialize();

    return pFunc;
}

void* Get_IWbemClassObject_Get_funcAddress()
{
    HMODULE hFastprox = NULL;
    hFastprox = LoadLibrary("fastprox.dll");

    if (hFastprox)
        return GetProcAddress(hFastprox, "?Get@CWbemObject@@UEAAJPEBGJPEAUtagVARIANT@@PEAJ2@Z");
    return NULL;
}

