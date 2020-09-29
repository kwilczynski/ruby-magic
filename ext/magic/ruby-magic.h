#if !defined(_RUBY_MAGIC_H)
#define _RUBY_MAGIC_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"
#include "functions.h"

#define DATA_P(x)    (RB_TYPE_P((x), T_DATA))
#define BOOLEAN_P(x) (RB_TYPE_P((x), T_TRUE) || RB_TYPE_P((x), T_FALSE))
#define STRING_P(x)  (RB_TYPE_P((x), T_STRING))
#define ARRAY_P(x)   (RB_TYPE_P((x), T_ARRAY))
#define FILE_P(x)    (RB_TYPE_P((x), T_FILE))

#if !defined(RVAL2CBOOL)
# define RVAL2CBOOL(x) (RTEST(x))
#endif

#if !defined(CBOOL2RVAL)
# define CBOOL2RVAL(x) ((x) ? Qtrue : Qfalse)
#endif

#if !defined(STR2CSTR)
# define STR2CSTR(x) StringValueCStr(x)
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

#if !defined(NUM2SIZET)
# if SIZEOF_SIZE_T == SIZEOF_LONG
#  define NUM2SIZET(n) ((size_t)NUM2ULONG(n))
#  define SIZET2NUM(n) ((size_t)ULONG2NUM(n))
# else
#  define NUM2SIZET(n) ((size_t)NUM2ULL(n))
#  define SIZET2NUM(n) ((size_t)ULL2NUM(n))
# endif
#endif

#define RSTRING_EMPTY_P(s) (RSTRING_LEN(s) == 0)
#define RARRAY_EMPTY_P(a)  (RARRAY_LEN(a) == 0)
#define RARRAY_FIRST(a)    (RARRAY_EMPTY_P(a) ? Qnil : rb_ary_entry((a), 0))

#define CLASS_NAME(o) (NIL_P((o)) ? "nil" : rb_obj_classname((o)))

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

#define MAGIC_SYNCHRONIZED(f, d) magic_lock(object, (f), (d))

#define MAGIC_OBJECT(o) \
	Data_Get_Struct(object, magic_object_t, (o))

#define MAGIC_COOKIE(o, c) \
	((c) = MAGIC_OBJECT((o))->cookie)

#define MAGIC_CLOSED_P(o) RTEST(rb_mgc_close_p((o)))

#define MAGIC_ARGUMENT_TYPE_ERROR(o, ...) \
	rb_raise(rb_eTypeError, error(E_ARGUMENT_TYPE_INVALID), CLASS_NAME((o)), __VA_ARGS__)

#define MAGIC_GENERIC_ERROR(k, e, m) \
	rb_exc_raise(magic_generic_error((k), (e), error(m)))

#define MAGIC_LIBRARY_ERROR(c) \
	rb_exc_raise(magic_library_error(rb_mgc_eMagicError, (c)))

#define MAGIC_CHECK_INTEGER_TYPE(o) magic_check_type((o), T_FIXNUM)
#define MAGIC_CHECK_STRING_TYPE(o)  magic_check_type((o), T_STRING)

#define MAGIC_CHECK_ARGUMENT_MISSING(t, o)				 \
    do {								 \
	if ((t) < (o))							 \
	    rb_raise(rb_eArgError, error(E_ARGUMENT_MISSING), (t), (o)); \
    } while(0)

#define MAGIC_CHECK_ARRAY_EMPTY(o)					      \
    do {								      \
	if (RARRAY_EMPTY_P(o))						      \
	    rb_raise(rb_eArgError, "%s", error(E_ARGUMENT_TYPE_ARRAY_EMPTY)); \
    } while(0)

#define MAGIC_CHECK_ARRAY_OF_STRINGS(o) \
	magic_check_type_array_of_strings((o))

#define MAGIC_CHECK_OPEN(o)					\
    do {							\
	if (MAGIC_CLOSED_P(o))					\
	    MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError, EFAULT,	\
				E_MAGIC_LIBRARY_CLOSED);	\
    } while(0)

#define MAGIC_STRINGIFY(s) #s

#define MAGIC_DEFINE_FLAG(c) \
	rb_define_const(rb_cMagic, MAGIC_STRINGIFY(c), INT2NUM(MAGIC_##c));

#define MAGIC_DEFINE_PARAMETER(c) \
	rb_define_const(rb_cMagic, MAGIC_STRINGIFY(PARAM_##c), INT2NUM(MAGIC_PARAM_##c));

#define error(t) errors[(t)]

enum error {
    E_UNKNOWN = 0,
    E_NOT_ENOUGH_MEMORY,
    E_ARGUMENT_MISSING,
    E_ARGUMENT_TYPE_INVALID,
    E_ARGUMENT_TYPE_ARRAY_EMPTY,
    E_ARGUMENT_TYPE_ARRAY_STRINGS,
    E_NOT_IMPLEMENTED,
    E_MAGIC_LIBRARY_INITIALIZE,
    E_MAGIC_LIBRARY_CLOSED,
    E_PARAM_INVALID_TYPE,
    E_PARAM_INVALID_VALUE,
    E_FLAG_NOT_IMPLEMENTED,
    E_FLAG_INVALID_VALUE
};

typedef struct parameter {
    int tag;
    size_t value;
} parameter_t;

typedef union file {
    int fd;
    const char *path;
} file_t;

typedef struct buffers {
    size_t count;
    size_t *sizes;
    void **buffers;
} buffers_t;

typedef struct magic_object {
    magic_t cookie;
    VALUE mutex;
    unsigned int database_loaded:1;
    unsigned int stop_on_errors:1;
} magic_object_t;

typedef struct magic_arguments {
    int flags;
    magic_t cookie;
    union {
	file_t file;
	parameter_t parameter;
	buffers_t buffers;
    } type;
    int status;
    const char *result;
} magic_arguments_t;

typedef struct magic_exception {
    int magic_errno;
    const char *magic_error;
    VALUE klass;
} magic_exception_t;

static const char *errors[] = {
    [E_UNKNOWN]			    = "an unknown error has occurred",
    [E_NOT_ENOUGH_MEMORY]	    = "cannot allocate memory",
    [E_ARGUMENT_MISSING]	    = "wrong number of arguments (given %d, expected %d)",
    [E_ARGUMENT_TYPE_INVALID]	    = "wrong argument type %s (expected %s)",
    [E_ARGUMENT_TYPE_ARRAY_EMPTY]   = "arguments list cannot be empty (expected array of String)",
    [E_ARGUMENT_TYPE_ARRAY_STRINGS] = "wrong argument type %s in arguments list (expected String)",
    [E_NOT_IMPLEMENTED]		    = "function is not implemented",
    [E_MAGIC_LIBRARY_INITIALIZE]    = "failed to initialize Magic library",
    [E_MAGIC_LIBRARY_CLOSED]	    = "Magic library is not open",
    [E_PARAM_INVALID_TYPE]	    = "unknown or invalid parameter specified",
    [E_PARAM_INVALID_VALUE]	    = "invalid parameter value specified",
    [E_FLAG_NOT_IMPLEMENTED]	    = "flag is not implemented",
    [E_FLAG_INVALID_VALUE]	    = "unknown or invalid flag specified",
    NULL
};

static VALUE
magic_size(VALUE v)
{
    return (ARRAY_P(v) || STRING_P(v)) ?		 \
	   rb_funcall(v, rb_intern("size"), 0, Qundef) : \
	   Qnil;
}

static VALUE
magic_shift(VALUE v)
{
    return ARRAY_P(v) ?					  \
	   rb_funcall(v, rb_intern("shift"), 0, Qundef) : \
	   Qnil;
}

static VALUE
magic_split(VALUE a, VALUE b)
{
    return (STRING_P(a) && STRING_P(b)) ?	     \
	   rb_funcall(a, rb_intern("split"), 1, b) : \
	   Qnil;
}

static VALUE
magic_join(VALUE a, VALUE b)
{
    return (ARRAY_P(a) && STRING_P(b)) ?	    \
	   rb_funcall(a, rb_intern("join"), 1, b) : \
	   Qnil;
}

static VALUE
magic_flatten(VALUE v)
{
    return ARRAY_P(v) ?					    \
	   rb_funcall(v, rb_intern("flatten"), 0, Qundef) : \
	   Qnil;
}

static int
magic_fileno(VALUE object)
{
    int fd;
    rb_io_t *io;

    if (!FILE_P(object))
	object = rb_convert_type(object, T_FILE, "IO", "to_io");

    GetOpenFile(object, io);
    if ((fd = FPTR_TO_FD(io)) < 0)
	rb_raise(rb_eIOError, "closed stream");

    return fd;
}

static void
magic_check_type(VALUE object, int type)
{
    if (type == T_FIXNUM) {
	if (!RVAL2CBOOL(rb_obj_is_kind_of(object, T_INTEGER)))
	    MAGIC_ARGUMENT_TYPE_ERROR(object, rb_class2name(T_INTEGER));
    }

    Check_Type(object, type);
}

static void
magic_check_type_array_of_strings(VALUE object)
{
    VALUE value = Qundef;

    for (int i = 0; i < RARRAY_LEN(object); i++) {
	value = RARRAY_AREF(object, (long)i);

	if (NIL_P(value) || !STRING_P(value))
	    rb_raise(rb_eTypeError,
		     error(E_ARGUMENT_TYPE_ARRAY_STRINGS),
		     CLASS_NAME(value));
    }
}

RUBY_EXTERN int rb_mgc_do_not_auto_load;
RUBY_EXTERN int rb_mgc_do_not_stop_on_error;

RUBY_EXTERN ID id_to_io;
RUBY_EXTERN ID id_to_path;

RUBY_EXTERN ID id_at_flags;
RUBY_EXTERN ID id_at_paths;

RUBY_EXTERN VALUE rb_cMagic;

RUBY_EXTERN VALUE rb_mgc_eError;
RUBY_EXTERN VALUE rb_mgc_eMagicError;
RUBY_EXTERN VALUE rb_mgc_eLibraryError;
RUBY_EXTERN VALUE rb_mgc_eParameterError;
RUBY_EXTERN VALUE rb_mgc_eFlagsError;
RUBY_EXTERN VALUE rb_mgc_eNotImplementedError;

RUBY_EXTERN VALUE rb_mgc_get_do_not_auto_load_global(VALUE object);
RUBY_EXTERN VALUE rb_mgc_set_do_not_auto_load_global(VALUE object,
						     VALUE value);

RUBY_EXTERN VALUE rb_mgc_get_do_not_stop_on_error_global(VALUE object);
RUBY_EXTERN VALUE rb_mgc_set_do_not_stop_on_error_global(VALUE object,
							 VALUE value);

RUBY_EXTERN VALUE rb_mgc_initialize(VALUE object, VALUE arguments);

RUBY_EXTERN VALUE rb_mgc_get_do_not_stop_on_error(VALUE object);
RUBY_EXTERN VALUE rb_mgc_set_do_not_stop_on_error(VALUE object, VALUE value);

RUBY_EXTERN VALUE rb_mgc_open(VALUE object);
RUBY_EXTERN VALUE rb_mgc_close(VALUE object);
RUBY_EXTERN VALUE rb_mgc_close_p(VALUE object);

RUBY_EXTERN VALUE rb_mgc_get_paths(VALUE object);

RUBY_EXTERN VALUE rb_mgc_get_parameter(VALUE object, VALUE tag);
RUBY_EXTERN VALUE rb_mgc_set_parameter(VALUE object, VALUE tag, VALUE value);

RUBY_EXTERN VALUE rb_mgc_get_flags(VALUE object);
RUBY_EXTERN VALUE rb_mgc_set_flags(VALUE object, VALUE value);

RUBY_EXTERN VALUE rb_mgc_load(VALUE object, VALUE arguments);
RUBY_EXTERN VALUE rb_mgc_load_buffers(VALUE object, VALUE arguments);
RUBY_EXTERN VALUE rb_mgc_load_p(VALUE object);

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
