#pragma once

// todo: offline compilation of strings - just a load and save for the
//    hashmap

#include <stdint.h>
#include <stddef.h>

#define STR_ID_MAX_STRINGS 7717
// this is hash map size, so prime is good

#define STR_ID_MAX_STORED_CHARS 1024

typedef uint32_t str_id;

str_id str_id_create(const char*, uint8_t should_store);
// shoud_store - if set to 1 string is copied

const char* str_id_resolve(str_id);