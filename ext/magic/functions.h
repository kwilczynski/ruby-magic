/* :enddoc: */

/*
 * functions.h
 *
 * Copyright 2013 Krzysztof Wilczynski
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !defined(_FUNCTIONS_H)
#define _FUNCTIONS_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"

#define SUPPRESS_LOCALE(f, r, ...)                    \
    do {                                              \
        int __##f;                                    \
        save_t __l_##f;                               \
        __##f = override_current_locale(&(__l_##f));  \
        r = f(__VA_ARGS__);                           \
        if (!(__##f)) {                               \
            restore_current_locale(&(__l_##f));       \
        }                                             \
    } while(0)

#define SUPPRESS_EVERYTHING(f, r, ...)                \
    do {                                              \
        int _l_##f, _e_##f;                           \
        save_t __l_##f, __e_##f;                      \
        _l_##f = override_current_locale(&(__l_##f)); \
        _e_##f = suppress_error_output(&(__e_##f));   \
        r = f(__VA_ARGS__);                           \
        if (!(_l_##f)) {                              \
            restore_current_locale(&(__l_##f));       \
        }                                             \
        if (!(_e_##f))  {                             \
            restore_error_output(&(__e_##f));         \
        }                                             \
    } while(0)

#define MAGIC_FUNCTION(f, r, x, ...)                \
     do {                                           \
        if ((x) & MAGIC_ERROR) {                    \
            SUPPRESS_LOCALE(f, r, __VA_ARGS__);     \
        }                                           \
        else {                                      \
            SUPPRESS_EVERYTHING(f, r, __VA_ARGS__); \
         }                                          \
     } while(0)

struct file_data {
    int old_fd;
    int new_fd;
    fpos_t position;
};

typedef struct file_data file_data_t;

struct save {
    int status;
    union {
        file_data_t file;
        char *locale;
    } data;
};

typedef struct save save_t;

extern const char* magic_getpath_wrapper(void);

extern int magic_setflags_wrapper(struct magic_set *ms, int flags);
extern int magic_load_wrapper(struct magic_set *ms, const char *magicfile, int flags);
extern int magic_compile_wrapper(struct magic_set *ms, const char *magicfile, int flags);
extern int magic_check_wrapper(struct magic_set *ms, const char *magicfile, int flags);

extern const char* magic_file_wrapper(struct magic_set *ms, const char *filename, int flags);
extern const char* magic_buffer_wrapper(struct magic_set *ms, const char *buffer, size_t size, int flags);
extern const char* magic_descriptor_wrapper(struct magic_set *ms, int fd, int flags);

extern int magic_version_wrapper(void);

#if defined(__cplusplus)
}
#endif

#endif /* _FUNCTIONS_H */

/* vim: set ts=8 sw=4 sts=2 et : */
