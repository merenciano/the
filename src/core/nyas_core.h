#ifndef NYAS_NYAS_CORE_H
#define NYAS_NYAS_CORE_H

#include <stdint.h>

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

union nyas2 {
	uint64_t bits;

	struct nyas_point2i {
		int32_t x, y;
	} pi;

	struct nyas_rect2i {
		int32_t w, h;
	} rect;

	int32_t vi[2];

	struct nyas_point2u {
		uint32_t x, y;
	} pu;

	struct nyas_rect2u {
		uint32_t w, h;
	} rectu;

	uint32_t vu[2];

	struct nyas_point2 {
		float x, y;
	} p;

	struct nyas_rect2f {
		float w, h;
	} rectf;

	struct nyas_color2f {
		float rgb, a;
	} color;

	float v[2];
};

union nyas3 {
	struct nyas_point3i {
		int32_t x, y, z;
	} pi;

	int32_t vi[3];

	struct nyas_point3u {
		uint32_t x, y, z;
	} pu;

	uint32_t vu[3];

	struct nyas_point3 {
		float x, y, z;
	} p;

	struct nyas_color3f {
		float r, g, b;
	} color;

	float v[3];
};

union nyas4 {
	struct nyas_point4i {
		int32_t x, y, z, w;
	} pi;

	struct nyas_rect {
		int32_t x, y, w, h;
	} rect;

	int32_t vi[4];

	struct nyas_point4u {
		uint32_t x, y, z, w;
	} pu;

	struct nyas_rectu {
		uint32_t x, y, w, h;
	} rectu;

	uint32_t vu[4];

	struct nyas_point4 {
		float x, y, z, w;
	} p;

	struct nyas_rectf {
		float x, y, w, h;
	} rectf;

	struct nyas_color4f {
		float r, g, b, a;
	} color;

	float v[4];
};

struct nyas_vec2i {
	int x, y;
};

#endif // NYAS_NYAS_CORE_H
