#ifndef CCTOOLS_SYS_ATTR_H_
#define CCTOOLS_SYS_ATTR_H_

#ifdef __APPLE__
#include_next <sys/attr.h>
#else // !__APPLE__
#include <stdlib.h> // for size_t

// https://opensource.apple.com/source/xnu/xnu-4903.221.2/bsd/sys/attr.h

// https://developer.apple.com/documentation/kernel/u_short
typedef unsigned short u_short;

// https://developer.apple.com/documentation/kernel/u_int16_t
typedef unsigned short u_int16_t;

// https://developer.apple.com/documentation/kernel/u_int32_t
typedef unsigned int u_int32_t;

// https://developer.apple.com/documentation/kernel/attrgroup_t
typedef u_int32_t attrgroup_t;

// https://developer.apple.com/documentation/kernel/attrlist
struct attrlist {
    u_short bitmapcount;
    u_int16_t reserved;
    attrgroup_t commonattr;
    attrgroup_t volattr;
    attrgroup_t dirattr;
    attrgroup_t fileattr;
    attrgroup_t forkattr;
};

#define ATTR_CMN_FNDRINFO 0x00004000
#define ATTR_FILE_DATALENGTH 0x00000200
#define ATTR_FILE_RSRCLENGTH 0x00001000

// Stub
#define getattrlist(...) -1

#endif // __APPLE__

#endif // CCTOOLS_SYS_ATTR_H_
