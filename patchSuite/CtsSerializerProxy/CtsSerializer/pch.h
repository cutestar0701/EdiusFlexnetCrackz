// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

void InstallBridge();
void wmi_install_patch();
void wmi_uninstall();
void kgh_log(char* fmt, ...);

#define KGH_LOG_ENABLE 1
#define GVKK_LICENSEKEY "412019-E9GG97005E6LQY2R"
void* Get_IWbemServices_ExecQuery_funcAddress();
void* Get_IWbemClassObject_Get_funcAddress();
#endif //PCH_H
