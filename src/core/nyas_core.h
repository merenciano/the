#ifndef NYAS_NYAS_CORE_H
#define NYAS_NYAS_CORE_H

enum nyas_defs {
	NYAS_ERR = 0,
	NYAS_OK = 1,
	NYAS_END = -2,
	NYAS_NULL = -3,
	NYAS_ERR_ALLOC = -10,
	NYAS_ERR_FILE = -20,
	NYAS_ERR_THREAD = -30,
	NYAS_DEFAULT = -50,
	NYAS_IGNORE = -51,
	NYAS_INVALID = -52,
	NYAS_NONE = -53,
};

struct nyas_vec2i {
	int x, y;
};

struct nyas_vec2f {
	float x, y;
};

struct nyas_color {
	float r, g, b, a;
};

#endif // NYAS_NYAS_CORE_H
