#if defined(__cplusplus)
extern "C" {
#endif

#include "ruby-magic.h"

static int rb_mgc_do_not_auto_load;
static int rb_mgc_do_not_stop_on_error;
static int rb_mgc_warning;

ID id_at_flags;
ID id_at_paths;

VALUE rb_cMagic = Qundef;

VALUE rb_mgc_eError = Qundef;
VALUE rb_mgc_eMagicError = Qundef;
VALUE rb_mgc_eLibraryError = Qundef;
VALUE rb_mgc_eParameterError = Qundef;
VALUE rb_mgc_eFlagsError = Qundef;
VALUE rb_mgc_eNotImplementedError = Qundef;

void Init_magic(void);

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

static void* nogvl_magic_load(void *data);
static void* nogvl_magic_compile(void *data);
static void* nogvl_magic_check(void *data);
static void* nogvl_magic_file(void *data);
static void* nogvl_magic_descriptor(void *data);

static VALUE magic_allocate(VALUE klass);
static void magic_mark(void *data);
static void magic_free(void *data);
static VALUE magic_exception_wrapper(VALUE value);
static VALUE magic_exception(void *data);
static void magic_library_close(void *data);
static VALUE magic_library_error(VALUE klass, void *data);
static VALUE magic_generic_error(VALUE klass, int magic_errno,
				 const char *magic_error);
static VALUE magic_lock(VALUE object, VALUE (*function)(ANYARGS),
			void *data);
static VALUE magic_unlock(VALUE object);
static VALUE magic_return(void *data);
static int magic_flags(VALUE object);
static int magic_set_flags(VALUE object, VALUE value);
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
	magic_object_t *mo;
	const char *klass = "Magic";

	if (!NIL_P(object))
		klass = rb_obj_classname(object);

	if (rb_block_given_p())
		MAGIC_WARNING(0, "%s::new() does not take block; use %s::open() instead",
				 klass, klass);

	if(RTEST(rb_eval_string("ENV['MAGIC_DO_NOT_STOP_ON_ERROR']")))
		rb_mgc_do_not_stop_on_error = 1;

	if(RTEST(rb_eval_string("ENV['MAGIC_DO_NOT_AUTOLOAD']")))
		rb_mgc_do_not_auto_load = 1;

	MAGIC_OBJECT(mo);
	mo->stop_on_errors = 1;
	if (rb_mgc_do_not_stop_on_error)
		mo->stop_on_errors = 0;

	mo->mutex = rb_class_new_instance(0, 0, rb_const_get(rb_cObject,
					  rb_intern("Mutex")));

	magic_set_flags(object, INT2NUM(MAGIC_NONE));
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
	magic_object_t *mo;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(mo);

	return CBOOL2RVAL(!mo->stop_on_errors);
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
	magic_object_t *mo;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(mo);

	mo->stop_on_errors = !RVAL2CBOOL(value);
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
	magic_object_t *mo;

	if (MAGIC_CLOSED_P(object))
		return Qnil;

	MAGIC_OBJECT(mo);
	if (mo) {
		MAGIC_SYNCHRONIZED(magic_close_internal, mo);
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
	magic_object_t *mo;
	magic_t cookie = NULL;

	MAGIC_OBJECT(mo);
	if (mo)
		cookie = mo->cookie;

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
	if (!NIL_P(value) && !RARRAY_EMPTY_P(value) && !getenv("MAGIC"))
		return value;

	cstring = magic_getpath_wrapper();
	value = magic_split(CSTR2RVAL(cstring), CSTR2RVAL(":"));
	RB_GC_GUARD(value);

	return value;
}

/*
 * call-seq:
 *    magic.get_parameter( integer ) -> integer
 */
VALUE
rb_mgc_get_parameter(VALUE object, VALUE tag)
{
	int local_errno;
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_INTEGER_TYPE(tag);
	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	ma.type.parameter.tag = NUM2INT(tag);

	MAGIC_SYNCHRONIZED(magic_get_parameter_internal, &ma);
	local_errno = errno;

	if (ma.status < 0)  {
		if (local_errno == EINVAL)
			MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
					    local_errno,
					    E_PARAM_INVALID_TYPE);

		MAGIC_LIBRARY_ERROR(ma.cookie);
	}

	return SIZET2NUM(ma.type.parameter.value);
}

/*
 * call-seq:
 *    magic.set_parameter( integer, integer ) -> nil
 */
VALUE
rb_mgc_set_parameter(VALUE object, VALUE tag, VALUE value)
{
	int local_errno;
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_INTEGER_TYPE(tag);
	MAGIC_CHECK_INTEGER_TYPE(value);
	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	ma.type.parameter.tag = NUM2INT(tag);
	ma.type.parameter.value = NUM2SIZET(value);

	MAGIC_SYNCHRONIZED(magic_set_parameter_internal, &ma);
	local_errno = errno;

	if (ma.status < 0)  {
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
		MAGIC_LIBRARY_ERROR(ma.cookie);
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
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	MAGIC_SYNCHRONIZED(magic_get_flags_internal, &ma);
	local_errno = errno;

	if (ma.flags < 0 && local_errno == ENOSYS)
		return rb_ivar_get(object, id_at_flags);

	return INT2NUM(ma.flags);
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
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_INTEGER_TYPE(value);
	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	ma.flags = NUM2INT(value);

	MAGIC_SYNCHRONIZED(magic_set_flags_internal, &ma);
	local_errno = errno;

	if (ma.status < 0)  {
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
		MAGIC_LIBRARY_ERROR(ma.cookie);
	}

	return rb_ivar_set(object, id_at_flags, INT2NUM(ma.flags));
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
	magic_object_t *mo;
	magic_arguments_t ma;
	const char *klass = "Magic";
	VALUE value = Qundef;

	if (ARRAY_P(RARRAY_FIRST(arguments)))
		arguments = magic_flatten(arguments);

	MAGIC_CHECK_ARRAY_OF_STRINGS(arguments);
	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	if (rb_mgc_do_not_auto_load) {
		if (!NIL_P(object))
			klass = rb_obj_classname(object);

		MAGIC_WARNING(2, "%s::do_not_auto_load is set; using %s#load "
				  "will load Magic database from a file",
				  klass, klass);
	}

	ma.flags = magic_flags(object);

	if (!RARRAY_EMPTY_P(arguments)) {
		value = magic_join(arguments, CSTR2RVAL(":"));
		ma.type.file.path = RVAL2CSTR(value);
	}
	else
		ma.type.file.path = magic_getpath_wrapper();

	magic_set_paths(object, RARRAY_EMPTY);

	MAGIC_SYNCHRONIZED(magic_load_internal, &ma);
	if (ma.status < 0) {
		mo->database_loaded = 0;
		MAGIC_LIBRARY_ERROR(ma.cookie);
	}
	mo->database_loaded = 1;

	value = magic_split(CSTR2RVAL(ma.type.file.path), CSTR2RVAL(":"));
	magic_set_paths(object, value);
	RB_GC_GUARD(value);

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
	magic_object_t *mo;
	magic_arguments_t ma;
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
	MAGIC_COOKIE(mo, ma.cookie);

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

	ma.flags = magic_flags(object);
	ma.type.buffers.count = count;
	ma.type.buffers.pointers = pointers;
	ma.type.buffers.sizes = sizes;

	magic_set_paths(object, RARRAY_EMPTY);

	MAGIC_SYNCHRONIZED(magic_load_buffers_internal, &ma);
	if (ma.status < 0) {
		local_errno = errno;
		ruby_xfree(pointers);
		ruby_xfree(sizes);
		goto error;
	}
	mo->database_loaded = 1;

	ruby_xfree(pointers);
	ruby_xfree(sizes);

	return Qnil;
error:
	mo->database_loaded = 0;

	if (local_errno == ENOMEM)
		MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
				    local_errno,
				    E_NOT_ENOUGH_MEMORY);

	MAGIC_LIBRARY_ERROR(ma.cookie);
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
	magic_object_t *mo;

	MAGIC_CHECK_OPEN(object);
	MAGIC_OBJECT(mo);

	return CBOOL2RVAL(mo->database_loaded);
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
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_STRING_TYPE(value);
	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	ma.flags = magic_flags(object);
	ma.type.file.path = RVAL2CSTR(value);

	MAGIC_SYNCHRONIZED(magic_compile_internal, &ma);
	if (ma.status < 0)
		MAGIC_LIBRARY_ERROR(ma.cookie);

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
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_STRING_TYPE(value);
	MAGIC_CHECK_OPEN(object);
	MAGIC_COOKIE(mo, ma.cookie);

	ma.flags = magic_flags(object);
	ma.type.file.path = RVAL2CSTR(value);

	MAGIC_SYNCHRONIZED(magic_check_internal, &ma);
	return ma.status < 0 ? Qfalse : Qtrue;
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
	magic_object_t *mo;
	magic_arguments_t ma;
	const char *empty = "(null)";

	if (NIL_P(value))
		goto error;

	MAGIC_CHECK_OPEN(object);
	MAGIC_CHECK_LOADED(object);
	MAGIC_COOKIE(mo, ma.cookie);

	if (rb_respond_to(value, rb_intern("to_io")))
		return rb_mgc_descriptor(object, value);

	value = magic_path(value);
	if (NIL_P(value))
		goto error;

	ma.stop_on_errors = mo->stop_on_errors;
	ma.flags = magic_flags(object);
	ma.type.file.path = RVAL2CSTR(value);

	MAGIC_SYNCHRONIZED(magic_file_internal, &ma);
	if (!ma.result) {
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
		if (mo->stop_on_errors || (ma.flags & MAGIC_ERROR))
			MAGIC_LIBRARY_ERROR(ma.cookie);
		else
			ma.result = magic_error_wrapper(ma.cookie);
	}
	if (!ma.result)
		MAGIC_GENERIC_ERROR(rb_mgc_eMagicError, EINVAL, E_UNKNOWN);

	assert(ma.result != NULL && \
	       "Must be a valid pointer to `const char' type");

	/*
	 * Depending on the version of the underlying Magic library the magic_file()
	 * function can fail and either yield no results or return the "(null)"
	 * string instead. Often this would indicate that an older version of the
	 * Magic library is in use.
	 */
	assert(strncmp(ma.result, empty, strlen(empty)) != 0 && \
		       "Empty or invalid result");

	return magic_return(&ma);
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
	magic_object_t *mo;
	magic_arguments_t ma;

	MAGIC_CHECK_STRING_TYPE(value);
	MAGIC_CHECK_OPEN(object);
	MAGIC_CHECK_LOADED(object);
	MAGIC_COOKIE(mo, ma.cookie);

	StringValue(value);

	ma.flags = magic_flags(object);
	ma.type.buffers.pointers = (void **)RSTRING_PTR(value);
	ma.type.buffers.sizes = (size_t *)RSTRING_LEN(value);

	MAGIC_SYNCHRONIZED(magic_buffer_internal, &ma);
	if (!ma.result)
		MAGIC_LIBRARY_ERROR(ma.cookie);

	assert(ma.result != NULL && \
	       "Must be a valid pointer to `const char' type");

	return magic_return(&ma);
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
	magic_object_t *mo;
	magic_arguments_t ma;

	if (rb_respond_to(value, rb_intern("to_io")))
		value = INT2NUM(magic_fileno(value));

	MAGIC_CHECK_INTEGER_TYPE(value);
	MAGIC_CHECK_OPEN(object);
	MAGIC_CHECK_LOADED(object);
	MAGIC_COOKIE(mo, ma.cookie);

	ma.flags = magic_flags(object);
	ma.type.file.fd = NUM2INT(value);

	MAGIC_SYNCHRONIZED(magic_descriptor_internal, &ma);
	local_errno = errno;

	if (!ma.result) {
		if (local_errno == EBADF)
			rb_raise(rb_eIOError, "Bad file descriptor");

		MAGIC_LIBRARY_ERROR(ma.cookie);
	}

	assert(ma.result != NULL && \
	       "Must be a valid pointer to `const char' type");

	return magic_return(&ma);
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
	magic_arguments_t *ma = data;

	ma->status = magic_load_wrapper(ma->cookie,
					ma->type.file.path,
					ma->flags);
	return NULL;
}

static inline void*
nogvl_magic_compile(void *data)
{
	magic_arguments_t *ma = data;

	ma->status = magic_compile_wrapper(ma->cookie,
					   ma->type.file.path,
					   ma->flags);
	return NULL;
}

static inline void*
nogvl_magic_check(void *data)
{
	magic_arguments_t *ma = data;

	ma->status = magic_check_wrapper(ma->cookie,
					 ma->type.file.path,
					 ma->flags);
	return NULL;
}

static inline void*
nogvl_magic_file(void *data)
{
	magic_arguments_t *ma = data;

	ma->result = magic_file_wrapper(ma->cookie,
					ma->type.file.path,
					ma->flags);
	return NULL;
}

static inline void*
nogvl_magic_descriptor(void *data)
{
	magic_arguments_t *ma = data;

	ma->result = magic_descriptor_wrapper(ma->cookie,
					      ma->type.file.fd,
					      ma->flags);
	return NULL;
}

static inline VALUE
magic_get_parameter_internal(void *data)
{
	size_t value;
	magic_arguments_t *ma = data;

	ma->status = magic_getparam_wrapper(ma->cookie,
					    ma->type.parameter.tag,
					    &value);
	ma->type.parameter.value = value;
	return (VALUE)NULL;
}

static inline VALUE
magic_set_parameter_internal(void *data)
{
	size_t value;
	magic_arguments_t *ma = data;

	value = ma->type.parameter.value;
	ma->status = magic_setparam_wrapper(ma->cookie,
					    ma->type.parameter.tag,
					    &value);
	return (VALUE)NULL;
}

static inline VALUE
magic_get_flags_internal(void *data)
{
	magic_arguments_t *ma = data;

	ma->flags = magic_getflags_wrapper(ma->cookie);
	return (VALUE)NULL;
}

static inline VALUE
magic_set_flags_internal(void *data)
{
	magic_arguments_t *ma = data;

	ma->status = magic_setflags_wrapper(ma->cookie, ma->flags);
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
	return (VALUE)NOGVL(nogvl_magic_load, data);
}

static inline VALUE
magic_load_buffers_internal(void *data)
{
	magic_arguments_t *ma = data;

	ma->status = magic_load_buffers_wrapper(ma->cookie,
						ma->type.buffers.pointers,
						ma->type.buffers.sizes,
						ma->type.buffers.count,
						ma->flags);
	return (VALUE)NULL;
}

static inline VALUE
magic_compile_internal(void *data)
{
	return (VALUE)NOGVL(nogvl_magic_compile, data);
}

static inline VALUE
magic_check_internal(void *data)
{
	return (VALUE)NOGVL(nogvl_magic_check, data);
}

static VALUE
magic_file_internal(void *data)
{
	int local_errno;
	int old_flags = 0;
	int restore_flags = 0;
	magic_arguments_t *ma = data;

	if (ma->stop_on_errors) {
		old_flags = ma->flags;
		ma->flags |= MAGIC_ERROR;
		restore_flags = 1;
	}
	if (ma->flags & MAGIC_CONTINUE) {
		old_flags = ma->flags;
		ma->flags |= MAGIC_RAW;
		restore_flags = 1;
	}
	if (restore_flags && ma->flags)
		magic_setflags_wrapper(ma->cookie, ma->flags);

	NOGVL(nogvl_magic_file, ma);
	local_errno = errno;
	/*
	 * The Magic library often does not correctly report errors,
	 * especially when certain flags (such as e.g., MAGIC_EXTENSION,
	 * etc.) are set. Attempt to obtain an error code first from the
	 * Magic library itself, and if that does not work, then from
	 * the saved errno value.
	 */
	if (magic_errno_wrapper(ma->cookie))
		ma->status = -1;
	else if (local_errno)
		ma->status = -1;

	if (restore_flags && old_flags)
		magic_setflags_wrapper(ma->cookie, old_flags);

	return (VALUE)NULL;
}

static VALUE
magic_buffer_internal(void *data)
{
	int old_flags = 0;
	int restore_flags = 0;
	magic_arguments_t *ma = data;

	if (ma->flags & MAGIC_CONTINUE) {
		old_flags = ma->flags;
		ma->flags |= MAGIC_RAW;
		restore_flags = 1;
	}
	if (restore_flags && ma->flags)
		magic_setflags_wrapper(ma->cookie, ma->flags);

	ma->result = magic_buffer_wrapper(ma->cookie,
					  (const void *)ma->type.buffers.pointers,
					  (size_t)ma->type.buffers.sizes,
					  ma->flags);
	if (restore_flags && old_flags)
		magic_setflags_wrapper(ma->cookie, old_flags);

	return (VALUE)NULL;
}

static VALUE
magic_descriptor_internal(void *data)
{
	int old_flags = 0;
	int restore_flags = 0;
	magic_arguments_t *ma = data;

	if (ma->flags & MAGIC_CONTINUE) {
		old_flags = ma->flags;
		ma->flags |= MAGIC_RAW;
		restore_flags = 1;
	}
	if (restore_flags && ma->flags)
		magic_setflags_wrapper(ma->cookie, ma->flags);

	NOGVL(nogvl_magic_descriptor, ma);

	if (restore_flags && old_flags)
		magic_setflags_wrapper(ma->cookie, old_flags);

	return (VALUE)NULL;
}

static VALUE
magic_allocate(VALUE klass)
{
	int local_errno;
	magic_object_t *mo;

	mo = (magic_object_t *)ruby_xmalloc(sizeof(magic_object_t));
	local_errno = ENOMEM;

	if (!mo) {
		errno = local_errno;
		MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
				    local_errno,
				    E_NOT_ENOUGH_MEMORY);
	}

	mo->cookie = NULL;
	mo->mutex = Qundef;
	mo->database_loaded = 0;
	mo->stop_on_errors = 0;

	mo->cookie = magic_open_wrapper(MAGIC_NONE);
	local_errno = ENOMEM;

	if (!mo->cookie) {
		ruby_xfree(mo);
		mo = NULL;
		errno = local_errno;
		MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
				    local_errno,
				    E_MAGIC_LIBRARY_INITIALIZE);
	}

	return Data_Wrap_Struct(klass, magic_mark, magic_free, mo);
}

static inline void
magic_library_close(void *data)
{
	magic_object_t *mo = data;

	assert(mo != NULL && \
	       "Must be a valid pointer to `magic_object_t' type");

	if (mo->cookie)
		magic_close_wrapper(mo->cookie);

	mo->cookie = NULL;
}

static inline void
magic_mark(void *data)
{
	magic_object_t *mo = data;

	assert(mo != NULL && \
	       "Must be a valid pointer to `magic_object_t' type");

	rb_gc_mark(mo->mutex);
}

static inline void
magic_free(void *data)
{
	magic_object_t *mo = data;

	assert(mo != NULL && \
	       "Must be a valid pointer to `magic_object_t' type");

	if (mo->cookie)
		magic_library_close(data);

	mo->cookie = NULL;
	mo->mutex = Qundef;

	ruby_xfree(mo);
}

static inline VALUE
magic_exception_wrapper(VALUE value)
{
	magic_exception_t *e = (struct magic_exception *)value;

	return rb_exc_new2(e->klass, e->magic_error);
}

static VALUE
magic_exception(void *data)
{
	magic_exception_t *e = data;
	int exception = 0;
	VALUE object = Qundef;

	assert(e != NULL && \
	       "Must be a valid pointer to `magic_exception_t' type");

	object = rb_protect(magic_exception_wrapper, (VALUE)e, &exception);

	if (exception)
		rb_jump_tag(exception);

	rb_iv_set(object, "@errno", INT2NUM(e->magic_errno));
	RB_GC_GUARD(object);

	return object;
}

static inline VALUE
magic_generic_error(VALUE klass, int magic_errno, const char *magic_error)
{
	magic_exception_t e;

	e.magic_errno = magic_errno;
	e.magic_error = magic_error;
	e.klass = klass;

	return magic_exception(&e);
}

static VALUE
magic_library_error(VALUE klass, void *data)
{
	magic_exception_t e;
	const char *message = NULL;
	const char *empty = "(null)";
	magic_t cookie = data;

	assert(cookie != NULL && \
	       "Must be a valid pointer to `magic_t' type");

	e.magic_errno = -1;
	e.magic_error = error(E_UNKNOWN);
	e.klass = klass;

	message = magic_error_wrapper(cookie);
	if (message) {
		e.magic_errno = magic_errno_wrapper(cookie);
		e.magic_error = message;
	}

	assert(strncmp(e.magic_error, empty, strlen(empty)) != 0 && \
		       "Empty or invalid error message");

	return magic_exception(&e);
}

VALUE
magic_lock(VALUE object, VALUE(*function)(ANYARGS), void *data)
{
	magic_object_t *mo;

	MAGIC_OBJECT(mo);
	rb_funcall(mo->mutex, rb_intern("lock"), 0, Qundef);

	return rb_ensure(function, (VALUE)data, magic_unlock, object);
}

VALUE
magic_unlock(VALUE object)
{
	magic_object_t *mo;

	MAGIC_OBJECT(mo);
	rb_funcall(mo->mutex, rb_intern("unlock"), 0, Qundef);

	return Qnil;
}

static VALUE
magic_return(void *data)
{
	magic_arguments_t *ma = data;
	const char *unknown = "???";
	VALUE separator = Qundef;
	VALUE array = Qundef;
	VALUE string = Qundef;

	string = CSTR2RVAL(ma->result);
	RB_GC_GUARD(string);

	/*
	 * The value below is a field separator that can be used to split results
	 * when the CONTINUE flag is set causing all valid matches found by the
	 * Magic library to be returned.
	 */
	if (ma->flags & MAGIC_CONTINUE)
		separator = CSTR2RVAL(MAGIC_CONTINUE_SEPARATOR);

	if (ma->flags & MAGIC_EXTENSION) {
		/*
		 * A possible I/O-related error has occurred, and there is very
		 * little sense processing the results, so return string as-is.
		 */
		if (ma->status < 0)
			return string;
		/*
		 * A number of Magic flags that support primarily files e.g.,
		 * MAGIC_EXTENSION, etc., would not return a meaningful value for
		 * directories and special files, and such. Thus, it's better to
		 * return an empty string, to indicate lack of results, rather
		 * than a confusing string consisting of three questions marks.
		 */
		if (strncmp(ma->result, unknown, strlen(unknown)) == 0)
			return CSTR2RVAL("");

		separator = CSTR2RVAL(MAGIC_EXTENSION_SEPARATOR);
	}

	if (ma->flags & (MAGIC_CONTINUE | MAGIC_EXTENSION)) {
		array = magic_split(string, separator);
		RB_GC_GUARD(array);
		return (RARRAY_LEN(array) > 1) ? array : magic_shift(array);
	}

	return string;
}

static inline int
magic_flags(VALUE object)
{
	return NUM2INT(rb_ivar_get(object, id_at_flags));
}

static inline int
magic_set_flags(VALUE object, VALUE value)
{
	return NUM2INT(rb_ivar_set(object, id_at_flags, value));
}

static inline VALUE
magic_set_paths(VALUE object, VALUE value)
{
	return rb_ivar_set(object, id_at_paths, value);
}

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
