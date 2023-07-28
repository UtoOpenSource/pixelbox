/*
C89 compatible threads. Choice of public domain or MIT-0. See license statements at the end of this file.

David Reid - mackron@gmail.com
*/

/*
Introduction
============
This library aims to implement an equivalent to the C11 threading library. Not everything is implemented:

  * Condition variables are not supported on the Win32 build. If your compiler supports pthread, you
    can use that instead by putting `#define C89THREAD_USE_PTHREAD` before including c89thread.h.
  * Thread-specific storage (TSS/TLS) is not yet implemented.

The API should be compatible with the main C11 API, but all APIs have been namespaced with `c89`:

    +----------+----------------+
    | C11 Type | c89thread Type |
    +----------+----------------+
    | thrd_t   | c89thrd_t      |
    | mtx_t    | c89mtx_t       |
    | cnd_t    | c89cnd_t       |
    +----------+----------------+

In addition to types defined by the C11 standard, c89thread also implements the following primitives:

    +----------------+-------------+
    | c89thread Type | Description |
    +----------------+-------------+
    | c89sem_t       | Semaphore   |
    | c89evnt_t      | Event       |
    +----------------+-------------+

The C11 threading library uses the timespec function for specifying times, however this is not well
supported on older compilers. Therefore, c89thread implements some helper functions for working with
the timespec object. For known compilers that do not support the timespec struct, c89thread will
define it.

Sometimes c89thread will need to allocate memory internally. You can set a custom allocator at the
global level with `c89thread_set_allocation_callbacks()`. This is not thread safe, but can be called
from any thread so long as you do your own synchronization. Threads can be created with an extended
function called `c89thrd_create_ex()` which takes a pointer to a structure containing custom allocation
callbacks which will be used instead of the global callbacks if specified. This function is specific to
c89thread and is not usable if you require strict C11 compatibility.

This is still work-in-progress and not much testing has been done. Use at your own risk.


Building
========
c89thread is a single file library. To use it, do something like the following in one .c file.

    ```c
    #define C89THREAD_IMPLEMENTATION
    #include "c89thread.h"
    ```

You can then #include this file in other parts of the program as you would with any other header file.

When compiling for Win32 it should work out of the box without needing to link to anything. If you're
using pthreads, you may need to link with `-lpthread`.
*/

#ifndef c89thread_h
#define c89thread_h

#if defined(__cplusplus)
extern "C" {
#endif

typedef signed   int c89thread_int32;
typedef unsigned int c89thread_uint32;
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wlong-long"
    #if defined(__clang__)
        #pragma GCC diagnostic ignored "-Wc++11-long-long"
    #endif
#endif
typedef signed   long long c89thread_int64;
typedef unsigned long long c89thread_uint64;
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
    #pragma GCC diagnostic pop
#endif

#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
    typedef c89thread_int64  c89thread_intptr;
    typedef c89thread_uint64 c89thread_uintptr;
#else
    typedef c89thread_int32  c89thread_intptr;
    typedef c89thread_uint32 c89thread_uintptr;
#endif
typedef void* c89thread_handle;

#if defined(_WIN32) && !defined(C89THREAD_USE_PTHREAD)
    /* Win32. Do *not* include windows.h here. It will be included in the implementation section. */
    #define C89THREAD_WIN32
#else
    /* Using pthread */
    #define C89THREAD_POSIX
#endif

#if defined(C89THREAD_POSIX)
    #ifndef C89THREAD_USE_PTHREAD
    #define C89THREAD_USE_PTHREAD
    #endif

    /*
    This is, hopefully, a temporary measure to get compilation working with the -std=c89 switch on
    GCC and Clang. Unfortunately without this we get errors about the following functions not being
    declared:

        pthread_mutexattr_settype()

    I am not sure yet how a fallback would work for pthread_mutexattr_settype(). It may just be
    that it's fundamentally not compatible without explicit pthread support which would make the
    _XOPEN_SOURCE define mandatory. Needs further investigation.

    In addition, pthread_mutex_timedlock() is only available since 2001 which is only enabled if
    _XOPEN_SOURCE is defined to something >= 600. If this is not the case, a suboptimal fallback
    will be used instead which calls pthread_mutex_trylock() in a loop, with a sleep after each
    loop iteration. By setting _XOPEN_SOURCE here we reduce the likelyhood of users accidentally
    falling back to the suboptimal fallback.

    I'm setting this to the latest version here (700) just in case this file is included at the top
    of a source file which later on depends on some POSIX functions from later revisions.
    */
    #ifndef _XOPEN_SOURCE
    #define _XOPEN_SOURCE   700
    #else
        #if _XOPEN_SOURCE < 500
        #error _XOPEN_SOURCE must be >= 500. c89thread is not usable.
        #endif
    #endif

    #ifndef C89THREAD_NO_PTHREAD_IN_HEADER
        #include <pthread.h>
        typedef pthread_t           c89thread_pthread_t;
        typedef pthread_mutex_t     c89thread_pthread_mutex_t;
        typedef pthread_cond_t      c89thread_pthread_cond_t;
    #else
        typedef c89thread_uintptr   c89thread_pthread_t;
        typedef union               c89thread_pthread_mutex_t { char __data[40]; c89thread_uint64 __alignment; } c89thread_pthread_mutex_t;
        typedef union               c89thread_pthread_cond_t  { char __data[48]; c89thread_uint64 __alignment; } c89thread_pthread_cond_t;
    #endif
#endif

#include <time.h>   /* For timespec. */

#ifndef TIME_UTC
#define TIME_UTC    1
#endif

#if (defined(_MSC_VER) && _MSC_VER < 1900) || defined(__DMC__)  /* 1900 = Visual Studio 2015 */
struct timespec
{
    time_t tv_sec;
    long tv_nsec;
};
#endif

enum
{
    c89thrd_success  =  0,
    c89thrd_signal   = -1,  /* Not one of the standard results specified by C11, but -1 is used to indicate a signal in some APIs (thrd_sleep(), for example). */
    c89thrd_nomem    = -2,
    c89thrd_timedout = -3,
    c89thrd_busy     = -4,
    c89thrd_error    = -5
};


/* Memory Management. */
typedef struct
{
    void* pUserData;
    void* (* onMalloc)(size_t sz, void* pUserData);
    void* (* onRealloc)(void* p, size_t sz, void* pUserData);
    void  (* onFree)(void* p, void* pUserData);
} c89thread_allocation_callbacks;

void c89thread_set_allocation_callbacks(const c89thread_allocation_callbacks* pCallbacks);
void* c89thread_malloc(size_t sz, const c89thread_allocation_callbacks* pCallbacks);
void* c89thread_realloc(void* p, size_t sz, const c89thread_allocation_callbacks* pCallbacks);
void  c89thread_free(void* p, const c89thread_allocation_callbacks* pCallbacks);


/* thrd_t */
#if defined(C89THREAD_WIN32)
typedef c89thread_handle    c89thrd_t;  /* HANDLE, CreateThread() */
#else
typedef c89thread_pthread_t c89thrd_t;
#endif

typedef int (* c89thrd_start_t)(void*);

typedef struct
{
    void* pUserData;
    void (* onEntry)(void* pUserData);
    void (* onExit)(void* pUserData);
} c89thread_entry_exit_callbacks;

int c89thrd_create_ex(c89thrd_t* thr, c89thrd_start_t func, void* arg, const c89thread_entry_exit_callbacks* pEntryExitCallbacks, const c89thread_allocation_callbacks* pAllocationCallbacks);
int c89thrd_create(c89thrd_t* thr, c89thrd_start_t func, void* arg);
int c89thrd_equal(c89thrd_t lhs, c89thrd_t rhs);
c89thrd_t c89thrd_current(void);
int c89thrd_sleep(const struct timespec* duration, struct timespec* remaining);
void c89thrd_yield(void);
void c89thrd_exit(int res);
int c89thrd_detach(c89thrd_t thr);
int c89thrd_join(c89thrd_t thr, int* res);


/* mtx_t */
#if defined(C89THREAD_WIN32)
typedef struct
{
    c89thread_handle handle;    /* HANDLE, CreateMutex(), CreateEvent() */
    int type;
} c89mtx_t;
#else
typedef c89thread_pthread_mutex_t c89mtx_t;
#endif

enum
{
    c89mtx_plain     = 0x00000000,
    c89mtx_timed     = 0x00000001,
    c89mtx_recursive = 0x00000002
};

int c89mtx_init(c89mtx_t* mutex, int type);
void c89mtx_destroy(c89mtx_t* mutex);
int c89mtx_lock(c89mtx_t* mutex);
int c89mtx_timedlock(c89mtx_t* mutex, const struct timespec* time_point);
int c89mtx_trylock(c89mtx_t* mutex);
int c89mtx_unlock(c89mtx_t* mutex);


/* cnd_t */
#if defined(C89THREAD_WIN32)
/* Not implemented. */
typedef void*                    c89cnd_t;
#else
typedef c89thread_pthread_cond_t c89cnd_t;
#endif

int c89cnd_init(c89cnd_t* cnd);
void c89cnd_destroy(c89cnd_t* cnd);
int c89cnd_signal(c89cnd_t* cnd);
int c89cnd_broadcast(c89cnd_t* cnd);
int c89cnd_wait(c89cnd_t* cnd, c89mtx_t* mtx);
int c89cnd_timedwait(c89cnd_t* cnd, c89mtx_t* mtx, const struct timespec* time_point);


/* c89sem_t (not part of C11) */
#if defined(C89THREAD_WIN32)
typedef c89thread_handle c89sem_t;
#else
typedef struct
{
    int value;
    int valueMax;
    c89thread_pthread_mutex_t lock;
    c89thread_pthread_cond_t cond;
} c89sem_t;
#endif

int c89sem_init(c89sem_t* sem, int value, int valueMax);
void c89sem_destroy(c89sem_t* sem);
int c89sem_wait(c89sem_t* sem);
int c89sem_timedwait(c89sem_t* sem, const struct timespec* time_point);
int c89sem_post(c89sem_t* sem);


/* c89evnt_t (not part of C11) */
#if defined(C89THREAD_WIN32)
typedef c89thread_handle c89evnt_t;
#else
typedef struct
{
    int value;
    c89thread_pthread_mutex_t lock;
    c89thread_pthread_cond_t cond;
} c89evnt_t;
#endif

int c89evnt_init(c89evnt_t* evnt);
void c89evnt_destroy(c89evnt_t* evnt);
int c89evnt_wait(c89evnt_t* evnt);
int c89evnt_timedwait(c89evnt_t* evnt, const struct timespec* time_point);
int c89evnt_signal(c89evnt_t* evnt);


/* Timing Helpers */
int c89timespec_get(struct timespec* ts, int base);
struct timespec c89timespec_now();
struct timespec c89timespec_nanoseconds(time_t nanoseconds);
struct timespec c89timespec_milliseconds(time_t milliseconds);
struct timespec c89timespec_seconds(time_t seconds);
struct timespec c89timespec_diff(struct timespec lhs, struct timespec rhs);
struct timespec c89timespec_add(struct timespec tsA, struct timespec tsB);
int c89timespec_cmp(struct timespec tsA, struct timespec tsB);

/* Thread Helpers. */
int c89thrd_sleep_timespec(struct timespec ts);
int c89thrd_sleep_milliseconds(int milliseconds);


#if defined(__cplusplus)
}
#endif
#endif  /* c89thread_h */


