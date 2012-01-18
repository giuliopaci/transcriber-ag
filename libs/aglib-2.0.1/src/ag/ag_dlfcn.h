/*========================================================================*
 * TITLE:       Dynamic Loading
 * PORT:        Ross Linder
 *
 * Interface for stupid non ELF OS's
 *========================================================================*/

#ifdef __hpux
#include <dl.h>

#define RTLD_LAZY (BIND_DEFERRED | BIND_NONFATAL)
#define RTLD_NOW BIND_IMMEDIATE

#ifdef __cplusplus
extern "C"
{
#endif
    void *dlopen(const char *, int);
    void *dlsym(void *, const char *);
    int dlclose(void *);
    char *dlerror(void);
#ifdef __cplusplus
}
#endif
#endif

/* (( BT Patch -- */
/*
#ifdef _WIN32
#define RTLD_LAZY 1
#define RTLD_NOW  2
#define RTLD_GLOBAL (1 << 1)

#ifdef __cplusplus
extern "C"
{
#endif
    void *dlopen(const char *, int);
    void *dlsym(void *, const char *);
    int dlclose(void *);
    char *dlerror(void);
#ifdef __cplusplus
}
#endif
#endif
*/
/* -- BT Patch )) */

/* (( BT Patch -- */
//#if !defined(__hpux) && !defined(_WIN32)
/* -- BT Patch )) */
#include <dlfcn.h>
/* (( BT Patch -- */
//#endif
/* -- BT Patch )) */
