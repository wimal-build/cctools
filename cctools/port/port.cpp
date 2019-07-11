extern "C" const char apple_version[] = "cctools-921";

#include <errno.h>
#include <libc.h>
#include <mach-o/dyld.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

__BEGIN_DECLS
#include <stuff/allocate.h>
#include <stuff/execute.h>
__END_DECLS
#include <sys/file.h>
#include <sys/param.h>
#include <vector>

void add_execute_list_with_prefix(char *str) {
    add_execute_list(cmd_with_prefix(str));
}

char *cmd_with_prefix(char *str) {
    std::vector<char> path(MAXPATHLEN);
    uint32_t size = path.size();
    if (_NSGetExecutablePath(path.data(), &size) == -1) {
        path.resize(size);
        _NSGetExecutablePath(path.data(), &size);
    }
    std::vector<char> absolute(PATH_MAX);
    char *prefix = realpath(path.data(), absolute.data());
    auto cursor = std::max(rindex(prefix, '/'), rindex(prefix, '-'));
    if (cursor) {
        cursor[1] = '\0';
    }
    return (makestr(prefix, str, NULL));
}

extern "C" int cctools_strncmp(const char *s1, const char *s2, size_t n) {
    if (strncmp("ranlib", s2, n) == 0) {
        auto l1 = strlen(s1);
        if (l1 > n) {
            return strncmp(s1 + l1 - n, s2, n);
        }
    }
    return strncmp(s1, s2, n);
}

#ifndef __APPLE__
#include <mach/mach.h>

// https://opensource.apple.com/source/Libc/Libc-825.26/gen/scalable_malloc.c
vm_size_t vm_page_size = 4096;

kern_return_t host_info(
    host_t,
    host_flavor_t flavor,
    host_info_t host_info_out,
    mach_msg_type_number_t *
) {
    if (flavor == HOST_BASIC_INFO) {
        auto basic_info = (host_basic_info_t) host_info_out;
        memset(basic_info, 0x00, sizeof(*basic_info));
        basic_info->cpu_type = CPU_TYPE_X86_64;
        basic_info->cpu_subtype = CPU_SUBTYPE_X86_64_ALL;
    }
    return 0;
}

host_name_port_t mach_host_self(void) {
    return nullptr;
}

mach_port_t mach_task_self(void) {
    return MACH_PORT_NULL;
}

kern_return_t vm_allocate(vm_map_t, vm_address_t *address, vm_size_t size, int) {
    *address = (vm_address_t) calloc(size, 1);
    return *address ? KERN_SUCCESS : KERN_INVALID_ADDRESS;
}

kern_return_t vm_deallocate(vm_map_t, vm_address_t, vm_size_t) {
    return KERN_SUCCESS;
}

kern_return_t mach_port_deallocate(ipc_space_t, mach_port_name_t) {
    return KERN_SUCCESS;
}

// _NSGetExecutablePath
#include <mach-o/dyld.h>
#include <unistd.h>

// https://github.com/libuv/libuv/blob/master/src/unix/procfs-exepath.c
int _NSGetExecutablePath(char *buffer, unsigned int *size) {
    if (buffer == nullptr || size == nullptr || *size <= 1) {
        return -1;
    }
    *size = readlink("/proc/self/exe", buffer, *size - 1);
    if (*size == -1) {
        return -1;
    }
    buffer[*size] = '\0';
    return 0;
}

// https://www.freebsd.org/cgi/man.cgi?query=strmode
// https://opensource.apple.com/source/Libc/Libc-167/string.subproj/strmode.c
extern "C" void strmode(mode_t mode, char *p) {
    switch (mode & S_IFMT) {
        case S_IFDIR:
            *p++ = 'd';
            break;
        case S_IFCHR:
            *p++ = 'c';
            break;
        case S_IFBLK:
            *p++ = 'b';
            break;
        case S_IFREG:
            *p++ = '-';
            break;
        case S_IFLNK:
            *p++ = 'l';
            break;
        case S_IFSOCK:
            *p++ = 's';
            break;
#ifdef S_IFIFO
        case S_IFIFO:
            *p++ = 'p';
            break;
#endif
#ifdef S_IFWHT
        case S_IFWHT:
            *p++ = 'w';
            break;
#endif
        default:
            *p++ = '?';
            break;
    }
    if (mode & S_IRUSR) {
        *p++ = 'r';
    } else {
        *p++ = '-';
    }
    if (mode & S_IWUSR) {
        *p++ = 'w';
    } else {
        *p++ = '-';
    }
    switch (mode & (S_IXUSR | S_ISUID)) {
        case 0:
            *p++ = '-';
            break;
        case S_IXUSR:
            *p++ = 'x';
            break;
        case S_ISUID:
            *p++ = 'S';
            break;
        case S_IXUSR | S_ISUID:
            *p++ = 's';
            break;
    }
    if (mode & S_IRGRP) {
        *p++ = 'r';
    } else {
        *p++ = '-';
    }
    if (mode & S_IWGRP) {
        *p++ = 'w';
    } else {
        *p++ = '-';
    }
    switch (mode & (S_IXGRP | S_ISGID)) {
        case 0:
            *p++ = '-';
            break;
        case S_IXGRP:
            *p++ = 'x';
            break;
        case S_ISGID:
            *p++ = 'S';
            break;
        case S_IXGRP | S_ISGID:
            *p++ = 's';
            break;
    }
    if (mode & S_IROTH) {
        *p++ = 'r';
    } else {
        *p++ = '-';
    }
    if (mode & S_IWOTH) {
        *p++ = 'w';
    } else {
        *p++ = '-';
    }
    switch (mode & (S_IXOTH | S_ISVTX)) {
        case 0:
            *p++ = '-';
            break;
        case S_IXOTH:
            *p++ = 'x';
            break;
        case S_ISVTX:
            *p++ = 'T';
            break;
        case S_IXOTH | S_ISVTX:
            *p++ = 't';
            break;
    }
    *p++ = ' ';
    *p = '\0';
}

#endif // __APPLE__
