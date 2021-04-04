#pragma once

#include <stdio.h>
#include <stdlib.h>
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

void print_package_data(PackageData data) {
    int termw = get_terminal_width();
    printf("aur/%s %s\n", data.name, data.ver);

    // printf("\n");
    // remquotes(data.desc);
    pretty_print(4, data.desc);

    // printf("\n");
}