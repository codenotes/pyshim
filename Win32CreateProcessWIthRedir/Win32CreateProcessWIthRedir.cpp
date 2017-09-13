// Win32CreateProcessWIthRedir.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#define BUFSIZE 4096 

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;




void CreateChildProcess(TCHAR * sz);
void WriteToPipe(void);
void ReadFromPipe(void);
void ErrorExit(PTSTR);

int asd(int argc, TCHAR *argv[])
{
	SECURITY_ATTRIBUTES saAttr;

	printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ErrorExit(TEXT("StdoutRd CreatePipe"));

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit(TEXT("Stdin CreatePipe"));

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdin SetHandleInformation"));

	// Create the child process. 

	//CreateChildProcess();

	// Get a handle to an input file for the parent. 
	// This example assumes a plain text file and uses string output to verify data flow. 

	if (argc == 1)
		ErrorExit(TEXT("Please specify an input file.\n"));

	g_hInputFile = CreateFile(
		argv[1],
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);

	if (g_hInputFile == INVALID_HANDLE_VALUE)
		ErrorExit(TEXT("CreateFile"));

	// Write to the pipe that is the standard input for a child process. 
	// Data is written to the pipe's buffers, so it is not necessary to wait
	// until the child process is running before writing data.

	WriteToPipe();
//	printf("\n->Contents of %s written to child STDIN pipe.\n", argv[1]);

	// Read from pipe that is the standard output for child process. 

//	printf("\n->Contents of child process STDOUT:\n\n", argv[1]);
	ReadFromPipe();

	printf("\n->End of parent execution.\n");

	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 

	return 0;
}

#include <string>

std::string GetEnvString()
{
	char * env = GetEnvironmentStrings();

	if (!env)
		abort();
	const char* var = env;
	size_t totallen = 0;
	size_t len;
	while ((len = strlen(var)) > 0)
	{
		totallen += len + 1;
		var += len + 1;
	}
	std::string result(env, totallen);
	FreeEnvironmentStrings(env);
	return result;
}


void cptest(int argc, TCHAR *argv[])
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (argc != 2)
	{
		printf("Usage: %s [cmdline]\n", argv[0]);
		return;
	}

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		argv[1],        // Command line
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
		return;
	}

	// Wait until child process exits.
//	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void CreateChildProcess(TCHAR * szCmdlineIn, HANDLE ghJob=NULL)
	// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	TCHAR szCmdline[512]; //= TEXT("child");
	strcpy(szCmdline, szCmdlineIn);
		
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
	siStartInfo.hStdInput = NULL;// g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 


	//TCHAR lpszClientPath[500] = TEXT("c:\\temp\\rosout.exe");
	//if (!CreateProcess(NULL, lpszClientPath, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &siStartInfo, &piProcInfo))
	//{ 
	//	printf("CreateProcess failed (%d).\n", GetLastError());
	//	return;
	//}

	//std::wstring env = GetEnvString();
	
	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		FALSE,          // handles are inherited 
		CREATE_NEW_CONSOLE,//| CREATE_UNICODE_ENVIRONMENT,//0,             // creation flags //NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | 
		0,//(LPVOID)env.c_str(),          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

						// If an error occurs, exit the application. 
	if (!bSuccess)
		ErrorExit(TEXT("CreateProcess"));
	else
	{
		//add to job if there is a job
		if (ghJob)
			AssignProcessToJobObject(ghJob, piProcInfo.hProcess);
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		WaitForSingleObject(piProcInfo.hProcess, INFINITE);

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}

void WriteToPipe(void)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;

	for (;;)
	{
		bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}

	// Close the pipe handle so the child process stops reading. 

	if (!CloseHandle(g_hChildStd_IN_Wr))
		ErrorExit(TEXT("StdInWr CloseHandle"));
}

void ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(hParentStdOut, chBuf,
			dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}
}

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}

#include <algorithm>
#include <iostream>
#include <string>

//not called directly, just for reference.
void AddChildToJob(HANDLE ghJob)
{
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;

	// Launch child process - example is notepad.exe
	if (::CreateProcess(NULL, "notepad.exe", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	{
		::MessageBox(0, "CreateProcess succeeded.", "TEST", MB_OK);
		if (ghJob)
		{
			if (0 == AssignProcessToJobObject(ghJob, processInfo.hProcess))
			{
				::MessageBox(0, "Could not AssignProcessToObject", "TEST", MB_OK);
			}
		}

		// Can we free handles now? Not sure about this.
		//CloseHandle(processInfo.hProcess); 
		CloseHandle(processInfo.hThread);
	}
}

HANDLE createJob()
{

	/*accepted
		The Windows API supports objects called "Job Objects".The following code will create a "job" that is configured to shut down all processes when the main application ends(when its handles are cleaned up).This code should only be run once.:
*/
	HANDLE ghJob = CreateJobObject(NULL, NULL); // GLOBAL
	if (ghJob == NULL)
	{
		::MessageBox(0, "Could not create job object", "TEST", MB_OK);
	}
	else
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

		// Configure all child processes associated with the job to terminate when the
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (0 == SetInformationJobObject(ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		{
			::MessageBox(0, "Could not SetInformationJobObject", "TEST", MB_OK);
		}
	}

	return ghJob;
}

int main(int argc, char **argv)
{
	/*auto e = GetEnvString();
	
	replace(e.begin(), e.end(), '=', '\n');
	std::cout << e << std::endl;
*/
//	cptest(argc, argv);
//	return 0;

	//SECURITY_ATTRIBUTES saAttr;

	//printf("\n->Start of parent execution.\n");

	//// Set the bInheritHandle flag so pipe handles are inherited. 

	//saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	//saAttr.bInheritHandle = TRUE;
	//saAttr.lpSecurityDescriptor = NULL;

	//// Create a pipe for the child process's STDOUT. 

	//if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
	//	ErrorExit(TEXT("StdoutRd CreatePipe"));

	//// Ensure the read handle to the pipe for STDOUT is not inherited.

	//if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	//	ErrorExit(TEXT("Stdout SetHandleInformation"));

	//// Create a pipe for the child process's STDIN. 

	//if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
	//	ErrorExit(TEXT("Stdin CreatePipe"));

	//// Ensure the write handle to the pipe for STDIN is not inherited. 

	//if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
	//	ErrorExit(TEXT("Stdin SetHandleInformation"));

	//// Create the child process. 
	HANDLE job = createJob();

	if (job)
		AssignProcessToJobObject(job, GetCurrentProcess());

	
	CreateChildProcess("c:\\temp\\rosout.exe",job);
    return 0;
}

