#ifndef NYAS_CORE_HMAP_H
#define NYAS_CORE_HMAP_H

#include <stdint.h>

#define NYAS_HMAP_INVALID_VALUE (void*)0xFFFFFFFFFFFFFFFF

typedef struct nyas_hmap nyas_hmap;

nyas_hmap *nyas_hmap_create(unsigned int capacity, unsigned int value_size);
void nyas_hmap_insert(nyas_hmap *hm, const char *key, void *value);
void *nyas_hmap_get(nyas_hmap *hm, const char *key);
int nyas_hmap_count(nyas_hmap *hm);
int nyas_hmap_capacity(nyas_hmap *hm);
void nyas_hmap_clear(nyas_hmap *hm);
void nyas_hmap_destroy(nyas_hmap *hm);

#endif
