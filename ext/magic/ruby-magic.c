#if defined(__cplusplus)
extern "C" {
#endif

#include "ruby-magic.h"

int rb_mgc_do_not_auto_load = 0;
int rb_mgc_do_not_stop_on_error = 0;

ID id_to_io;
ID id_to_path;

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
static VALUE magic_close_internal(void *data);
static VALUE magic_load_internal(void *data);
static VALUE magic_load_buffers_internal(void *data);
static VALUE magic_compile_internal(void *data);
static VALUE magic_check_internal(void *data);
static VALUE magic_file_internal(void *data);
static VALUE magic_buffer_internal(void *data);
static VALUE magic_descriptor_internal(void *data);

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

/*
 * call-seq:
 *    Magic.do_not_auto_load -> boolean
 *
 * Returns +true+ if the global +do_not_auto_load+ flag is set, or +false+
 * otherwise.
 *
 * Example:
 *
 *    Magic.do_not_auto_load	      #=> false
 *    Magic.do_not_auto_load = true   #=> true
 *    Magic.do_not_auto_load	      #=> true
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
 *    Magic.do_not_auto_load= (boolean) -> boolean
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
 *    Magic.do_not_auto_load	      #=> false
 *    Magic.do_not_auto_load = true   #=> true
 *    Magic.do_not_auto_load	      #=> true
 *
 * Example:
 *
 *    Magic.do_not_auto_load = true			       #=> true
 *    magic = Magic.new					       #=> #<Magic:0x00007fcc33070c90>
 *    magic.loaded?					       #=> false
 *    magic.load_buffers(File.read(magic.paths[0] + ".mgc"))   #=> true
 *    magic.loaded?					       #=> true
 *
 * Will raise a <i>TypeError</i> exception if given value is not an _TrueClass_
 * or _FalseClass_ type.
 *
 * See also: Magic::new, Magic#loaded?, Magic#load and Magic#load_buffers
 */
VALUE
rb_mgc_set_do_not_auto_load_global(RB_UNUSED_VAR(VALUE object), VALUE value)
{
    if (!BOOLEAN_P(value))
	MAGIC_ARGUMENT_TYPE_ERROR(value, "TrueClass or FalseClass");

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
 *    Magic.do_not_stop_on_error          #=> false
 *    Magic.do_not_stop_on_error = true   #=> true
 *    Magic.do_not_stop_on_error          #=> true
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
 * Returns
 *
 * Example:
 *
 *    Magic.do_not_stop_on_error          #=> false
 *    Magic.do_not_stop_on_error = true   #=> true
 *    Magic.do_not_stop_on_error          #=> true
 *
 * Will raise a <i>TypeError</i> exception if given value is not an _TrueClass_
 * or _FalseClass_ type.
 *
 * See also: Magic::new, Magic::open and Magic#do_not_stop_on_error
 */
VALUE
rb_mgc_set_do_not_stop_on_error_global(RB_UNUSED_VAR(VALUE object), VALUE value)
{
    if (!BOOLEAN_P(value))
	MAGIC_ARGUMENT_TYPE_ERROR(value, "TrueClass or FalseClass");

    rb_mgc_do_not_stop_on_error = RVAL2CBOOL(value);

    return value;
}

/*
 * call-seq:
 *    Magic.new              -> self
 *    Magic.new( path, ... ) -> self
 *    Magic.new( array )     -> self
 *
 * Opens the underlying _Magic_ database and returns a new _Magic_.
 *
 * Example:
 *
 *    magic = Magic.new    #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 * <i>Magic::MagicError</i> exception if
 *
 * See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, Magic::compile and Magic::check
 */
VALUE
rb_mgc_initialize(VALUE object, VALUE arguments)
{
    magic_object_t *mo;
    const char *klass = NULL;
    VALUE boolean = Qundef;

    if (rb_block_given_p()) {
	klass = "Magic";

	if (!NIL_P(object))
	    klass = rb_obj_classname(object);

	rb_warn("%s::new() does not take block; use %s::open() instead",
		klass, klass);
    }

    MAGIC_OBJECT(mo);

    mo->mutex = rb_class_new_instance(0, 0, rb_const_get(rb_cObject,
					 rb_intern("Mutex")));

    rb_ivar_set(object, id_at_paths, rb_ary_new());

    mo->stop_on_errors = 1;

    boolean = rb_mgc_get_do_not_stop_on_error_global(object);
    if (RVAL2CBOOL(boolean))
	mo->stop_on_errors = 0;

    rb_mgc_set_flags(object, INT2NUM(MAGIC_NONE));

    boolean = rb_mgc_get_do_not_auto_load_global(object);
    if (RVAL2CBOOL(boolean)) {
	rb_mgc_get_paths(object);
	return object;
    }

    rb_mgc_load(object, arguments);

    return object;
}

/*
 * call-seq:
 *    magic.do_not_stop_on_error -> boolean
 *
 * Returns
 *
 * Example:
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
 *    magic.do_not_stop_on_error= (boolean) -> boolean
 *
 * Returns
 *
 * Example:
 *
 * Will raise a <i>TypeError</i> exception if given value is not
 * an _TrueClass_ or _FalseClass_ type.
 *
 * See also: Magic::new, Magic::open and Magic::do_not_stop_on_error
 */
VALUE
rb_mgc_set_do_not_stop_on_error(VALUE object, VALUE value)
{
    magic_object_t *mo;

    if (!BOOLEAN_P(value))
	MAGIC_ARGUMENT_TYPE_ERROR(value, "TrueClass or FalseClass");

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
 *    magic = Magic.new    #=> #<Magic:0x007f8fdc012e58>
 *    magic.open?          #=> true
 *    magic.close          #=> nil
 *    magic.open?          #=> false
 *
 * See also: Magic#close?, Magic#close and Magic#new
 */
VALUE
rb_mgc_open(VALUE object)
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
 *    magic = Magic.new    #=> #<Magic:0x007f8fdc012e58>
 *    magic.close          #=> nil
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
 *    magic = Magic.new    #=> #<Magic:0x007f8fdc012e58>
 *    magic.closed?        #=> false
 *    magic.close          #=> nil
 *    magic.closed?        #=> true
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
 * Returns an +array+
 *
 * Example:
 *
 *    magic = Magic.new    #=> #<Magic:0x007f8fdc012e58>
 *    magic.paths          #=> ["/etc/magic", "/usr/share/misc/magic"]
 *
 * Will raise <i>Magic::LibraryError</i> exception if
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

    return rb_ivar_set(object, id_at_paths, value);
}

/*
 * call-seq:
 *
 * Example:
 *
 * See also:
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

    if (ma.status < 0)	{
	switch (local_errno) {
	case EINVAL:
	    MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
				local_errno,
				E_PARAM_INVALID_TYPE);
	case ENOSYS:
	    MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError,
				local_errno,
				E_NOT_IMPLEMENTED);
	}

	MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return SIZET2NUM(ma.type.parameter.value);
}

/*
 * call-seq:
 *
 * Example:
 *
 * See also:
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

    if (ma.status < 0)	{
	switch (local_errno) {
	case EINVAL:
	    MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
				local_errno,
				E_PARAM_INVALID_TYPE);
	case EOVERFLOW:
	    MAGIC_GENERIC_ERROR(rb_mgc_eParameterError,
				local_errno,
				E_PARAM_INVALID_VALUE);
	case ENOSYS:
	    MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError,
				local_errno,
				E_NOT_IMPLEMENTED);
	}

	MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return Qtrue;
}

/*
 * call-seq:
 *    magic.flags -> integer
 *
 * Returns
 *
 * Example:
 *
 *    magic = Magic.new            #=> #<Magic:0x007f8fdc012e58>
 *    magic.flags                  #=> 0
 *    magic.flags = Magic::MIME    #=> 1040
 *    magic.flags                  #=> 1040
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
 *    magic.flags= (integer) -> integer
 *
 * Example:
 *
 *    magic = Magic.new                 #=> #<Magic:0x007f8fdc012e58>
 *    magic.flags = Magic::MIME         #=> 1040
 *    magic.flags = Magic::MIME_TYPE    #=> 16
 *
 * Will raise <i>Magic::FlagsError</i> exception if, or
 * <i>Magic::LibraryError</i> exception if, or
 * <i>Magic::NotImplementedError</i> exception if, or
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

    if (ma.status < 0)	{
	switch (local_errno) {
	case EINVAL:
	    MAGIC_GENERIC_ERROR(rb_mgc_eFlagsError,
				local_errno,
				E_FLAG_INVALID_VALUE);
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
 *    magic.load              -> array
 *    magic.load( path, ... ) -> array
 *    magic.load( array )     -> array
 *
 * Example:
 *
 *    magic = Magic.new                                    #=> #<Magic:0x007f8fdc012e58>
 *    magic.load                                           #=> ["/etc/magic", "/usr/share/misc/magic"]
 *    magic.load("/usr/share/misc/magic", "/etc/magic")    #=> ["/usr/share/misc/magic", "/etc/magic"]
 *    magic.load                                           #=> ["/etc/magic", "/usr/share/misc/magic"]
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 *
 * See also: Magic#check, Magic#compile, Magic::check and Magic::compile
 */
VALUE
rb_mgc_load(VALUE object, VALUE arguments)
{
    magic_object_t *mo;
    magic_arguments_t ma;

    const char *klass = NULL;
    VALUE value = Qundef;
    VALUE boolean = Qundef;

    if (ARRAY_P(RARRAY_FIRST(arguments)))
	arguments = magic_flatten(arguments);

    MAGIC_CHECK_ARRAY_OF_STRINGS(arguments);
    MAGIC_CHECK_OPEN(object);

    MAGIC_COOKIE(mo, ma.cookie);

    boolean = rb_mgc_get_do_not_auto_load_global(object);
    if (RVAL2CBOOL(boolean)) {
	klass = "Magic";

	if (!NIL_P(object))
	    klass = rb_obj_classname(object);

	rb_warn("%s::do_not_auto_load is set; using %s#load "
		"will load Magic database from a file",
		klass, klass);
    }

    if (!RARRAY_EMPTY_P(arguments)) {
	value = magic_join(arguments, CSTR2RVAL(":"));
	ma.type.file.path = RVAL2CSTR(value);
    }
    else
	ma.type.file.path = magic_getpath_wrapper();

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    MAGIC_SYNCHRONIZED(magic_load_internal, &ma);
    if (ma.status < 0)
	MAGIC_LIBRARY_ERROR(ma.cookie);

    mo->database_loaded = 1;

    value = magic_split(CSTR2RVAL(ma.type.file.path), CSTR2RVAL(":"));

    RB_GC_GUARD(value);

    return rb_ivar_set(object, id_at_paths, value);
}

/*
 * call-seq:
 *    magic.load_buffers( string ) -> true
 *    magic.load_buffers( array )  -> true
 *
 * Example:
 *
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
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

    void **buffers = NULL;
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

    buffers = ALLOC_N(void *, count);
    if (!buffers) {
	local_errno = ENOMEM;
	goto error;
    }

    sizes = ALLOC_N(size_t, count);
    if (!sizes) {
	ruby_xfree(buffers);
	buffers = NULL;
	local_errno = ENOMEM;
	goto error;
    }

    for (size_t i = 0; i < count; i++) {
	value = RARRAY_AREF(arguments, (long)i);
	buffers[i] = (void *)RSTRING_PTR(value);
	sizes[i] = (size_t)RSTRING_LEN(value);
    }

    ma.type.buffers.count = count;
    ma.type.buffers.buffers = buffers;
    ma.type.buffers.sizes = sizes;

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    MAGIC_SYNCHRONIZED(magic_load_buffers_internal, &ma);
    if (ma.status < 0) {
	local_errno = errno;
	goto error;
    }

    mo->database_loaded = 1;

    ruby_xfree(buffers);
    ruby_xfree(sizes);

    return Qtrue;

error:
    switch (local_errno) {
    case ENOMEM:
	MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
			    local_errno,
			    E_NOT_ENOUGH_MEMORY);
    case ENOSYS:
	MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError,
			    local_errno,
			    E_NOT_IMPLEMENTED);
    }

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
 *    magic = Magic.new	=> #<Magic:0x00007fa13009af78>
 *    magic.loaded?	=> true
 *
 * Example:
 *
 *    Magic.do_not_auto_load = true	#=> true
 *    magic = Magic.new			#=> #<Magic:0x00007fa637873068>
 *    magic.loaded?			#=> false
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
 *    magic.compile              -> true
 *    magic.compile( path, ... ) -> true
 *    magic.compile( array )     -> true
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 *
 * See also: Magic#check, Magic::check and Magic::compile
 */
VALUE
rb_mgc_compile(VALUE object, VALUE arguments)
{
    magic_object_t *mo;
    magic_arguments_t ma;
    VALUE value = Qundef;

    if (ARRAY_P(RARRAY_FIRST(arguments)))
	arguments = magic_flatten(arguments);

    MAGIC_CHECK_ARRAY_OF_STRINGS(arguments);
    MAGIC_CHECK_OPEN(object);

    MAGIC_COOKIE(mo, ma.cookie);

    if (!RARRAY_EMPTY_P(arguments))
	value = magic_join(arguments, CSTR2RVAL(":"));
    else
	value = magic_join(rb_mgc_get_paths(object), CSTR2RVAL(":"));

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
    ma.type.file.path = RVAL2CSTR(value);

    MAGIC_SYNCHRONIZED(magic_compile_internal, &ma);
    if (ma.status < 0)
	MAGIC_LIBRARY_ERROR(ma.cookie);

    RB_GC_GUARD(value);

    return Qtrue;
}

/*
 * call-seq:
 *    magic.check              -> true or false
 *    magic.check( path, ... ) -> true or false
 *    magic.check( array )     -> true or false
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 *
 * See also: Magic#compile, Magic::compile and Magic::check
 */
VALUE
rb_mgc_check(VALUE object, VALUE arguments)
{
    magic_object_t *mo;
    magic_arguments_t ma;
    VALUE value = Qundef;

    if (ARRAY_P(RARRAY_FIRST(arguments)))
	arguments = magic_flatten(arguments);

    MAGIC_CHECK_ARRAY_OF_STRINGS(arguments);
    MAGIC_CHECK_OPEN(object);

    MAGIC_COOKIE(mo, ma.cookie);

    if (!RARRAY_EMPTY_P(arguments))
	value = magic_join(arguments, CSTR2RVAL(":"));
    else
	value = magic_join(rb_mgc_get_paths(object), CSTR2RVAL(":"));

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
    ma.type.file.path = RVAL2CSTR(value);

    MAGIC_SYNCHRONIZED(magic_check_internal, &ma);
    if (ma.status < 0)
	return Qfalse;

    RB_GC_GUARD(value);

    return Qtrue;
}

/*
 * call-seq:
 *    magic.file( io )   -> string or array
 *    magic.file( path ) -> string or array
 *
 * Returns
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 *
 * See also: Magic#buffer and Magic#descriptor
 */
VALUE
rb_mgc_file(VALUE object, VALUE value)
{
    int rv;
    magic_object_t *mo;
    magic_arguments_t ma;

    int clear_error = 0;
    const char *empty = "(null)";
    VALUE boolean = Qundef;

    if (NIL_P(value))
	goto error;

    MAGIC_CHECK_OPEN(object);
    MAGIC_COOKIE(mo, ma.cookie);

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    if (rb_respond_to(value, id_to_io)) {
	ma.type.file.fd = magic_fileno(value);
	MAGIC_SYNCHRONIZED(magic_descriptor_internal, &ma);
    }
    else {
	if (rb_respond_to(value, id_to_path))
	    value = rb_funcall(value, id_to_path, 0, Qundef);

	if (!STRING_P(value))
	    goto error;

	ma.type.file.path = RVAL2CSTR(value);

	if (mo->stop_on_errors && !(ma.flags & MAGIC_ERROR)) {
	    ma.flags |= MAGIC_ERROR;
	    rb_mgc_set_flags(object, INT2NUM(ma.flags));
	    clear_error = 1;
	}

	MAGIC_SYNCHRONIZED(magic_file_internal, &ma);
    }

    if (clear_error) {
	ma.flags &= ~MAGIC_ERROR;
	rb_mgc_set_flags(object, INT2NUM(ma.flags));
    }

    if (!ma.result) {
	rv = magic_version_wrapper();

	/*
	 * Handle the case when the "ERROR" flag is set regardless of the
	 * current version of the underlying Magic library.
	 *
	 * Prior to version 5.15 the correct behaviour that concerns the
	 * following IEEE 1003.1 standards was broken:
	 *
	 * http://pubs.opengroup.org/onlinepubs/007904975/utilities/file.html
	 * http://pubs.opengroup.org/onlinepubs/9699919799/utilities/file.html
	 *
	 * This is an attempt to mitigate the problem and correct it to achieve
	 * the desired behaviour as per the standards.
	 */
	if (mo->stop_on_errors || (ma.flags & MAGIC_ERROR))
	    MAGIC_LIBRARY_ERROR(ma.cookie);
	else {
	    boolean = rb_mgc_get_do_not_auto_load_global(object);
	    if (rv < 515 || RVAL2CBOOL(boolean) || ma.flags & MAGIC_EXTENSION) {
		(void)magic_errno(ma.cookie);
		ma.result = magic_error(ma.cookie);
	    }
	}
    }

    if (!ma.result)
	MAGIC_GENERIC_ERROR(rb_mgc_eMagicError, EINVAL, E_UNKNOWN);

    assert(ma.result != NULL && \
	   "Must be a valid pointer to `const char' type");

    /*
     * Depending on the version of the underlying Magic library the magic_file()
     * function can fail and either yield no results or return the "(null)"
     * string instead.  Often this would indicate that an older version of the
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
 * Returns
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
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

    MAGIC_COOKIE(mo, ma.cookie);

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    StringValue(value);

    ma.type.buffers.sizes = (size_t *)RSTRING_LEN(value);
    ma.type.buffers.buffers = (void **)RSTRING_PTR(value);

    MAGIC_SYNCHRONIZED(magic_buffer_internal, &ma);
    if (!ma.result)
	MAGIC_LIBRARY_ERROR(ma.cookie);

    assert(ma.result != NULL && \
	   "Must be a valid pointer to `const char' type");

    return magic_return(&ma);
}

/*
 * call-seq:
 *    magic.descriptor( integer ) -> string or array
 *
 * Returns
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 *
 * See also: Magic#file and Magic#buffer
 */
VALUE
rb_mgc_descriptor(VALUE object, VALUE value)
{
    int local_errno;
    magic_object_t *mo;
    magic_arguments_t ma;

    MAGIC_CHECK_INTEGER_TYPE(value);
    MAGIC_CHECK_OPEN(object);
    MAGIC_COOKIE(mo, ma.cookie);

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
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
 * Returns
 *
 * Example:
 *
 *    Magic.version   #=> 517
 *
 * Will raise <i>Magic::NotImplementedError</i> exception if, or
 *
 * See also: Magic::version_to_a and Magic::version_to_s
 */
VALUE
rb_mgc_version(RB_UNUSED_VAR(VALUE object))
{
    int rv;
    int local_errno;

    rv = magic_version_wrapper();
    local_errno = errno;

    if (rv < 0 && local_errno == ENOSYS)
	MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError,
			    local_errno,
			    E_NOT_IMPLEMENTED);

    return INT2NUM(rv);
}

static inline void*
nogvl_magic_load(void *data)
{
    magic_arguments_t *ma = data;
    ma->status = magic_load_wrapper(ma->cookie,
				    ma->type.file.path,
				    ma->flags);
    return ma;
}

static inline void*
nogvl_magic_compile(void *data)
{
    magic_arguments_t *ma = data;
    ma->status = magic_compile_wrapper(ma->cookie,
				       ma->type.file.path,
				       ma->flags);
    return ma;
}

static inline void*
nogvl_magic_check(void *data)
{
    magic_arguments_t *ma = data;
    ma->status = magic_check_wrapper(ma->cookie,
				     ma->type.file.path,
				     ma->flags);
    return ma;
}

static inline void*
nogvl_magic_file(void *data)
{
    magic_arguments_t *ma = data;
    ma->result = magic_file_wrapper(ma->cookie,
				    ma->type.file.path,
				    ma->flags);
    return ma;
}

static inline void*
nogvl_magic_descriptor(void *data)
{
    magic_arguments_t *ma = data;
    ma->result = magic_descriptor_wrapper(ma->cookie,
					  ma->type.file.fd,
					  ma->flags);
    return ma;
}

static inline VALUE
magic_get_parameter_internal(void *data)
{
    magic_arguments_t *ma = data;
    ma->status = magic_getparam_wrapper(ma->cookie,
					ma->type.parameter.tag,
					&ma->type.parameter.value);
    return (VALUE)ma;
}

static inline VALUE
magic_set_parameter_internal(void *data)
{
    magic_arguments_t *ma = data;
    ma->status = magic_setparam_wrapper(ma->cookie,
					ma->type.parameter.tag,
					&ma->type.parameter.value);
    return (VALUE)ma;
}

static inline VALUE
magic_get_flags_internal(void *data)
{
    magic_arguments_t *ma = data;
    ma->flags = magic_getflags_wrapper(ma->cookie);
    return (VALUE)ma;
}

static inline VALUE
magic_set_flags_internal(void *data)
{
    magic_arguments_t *ma = data;
    ma->status = magic_setflags_wrapper(ma->cookie, ma->flags);
    return (VALUE)ma;
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
					    ma->type.buffers.buffers,
					    ma->type.buffers.sizes,
					    ma->type.buffers.count,
					    ma->flags);
    return (VALUE)ma;
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

static inline VALUE
magic_file_internal(void *data)
{
    return (VALUE)NOGVL(nogvl_magic_file, data);
}

static inline VALUE
magic_buffer_internal(void *data)
{
    magic_arguments_t *ma = data;
    ma->result = magic_buffer_wrapper(ma->cookie,
				      (const void *)ma->type.buffers.buffers,
				      (size_t)ma->type.buffers.sizes,
				      ma->flags);
    return (VALUE)ma;
}

static inline VALUE
magic_descriptor_internal(void *data)
{
    return (VALUE)NOGVL(nogvl_magic_descriptor, data);
}

static VALUE
magic_allocate(VALUE klass)
{
    magic_object_t *mo;

    mo = (magic_object_t *)ruby_xmalloc(sizeof(magic_object_t));
    if (!mo) {
	errno = ENOMEM;
	MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
			    errno,
			    E_NOT_ENOUGH_MEMORY);
    }

    mo->cookie = NULL;
    mo->mutex = Qundef;

    mo->cookie = magic_open_wrapper(MAGIC_NONE);
    if (!mo->cookie) {
	ruby_xfree(mo);
	mo = NULL;
	errno = ENOMEM;
	MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError,
			    errno,
			    E_MAGIC_LIBRARY_INITIALIZE);
    }

    return Data_Wrap_Struct(klass, magic_mark, magic_free, mo);
}

static void
magic_library_close(void *data)
{
    magic_object_t *mo = data;

    assert(mo != NULL && \
	   "Must be a valid pointer to `magic_object_t' type");

    if (mo->cookie)
	magic_close_wrapper(mo->cookie);
    
    mo->cookie = NULL;
}

static void
magic_mark(void *data)
{
    magic_object_t *mo = data;

    assert(mo != NULL && \
	   "Must be a valid pointer to `magic_object_t' type");

    rb_gc_mark(mo->mutex);
}

static void
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

static VALUE
magic_exception_wrapper(VALUE value)
{
    magic_exception_t *e = (struct magic_exception *)value;

    return rb_exc_new2(e->klass, e->magic_error);
}

static VALUE
magic_exception(void *data)
{
    int exception = 0;
    VALUE object = Qundef;

    magic_exception_t *e = data;

    assert(e != NULL && \
	   "Must be a valid pointer to `magic_exception_t' type");

    object = rb_protect(magic_exception_wrapper, (VALUE)e, &exception);

    if (exception)
	rb_jump_tag(exception);

    rb_iv_set(object, "@errno", INT2NUM(e->magic_errno));

    RB_GC_GUARD(object);

    return object;
}

static VALUE
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

    message = magic_error(cookie);
    if (message) {
	e.magic_errno = magic_errno(cookie);
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
    const char *empty = "???";

    VALUE value = Qundef;
    VALUE array = Qundef;

    value = CSTR2RVAL(ma->result);

    if (ma->flags & MAGIC_EXTENSION) {
	/*
	 * A number of Magic flags that support primarily files e.g.,
	 * MAGIC_EXTENSION, etc., would not return a meaningful value for
	 * directories and special files, and such.  Thus, it's better to
	 * return a nil value, to indicate lack of results, rather than a
	 * confusing string consisting of three questions marks.
	 */
	if (strncmp(ma->result, empty, strlen(empty)) == 0)
	    return Qnil;

	array = magic_split(value, CSTR2RVAL("\x5c"));

	RB_GC_GUARD(array);
	return (NUM2INT(magic_size(array)) > 1) ? array : magic_shift(array);
    }

    /*
     * The value below is a field separator that can be used to split results
     * when the CONTINUE flag is set causing all valid matches found by the
     * Magic library to be returned.
     */
    if (ma->flags & MAGIC_CONTINUE) {
	array = magic_split(value, CSTR2RVAL("\x5c\x30\x31\x32\x2d\x20"));

	RB_GC_GUARD(array);
	return (NUM2INT(magic_size(array)) > 1) ? array : magic_shift(array);
    }

    RB_GC_GUARD(value);

    return value;
}

void
Init_magic(void)
{
    id_to_io = rb_intern("to_io");
    id_to_path = rb_intern("to_path");

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

    rb_define_method(rb_cMagic, "initialize", RUBY_METHOD_FUNC(rb_mgc_initialize), -2);

    rb_define_method(rb_cMagic, "do_not_stop_on_error", RUBY_METHOD_FUNC(rb_mgc_get_do_not_stop_on_error), 0);
    rb_define_method(rb_cMagic, "do_not_stop_on_error=", RUBY_METHOD_FUNC(rb_mgc_set_do_not_stop_on_error), 1);

    rb_define_method(rb_cMagic, "open?", RUBY_METHOD_FUNC(rb_mgc_open), 0);
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

    rb_define_method(rb_cMagic, "load", RUBY_METHOD_FUNC(rb_mgc_load), -2);
    rb_define_method(rb_cMagic, "load_buffers", RUBY_METHOD_FUNC(rb_mgc_load_buffers), -2);
    rb_define_method(rb_cMagic, "loaded?", RUBY_METHOD_FUNC(rb_mgc_load_p), 0);

    rb_define_method(rb_cMagic, "compile", RUBY_METHOD_FUNC(rb_mgc_compile), -2);
    rb_define_method(rb_cMagic, "check", RUBY_METHOD_FUNC(rb_mgc_check), -2);

    rb_alias(rb_cMagic, rb_intern("valid?"), rb_intern("check"));

    rb_define_singleton_method(rb_cMagic, "version", RUBY_METHOD_FUNC(rb_mgc_version), 0);

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
     * No special handling and/or flags specified. Default behaviour.
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
