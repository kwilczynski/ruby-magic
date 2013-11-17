/* :enddoc: */

/*
 * ruby-magic.h
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

#if !defined(_RUBY_MAGIC_H)
#define _RUBY_MAGIC_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"
#include "functions.h"

#define DATA_P(x)   (TYPE(x) == T_DATA)
#define STRING_P(x) (TYPE(x) == T_STRING)
#define ARRAY_P(x)  (TYPE(x) == T_ARRAY)

#if !defined(STR2CSTR)
# define STR2CSTR(x) StringValuePtr(x)
#endif

#if !defined(RVAL2CSTR)
# define RVAL2CSTR(x) (NIL_P(x) ? NULL : STR2CSTR(x))
#endif

#if !defined(CSTR2RVAL)
# define CSTR2RVAL(x) ((x) == NULL ? Qnil : rb_str_new2(x))
#endif

#if !defined(RARRAY_LEN)
# define RARRAY_LEN(a) (RARRAY(a)->len)
#endif

#if !defined(RSTRING_LEN)
# define RSTRING_LEN(s) (RSTRING(s)->len)
#endif

#if !defined(RSTRING_PTR)
# define RSTRING_PTR(s) (RSTRING(s)->ptr)
#endif

#define RSTRING_EMPTY_P(s) (RSTRING_LEN(s) == 0)
#define RARRAY_EMPTY_P(a)  (RARRAY_LEN(a) == 0)

#define NOGVL_FUNCTION (VALUE (*)(void *))

#if defined(HAVE_RB_THREAD_CALL_WITHOUT_GVL) && \
     defined(HAVE_RUBY_THREAD_H) && HAVE_RUBY_THREAD_H
# include <ruby/thread.h>
# define NOGVL(f, d) \
   rb_thread_call_without_gvl((f), (d), RUBY_UBF_IO, NULL)
#elif defined(HAVE_RB_THREAD_BLOCKING_REGION)
# define NOGVL(f, d) \
   rb_thread_blocking_region(NOGVL_FUNCTION(f), (d), RUBY_UBF_IO, NULL)
#else
# include <rubysig.h>
static inline VALUE
fake_blocking_region(VALUE (*f)(ANYARGS), void *data)
{
  VALUE rv;

  TRAP_BEG;
  rv = f(data);
  TRAP_END;

  return rv;
}
# define NOGVL(f, d) fake_blocking_region(NOGVL_FUNCTION(f), (d))
#endif

#define MAGIC_SYNCHRONIZED(f, d) magic_lock(object, (f), (d))

#define MAGIC_COOKIE(c) \
    Data_Get_Struct(object, struct magic_set, (c))

#define MAGIC_CLOSED_P(o) RTEST(rb_mgc_closed((o)))

#define MAGIC_GENERIC_ERROR(k, e, m)                        \
    do {                                                    \
        VALUE __e_##k = magic_generic_error((k), (e), (m)); \
        rb_exc_raise(__e_##k);                              \
    } while(0)

#define MAGIC_LIBRARY_ERROR(c)                                              \
    do {                                                                    \
        VALUE __e_library = magic_library_error(rb_mgc_eMagicError, (c));   \
        rb_exc_raise(__e_library);                                          \
    } while(0)

#define CHECK_MAGIC_OPEN(o)                                         \
    do {                                                            \
        if (MAGIC_CLOSED_P(o)) {                                    \
            MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError, EFAULT,       \
                                    error(E_MAGIC_LIBRARY_CLOSED)); \
        }                                                           \
    } while(0)                                                      \

#define error(t) errors[(t)]

enum error {
    E_UNKNOWN = 0,
    E_NOT_IMPLEMENTED,
    E_MAGIC_LIBRARY_INITIALIZE,
    E_MAGIC_LIBRARY_CLOSED,
    E_FLAG_INVALID_VALUE,
    E_FLAG_NOT_IMPLEMENTED
};

struct magic_exception {
    int magic_errno;
    const char *magic_error;
    VALUE klass;
};

struct magic_arguments {
    int flags;
    magic_t cookie;
    union {
        int fd;
        const char *path;
    } file;
};

typedef struct magic_exception magic_exception_t;
typedef struct magic_arguments magic_arguments_t;

static const char *errors[] = {
    "unknown error",
    "function is not implemented",
    "failed to initialize Magic library",
    "Magic library is not open",
    "unknown or invalid flag specified",
    "flag is not implemented",
    NULL
};

inline static VALUE
magic_split(VALUE a, VALUE b)
{
    if (STRING_P(a) && STRING_P(b)) {
        return rb_funcall(a, rb_intern("split"), 1, b);
    }

    return Qnil;
}

inline static VALUE
magic_join(VALUE a, VALUE b)
{
    if (ARRAY_P(a) && STRING_P(b)) {
        return rb_funcall(a, rb_intern("join"), 1, b);
    }

    return Qnil;
}

RUBY_EXTERN ID id_at_flags, id_at_path, id_at_mutex;

RUBY_EXTERN VALUE rb_cMagic;

RUBY_EXTERN VALUE rb_mgc_eError;
RUBY_EXTERN VALUE rb_mgc_eMagicError;
RUBY_EXTERN VALUE rb_mgc_eLibraryError;
RUBY_EXTERN VALUE rb_mgc_eFlagsError;
RUBY_EXTERN VALUE rb_mgc_eNotImplementedError;

RUBY_EXTERN VALUE rb_mgc_initialize(VALUE object);

RUBY_EXTERN VALUE rb_mgc_close(VALUE object);
RUBY_EXTERN VALUE rb_mgc_closed(VALUE object);

RUBY_EXTERN VALUE rb_mgc_get_path(VALUE object);

RUBY_EXTERN VALUE rb_mgc_get_flags(VALUE object);
RUBY_EXTERN VALUE rb_mgc_set_flags(VALUE object, VALUE value);

RUBY_EXTERN VALUE rb_mgc_load(VALUE object, VALUE arguments);
RUBY_EXTERN VALUE rb_mgc_compile(VALUE object, VALUE arguments);
RUBY_EXTERN VALUE rb_mgc_check(VALUE object, VALUE arguments);

RUBY_EXTERN VALUE rb_mgc_file(VALUE object, VALUE value);
RUBY_EXTERN VALUE rb_mgc_buffer(VALUE object, VALUE value);
RUBY_EXTERN VALUE rb_mgc_descriptor(VALUE object, VALUE value);

RUBY_EXTERN VALUE rb_mgc_version(VALUE object);

#if defined(__cplusplus)
}
#endif

#endif /* _RUBY_MAGIC_H */

/* vim: set ts=8 sw=4 sts=2 et : */
