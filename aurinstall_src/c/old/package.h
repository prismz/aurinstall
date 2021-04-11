#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "simple-json/simple_json.h"
#include "globals.h"

PackageData parse_package_json(char package_json[]) {
    PackageData data;
    char* name = json_parse_dict(package_json, "Name");
    char* desc = json_parse_dict(package_json, "Description");
    char* ver  = json_parse_dict(package_json, "Version");
    char* ood  = json_parse_dict(package_json, "OutOfDate");
    char* base_url = "https://aur.archlinux.org/";
    char* url = malloc(sizeof(char)*(strlen(name)+strlen(base_url)+10));

    strcpy(url, base_url);
    strcat(url, name);
    strcat(url, ".git");
    remquotes(name);
    remquotes(desc);
    remquotes(ver);
    remquotes(ood);

    data.name = name;
    data.desc = desc;
    data.ver = ver;
    data.ood = ood;
    data.url = url;

    return data;
}

void print_package_data(PackageData data, Options* opts) {
    if (opts->normal_term) {
        printf("%saur/%s%s %s%s%s\n", RED, ENDC, data.name, GREEN, data.ver, ENDC);
        pretty_print(4, data.desc, opts);
    } else {
        printf("aur/%s %s\n", data.name, data.ver);
        printf("    %s\n", data.desc);
    }
}

void search_package(char* searchterm, char args[][MAX_ARGLEN], int argcount, Options* opts) {
    char url[52+strlen(searchterm)];
    strcpy(url, "https://aur.archlinux.org/rpc/?v=5&type=search&arg=");
    strcat(url, searchterm);

    struct curl_res_string response = get(url);
    char* _resultcount = json_parse_dict(response.ptr, "resultcount");
    char* ptr;
    long resultcount = strtol(_resultcount, &ptr, 10);

    free(_resultcount);

    if (resultcount == 0)
        return;

    char* results = json_parse_dict(response.ptr, "results");

    for (int i = 0; i < resultcount; i++) {
        char* current_package_json = json_parse_arr(results, i);
        PackageData data = parse_package_json(current_package_json);

        for (int j = 0; j < argcount; j++) {
            char* cstr = args[j];
            // printf("%s ", cstr);
            // if (strstr(data.name, cstr))
        }

        print_package_data(data, opts);
        free_package_data(data);
        free(current_package_json);
    }

    free(response.ptr);
    free(results);
}

void install_package(PackageData* data) {
    char cmd[200] = "rm -rf ";
    strcat(cmd, cache_path); 
    strcat(cmd, "/");
    strcat(cmd, data->name);
    strcat(cmd, " && ");
    strcat(cmd, "git clone  ");
    strcat(cmd, git_args);
    strcat(cmd, "  https://aur.archlinux.org/");
    strcat(cmd, data->name);
    strcat(cmd, ".git ");
    strcat(cmd, cache_path);
    strcat(cmd, "/");
    strcat(cmd, data->name);
    printf("%s\n", cmd);
    int gitclone_returncode = system(cmd);

    if (gitclone_returncode != 0) {
        printf("ERROR: error cloning from AUR.\n");
        exit(1);
    }

    char makepkg_cmd[500] = "cd ";
    strcat(makepkg_cmd, cache_path);
    strcat(makepkg_cmd, "/");
    strcat(makepkg_cmd, data->name);
    strcat(makepkg_cmd, " && ");
    strcat(makepkg_cmd, "makepkg -si ");
    strcat(makepkg_cmd, makepkg_args);

    int makepkg_code = system(makepkg_cmd);
    if (makepkg_cmd != 0) {
        printf("ERROR: error running makepkg.\n");
        exit(1);
    }


}