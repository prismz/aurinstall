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

#ifndef INSTALL_H
#define INSTALL_H

#include "json.h"

#include <stdbool.h>

int import_pgp_keys(char *path);
int install_non_aur_package(char *name);
int install_package(char *name, char *cache_path);
HashMap *get_installed_packages(void);
int update_packages(char *cache_path);
int clean_cache(char *cache_path);
int remove_packages(int n, char **pkgs);

#endif  /* INSTALL_H */
