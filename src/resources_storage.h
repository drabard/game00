#pragma once

#include <assert.h>

#include "resources.h"
#include "string_id.h"
#include "file.h"

#define RSRC_MAX_MESHES 128
#define RSRC_MAX_TEXTURES 128
#define RSRC_MAX_FONTS 128

enum rsrc_resource_type
{
	RSRC_MESH = 0,
	RSRC_TEXTURE,
	RSRC_FONT,

	RSRC_COUNT
};

struct rsrc_token
{
	enum rsrc_resource_type type;
	str_id name;
	union
	{
		struct rsrc_mesh* mesh;
		struct rsrc_texture* texture;
		struct rsrc_font* font;
	};
};

enum rsrc_status rsrc_load(enum rsrc_resource_type type, str_id name, struct rsrc_token* out_token);

enum rsrc_status rsrc_unload(struct rsrc_token* token);

