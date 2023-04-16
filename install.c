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

#include "install.h"
#include "requests.h"
#include "alloc.h"
#include "util.h"
#include "rpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>

int import_pgp_keys(char *path)
{
        char *full_path = safe_malloc(sizeof(char) * PATH_MAX);
        snprintf(full_path, PATH_MAX, "%s/.SRCINFO", path);

        FILE *fp = fopen(full_path, "rb");
        if (!fp) {
                free(full_path);
                return 1;
        }

        char buf[1024];
        while ((fgets(buf, 1024, fp)) != NULL) {
                size_t len = strlen(buf);
                size_t i;
                for (i = 0; i < len; i++) {
                        if (!isspace(buf[i]))
                                break;
                }

                char *searchterm = "validpgpkeys";
                if (strncmp(searchterm, buf + i, strlen(searchterm)) != 0)
                        continue;

                char *stripped = buf + i;
                for (i = 0; i < strlen(stripped); i++) {
                        char c = stripped[i];
                        if (c == '=') {
                                i++;
                                break;
                        }
                }

                char *key = stripped + i;

                if (snsystem("gpg --recv-keys %s", 
                                        32 + strlen(key), key) != 0) {
                        fprintf(stderr, "failed to import key: %s\n", key);
                        return 1;
                }
        }

        free(full_path);
        fclose(fp);

        return 0;
}

int install_package(char *name, char *cache_path)
{
        int rc = 0;

        struct rpc_data *api_json_result = NULL;
        struct json *pkg_json = NULL;
        struct package *pkg_info = NULL;

        size_t api_info_str_len = strlen(name) + 128;
        char *api_info_str = safe_malloc(sizeof(char) * api_info_str_len);
        snprintf(api_info_str, api_info_str_len,
                        "https://aur.archlinux.org/rpc/?v=5&type=info&arg=%s",
                        name
        );

        api_json_result = make_rpc_request(api_info_str);

        assert(api_json_result->resultcount == 1);
        free(api_info_str);

        pkg_json = json_get_array_item(api_json_result->results, 0);
        pkg_info = parse_package_json(pkg_json);

        if (pkg_info->outofdate) {
                bool shouldcontinue = yesno_prompt(
                                "Package is out of date. Continue?", false);
                if (!shouldcontinue)
                        goto end;
        }

        char *dest_path = safe_malloc(sizeof(char) * PATH_MAX);
        snprintf(dest_path, PATH_MAX, "%s/%s", cache_path, name);

        printf("installing to directory %s\n", dest_path);

        int should_clone = 1;
        if (!dir_is_empty(dest_path)) {
                char *prompt = "Existing package data found. Rebuild?";
                should_clone = yesno_prompt(prompt, false);
                
                if (should_clone)
                        snsystem("rm -rfv %s", PATH_MAX + 16, dest_path);
        }

        if (should_clone) {
                char *fmt = "git clone https://aur.archlinux.org/%s.git %s";
                int git_clone_r = snsystem(fmt, 
                                strlen(pkg_info->name) + strlen(dest_path) + 64,
                                pkg_info->name, dest_path
                );

                if (git_clone_r) {
                        fprintf(stderr, "Failed to clone git repository.\n");
                        rc = 1;
                        goto end;
                }
        }

        if (import_pgp_keys(dest_path)) {
                fprintf(stderr, "failed to import PGP keys.\n");
                rc = 1;
                goto end;
        }

        char *makepkg_fmt = "cd %s && makepkg -si";
        int makepkg_r = snsystem(makepkg_fmt, PATH_MAX + 128, dest_path);
        
        if (makepkg_r) {
                fprintf(stderr, "makepkg failed.\n");
                rc = 1;
        }

end:
        free(dest_path);
        free_rpc_data(api_json_result);
        free_package_data(pkg_info);

        return rc;
}

/* returns a hashmap with the package names and values */
HashMap *get_installed_packages(void)
{
        HashMap *installed_packages = new_hashmap(16);
        if (installed_packages == NULL)
                return NULL;

        FILE *fp = popen("pacman -Qm", "r");
        char *line_buf = safe_malloc(sizeof(char) * 1024);

        while (fgets(line_buf, 1024, fp) != NULL) {
                size_t len = strlen(line_buf);
                line_buf[len - 1] = '\0';
                len--;
                
                int splitspace_idx = -1;
                for (size_t i = 0; i < len; i++) {
                        if (line_buf[i] == ' ') {
                                splitspace_idx = (int)i;
                                break;
                        }
                }

                if (splitspace_idx == -1) {
                        fprintf(stderr, "invalid package atom: %s\n", line_buf);
                        free_hashmap(installed_packages);
                        pclose(fp);
                        free(line_buf);

                        return NULL;
                }

                line_buf[splitspace_idx] = '\0';
                char *version_ro = line_buf + splitspace_idx + 1;
                char *name_ro = line_buf;

                char *name = safe_strdup(name_ro);
                char *version = safe_strdup(version_ro);

                HMItem *item = new_item(name, version, free, free);
                if (hashmap_set(installed_packages, item) != 0) {
                        fprintf(stderr, "failed to set hashmap item.\n");
                        free_hashmap(installed_packages);
                        pclose(fp);
                        free(name);
                        free(version);
                        free(line_buf);

                        return NULL;
                }
        }

        free(line_buf);
        pclose(fp);

        return installed_packages;
}

int update_packages(char *cache_path)
{
        int repeat = 0;  /* multiple updates needed */

        HashMap *installed_packages = get_installed_packages();
        if (installed_packages == NULL)
                return 1;

        /* no installed AUR packages */
        if (installed_packages->stored == 0)
                return 0;

        /* 
         * build one long api request - this is tedious, but works better for
         * not DDoS'ing the AUR.
         */
        char *fmt = safe_calloc(1, sizeof(char) * 4443);
        strcpy(fmt, "https://aur.archlinux.org/rpc/?v=5&type=info");

        /* keep track of the longest package for nicer formatting below */
        size_t largest_package_name_length = 0;

        for (size_t i = 0; i < installed_packages->can_store; i++) {
                HMItem *item = installed_packages->items[i];
                if (item == NULL)
                        continue;

                char *key = item->key;
                size_t klen = strlen(key);

                if (klen > largest_package_name_length)
                        largest_package_name_length = klen;

                char *delim = "&arg[]=";
                size_t added = strlen(delim) + klen;

                if (added + strlen(fmt) >= 4443) {
                        repeat = 1;
                        break;
                }

                strcat(fmt, "&arg[]=");
                strcat(fmt, key);
        }

        struct rpc_data *data = make_rpc_request(fmt);
        if (data == NULL) {
                free(fmt);
                free_hashmap(installed_packages);
                return 1;
        }


        size_t update_queue_i = 0;
        char *update_queue[1024];

        printf("The following have updates:\n"); 
        /* parse the results */
        struct json *results = data->results;
        for (size_t i = 0; i < data->resultcount; i++) {
                struct json *pkg_j = json_get_array_item(results, i);
                struct package *pkg = parse_package_json(pkg_j);

                char *name = pkg->name;

                char *new_version = pkg->version;
                char *installed_version = hashmap_index(installed_packages,
                                name);

                /* add to list of old packages */
                if (strcmp(new_version, installed_version) != 0) {
                        update_queue[update_queue_i++] = safe_strdup(name);

                        char name_fmt[1024];
                        snprintf(name_fmt, 1024, "%%-%zus: ", 
                                        largest_package_name_length + 3);

                        printf(name_fmt, name);
                        printf("%s -> %s\n", installed_version, new_version);

                        if (update_queue_i + 1 > 1024) {
                                repeat = 1;
                                free_package_data(pkg);
                                break;
                        }
                }

                free_package_data(pkg);
        }

        bool should_update = yesno_prompt("update?", true);
        if (!should_update) {
                repeat = 0;
                goto end;
        }

        for (size_t i = 0; i < update_queue_i; i++) {
                char *name = update_queue[i];
                snsystem("rm -rf %s/%s", 2048, cache_path, name);

                install_package(name, cache_path);

                free(update_queue[i]);
        }

end:
        free(fmt);
        free_rpc_data(data);
        free_hashmap(installed_packages);

        if (repeat) {
                printf("Multiple updates needed. Re-running.\n");
                return update_packages(cache_path);
        }

        return 0;
}

int clean_cache(char *cache_path)
{
        return snsystem("rm -rfv %s", PATH_MAX + 32, cache_path);
}

int remove_packages(int n, char **pkgs)
{
        size_t curr_len = 0;
        size_t capacity = 4096;
        char *str = safe_malloc(sizeof(char) * capacity);
        for (int i = 0; i < n; i++) {
                char *pkg = pkgs[i];
                size_t len = strlen(pkg);
                if (curr_len + len + 3 > capacity) {
                        int extra = (len > 32) ? len : 32;
                        capacity += extra;
                        str = safe_realloc(str, sizeof(char) * capacity);
                }

                strcat(str, pkg);
                strcat(str, " ");
        }

        return snsystem("pacman -R %s", 32 + curr_len, str);
}
