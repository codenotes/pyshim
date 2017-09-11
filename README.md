# pyshim
In windows we can't run python files without invoking the python.exe, but we may need to. In my case, I have a large python library I am 
porting to windows, and it attempts to run python files as if they were processes. So it is failing. 

For example, on *nix we might have a python file called, just "somepythoncode".  It may not even have a .py extension.  But its 
first line will be:

#!/usr/bin/python

and it can be run on *nix as if it were an exe.  On Windows, not.

So pyshim.exe is a C++ console program that you rename to be the python file you wish to run on windows. For example, if there
was a python file called "somepyfile", you would rename pyshim.exe to somepyfile.exe. 

When the renamied physim.exe is run, it will:

1) verify python.exe is in your path
2) internally shell execute "python somepyfile [and any command line arguments]"
3) redirect stdout and stderr from the python file.

So in short, it will enable you to "run" any python file as if it were an .exe without needing py2exe or some other, heavy, utility.


