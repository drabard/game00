#pragma once

#include <stdint.h>
#include <stddef.h>

// todo:
// - malloc callbacks
// - logging

enum file_status { FILE_OK = 0,
                   FILE_FAILURE };

enum file_status file_load_text(const char* path, const char** buf,
                                uint32_t* size);
enum file_status file_save_text(const char* path, const char* buf,
                                uint32_t size);
void file_unload_text(const char** buf);

enum file_status file_load_binary(const char* path, uint8_t** buf,
                                  uint32_t* size);
enum file_status file_save_binary(const char* path, const uint8_t* buf,
                                  uint32_t size);
void file_unload_binary(uint8_t** buf);
