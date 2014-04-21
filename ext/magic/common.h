/* :enddoc: */

/*
 * common.h
 *
 * Copyright 2013-2014 Krzysztof Wilczynski
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
#include <locale.h>
#include <sys/stat.h>
#include <magic.h>
#include <ruby.h>

#if !defined(EPERM)
# define EPERM 1
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

#if defined(UNUSED)
# undef(UNUSED)
#endif

#define UNUSED(x) (void)(x)

#if !defined(HAVE_MAGIC_VERSION) || MAGIC_VERSION < 518
# define HAVE_BROKEN_MAGIC 1
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _COMMON_H */

/* vim: set ts=8 sw=4 sts=2 et : */
