#include "../include/core.h"

char* finite_readfile(const char* filename, size_t* fileSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("failed to open file: %s\n", filename);
        return NULL;
    }

    // Seek to end to get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    // Allocate buffer
    char* buffer = malloc(size);
    if (!buffer) {
        printf("failed to allocate memory for file: %s\n", filename);
        fclose(file);
        return NULL;
    }

    // Read file contents
    size_t read = fread(buffer, 1, size, file);
    if (read != size) {
        printf("failed to read complete file: %s\n", filename);
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);

    if (fileSize) {
        *fileSize = size;
    }

    return buffer;
}

FileBuffer finite_read_raw_file(const char* filename) {
    FileBuffer buffer = {NULL, 0};

    FILE* file = fopen(filename, "rb"); // "rb" = read binary
    if (!file) {
        perror("Failed to open file");
        return buffer;
    }

    fseek(file, 0, SEEK_END);
    buffer.size = ftell(file);
    rewind(file);

    buffer.data = (uint32_t*)malloc(buffer.size);
    if (!buffer.data) {
        perror("Failed to allocate memory");
        fclose(file);
        return buffer;
    }

    fread(buffer.data, 1, buffer.size, file);
    fclose(file);

    return buffer;
}