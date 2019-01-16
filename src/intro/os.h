
/** @example Intro/os.h
 */

/* 
 * OS-specific methods for abstracting.  
 * Copyright 2008-2019 Solace Corporation. All rights reserved. 
 */

#ifndef ___OS_H_
#define ___OS_H_

#ifdef __cplusplus
extern          "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#define _WIN32_WINNT 0x400      /* Require Windows NT5 (2000, XP, 2003) for SignalObjectAndWait */
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>

#define SLEEP(sec)  Sleep ( (sec) * 1000 )
#define strcasecmp (_stricmp)
#define strncasecmp (_strnicmp)
#else
#include <unistd.h>

#define SLEEP(sec) sleep ( (sec) )
#endif



#ifdef __cplusplus
}
#endif

#endif
