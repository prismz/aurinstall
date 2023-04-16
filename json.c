/*
 * json - A simple JSON library using only the standard C library.
 * Copyright (C) 2022 Hasan Zahra
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "json.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

HMItem *new_item(char *key, void *val,
                void (*k_free_func)(void *), void (*v_free_func)(void *))
{
	HMItem *item = calloc(1, sizeof(*item));
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
	HashMap *map = calloc(1, sizeof(*map));
        if (map == NULL)
                return NULL;

	map->can_store = capacity;
	map->stored = 0;
	map->items = calloc(map->can_store, sizeof(HMItem *));
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
	HMItem **new = calloc(new_size, sizeof(HMItem *));
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

/* Begin JSON implementation */

struct json *new_json(void)
{
        struct json *j = calloc(1, sizeof(struct json));
        if (j == NULL)
                return NULL;

        j->type = json_null;

        return j;
}

void free_json_item(struct json *j)
{
        if (j == NULL)
                return;

        switch (j->type) {
        case json_string:
                free(j->data.string);
                break;
        case json_array:
                for (int i = 0; i < j->n_data_items; i++)
                        free_json_item(j->data.json_data_array[i]);

                free(j->data.json_data_array);
                break;
        case json_dict:
                free_hashmap(j->data.json_data_dict);
                break;
        default:
                break;
        }

        free(j);
}

void print_json(struct json *j)
{
        HashMap *dict;
        switch (j->type) {
        case json_bool:
                if (j->data.boolean)
                        printf("true\n");
                else
                        printf("false\n");

                break;
        case json_string:
                printf("%s\n", j->data.string);
                break;
        case json_array:
                for (int i = 0; i < j->n_data_items; i++)
                        print_json(j->data.json_data_array[i]);
                break;
        case json_dict:
                dict = j->data.json_data_dict;
                for (int i = 0; i < (int)dict->can_store; i++) {
                        HMItem *item = dict->items[i];

                        if (item == NULL)
                                continue;

                        printf("\"%s\": ", dict->items[i]->key);
                        print_json(dict->items[i]->val);
                }
                break;
        case json_number:
                printf("%f\n", j->data.number);
                break;
        case json_null:
                printf("null\n");
                break;
        }
}

HMItem *json_parse_dict_tuple(char *str, int *idx)
{
        size_t len = strlen(str);
        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }

        int key_end_idx;
        struct json *json_key = json_parse_string(str + start, &key_end_idx);
        if (json_key == NULL)
                return NULL;

        key_end_idx += start;

        size_t val_start;
        for (val_start = key_end_idx; val_start < len; val_start++) {
                if (!isspace(str[val_start]) && str[val_start] != ':')
                        break;
        }

        int val_end_idx;
        struct json *val = json_parse_item(str + val_start, &val_end_idx);
        if (val == NULL) {
                free_json_item(json_key);
                return NULL;
        }

        val_end_idx += val_start;

        char *key = json_key->data.string;
        free(json_key);

        if (idx != NULL)
                *idx = val_end_idx;

        struct HMItem *tuple = new_item(key, val, free,
                        hashmap_item_free_func(free_json_item));
        if (tuple == NULL) {
                free(key);
                free_json_item(val);
                return NULL;
        }

        return tuple;
}

struct json *json_parse_dict(char *str, int *idx)
{
        struct json *j = new_json();
        if (j == NULL)
                return NULL;

        j->type = json_dict;

        size_t len = strlen(str);
        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }

        HashMap *dict = new_hashmap(16);
        if (dict == NULL) {
                free_json_item(j);
                return NULL;
        }

        j->data.json_data_dict = dict;

        bool done = false;
        size_t current_idx = start + 1;
        for (size_t i = start + 1; i < len; i++) {
                char c = str[i];

                if (isspace(c)) {
                        current_idx++;
                        continue;
                }

                if (c == '}') {
                        current_idx++;
                        break;
                }

                int end_idx;
                HMItem *current_tuple = json_parse_dict_tuple(
                                str + current_idx, &end_idx);

                if (current_tuple == NULL) {
                        free_json_item(j);
                        return NULL;
                }

                current_idx += end_idx;

                for (; current_idx < len; current_idx++) {
                        char cc = str[current_idx];
                        if (cc == '}') {
                                current_idx++;
                                done = true;
                                goto append;
                        }

                        if (!isspace(cc) && cc != ',')
                                break;
                }

append:
                i = current_idx;

                if (hashmap_set(dict, current_tuple)) {
                        free_json_item(j);
                        return NULL;
                } 

                if (done)
                        break;
        }

        j->n_data_items = j->data.json_data_dict->stored;
        j->data_list_capacity = j->data.json_data_dict->can_store;

        if (idx != NULL)
                *idx = current_idx;

        return j;
}

struct json *json_parse_string(char *str, int *idx)
{
        struct json *j = new_json();
        if (j == NULL)
                return NULL;

        j->type = json_string;

        size_t len = strlen(str);

        if (strncmp(str, "\"\"", strlen("\"\"")) == 0) {
                j->data.string = strdup("");
                if (j->data.string == NULL) {
                        free_json_item(j);
                        return NULL;
                }

                if (idx != NULL)
                        *idx = 2;

                return j;
        }

        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]) && str[start] != '"')
                        break;
        }

        size_t size = 1024;
        size_t buffer_i = 0;
        char *buffer = malloc(sizeof(char) * size);
        if (buffer == NULL) {
                free_json_item(j);
                return NULL;
        }

        bool escaped = false;
        size_t i;
        for (i = start; i < len; i++) {
                char c = str[i];
                if (c == '\\') {
                        escaped = true;
                        continue;
                }

                if (!escaped && c == '"')
                        break;

                if (buffer_i + 2 > size) {
                        char *nbuffer = realloc(buffer, sizeof(char) *
                                        (size += 512));
                        if (nbuffer == NULL) {
                                free(buffer);
                                free_json_item(j);
                                return NULL;
                        }
                        buffer = nbuffer;
                }

                buffer[buffer_i++] = c;
                buffer[buffer_i] = 0;

                if (escaped)
                        escaped = false;
        }

        if (i != start)
                i++;

        if (idx != NULL)
                *idx = i;

        j->data.string = buffer;

        return j;
}

struct json *json_parse_array(char *str, int *idx)
{
        struct json *j = new_json();
        if (j == NULL)
                return NULL;

        j->type = json_array;

        size_t len = strlen(str);

        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }
        start++;  /* skip first bracket */

        for (; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }


        size_t capacity = 4;
        size_t array_i = 0;
        struct json **array = calloc(capacity,
                        sizeof(struct json *));

        if (array == NULL) {
                free_json_item(j);
                return NULL;
        }
        
        j->data.json_data_array = array;

        if (str[start]== ']') {
                j->n_data_items = array_i;
                j->data_list_capacity = capacity;
                if (idx != NULL)
                        *idx = start + 1;
                return j;
        }

        size_t current_point = start;
        size_t i;
        for (i = start; i < len; i++) {
                int current_element_end;
                struct json *elem = json_parse_item(str + current_point,
                                &current_element_end);

                if (elem == NULL) {
                        free_json_item(j);
                        return NULL; 
                }

                current_point += current_element_end;
                for (; current_point < len; current_point++) {
                        char cc = str[current_point];
                        if (!isspace(cc) && cc != ',')
                                break;
                }

                if (array_i + 1 > capacity) {
                        struct json **narray = realloc(array,
                                    sizeof(struct json *) * (capacity += 4));
                        if (narray == NULL) {
                               free_json_item(elem);
                               free_json_item(j);
                               return NULL;
                        }

                        array = narray;
                        j->data.json_data_array = narray;
                }

                array[array_i++] = elem;

                if (str[current_point] == ']') {
                        i = current_point + 1;
                        break;
                }

                i = current_point;
        }

        j->n_data_items = array_i;
        j->data_list_capacity = capacity;

        if (idx != NULL)
                *idx = i;

        return j;
}

struct json *json_parse_number(char *str, int *idx)
{
        struct json *j = new_json();
        if (j == NULL)
                return NULL;
        j->type = json_number;

        size_t len = strlen(str);
        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }

        size_t i;

        size_t capacity = 16;
        size_t data_i = 0;
        char *data = malloc(sizeof(char) * capacity);
        if (data == NULL) {
                free_json_item(j);
                return NULL;
        }

        for (i = start; i < len; i++) {
                char c = str[i];
                if (!isdigit(c) && c != '.' && c != 'e')
                        break;

                if (data_i + 2 > capacity) {
                        char *ndata = realloc(data,
                                        sizeof(char) * (capacity += 4));
                        if (ndata == NULL) {
                                free_json_item(j);
                                free(data);
                                return NULL;
                        }
                        data = ndata;
                }

                data[data_i++] = c;
                data[data_i] = 0;
        }

        sscanf(data, "%lf", &j->data.number);
        free(data);

        if (idx != NULL)
                *idx = i;

        return j;
}

struct json *json_parse_bool(char *str, int *idx)
{
        struct json *j = new_json();
        j->type = json_bool;

        size_t len = strlen(str);
        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }

        int stop_idx = start;
        size_t tl = strlen("true");
        size_t fl = strlen("false");

        if (strncmp(str + start, "true", tl) == 0) {
                j->data.boolean = true;
                stop_idx += tl;
        } else {
                j->data.boolean = false;
                stop_idx += fl;
        }

        if (idx != NULL)
                *idx = stop_idx;

        return j;
}

struct json *json_parse_null(char *str, int *idx)
{
        struct json *j = new_json();
        j->type = json_null;

        size_t len = strlen(str);
        size_t start;
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }

        int end_idx = start + strlen("null");
        if (idx != NULL)
                *idx = end_idx;

        return j;
}

struct json *json_parse_item(char *str, int *idx)
{
        size_t len = strlen(str);
        size_t start;

        /* skip leading whitespace if there is any */
        for (start = 0; start < len; start++) {
                if (!isspace(str[start]))
                        break;
        }

        int end_idx = 0;
        struct json *j;

        char c = str[start];
        switch (c) {
        case '{':
                j = json_parse_dict(str + start, &end_idx);
                break;
        case '[':
                j = json_parse_array(str + start, &end_idx);
                break;
        case '"':
                j = json_parse_string(str + start, &end_idx);
                break;
        default:
                if (c == 'n')  /* null */
                        j = json_parse_null(str + start, &end_idx);
                else if (isdigit(c))  /* number */
                        j = json_parse_number(str + start, &end_idx);
                else if (c == 't' || c == 'f')  /* true / false */
                        j = json_parse_bool(str + start, &end_idx);

                break;
        }

        if (idx != NULL)
                *idx = end_idx;

        return j;
}

struct json *json_get_array_item(struct json *arr, int idx)
{
        if (idx >= arr->n_data_items)
                return NULL;

        return arr->data.json_data_array[idx];
}

struct json *json_get_dict_item(struct json *dict, char *key)
{
        HashMap *map = dict->data.json_data_dict;
        return (struct json *)hashmap_index(map, key);
}

int json_get_size(struct json *arr)
{
        return arr->n_data_items;
}

int json_get_capacity(struct json *arr)
{
        return arr->data_list_capacity;
}

bool _is_whole_number(char *s)
{
        size_t len = strlen(s);
        for (size_t i = 0; i < len; i++) {
                if (!isdigit(s[i]))
                        return false;
        }

        return true;
}

struct json *json_access(struct json *j, ...)
{
        va_list ap;
        va_start(ap, j);

        struct json *current = j;

        char *arg;
        while ((arg = va_arg(ap, char *)) != NULL) {
                struct json *s;
                if (_is_whole_number(arg)) {
                        int d;
                        sscanf(arg, "%d", &d);

                        s = json_get_array_item(current, d);
                        if (s == NULL)
                                return NULL;
                } else {
                        s = json_get_dict_item(current, arg);
                        if (s == NULL)
                                return NULL;
                }


                current = s;
        }

        va_end(ap);

        return current;
}

struct json *json_safe_access(struct json *j, char *fmt, ...)
{
        /* if fmt is read-only strtok() will fail */
        char *fmt_cpy = strdup(fmt);  
        if (fmt_cpy == NULL)
                return NULL;

        va_list ap;
        va_start(ap, fmt);

        struct json *current = j;

        char *ptr = strtok(fmt_cpy, " ");
        while (ptr != NULL) {
                if (strcmp(ptr, "%s") == 0) {
                        char *arg = va_arg(ap, char *);
                        current = json_get_dict_item(current, arg);
                        if (current == NULL)
                                goto fail;
                } else if (strcmp(ptr, "%d") == 0) {
                        int arg = va_arg(ap, int);
                        current = json_get_array_item(current, arg);
                        if (current == NULL)
                                goto fail;
                } else {
                        goto fail;
                }

                ptr = strtok(NULL, " ");
        }

        free(fmt_cpy);
        va_end(ap);

        return current;

fail:
        free(fmt_cpy);
        va_end(ap);
        return NULL;
}

char *json_read_file(char *path)
{
        FILE *fp = fopen(path, "rb");
        if (!fp)
                return NULL;

        size_t capacity = 1024;
        size_t buffer_i = 0;
        char *buffer = malloc(sizeof(char) * capacity);
        if (buffer == NULL) {
                fclose(fp);
                return NULL;
        }

        char c;
        while ((c = fgetc(fp)) != EOF) {
                if (buffer_i + 2 > capacity) {
                        char *nbuffer = realloc(buffer,
                                        sizeof(char) * (capacity += 512));
                        if (nbuffer == NULL) {
                                free(buffer);
                                fclose(fp);
                                return NULL;
                        }
                        buffer = nbuffer;
                }

                buffer[buffer_i++] = c;
                buffer[buffer_i] = 0;
        }

        fclose(fp);

        return buffer;
}
