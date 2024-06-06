#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdint.h>

#define HM_CALLOC_FUNC calloc
#define HM_STRDUP_FUNC strdup
#define HM_EXIT_ON_ALLOC_FAIL 1

/*
 * if using structs as values, this conveniently
 * typecasts your struct's free function
 * so that it can be passed to new_hashmap_item()
 */
#define hashmap_item_free_func(a) (void (*)(void *))a

#define HM_FNV_OFFSET 14695981039346656037UL
#define HM_FNV_PRIME 1099511628211UL

struct hashmap_item;
struct hashmap_item {
        char *key;
        uint32_t hash;
        void *val;
        void (*val_free_func)(void *);

        /* linked list to store collisions */
        struct hashmap_item *next;
};

struct hashmap {
       struct hashmap_item **items;
       size_t n;
       size_t capacity;
};

struct hashmap_item *new_hashmap_item(const char *key, void *val,
                void (*val_free_func)(void *));
struct hashmap *new_hashmap(size_t capacity);
void free_hashmap(struct hashmap *hm);
uint32_t hm_fnv1a_hash(const char *str, size_t size);
int hashmap_insert(struct hashmap *hm, struct hashmap_item *item);
int hashmap_remove(struct hashmap *hm, const char *key);
void *hashmap_get(struct hashmap *hm, const char *key);
void hashmap_print(struct hashmap *hm);

#endif  /* HASHMAP_H */
