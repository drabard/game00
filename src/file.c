#include "file.h"

#include <stdio.h>
#include <stdlib.h>

enum file_status file_load_text(const char* path, const char** buf,
                                uint32_t* size)
{
    FILE* f = 0;
    char* b = 0;
    uint32_t s = 0;

    f = fopen(path, "rb");
    if (!f)
        goto error;

    fseek(f, 0, SEEK_END);
    s = ftell(f);

    b = realloc((void*)*buf, s + 1);
    if (!b)
        goto error;

    fseek(f, 0, SEEK_SET);

    fread(b, s, 1, f);
    b[s] = '\0';

    fclose(f);

    *buf = b;
    *size = s;

    return FILE_OK;

error:
    if (f)
        fclose(f);
    free(b);
    *buf = 0;
    *size = 0;

    return FILE_FAILURE;
}

void file_unload_text(const char** buf)
{
    free((void*)*buf);
    *buf = 0;
}

enum file_status file_save_text(const char* path, const char* buf,
                                uint32_t size)
{
    FILE* f = 0;
    f = fopen(path, "wb");

    if (!f)
        goto error;
    if (fwrite(buf, size, 1, f) == 0)
        goto error;

    return FILE_OK;

error:
    fclose(f);

    return FILE_FAILURE;
}

enum file_status file_load_binary(const char* path, uint8_t** buf,
                                  uint32_t* size)
{
    FILE* f = 0;
    uint8_t* b = 0;

    f = fopen(path, "rb");
    if (!f)
        goto error;

    fseek(f, 0, SEEK_END);

    uint32_t s = ftell(f);
    b = (uint8_t*)realloc((void*)*buf, s);
    if (!b)
        goto error;

    fseek(f, 0, SEEK_SET);

    fread(b, s, 1, f);

    fclose(f);

    *buf = b;
    *size = s;

    return FILE_OK;

error:
    if (f)
        fclose(f);
    free(b);
    *buf = 0;
    *size = 0;

    return FILE_FAILURE;
}

enum file_status file_save_binary(const char* path, const uint8_t* buf,
                                  uint32_t size)
{
    FILE* f = 0;
    f = fopen(path, "wb");

    if (!f)
        goto error;
    if (fwrite(buf, size, 1, f) == 0)
        goto error;

    return FILE_OK;

error:
    if (f)
        fclose(f);

    return FILE_FAILURE;
}

void file_unload_binary(uint8_t** buf)
{
    free(*buf);
    *buf = 0;
}
