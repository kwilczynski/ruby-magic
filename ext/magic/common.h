#if !defined(_COMMON_H)
#define _COMMON_H 1

#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE 1
#endif

#if !defined(_BSD_SOURCE)
# define _BSD_SOURCE 1
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <magic.h>
#include <ruby.h>

#if defined(HAVE_LOCALE_H)
# include <locale.h>
#endif

#if defined(HAVE_XLOCALE_H)
# include <xlocale.h>
#endif

#if !defined(ENOMEM)
# define ENOMEM 12
#endif

#if !defined(EFAULT)
# define EFAULT 14
#endif

#if !defined(EINVAL)
# define EINVAL 22
#endif

#if !defined(ENOSYS)
# define ENOSYS 38
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

#if !defined(HAVE_MAGIC_VERSION) || MAGIC_VERSION < 518
# define HAVE_BROKEN_MAGIC 1
#endif

#if defined(HAVE_NEWLOCALE) && defined(HAVE_USELOCALE) && \
    defined(HAVE_FREELOCALE)
# define HAVE_SAFE_LOCALE 1
#endif

#if !defined(MAGIC_EXTENSION)
# define MAGIC_EXTENSION -1
# if defined(HAVE_WARNING)
#  warning "constant `MAGIC_EXTENSION' not implemented"
# else
#  pragma message("constant `MAGIC_EXTENSION' not implemented")
# endif
#endif

#if !defined(MAGIC_COMPRESS_TRANSP)
# define MAGIC_COMPRESS_TRANSP -1
# if defined(HAVE_WARNING)
#  warning "constant `MAGIC_COMPRESS_TRANSP' not implemented"
# else
#  pragma message("constant `MAGIC_COMPRESS_TRANSP' not implemented")
# endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _COMMON_H */
