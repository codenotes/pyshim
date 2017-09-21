// Win32CreateProcessWIthRedir.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#define BUFSIZE 4096 

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;

HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
HANDLE g_hChildStd_ERR_Rd = NULL;
HANDLE g_hChildStd_ERR_Wr = NULL;

HANDLE g_hInputFile = NULL;

int gTimeout = 0;


//void CreateChildProcess(TCHAR * sz);
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




void CreateChildProcess(TCHAR * szCmdlineIn, HANDLE ghJob, HANDLE hStdout, HANDLE hStderr) // Create a child process that uses the previously created pipes for STDIN and STDOUT.
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

	//siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	//siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;

	siStartInfo.hStdError = hStderr;
	siStartInfo.hStdOutput = hStdout;
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
	
	DWORD mode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
	::SetNamedPipeHandleState(hStdout, &mode, NULL, NULL);
	::SetNamedPipeHandleState(hStderr, &mode, NULL, NULL);

	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 

		//FALSE,          // handles are inherited 
		TRUE,

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



std::string vectostring(std::vector<std::string> &vec)
{
	std::ostringstream oss;

	for (auto s : vec)
		oss << s << " ";

	return oss.str();
}



namespace po = boost::program_options;



#undef TEXT
#define TEXT

void CreateChildProcessWithOutAndErr(TCHAR * szCmdlineIn, HANDLE ghJob, HANDLE hStdout, HANDLE hStderr) // Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	TCHAR szCmdline[512]; //= TEXT("child");
	strcpy(szCmdline, szCmdlineIn);

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;


	SECURITY_ATTRIBUTES saAttr;

//	printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ErrorExit(TEXT("StdoutRd CreatePipe"));
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdout SetHandleInformation"));


	if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &saAttr, 0))
		ErrorExit(TEXT("StdoutErr CreatePipe"));
	if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit(TEXT("Stdin CreatePipe"));
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdin SetHandleInformation"));

//	TCHAR szCmdline[] = TEXT("child");
	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_ERR_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = NULL; // g_hChildStd_IN_Rd;
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
	if (!bSuccess)
		ErrorExit(TEXT("CreateProcess"));

	
		//add to job if there is a job
	if (ghJob)
		AssignProcessToJobObject(ghJob, piProcInfo.hProcess);
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 


		// Read output from the child process's pipe for STDOUT
		// and write to the parent process's pipe for STDOUT. 
		// Stop when there is no more data. 
		
		DWORD dwRead, dwWritten;
		CHAR chBuf[BUFSIZE];
		DWORD dwRead2, dwWritten2;
		CHAR chBuf2[BUFSIZE];
		BOOL bSuccess2 = FALSE;
		DWORD avail = 0;

		HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE hParentStdErr = GetStdHandle(STD_ERROR_HANDLE);
		//setvbuf(stdout, NULL, _IONBF, 0) , if at begginingg of child process will solve it but ick...can I do this without fucking with the child process?
		int i = 0;
		for (;;)
		{

			auto ret=WaitForSingleObject(piProcInfo.hProcess, 0);

			if (ret == WAIT_TIMEOUT) //still running
			{
			
			}
			else if (ret == WAIT_OBJECT_0) //it is dead
			{
				//printf("so long child...host existing\n");
				break;

			}
			

			//withoug this clause err and out clobber each other
			std::this_thread::sleep_for(std::chrono::milliseconds(200)); //seems to be the right number...I dunno...
			
			//see if out has and if so read
			bSuccess = PeekNamedPipe(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, &avail, NULL);
			if (avail>0)
				bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);

			//see if stderr has and if so read
			bSuccess2 = PeekNamedPipe(g_hChildStd_ERR_Rd, chBuf2, BUFSIZE, &dwRead2,& avail, NULL);
			if(avail>0)
				bSuccess2 = ReadFile(g_hChildStd_ERR_Rd, chBuf2, BUFSIZE, &dwRead2, NULL);

			//	std::this_thread::sleep_for(std::chrono::milliseconds(200));
			//printf("%d %d i:%d\n", dwRead, dwRead2,i);

			if (dwRead > 0) {

				bSuccess = WriteFile(hParentStdOut, chBuf,	dwRead, &dwWritten, NULL);
			
			}

//			std::this_thread::sleep_for(std::chrono::milliseconds(20));

			if (dwRead2 > 0) {

				bSuccess = WriteFile(hParentStdErr, chBuf2,	dwRead2, &dwWritten2, NULL);
			}

			

			if (!bSuccess || !bSuccess2) //never is true
			{
				printf("OH NO!!! Read failure!\n");
				break;
			}


			i++;
		
		
		}


		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

}



//std::string vectostring(std::vector<std::string> &vec)
//{
//	std::ostringstream oss;
//
//	for (auto s : vec)
//		oss << s << " ";
//
//	return oss.str();
//}



namespace po = boost::program_options;







int main(int argc, char **argv)
{
	SetConsoleMode(GetStdHandle(-11), 7);

	int opt;
	string fileToShim;
	bool quoteify=false;
	
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("st,S", po::value<string>(&fileToShim)->default_value(""),   "shim this file, st==somepyfile, or -Sthefile")
		("normal-args", po::value<std::vector<std::string>>(), "normal arguments, you don't call this, it is positional")
		("quoteify,q", "put quotes around all args")
		("timeout,T", po::value<int>(&gTimeout)->default_value(0),"when should timeout occur and host return if no output? NOOP")

	//	("include-path,I", po::value< vector<string> >(), 	  "include path")
//		("input-file", po::value< vector<string> >(), "input file")
;

	po::positional_options_description pod;
	pod.add("normal-args", -1);

	po::variables_map vm;
	try
	{
	//	po::store(po::parse_command_line(argc, argv, desc), vm);
		po::store(po::command_line_parser(argc, argv).
			options(desc).positional(pod).run(), vm);
		po::notify(vm);

	}
	catch (const std::exception&e)
	{
		cerr << e.what() << endl;
		return 0;
	}

	std::ostringstream ossArgs;
	
	if (vm.count("quoteify"))
		quoteify = true;

	if (vm.count("normal-args"))
	{
		std::vector<std::string> files = vm["normal-args"].as<std::vector<std::string>>();
		for (std::string file : files) 
		{
			if(!quoteify)
				ossArgs << file <<" " ;
			else
				ossArgs << "\""+file + "\"" << " ";

		}
	}

	if (vm.count("help")) 
	{
		cout << desc << "\n";
		return 1;
	}


	//cout << ossArgs.str() << endl;
		
	path p{ argv[0] };
	vector<string> args;
	auto pypath = bp::search_path("python");

	if (bp::search_path("python") == "")
	{
		cout << "Python.exe not found, so this can't work.  Make sure it is in the PATH." << endl;
		return 0;
	}
	//cout << "my python name is:" << p.filename() << " and I am located at:"<< p.root_directory() << endl;

	p.replace_extension(""); //make ours python file the same name as this renamed one, and with no extension so that its like what bash does

	args.push_back(p.string());//first argument should be the renamed python file which is really the name of this renamed exe

	//for (int i = 1; i < argc; i++) //all other arguments that are intended for the python file
	//{
	//	//	cout << "arg:" << i << "->" << argv[i] << endl ;
	//	args.push_back(argv[i]);
	//}

	

	std::ostringstream oss;
	std::string allArgs = vectostring(args);
	

	if (fileToShim.empty())
	{
		
		oss << pypath << " \"" << argv[0]<<"\" "<<ossArgs.str();
		//cout << "just go with:" << argv[0] << endl;
	}
	else {
		oss << pypath <<" " <<fileToShim << " " << ossArgs.str();
		//cout << "dont, do this..not implemented. Shim file:" << fileToShim << endl;
	
	}
	
	/*cout << oss.str() << endl;
	return 0;
	*/

	//// Create the child process. 
	HANDLE job = createJob();

	auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	auto hStderr = GetStdHandle(STD_ERROR_HANDLE);


	
	if (job)
		AssignProcessToJobObject(job, GetCurrentProcess());
	
	char * buff=	 new char[oss.str().size() + 1];
	strcpy(buff, oss.str().c_str());
//	CreateChildProcess(argv[1],job, hStdout, hStderr); //for rosout and such
	cerr << buff << endl;
	CreateChildProcessWithOutAndErr(buff, job, hStdout, hStderr); //redirected pipes to parent
    return 0;
}



//if (
//	(hStdout == INVALID_HANDLE_VALUE) ||
//	(hStderr == INVALID_HANDLE_VALUE)
//	)
//	printf("bad news...\n");
//DWORD did;

//yay! worked from the python call
//WriteFile(hStdout, "Test string\n",12,&did,0);
//CloseHandle(hStdout);



/*auto e = GetEnvString();

replace(e.begin(), e.end(), '=', '\n');
std::cout << e << std::endl;
*/
//	cptest(argc, argv);
//	return 0;
#if 0
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
#endif