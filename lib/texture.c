#include "texture.h"
#include "shader.h"

#define MAX_TEXTURE 256

struct texture {
	int width;
	int height;
	float invw;
	float invh;
	RID id;
    RID alphaid;
	RID fb; /// rt 's frame buffer
};

struct texture_pool {
	int count;
	struct texture tex[MAX_TEXTURE];
};

static struct texture_pool POOL;
static struct render *R = NULL;

void 
texture_initrender(struct render *r) {
	R = r;
}

static inline uint32_t
average4(uint32_t c[4]) {
	int i;
	uint32_t hi = 0;
	uint32_t low = 0;
	for (i=0;i<4;i++) {
		uint32_t v = c[i];
		low += v & 0x00ff00ff;
		hi += (v & 0xff00ff00) >> 8;
	}
	hi = (hi/4) & 0x00ff00ff;
	low = (low/4) & 0x00ff00ff;

	return hi << 8 | low; 
}

static void 
texture_downsample(enum TEXTURE_FORMAT type, int *width, int *height, void *buffer) {
	int w = *width;
	int h = *height;
//    if (w%2 == 1 || h%2 == 1) {
//		return;
//    }

	// only support RGBA8888 now
	if (type != TEXTURE_RGBA8) {
		return;
	}

	uint32_t *src = (uint32_t*)buffer;
	char *dst = (char*)buffer;
    uint32_t average;
    int count = 0;
	int i,j;
	for (i=0;i+1<h;i+=2) {
		for (j=0;j+1<w;j+=2) {
            uint32_t c[4] = { src[j], src[j + 1], src[j + w], src[j + w + 1]};
            
            average = average4(c);
            dst[count] = average & 0xff;
            dst[count + 1] = (average >> 8) & 0xff;
            dst[count + 2] = (average >> 16) & 0xff;
            dst[count + 3] = (average >> 24) & 0xff;
            count += 4;
		}
		src += w*2;
	}
	*width = w/2;
	*height = h/2;
}

const char * 
texture_load(int id, enum TEXTURE_FORMAT pixel_format, int pixel_width, int pixel_height, void *data, int downsample) {
	if (id >= MAX_TEXTURE) {
		return "Too many texture";
	}
	struct texture * tex = &POOL.tex[id];
	if (id >= POOL.count) {
		POOL.count = id + 1;
	} 
	tex->fb = 0;
	tex->width = pixel_width;
	tex->height = pixel_height;
	tex->invw = 1.0f / (float)pixel_width;
	tex->invh = 1.0f / (float)pixel_height;
	if (tex->id == 0) {
		tex->id = render_texture_create(R, pixel_width, pixel_height, pixel_format, TEXTURE_2D, 0);
	}
	if (data == NULL) {
		// empty texture
		return NULL;
	}

    int compressed = 0;
    switch (pixel_format) {
        case TEXTURE_PVR2 :
        case TEXTURE_PVR4 :
        case TEXTURE_ETC1 :
            compressed = 1;
            break;
        default:
            compressed = 0;
    }
    
	if (downsample && !compressed) {
		texture_downsample(pixel_format, &pixel_width, &pixel_height, data);
	}
	render_texture_update(R, tex->id, pixel_width, pixel_height, data, 0, 0);
    
    // 还需继续创建alpha贴图
    if (pixel_format == TEXTURE_ETC1) {
        if (tex->alphaid == 0) {
            tex->alphaid = render_texture_create(R, pixel_width, pixel_height, TEXTURE_A8, TEXTURE_2D, 0);
        }
        int offset = pixel_width * pixel_height >> 1;
        render_texture_update(R, tex->alphaid, pixel_width, pixel_height, data + offset, 0, 0);
    } else {
        tex->alphaid = 0;
    }
 
	return NULL;
}

const char *
texture_load_alpha(int id, enum TEXTURE_FORMAT pixel_format, int pixel_width, int pixel_height, void *data, void *alphadata, int reduce) {
    if (id >= MAX_TEXTURE) {
        return "Too many texture";
    }
    struct texture * tex = &POOL.tex[id];
    if (id >= POOL.count) {
        POOL.count = id + 1;
    }
    tex->fb = 0;
    tex->width = pixel_width;
    tex->height = pixel_height;
    tex->invw = 1.0f / (float)pixel_width;
    tex->invh = 1.0f / (float)pixel_height;
    if (tex->id == 0) {
        tex->id = render_texture_create(R, pixel_width, pixel_height, pixel_format, TEXTURE_2D, 0);
    }
    if (data == NULL) {
        // empty texture
        return NULL;
    }
    
    if (reduce) {
        texture_downsample(pixel_format, &pixel_width, &pixel_height, data);
        texture_downsample(pixel_format, &pixel_width, &pixel_height, alphadata);
    }
    render_texture_update(R, tex->id, pixel_width, pixel_height, data, 0, 0);
    
    if (tex->alphaid == 0) {
        tex->alphaid = render_texture_create(R, pixel_width, pixel_height, TEXTURE_A8, TEXTURE_2D, 0);
    }
    render_texture_update(R, tex->alphaid, pixel_width, pixel_height, alphadata, 0, 0);
    
    return NULL;
}


const char*
texture_new_rt(int id, int w, int h){
	if (id >= MAX_TEXTURE) {
		return "Too many texture";
	}

	struct texture * tex = &POOL.tex[id];
	if (id >= POOL.count) {
		POOL.count = id + 1;
	}

	tex->width = w;
	tex->height = h;
	tex->invw = 1.0f / (float) w;
	tex->invh = 1.0f / (float) h;
	if (tex->id == 0) {
		tex->fb = render_target_create(R, w, h, TEXTURE_RGBA8);
		tex->id = render_target_texture(R, tex->fb);
        tex->alphaid = 0;
	}

	return NULL;
}

const char*
texture_active_rt(int id) {
	if (id < 0 || id >= POOL.count)
		return "Invalid rt id";
	struct texture *tex = &POOL.tex[id];

	render_set(R, TARGET, tex->fb, 0);
    render_state_commit(R);
    
	return NULL;
}

void
texture_reset_rt() {
    render_set(R, TARGET, 0, 0);
}

void
texture_deactive_rt() {
    render_set(R, TARGET, 0, 0);
    render_state_commit(R);
}

int
texture_coord(int id, float x, float y, uv_type *u, uv_type *v) {
	if (id < 0 || id >= POOL.count) {
		*u = (uint16_t)x;
		*v = (uint16_t)y;
		return 1;
	}
	struct texture *tex = &POOL.tex[id];
	if (tex->invw == 0) {
		// not load the texture
		*u = (uv_type)x;
		*v = (uv_type)y;
		return 1;
	}
    
    x *= tex->invw;
    y *= tex->invh;
    
#ifndef UV_FLOAT
	if (x > 1.0f)
		x = 1.0f;
	if (y > 1.0f)
		y = 1.0f;

	x *= 0xffff;
	y *= 0xffff;
#endif
    
    *u = (uv_type)x;
    *v = (uv_type)y;
    
	return 0;
}


void 
texture_unload(int id) {
	if (id < 0 || id >= POOL.count)
		return;
	struct texture *tex = &POOL.tex[id];
	if (tex->id == 0)
		return;
    
	render_release(R, TEXTURE, tex->id);
	if (tex->fb != 0)
		render_release(R, TARGET, tex->fb);
    if (tex->alphaid != 0)
        render_release(R, TEXTURE, tex->alphaid);
    
	tex->id = 0;
    tex->alphaid = 0;
	tex->fb = 0;
    tex->width = 0;
    tex->height = 0;
    tex->invh = 0;
    tex->invw = 0;
}

RID
texture_glid(int id) {
	if (id < 0 || id >= POOL.count)
		return 0;
	struct texture *tex = &POOL.tex[id];
	return tex->id;
}

RID
texture_glalphaid(int id) {
    if (id < 0 || id >= POOL.count)
        return 0;
    struct texture *tex = &POOL.tex[id];
    return tex->alphaid;
}

void 
texture_clearall() {
	int i;
	for (i=0;i<POOL.count;i++) {
		texture_unload(i);
	}
}

void 
texture_exit() {
	texture_clearall();
	POOL.count = 0;
}

void
texture_set_inv(int id, float invw, float invh) {
   if (id < 0 || id >= POOL.count)
       return ;
    
    struct texture *tex = &POOL.tex[id];
    tex->invw = invw;
    tex->invh = invh;
}

void
texture_swap(int ida, int idb) {
    if (ida < 0 || idb < 0 || ida >= POOL.count || idb >= POOL.count)
        return ;
    
    struct texture tex = POOL.tex[ida];
    POOL.tex[ida] = POOL.tex[idb];
    POOL.tex[idb] = tex;
}

void
texture_size(int id, int *width, int *height) {
    if (id < 0 || id >= POOL.count) {
        *width = *height = 0;
        return ;
    }
    
    struct texture *tex = &POOL.tex[id];
    *width = tex->width;
    *height = tex->height;
}

void
texture_delete_framebuffer(int id) {
    if (id < 0 || id >= POOL.count) {
        return;
    }
    
    struct texture *tex = &POOL.tex[id];
    if (tex->fb != 0) {
		render_release(R, TARGET, tex->fb);
        tex->fb = 0;
    }
}

const char * 
texture_update(int id, int pixel_width, int pixel_height, void *data) {
	if (id >= MAX_TEXTURE) {
		return "Too many texture";
	}

	if(data == NULL){
		return "no content";
	}
	struct texture * tex = &POOL.tex[id];
	if(tex->id == 0){
		return "not a valid texture";
	}
	render_texture_update(R, tex->id, pixel_width, pixel_height, data, 0, 0);

	return NULL;
}

enum TEXTURE_FORMAT texture_format(int id) {
    if (id >= MAX_TEXTURE) {
        return TEXTURE_INVALID;
    }
    
    struct texture * tex = &POOL.tex[id];
    if(tex->id == 0){
        return TEXTURE_INVALID;
    }
    
    return render_texture_format(R, tex->id);
}


const char*
texture_sub_update(int id, int x, int y, int width, int height, void *data) {
	if (id >= MAX_TEXTURE) {
		return "Too many texture";
	}

	if(data == NULL){
		return "no content";
	}
	struct texture * tex = &POOL.tex[id];
	if(tex->id == 0){
		return "not a valid texture";
	}
	render_texture_subupdate(R, tex->id, data, x, y, width, height);

	return NULL;
}
