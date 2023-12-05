#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mathc.h"
#define STS_VERTEX_CACHE_OPTIMIZER_IMPLEMENTATION
#include "sts_vertex_cache_optimizer.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

typedef struct obj_t {
    float *vert;
    size_t vsz;
    unsigned int *indices;
    size_t isz;
} obj_t;

typedef struct vec3_t {
    float x, y, z;
} vec3_t;

typedef struct vertex_t {
    vec3_t p, n, t;
} vertex_t;

static vertex_t *vtxarr = NULL;

static void
ReadFile(void *ctx, const char *path, int is_mtl, const char *obj_path, char **buf, size_t *size)
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
		assert(false && "Read failed.");
	}

	fclose(f);
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
	obj->indices = malloc(obj->isz * sizeof(unsigned int));
	float *vit = obj->vert;
	size_t ii = 0; // TODO: Indices right.

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
			struct vec3 delta_p1 = svec3_subtract(svec3(v2[0], v2[1], v2[2]),
			                                      svec3(v1[0], v1[1], v1[2]));
			struct vec3 delta_p2 = svec3_subtract(svec3(v3[0], v3[1], v3[2]),
			                                      svec3(v1[0], v1[1], v1[2]));
			struct vec2 delta_uv1 = svec2_subtract(svec2(v2[12], v2[13]),
			                                       svec2(v1[12], v1[13]));
			struct vec2 delta_uv2 = svec2_subtract(svec2(v3[12], v3[13]),
			                                       svec2(v1[12], v1[13]));
			float r = 1.0f /
			  (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
			struct vec3 tan = svec3_multiply_f(
			  svec3_subtract(svec3_multiply_f(delta_p1, delta_uv2.y),
			                 svec3_multiply_f(delta_p2, delta_uv1.y)),
			  r);
			struct vec3 bitan = svec3_multiply_f(
			  svec3_subtract(svec3_multiply_f(delta_p2, delta_uv1.x),
			                 svec3_multiply_f(delta_p1, delta_uv2.x)),
			  r);

			v1[6] = tan.x;
			v1[7] = tan.y;
			v1[8] = tan.z;
			v2[6] = tan.x;
			v2[7] = tan.y;
			v2[8] = tan.z;
			v3[6] = tan.x;
			v3[7] = tan.y;
			v3[8] = tan.z;

			v1[9] = bitan.x;
			v1[10] = bitan.y;
			v1[11] = bitan.z;
			v2[9] = bitan.x;
			v2[10] = bitan.y;
			v2[11] = bitan.z;
			v3[9] = bitan.x;
			v3[10] = bitan.y;
			v3[11] = bitan.z;

			for (int j = 0; j < 14; ++j) {
				*vit++ = v1[j];
			}

			for (int j = 0; j < 14; ++j) {
				*vit++ = v2[j];
			}

			for (int j = 0; j < 14; ++j) {
				*vit++ = v3[j];
			}

			for (int j = 0; j < 3; ++j, ++ii) {
				obj->indices[ii] = ii;
			}
		}
	}
}

unsigned int CheckVtx(float *v, float *end, float *newvtx, int *oexist)
{
    *oexist = 0;
    int i = 0;
    for (; v < end; ++i, v += 14)
    {
        int eq = (v[0] == newvtx[0]) && 
            (v[1] == newvtx[1]) && 
            (v[2] == newvtx[2]) && 
            (v[3] == newvtx[3]) && 
            (v[4] == newvtx[4]) && 
            (v[5] == newvtx[5]) && 
            (v[12] == newvtx[12]) && 
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
LoadWithIndices(const char *path, obj_t *obj)
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
	obj->indices = malloc(obj->isz * sizeof(unsigned int));
	float *vit = obj->vert;
	size_t ii = 0; // TODO: Indices right.

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

            int eq1;
            int eq2;
            int eq3;
            unsigned int i1 = CheckVtx(obj->vert, vit, v1, &eq1);
            unsigned int i2 = CheckVtx(obj->vert, vit, v2, &eq2);
            unsigned int i3 = CheckVtx(obj->vert, vit, v3, &eq3);

			// Calculate tangent and bitangent
			struct vec3 delta_p1 = svec3_subtract(svec3(v2[0], v2[1], v2[2]),
			                                      svec3(v1[0], v1[1], v1[2]));
			struct vec3 delta_p2 = svec3_subtract(svec3(v3[0], v3[1], v3[2]),
			                                      svec3(v1[0], v1[1], v1[2]));
			struct vec2 delta_uv1 = svec2_subtract(svec2(v2[12], v2[13]),
			                                       svec2(v1[12], v1[13]));
			struct vec2 delta_uv2 = svec2_subtract(svec2(v3[12], v3[13]),
			                                       svec2(v1[12], v1[13]));
			float r = 1.0f /
			  (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
			struct vec3 tan = svec3_multiply_f(
			  svec3_subtract(svec3_multiply_f(delta_p1, delta_uv2.y),
			                 svec3_multiply_f(delta_p2, delta_uv1.y)),
			  r);
			struct vec3 bitan = svec3_multiply_f(
			  svec3_subtract(svec3_multiply_f(delta_p2, delta_uv1.x),
			                 svec3_multiply_f(delta_p1, delta_uv2.x)),
			  r);

			v1[6] = tan.x;
			v1[7] = tan.y;
			v1[8] = tan.z;
			v2[6] = tan.x;
			v2[7] = tan.y;
			v2[8] = tan.z;
			v3[6] = tan.x;
			v3[7] = tan.y;
			v3[8] = tan.z;

			v1[9] = bitan.x;
			v1[10] = bitan.y;
			v1[11] = bitan.z;
			v2[9] = bitan.x;
			v2[10] = bitan.y;
			v2[11] = bitan.z;
			v3[9] = bitan.x;
			v3[10] = bitan.y;
			v3[11] = bitan.z;

            if (eq1) {
                obj->indices[ii++] = i1;
            } else {
                for (int j = 0; j < 14; ++j) {
                    *vit++ = v1[j];
                }
                obj->indices[ii++] = obj->isz++;
            }

            if (eq2) {
                obj->indices[ii++] = i2;
            } else {
                for (int j = 0; j < 14; ++j) {
                    *vit++ = v2[j];
                }
                obj->indices[ii++] = obj->isz++;
            }

            if (eq3) {
                obj->indices[ii++] = i3;
            } else {
                for (int j = 0; j < 14; ++j) {
                    *vit++ = v3[j];
                }
                obj->indices[ii++] = obj->isz++;
            }
		}
	}
    obj->vsz = vit - obj->vert;
}

int
main(int argc, char **argv)
{
    obj_t obj;
    obj_t objopt;
    Load(argv[1], &obj);

    float first = stsvco_compute_ACMR(obj.indices, obj.isz, 64);
    printf("First ACMR %f\n", first);
    LoadWithIndices(argv[1], &objopt);
    float optim = stsvco_compute_ACMR(objopt.indices, objopt.isz, 64);
    printf("Opt ACMR %f\n", optim);
    stsvco_optimize(objopt.indices, objopt.isz, objopt.vsz, 64);
    float optimalgo = stsvco_compute_ACMR(objopt.indices, objopt.isz, 64);
    printf("Opt ACMR %f\n", optimalgo);
    
    return 0;
}
