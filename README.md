# pyshim
In windows we can't run python files without invoking the python executable, but we may need to. In my case, I have a large python library I am porting to windows, and it attempts to run python files as if they were executable. So it is failing. 

For example, on *nix we might have a python file called, just "somepythoncode".  It may not even have a .py extension.  But its 
first line will be:

#!/usr/bin/python

and it can be run on *nix as if it were an exe.  On Windows, not. 

So pyshim.exe is a C++ console program that you rename to be the python file you wish to run on windows. Place it in the same directory as the python file with the same name. For example, if there was a python file called "somepyfile", you would rename pyshim.exe to somepyfile.exe and put it in the same directory as "somepyfile". 

In this way, anything wanting to execute "somepyfile" or "somepyfile.exe" will always get the executable. The executable will then do a 
sleight of hand. Specifically, when this renamed physim.exe is run, it will:

1) verify python.exe is in your path
2) internally shell execute "python somepyfile [and any command line arguments you pass in]"
3) redirect stdout and stderr from the python file.

So in short, it will enable you to "run" any python file as if it were an .exe without needing py2exe or some other, heavy, utility. In this was CreateProcess, or popen, etc. can operate on and execute the python file directly on windows. 

NOTE: boost 1_65 libraries are required because of the process management being done.  

# whodowhat

This is a win64 exe that will investigate every process that has a TCP port open and (optionally) what DLLs it has loaded. 

It is a fast, concise program to get the facts quickly.  It will print both PID and process name. And it is instant.  Also color coded (on windows 10, it uses ANSI capabilities). 



