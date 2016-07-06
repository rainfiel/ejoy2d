#ifndef ejoy_2d_screen_h
#define ejoy_2d_screen_h
#include <stdbool.h>
#include "ejoy2d.h"

struct render;
struct vertex_pack;

void screen_initrender(struct render *R);
EJOY_API void screen_init(float w, float h, float scale);
void screen_trans(float *x, float *y);
void screen_scissor(int x, int y, int w, int h);
bool screen_is_visible(float x,float y);
bool screen_is_poly_invisible(const struct vertex_pack* vp, int len);
void screen_get_info(int* w, int* h, float* scale);
float screen_invw();
float screen_invh();
#endif
