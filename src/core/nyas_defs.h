#ifndef NYAS_CORE_DEFS_H
#define NYAS_CORE_DEFS_H

enum nyas_defs {
	NYAS_EC_FAIL = 0,
	NYAS_OK = 1,
	NYAS_ERR_ALLOC = -100,
	NYAS_ERR_FILE = -200,
	NYAS_UNINIT = -5000,
	NYAS_DELETED = -5001,
	NYAS_MARKED_FOR_DELETE = -5002,
	NYAS_INVALID = -5003,
	NYAS_INACTIVE = -5004,
	NYAS_DEFAULT = -5005,
	NYAS_IGNORE = -5006
};

typedef struct nyas_v2i {
	int x, y;
} nyas_v2i;

static inline float *
nyas_set_color(float *out, float r, float g, float b, float a)
{
	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = a;
	return out;
}

#endif // NYAS_CORE_DEFS_H
