#pragma once

#ifndef SEARCH_MAX_ARG_COUNT
#define SEARCH_MAX_ARG_COUNT 256
#endif

#ifndef SEARCH_MAX_ARG_LEN
#define SEARCH_MAX_ARG_LEN 256
#endif

static char* cache_path;
static int normal_tty;

#ifndef GIT_UNKNOWN_IDENTITY_ERR
#define GIT_UNKNOWN_INDENTITY_ERR 32768
#endif

#ifndef BOLD
#define BOLD "\033[1m"
#endif

#ifndef GREEN
#define GREEN "\033[92m"
#endif

#ifndef RED
#define RED "\033[91m"
#endif

#ifndef ENDC
#define ENDC "\033[0m"
#endif
