# frozen_string_literal: true

require 'test/unit'
require 'magic'

class MagicConstantsTest < Test::Unit::TestCase
  def setup
    @version = Magic.version rescue nil
    @magic_parameters = @version && @version > 520
  end

  def test_flags_constants_defiend
    [
      :NONE,
      :DEBUG,
      :SYMLINK,
      :COMPRESS,
      :DEVICES,
      :MIME_TYPE,
      :CONTINUE,
      :CHECK,
      :PRESERVE_ATIME,
      :RAW,
      :ERROR,
      :MIME_ENCODING,
      :MIME,
      :APPLE,
      :NO_CHECK_COMPRESS,
      :NO_CHECK_TAR,
      :NO_CHECK_SOFT,
      :NO_CHECK_APPTYPE,
      :NO_CHECK_ELF,
      :NO_CHECK_TEXT,
      :NO_CHECK_CDF,
      :NO_CHECK_TOKENS,
      :NO_CHECK_ENCODING,
      :NO_CHECK_BUILTIN,
      :NO_CHECK_ASCII,
      :NO_CHECK_FORTRAN,
      :NO_CHECK_TROFF,
      :EXTENSION,
      :COMPRESS_TRANSP
    ].each {|i| assert_const_defined(Magic, i) }
  end

  def test_parameters_constants_defiend
    [
      :PARAM_INDIR_MAX,
      :PARAM_NAME_MAX,
      :PARAM_ELF_PHNUM_MAX,
      :PARAM_ELF_SHNUM_MAX,
      :PARAM_ELF_NOTES_MAX,
      :PARAM_REGEX_MAX,
      :PARAM_BYTES_MAX
    ].each {|i| assert_const_defined(Magic, i) }
  end

  def test_MIME_flag_constant
    assert_equal(Magic::MIME, Magic::MIME_TYPE | Magic::MIME_ENCODING)
  end

  def test_NO_CHECK_BUILTIN_flag_constat
    # Any recent version of libmagic will have 0x37b000 by default.
    custom_NO_CHECK_BUILTIN = Magic::NO_CHECK_COMPRESS | Magic::NO_CHECK_TAR |
      Magic::NO_CHECK_APPTYPE | Magic::NO_CHECK_ELF | Magic::NO_CHECK_TEXT |
      Magic::NO_CHECK_CDF | Magic::NO_CHECK_TOKENS | Magic::NO_CHECK_ENCODING

    # Older versions of libmagic will have 0x3fb000 here historically ...
    if @version.nil? && Magic::NO_CHECK_BUILTIN != 0x37b000
      custom_NO_CHECK_BUILTIN ^= 0x080000 # 0x37b000 ^ 0x080000 is 0x3fb000
    end

    # Latest version of libmagic have 0x77b000 by default.
    if @version && @version > 533
		  custom_NO_CHECK_BUILTIN ^= 0x0400000 # 0x37b000 ^ 0x040000 is 0x77b000
    end

    assert_equal(Magic::NO_CHECK_BUILTIN, custom_NO_CHECK_BUILTIN)
  end

  def check_NO_CHECK_ASCII_flag_constant
    assert_equal(Magic::NO_CHECK_ASCII, Magic::NO_CHECK_TEXT)
  end

  def check_PARAM_INDIR_MAX_parameter_constant
    unless @magic_parameters && Magic::PARAM_INDIR_MAX > -1
      omit('Magic library is too old')
    end

    assert_equal(Magic::PARAM_INDIR_MAX, 0)
  end

  def check_PARAM_BYTES_MAX_parameter_constant
    unless @magic_parameters && Magic::PARAM_BYTES_MAX > -1
      omit('Magic library is too old')
    end

    assert_equal(Magic::PARAM_BYTES_MAX, 6)
  end
end
