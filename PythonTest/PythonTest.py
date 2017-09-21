from subprocess import *
import os
import sys
import signal
import subprocess 
import time
import traceback

import win32process
import win32api
import win32con


import rospkg

from roslaunch.core import *
#from roslaunch.core import setup_env
from roslaunch.node_args import create_local_process_args
from roslaunch.pmon import Process, FatalProcessLaunch
from rosmaster.master_api import NUM_WORKERS

if sys.platform == "win32":
    import msvcrt
    import _subprocess
else:
    import fcntl




#C:\Users\gbrill\Documents\Visual Studio 2017\Projects\pyshim\x64\Debug\Win32CreateProcessWIthRedir.exe

#logfileout, logfileerr = subprocess.PIPE, subprocess.PIPE


#name = "%s-%s"%(rosgraph.names.ns_join(node.namespace, node.name), _next_counter())
#if name[0] == '/':
#    name = name[1:]

#def _log_name(self):
#    return self.name.replace('/', '-')




def _configure_logging():
        """
        Configure logging of node's log file and stdout/stderr
        @return: stdout log file name, stderr log file
        name. Values are None if stdout/stderr are not logged.
        @rtype: str, str
        """    
        log_dir = rospkg.get_log_dir(env=os.environ)

        #if self.run_id:
        #    log_dir = os.path.join(log_dir, self.run_id)

        if not os.path.exists(log_dir):
            try:
                os.makedirs(log_dir)
            except OSError as e:
                if e.errno == 13:
                    raise RLException("unable to create directory for log file [%s].\nPlease check permissions."%log_dir)
                else:
                    raise RLException("unable to create directory for log file [%s]: %s"%(log_dir, e.strerror))
        # #973: save log dir for error messages
        #self.log_dir = log_dir

        # send stdout/stderr to file. in the case of respawning, we have to
        # open in append mode
        # note: logfileerr: disabling in favor of stderr appearing in the console.
        # will likely reinstate once roserr/rosout is more properly used.
        logfileout = logfileerr = None
        logfname = 'boob' #_log_name()
        


        #if self.log_output:
        #    outf, errf = [os.path.join(log_dir, '%s-%s.log'%(logfname, n)) for n in ['stdout', 'stderr']]
        #    if self.respawn:
        #        mode = 'a'
        #    else:
        #        mode = 'w'
        outf, errf = [os.path.join(log_dir, '%s-%s.log'%(logfname, n)) for n in ['stdout', 'stderr']]
        logfileout = open(outf, 'w')
        logfileerr = open(errf, 'w')

        # #986: pass in logfile name to node
        node_log_file = log_dir
        #if self.is_node:
        #    # #1595: on respawn, these keep appending
        #    self.args = _cleanup_remappings(self.args, '__log:=')
        #    self.args.append("__log:=%s"%os.path.join(log_dir, "%s.log"%(logfname)))

        return logfileout, logfileerr






#go()

def rawCreateProcess(cmd, login=None,
                 hStdin=None, hStdout=None, hStderr=None,
                 show=1, xy=None, xySize=None,
                 desktop=None):
        """
        Create a Windows process.
        cmd:     command to run
        login:   run as user 'Domain\nUser\nPassword'
        hStdin, hStdout, hStderr:
                 handles for process I/O; default is caller's stdin,
                 stdout & stderr
        show:    wShowWindow (0=SW_HIDE, 1=SW_NORMAL, ...)
        xy:      window offset (x, y) of upper left corner in pixels
        xySize:  window size (width, height) in pixels
        desktop: lpDesktop - name of desktop e.g. 'winsta0\\default'
                 None = inherit current desktop
                 '' = create new desktop if necessary

        User calling login requires additional privileges:
          Act as part of the operating system [not needed on Windows XP]
          Increase quotas
          Replace a process level token
        Login string must EITHER be an administrator's account
        (ordinary user can't access current desktop - see Microsoft
        Q165194) OR use desktop='' to run another desktop invisibly
        (may be very slow to startup & finalize).
        """
        print ('creating',cmd)
        si = win32process.STARTUPINFO()
        #si.dwFlags = (win32con.STARTF_USESTDHANDLES ^
        #              win32con.STARTF_USESHOWWINDOW) | win32con.STARTF_USESTDHANDLES
        si.dwFlags = win32con.STARTF_USESTDHANDLES
        


        if hStdin is None:
           # si.hStdInput = win32api.GetStdHandle(win32api.STD_INPUT_HANDLE)
           si.hStdInput = None
        else:
            si.hStdInput = hStdin
        if hStdout is None:
            si.hStdOutput=None
            #si.hStdOutput = win32api.GetStdHandle(win32api.STD_OUTPUT_HANDLE)
        else:
            si.hStdOutput = hStdout
        if hStderr is None:
            si.hStdError = None
            #si.hStdError = win32api.GetStdHandle(win32api.STD_ERROR_HANDLE)
        else:
            si.hStdError = hStderr
        si.wShowWindow = show
        if xy is not None:
            si.dwX, si.dwY = xy
            si.dwFlags ^= win32con.STARTF_USEPOSITION
        if xySize is not None:
            si.dwXSize, si.dwYSize = xySize
            si.dwFlags ^= win32con.STARTF_USESIZE
        if desktop is not None:
            si.lpDesktop = desktop
        procArgs = (None,  # appName
                    cmd,  # commandLine
                    None,  # processAttributes
                    None,  # threadAttributes
                    1,  # bInheritHandles
                    #win32process.CREATE_NO_WINDOW,    
                    win32process.CREATE_NEW_CONSOLE,
                    None,  # newEnvironment
                    None,  # currentDirectory
                    si)  # startupinfo
        if login is not None:
            hUser = logonUser(login)
            win32security.ImpersonateLoggedOnUser(hUser)
            procHandles = win32process.CreateProcessAsUser(hUser, *procArgs)
            win32security.RevertToSelf()
        else:
            procHandles = win32process.CreateProcess(*procArgs)
        #self.hProcess, self.hThread, self.PId, self.TId = procHandles
        return procHandles[2]


import win32pipe, win32file

import win32file
import win32security

def rawCreateFile(f):

        sa = win32security.SECURITY_ATTRIBUTES()
        sa.bInheritHandle=True
        # sa.lpSecurityDescriptor = NULL;


        hand = win32file.CreateFile(
            f,
            win32con.FILE_SHARE_WRITE,
            win32con.FILE_SHARE_READ | win32con.FILE_SHARE_WRITE,
            sa,#,
            win32con.OPEN_ALWAYS,
            win32con.FILE_ATTRIBUTE_NORMAL,
            None)
        
        return hand



def go():
    try:
        logfileout, logfileerr = _configure_logging()
    except Exception as e:
        print 'nope, using PIPE for popen'
        #_logger.error(traceback.format_exc())
        #printerrlog("[%s] ERROR: unable to configure logging [%s]"%(self.name, str(e)))
        # it's not safe to inherit from this process as
        # rostest changes stdout to a StringIO, which is not a
        # proper file.
        logfileout, logfileerr = subprocess.PIPE, subprocess.PIPE

    
    #args=['C:/Users/gbrill/Documents/Visual Studio 2017/Projects/pyshim/x64/Debug/Win32CreateProcessWIthRedir.exe', 'c:/temp/rosout']
    #cwd=os.path.dirname(args[0])
    full_env=os.environ
    #subprocess.Popen(args, cwd=cwd, stdout=logfileout, stderr=logfileerr, env=full_env)#, close_fds=True)
    
    #try alternative
    args=['c:/temp/rosout.exe']

   # p = win32pipe.CreateNamedPipe(r'\\.\pipe\test_pipe',
   # win32pipe.PIPE_ACCESS_DUPLEX,
   # win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
   # 1, 65536, 65536,300,None)

   # p=win32pipe.CreatePipe()
       
   # SECURITY_ATTRIBUTES saAttr; 
   #  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   #saAttr.bInheritHandle = TRUE; 
   #saAttr.lpSecurityDescriptor = NULL; 

   # CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 


    testErrorHandle = msvcrt.get_osfhandle(logfileerr.fileno())
    testStdHandle = msvcrt.get_osfhandle(logfileout.fileno())
    
    #pipeoutih = _subprocess.DuplicateHandle(curproc, pipeouth, curproc, 0, 1,
    #        _subprocess.DUPLICATE_SAME_ACCESS)


    #testHandle=rawCreateFile("C:/Users/gbrill/.ros/log/fucknut.log")
    
    #curproc = _subprocess.GetCurrentProcess()
    #dupHandle = _subprocess.DuplicateHandle(curproc, testHandle, curproc, 0, 1, _subprocess.DUPLICATE_SAME_ACCESS)
    #print ( 'dup handle is:',dupHandle)


    pid=rawCreateProcess(args[0],hStdout=testStdHandle, hStderr=testErrorHandle)
    return pid
  #  pid=rawCreateProcess(args[0],hStderr=logfileout)

    #pid=rawCreateProcess(args[0],hStdout=logfileout,hStderr=logfileerr)
    #pid=rawCreateProcess(args[0])
    

print ('about to go...')
print go()



#job = winprocess.CreateJobObject()

