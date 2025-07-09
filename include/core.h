#ifndef __CORE_H__
#define __CORE_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint32_t* data;
    size_t size;
} FileBuffer;

char* finite_readfile(const char* filename, size_t* fileSize);
FileBuffer finite_read_raw_file(const char* filename);
#endif