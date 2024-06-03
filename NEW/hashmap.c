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

#include "hashmap.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

HMItem *new_item(char *key, void *val,
                void (*k_free_func)(void *), void (*v_free_func)(void *))
{
	HMItem *item = safe_calloc(1, sizeof(*item));
        if (item == NULL)
                return NULL;

	item->key = key;
	item->val = val;
	item->k_free_func = k_free_func;
	item->v_free_func = v_free_func;

	return item;
}

void free_item(HMItem *item)
{
	if (item == NULL)
		return;

	if (item->key != NULL && item->k_free_func != NULL)
		item->k_free_func(item->key);
	if (item->val != NULL && item->v_free_func != NULL)
		item->v_free_func(item->val);

	free(item);
}

HashMap *new_hashmap(size_t capacity)
{
	HashMap *map = safe_calloc(1, sizeof(*map));
        if (map == NULL)
                return NULL;

	map->can_store = capacity;
	map->stored = 0;
	map->items = safe_calloc(map->can_store, sizeof(HMItem *));
        if (map->items == NULL) {
                free(map);
                return NULL;
        }

	return map;
}

void free_hashmap(HashMap *map)
{
	if (map == NULL)
		return;

	for (size_t i = 0; i < map->can_store; i++)
		free_item(map->items[i]);

	free(map->items);
	free(map);
}

int check_hashmap_capacity(HashMap *map, size_t n)
{
	if (!((map->stored + n + 1) > map->can_store))
		return 0;

        int ns_a = ((n + 8) > 32) ? (n + 8) : 32;
	size_t new_size = map->can_store + ns_a;
	HMItem **new = safe_calloc(new_size, sizeof(HMItem *));
        if (new == NULL)
                return 1;

	for (size_t i = 0; i < map->can_store; i++) {
		HMItem *item = map->items[i];
		if (item != NULL)
			new[i] = item;
	}

	free(map->items);
	map->can_store = new_size;
	map->items = new;

        return 0;
}

int hashmap_set(HashMap *map, HMItem *item)
{
	if (check_hashmap_capacity(map, 1))
                return 1;

	uint64_t hash = hashmap_hash_func(item->key);
	size_t index = (size_t)(hash & (uint64_t)(map->can_store - 1));

	/* find an empty entry */
	while (map->items[index] != NULL) {
		if (strcmp(item->key, map->items[index]->key) == 0) {
			free_item(map->items[index]);
			map->items[index] = item;
			return 0;
		}

		index++;
		if (index >= map->can_store)
			index = 0;
	}

	map->items[index] = item;
	map->stored++;
        
        return 0;
}

/* TODO: https://github.com/benhoyt/ht/blob/master/ht.c */
uint64_t hashmap_hash_func(char *key)
{
	uint64_t hash = FNV_OFFSET;
	for (const char* p = key; *p; p++) {
		hash ^= (uint64_t)(unsigned char)(*p);
		hash *= FNV_PRIME;
	}
	return hash;
}

void *hashmap_index(HashMap *map, char *key)
{
	uint64_t hash = hashmap_hash_func(key);
	size_t index = (size_t)(hash & (uint64_t)(map->can_store - 1));

        if (map->items[index] == NULL)
                goto collision;

	if (strcmp(map->items[index]->key, key) == 0)
		return map->items[index]->val;

collision:

        for (size_t i = 0; i < map->can_store; i++) {
                if (map->items[i] == NULL)
                        continue;
                if (strcmp(map->items[i]->key, key) == 0)
                        return map->items[i]->val;
	}

	return NULL;
}

void hashmap_remove(HashMap *map, char *key)
{
	uint64_t hash = hashmap_hash_func(key);
	size_t index = (size_t)(hash & (uint64_t)(map->can_store - 1));

	while (map->items[index] != NULL) {
		if (strcmp(map->items[index]->key, key) == 0) {
			free_item(map->items[index]);
			map->items[index] = NULL;
			map->stored--;
		}

		/* collision */
		index++;
		if (index >= map->can_store)
			index = 0;
	}
}
