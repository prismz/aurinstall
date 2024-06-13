#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef HM_DEBUG
#include <stdarg.h>
static void debug_print(char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
}
#else
/* compiler should just optimize this away */
static void debug_print()
{
}
#endif

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

/* hash a single byte */
static uint32_t fnv1a(unsigned char byte, uint32_t hash)
{
        return (byte ^ hash) * HM_FNV_PRIME;
}

static uint32_t fnv1a_hash(const char *str)
{
        uint32_t hash = HM_FNV_SEED;
        while (*str)
                hash = fnv1a((unsigned char)*str++, hash);
        return hash;
}

/* will duplicate key */
static struct bucket *new_bucket(const char *key, void *val)
{
        if (key == NULL)
                return NULL;

        struct bucket *b = HM_CALLOC_FUNC(1, sizeof(struct bucket));
        if (hm_check_mem(b))
                return NULL;

        b->key = HM_STRDUP_FUNC(key);
        if (hm_check_mem(b->key)) {
                free(b);
                return NULL;
        }

        b->val = val;
        b->next = NULL;

        b->hash = fnv1a_hash(key);

        debug_print("creating bucket with key %s, hash of this key is %" PRIu32 "\n",
                        key, b->hash);

        return b;
}

struct hashmap *hashmap_new(void (*val_free_func)(void *))
{
        /* Any smaller and we'd pretty much instantly have to resize */
        size_t capacity = HM_DEFAULT_HASHMAP_SIZE;

        debug_print("creating hashmap with capacity %ld\n", capacity);

        struct hashmap *hm = HM_CALLOC_FUNC(1, sizeof(struct hashmap));
        if (hm_check_mem(hm))
                return NULL;

        hm->capacity = capacity;
        hm->n_buckets = 0;
        hm->val_free_func = val_free_func;
        hm->buckets = HM_CALLOC_FUNC(capacity, sizeof(struct bucket *));
        if (hm_check_mem(hm->buckets)) {
                free(hm);
                return NULL;
        }

        return hm;
}

static void free_bucket(struct bucket *b, void (*val_free_func)(void *))
{
        if (b == NULL)
                return;

        struct bucket *curr;
        while (b != NULL) {
                curr = b;
                b = b->next;

                free(curr->key);
                if (val_free_func != NULL && curr->val != NULL)
                        val_free_func(curr->val);
                free(curr);
        }
}

void hashmap_free(struct hashmap *hm)
{
        if (hm == NULL)
                return;
        for (size_t i = 0; i < hm->capacity; i++) {
                if (hm->buckets[i] == NULL)
                        continue;
                free_bucket(hm->buckets[i], hm->val_free_func);
        }
        free(hm->buckets);
        free(hm);
}

int hashmap_resize(struct hashmap *hm)
{
        if (hm == NULL)
                return -1;

        size_t new_size = hm->capacity * HM_RESIZE_SCALE_FACTOR;
        debug_print("resizing hashmap from size %ld to %ld...\n", hm->capacity,
                        new_size);

        struct bucket **buckets = HM_CALLOC_FUNC(new_size,
                        sizeof(struct bucket *));
        if (hm_check_mem(buckets))
                return 1;

        for (size_t i = 0; i < hm->capacity; i++) {
                if (hm->buckets[i] == NULL)
                        continue;

                struct bucket *target = hm->buckets[i];
                struct bucket *curr;
                while (target != NULL) {
                        curr = target;
                        target = target->next;

                        size_t curr_new_idx = curr->hash % (uint32_t)(new_size);

                        if (buckets[curr_new_idx] == NULL) {
                                buckets[curr_new_idx] = curr;
                                buckets[curr_new_idx]->next = NULL;
                                continue;
                        }

                        /* collision */

                        struct bucket *ptr = buckets[curr_new_idx];
                        while (ptr->next != NULL)
                                ptr = ptr->next;

                        ptr->next = curr;
                        ptr->next->next = NULL;
                }

                /* size_t new_idx = hm->buckets[i]->hash % (uint32_t)new_size;
                debug_print("buckets at idx %ld move to idx %ld\n", i, new_idx);
                if (buckets[new_idx] != NULL) {
                        free(buckets);
                        return 1;
                }
                buckets[new_idx] = hm->buckets[i]; */
        }

        free(hm->buckets);
        hm->buckets = buckets;
        hm->capacity = new_size;

        return 0;
}

int hashmap_insert(struct hashmap *hm, const char *key, void *val)
{
        if (hm == NULL || key == NULL)
                return 1;

        if ((float)(hm->n_buckets + 1) >
                        (HM_HASHMAP_MAX_LOAD * (float)hm->capacity)) {
                debug_print("resize required... calling resize function\n");
                if (hashmap_resize(hm))
                        return -1;
        }

        hm->n_buckets++;
        struct bucket *b = new_bucket(key, val);
        if (hm_check_mem(b))
                return 1;
        size_t idx = (b->hash) % (uint32_t)(hm->capacity);

        struct bucket *target_bucket = hm->buckets[idx];
        if (target_bucket == NULL) {
                debug_print("insert: bucket with key %s goes to idx %ld\n", b->key, idx);
                hm->buckets[idx] = b;
                return 0;
        }

        /* changing value of something already in the hashmap */
        struct bucket *curr;
        while (target_bucket != NULL) {
                curr = target_bucket;
                target_bucket = target_bucket->next;
                if (curr->hash == b->hash && strcmp(curr->key, key) == 0) {
                        debug_print("key already exists, reassigning value\n");

                        /* we free the old value, should probably add an
                         * option for this */
                        if (hm->val_free_func != NULL)
                                hm->val_free_func(curr->val);
                        curr->val = val;
                        free_bucket(b, NULL);
                        return 0;
                }
        }

        debug_print("collision. appending to linked list\n");

        target_bucket = hm->buckets[idx];

        while (target_bucket->next != NULL)
                target_bucket = target_bucket->next;

        target_bucket->next = b;
        b->next = NULL;

        return 0;
}

void *hashmap_get(struct hashmap *hm, const char *key)
{
        if (hm == NULL || key == NULL)
                return NULL;

        uint32_t hash = fnv1a_hash(key);
        size_t idx = hash % (uint32_t)(hm->capacity);
        debug_print(
                "retreiving object with key %s - hashes to %" PRIu32 " giving idx %ld\n",
                key, hash, idx
        );

        struct bucket *target_bucket = hm->buckets[idx];
        if (target_bucket == NULL)
                return NULL;

        if (target_bucket->hash == hash && strcmp(target_bucket->key, key) == 0)
                return target_bucket->val;

        /* collision */
        struct bucket *curr;
        while (target_bucket != NULL) {
                curr = target_bucket;
                target_bucket = target_bucket->next;
                if (curr->hash == hash && strcmp(curr->key, key) == 0)
                        return curr->val;
        }

#ifdef HM_DONTTRUSTHASH
        /* search all items as a last resort */
        for (size_t i = 0; i < hm->capacity; i++) {
                struct bucket *b = hm->buckets[i];
                struct bucket *curr;
                while (b != NULL) {
                        curr = b;
                        b = b->next;

                        if (curr->hash == hash && strcmp(curr->key, key) == 0)
                                return curr->val;
                }
        }
#endif

        return NULL;
}

/*
 * wrapper function, so that if we don't find it with normal
 * methods, we can easily re-call the function with a custom index, allowing
 * us to iterate through the hashmap to find the item if for some
 * reason the hash gives an incorrect index
 */
static int _hashmap_remove(struct hashmap *hm, const char *key, uint32_t hash,
                size_t idx)
{
        if (hm == NULL || key == NULL)
                return 1;

        //uint32_t hash = fnv1a_hash(key);
        //size_t idx = hash % (uint32_t)(hm->capacity);

        debug_print(
                "removing object with key %s which hashes to %" PRIu32
                ". Supplied index to search is %ld\n",
                key, hash, idx
        );

        struct bucket *target = hm->buckets[idx];
        if (target == NULL)
                return 1;

        if (target->hash == hash && strcmp(target->key, key) == 0) {
                /* bucket with single item */
                if (target->next == NULL) {
                        debug_print("simple removal of bucket with single node\n");
                        free_bucket(hm->buckets[idx], hm->val_free_func);
                        hm->buckets[idx] = NULL;
                        hm->n_buckets--;
                        return 0;
                }

                /* bucket where first item is to be removed,
                 * but there are other items in the linked list */
                debug_print("removing first node and preserving the rest of the linked list\n");

                struct bucket *next = hm->buckets[idx]->next;
                hm->buckets[idx] = next;
                target->next = NULL;
                free_bucket(target, hm->val_free_func);
                return 0;
        }

        /* the bucket to be removed is not the first
         * bucket in the linked list */
        debug_print("bucket to be removed is within the linked list - searching...\n");
        if (target->next == NULL)
                return 1;

        target = target->next;

        struct bucket *curr = target;
        struct bucket *prev = NULL;


        while (target != NULL) {
                curr = target;
                target = target->next;

                if (curr->hash == hash && strcmp(curr->key, key) != 0) {
                        prev = curr;
                        continue;
                }

                if (prev == NULL) {
                        debug_print("this should never happen...\n");
                        return 1;
                }

                prev->next = curr->next;
                curr->next = NULL;
                free_bucket(curr, hm->val_free_func);

                return 0;
        }

        return 1;
}

int hashmap_remove(struct hashmap *hm, const char *key)
{
        if (hm == NULL || key == NULL)
                return 1;

        uint32_t hash = fnv1a_hash(key);
        size_t idx = hash % (uint32_t)(hm->capacity);

        if (_hashmap_remove(hm, key, hash, idx) == 0)
                return 0;

#ifdef HM_DONTTRUSTHASH
        debug_print("couldn't find item to remove by hashing, must search entire hashmap\n");
        for (size_t i = 0; i < hm->capacity; i++) {
                /* skip if we already processed the index or if the item is NULL */
                if (hm->buckets[i] == NULL || i == idx)
                        continue;

                if (_hashmap_remove(hm, key, hash, i) == 0) {
                        debug_print("successfully found and removed item at idx %ld\n", i);
                        return 0;
                }
        }

        debug_print("couldn't find item to remove anywhere, returning 1\n");
#endif

        return 1;
}

/* only works for hashmap with strings as values */
void hashmap_print(struct hashmap *hm)
{
        if (hm == NULL) {
                printf("NULL\n");
                return;
        }

        printf("Printing hashmap containing %ld buckets with capacity %ld:\n",
                        hm->n_buckets, hm->capacity);

        for (size_t i = 0; i < hm->capacity; i++) {
                struct bucket *target = hm->buckets[i];
                printf("bucket %02ld:\n", i);
                if (target == NULL)
                        continue;

                struct bucket *curr;
                int n = 0;
                while (target != NULL) {
                        curr = target;
                        target = target->next;
                        printf("    %02d: %s=%s\n", n,
                                        curr->key, (char *)curr->val);

                        n++;
                }

        }
}
