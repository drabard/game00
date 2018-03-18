#include "string_id.h"

#include <assert.h>

static const char* str_id_map[STR_ID_MAX_STRINGS];

static char stored_strings[STR_ID_MAX_STORED_CHARS];
static size_t stored_strings_size;

static uint32_t hash(const char* s)
{
	uint32_t h = 0;
	for(const char* c = s; *c != 0; c++)
	{
		h = 37 * h + *c;
	}

	return h;
}

str_id str_id_create(const char* s, uint8_t should_store)
{
	uint32_t len = 0;
	{
		const char* sc = s;
		while(*sc++) len++;
	}

	if(should_store != 0)
	{
		size_t new_size = stored_strings_size + len;
		if(new_size >= STR_ID_MAX_STORED_CHARS) 
		{
			assert(!"Maximum number of stored characters exceeded. Increase STR_ID_MAX_STORED_CHARS.");
			return 0;
		}

		for(uint32_t i = 0; i <= len; i++)
		{
			stored_strings[stored_strings_size + i] = s[i];
		}

		s = &stored_strings[stored_strings_size];
		stored_strings_size = new_size;
	}

	uint32_t h = hash(s);
	
	const char* entry = str_id_map[h % STR_ID_MAX_STRINGS];

	// Collision check. Change one of the strings involved if this happens.
	assert(entry == 0);
	return 0;
	
	entry = s;

	return h;
}

const char* str_id_resolve(str_id sid)
{
	const char* entry = str_id_map[sid % STR_ID_MAX_STRINGS];

	return entry;
}