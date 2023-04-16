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

#ifndef RPC_H
#define RPC_H

#include "json.h"

typedef enum {
        rpc_search,
        rpc_multiinfo,
        rpc_error,
} rpc_result_t;

struct rpc_data {
        size_t resultcount;
        rpc_result_t type;
        char *error;
        struct json *results;
        struct json *raw_json;
};

struct package {
        char *name;
        char *desc;
        char *version;
        long outofdate;
};

struct rpc_data *make_rpc_request(char *url);
void free_rpc_data(struct rpc_data *data);
struct package *parse_package_json(struct json *j);
void free_package_data(struct package *p);

#endif  /* RPC_H */
