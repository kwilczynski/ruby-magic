#if !defined(_FUNCTIONS_H)
#define _FUNCTIONS_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"

#define MAGIC_FUNCTION(f, r, x, ...)			 \
	do {						 \
		if ((x) & (MAGIC_DEBUG | MAGIC_CHECK))   \
			r = f(__VA_ARGS__);		 \
		else {					 \
			save_t __##f;			 \
			override_error_output(&(__##f)); \
			r = f(__VA_ARGS__);		 \
			restore_error_output(&(__##f));  \
		}					 \
	} while (0)

typedef struct file_data {
	fpos_t position;
	int old_fd;
	int new_fd;
} file_data_t;

typedef struct save {
	file_data_t file;
	int status;
} save_t;

extern magic_t magic_open_wrapper(int flags);
extern void magic_close_wrapper(magic_t magic);

extern const char *magic_error_wrapper(magic_t magic);
extern int magic_errno_wrapper(magic_t magic);

extern const char *magic_getpath_wrapper(void);

extern int magic_getparam_wrapper(magic_t magic, int parameter,
				  void *value);
extern int magic_setparam_wrapper(magic_t magic, int parameter,
				  const void *value);

extern int magic_getflags_wrapper(magic_t magic);
extern int magic_setflags_wrapper(magic_t magic, int flags);

extern int magic_load_wrapper(magic_t magic, const char *magic_file,
			      int flags);
extern int magic_load_buffers_wrapper(magic_t magic, void **buffers,
				      size_t *sizes, size_t count,
				      int flags);

extern int magic_compile_wrapper(magic_t magic, const char *magic_file,
				 int flags);
extern int magic_check_wrapper(magic_t magic, const char *magic_file,
			       int flags);

extern const char *magic_file_wrapper(magic_t magic, const char *filename,
				      int flags);
extern const char *magic_buffer_wrapper(magic_t magic, const void *buffer,
					size_t size, int flags);
extern const char *magic_descriptor_wrapper(magic_t magic, int fd,
					    int flags);

extern int magic_version_wrapper(void);

#if defined(__cplusplus)
}
#endif

#endif /* _FUNCTIONS_H */
