#include "hmap.h"
#include "core/log.h"
#include "core/mem.h"

#include <stddef.h>
#include <string.h>

#define hdr(HM) ((struct hm_hdr*)(HM) - 1)
#define hmcap(HM) (hdr(HM)->capacity)
#define hmcount(HM) (hdr(HM)->count)
#define hmnodesz(HM) (hdr(HM)->node_size)
#define hmvalsz(HM) (hdr(HM)->value_size)

struct hm_hdr {
	uint32_t capacity;
	uint32_t count;
	uint32_t node_size;
	uint32_t value_size;
};

typedef uint64_t hm_key;

struct nyas_hmap {
	hm_key key;
	void *value;
};

static inline unsigned int next_pow2(unsigned int v)
{
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

static inline uint64_t hm_hash(const char *key)
{
	hm_key mk;
	mk = 0;
	strncpy((char*)mk, key, 8);
	return mk;
}

static inline nyas_hmap *getnode(nyas_hmap *hm, unsigned int offset)
{
	return (nyas_hmap *)((uint8_t*)hm + hmnodesz(hm) * offset);
}

nyas_hmap *
nyas_hmap_create(unsigned int capacity, unsigned int value_size)
{
	NYAS_ASSERT(capacity <= (1U << 31));
	capacity = next_pow2(capacity);
	size_t map_size = (value_size + sizeof(hm_key)) * capacity
		+ sizeof(struct hm_hdr);
	struct hm_hdr *hm = NYAS_CALLOC(map_size, 1);

	hm->capacity = capacity;
	hm->count = 0;
	hm->node_size = value_size + sizeof(hm_key);
	hm->value_size = value_size;

	return (nyas_hmap *)(hm + 1);
}

void
nyas_hmap_destroy(nyas_hmap *hm)
{
	NYAS_FREE((struct hm_hdr *)hm - 1);
}

void
nyas_hmap_insert(nyas_hmap *hm, const char *key, void *value)
{
	NYAS_ASSERT(value);
	NYAS_ASSERT(hmcount(hm) < hmcap(hm)); // Change this to 80% capacity

	hm_key mk;
	mk = hm_hash(key);
	uint32_t offset = mk & (hmcap(hm) - 1);

	nyas_hmap *node;
	for (node = getnode(hm, offset); node->key != 0; node = getnode(hm, offset)) {
		if (node->key == mk) {
			memcpy(&(node->value), value, (hmvalsz(hm)));
			NYAS_LOG("Key %s already here!\n", key);
			return;
		}
		offset = (offset + 1) & (hmcap(hm) - 1);
	}

	node->key = mk;
	memcpy(&(node->value), value, hmvalsz(hm));
	++hmcount(hm);
}

void *
nyas_hmap_get(nyas_hmap *hm, const char *key)
{
	hm_key mk;
	mk = hm_hash(key);
	uint32_t offset = mk & (hmcap(hm) - 1);

	nyas_hmap *node;
	for (node = getnode(hm, offset); node->key != mk; node = getnode(hm, offset)) {
		if (node->key == 0) {
			return NYAS_HMAP_INVALID_VALUE;
		}
		offset = (offset + 1) & (hmcap(hm) - 1);
	}

	return (void*)(&(node->value));
}

void
nyas_hmap_clear(nyas_hmap *hm)
{
	memset(hm, 0, hmcap(hm) * hmnodesz(hm));
	hmcount(hm) = 0;
}

int
nyas_hmap_count(nyas_hmap *hm)
{
	return hmcount(hm);
}

int
nyas_hmap_capacity(nyas_hmap *hm)
{
	return hmcap(hm);
}
