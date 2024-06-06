#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int hm_check_mem(void *ptr)
{
        if (ptr != NULL)
                return 0;

        if (HM_EXIT_ON_ALLOC_FAIL) {
                fprintf(stderr, "failed to allocate memory\n");
                exit(1);
        }

        return 1;
}

/* will duplicate key, but not val */
struct hashmap_item *new_hashmap_item(const char *key, void *val,
                void (*val_free_func)(void *))
{
        if (key == NULL || val == NULL)
                return NULL;

        struct hashmap_item *item = HM_CALLOC_FUNC(1,
                        sizeof(struct hashmap_item));

        if (hm_check_mem(item))
                return NULL;

        item->key = HM_STRDUP_FUNC(key);
        if (hm_check_mem(item)) {
                free(item);
                return NULL;
        }

        item->val = val;
        item->val_free_func = val_free_func;

        item->next = NULL;

        item->hash = hm_fnv1a_hash(key, strlen(key));

        return item;
}

struct hashmap *new_hashmap(size_t capacity)
{
        if (capacity == 0)
                return NULL;

        struct hashmap *hm = HM_CALLOC_FUNC(1, sizeof(struct hashmap));
        if (hm_check_mem(hm))
                return NULL;

        struct hashmap_item **items = HM_CALLOC_FUNC(capacity,
                        sizeof(struct hashmap_item *));

        if (hm_check_mem(items)) {
                free(hm);
                return NULL;
        }

        hm->capacity = capacity;
        hm->n = 0;
        hm->items = items;

        return hm;
}

static void free_hashmap_item(struct hashmap_item *item)
{
        if (item == NULL)
                return;

        struct hashmap_item *curr;
        size_t i = 0;
        while (item != NULL) {
                curr = item;
                item = item->next;
                free(curr->key);
                if (curr->val_free_func != NULL)
                        curr->val_free_func(curr->val);
                free(curr);
                i++;
        }
}

void free_hashmap(struct hashmap *hm)
{
        if (hm == NULL)
                return;

        for (size_t i = 0; i < hm->capacity; i++) {
                if (hm->items[i] == NULL)
                        continue;
                free_hashmap_item(hm->items[i]);
        }
        free(hm->items);
        free(hm);
}

#define HASHMAP_HASH_INIT 2166136261u
uint32_t hm_fnv1a_hash(const char* data, size_t size)
{
	size_t nblocks = size / 8;
	uint64_t hash = HASHMAP_HASH_INIT;
	for (size_t i = 0; i < nblocks; ++i)
	{
		hash ^= (uint64_t)data[0] << 0 | (uint64_t)data[1] << 8 |
			 (uint64_t)data[2] << 16 | (uint64_t)data[3] << 24 |
			 (uint64_t)data[4] << 32 | (uint64_t)data[5] << 40 |
			 (uint64_t)data[6] << 48 | (uint64_t)data[7] << 56;
		hash *= 0xbf58476d1ce4e5b9;
		data += 8;
	}

	uint64_t last = size & 0xff;
	switch (size % 8)
	{
	case 7:
		last |= (uint64_t)data[6] << 56; /* fallthrough */
	case 6:
		last |= (uint64_t)data[5] << 48; /* fallthrough */
	case 5:
		last |= (uint64_t)data[4] << 40; /* fallthrough */
	case 4:
		last |= (uint64_t)data[3] << 32; /* fallthrough */
	case 3:
		last |= (uint64_t)data[2] << 24; /* fallthrough */
	case 2:
		last |= (uint64_t)data[1] << 16; /* fallthrough */
	case 1:
		last |= (uint64_t)data[0] << 8;
		hash ^= last;
		hash *= 0xd6e8feb86659fd93;
	}

	// compress to a 32-bit result.
	// also serves as a finalizer.
	return (uint32_t)(hash ^ hash >> 32);
}

static int check_hashmap_capacity(struct hashmap *hm)
{
        if (hm == NULL)
                return 1;
        if (hm->n + 1 < hm->capacity)
                return 0;

        size_t increase_value = 8;
        size_t new_size = hm->capacity + increase_value;

        struct hashmap_item **items = HM_CALLOC_FUNC(hm->capacity + increase_value,
                        sizeof(struct hashmap_item *));
        if (hm_check_mem(items))
                return 1;

        for (size_t i = 0; i < hm->capacity; i++) {
                struct hashmap_item *item = hm->items[i];
                if (item == NULL)
                        continue;

                items[item->hash % (uint32_t)(hm->capacity)] = item;
        }
        free(hm->items);
        hm->items = items;
        hm->capacity = new_size;

        return 0;
}

int hashmap_insert(struct hashmap *hm, struct hashmap_item *item)
{
        if (hm == NULL || item == NULL)
                return -1;

        if (check_hashmap_capacity(hm))
                return -1;

        size_t idx = (size_t)(item->hash % (uint32_t)(hm->capacity));

        /*printf("insert key %s at idx %ld with cap of %ld\n",
                        item->key, idx, hm->capacity); */

        if (hm->items[idx] == NULL) {
                hm->items[idx] = item;
                hm->n++;
                return idx;
        }

        /* collision, handle linked list */
        struct hashmap_item *target_bucket = hm->items[idx];
        while (target_bucket->next != NULL)
                target_bucket = target_bucket->next;

        target_bucket->next = item;
        item->next = NULL;
        hm->n++;

        return idx;
}

/*
int hashmap_remove(struct hashmap *hm, const char *key)
{
}
*/

void *hashmap_get(struct hashmap *hm, const char *key)
{
        if (hm == NULL || key == NULL)
                return NULL;

        uint32_t hash = hm_fnv1a_hash(key, strlen(key));
        size_t idx = (size_t)(hash % (uint32_t)(hm->capacity));

        struct hashmap_item *item = hm->items[idx];
        if (item == NULL)
                return NULL;

        if (strcmp(item->key, key) == 0)
                return item->val;

        /* not in the first bucket, iterate through linked list */
        struct hashmap_item *curr;
        while (item != NULL) {
                curr = item;
                item = item->next;
                if (strcmp(curr->key, key) == 0)
                        return curr->val;
        }

        return NULL;
}

/* only for maps with strings as values */
void hashmap_print(struct hashmap *hm)
{
        printf("%ld entries of cap %ld\n", hm->n, hm->capacity);
        for (size_t i = 0; i < hm->capacity; i++) {
                printf("%02ld: ", i);
                struct hashmap_item *item = hm->items[i];
                if (item == NULL) {
                        printf("NULL\n");
                        continue;
                }

                struct hashmap_item *curr;
                while (item != NULL) {
                        curr = item;
                        item = item->next;
                        printf("%s=%s ", curr->key, (char *)curr->val);
                }
                printf("\n");
        }
}


