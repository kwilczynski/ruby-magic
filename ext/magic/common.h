#if !defined(_COMMON_H)
#define _COMMON_H 1

#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE 1
#endif

#if !defined(_BSD_SOURCE)
# define _BSD_SOURCE 1
#endif

#if !defined(_DEFAULT_SOURCE)
# define _DEFAULT_SOURCE 1
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <magic.h>
#include <ruby.h>

#if defined(HAVE_RUBY_IO_H)
# include <ruby/io.h>
#else
# include <rubyio.h>
#endif

#if !defined(BIT)
# define BIT(n) (1 << (n))
#endif

#if !defined(UNUSED)
# define UNUSED(x) (void)(x)
#endif

#if defined(F_DUPFD_CLOEXEC)
# define HAVE_F_DUPFD_CLOEXEC 1
#endif

#if defined(O_CLOEXEC)
# define HAVE_O_CLOEXEC 1
#endif

#if defined(POSIX_CLOSE_RESTART)
# define HAVE_POSIX_CLOSE_RESTART 1
#endif

#define MAGIC_EXTENSION_SEPARATOR "/"
#define MAGIC_CONTINUE_SEPARATOR "\n- "

#if !defined(MAGIC_NO_CHECK_CSV)
# define MAGIC_NO_CHECK_CSV 0
#endif

#if !defined(MAGIC_NO_CHECK_JSON)
# define MAGIC_NO_CHECK_JSON 0
#endif

#define DATA_P(x)    (RB_TYPE_P((x), T_DATA))
#define BOOLEAN_P(x) (RB_TYPE_P((x), T_TRUE) || RB_TYPE_P((x), T_FALSE))
#define STRING_P(x)  (RB_TYPE_P((x), T_STRING))
#define ARRAY_P(x)   (RB_TYPE_P((x), T_ARRAY))
#define FILE_P(x)    (RB_TYPE_P((x), T_FILE))

#define RVAL2CBOOL(x) (RTEST(x))
#define CBOOL2RVAL(x) ((x) ? Qtrue : Qfalse)

#define RVAL2CSTR(x) (NIL_P(x) ? NULL : StringValueCStr(x))
#define CSTR2RVAL(x) ((x) == NULL ? Qnil : rb_str_new2(x))

#define RARRAY_EMPTY	  rb_ary_new()
#define RARRAY_EMPTY_P(a) (RARRAY_LEN(a) == 0)
#define RARRAY_FIRST(a)   (RARRAY_EMPTY_P(a) ? Qnil : rb_ary_entry((a), 0))

#define CLASS_NAME(o) (NIL_P((o)) ? "nil" : rb_obj_classname((o)))

#if !defined(T_INTEGER)
# define T_INTEGER rb_cInteger
#endif

#if !defined(HAVE_RB_IO_T)
# define rb_io_t OpenFile
#endif

#if !defined(GetReadFile)
# define FPTR_TO_FD(p) ((p)->fd)
#else
# define FPTR_TO_FD(p) (fileno(GetReadFile(p)))
#endif

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
# define NOGVL(f, d) \
	fake_blocking_region(NOGVL_FUNCTION(f), (d))
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _COMMON_H */
