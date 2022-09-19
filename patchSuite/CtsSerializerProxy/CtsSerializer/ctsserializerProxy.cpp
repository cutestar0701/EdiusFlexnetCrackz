#include "pch.h"

#define KGH_ORI_CTSSERIALIZER_DLL_NAME "CtsSerializerOri.dll"

typedef HRESULT (__stdcall *pfn_DllCanUnloadNow)();
typedef HRESULT (__stdcall *pfn_DllGetClassObject)(const IID* const rclsid, const IID* const riid, LPVOID* ppv);
typedef HRESULT (__stdcall *pfn_DllRegisterServer)();
typedef HRESULT (__stdcall *pfn_DllUnregisterServer)();

pfn_DllCanUnloadNow g_pfn_DllCanUnloadNow;
pfn_DllGetClassObject g_pfn_DllGetClassObject;
pfn_DllRegisterServer g_pfn_DllRegisterServer;
pfn_DllUnregisterServer g_pfn_DllUnregisterServer;
int g_isLoaded = 0;

extern "C" HRESULT  __declspec(dllexport) __stdcall DllCanUnloadNow()
{
	InstallBridge();
	return g_pfn_DllCanUnloadNow();
}
extern "C" HRESULT __declspec(dllexport) __stdcall DllGetClassObject(const IID* const rclsid, const IID* const riid, LPVOID * ppv)
{
	InstallBridge();
	return g_pfn_DllGetClassObject(rclsid, riid, ppv);
}
extern "C" HRESULT __declspec(dllexport) __stdcall DllRegisterServer()
{
	InstallBridge();
	return g_pfn_DllRegisterServer();
}
extern "C" HRESULT __declspec(dllexport) __stdcall DllUnregisterServer()
{
	InstallBridge();
	return g_pfn_DllUnregisterServer();
}

void InstallBridge()
{
	if (g_isLoaded == 1)
		return;
	
	HMODULE hmod = LoadLibrary(KGH_ORI_CTSSERIALIZER_DLL_NAME);
	if (hmod == NULL)
	{
		MessageBoxA(0, "Can't find original dll.", KGH_ORI_CTSSERIALIZER_DLL_NAME, 0);
		return;
	}
	g_pfn_DllCanUnloadNow = (pfn_DllCanUnloadNow)GetProcAddress(hmod, "DllCanUnloadNow");
	g_pfn_DllGetClassObject = (pfn_DllGetClassObject)GetProcAddress(hmod, "DllGetClassObject");
	g_pfn_DllRegisterServer = (pfn_DllRegisterServer)GetProcAddress(hmod, "DllRegisterServer");
	g_pfn_DllUnregisterServer = (pfn_DllUnregisterServer)GetProcAddress(hmod, "DllUnregisterServer");
	g_isLoaded = 1;
	kgh_log((char*)"Installed Bridge\n");
}

