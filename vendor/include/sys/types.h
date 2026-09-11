#ifndef KF_SYS_TYPES_SHIM
#define KF_SYS_TYPES_SHIM
/* Case shim: several Psy-Q headers #include <sys/types.h> (lower case), which
   does not resolve on a case-sensitive host. Forward to the real SYS/TYPES.H,
   guarded so it is pulled in exactly once. */
#include <SYS/TYPES.H>
#endif
