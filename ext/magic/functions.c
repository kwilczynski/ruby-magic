#if defined(__cplusplus)
extern "C" {
#endif

#include "functions.h"

static int check_fd(int fd);
static int safe_dup(int fd);
static int safe_close(int fd);
static int safe_cloexec(int fd);
static int override_error_output(void *data);
static int restore_error_output(void *data);

static inline int
check_fd(int fd)
{
	errno = 0;
	if (fd < 0 || (fcntl(fd, F_GETFD) < 0 && errno == EBADF)) {
		errno = EBADF;
		return -EBADF;
	}

	return 0;
}

static int
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
			goto error;
		}
	}

	if (safe_cloexec(new_fd) < 0) {
		local_errno = errno;
		goto error;
	}

	return new_fd;
error:
	errno = local_errno;
	return -1;
}

static int
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

static inline int
safe_cloexec(int fd)
{
	int local_errno;
	int flags = fcntl(fd, F_GETFD);

	if (flags < 0) {
		local_errno = errno;
		goto error;
	}

	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
		local_errno = errno;
		goto error;
	}

	return 0;
error:
	errno = local_errno;
	return -1;
}

static int
override_error_output(void *data)
{
	int local_errno;
	int flags = O_WRONLY | O_APPEND;
	save_t *s = data;

#if defined(HAVE_O_CLOEXEC)
	flags |= O_CLOEXEC;
#endif

	assert(s != NULL &&
	       "Must be a valid pointer to `save_t' type");

	s->file.old_fd = -1;
	s->file.new_fd = -1;
	s->status = -1;

	fflush(stderr);
	fgetpos(stderr, &s->file.position);

	s->file.old_fd = safe_dup(fileno(stderr));
	if (s->file.old_fd < 0) {
		local_errno = errno;
		goto error;
	}

	s->file.new_fd = open("/dev/null", flags, 0222);
	if (s->file.new_fd < 0) {
		local_errno = errno;

		if (dup2(s->file.old_fd, fileno(stderr)) < 0) {
			local_errno = errno;
			goto error;
		}

		safe_close(s->file.old_fd);
		goto error;
	}

	if (safe_cloexec(s->file.new_fd) < 0) {
		local_errno = errno;
		goto error;
	}

	if (dup2(s->file.new_fd, fileno(stderr)) < 0) {
		local_errno = errno;
		goto error;
	}

	safe_close(s->file.new_fd);

	return 0;
error:
	s->status = local_errno;
	errno = s->status;
	return -1;
}

static int
restore_error_output(void *data)
{
	int local_errno;
	save_t *s = data;

	assert(s != NULL &&
	       "Must be a valid pointer to `save_t' type");

	if (s->file.old_fd < 0 && s->status != 0)
		return -1;

	fflush(stderr);

	if (dup2(s->file.old_fd, fileno(stderr)) < 0) {
		local_errno = errno;
		goto error;
	}

	safe_close(s->file.old_fd);
	clearerr(stderr);
	fsetpos(stderr, &s->file.position);

	if (setvbuf(stderr, NULL, _IONBF, 0) != 0) {
		local_errno = errno;
		goto error;
	}

	return 0;
error:
	s->status = local_errno;
	errno = s->status;
	return -1;
}

inline magic_t
magic_open_wrapper(int flags)
{
	return magic_open(flags);
}

inline void
magic_close_wrapper(magic_t magic)
{
	magic_close(magic);
}

inline const char*
magic_error_wrapper(magic_t magic)
{
	return magic_error(magic);
}

inline int
magic_errno_wrapper(magic_t magic)
{
	return magic_errno(magic);
}

inline const char*
magic_getpath_wrapper(void)
{
	/*
	 * The second argument translates to same value as the
	 * FILE_LOAD constant, which when used results in this
	 * function calling the get_default_magic() internally.
	 *
	 * N.B. magic_getpath() also honors the "MAGIC"
	 * environment variable."
	 */
	return magic_getpath(NULL, 0);
}

inline int
magic_getparam_wrapper(magic_t magic, int parameter, void *value)
{
	return magic_getparam(magic, parameter, value);
}

inline int
magic_setparam_wrapper(magic_t magic, int parameter, const void *value)
{
	if (*(const int *)value < 0 || *(const size_t *)value > UINT_MAX) {
		errno = EOVERFLOW;
		return -EOVERFLOW;
	}

	if (parameter == MAGIC_PARAM_BYTES_MAX)
		return magic_setparam(magic, parameter, value);

	if (*(const size_t *)value > USHRT_MAX) {
		errno = EOVERFLOW;
		return -EOVERFLOW;
	}

	return magic_setparam(magic, parameter, value);
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

#if !defined(HAVE_UTIME) && !defined(HAVE_UTIMES)
	if (flags & MAGIC_PRESERVE_ATIME) {
		errno = ENOSYS;
		return -ENOSYS;
	}
#endif

	return magic_setflags(magic, flags);
}

inline int
magic_load_wrapper(magic_t magic, const char *magic_file, int flags)
{
	int rv;

	MAGIC_FUNCTION(magic_load, rv, flags, magic, magic_file);

	return rv;
}

inline int
magic_load_buffers_wrapper(magic_t magic, void **buffers, size_t *sizes,
			   size_t count, int flags)
{
	int rv;

	MAGIC_FUNCTION(magic_load_buffers, rv, flags, magic, buffers, sizes,
		       count);

	return rv;
}

inline int
magic_compile_wrapper(magic_t magic, const char *magic_file, int flags)
{
	int rv;

	MAGIC_FUNCTION(magic_compile, rv, flags, magic, magic_file);

	return rv;
}

inline int
magic_check_wrapper(magic_t magic, const char *magic_file, int flags)
{
	int rv;

	MAGIC_FUNCTION(magic_check, rv, flags, magic, magic_file);

	return rv;
}

inline const char*
magic_file_wrapper(magic_t magic, const char *filename, int flags)
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
	int local_errno;
	const char *cstring;

	if (check_fd(fd) < 0) {
		local_errno = errno;
		goto error;
	}

	MAGIC_FUNCTION(magic_descriptor, cstring, flags, magic, fd);
	return cstring;

error:
	errno = local_errno;
	return NULL;
}

inline int
magic_version_wrapper(void)
{
	return magic_version();
}

#if defined(__cplusplus)
}
#endif
