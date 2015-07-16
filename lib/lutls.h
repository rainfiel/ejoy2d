#ifndef EJOY_2D_LUTLS_H
#define EJOY_2D_LUTLS_H

#include <lauxlib.h>

static const char * srt_key[] = {
	"x",
	"y",
	"sx",
	"sy",
	"rot",
	"scale",
};

void fill_srt(lua_State *L, struct srt *srt, int idx);

#endif