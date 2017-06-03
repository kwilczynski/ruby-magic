#if defined(__cplusplus)
extern "C" {
#endif

#include "functions.h"

static inline int safe_cloexec(int fd);

int safe_dup(int fd);
int safe_close(int fd);

int suppress_error_output(void *data);
int restore_error_output(void *data);

int override_current_locale(void *data);
int restore_current_locale(void *data);

static inline int
safe_cloexec(int fd)
{
    int local_errno;
    int flags = fcntl(fd, F_GETFD);

    if (flags < 0) {
	local_errno = errno;
	goto out;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
	local_errno = errno;
	goto out;
    }

    return 0;

out:
    errno = local_errno;
    return -1;
}

int
safe_dup(int fd)
{
    int new_fd;
    int local_errno;
    int flags = F_DUPFD;

#if defined(HAVE_F_DUPFD_CLOEXEC)
    flags = F_DUPFD_CLOEXEC;
#endif

    new_fd = fcntl(fd, flags, fileno(stderr) + 1);
    if (new_fd < 0 && errno == EINVAL) {
	new_fd = dup(fd);
	if (new_fd < 0) {
	   local_errno = errno;
	   goto out;
	}
    }

    if (safe_cloexec(new_fd) < 0) {
	local_errno = errno;
	goto out;
    }

    return new_fd;

out:
    errno = local_errno;
    return -1;
}

int
safe_close(int fd)
{
    int rv;
#if defined(HAVE_POSIX_CLOSE_RESTART)
    rv = posix_close(fd, 0);
#else
    rv = close(fd);
    if (rv < 0 && errno == EINTR)
	errno = EINPROGRESS;
#endif

    return rv;
}

int
suppress_error_output(void *data)
{
    int local_errno;
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    save_t *s = data;

#if defined(HAVE_O_CLOEXEC)
    mode |= O_CLOEXEC;
#endif

    assert(s != NULL && \
	   "Must be a valid pointer to `save_t' type");

    s->data.file.old_fd = -1;
    s->data.file.new_fd = -1;
    s->status = -1;

    fflush(stderr);
    fgetpos(stderr, &s->data.file.position);

    s->data.file.old_fd = safe_dup(fileno(stderr));
    if (s->data.file.old_fd < 0) {
	local_errno = errno;
	goto out;
    }

    s->data.file.new_fd = open("/dev/null", O_WRONLY | O_APPEND, mode);
    if (s->data.file.new_fd < 0) {
	local_errno = errno;

	if (dup2(s->data.file.old_fd, fileno(stderr)) < 0) {
	    local_errno = errno;
	    goto out;
	}

	safe_close(s->data.file.old_fd);
	goto out;
    }

    if (safe_cloexec(s->data.file.new_fd) < 0) {
	local_errno = errno;
	goto out;
    }

    if (dup2(s->data.file.new_fd, fileno(stderr)) < 0) {
	local_errno = errno;
	goto out;
    }

    safe_close(s->data.file.new_fd);
    return 0;

out:
    s->status = local_errno;
    errno = s->status;

    return -1;
}

int
restore_error_output(void *data)
{
    int local_errno;
    save_t *s = data;

    assert(s != NULL && \
	   "Must be a valid pointer to `save_t' type");

    if (s->data.file.old_fd < 0 && s->status != 0)
	return -1;

    fflush(stderr);

    if (dup2(s->data.file.old_fd, fileno(stderr)) < 0) {
	local_errno = errno;
	goto out;
    }

    safe_close(s->data.file.old_fd);
    clearerr(stderr);
    fsetpos(stderr, &s->data.file.position);

    if (setvbuf(stderr, NULL, _IONBF, 0) != 0) {
	local_errno = errno;
	goto out;
    }

    return 0;

out:
    s->status = local_errno;
    errno = s->status;

    return -1;
}

int
override_current_locale(void *data)
{
#if !defined(HAVE_SAFE_LOCALE)
    char *current_locale = NULL;
#endif
    save_t *s = data;

    assert(s != NULL && \
	   "Must be a valid pointer to `save_t' type");

    s->status = -1;

#if defined(HAVE_SAFE_LOCALE)
    s->data.locale.old_locale = NULL;
    s->data.locale.new_locale = NULL;

    s->data.locale.new_locale = newlocale(LC_ALL_MASK, "C", NULL);
    if (s->data.locale.new_locale == (locale_t)0)
	goto out;

    assert(s->data.locale.new_locale != NULL && \
	   "Must be a valid pointer to `locale_t' type");

    s->data.locale.old_locale = uselocale(s->data.locale.new_locale);
    if (s->data.locale.old_locale == (locale_t)0)
	goto out;

    assert(s->data.locale.old_locale != NULL && \
	   "Must be a valid pointer to `locale_t' type");
#else
    s->data.locale.old_locale = NULL;

    current_locale = setlocale(LC_ALL, NULL);
    if (!current_locale)
	goto out;

    s->data.locale.old_locale = strndup(current_locale,
					strlen(current_locale));
    if (!s->data.locale.old_locale)
	goto out;

    if (!setlocale(LC_ALL, "C"))
	goto out;

    assert(s->data.locale.old_locale != NULL && \
	   "Must be a valid pointer to `char' type");
#endif
    s->status = 0;

out:
    return s->status;
}

int
restore_current_locale(void *data)
{
    save_t *s = data;

    assert(s != NULL && \
	   "Must be a valid pointer to `save_t' type");

#if defined(HAVE_SAFE_LOCALE)
    if (!(s->data.locale.new_locale || \
	s->data.locale.old_locale) && \
	s->status != 0)
	return -1;

    if (uselocale(s->data.locale.old_locale) == (locale_t)0)
	goto out;

    assert(s->data.locale.new_locale != NULL && \
	   "Must be a valid pointer to `locale_t' type");

    freelocale(s->data.locale.new_locale);
#else
    if (!s->data.locale.old_locale && s->status != 0)
	return -1;

    if (!setlocale(LC_ALL, s->data.locale.old_locale))
	goto out;

    assert(s->data.locale.old_locale != NULL && \
	   "Must be a valid pointer to `char' type");

    xfree(s->data.locale.old_locale);
#endif

    return 0;

out:
    s->status = -1;
#if defined(HAVE_SAFE_LOCALE)
    s->data.locale.old_locale = NULL;
    s->data.locale.new_locale = NULL;
#else
    s->data.locale.old_locale = NULL;
#endif

    return -1;
}

inline const char*
magic_getpath_wrapper(void)
{
    return magic_getpath(NULL, 0);
}

inline int
magic_getparam_wrapper(magic_t magic, int parameter, void *value)
{
#if defined(HAVE_MAGIC_PARAM)
    return magic_getparam(magic, parameter, value);
#else
    UNUSED(magic);
    UNUSED(parameter);
    UNUSED(value);

    errno = ENOSYS;
    return -ENOSYS;
#endif
}

inline int
magic_setparam_wrapper(magic_t magic, int parameter, const void *value)
{
#if defined(HAVE_MAGIC_PARAM)
    if (parameter == MAGIC_PARAM_BYTES_MAX)
	return magic_setparam(magic, parameter, value);

    if (*(const size_t *)value > USHRT_MAX) {
	errno = EOVERFLOW;
	return -EOVERFLOW;
    }

    return magic_setparam(magic, parameter, value);
#else
    UNUSED(magic);
    UNUSED(parameter);
    UNUSED(value);

    errno = ENOSYS;
    return -ENOSYS;
#endif
}

inline int
magic_getflags_wrapper(magic_t magic)
{
#if defined(HAVE_MAGIC_GETFLAGS)
    return magic_getflags(magic);
#else
    UNUSED(magic);
    errno = ENOSYS;
    return -ENOSYS;
#endif
}

inline int
magic_setflags_wrapper(magic_t magic, int flags)
{
    if (flags < 0 || flags > 0xfffffff) {
	errno = EINVAL;
	return -EINVAL;
    }

#if !(defined(HAVE_UTIME) || defined(HAVE_UTIMES))
    if (flags & MAGIC_PRESERVE_ATIME) {
	errno = ENOSYS;
	return -ENOSYS;
    }
#endif

    return magic_setflags(magic, flags);
}

inline int
magic_load_wrapper(magic_t magic, const char *magicfile, int flags)
{
    int rv;
    MAGIC_FUNCTION(magic_load, rv, flags, magic, magicfile);
    return rv;
}

inline int
magic_compile_wrapper(magic_t magic, const char *magicfile, int flags)
{
    int rv;
    MAGIC_FUNCTION(magic_compile, rv, flags, magic, magicfile);
    return rv;
}

inline int
magic_check_wrapper(magic_t magic, const char *magicfile, int flags)
{
    int rv;
    MAGIC_FUNCTION(magic_check, rv, flags, magic, magicfile);
    return rv;
}

inline const char*
magic_file_wrapper(magic_t magic, const char* filename, int flags)
{
    const char *cstring;
    MAGIC_FUNCTION(magic_file, cstring, flags, magic, filename);
    return cstring;
}

inline const char*
magic_buffer_wrapper(magic_t magic, const void *buffer, size_t size, int flags)
{
    const char *cstring;
    MAGIC_FUNCTION(magic_buffer, cstring, flags, magic, buffer, size);
    return cstring;
}

inline const char*
magic_descriptor_wrapper(magic_t magic, int fd, int flags)
{
    const char *cstring;
    MAGIC_FUNCTION(magic_descriptor, cstring, flags, magic, fd);
    return cstring;
}

inline int
magic_version_wrapper(void)
{
#if defined(HAVE_MAGIC_VERSION)
    return magic_version();
#else
    errno = ENOSYS;
    return -ENOSYS;
#endif
}

#if defined(__cplusplus)
}
#endif
