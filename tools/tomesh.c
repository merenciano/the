#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STSVCO_IDX_T unsigned short
#define STS_VERTEX_CACHE_OPTIMIZER_STATIC
#define STS_VERTEX_CACHE_OPTIMIZER_IMPLEMENTATION
#include "sts_vertex_cache_optimizer.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

typedef STSVCO_IDX_T index_t;

typedef struct obj_t {
	float *vert;
	size_t vsz;
	index_t *indices;
	size_t isz;
} obj_t;

static void
v3sub(float *vout, float *v0, float *v1)
{
	vout[0] = v0[0] - v1[0];
	vout[1] = v0[1] - v1[1];
	vout[2] = v0[2] - v1[2];
}

static void
v2sub(float *vout, float *v0, float *v1)
{
	vout[0] = v0[0] - v1[0];
	vout[1] = v0[1] - v1[1];
}

static void
v3muls(float *vout, float *v0, float s)
{
	vout[0] = v0[0] * s;
	vout[1] = v0[1] * s;
	vout[2] = v0[2] * s;
}

static void
ReadFile(void *c,
         const char *path,
         int mt,
         const char *op,
         char **buf,
         size_t *size)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		return;
	}

	fseek(f, 0L, SEEK_END);
	*size = ftell(f);
	rewind(f);

	*buf = malloc(*size + 1);
	assert(*buf && "Allocation failed.");

	if (fread(*buf, *size, 1, f) != 1) {
		assert(!"Read failed.");
	}

	fclose(f);
}

index_t
CheckVtx(float *v, float *end, float *newvtx, int *oexist)
{
	*oexist = 0;
	index_t i = 0;
	for (; v < end; ++i, v += 14) {
		int eq = (v[0] == newvtx[0]) && (v[1] == newvtx[1]) &&
		  (v[2] == newvtx[2]) && (v[3] == newvtx[3]) && (v[4] == newvtx[4]) &&
		  (v[5] == newvtx[5]) && (v[12] == newvtx[12]) &&
		  (v[13] == newvtx[13]);

		if (eq) {
			*oexist = 1;
			goto iseq;
		}
	}
iseq:
	return i;
}

void
Load(const char *path, obj_t *obj)
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t *shapes = NULL;
	size_t shape_count;
	tinyobj_material_t *mats = NULL;
	size_t mats_count;

	int result = tinyobj_parse_obj(&attrib, &shapes, &shape_count, &mats,
	                               &mats_count, path, ReadFile, NULL,
	                               TINYOBJ_FLAG_TRIANGULATE);

	if (result != TINYOBJ_SUCCESS) {
		printf("Error loading obj. Err: %d", result);
	}

	size_t tri_count = attrib.num_face_num_verts;
	obj->vsz = tri_count * 3 * 14;
	obj->isz = tri_count * 3;
	obj->vert = malloc(obj->vsz * sizeof(float));
	obj->indices = malloc(obj->isz * sizeof(index_t));
	float *vit = obj->vert;
	size_t ii = 0;

	obj->isz = 0;
	size_t index_offset = 0;
	for (size_t i = 0; i < attrib.num_face_num_verts; ++i) {
		for (size_t f = 0; f < attrib.face_num_verts[i] / 3; ++f) {
			tinyobj_vertex_index_t idx = attrib.faces[3 * f + index_offset++];
			float v1[14], v2[14], v3[14];

			v1[0] = attrib.vertices[3 * idx.v_idx + 0];
			v1[1] = attrib.vertices[3 * idx.v_idx + 1];
			v1[2] = attrib.vertices[3 * idx.v_idx + 2];
			v1[3] = attrib.normals[3 * idx.vn_idx + 0];
			v1[4] = attrib.normals[3 * idx.vn_idx + 1];
			v1[5] = attrib.normals[3 * idx.vn_idx + 2];
			v1[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v1[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset++];
			v2[0] = attrib.vertices[3 * idx.v_idx + 0];
			v2[1] = attrib.vertices[3 * idx.v_idx + 1];
			v2[2] = attrib.vertices[3 * idx.v_idx + 2];
			v2[3] = attrib.normals[3 * idx.vn_idx + 0];
			v2[4] = attrib.normals[3 * idx.vn_idx + 1];
			v2[5] = attrib.normals[3 * idx.vn_idx + 2];
			v2[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v2[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset++];
			v3[0] = attrib.vertices[3 * idx.v_idx + 0];
			v3[1] = attrib.vertices[3 * idx.v_idx + 1];
			v3[2] = attrib.vertices[3 * idx.v_idx + 2];
			v3[3] = attrib.normals[3 * idx.vn_idx + 0];
			v3[4] = attrib.normals[3 * idx.vn_idx + 1];
			v3[5] = attrib.normals[3 * idx.vn_idx + 2];
			v3[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v3[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			// Calculate tangent and bitangent
			float dp1[3], dp2[3], duv1[2], duv2[2];
			v3sub(dp1, v2, v1);
			v3sub(dp2, v3, v1);
			v2sub(duv1, &v2[12], &v1[12]);
			v2sub(duv2, &v3[12], &v1[12]);
			float r = 1.0f / (duv1[0] * duv2[1] - duv1[1] * duv2[0]);

			float tn[3], bitn[3], tmp[3];
			v3muls(tn, dp1, duv2[1]);
			v3muls(tmp, dp2, duv1[1]);
			v3sub(tn, tn, tmp);
			v3muls(tn, tn, r);

			v3muls(bitn, dp2, duv1[0]);
			v3muls(tmp, dp1, duv2[0]);
			v3sub(bitn, bitn, tmp);
			v3muls(bitn, bitn, r);

			v1[6] = tn[0];
			v1[7] = tn[1];
			v1[8] = tn[2];
			v2[6] = tn[0];
			v2[7] = tn[1];
			v2[8] = tn[2];
			v3[6] = tn[0];
			v3[7] = tn[1];
			v3[8] = tn[2];

			v1[9] = bitn[0];
			v1[10] = bitn[1];
			v1[11] = bitn[2];
			v2[9] = bitn[0];
			v2[10] = bitn[1];
			v2[11] = bitn[2];
			v3[9] = bitn[0];
			v3[10] = bitn[1];
			v3[11] = bitn[2];

			/*index_t nxt_idx = CheckVtx(obj->vert, vit, v1, NULL);
			obj->indices[obj->isz++] = nxt_idx;
			if (nxt_idx == (vit - obj->vert)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v1[j];
				}
			}

			nxt_idx = CheckVtx(obj->vert, vit, v2, NULL);
			obj->indices[obj->isz++] = nxt_idx;
			if (nxt_idx == (vit - obj->vert)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v2[j];
				}
			}

			nxt_idx = CheckVtx(obj->vert, vit, v3, NULL);
			obj->indices[obj->isz++] = nxt_idx;
			if (nxt_idx == (vit - obj->vert)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v3[j];
				}
			}*/

			// Check vertex rep
			int eq1;
			int eq2;
			int eq3;
			index_t i1 = CheckVtx(obj->vert, vit, v1, &eq1);
			index_t i2 = CheckVtx(obj->vert, vit, v2, &eq2);
			index_t i3 = CheckVtx(obj->vert, vit, v3, &eq3);

			if (eq1) {
				obj->indices[obj->isz++] = i1;
			} else {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v1[j];
				}
				printf("NotMatch: i1 = %d, ii = %lu\n", i1, ii);
				obj->indices[obj->isz++] = ii++;
			}

			if (eq2) {
				obj->indices[obj->isz++] = i2;
			} else {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v2[j];
				}
				printf("NotMatch: i2 = %d, ii = %lu\n", i2, ii);
				obj->indices[obj->isz++] = ii++;
			}

			if (eq3) {
				obj->indices[obj->isz++] = i3;
			} else {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v3[j];
				}
				printf("NotMatch: i3 = %d, ii = %lu\n", i3, ii);
				obj->indices[obj->isz++] = ii++;
			}
		}
	}
	obj->vsz = vit - obj->vert;
}

static void Export(const char *path, obj_t *obj)
{
	FILE *f = fopen(path, "wb");
	if (!f) {
		return;
	}

	// sizes in bytes
	size_t vsz = obj->vsz * sizeof(float);
	size_t isz = obj->isz * sizeof(index_t);
	fwrite(&vsz, sizeof(size_t), 1, f);
	fwrite(obj->vert, vsz, 1, f);

	fwrite(&isz, sizeof(size_t), 1, f);
	fwrite(obj->indices, isz, 1, f);

	fclose(f);
}

int
main(int argc, char **argv)
{
	obj_t obj;
	Load(argv[1], &obj);
	float acmr = stsvco_compute_ACMR(obj.indices, obj.isz, 64);
	printf("ACMR before: %f\n", acmr);
	stsvco_optimize(obj.indices, obj.isz, obj.vsz, 64);
	acmr = stsvco_compute_ACMR(obj.indices, obj.isz, 64);
	printf("ACMR after: %f\n", acmr);

	printf("%zu vtcs, %zu indices.\n", obj.vsz / 14, obj.isz);

	Export(argv[2], &obj);
	return 0;
}
