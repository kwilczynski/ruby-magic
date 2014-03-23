/* :stopdoc: */

/*
 * ruby-magic.c
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

#include "ruby-magic.h"

ID id_at_flags, id_at_path, id_at_mutex;

VALUE rb_cMagic = Qnil;

VALUE rb_mgc_eError = Qnil;
VALUE rb_mgc_eMagicError = Qnil;
VALUE rb_mgc_eLibraryError = Qnil;
VALUE rb_mgc_eFlagsError = Qnil;
VALUE rb_mgc_eNotImplementedError = Qnil;

void Init_magic(void);

static VALUE magic_close_internal(void *data);
static VALUE magic_load_internal(void *data);
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
static void magic_free(void *data);

static VALUE magic_exception_wrapper(VALUE value);
static VALUE magic_exception(void *data);

static VALUE magic_library_error(VALUE klass, void *data);
static VALUE magic_generic_error(VALUE klass, int magic_errno,
        const char *magic_error);

static VALUE magic_lock(VALUE object, VALUE (*function)(ANYARGS), void *data);
static VALUE magic_unlock(VALUE object);

static VALUE magic_return(VALUE value, void *data);

/* :startdoc: */

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
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 * <i>Magic::MagicError</i> exception if, or
 *
 * See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, Magic::compile and Magic::check
 */
VALUE
rb_mgc_initialize(VALUE object, VALUE arguments)
{
    VALUE mutex;

    magic_arguments_t ma;
    const char *klass = NULL;

    if (rb_block_given_p()) {
        klass = "Magic";

        if (!NIL_P(object)) {
            klass = rb_class2name(CLASS_OF(object));
        }

        rb_warn("%s::new() does not take block; use %s::open() instead",
                klass, klass);
    }

    mutex = rb_class_new_instance(0, 0, rb_const_get(rb_cObject,
                rb_intern("Mutex")));

    rb_ivar_set(object, id_at_mutex, mutex);

    MAGIC_COOKIE(ma.cookie);

    ma.flags = MAGIC_NONE;
    ma.data.file.path = NULL;

    rb_ivar_set(object, id_at_flags, INT2NUM(ma.flags));
    rb_mgc_load(object, arguments);

    return object;
}

/*
 * call-seq:
 *    magic.close -> nil
 *
 * Closes the underlying _Magic_ database.
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *    magic.close         #=> nil
 *
 * See also: Magic#closed?
 */
VALUE
rb_mgc_close(VALUE object)
{
    magic_t cookie;

    MAGIC_COOKIE(cookie);

    if (cookie) {
        MAGIC_SYNCHRONIZED(magic_close_internal, cookie);

        if (DATA_P(object)) {
            DATA_PTR(object) = NULL;
        }
    }

    return Qnil;
}

/*
 * call-seq:
 *    magic.closed? -> true or false
 *
 * Returns +true+ if the underlying _Magic_ database is open,
 * or +false+ otherwise.
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *    magic.closed?       #=> false
 *    magic.close         #=> nil
 *    magic.closed?       #=> true
 *
 * See also: Magic#close
 */
VALUE
rb_mgc_closed(VALUE object)
{
    magic_t cookie;

    MAGIC_COOKIE(cookie);

    if (DATA_P(object) && DATA_PTR(object) && cookie) {
        return Qfalse;
    }

    return Qtrue;
}

/*
 * call-seq:
 *    magic.path -> array
 *
 * Returns an +array+
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:0x007f8fdc012e58>
 *    magic.path          #=> ["/etc/magic", "/usr/share/misc/magic"]
 *
 * Will raise <i>Magic::LibraryError</i> exception if
 */
VALUE
rb_mgc_get_path(VALUE object)
{
    VALUE value = Qnil;
    const char *cstring = NULL;

    CHECK_MAGIC_OPEN(object);

    value = rb_ivar_get(object, id_at_path);
    if (!NIL_P(value) && !RARRAY_EMPTY_P(value) && !getenv("MAGIC")) {
        return value;
    }

    cstring = magic_getpath_wrapper();
    value = magic_split(CSTR2RVAL(cstring), CSTR2RVAL(":"));

    return rb_ivar_set(object, id_at_path, value);
}

/*
 * call-seq:
 *    magic.flags -> integer
 *
 * Returns
 *
 * Example:
 *
 *    magic = Magic.new           #=> #<Magic:0x007f8fdc012e58>
 *    magic.flags                 #=> 0
 *    magic.flags = Magic::MIME   #=> 1040
 *    magic.flags                 #=> 1040
 *
 * See also: Magic#flags_to_a
 */
VALUE
rb_mgc_get_flags(VALUE object)
{
    CHECK_MAGIC_OPEN(object);
    return rb_ivar_get(object, id_at_flags);
}

/*
 * call-seq:
 *    magic.flags= (integer) -> integer
 *
 * Example:
 *
 *    magic = Magic.new                #=> #<Magic:0x007f8fdc012e58>
 *    magic.flags = Magic::MIME        #=> 1040
 *    magic.flags = Magic::MIME_TYPE   #=> 16
 *
 * Will raise <i>Magic::FlagsError</i> exception if, or
 * <i>Magic::LibraryError</i> exception if, or 
 * <i>Magic::NotImplementedError</i> exception if, or
 */
VALUE
rb_mgc_set_flags(VALUE object, VALUE value)
{
    int local_errno;
    magic_t cookie;

    Check_Type(value, T_FIXNUM);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(cookie);

    if (magic_setflags_wrapper(cookie, NUM2INT(value)) < 0) {
        local_errno = errno;

        switch (local_errno) {
            case EINVAL:
                MAGIC_GENERIC_ERROR(rb_mgc_eFlagsError, EINVAL,
                        error(E_FLAG_INVALID_VALUE));
                break;
            case ENOSYS:
                MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError, ENOSYS,
                        error(E_FLAG_NOT_IMPLEMENTED));
                break;
            default:
                MAGIC_LIBRARY_ERROR(cookie);
                break;
        }
    }

    return rb_ivar_set(object, id_at_flags, value);
}

/*
 * call-seq:
 *    magic.load              -> array
 *    magic.load( path, ... ) -> array
 *    magic.load( array )     -> array
 *
 * Example:
 *
 *    magic = Magic.new                                   #=> #<Magic:0x007f8fdc012e58>
 *    magic.load                                          #=> ["/etc/magic", "/usr/share/misc/magic"]
 *    magic.load("/usr/share/misc/magic", "/etc/magic")   #=> ["/usr/share/misc/magic", "/etc/magic"]
 *    magic.load                                          #=> ["/etc/magic", "/usr/share/misc/magic"]
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or 
 */
VALUE
rb_mgc_load(VALUE object, VALUE arguments)
{
    magic_arguments_t ma;
    VALUE value = Qnil;

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(ma.cookie);

    if (!RARRAY_EMPTY_P(arguments)) {
        value = magic_join(arguments, CSTR2RVAL(":"));
        ma.data.file.path = RVAL2CSTR(value);
    }
    else {
        ma.data.file.path = magic_getpath_wrapper();
    }

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    if (!MAGIC_SYNCHRONIZED(magic_load_internal, &ma)) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    value = magic_split(CSTR2RVAL(ma.data.file.path), CSTR2RVAL(":"));

    return rb_ivar_set(object, id_at_path, value);
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
 * See also: Magic#check, Magic::check and Magic::compile
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or
 */
VALUE
rb_mgc_compile(VALUE object, VALUE arguments)
{
    magic_arguments_t ma;
    VALUE value = Qnil;

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(ma.cookie);

    if (!RARRAY_EMPTY_P(arguments)) {
        value = magic_join(arguments, CSTR2RVAL(":"));
    }
    else {
        value = magic_join(rb_mgc_get_path(object), CSTR2RVAL(":"));
    }

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
    ma.data.file.path = RVAL2CSTR(value);

    if (!MAGIC_SYNCHRONIZED(magic_compile_internal, &ma)) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

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
 * See also: Magic#compile, Magic::compile and Magic::check
 *
 * Will raise <i>Magic::LibraryError</i> exception if, or 
 */
VALUE
rb_mgc_check(VALUE object, VALUE arguments)
{
    magic_arguments_t ma;
    VALUE value = Qnil;

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(ma.cookie);

    if (!RARRAY_EMPTY_P(arguments)) {
        value = magic_join(arguments, CSTR2RVAL(":"));
    }
    else {
        value = magic_join(rb_mgc_get_path(object), CSTR2RVAL(":"));
    }

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
    ma.data.file.path = RVAL2CSTR(value);

    if (!MAGIC_SYNCHRONIZED(magic_check_internal, &ma)) {
        return Qfalse;
    }

    return Qtrue;
}

/*
 * call-seq:
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
    magic_arguments_t ma;
    const char *cstring = NULL;
    const char *empty = "(null)";

    Check_Type(value, T_STRING);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(ma.cookie);

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
    ma.data.file.path = RVAL2CSTR(value);

    cstring = (const char *)MAGIC_SYNCHRONIZED(magic_file_internal, &ma);
    if (!cstring) {
        rv = magic_version_wrapper();

        if (ma.flags & MAGIC_ERROR) {
            MAGIC_LIBRARY_ERROR(ma.cookie);
        }
        else if (rv < 515) {
            (void)magic_errno(ma.cookie);
            cstring = magic_error(ma.cookie);
        }
    }

    assert(cstring != NULL && "Must be a valid pointer to `const char' type");
    assert(strncmp(cstring, empty, strlen(empty)) != 0 && \
            "Empty or invalid result");

    return magic_return(CSTR2RVAL(cstring), &ma);
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
    magic_arguments_t ma;
    const char *cstring = NULL;

    Check_Type(value, T_STRING);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(ma.cookie);

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    ma.data.buffer.size = RSTRING_LEN(value);
    ma.data.buffer.buffer = RVAL2CSTR(value);

    cstring = (const char *)MAGIC_SYNCHRONIZED(magic_buffer_internal, &ma);
    if (!cstring) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return magic_return(CSTR2RVAL(cstring), &ma);
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
    magic_arguments_t ma;
    const char *cstring = NULL;

    Check_Type(value, T_FIXNUM);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(ma.cookie);

    ma.flags = NUM2INT(rb_mgc_get_flags(object));
    ma.data.file.fd = NUM2INT(value);

    cstring = (const char *)MAGIC_SYNCHRONIZED(magic_descriptor_internal, &ma);
    if (!cstring) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return magic_return(CSTR2RVAL(cstring), &ma);
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
rb_mgc_version(VALUE object)
{
    int rv;
    int local_errno;

    UNUSED(object);

    rv = magic_version_wrapper();
    local_errno = errno;

    if (rv < 0 && local_errno == ENOSYS) {
        MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError, ENOSYS,
                error(E_NOT_IMPLEMENTED));
    }

    return INT2NUM(rv);
}

/* :enddoc: */

static inline void*
nogvl_magic_load(void *data)
{
    int rv;
    magic_arguments_t *ma = data;

    rv = magic_load_wrapper(ma->cookie, ma->data.file.path, ma->flags);
    return rv < 0 ? NULL : data;
}

static inline void*
nogvl_magic_compile(void *data)
{
    int rv;
    magic_arguments_t *ma = data;

    rv = magic_compile_wrapper(ma->cookie, ma->data.file.path, ma->flags);
    return rv < 0 ? NULL : data;
}

static inline void*
nogvl_magic_check(void *data)
{
    int rv;
    magic_arguments_t *ma = data;

    rv = magic_check_wrapper(ma->cookie, ma->data.file.path, ma->flags);
    return rv < 0 ? NULL : data;
}

static inline void*
nogvl_magic_file(void *data)
{
    magic_arguments_t *ma = data;
    return (void *)magic_file_wrapper(ma->cookie, ma->data.file.path, ma->flags);
}

static inline void*
nogvl_magic_descriptor(void *data)
{
    magic_arguments_t *ma = data;
    return (void *)magic_descriptor_wrapper(ma->cookie, ma->data.file.fd, ma->flags);
}

static inline VALUE
magic_close_internal(void *data)
{
    magic_free(data);
    return Qnil;
}

static inline VALUE
magic_load_internal(void *data)
{
    return (VALUE)NOGVL(nogvl_magic_load, data);
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
    return (VALUE)magic_buffer_wrapper(ma->cookie, ma->data.buffer.buffer,
            ma->data.buffer.size, ma->flags);
}

static inline VALUE
magic_descriptor_internal(void *data)
{
    return (VALUE)NOGVL(nogvl_magic_descriptor, data);
}

static VALUE
magic_allocate(VALUE klass)
{
    magic_t cookie;

    cookie = magic_open(MAGIC_NONE);
    if (!cookie) {
        MAGIC_GENERIC_ERROR(rb_mgc_eLibraryError, EPERM,
                error(E_MAGIC_LIBRARY_INITIALIZE));
    }

    return Data_Wrap_Struct(klass, NULL, magic_free, cookie);
}

static void
magic_free(void *data)
{
    magic_t cookie = data;
    assert(cookie != NULL && "Must be a valid pointer to `magic_t' type");

    if (cookie) {
        magic_close(cookie);
        cookie = NULL;
    }
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
    VALUE object = Qnil;

    magic_exception_t *e = data;
    assert(e != NULL && "Must be a valid pointer to `magic_exception_t' type");

    object = rb_protect(magic_exception_wrapper, (VALUE)e, &exception);

    if (exception) {
        rb_jump_tag(exception);
    }

    rb_iv_set(object, "@errno", INT2NUM(e->magic_errno));

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
    assert(cookie != NULL && "Must be a valid pointer to `magic_t' type");

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
    VALUE mutex = rb_ivar_get(object, id_at_mutex);
    rb_funcall(mutex, rb_intern("lock"), 0);
    return rb_ensure(function, (VALUE)data, magic_unlock, object);
}

VALUE
magic_unlock(VALUE object)
{
    VALUE mutex = rb_ivar_get(object, id_at_mutex);
    rb_funcall(mutex, rb_intern("unlock"), 0);
    return Qnil;
}

static VALUE
magic_return(VALUE value, void *data)
{
    magic_arguments_t *ma = data;
    VALUE array = Qnil;

    if (ma->flags & MAGIC_CONTINUE) {
        array = magic_split(value, CSTR2RVAL("\\012\055\040"));
        return (NUM2INT(magic_size(array)) > 1) ? array : magic_shift(array);
    }

    return value;
}

void
Init_magic(void)
{
    id_at_path  = rb_intern("@path");
    id_at_flags = rb_intern("@flags");
    id_at_mutex = rb_intern("@mutex");

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
    rb_mgc_eFlagsError = rb_define_class_under(rb_cMagic, "FlagsError", rb_mgc_eError);

    /*
     * Raised when
     */
    rb_mgc_eNotImplementedError = rb_define_class_under(rb_cMagic, "NotImplementedError", rb_mgc_eError);

    rb_define_method(rb_cMagic, "initialize", RUBY_METHOD_FUNC(rb_mgc_initialize), -2);

    rb_define_method(rb_cMagic, "close", RUBY_METHOD_FUNC(rb_mgc_close), 0);
    rb_define_method(rb_cMagic, "closed?", RUBY_METHOD_FUNC(rb_mgc_closed), 0);

    rb_define_method(rb_cMagic, "path", RUBY_METHOD_FUNC(rb_mgc_get_path), 0);
    rb_define_method(rb_cMagic, "flags", RUBY_METHOD_FUNC(rb_mgc_get_flags), 0);
    rb_define_method(rb_cMagic, "flags=", RUBY_METHOD_FUNC(rb_mgc_set_flags), 1);

    rb_define_method(rb_cMagic, "file", RUBY_METHOD_FUNC(rb_mgc_file), 1);
    rb_define_method(rb_cMagic, "buffer", RUBY_METHOD_FUNC(rb_mgc_buffer), 1);
    rb_define_method(rb_cMagic, "descriptor", RUBY_METHOD_FUNC(rb_mgc_descriptor), 1);

    rb_define_method(rb_cMagic, "load", RUBY_METHOD_FUNC(rb_mgc_load), -2);
    rb_define_method(rb_cMagic, "compile", RUBY_METHOD_FUNC(rb_mgc_compile), -2);
    rb_define_method(rb_cMagic, "check", RUBY_METHOD_FUNC(rb_mgc_check), -2);

    rb_alias(rb_cMagic, rb_intern("valid?"), rb_intern("check"));

    rb_define_singleton_method(rb_cMagic, "version", RUBY_METHOD_FUNC(rb_mgc_version), 0);

    /*
     * No special handling and/or flags specified. Default behaviour.
     */
    rb_define_const(rb_cMagic, "NONE", INT2NUM(MAGIC_NONE));

    /*
     * Print debugging messages to standard error output.
     */
    rb_define_const(rb_cMagic, "DEBUG", INT2NUM(MAGIC_DEBUG));

    /*
     * If the file queried is a symbolic link, follow it.
     */
    rb_define_const(rb_cMagic, "SYMLINK", INT2NUM(MAGIC_SYMLINK));

    /*
     * If the file is compressed, unpack it and look at the contents.
     */
    rb_define_const(rb_cMagic, "COMPRESS", INT2NUM(MAGIC_COMPRESS));

    /*
     * If the file is a block or character special device, then open
     * the device and try to look at the contents.
     */
    rb_define_const(rb_cMagic, "DEVICES", INT2NUM(MAGIC_DEVICES));

    /*
     * Return a MIME type string, instead of a textual description.
     */
    rb_define_const(rb_cMagic, "MIME_TYPE", INT2NUM(MAGIC_MIME_TYPE));

    /*
     * Return all matches, not just the first.
     */
    rb_define_const(rb_cMagic, "CONTINUE", INT2NUM(MAGIC_CONTINUE));

    /*
     * Check the Magic database for consistency and print warnings to
     * standard error output.
     */
    rb_define_const(rb_cMagic, "CHECK", INT2NUM(MAGIC_CHECK));

    /*
     * Attempt to preserve access time (atime, utime or utimes) of the
     * file queried on systems that support such system calls.
     */
    rb_define_const(rb_cMagic, "PRESERVE_ATIME", INT2NUM(MAGIC_PRESERVE_ATIME));

    /*
     * Do not translate unprintable characters to an octal representation.
     */
    rb_define_const(rb_cMagic, "RAW", INT2NUM(MAGIC_RAW));

    /*
     * Treat operating system errors while trying to open files and follow
     * symbolic links as first class errors, instead of storing them in the
     * Magic library error buffer for retrieval later.
     */
    rb_define_const(rb_cMagic, "ERROR", INT2NUM(MAGIC_ERROR));

    /*
     * Return a MIME encoding, instead of a textual description.
     */
    rb_define_const(rb_cMagic, "MIME_ENCODING", INT2NUM(MAGIC_MIME_ENCODING));

    /*
     * A shorthand for using MIME_TYPE and MIME_ENCODING flags together.
     */
    rb_define_const(rb_cMagic, "MIME", INT2NUM(MAGIC_MIME));

    /*
     * Return the Apple creator and type.
     */
    rb_define_const(rb_cMagic, "APPLE", INT2NUM(MAGIC_APPLE));

    /*
     * Do not look for, or inside compressed files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_COMPRESS", INT2NUM(MAGIC_NO_CHECK_COMPRESS));

    /*
     * Do not look for, or inside tar archive files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TAR", INT2NUM(MAGIC_NO_CHECK_TAR));

    /*
     * Do not consult Magic files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_SOFT", INT2NUM(MAGIC_NO_CHECK_SOFT));

    /*
     * Check for EMX application type (only supported on EMX).
     */
    rb_define_const(rb_cMagic, "NO_CHECK_APPTYPE", INT2NUM(MAGIC_NO_CHECK_APPTYPE));

    /*
     * Do not check for ELF files (do not examine ELF file details).
     */
    rb_define_const(rb_cMagic, "NO_CHECK_ELF", INT2NUM(MAGIC_NO_CHECK_ELF));

    /*
     * Do not check for various types of text files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TEXT", INT2NUM(MAGIC_NO_CHECK_TEXT));

    /*
     * Do not check for CDF files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_CDF", INT2NUM(MAGIC_NO_CHECK_CDF));

    /*
     * Do not look for known tokens inside ASCII files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TOKENS", INT2NUM(MAGIC_NO_CHECK_TOKENS));

    /*
     * Return a MIME encoding, instead of a textual description.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_ENCODING", INT2NUM(MAGIC_NO_CHECK_ENCODING));

    /*
     * Do not use built-in tests; only consult the Magic file.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_BUILTIN", INT2NUM(MAGIC_NO_CHECK_BUILTIN));

    /*
     * Do not check for various types of text files, same as NO_CHECK_TEXT.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_ASCII", INT2NUM(MAGIC_NO_CHECK_ASCII));

    /*
     * Do not look for Fortran sequences inside ASCII files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_FORTRAN", INT2NUM(MAGIC_NO_CHECK_FORTRAN));

    /*
     * Do not look for troff sequences inside ASCII files.
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TROFF", INT2NUM(MAGIC_NO_CHECK_TROFF));
}

/* vim: set ts=8 sw=4 sts=2 et : */
