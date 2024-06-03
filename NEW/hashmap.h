/*
 * This file is part of aurinstall.
 *
 * aurinstall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * aurinstall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurinstall.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Copyright (C) 2023 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct HMItem HMItem;

struct HMItem {
	char *key;
	void *val;

	void (*k_free_func)(void *);
	void (*v_free_func)(void *);
};

#define hashmap_item_free_func(a) (void (*)(void *))a

typedef struct {
	HMItem **items;  /* buckets */

	size_t can_store;
	size_t stored;
} HashMap;

HMItem *new_item(char *key, void *val,
                void (*k_free_func)(void *), void (*v_free_func)(void *));
void free_item(HMItem *item);
HashMap *new_hashmap(size_t capacity);
void free_hashmap(HashMap *map);
int check_hashmap_capacity(HashMap *map, size_t n);
int hashmap_set(HashMap *map, HMItem *item);

/*
 * 64-bit FNV-1a hash:
 * https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 * https://github.com/benhoyt/ht/blob/master/ht.c
 */
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
uint64_t hashmap_hash_func(char *key);

void *hashmap_index(HashMap *map, char *key);
void hashmap_remove(HashMap *map, char *key);

#endif  /* HASHMAP_H */
