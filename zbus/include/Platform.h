#ifndef __ZBUS_PLATFORM_H__
#define __ZBUS_PLATFORM_H__  

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <mutex> 
#include <thread>
#include <exception>


#if (defined WIN32 || defined _WIN32)
#   undef __WINDOWS__
#   define __WINDOWS__
#endif 

 
#if (defined (unix) || defined (__unix__) || defined (_POSIX_SOURCE))
#   if (!defined (__VMS__))
#       undef __UNIX__
#       define __UNIX__
#   endif
#endif


#if defined (__WINDOWS__) 

#include <winsock2.h>
#include <WS2tcpip.h> 
#include <direct.h>
#include <windows.h>
#include <winsock.h>
#include <process.h>
#include <signal.h>  
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <share.h>

#if (!defined(FD_SETSIZE))
#define FD_SETSIZE 1024  
#endif

#endif  
 

#if (defined (__UNIX__))
#   include <fcntl.h>
#   include <netdb.h>
#   include <unistd.h> 
#   include <dirent.h>
#   include <pwd.h>
#   include <grp.h>
#   include <utime.h>
#   include <syslog.h>
#   include <inttypes.h>
#   include <sys/types.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <sys/ioctl.h>
#   include <sys/file.h>
#   include <sys/wait.h>
#   include <netinet/in.h>              
#endif
 


#if defined (__WINDOWS__)     
#   define ZBUS_API __declspec(dllexport) 
#else
#   define ZBUS_API
#endif

	 

#endif
