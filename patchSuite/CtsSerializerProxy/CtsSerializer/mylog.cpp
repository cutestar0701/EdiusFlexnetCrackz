#include "pch.h"
#include <stdio.h >

void kgh_log(char* fmt, ...)
{
#if KGH_LOG_ENABLE
	va_list va;

	va_start(va, fmt);
#if 0
	char pchlog[1024] = { 0 };
	vsnprintf_s(pchlog, sizeof(pchlog), sizeof(pchlog), fmt, va);
#endif

	FILE* fp = fopen("D:\\kgh_oneshotlog.txt", "a+");
	if (fp)
	{
		char exefilename[MAX_PATH];
		GetModuleFileNameA(NULL, exefilename,MAX_PATH);
		SYSTEMTIME st;
		GetSystemTime(&st);

		fprintf(fp,"[%s] (%04d/%02d/%02d %02d:%02d:%02d) ", exefilename, st.wYear, st.wMonth, st.wDay, st.wHour + 9, st.wMinute, st.wSecond);
		vfprintf(fp, fmt, va);
		fclose(fp);
	}
	va_end(va);
#endif
}
