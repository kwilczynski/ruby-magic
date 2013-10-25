/* :stopdoc: */

/*
 * ruby-magic.c
 *
 * Copyright 2013 Krzysztof Wilczynski
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

ID id_at_flags, id_at_path;

VALUE rb_cMagic = Qnil;

VALUE rb_mgc_eError = Qnil;
VALUE rb_mgc_eMagicError = Qnil;
VALUE rb_mgc_eBadAddressError = Qnil;
VALUE rb_mgc_eFlagsError = Qnil;
VALUE rb_mgc_eNotImplementedError = Qnil;

void Init_magic(void);

static void* nogvl_magic_load_wrapper(void *data);
static void* nogvl_magic_check_wrapper(void *data);
static void* nogvl_magic_compile_wrapper(void *data);
static void* nogvl_magic_file_wrapper(void *data);
static void* nogvl_magic_descriptor_wrapper(void *data);

static int magic_version(void);
#if defined(HAVE_MAGIC_VERSION)
static const char* magic_version_string(void);
#endif
static VALUE magic_allocate(VALUE klass);
static void magic_free(void *data);

static VALUE magic_exception_wrapper(VALUE value);
static VALUE magic_exception(void *data);

static VALUE magic_library_error(VALUE klass, void *data);
static VALUE magic_generic_error(VALUE klass, int magic_errno,
        const char *magic_error);

/* :startdoc: */

/*
 * call-seq:
 *    Magic.new -> self
 *
 * Returns a new _Magic_.
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 *
 * See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, Magic::compile and Magic::check
 */
VALUE
rb_mgc_initialize(VALUE object)
{
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

    MAGIC_COOKIE(object, ma.cookie);

    ma.flags = MAGIC_NONE;
    ma.file.path = NULL;

    if (!NOGVL(nogvl_magic_load_wrapper, &ma)) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    rb_ivar_set(object, id_at_flags, INT2NUM(ma.flags));

    return object;
}

/*
 * call-seq:
 *    magic.close -> nil
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 *    magic.close         #=> nil
 */
VALUE
rb_mgc_close(VALUE object)
{
    magic_t cookie;

    MAGIC_COOKIE(object, cookie);

    if (cookie) {
        magic_free(cookie);

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
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 *    magic.closed?       #=> false
 *    magic.close         #=> nil
 *    magic.closed?       #=> true
 */
VALUE
rb_mgc_closed(VALUE object)
{
    magic_t cookie;

    MAGIC_COOKIE(object, cookie);

    if (DATA_P(object) && DATA_PTR(object) && cookie) {
        return Qfalse;
    }

    return Qtrue;
}

/*
 * call-seq:
 *    magic.path -> array
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 *    magic.path          #=> ["/etc/magic", "/usr/share/misc/magic"]
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
 * Example:
 *
 *    magic = Magic.new           #=> #<Magic:>
 *    magic.flags                 #=> 0
 *    magic.flags = Magic::MIME   #=> 1040
 *    magic.flags                 #=> 1040
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
 *    magic = Magic.new                #=> #<Magic:>
 *    magic.flags = Magic::MIME        #=> 1040
 *    magic.flags = Magic::MIME_TYPE   #=> 16
 */
VALUE
rb_mgc_set_flags(VALUE object, VALUE value)
{
    int old_errno;
    magic_t cookie;

    Check_Type(value, T_FIXNUM);

    CHECK_MAGIC_OPEN(object);

    MAGIC_COOKIE(object, cookie);

    if (magic_setflags_wrapper(cookie, NUM2INT(value)) < 0) {
        old_errno = errno;

        switch (old_errno) {
            case ENOSYS:
                MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError, ENOSYS,
                        error(E_NOT_IMPLEMENTED));
                break;
            case EINVAL:
                MAGIC_GENERIC_ERROR(rb_mgc_eFlagsError, EINVAL,
                        error(E_INVALID_ARGUMENT));
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
 *    magic.load          -> array
 *    magic.load( array ) -> array
 *
 * Example:
 *
 *    magic = Magic.new                                   #=> #<Magic:>
 *    magic.load                                          #=> ["/etc/magic", "/usr/share/misc/magic"]
 *    magic.load("/usr/share/misc/magic", "/etc/magic")   #=> ["/usr/share/misc/magic", "/etc/magic"]
 *    magic.load                                          #=> ["/etc/magic", "/usr/share/misc/magic"]
 */
VALUE
rb_mgc_load(VALUE object, VALUE arguments)
{
    magic_arguments_t ma;
    VALUE value = Qnil;

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(object, ma.cookie);

    if (!RARRAY_EMPTY_P(arguments)) {
        value = magic_join(arguments, CSTR2RVAL(":"));
        ma.file.path = RVAL2CSTR(value);
    }
    else {
        ma.file.path = magic_getpath_wrapper();
    }

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    if (!NOGVL(nogvl_magic_load_wrapper, &ma)) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    value = magic_split(CSTR2RVAL(ma.file.path), CSTR2RVAL(":"));

    return rb_ivar_set(object, id_at_path, value);
}

/*
 * call-seq:
 *    magic.compile          -> true
 *    magic.compile( array ) -> true
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 */
VALUE
rb_mgc_compile(VALUE object, VALUE arguments)
{
    magic_arguments_t ma;
    VALUE value = Qnil;

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(object, ma.cookie);

    if (!RARRAY_EMPTY_P(arguments)) {
        value = magic_join(arguments, CSTR2RVAL(":"));
        ma.file.path = RVAL2CSTR(value);
    }

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    if (!NOGVL(nogvl_magic_compile_wrapper, &ma)) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return Qtrue;
}

/*
 * call-seq:
 *    magic.check          -> true or false
 *    magic.check( array ) -> true or false
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 */
VALUE
rb_mgc_check(VALUE object, VALUE arguments)
{
    magic_arguments_t ma;
    VALUE value = Qnil;

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(object, ma.cookie);

    if (!RARRAY_EMPTY_P(arguments)) {
        value = magic_join(arguments, CSTR2RVAL(":"));
        ma.file.path = RVAL2CSTR(value);
    }

    ma.flags = NUM2INT(rb_mgc_get_flags(object));

    if (!NOGVL(nogvl_magic_check_wrapper, &ma)) {
        return Qfalse;
    }

    return Qtrue;
}

/*
 * call-seq:
 *    magic.file( string ) -> string
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 */
VALUE
rb_mgc_file(VALUE object, VALUE value)
{
    magic_arguments_t ma;
    const char *cstring = NULL;

    Check_Type(value, T_STRING);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(object, ma.cookie);

    ma.file.path = RVAL2CSTR(value);

    cstring = (const char *)NOGVL(nogvl_magic_file_wrapper, &ma);
    if (!cstring) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return CSTR2RVAL(cstring);
}

/*
 * call-seq:
 *    magic.buffer( string ) -> string
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 */
VALUE
rb_mgc_buffer(VALUE object, VALUE value)
{
    magic_t cookie;
    const char *cstring = NULL;

    Check_Type(value, T_STRING);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(object, cookie);

    cstring = magic_buffer(cookie, RVAL2CSTR(value), RSTRING_LEN(value));
    if (!cstring) {
        MAGIC_LIBRARY_ERROR(cookie);
    }

    return CSTR2RVAL(cstring);
}

/*
 * call-seq:
 *    magic.descriptor( integer ) -> string
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 */
VALUE
rb_mgc_descriptor(VALUE object, VALUE value)
{
    magic_arguments_t ma;
    const char *cstring = NULL;

    Check_Type(value, T_FIXNUM);

    CHECK_MAGIC_OPEN(object);
    MAGIC_COOKIE(object, ma.cookie);

    ma.file.fd = NUM2INT(value);

    cstring = (const char *)NOGVL(nogvl_magic_descriptor_wrapper, &ma);
    if (!cstring) {
        MAGIC_LIBRARY_ERROR(ma.cookie);
    }

    return CSTR2RVAL(cstring);
}

/*
 * call-seq:
 *    magic.version -> integer
 *
 * Example:
 *
 *    magic = Magic.new   #=> #<Magic:>
 *    magic.version       #=>
 */
VALUE
rb_mgc_version(VALUE object)
{
    int rv;

    UNUSED(object);

    rv = magic_version();
    if (rv < 0) {
        MAGIC_GENERIC_ERROR(rb_mgc_eNotImplementedError, ENOSYS,
                error(E_NOT_IMPLEMENTED));
    }

    return INT2NUM(rv);
}

/* :enddoc: */

inline void*
nogvl_magic_load_wrapper(void *data)
{
    int rv;
    magic_arguments_t *ma = data;

    rv = magic_load_wrapper(ma->cookie, ma->file.path, ma->flags);
    return rv < 0 ? NULL : data;
}

inline void*
nogvl_magic_check_wrapper(void *data)
{
    int rv;
    magic_arguments_t *ma = data;

    rv = magic_check_wrapper(ma->cookie, ma->file.path, ma->flags);
    return rv < 0 ? NULL : data;
}

inline void*
nogvl_magic_compile_wrapper(void *data)
{
    int rv;
    magic_arguments_t *ma = data;

    rv = magic_compile_wrapper(ma->cookie, ma->file.path, ma->flags);
    return rv < 0 ? NULL : data;
}

inline void*
nogvl_magic_file_wrapper(void *data)
{
    magic_arguments_t *ma = data;
    return (void *)magic_file(ma->cookie, ma->file.path);
}

inline void*
nogvl_magic_descriptor_wrapper(void *data)
{
    magic_arguments_t *ma = data;
    return (void *)magic_descriptor(ma->cookie, ma->file.fd);
}

int
magic_version(void)
{
    int rv;
    int old_errno;

    rv = magic_version_wrapper();
    old_errno = errno;

    if (rv < 0 && old_errno == ENOSYS) {
        return -1;
    }

    return rv;
}

#if defined(HAVE_MAGIC_VERSION)
const char*
magic_version_string(void)
{
    int rv;

    rv = magic_version();
    if (rv < 0) {
        return NULL;
    }

    return sprintf("%d.%02d", rv / 100, rv % 100);
}
#endif

VALUE
magic_allocate(VALUE klass)
{
    magic_t cookie;

    cookie = magic_open(MAGIC_NONE);
    if (!cookie) {
        rb_memerror();
    }

    return Data_Wrap_Struct(klass, NULL, magic_free, cookie);
}

void
magic_free(void *data)
{
    magic_t cookie = data;

    if (cookie) {
        magic_close(cookie);
        cookie = NULL;
    }
}

VALUE
magic_exception_wrapper(VALUE value)
{
    magic_exception_t *e = (struct magic_exception *)value;

    return rb_exc_new2(e->klass, e->magic_error);
}

VALUE
magic_exception(void *data)
{
    int exception = 0;
    VALUE object = Qnil;

    magic_exception_t *e = data;

    object = rb_protect(magic_exception_wrapper, (VALUE)e, &exception);

    if (exception) {
        rb_jump_tag(exception);
    }

    rb_iv_set(object, "@errno", INT2NUM(e->magic_errno));

    return object;
}

VALUE
magic_generic_error(VALUE klass, int magic_errno, const char *magic_error)
{
    magic_exception_t e;

    e.magic_errno = magic_errno;
    e.magic_error = magic_error;
    e.klass = klass;

    return magic_exception(&e);
}

VALUE
magic_library_error(VALUE klass, void *data)
{
    magic_exception_t e;
    const char *message = NULL;

    magic_t cookie = data;

    e.magic_errno = -1;
    e.magic_error = error(E_UNKNOWN);
    e.klass = klass;

    message = magic_error(cookie);
    if (message) {
        e.magic_errno = magic_errno(cookie);
        e.magic_error = message;
    }

    return magic_exception(&e);
}

void
Init_magic(void)
{
#if defined(HAVE_MAGIC_VERSION)
    int rv;
#endif

    id_at_path  = rb_intern("@path");
    id_at_flags = rb_intern("@flags");

    rb_cMagic = rb_define_class("Magic", rb_cObject);
    rb_define_alloc_func(rb_cMagic, magic_allocate);

    /*
     *
     */
    rb_mgc_eError = rb_define_class_under(rb_cMagic, "Error", rb_eStandardError);

    /*
     *
     */
    rb_define_attr(rb_mgc_eError, "errno", 1, 0);

    /*
     *
     */
    rb_mgc_eMagicError = rb_define_class_under(rb_cMagic, "MagicError", rb_mgc_eError);

    /*
     *
     */
    rb_mgc_eBadAddressError = rb_define_class_under(rb_cMagic, "BadAddressError", rb_mgc_eError);

    /*
     *
     */
    rb_mgc_eFlagsError = rb_define_class_under(rb_cMagic, "FlagsError", rb_mgc_eError);

    /*
     *
     */
    rb_mgc_eNotImplementedError = rb_define_class_under(rb_cMagic, "NotImplementedError", rb_mgc_eError);

    rb_define_method(rb_cMagic, "initialize", RUBY_METHOD_FUNC(rb_mgc_initialize), 0);

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

    rb_define_method(rb_cMagic, "version", RUBY_METHOD_FUNC(rb_mgc_version), 0);

    /*
     *
     */
#if defined(HAVE_MAGIC_VERSION)
    rb_define_const(rb_cMagic, "MAGIC_VERSION", CSTR2RVAL(magic_version_string()));
#else
    rb_define_const(rb_cMagic, "MAGIC_VERSION", Qnil);
#endif

    /*
     *
     */
    rb_define_const(rb_cMagic, "NONE", INT2NUM(MAGIC_NONE));

    /*
     *
     */
    rb_define_const(rb_cMagic, "DEBUG", INT2NUM(MAGIC_DEBUG));

    /*
     *
     */
    rb_define_const(rb_cMagic, "SYMLINK", INT2NUM(MAGIC_SYMLINK));

    /*
     *
     */
    rb_define_const(rb_cMagic, "COMPRESS", INT2NUM(MAGIC_COMPRESS));

    /*
     *
     */
    rb_define_const(rb_cMagic, "DEVICES", INT2NUM(MAGIC_DEVICES));

    /*
     *
     */
    rb_define_const(rb_cMagic, "MIME_TYPE", INT2NUM(MAGIC_MIME_TYPE));

    /*
     *
     */
    rb_define_const(rb_cMagic, "CONTINUE", INT2NUM(MAGIC_CONTINUE));

    /*
     *
     */
    rb_define_const(rb_cMagic, "CHECK", INT2NUM(MAGIC_CHECK));

    /*
     *
     */
    rb_define_const(rb_cMagic, "PRESERVE_ATIME", INT2NUM(MAGIC_PRESERVE_ATIME));

    /*
     *
     */
    rb_define_const(rb_cMagic, "RAW", INT2NUM(MAGIC_RAW));

    /*
     *
     */
    rb_define_const(rb_cMagic, "ERROR", INT2NUM(MAGIC_ERROR));

    /*
     *
     */
    rb_define_const(rb_cMagic, "MIME_ENCODING", INT2NUM(MAGIC_MIME_ENCODING));

    /*
     *
     */
    rb_define_const(rb_cMagic, "MIME", INT2NUM(MAGIC_MIME));

    /*
     *
     */
    rb_define_const(rb_cMagic, "APPLE", INT2NUM(MAGIC_APPLE));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_COMPRESS", INT2NUM(MAGIC_NO_CHECK_COMPRESS));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TAR", INT2NUM(MAGIC_NO_CHECK_TAR));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_SOFT", INT2NUM(MAGIC_NO_CHECK_SOFT));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_APPTYPE", INT2NUM(MAGIC_NO_CHECK_APPTYPE));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_ELF", INT2NUM(MAGIC_NO_CHECK_ELF));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TEXT", INT2NUM(MAGIC_NO_CHECK_TEXT));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_CDF", INT2NUM(MAGIC_NO_CHECK_CDF));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TOKENS", INT2NUM(MAGIC_NO_CHECK_TOKENS));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_ENCODING", INT2NUM(MAGIC_NO_CHECK_ENCODING));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_BUILTIN", INT2NUM(MAGIC_NO_CHECK_BUILTIN));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_ASCII", INT2NUM(MAGIC_NO_CHECK_ASCII));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_FORTRAN", INT2NUM(MAGIC_NO_CHECK_FORTRAN));

    /*
     *
     */
    rb_define_const(rb_cMagic, "NO_CHECK_TROFF", INT2NUM(MAGIC_NO_CHECK_TROFF));
}

/* vim: set ts=8 sw=4 sts=2 et : */
