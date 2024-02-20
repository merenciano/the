#ifndef NYAS_NYAS_CORE_H
#define NYAS_NYAS_CORE_H

#include <stdint.h>
#include <assert.h>

#define NYAS_ASSERT(X) (assert(X))

enum nyas_defs {
	NYAS_ERR = 0,
	NYAS_OK = 1,
	NYAS_END = -2,
	NYAS_NULL = -3,
	NYAS_ERR_ALLOC = -10,
	NYAS_ERR_FILE = -20,
	NYAS_ERR_THREAD = -30,
	NYAS_DEFAULT = -50,
	NYAS_NOOP = -51,
	NYAS_INVALID = -52,
	NYAS_NONE = -53,
};

struct nyas_point {
	int x, y;
};

struct nyas_rect {
	int x, y, w, h;
};

struct nyas_vec2 {
	float x, y;
};

struct nyas_vec3 {
	float x, y, z;
};

union nyas4f {
	struct nyas_vec4 {
		float x, y, z, w;
	} p;

	struct nyas_color {
		float r, g, b, a;
	} c;

	float v[4];
};

typedef float nyas_mat4[16];

union nyas_mat4 {
	struct nyas_vec4 r[4];
	struct nyas_4x4 {
		struct nyas_vec4 x;
		struct nyas_vec4 y;
		struct nyas_vec4 z;
		struct nyas_vec4 w;
	} row;
	float v[16];
	float p[4][4];
	struct nyas_mat4x4 {
		float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33;
	} m;
};

#endif // NYAS_NYAS_CORE_H
