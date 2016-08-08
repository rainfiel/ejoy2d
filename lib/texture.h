#ifndef EJOY_2D_TEXTURE_H
#define EJOY_2D_TEXTURE_H

#include "ejoy2d.h"
#include "render.h"
#include <stdint.h>

void texture_initrender(struct render *R);
EJOY_API const char * texture_load(int id, enum TEXTURE_FORMAT type, int width, int height, void *buffer, int reduce);
EJOY_API void texture_unload(int id);
RID texture_glid(int id);
int texture_coord(int id, float x, float y, uint16_t *u, uint16_t *v);
void texture_clearall();
void texture_exit();

const char* texture_new_rt(int id, int width, int height);
const char* texture_active_rt(int id);
void texture_reset_rt();
void read_rt_pixels(int width, int height, void* buf);

void texture_set_inv(int id, float invw, float invh);
void texture_swap(int ida, int idb);
void texture_size(int id, int *width, int *height);
void texture_delete_framebuffer(int id);

/// update content of texture
/// width and height may not equal the original by design
/// useful for some condition 
/// async texture load,for example,
/// becasue we can first push a much more small avatar
EJOY_API const char* texture_update(int id, int width, int height, void *buffer);
EJOY_API const char* texture_sub_update(int id, int x, int y, int width, int height, void *buffer);


#endif
