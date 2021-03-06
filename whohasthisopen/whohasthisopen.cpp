// whohasthisopen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib,"Rstrtmgr.lib")

int __cdecl whohasthis(int argc, WCHAR **argv)
{
	DWORD dwSession;
	WCHAR szSessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
	DWORD dwError = RmStartSession(&dwSession, 0, szSessionKey);
	wprintf(L"RmStartSession returned %d\n", dwError);
	if (dwError == ERROR_SUCCESS) {
		PCWSTR pszFile = argv[1];
		dwError = RmRegisterResources(dwSession, 1, &pszFile,
			0, NULL, 0, NULL);
		wprintf(L"RmRegisterResources(%ls) returned %d\n",
			pszFile, dwError);
		if (dwError == ERROR_SUCCESS) {
			DWORD dwReason;
			UINT i;
			UINT nProcInfoNeeded;
			UINT nProcInfo = 10;
			RM_PROCESS_INFO rgpi[10];
			dwError = RmGetList(dwSession, &nProcInfoNeeded,
				&nProcInfo, rgpi, &dwReason);
			wprintf(L"RmGetList returned %d\n", dwError);
			if (dwError == ERROR_SUCCESS) {
				wprintf(L"RmGetList returned %d infos (%d needed)\n",
					nProcInfo, nProcInfoNeeded);
				for (i = 0; i < nProcInfo; i++) {
					wprintf(L"%d.ApplicationType = %d\n", i,
						rgpi[i].ApplicationType);
					wprintf(L"%d.strAppName = %ls\n", i,
						rgpi[i].strAppName);
					wprintf(L"%d.Process.dwProcessId = %d\n", i,
						rgpi[i].Process.dwProcessId);
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
						FALSE, rgpi[i].Process.dwProcessId);
					if (hProcess) {
						FILETIME ftCreate, ftExit, ftKernel, ftUser;
						if (GetProcessTimes(hProcess, &ftCreate, &ftExit,
							&ftKernel, &ftUser) &&
							CompareFileTime(&rgpi[i].Process.ProcessStartTime,
								&ftCreate) == 0) {
							WCHAR sz[MAX_PATH];
							DWORD cch = MAX_PATH;
							if (QueryFullProcessImageNameW(hProcess, 0, sz, &cch) &&
								cch <= MAX_PATH) {
								wprintf(L"  = %ls\n", sz);
							}
						}
						CloseHandle(hProcess);
					}
				}
			}
		}
		RmEndSession(dwSession);
	}
	return 0;
}

int wmain(int argc, WCHAR **argv)
{
	whohasthis(argc, argv);
    return 0;
}

