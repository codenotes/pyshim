// pyshim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

DWORD enableVTMode()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
	}

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
	{
		return GetLastError();
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
	{
		return GetLastError();
	}
}
template<typename T>
string vecTostr(T begin, T end)
{
	stringstream ss;
	bool first = true;
	for (; begin != end; begin++)
	{
		if (!first)
			ss << " ";
		ss << *begin;
		first = false;
	}
	return ss.str();
}

int createProcessAndRedirOutput_nw(string exeName) //notworking
{


	
	bp::ipstream is; //reading pipe-stream
	string output;

	//printf("stringtest:%s\n", exeName.c_str());

	try
	{
		
		bp::child c(bp::search_path(exeName).c_str(),output, bp::std_out > is);
		
		std::vector<std::string> data;
		std::string line;
		
		while (c.running() && std::getline(is, line) && !line.empty())
		{
		
			data.push_back(line);
		
			cout << line << endl;
		}
	}
	catch (const std::exception&e)
	{
		cout << e.what() << endl;
	}

	

	return 0;

}

using namespace std;

void test1()
{
	string output;
	bp::ipstream is;
	string line;
	boost::asio::io_service ios;
	std::vector<char> buf;
	bp::async_pipe ap(ios);

	try
	{
		//bp::child c("c:\\python27\\python.exe", "C:\\ros_kinetic_win32\\kinetic\\bin\\rosmaster",  bp::std_out > ap);
		bp::child c(R"(c:\Users\gbrill\documents\visual studio 2017\Projects\pyshim\x64\Debug\dummyoutput.exe)", "sdf", bp::std_out > boost::asio::buffer(buf), ios);

		cout << buf << endl;
		ios.run();

		c.wait();
		int result = c.exit_code();

	}
	catch (const std::exception&e)
	{
		cout << e.what() << endl;
	}

}

void test2(string exeName)
{
	bp::opstream in;
	bp::ipstream out;

	bp::child c(exeName.c_str(), bp::std_out > out, bp::std_in < in);

//	in << "_ZN5boost7process8tutorialE" << endl;
	std::string value;
	
	while (c.running() )
	{
		out >> value;
		cout <<"!"<< value << endl;


	}
	c.wait();
	cout << "terminating..." << endl;
	c.terminate();
}

int createProcessAndRedirOutput(const string exeName, vector<string> args)
{
	bp::ipstream is; //reading pipe-stream
	bp::ipstream iserr; //reading pipe-stream
	bp::child c(exeName, args, bp::std_out > is, bp::std_err > iserr);

	std::vector<std::string> data,data_err;
	std::string line,line_err;

	while (c.running())// && std::getline(is, line) && !line.empty())
	{
		std::getline(is, line);
		std::getline(iserr, line_err);

		if (!line_err.empty())
			data_err.push_back(line_err);
		//cout << "running...\n";
		if(!line.empty())
			data.push_back(line);
	}
	

	c.wait();

	for (auto s : data)
	{
		cout << s << endl;
	}

	for (auto s : data_err)
	{
		cerr << s << endl;
	}

	return c.exit_code();
}

void testOfTests()
{
	vector<string>targs;

	//targs.push_back("arg1");
	//targs.push_back("arg2");
	//string sexe = R"(c:\Users\gbrill\documents\visual studio 2017\Projects\pyshim\x64\Debug\dummyoutput.exe)";
	string sexe = R"(c:\python27\python.exe)";
	string arg1 = R"(C:\ros_kinetic_win32\kinetic\bin\rosmaster)";

	//string arg1 = R"(C:\bin\printsomethingNoExt)";

	//c:\python27\python.exe C:\bin\printsomething.py

	targs.push_back(arg1);
	createProcessAndRedirOutput(sexe, targs);



}
int main(int argc, char**argv)
{
	enableVTMode();


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

	for (int i = 1; i < argc; i++) //all other arguments that are intended for the python file
	{
	//	cout << "arg:" << i << "->" << argv[i] << endl ;
		args.push_back(argv[i]);
	}

//	cout << endl;
	//auto str = vecTostr< vector<string>::iterator >(args.begin(), args.end());


//	cout << "we would call:" << "python "<<p.filename() << " " << str << endl;
	//stringstream ss;
//	ss  << "python " << p.filename() << " " << str;
	//cout << "sys call is:" << ss.str() << endl;
//	int result = bp::system(ss.str());

	return createProcessAndRedirOutput(pypath.string(),args);
	    
}

