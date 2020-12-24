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

#if !defined(T_INTEGER)
# define T_INTEGER rb_cInteger
#endif

#if !defined(ANYARGS)
# if defined(__cplusplus)
#  define ANYARGS ...
# else
#  define ANYARGS
# endif
#endif

#if !defined(RB_UNUSED_VAR)
# if defined(__GNUC__)
#  define RB_UNUSED_VAR(x) (x) __attribute__ ((unused))
# else
#  define RB_UNUSED_VAR(x) (x)
# endif
#endif

#if !defined(UNUSED)
# define UNUSED(x) (void)(x)
#endif

#if !defined(RUBY_METHOD_FUNC)
# define RUBY_METHOD_FUNC(f) ((VALUE (*)(ANYARGS))(f))
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


#if defined(MAGIC_VERSION) && MAGIC_VERSION >= 513
# define HAVE_MAGIC_VERSION 1
#endif

#if !defined(HAVE_MAGIC_VERSION) || MAGIC_VERSION < 518
# define HAVE_BROKEN_MAGIC 1
#endif

#if defined(HAVE_MAGIC_GETPARAM) && defined(HAVE_MAGIC_SETPARAM)
# define HAVE_MAGIC_PARAM 1
#endif

#if !defined(MAGIC_PARAM_INDIR_MAX)
# define MAGIC_PARAM_INDIR_MAX -1
#endif

#if !defined(MAGIC_PARAM_NAME_MAX)
# define MAGIC_PARAM_NAME_MAX -1
#endif

#if !defined(MAGIC_PARAM_ELF_PHNUM_MAX)
# define MAGIC_PARAM_ELF_PHNUM_MAX -1
#endif

#if !defined(MAGIC_PARAM_ELF_SHNUM_MAX)
# define MAGIC_PARAM_ELF_SHNUM_MAX -1
#endif

#if !defined(MAGIC_PARAM_ELF_NOTES_MAX)
# define MAGIC_PARAM_ELF_NOTES_MAX -1
#endif

#if !defined(MAGIC_PARAM_REGEX_MAX)
# define MAGIC_PARAM_REGEX_MAX -1
#endif

#if !defined(MAGIC_PARAM_BYTES_MAX)
# define MAGIC_PARAM_BYTES_MAX -1
#endif

#if !defined(MAGIC_EXTENSION)
# define MAGIC_EXTENSION -1
#endif

#if !defined(MAGIC_NO_CHECK_CSV)
# define MAGIC_NO_CHECK_CSV -1
#endif

#if !defined(MAGIC_NO_CHECK_JSON)
# define MAGIC_NO_CHECK_JSON -1
#endif

#if !defined(MAGIC_COMPRESS_TRANSP)
# define MAGIC_COMPRESS_TRANSP -1
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _COMMON_H */
