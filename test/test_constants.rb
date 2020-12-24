# frozen_string_literal: true

require 'test/unit'
require 'magic'

class MagicConstantsTest < Test::Unit::TestCase
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
      :NO_CHECK_CSV,
      :NO_CHECK_TOKENS,
      :NO_CHECK_ENCODING,
      :NO_CHECK_JSON,
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

  def test_NO_CHECK_ASCII_flag_constant
    assert_equal(Magic::NO_CHECK_ASCII, Magic::NO_CHECK_TEXT)
  end

  def test_NO_CHECK_BUILTIN_flag_constat
    expected = 0

    # A flag with value of -1 indicates that the underlying version
    # of the Magic library does not include this particular flag,
    # thus we skip such flag.
    [
      Magic::NO_CHECK_COMPRESS,
      Magic::NO_CHECK_TAR,
      Magic::NO_CHECK_APPTYPE,
      Magic::NO_CHECK_ELF,
      Magic::NO_CHECK_TEXT,
      Magic::NO_CHECK_CSV,
      Magic::NO_CHECK_CDF,
      Magic::NO_CHECK_TOKENS,
      Magic::NO_CHECK_ENCODING,
      Magic::NO_CHECK_JSON
    ].each {|i| expected |= i if i > -1 }

    assert_equal(Magic::NO_CHECK_BUILTIN, expected)
  end

  def test_PARAM_INDIR_MAX_parameter_constant
    assert_equal(Magic::PARAM_INDIR_MAX, 0)
  end

  def test_PARAM_NAME_MAX_parameter_constant
    assert_equal(Magic::PARAM_NAME_MAX, 1)
  end

  def test_PARAM_ELF_PHNUM_MAX_parameter_constant
    assert_equal(Magic::PARAM_ELF_PHNUM_MAX, 2)
  end

  def test_PARAM_ELF_SHNUM_MAX_parameter_constant
    assert_equal(Magic::PARAM_ELF_SHNUM_MAX, 3)
  end

  def test_PARAM_ELF_NOTES_MAX_parameter_constant
    assert_equal(Magic::PARAM_ELF_NOTES_MAX, 4)
  end

  def test_PARAM_REGEX_MAX_parameter_constant
    assert_equal(Magic::PARAM_REGEX_MAX, 5)
  end

  def test_PARAM_BYTES_MAX_parameter_constant
    assert_equal(Magic::PARAM_BYTES_MAX, 6)
  end
end
