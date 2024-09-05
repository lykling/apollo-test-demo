#include "fake_io.h"
#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

typedef int (*original_open_func)(const char* pathname, int flags, ...);

int open(const char* pathname, int flags, ...) {
    static original_open_func original_open = NULL;
    if (!original_open) {
        original_open = (original_open_func)dlsym(RTLD_NEXT, "open");
        if (!original_open) {
            fprintf(stderr, "dlsym: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
    }

    char* fake_path = getenv("FAKE_IO_PATH");

    char* new_pathname = (char*)malloc(strlen(fake_path) + strlen(pathname) + 1);
    strcpy(new_pathname, fake_path);
    strcat(new_pathname, pathname);

    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        int mode = va_arg(args, int);
        va_end(args);
        return original_open(new_pathname, flags, mode);
    } else {
        return original_open(new_pathname, flags);
    }
    return 0;
}
