/**
 *  Copyright 2015 Solace Systems, Inc. All rights reserved.
 * 
 *  http://www.solacesystems.com
 * 
 *  This source is distributed under the terms and conditions of
 *  any contract or license agreement between Solace Systems, Inc.
 *  ("Solace") and you or your company. If there are no licenses or
 *  contracts in place use of this source is not authorized. This 
 *  source is provided as is and is not supported by Solace unless
 *  such support is provided for under an agreement signed between 
 *  you and Solace.
 */

#include "os.h"

#ifdef WIN32
#else
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/resource.h>
#endif
#include <errno.h>
#include <stdio.h>

/* Semaphore that will be posted to when CTRL-C is hit. */
SEM_T           ctlCSem;
BOOL            gotCtlC = FALSE;

/* 
 * Signal handler -- posts to ctlCSem 
 * If we already got a CTRL-C, then exit immediately.
 */
#ifdef WIN32
BOOL
sigHandler ( DWORD ctlType )
{
    BOOL            rc = FALSE; /* signal not handled */

    if ( ctlType == CTRL_C_EVENT ) {
        if ( !gotCtlC ) {
            gotCtlC = TRUE;
            semPost ( &ctlCSem );
            rc = TRUE;
        } else {
            exit ( 0 );
        }
    }
    return rc;
}
#else
static void
sigHandler ( int sigNum )
{
    if ( sigNum == SIGINT ) {
        if ( !gotCtlC ) {
            gotCtlC = TRUE;
            semPost ( &ctlCSem );
        } else {
            exit ( 0 );
        }
    }
}
#endif

void
initSigHandler ( void )
{
    semInit ( &ctlCSem, 0, 1 );
#ifdef WIN32
    if ( SetConsoleCtrlHandler ( ( PHANDLER_ROUTINE ) sigHandler, TRUE /* add */  ) == 0 ) {
        printf ( "Could not initialize Control C handler\n" );
    }
#else
    if ( signal ( SIGINT, sigHandler ) == SIG_ERR ) {
        printf ( "Could not initialize Control C handler\n" );
    }
#endif
}

void
sleepInSec ( int secToSleep )
{
    sleepInUs ( secToSleep * 1000000 );
}

void
sleepInUs ( int usToSleep )
{
#ifdef WIN32
    DWORD           millis = ( DWORD ) ( usToSleep / 1000 );
    DWORD           extra = ( DWORD ) ( usToSleep % 1000 );
    if ( extra > 0 ) {
        millis++;
    }

    Sleep ( millis );
#else
    struct timespec timeVal;
    struct timespec timeRem;

    timeVal.tv_sec = usToSleep / 1000000;
    timeVal.tv_nsec = ( usToSleep % 1000000 ) * 1000;
  again:
    if ( nanosleep ( &timeVal, &timeRem ) < 0 ) {
        if ( ( errno == EINTR ) && !gotCtlC ) {
            /* Nanosleep was interrupted */
            timeVal = timeRem;
            goto again;
        }
    }
#endif
}


UINT64
getTimeInUs ( void )
{

#ifdef WIN32

    FILETIME        fileTime;
    LARGE_INTEGER   theTime;

    /* Gets time in 100 nanosecond intervals */
    GetSystemTimeAsFileTime ( &fileTime );
    theTime.LowPart = fileTime.dwLowDateTime;
    theTime.HighPart = fileTime.dwHighDateTime;
    return theTime.QuadPart / 10;       /* convert to microseconds */

#else
    struct timespec tv;

    clock_gettime ( CLOCK_REALTIME, &tv );
    return ( ( UINT64 ) tv.tv_sec * ( UINT64 ) 1000000 ) + ( ( UINT64 ) tv.tv_nsec / ( UINT64 ) 1000 );

#endif

}

void
_getDateTime ( char *buf_p, int bufSize )
{

#ifdef WIN32
    SYSTEMTIME      sysTime;
    char           *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    char           *months[] = { "x", "Jan", "Feb", "Mar", "Apr", "May",
        "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    GetLocalTime ( &sysTime );
    /* Hmmm, cannot seem to get built-in formatting methods to work, so do it ourselves */
    snprintf ( buf_p, bufSize, "%s %s %2d %02d:%02d:%02d.%03ld %d",
               days[sysTime.wDayOfWeek], months[sysTime.wMonth], sysTime.wDay,
               sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, sysTime.wYear );
#else
    struct timeval  tv;
    struct tm       timeStruct;
    char            buffer[80]; /* temp staging buffer */

    gettimeofday ( &tv, NULL );
    localtime_r ( &tv.tv_sec, &timeStruct );
    strftime ( buffer, sizeof ( buffer ), "%a %b %d %T", &timeStruct );
    snprintf ( buf_p, bufSize, "%s.%03ld %d", buffer, ( tv.tv_usec + 500 ) / 1000, 1900 + timeStruct.tm_year );
#endif

}

/*
 * Also returns the amount of time waited.
 */
UINT64
waitUntil ( UINT64 nexttimeInUs )
{
    UINT64          currTime = getTimeInUs (  );
    UINT64          waitTime;
#ifdef WIN32
    UINT64          waitTimeInMSec;
#endif

    if ( currTime > nexttimeInUs ) {
        /* We're behind so just return. */
        return 0;
    }
    waitTime = nexttimeInUs - currTime;


#ifdef WIN32
    waitTimeInMSec = waitTime / 1000;
    Sleep ( ( DWORD ) waitTimeInMSec );
#else
    struct timeval  tv;
    tv.tv_sec = 0;
    tv.tv_usec = waitTime;
    select ( 0, 0, 0, 0, &tv );
#endif
    return waitTime;
}

volatile UINT64
numClockCycles ( void )
{
#ifdef _LINUX_BUILD
    UINT64          cycles;
#if _LINUX_X86_64
    solClient_uint32_t hi, lo;
    __asm__         __volatile__ ( "rdtsc":"=a" ( lo ), "=d" ( hi ) );
    cycles = ( ( solClient_uint64_t ) hi ) << 32 | ( solClient_uint64_t ) lo;
#else
    __asm__ volatile ( ".byte 0x0f,0x31":"=A" ( cycles ) );
#endif
    return cycles;
#elif WIN32
    LARGE_INTEGER   time;
    QueryPerformanceCounter ( &time );
    return time.QuadPart;
#else
    /*
     * See not in getCpuSpeedInHz!!  Return getTimeInUs to allow it to work
     * on non linux platforms.
     */
    return getTimeInUs (  );
#endif
}                               /* End of method numClockCycles */

UINT64
getCpuUsageInUs (  )
{
#if ! defined (WIN32) && ! defined (__VMS)
    struct rusage   resUsage;
    memset ( &resUsage, 0, sizeof ( resUsage ) );
    int             retval = getrusage ( RUSAGE_SELF, &resUsage );
    if ( retval != 0 )
        perror ( "getrusage" );
    UINT64          usage =
            ( UINT64 ) ( resUsage.ru_utime.tv_sec ) * 1000000LL +
            ( UINT64 ) ( resUsage.ru_utime.tv_usec ) +
            ( UINT64 ) ( resUsage.ru_stime.tv_sec ) * 1000000LL + ( UINT64 ) ( resUsage.ru_stime.tv_usec );
    return usage;
#else
    return 0;
#endif
}

void
getUsageTime ( long long *userTime_p, long long *systemTime_p )
{
#ifdef __VMS
*userTime_p = 0;
*systemTime_p = 0;
#else
#ifdef WIN32
    FILETIME        creationTime;
    FILETIME        exitTime;
    FILETIME        kernelTime;
    FILETIME        userTime;
    LARGE_INTEGER   theTime;

    GetProcessTimes ( GetCurrentProcess (  ), &creationTime, &exitTime, &kernelTime, &userTime );
    /* File times are in units of 100 ns */
    theTime.LowPart = userTime.dwLowDateTime;
    theTime.HighPart = userTime.dwHighDateTime;
    *userTime_p = theTime.QuadPart / 10;        /* convert to microseconds */
    theTime.LowPart = kernelTime.dwLowDateTime;
    theTime.HighPart = kernelTime.dwHighDateTime;
    *systemTime_p = theTime.QuadPart / 10;      /* convert to microseconds */
#else
    struct rusage   usage;

    getrusage ( RUSAGE_SELF, &usage );
    *userTime_p = ( long long ) usage.ru_utime.tv_sec * ( long long ) 1000000 + ( long long ) usage.ru_utime.tv_usec;
    *systemTime_p = ( long long ) usage.ru_stime.tv_sec * ( long long ) 1000000 + ( long long ) usage.ru_stime.tv_usec;
#endif
#endif
}

UINT64
getCpuSpeedInHz ( void )
{

#ifdef WIN32
    LARGE_INTEGER   frequency;
    QueryPerformanceFrequency ( &frequency );
    return frequency.QuadPart;

#elif _LINUX_BUILD
    UINT64          procSpeed = 0;
    UINT64          tmpSpeed;
    std::string cpuinfoLine;
    std::ifstream cpuinfoFile ( CPU_INFO_FILE );
    INT32           numProcessors = 0;

    if ( cpuinfoFile == FALSE ) {
        return 0;
    }

    cpuinfoLine.reserve ( 80 );
    while ( getline ( cpuinfoFile, cpuinfoLine ) ) {
        if ( cpuinfoLine.find ( "processor" ) != string::npos ) {

            /* Detected one processor - loop until we get the model */
            while ( getline ( cpuinfoFile, cpuinfoLine ) ) {
                if ( cpuinfoLine.find ( "model name" ) != string::npos ) {
                    string          model ( cpuinfoLine, cpuinfoLine.find ( ": " ) + 2 );
                    break;
                }
            }

            /* Now get the processor speed */
            while ( getline ( cpuinfoFile, cpuinfoLine ) ) {

                if ( cpuinfoLine.find ( "cpu MHz" ) != string::npos ) {
                    std::string model ( cpuinfoLine, cpuinfoLine.find ( ": " ) + 2 );
                    tmpSpeed = ( solace::pubsub::UINT64 ) ( atof ( model.c_str (  ) ) * 1000000.0 );

                    if ( ( procSpeed != 0 ) && ( procSpeed != tmpSpeed ) ) {
                        LOG ( PSL_WARNING,
                              "Processor %i has a different speed: %s  Mhz.  Timers may be incorrect.",
                              numProcessors, model.c_str (  ) );
                    }
                    if ( procSpeed == 0 ) {
                        procSpeed = tmpSpeed;
                    }
                    break;
                }
            }
            ++numProcessors;
        }
    }

    return procSpeed;
#else
    /* 
     * NOTE: Outside of linux, we haven't investigated getting the cpu
     *       ticks.  By default at least it definitely not portable.
     *       A compromize is to return microseconds here and implement
     *       the numClockCycles via a call to getTimeInUs.  This does 
     *       allow for the message feeder to work accross platforms
     *       albeit not as accurately.
     */
    return 1000000ull;
#endif
}

BOOL
mutexInit ( MUTEX_T * mutex_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    *mutex_p = CreateMutex ( NULL, FALSE, NULL );
    if ( *mutex_p == NULL ) {
        /* Unable to create Mutex */
        rc = FALSE;
    }
#else
    int             osRc = pthread_mutex_init ( ( MUTEX_T * ) mutex_p, NULL );
    if ( osRc != 0 ) {
        /* pthread does NOT set errno, but returns the equivalent value */
        rc = FALSE;
    }
#endif
    return rc;
}

BOOL
mutexDestroy ( MUTEX_T * mutex_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    if ( ( *mutex_p != NULL ) && ( !CloseHandle ( *mutex_p ) ) ) {
        /* Could not destroy mutex */
        rc = FALSE;
    }
#else
    int             osRc = pthread_mutex_destroy ( ( MUTEX_T * ) mutex_p );
    if ( osRc != 0 ) {
        /* pthread does NOT set errno, but returns the equivalent value */
        rc = FALSE;
    }
#endif
    return rc;
}

BOOL
mutexLock ( MUTEX_T * mutex_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    DWORD           waitResult = WaitForSingleObject ( *mutex_p, INFINITE );
    if ( waitResult != WAIT_OBJECT_0 ) {
        /* Could not lock  mutex, result */
        rc = FALSE;
    }
#else
    int             osRc = pthread_mutex_lock ( ( MUTEX_T * ) mutex_p );
    if ( osRc != 0 ) {
        /* pthread does NOT set errno, but returns the equivalent value */
        rc = FALSE;
    }
#endif
    return rc;
}

BOOL
mutexUnlock ( MUTEX_T * mutex_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    if ( !ReleaseMutex ( *mutex_p ) ) {
        /* Could not unlock mutex */
        rc = FALSE;
    }
#else
    int             osRc = pthread_mutex_unlock ( ( MUTEX_T * ) mutex_p );
    if ( osRc != 0 ) {
        /* pthread does NOT set errno, but returns the equivalent value */
        rc = FALSE;
    }
#endif
    return rc;

}

BOOL
condInit ( CONDITION_T * cond_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    *cond_p = CreateEvent ( NULL, TRUE, TRUE, NULL );
    if ( *cond_p == NULL ) {
        /* Could not create event for condition */
        rc = FALSE;
    } else {
        if ( !ResetEvent ( *cond_p ) ) {
            rc = FALSE;
        }
    }
#else
    int             osRc = pthread_cond_init ( cond_p, NULL );
    if ( osRc != 0 ) {
        /*
         * pthread does NOT set errno, but returns the equivalent value
         * Could not create event for condition 
         */
        rc = FALSE;
    }
#endif

    return rc;

}

BOOL
condWait ( CONDITION_T * cond_p, MUTEX_T * mutex_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    ( void ) SignalObjectAndWait ( *mutex_p, *cond_p, INFINITE, FALSE );
    rc = mutexLock ( mutex_p );
#else
    pthread_cond_wait ( cond_p, mutex_p );
#endif

    return rc;
}

BOOL
condReset ( CONDITION_T * cond_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    ResetEvent ( *cond_p );
#else
#endif

    return rc;
}

BOOL
condDestroy ( CONDITION_T * cond_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    if ( ( *cond_p != NULL ) && !CloseHandle ( *cond_p ) ) {
        /* Could not close condition event */
        rc = FALSE;
    }
#else
    int             osRc = pthread_cond_destroy ( cond_p );
    if ( osRc != 0 ) {
        /* 
         * pthread does NOT set errno, but returns the equivalent value
         * Could not close condition event 
         */
        rc = FALSE;
    }
#endif

    return rc;

}

BOOL
condTimedWait ( CONDITION_T * cond_p, MUTEX_T * mutex_p, int timeoutSec /* negative means forever */
         )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    DWORD           waitInterval;
    DWORD           waitResult;
    if ( timeoutSec < 0 ) {
        waitInterval = INFINITE;
    } else {
        waitInterval = ( DWORD ) timeoutSec *1000;      /* convert to ms */
    }

    waitResult = SignalObjectAndWait ( *cond_p, *mutex_p, waitInterval, FALSE );
    ( void ) mutexLock ( mutex_p );
    if ( waitResult == WAIT_TIMEOUT ) {
        /* Wait on condition timed out, timeout */
        return FALSE;
    } else if ( waitResult != WAIT_OBJECT_0 ) {
        /* Could not wait on condition, result = */
        rc = FALSE;
    }
#else
    int             osRc;
    struct timespec absTimeout;

    if ( timeoutSec < 0 ) {
        osRc = pthread_cond_wait ( cond_p, mutex_p );
    } else {
        osRc = clock_gettime ( CLOCK_REALTIME, &absTimeout );
        if ( osRc < 0 ) {
            /* Could not get time for condition wait, error = */
            return FALSE;
        }
        absTimeout.tv_sec += timeoutSec;
        osRc = pthread_cond_timedwait ( cond_p, mutex_p, &absTimeout );
    }
    if ( osRc != 0 ) {
        /* pthread does NOT set errno, but returns the equivalent value */
        if ( osRc == ETIMEDOUT ) {
            /* Wait on condition timed out, timeout = %d sec */
            return FALSE;
        }
        /* Could not wait on condition, error = %d */
        rc = FALSE;
    }
#endif
    return rc;

}

BOOL
condSignal ( CONDITION_T * cond_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    if ( !SetEvent ( *cond_p ) ) {
        /* Could not signal condition */
        rc = FALSE;
    }
#else
    int             osRc = pthread_cond_signal ( cond_p );
    if ( osRc != 0 ) {
        /*
         * pthread does NOT set errno, but returns the equivalent value
         * Could not signal condition, error = %d
         */
        rc = FALSE;
    }
#endif
    return rc;

}

BOOL
semInit ( SEM_T * sem_p, unsigned int initValue, unsigned int maxValue )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    *sem_p = CreateSemaphore ( NULL, ( LONG ) initValue, ( LONG ) maxValue, NULL );
    if ( *sem_p == NULL ) {
        rc = FALSE;
    }
#else
    if ( sem_init ( sem_p, 0, initValue ) != 0 ) {
        rc = FALSE;
    }
#endif
    return rc;
}

BOOL
semDestroy ( SEM_T * sem_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    if ( ( *sem_p != NULL ) && ( !CloseHandle ( *sem_p ) ) ) {
        rc = FALSE;
    }
#else
    if ( sem_destroy ( sem_p ) != 0 ) {
        rc = FALSE;
    }
#endif
    return rc;
}

BOOL
semWait ( SEM_T * sem_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    if ( WaitForSingleObject ( *sem_p, INFINITE ) != WAIT_OBJECT_0 ) {
        rc = FALSE;
    }
#else
    int             osRc;
  tryAgain:
    osRc = sem_wait ( sem_p );
    if ( osRc != 0 ) {
        if ( errno == EINTR )
            goto tryAgain;
        rc = FALSE;
    }
#endif
    return rc;
}

BOOL
semPost ( SEM_T * sem_p )
{
    BOOL            rc = TRUE;

#ifdef WIN32
    LONG            previousCount;
    if ( !ReleaseSemaphore ( *sem_p, 1, &previousCount ) ) {
        rc = FALSE;
    }
#else
    if ( sem_post ( sem_p ) != 0 ) {
        rc = FALSE;
    }
#endif
    return rc;
}

THREAD_HANDLE_T
startThread ( FP fp, void *arg )
{
    THREAD_HANDLE_T th = _NULL_THREAD_ID;

#ifdef WIN32
    DWORD           thread_id;
    th = CreateThread ( NULL,   /* default security attributes */
                        0,      /* use default stack size */
                        ( LPTHREAD_START_ROUTINE ) fp,  /* thread function */
                        arg,    /* argument to thread function */
                        0,      /* use default creation flags */
                        &thread_id );   /* return thread identifier */
    if ( th == NULL ) {
        return _NULL_THREAD_ID;
    } else {
        return th;
    }
#else
    if ( pthread_create ( &th, NULL, fp, arg ) ) {
    }
    if ( th == _NULL_THREAD_ID ) {
        return _NULL_THREAD_ID;
    } else {
        return th;
    }
#endif
}

void
waitOnThread ( THREAD_HANDLE_T handle )
{
#ifdef WIN32
    WaitForSingleObject ( handle, INFINITE );
    CloseHandle ( handle );
    handle = NULL;
#else
    void           *value_p;
    pthread_join ( handle, &value_p );
#endif
}
