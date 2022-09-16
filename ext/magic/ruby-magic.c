#if defined(__cplusplus)
extern "C" {
#endif

#include "ruby-magic.h"

static int rb_mgc_do_not_auto_load;
static int rb_mgc_do_not_stop_on_error;
static int rb_mgc_warning;

static ID id_at_flags;
static ID id_at_paths;

static VALUE rb_cMagic;

static VALUE rb_mgc_eError;
static VALUE rb_mgc_eMagicError;
static VALUE rb_mgc_eLibraryError;
static VALUE rb_mgc_eNotImplementedError;
static VALUE rb_mgc_eParameterError;
static VALUE rb_mgc_eFlagsError;

static const rb_data_type_t rb_mgc_type;

static VALUE magic_get_parameter_internal(void *data);
static VALUE magic_set_parameter_internal(void *data);

static VALUE magic_get_flags_internal(void *data);
static VALUE magic_set_flags_internal(void *data);

static VALUE magic_load_internal(void *data);
static VALUE magic_load_buffers_internal(void *data);

static VALUE magic_compile_internal(void *data);
static VALUE magic_check_internal(void *data);

static VALUE magic_file_internal(void *data);
static VALUE magic_buffer_internal(void *data);
static VALUE magic_descriptor_internal(void *data);

static VALUE magic_close_internal(void *data);

static void *nogvl_magic_load(void *data);
static void *nogvl_magic_compile(void *data);
static void *nogvl_magic_check(void *data);
static void *nogvl_magic_file(void *data);
static void *nogvl_magic_descriptor(void *data);

static void *magic_library_open(void);
static void magic_library_close(void *data);

static VALUE magic_allocate(VALUE klass);
static void magic_mark(void *data);
static void magic_free(void *data);
static size_t magic_size(const void *data);
#if defined(HAVE_RUBY_GC_COMPACT)
static void magic_compact(void *data);
#endif

static VALUE magic_exception_wrapper(VALUE value);
static VALUE magic_exception(void *data);

static VALUE magic_library_error(VALUE klass, void *data);
static VALUE magic_generic_error(VALUE klass, int magic_errno,
				 const char *magic_error);

static VALUE magic_lock(VALUE object, VALUE (*function)(ANYARGS),
			void *data);
static VALUE magic_unlock(VALUE object);

static VALUE magic_return(void *data);

static int magic_get_flags(VALUE object);
static void magic_set_flags(VALUE object, int flags);

static VALUE magic_set_paths(VALUE object, VALUE value);

/*
 * call-seq:
 *    Magic.do_not_auto_load -> boolean
 *
 * Returns +true+ if the global +do_not_auto_load+ flag is set, or +false+
 * otherwise.
 *
 * Example:
 *
 *    Magic.do_not_auto_load        #=> false
 *    Magic.do_not_auto_load = true #=> true
 *    Magic.do_not_auto_load        #=> true
 *
 * See also: Magic::new, Magic#loaded?, Magic#load and Magic#load_buffers
 */
VALUE
rb_mgc_get_do_not_auto_load_global(RB_UNUSED_VAR(VALUE object))
{
	return CBOOL2RVAL(rb_mgc_do_not_auto_load);
}

/*
 * call-seq:
 *    Magic.do_not_auto_load= ( boolean ) -> boolean
 *
 * Sets the global +do_not_auto_load+ flag for the Magic object and each of the
 * Magic object instances. This flag can be used to disable automatic loading of
 * the Magic database files.
 *
 * Returns +true+ if the global +do_not_auto_load+ flag is set, or +false+
 * otherwise.
 *
 * Example:
 *
 *    Magic.do_not_auto_load        #=> false
 *    Magic.do_not_auto_load = true #=> true
 *    Magic.do_not_auto_load        #=> true
 *
 * Example:
 *
 *    Magic.do_not_auto_load = true                          #=> true
 *    magic = Magic.new
 *    magic.loaded?                                          #=> false
 *    magic.load_buffers(File.read(magic.paths[0] + ".mgc")) #=> nil
 *    magic.loaded?                                          #=> true
 *
 * See also: Magic::new, Magic#loaded?, Magic#load and Magic#load_buffers
 */
VALUE
rb_mgc_set_do_not_auto_load_global(RB_UNUSED_VAR(VALUE object), VALUE value)
{
	rb_mgc_do_not_auto_load = RVAL2CBOOL(value);

	return value;
}

/*
 * call-seq:
 *    Magic.do_not_stop_on_error -> boolean
 *
 * Returns +true+ if the global +do_not_stop_on_error+ flag is set, or +false+
 * otherwise.
 *
 * Example:
 *
 *    Magic.do_not_stop_on_error        #=> false
 *    Magic.do_not_stop_on_error = true #=> true
 *    Magic.do_not_stop_on_error        #=> true
 *
 * See also: Magic::new, Magic::open and Magic#do_not_stop_on_error
 */
VALUE
rb_mgc_get_do_not_stop_on_error_global(RB_UNUSED_VAR(VALUE object))
{
	return CBOOL2RVAL(rb_mgc_do_not_stop_on_error);
}

/*
 * call-seq:
 *    Magic.do_not_stop_on_error= (boolean) -> boolean
 *
 * Example:
 *
 *    Magic.do_not_stop_on_error        #=> false
 *    Magic.do_not_stop_on_error = true #=> true
 *    Magic.do_not_stop_on_error        #=> true
 *
 * See also: Magic::new, Magic::open and Magic#do_not_stop_on_error
 */
VALUE
rb_mgc_set_do_not_stop_on_error_global(RB_UNUSED_VAR(VALUE object), VALUE value)
{
	rb_mgc_do_not_stop_on_error = RVAL2CBOOL(value);

	return value;
}

/*
 * call-seq:
 *    Magic.new                -> self
 *    Magic.new( string, ... ) -> self
 *    Magic.new( array )       -> self
 *
 * Opens the underlying _Magic_ database and returns a new _Magic_.
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.class       #=> Magic
 *
 * See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, Magic::compile and Magic::check
 */
VALUE
rb_mgc_initialize(VALUE object, VALUE arguments)
{
	rb_mgc_object_t *mgc;
	const char *klass = "Magic";

	if (!NIL_P(object))
		klass = rb_obj_classname(object);

	if (rb_block_given_p())
		MAGIC_WARNING(0, "%s::new() does not take block; use %s::open() instead",
				 klass, klass);

	if (RTEST(rb_eval_string("ENV['MAGIC_DO_NOT_STOP_ON_ERROR']")))
		rb_mgc_do_not_stop_on_error = 1;

	if (RTEST(rb_eval_string("ENV['MAGIC_DO_NOT_AUTOLOAD']")))
		rb_mgc_do_not_auto_load = 1;

	MAGIC_OBJECT(object, mgc);

	mgc->stop_on_errors = 1;
	if (rb_mgc_do_not_stop_on_error)
		mgc->stop_on_errors = 0;

	mgc->mutex = rb_class_new_instance(0, 0, rb_const_get(rb_cObject,
					   rb_intern("Mutex")));

	magic_set_flags(object, MAGIC_NONE);
	magic_set_paths(object, RARRAY_EMPTY);

	if (rb_mgc_do_not_auto_load) {
		if (!RARRAY_EMPTY_P(arguments))
			MAGIC_WARNING(1, "%s::do_not_auto_load is set; using %s#new() to load "
					 "Magic database from a file will have no effect",
					 klass, klass);
		return object;
	}

	rb_mgc_load(object, arguments);

	return object;
}

/*
 * call-seq:
 *    magic.do_not_stop_on_error -> boolean
 *
 * See also: Magic::new, Magic::open and Magic::do_not_stop_on_error
 */
VALUE
rb_mgc_get_do_not_stop_on_error(VALUE object)
{
	rb_mgc_object_t *mgc;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	return CBOOL2RVAL(!mgc->stop_on_errors);
}

/*
 * call-seq:
 *    magic.do_not_stop_on_error= ( boolean ) -> boolean
 *
 * See also: Magic::new, Magic::open and Magic::do_not_stop_on_error
 */
VALUE
rb_mgc_set_do_not_stop_on_error(VALUE object, VALUE value)
{
	rb_mgc_object_t *mgc;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mgc->stop_on_errors = !RVAL2CBOOL(value);

	return value;
}

/*
 * call-seq:
 *    magic.open? -> true or false
 *
 * Returns +true+ if the underlying _Magic_ database is open,
 * or +false+ otherwise.
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.open?       #=> true
 *    magic.close       #=> nil
 *    magic.open?       #=> false
 *
 * See also: Magic#close?, Magic#close and Magic#new
 */
VALUE
rb_mgc_open_p(VALUE object)
{
	return MAGIC_CLOSED_P(object) ? Qfalse : Qtrue;
}

/*
 * call-seq:
 *    magic.close -> nil
 *
 * Closes the underlying _Magic_ database.
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.close       #=> nil
 *
 * See also: Magic#closed?, Magic#open? and Magic#new
 */
VALUE
rb_mgc_close(VALUE object)
{
	rb_mgc_object_t *mgc;

	if (MAGIC_CLOSED_P(object))
		return Qnil;

	MAGIC_OBJECT(object, mgc);

	if (mgc) {
		MAGIC_SYNCHRONIZED(magic_close_internal, mgc);
		if (DATA_P(object))
			DATA_PTR(object) = NULL;
	}

	return Qnil;
}

/*
 * call-seq:
 *    magic.closed? -> true or false
 *
 * Returns +true+ if the underlying _Magic_ database is closed,
 * or +false+ otherwise.
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.closed?     #=> false
 *    magic.close       #=> nil
 *    magic.closed?     #=> true
 *
 * See also: Magic#close, Magic#open? and #Magic#new
 */
VALUE
rb_mgc_close_p(VALUE object)
{
	rb_mgc_object_t *mgc;
	magic_t cookie = NULL;

	MAGIC_OBJECT(object, mgc);

	if (mgc)
		cookie = mgc->cookie;

	if (DATA_P(object) && cookie)
		return Qfalse;

	return Qtrue;
}

/*
 * call-seq:
 *    magic.paths -> array
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.paths       #=> ["/etc/magic", "/usr/share/misc/magic"]
 *
 */
VALUE
rb_mgc_get_paths(VALUE object)
{
	const char *cstring = NULL;
	VALUE value = Qundef;

	MAGIC_CHECK_OPEN(object);

	value = rb_ivar_get(object, id_at_paths);
	if (!NIL_P(value) && !RARRAY_EMPTY_P(value))
		return value;

	value = rb_funcall(rb_cMagic, rb_intern("default_paths"), 0);
	if (getenv("MAGIC") || NIL_P(value)) {
		cstring = magic_getpath_wrapper();
		value = magic_split(CSTR2RVAL(cstring), CSTR2RVAL(":"));
		RB_GC_GUARD(value);
	}

	return magic_set_paths(object, value);
}

/*
 * call-seq:
 *    magic.get_parameter( integer ) -> integer
 */
VALUE
rb_mgc_get_parameter(VALUE object, VALUE tag)
{
	int local_errno;
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	MAGIC_CHECK_INTEGER_TYPE(tag);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.parameter = {
			.tag = NUM2INT(tag),
		},
	};

	MAGIC_SYNCHRONIZED(magic_get_parameter_internal, &mga);
	local_errno = errno;

	if (mga.status < 0)  {
		if (local_errno == EINVAL)
			MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
					    local_errno,
					    E_PARAM_INVALID_TYPE);

		MAGIC_LIBRARY_ERROR(mgc);
	}

	return SIZET2NUM(mga.parameter.value);
}

/*
 * call-seq:
 *    magic.set_parameter( integer, integer ) -> nil
 */
VALUE
rb_mgc_set_parameter(VALUE object, VALUE tag, VALUE value)
{
	int local_errno;
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	MAGIC_CHECK_INTEGER_TYPE(tag);
	MAGIC_CHECK_INTEGER_TYPE(value);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.parameter = {
			.tag   = NUM2INT(tag),
			.value = NUM2SIZET(value),
		},
	};

	MAGIC_SYNCHRONIZED(magic_set_parameter_internal, &mga);
	local_errno = errno;

	if (mga.status < 0)  {
		switch (local_errno) {
		case EINVAL:
			MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
					    local_errno,
					    E_PARAM_INVALID_TYPE);
		case EOVERFLOW:
			MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
					    local_errno,
					    E_PARAM_INVALID_VALUE);
		}

		MAGIC_LIBRARY_ERROR(mgc);
	}

	return Qnil;
}

/*
 * call-seq:
 *    magic.flags -> integer
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.flags               #=> 0
 *    magic.flags = Magic::MIME #=> 1040
 *    magic.flags               #=> 1040
 *
 * See also: Magic#flags_to_a
 */
VALUE
rb_mgc_get_flags(VALUE object)
{
	int local_errno;
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
	};

	MAGIC_SYNCHRONIZED(magic_get_flags_internal, &mga);
	local_errno = errno;

	if (mga.status < 0 && local_errno == ENOSYS)
		return rb_ivar_get(object, id_at_flags);

	return INT2NUM(mga.flags);
}

/*
 * call-seq:
 *    magic.flags= ( integer ) -> integer
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.flags = Magic::MIME      #=> 1040
 *    magic.flags = Magic::MIME_TYPE #=> 16
 */
VALUE
rb_mgc_set_flags(VALUE object, VALUE value)
{
	int local_errno;
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;
	const char *klass = NULL;
	const char *flag = NULL;

	MAGIC_CHECK_INTEGER_TYPE(value);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.flags = NUM2INT(value),
	};

	if (mga.flags < 0)
		MAGIC_GENERIC_ERROR(rb_mgc_eFlagsError, EINVAL,
				    E_FLAG_INVALID_TYPE);

	if (mga.flags & MAGIC_DEBUG)
		flag = "DEBUG";
	else if (mga.flags & MAGIC_CHECK)
		flag = "CHECK";

	if (flag) {
		klass = "Magic";
		if (!NIL_P(object))
			klass = rb_obj_classname(object);

		MAGIC_WARNING(0, "%s::%s flag is set; verbose information will "
				 "now be printed to the standard error output",
				 klass, flag);
	}

	MAGIC_SYNCHRONIZED(magic_set_flags_internal, &mga);
	local_errno = errno;

	if (mga.status < 0)  {
		switch (local_errno) {
		case EINVAL:
			MAGIC_GENERIC_ERROR(rb_mgc_eFlagsError,
					    local_errno,
					    E_FLAG_INVALID_TYPE);
		case ENOSYS:
			MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError,
					    local_errno,
					    E_FLAG_NOT_IMPLEMENTED);
		}

		MAGIC_LIBRARY_ERROR(mgc);
	}

	return rb_ivar_set(object, id_at_flags, INT2NUM(mga.flags));
}

/*
 * call-seq:
 *    magic.load                -> nil
 *    magic.load( string, ... ) -> nil
 *    magic.load( array )       -> nil
 *
 * Example:
 *
 * See also: Magic#check, Magic#compile, Magic::check and Magic::compile
 */
VALUE
rb_mgc_load(VALUE object, VALUE arguments)
{
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;
	const char *klass = NULL;
	VALUE value = Qundef;

	if (ARRAY_P(RARRAY_FIRST(arguments)))
		arguments = magic_flatten(arguments);

	MAGIC_CHECK_ARRAY_OF_STRINGS(arguments);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	if (rb_mgc_do_not_auto_load) {
		klass = "Magic";
		if (!NIL_P(object))
			klass = rb_obj_classname(object);

		MAGIC_WARNING(2, "%s::do_not_auto_load is set; using %s#load "
				 "will load Magic database from a file",
				 klass, klass);
	}

	if (RARRAY_EMPTY_P(arguments))
		arguments = rb_mgc_get_paths(object);

	value = magic_join(arguments, CSTR2RVAL(":"));
	RB_GC_GUARD(value);

	magic_set_paths(object, RARRAY_EMPTY);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.file = {
			.path = RVAL2CSTR(value),
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_load_internal, &mga);
	if (mga.status < 0) {
		mgc->database_loaded = 0;
		MAGIC_LIBRARY_ERROR(mgc);
	}

	mgc->database_loaded = 1;

	value = magic_split(CSTR2RVAL(mga.file.path), CSTR2RVAL(":"));
	RB_GC_GUARD(value);

	magic_set_paths(object, value);

	return Qnil;
}

/*
 * call-seq:
 *    magic.load_buffers( string, ... ) -> nil
 *    magic.load_buffers( array )       -> nil
 *
 * See also: Magic#load and Magic::do_not_auto_load
 */
VALUE
rb_mgc_load_buffers(VALUE object, VALUE arguments)
{
	size_t count;
	int local_errno;
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;
	void **pointers = NULL;
	size_t *sizes = NULL;
	VALUE value = Qundef;

	count = (size_t)RARRAY_LEN(arguments);
	MAGIC_CHECK_ARGUMENT_MISSING(count, 1);

	if (ARRAY_P(RARRAY_FIRST(arguments))) {
		arguments = magic_flatten(arguments);
		count = (size_t)RARRAY_LEN(arguments);
	}

	MAGIC_CHECK_ARRAY_EMPTY(arguments);
	MAGIC_CHECK_ARRAY_OF_STRINGS(arguments);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	pointers = ALLOC_N(void *, count);
	if (!pointers) {
		local_errno = ENOMEM;
		goto error;
	}

	sizes = ALLOC_N(size_t, count);
	if (!sizes) {
		ruby_xfree(pointers);
		local_errno = ENOMEM;
		goto error;
	}

	for (size_t i = 0; i < count; i++) {
		value = RARRAY_AREF(arguments, (long)i);
		pointers[i] = (void *)RSTRING_PTR(value);
		sizes[i] = (size_t)RSTRING_LEN(value);
	}

	magic_set_paths(object, RARRAY_EMPTY);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.buffers = {
			.count    = count,
			.pointers = pointers,
			.sizes    = sizes,
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_load_buffers_internal, &mga);
	if (mga.status < 0) {
		local_errno = errno;
		ruby_xfree(pointers);
		ruby_xfree(sizes);
		goto error;
	}

	mgc->database_loaded = 1;

	ruby_xfree(pointers);
	ruby_xfree(sizes);

	return Qnil;
error:
	mgc->database_loaded = 0;

	if (local_errno == ENOMEM)
		MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
				    local_errno,
				    E_NOT_ENOUGH_MEMORY);

	MAGIC_LIBRARY_ERROR(mgc);
}

/*
 * call-seq:
 *    magic.loaded? -> true or false
 *
 * Returns +true+ if at least a single Magic database file had been loaded, or
 * +false+ otherwise. Magic database files can be loaded from a file or from an
 * in-memory buffer.
 *
 * Example:
 *
 *    magic = Magic.new
 *    magic.loaded?     #=> true
 *
 * Example:
 *
 *    Magic.do_not_auto_load = true #=> true
 *    magic = Magic.new
 *    magic.loaded?                 #=> false
 *
 * See also: Magic#load and Magic#load_buffers
 */
VALUE
rb_mgc_load_p(VALUE object)
{
	rb_mgc_object_t *mgc;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	return CBOOL2RVAL(mgc->database_loaded);
}

/*
 * call-seq:
 *    magic.compile( string ) -> nil
 *    magic.compile( array )  -> nil
 *
 * See also: Magic#check, Magic::check and Magic::compile
 */
VALUE
rb_mgc_compile(VALUE object, VALUE value)
{
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	MAGIC_CHECK_STRING_TYPE(value);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.file = {
			.path = RVAL2CSTR(value),
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_compile_internal, &mga);
	if (mga.status < 0)
		MAGIC_LIBRARY_ERROR(mgc);

	return Qnil;
}

/*
 * call-seq:
 *    magic.check( string ) -> true or false
 *    magic.check( array )  -> true or false
 *
 * See also: Magic#compile, Magic::compile and Magic::check
 */
VALUE
rb_mgc_check(VALUE object, VALUE value)
{
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	MAGIC_CHECK_STRING_TYPE(value);

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.file = {
			.path = RVAL2CSTR(value),
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_check_internal, &mga);

	return mga.status < 0 ? Qfalse : Qtrue;
}

/*
 * call-seq:
 *    magic.file( object ) -> string or array
 *    magic.file( string ) -> string or array
 *
 * See also: Magic#buffer and Magic#descriptor
 */
VALUE
rb_mgc_file(VALUE object, VALUE value)
{
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;
	const char *empty = "(null)";

	UNUSED(empty);

	if (NIL_P(value))
		goto error;

	MAGIC_CHECK_OPEN(object);
	MAGIC_CHECK_LOADED(object);
	MAGIC_OBJECT(object, mgc);

	if (rb_respond_to(value, rb_intern("to_io")))
		return rb_mgc_descriptor(object, value);

	value = magic_path(value);
	if (NIL_P(value))
		goto error;

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.file = {
			.path = RVAL2CSTR(value),
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_file_internal, &mga);
	if (mga.status < 0 && !mga.result) {
		/*
		 * Handle the case when the "ERROR" flag is set regardless of the
		 * current version of the underlying Magic library.
		 *
		 * Prior to version 5.15 the correct behavior that concerns the
		 * following IEEE 1003.1 standards was broken:
		 *
		 * http://pubs.opengroup.org/onlinepubs/007904975/utilities/file.html
		 * http://pubs.opengroup.org/onlinepubs/9699919799/utilities/file.html
		 *
		 * This is an attempt to mitigate the problem and correct it to achieve
		 * the desired behavior as per the standards.
		 */
		if (mgc->stop_on_errors || (mga.flags & MAGIC_ERROR))
			MAGIC_LIBRARY_ERROR(mgc);

		mga.result = magic_error_wrapper(mgc->cookie);
	}
	if (!mga.result)
		MAGIC_GENERIC_ERROR(rb_mgc_eMagicError, EINVAL, E_UNKNOWN);

	assert(mga.result != NULL &&
	       "Must be a valid pointer to `const char' type");

	/*
	 * Depending on the version of the underlying Magic library the magic_file()
	 * function can fail and either yield no results or return the "(null)"
	 * string instead. Often this would indicate that an older version of the
	 * Magic library is in use.
	 */
	assert(strncmp(mga.result, empty, strlen(empty)) != 0 &&
		       "Empty or invalid result");

	return magic_return(&mga);
error:
	MAGIC_ARGUMENT_TYPE_ERROR(value, "String or IO-like object");
}

/*
 * call-seq:
 *    magic.buffer( string ) -> string or array
 *
 * See also: Magic#file and Magic#descriptor
 */
VALUE
rb_mgc_buffer(VALUE object, VALUE value)
{
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	MAGIC_CHECK_STRING_TYPE(value);

	MAGIC_CHECK_OPEN(object);
	MAGIC_CHECK_LOADED(object);
	MAGIC_OBJECT(object, mgc);

	StringValue(value);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.buffers = {
			.pointers = (void **)RSTRING_PTR(value),
			.sizes    = (size_t *)RSTRING_LEN(value),
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_buffer_internal, &mga);
	if (mga.status < 0)
		MAGIC_LIBRARY_ERROR(mgc);

	assert(mga.result != NULL &&
	       "Must be a valid pointer to `const char' type");

	return magic_return(&mga);
}

/*
 * call-seq:
 *    magic.descriptor( object )  -> string or array
 *    magic.descriptor( integer ) -> string or array
 *
 * See also: Magic#file and Magic#buffer
 */
VALUE
rb_mgc_descriptor(VALUE object, VALUE value)
{
	int local_errno;
	rb_mgc_object_t *mgc;
	rb_mgc_arguments_t mga;

	if (rb_respond_to(value, rb_intern("to_io")))
		value = INT2NUM(magic_fileno(value));

	MAGIC_CHECK_INTEGER_TYPE(value);

	MAGIC_CHECK_OPEN(object);
	MAGIC_CHECK_LOADED(object);
	MAGIC_OBJECT(object, mgc);

	mga = (rb_mgc_arguments_t) {
		.magic_object = mgc,
		.file = {
			.fd = NUM2INT(value),
		},
		.flags = magic_get_flags(object),
	};

	MAGIC_SYNCHRONIZED(magic_descriptor_internal, &mga);
	local_errno = errno;

	if (mga.status < 0) {
		if (local_errno == EBADF)
			rb_raise(rb_eIOError, "Bad file descriptor");

		MAGIC_LIBRARY_ERROR(mgc);
	}

	assert(mga.result != NULL &&
	       "Must be a valid pointer to `const char' type");

	return magic_return(&mga);
}

/*
 * call-seq:
 *    Magic.version -> integer
 *
 * Example:
 *
 *    Magic.version #=> 517
 *
 * See also: Magic::version_to_a and Magic::version_to_s
 */
VALUE
rb_mgc_version(RB_UNUSED_VAR(VALUE object))
{
	return INT2NUM(magic_version_wrapper());
}

static inline void*
nogvl_magic_load(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = magic_load_wrapper(cookie,
					 mga->file.path,
					 mga->flags);

	return NULL;
}

static inline void*
nogvl_magic_compile(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = magic_compile_wrapper(cookie,
					    mga->file.path,
					    mga->flags);

	return NULL;
}

static inline void*
nogvl_magic_check(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = magic_check_wrapper(cookie,
					  mga->file.path,
					  mga->flags);

	return NULL;
}

static inline void*
nogvl_magic_file(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->result = magic_file_wrapper(cookie,
					 mga->file.path,
					 mga->flags);

	mga->status = !mga->result ? -1 : 0;

	return NULL;
}

static inline void*
nogvl_magic_descriptor(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->result = magic_descriptor_wrapper(cookie,
					       mga->file.fd,
					       mga->flags);

	mga->status = !mga->result ? -1 : 0;

	return NULL;
}

static inline VALUE
magic_get_parameter_internal(void *data)
{
	size_t value;
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = magic_getparam_wrapper(cookie,
					     mga->parameter.tag,
					     &value);

	mga->parameter.value = value;

	return (VALUE)NULL;
}

static inline VALUE
magic_set_parameter_internal(void *data)
{
	size_t value;
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	value = mga->parameter.value;

	mga->status = magic_setparam_wrapper(cookie,
					     mga->parameter.tag,
					     &value);

	return (VALUE)NULL;
}

static inline VALUE
magic_get_flags_internal(void *data)
{
	int flags;
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = 0;

	flags = magic_getflags_wrapper(cookie);
	if (flags < 0)
		mga->status = -1;
	else
		mga->flags = flags;

	return (VALUE)NULL;
}

static inline VALUE
magic_set_flags_internal(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = magic_setflags_wrapper(cookie, mga->flags);

	return (VALUE)NULL;
}

static inline VALUE
magic_close_internal(void *data)
{
	magic_library_close(data);

	return Qnil;
}

static inline VALUE
magic_load_internal(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;
	int old_flags = mga->flags;

	NOGVL(nogvl_magic_load, mga);

	if (MAGIC_STATUS_CHECK(mga->status < 0))
		magic_setflags_wrapper(cookie, old_flags);

	return (VALUE)NULL;
}

static inline VALUE
magic_load_buffers_internal(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;

	mga->status = magic_load_buffers_wrapper(cookie,
						 mga->buffers.pointers,
						 mga->buffers.sizes,
						 mga->buffers.count,
						 mga->flags);

	return (VALUE)NULL;
}

static inline VALUE
magic_compile_internal(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;
	int old_flags = mga->flags;

	NOGVL(nogvl_magic_compile, mga);

	if (MAGIC_STATUS_CHECK(mga->status < 0))
		magic_setflags_wrapper(cookie, old_flags);

	return (VALUE)NULL;
}

static inline VALUE
magic_check_internal(void *data)
{
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;
	int old_flags = mga->flags;

	NOGVL(nogvl_magic_check, mga);

	if (MAGIC_STATUS_CHECK(mga->status < 0))
		magic_setflags_wrapper(cookie, old_flags);

	return (VALUE)NULL;
}

static VALUE
magic_file_internal(void *data)
{
	int local_errno;
	int restore_flags = 0;
	rb_mgc_arguments_t *mga = data;
	rb_mgc_object_t *mgc = mga->magic_object;
	magic_t cookie = mgc->cookie;
	int old_flags = mga->flags;

	if (mgc->stop_on_errors)
		mga->flags |= MAGIC_ERROR;

	if (mga->flags & MAGIC_CONTINUE)
		mga->flags |= MAGIC_RAW;

	if (old_flags != mga->flags)
		restore_flags = 1;

	if (restore_flags)
		magic_setflags_wrapper(cookie, mga->flags);

	NOGVL(nogvl_magic_file, mga);
	local_errno = errno;
	/*
	 * The Magic library often does not correctly report errors,
	 * especially when certain flags (such as e.g., MAGIC_EXTENSION,
	 * etc.) are set. Attempt to obtain an error code first from the
	 * Magic library itself, and if that does not work, then from
	 * the saved errno value.
	 */
	if (magic_errno_wrapper(cookie) || local_errno)
		mga->status = -1;

	if (restore_flags)
		magic_setflags_wrapper(cookie, old_flags);

	return (VALUE)NULL;
}

static VALUE
magic_buffer_internal(void *data)
{
	int restore_flags = 0;
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;
	int old_flags = mga->flags;

	if (mga->flags & MAGIC_CONTINUE)
		mga->flags |= MAGIC_RAW;

	if (old_flags != mga->flags)
		restore_flags = 1;

	if (restore_flags)
		magic_setflags_wrapper(cookie, mga->flags);

	mga->result = magic_buffer_wrapper(cookie,
					   (const void *)mga->buffers.pointers,
					   (size_t)mga->buffers.sizes,
					   mga->flags);

	mga->status = !mga->result ? -1 : 0;

	if (restore_flags)
		magic_setflags_wrapper(cookie, old_flags);

	return (VALUE)NULL;
}

static VALUE
magic_descriptor_internal(void *data)
{
	int restore_flags = 0;
	rb_mgc_arguments_t *mga = data;
	magic_t cookie = mga->magic_object->cookie;
	int old_flags = mga->flags;

	if (mga->flags & MAGIC_CONTINUE)
		mga->flags |= MAGIC_RAW;

	if (old_flags != mga->flags)
		restore_flags = 1;

	if (restore_flags)
		magic_setflags_wrapper(cookie, mga->flags);

	NOGVL(nogvl_magic_descriptor, mga);

	if (restore_flags)
		magic_setflags_wrapper(cookie, old_flags);

	return (VALUE)NULL;
}

static inline void*
magic_library_open(void)
{
	magic_t cookie;

	cookie = magic_open_wrapper(MAGIC_NONE);
	if (!cookie) {
		errno = ENOMEM;
		return NULL;
	}

	return cookie;
}

static inline void
magic_library_close(void *data)
{
	rb_mgc_object_t *mgc = data;

	assert(mgc != NULL &&
	       "Must be a valid pointer to `rb_mgc_object_t' type");

	if (mgc->cookie)
		magic_close_wrapper(mgc->cookie);

	mgc->cookie = NULL;
}

static VALUE
magic_allocate(VALUE klass)
{
	int local_errno;
	rb_mgc_object_t *mgc;

	mgc = RB_ALLOC(rb_mgc_object_t);
	local_errno = ENOMEM;

	if (!mgc) {
		errno = local_errno;
		MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
				    local_errno,
				    E_NOT_ENOUGH_MEMORY);
	}

	assert(mgc != NULL &&
	       "Must be a valid pointer to `rb_mgc_object_t' type");

	mgc->cookie = NULL;
	mgc->mutex = Qundef;
	mgc->database_loaded = 0;
	mgc->stop_on_errors = 0;

	mgc->cookie = magic_library_open();
	local_errno = errno;

	if (!mgc->cookie) {
		ruby_xfree(mgc);
		mgc = NULL;
		errno = local_errno;
		MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
				    local_errno,
				    E_MAGIC_LIBRARY_INITIALIZE);
	}

	return TypedData_Wrap_Struct(klass, &rb_mgc_type, mgc);
}

static inline void
magic_mark(void *data)
{
	rb_mgc_object_t *mgc = data;

	assert(mgc != NULL &&
	       "Must be a valid pointer to `rb_mgc_object_t' type");

	MAGIC_GC_MARK(mgc->mutex);
}

static inline void
magic_free(void *data)
{
	rb_mgc_object_t *mgc = data;

	assert(mgc != NULL &&
	       "Must be a valid pointer to `rb_mgc_object_t' type");

	if (mgc->cookie)
		magic_library_close(data);

	mgc->cookie = NULL;
	mgc->mutex = Qundef;

	ruby_xfree(mgc);
}

static inline size_t
magic_size(const void *data)
{
	const rb_mgc_object_t *mgc = data;

	assert(mgc != NULL &&
	       "Must be a valid pointer to `rb_mgc_object_t' type");

	return sizeof(*mgc);
}

#if defined(HAVE_RUBY_GC_COMPACT)
static inline void
magic_compact(void *data)
{
	rb_mgc_object_t *mgc = data;

	assert(mgc != NULL &&
	       "Must be a valid pointer to `rb_mgc_object_t' type");

	mgc->mutex = rb_gc_location(mgc->mutex);
}
#endif /* HAVE_RUBY_GC_COMPACT */

static inline VALUE
magic_exception_wrapper(VALUE value)
{
	rb_mgc_error_t *mge = (rb_mgc_error_t *)value;

	return rb_exc_new2(mge->klass, mge->magic_error);
}

static VALUE
magic_exception(void *data)
{
	int exception = 0;
	rb_mgc_error_t *mge = data;
	VALUE object = Qundef;

	assert(mge != NULL &&
	       "Must be a valid pointer to `rb_mgc_error_t' type");

	object = rb_protect(magic_exception_wrapper, (VALUE)mge, &exception);

	if (exception)
		rb_jump_tag(exception);

	rb_iv_set(object, "@errno", INT2NUM(mge->magic_errno));
	RB_GC_GUARD(object);

	return object;
}

static inline VALUE
magic_generic_error(VALUE klass, int magic_errno, const char *magic_error)
{
	rb_mgc_error_t mge;

	mge = (rb_mgc_error_t) {
		.klass       = klass,
		.magic_errno = magic_errno,
		.magic_error = magic_error,
	};

	return magic_exception(&mge);
}

static VALUE
magic_library_error(VALUE klass, void *data)
{
	rb_mgc_error_t mge;
	const char *message = NULL;
	const char *empty = "(null)";
	magic_t cookie = data;

	UNUSED(empty);

	assert(cookie != NULL &&
	       "Must be a valid pointer to `magic_t' type");

	mge = (rb_mgc_error_t) {
		.klass       = klass,
		.magic_errno = -1,
		.magic_error = MAGIC_ERRORS(E_UNKNOWN),
	};

	message = magic_error_wrapper(cookie);
	if (message) {
		mge.magic_errno = magic_errno_wrapper(cookie);
		mge.magic_error = message;
	}

	assert(strncmp(mge.magic_error, empty, strlen(empty)) != 0 &&
		       "Empty or invalid error message");

	return magic_exception(&mge);
}

VALUE
magic_lock(VALUE object, VALUE(*function)(ANYARGS), void *data)
{
	rb_mgc_object_t *mgc;

	MAGIC_OBJECT(object, mgc);

	rb_funcall(mgc->mutex, rb_intern("lock"), 0);

	return rb_ensure(function, (VALUE)data, magic_unlock, object);
}

VALUE
magic_unlock(VALUE object)
{
	rb_mgc_object_t *mgc;

	MAGIC_OBJECT(object, mgc);

	rb_funcall(mgc->mutex, rb_intern("unlock"), 0);

	return Qnil;
}

static VALUE
magic_return(void *data)
{
	rb_mgc_arguments_t *mga = data;
	const char *unknown = NULL;
	VALUE separator = Qundef;
	VALUE array, string;

	string = CSTR2RVAL(mga->result);
	RB_GC_GUARD(string);

	/*
	 * The value below is a field separator that can be used to split results
	 * when the CONTINUE flag is set causing all valid matches found by the
	 * Magic library to be returned.
	 */
	if (mga->flags & MAGIC_CONTINUE)
		separator = CSTR2RVAL(MAGIC_CONTINUE_SEPARATOR);

	if (mga->flags & MAGIC_EXTENSION) {
		/*
		 * A possible I/O-related error has occurred, and there is very
		 * little sense processing the results, so return string as-is.
		 */
		if (mga->status < 0)
			return string;
		/*
		 * A number of Magic flags that support primarily files e.g.,
		 * MAGIC_EXTENSION, etc., would not return a meaningful value for
		 * directories and special files, and such. Thus, it's better to
		 * return an empty string, to indicate lack of results, rather
		 * than a confusing string consisting of three questions marks.
		 */
		unknown = "???";
		if (strncmp(mga->result, unknown, strlen(unknown)) == 0)
			return CSTR2RVAL("");

		separator = CSTR2RVAL(MAGIC_EXTENSION_SEPARATOR);
	}

	RB_GC_GUARD(separator);

	if (mga->flags & (MAGIC_CONTINUE | MAGIC_EXTENSION)) {
		array = magic_split(string, separator);
		RB_GC_GUARD(array);

		if (RARRAY_LEN(array) > 1)
			return magic_strip_array(array);

		string = magic_shift(array);
	}

	return magic_strip(string);
}

static inline int
magic_get_flags(VALUE object)
{
	return NUM2INT(rb_ivar_get(object, id_at_flags));
}

static inline void
magic_set_flags(VALUE object, int flags)
{
	rb_ivar_set(object, id_at_flags, INT2NUM(flags));
}

static inline VALUE
magic_set_paths(VALUE object, VALUE value)
{
	return rb_ivar_set(object, id_at_paths, value);
}

static const rb_data_type_t rb_mgc_type = {
	.wrap_struct_name = "magic",
	.function = {
		.dmark	  = magic_mark,
		.dfree	  = magic_free,
		.dsize	  = magic_size,
#if defined(HAVE_RUBY_GC_COMPACT)
		.dcompact = magic_compact,
#endif /* HAVE_RUBY_GC_COMPACT */
	},
#if defined(RUBY_TYPED_FREE_IMMEDIATELY)
	.flags = RUBY_TYPED_FREE_IMMEDIATELY,
#endif /* RUBY_TYPED_FREE_IMMEDIATELY */
};

void
Init_magic(void)
{
	id_at_paths = rb_intern("@paths");
	id_at_flags = rb_intern("@flags");

	rb_cMagic = rb_define_class("Magic", rb_cObject);
	rb_define_alloc_func(rb_cMagic, magic_allocate);
	/*
	 * Raised when _Magic_ encounters an error.
	 */
	rb_mgc_eError = rb_define_class_under(rb_cMagic, "Error", rb_eStandardError);
	/*
	 * Stores current value of +errno+
	 */
	rb_define_attr(rb_mgc_eError, "errno", 1, 0);
	/*
	 * Raised when
	 */
	rb_mgc_eMagicError = rb_define_class_under(rb_cMagic, "MagicError", rb_mgc_eError);
	/*
	 * Raised when
	 */
	rb_mgc_eLibraryError = rb_define_class_under(rb_cMagic, "LibraryError", rb_mgc_eError);
	/*
	 * Raised when
	 */
	rb_mgc_eParameterError = rb_define_class_under(rb_cMagic, "ParameterError", rb_mgc_eError);
	/*
	 * Raised when
	 */
	rb_mgc_eFlagsError = rb_define_class_under(rb_cMagic, "FlagsError", rb_mgc_eError);
	/*
	 * Raised when
	 */
	rb_mgc_eNotImplementedError = rb_define_class_under(rb_cMagic, "NotImplementedError", rb_mgc_eError);

	rb_define_singleton_method(rb_cMagic, "do_not_auto_load", RUBY_METHOD_FUNC(rb_mgc_get_do_not_auto_load_global), 0);
	rb_define_singleton_method(rb_cMagic, "do_not_auto_load=", RUBY_METHOD_FUNC(rb_mgc_set_do_not_auto_load_global), 1);

	rb_define_singleton_method(rb_cMagic, "do_not_stop_on_error", RUBY_METHOD_FUNC(rb_mgc_get_do_not_stop_on_error_global), 0);
	rb_define_singleton_method(rb_cMagic, "do_not_stop_on_error=", RUBY_METHOD_FUNC(rb_mgc_set_do_not_stop_on_error_global), 1);

	rb_define_singleton_method(rb_cMagic, "version", RUBY_METHOD_FUNC(rb_mgc_version), 0);

	rb_define_method(rb_cMagic, "initialize", RUBY_METHOD_FUNC(rb_mgc_initialize), -2);

	rb_define_method(rb_cMagic, "do_not_stop_on_error", RUBY_METHOD_FUNC(rb_mgc_get_do_not_stop_on_error), 0);
	rb_define_method(rb_cMagic, "do_not_stop_on_error=", RUBY_METHOD_FUNC(rb_mgc_set_do_not_stop_on_error), 1);

	rb_define_method(rb_cMagic, "open?", RUBY_METHOD_FUNC(rb_mgc_open_p), 0);
	rb_define_method(rb_cMagic, "close", RUBY_METHOD_FUNC(rb_mgc_close), 0);
	rb_define_method(rb_cMagic, "closed?", RUBY_METHOD_FUNC(rb_mgc_close_p), 0);

	rb_define_method(rb_cMagic, "paths", RUBY_METHOD_FUNC(rb_mgc_get_paths), 0);

	rb_define_method(rb_cMagic, "get_parameter", RUBY_METHOD_FUNC(rb_mgc_get_parameter), 1);
	rb_define_method(rb_cMagic, "set_parameter", RUBY_METHOD_FUNC(rb_mgc_set_parameter), 2);

	rb_define_method(rb_cMagic, "flags", RUBY_METHOD_FUNC(rb_mgc_get_flags), 0);
	rb_define_method(rb_cMagic, "flags=", RUBY_METHOD_FUNC(rb_mgc_set_flags), 1);

	rb_define_method(rb_cMagic, "file", RUBY_METHOD_FUNC(rb_mgc_file), 1);
	rb_define_method(rb_cMagic, "buffer", RUBY_METHOD_FUNC(rb_mgc_buffer), 1);
	rb_define_method(rb_cMagic, "descriptor", RUBY_METHOD_FUNC(rb_mgc_descriptor), 1);

	rb_alias(rb_cMagic, rb_intern("fd"), rb_intern("descriptor"));

	rb_define_method(rb_cMagic, "load", RUBY_METHOD_FUNC(rb_mgc_load), -2);
	rb_define_method(rb_cMagic, "load_buffers", RUBY_METHOD_FUNC(rb_mgc_load_buffers), -2);
	rb_define_method(rb_cMagic, "loaded?", RUBY_METHOD_FUNC(rb_mgc_load_p), 0);

	rb_alias(rb_cMagic, rb_intern("load_files"), rb_intern("load"));

	rb_define_method(rb_cMagic, "compile", RUBY_METHOD_FUNC(rb_mgc_compile), 1);
	rb_define_method(rb_cMagic, "check", RUBY_METHOD_FUNC(rb_mgc_check), 1);

	rb_alias(rb_cMagic, rb_intern("valid?"), rb_intern("check"));

	/*
	 * Controls how many levels of recursion will be followed for
	 * indirect magic entries.
	 */
	MAGIC_DEFINE_PARAMETER(INDIR_MAX);
	/*
	 * Controls the maximum number of calls for name or use magic.
	 */
	MAGIC_DEFINE_PARAMETER(NAME_MAX);
	/*
	 * Controls how many ELF program sections will be processed.
	 */
	MAGIC_DEFINE_PARAMETER(ELF_PHNUM_MAX);
	/*
	 * Controls how many ELF sections will be processed.
	 */
	MAGIC_DEFINE_PARAMETER(ELF_SHNUM_MAX);
	/*
	 * Controls how many ELF notes will be processed.
	 */
	MAGIC_DEFINE_PARAMETER(ELF_NOTES_MAX);
	/*
	 * Controls the length limit for regular expression searches.
	 */
	MAGIC_DEFINE_PARAMETER(REGEX_MAX);
	/*
	 * Controls the maximum number of bytes to read from a file.
	 */
	MAGIC_DEFINE_PARAMETER(BYTES_MAX);
	/*
	 * No special handling and/or flags specified. Default behavior.
	 */
	MAGIC_DEFINE_FLAG(NONE);
	/*
	 * Print debugging messages to standard error output.
	 */
	MAGIC_DEFINE_FLAG(DEBUG);
	/*
	 * If the file queried is a symbolic link, follow it.
	 */
	MAGIC_DEFINE_FLAG(SYMLINK);
	/*
	 * If the file is compressed, unpack it and look at the contents.
	 */
	MAGIC_DEFINE_FLAG(COMPRESS);
	/*
	 * If the file is a block or character special device, then open
	 * the device and try to look at the contents.
	 */
	MAGIC_DEFINE_FLAG(DEVICES);
	/*
	 * Return a MIME type string, instead of a textual description.
	 */
	MAGIC_DEFINE_FLAG(MIME_TYPE);
	/*
	 * Return all matches, not just the first.
	 */
	MAGIC_DEFINE_FLAG(CONTINUE);
	/*
	 * Check the Magic database for consistency and print warnings to
	 * standard error output.
	 */
	MAGIC_DEFINE_FLAG(CHECK);
	/*
	 * Attempt to preserve access time (atime, utime or utimes) of the
	 * file queried on systems that support such system calls.
	 */
	MAGIC_DEFINE_FLAG(PRESERVE_ATIME);
	/*
	 * Do not convert unprintable characters to an octal representation.
	 */
	MAGIC_DEFINE_FLAG(RAW);
	/*
	 * Treat operating system errors while trying to open files and follow
	 * symbolic links as first class errors, instead of storing them in the
	 * Magic library error buffer for retrieval later.
	 */
	MAGIC_DEFINE_FLAG(ERROR);
	/*
	 * Return a MIME encoding, instead of a textual description.
	 */
	MAGIC_DEFINE_FLAG(MIME_ENCODING);
	/*
	 * A shorthand for using MIME_TYPE and MIME_ENCODING flags together.
	 */
	MAGIC_DEFINE_FLAG(MIME);
	/*
	 * Return the Apple creator and type.
	 */
	MAGIC_DEFINE_FLAG(APPLE);
	/*
	 * Do not look for, or inside compressed files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_COMPRESS);
	/*
	 * Do not allow decompression that needs to fork.
	 */
	MAGIC_DEFINE_FLAG(NO_COMPRESS_FORK);
	/*
	 * Do not look for, or inside tar archive files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_TAR);
	/*
	 * Do not consult Magic files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_SOFT);
	/*
	 * Check for EMX application type (only supported on EMX).
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_APPTYPE);
	/*
	 * Do not check for ELF files (do not examine ELF file details).
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_ELF);
	/*
	 * Do not check for various types of text files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_TEXT);
	/*
	 * Do not check for CDF files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_CDF);
	/*
	 * Do not check for CSV files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_CSV);
	/*
	 * Do not look for known tokens inside ASCII files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_TOKENS);
	/*
	 * Return a MIME encoding, instead of a textual description.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_ENCODING);
	/*
	 * Do not check for JSON files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_JSON);
	/*
	 * Do not use built-in tests; only consult the Magic file.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_BUILTIN);
	/*
	 * Do not check for various types of text files, same as NO_CHECK_TEXT.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_ASCII);
	/*
	 * Do not look for Fortran sequences inside ASCII files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_FORTRAN);
	/*
	 * Do not look for troff sequences inside ASCII files.
	 */
	MAGIC_DEFINE_FLAG(NO_CHECK_TROFF);
	/*
	 * Return a slash-separated list of extensions for this file type.
	 */
	MAGIC_DEFINE_FLAG(EXTENSION);
	/*
	 * Do not report on compression, only report about the uncompressed data.
	 */
	MAGIC_DEFINE_FLAG(COMPRESS_TRANSP);
}

#if defined(__cplusplus)
}
#endif
