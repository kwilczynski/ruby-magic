/* :enddoc: */

/*
 * functions.c
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

#include "functions.h"

static int suppress_error_output(void *data);
static void restore_error_output(void *data);

static int
suppress_error_output(void *data)
{
    save_t *s = data;
    assert(s != NULL && "Must be a valid pointer to `save_t' type");

    fflush(stderr);
    fgetpos(stderr, &s->position);

    s->status = 0;

    s->old_fd = dup(fileno(stderr));
    s->new_fd = open("/dev/null", O_WRONLY);
    if (s->new_fd < 0) {
        dup2(s->old_fd, fileno(stderr));
        close(s->old_fd);

        s->old_fd = -1;
        s->new_fd = s->old_fd;
        s->status = errno;

        return -1;
    }

    dup2(s->new_fd, fileno(stderr));
    close(s->new_fd);

    return 0;
}

static void
restore_error_output(void *data)
{
    save_t *s = data;
    assert(s != NULL && "Must be a valid pointer to `save_t' type");

    if (s->old_fd < 0 && s->status != 0) {
        return;
    }

    fflush(stderr);
    dup2(s->old_fd, fileno(stderr));
    close(s->old_fd);
    clearerr(stderr);
    fsetpos(stderr, &s->position);
    setvbuf(stderr, NULL, _IONBF, 0);
}

inline const char*
magic_getpath_wrapper(void)
{
    return magic_getpath(NULL, 0);
}

inline int
magic_setflags_wrapper(struct magic_set *ms, int flags) {
    if (flags < MAGIC_NONE || flags > MAGIC_NO_CHECK_BUILTIN) {
        errno = EINVAL;
        return -EINVAL;
    }

#if !defined(HAVE_UTIME) && !defined(HAVE_UTIMES)
    if (flags & MAGIC_PRESERVE_ATIME) {
        errno = ENOSYS;
        return -ENOSYS;
    }
#endif

    return magic_setflags(ms, flags);
}

inline int
magic_load_wrapper(struct magic_set *ms, const char *magicfile, int flags)
{
    int rv;

    if (flags & MAGIC_DEBUG) {
        return magic_load(ms, magicfile);
    }

    SUPPRESS_ERROR_OUTPUT(magic_load, rv, ms, magicfile);

    return rv;
}

inline int
magic_compile_wrapper(struct magic_set *ms, const char *magicfile, int flags)
{
    int rv;

    if (flags & MAGIC_DEBUG) {
        return magic_compile(ms, magicfile);
    }

    SUPPRESS_ERROR_OUTPUT(magic_compile, rv, ms, magicfile);

    return rv;
}

inline int
magic_check_wrapper(struct magic_set *ms, const char *magicfile, int flags)
{
    int rv;

    if (flags & MAGIC_DEBUG) {
        return magic_check(ms, magicfile);
    }

    SUPPRESS_ERROR_OUTPUT(magic_check, rv, ms, magicfile);

    return rv;
}

inline int
magic_version_wrapper(void)
{
#if defined(HAVE_MAGIC_VERSION)
    return magic_version();
#else
# if defined(HAVE_WARNING)
#  warning "function `int magic_version(void)' not implemented"
# else
#  pragma message("function `int magic_version(void)' not implemented")
# endif
    errno = ENOSYS;
    return -ENOSYS;
#endif
}

/* vim: set ts=8 sw=4 sts=2 et : */
