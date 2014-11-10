# -*- encoding: utf-8 -*-

# :enddoc:

#
# test_magic.rb
#
# Copyright 2013-2014 Krzysztof Wilczynski
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

begin
  require 'coveralls'
  Coveralls.wear!
rescue LoadError
  STDERR.puts 'The Coveralls gem is not installed, skipping ...'
end

gem 'test-unit', '>= 2.5.2'

require 'test/unit'
require "mocha/test_unit"
require 'magic'

require_relative 'helpers/magic_test_helper'

class MagicTest < Test::Unit::TestCase
  include MagicTestHelpers

  def setup
    @magic = Magic.new
    @version = Magic.version rescue nil
  end

  def test_magic_alias
    assert_same(FileMagic, Magic)
  end

  def test_magic_flags_alias
    assert_alias_method(@magic, :flags_array, :flags_to_a)
  end

  def test_magic_check_alias
    assert_alias_method(@magic, :valid?, :check)
  end

  def test_magic_version_to_a_alias
    assert_alias_method(Magic, :version_array, :version_to_a)
  end

  def test_magic_version_to_s_alias
    assert_alias_method(Magic, :version_string, :version_to_s)
  end

  def test_magic_singleton_methods
    [
      :open,
      :mime,
      :type,
      :encoding,
      :compile,
      :check,
      :version,
      :version_to_a,
      :version_to_s
    ].each {|i| assert_respond_to(Magic, i) }
  end

  def test_magic_new_instance
    assert(@magic.class == Magic)
  end

  def test_magic_new_instance_default_flags
    assert_equal(@magic.flags, 0)
  end

  def test_magic_new_with_block
    magic, output = capture_stderr do
      Magic.new {}
    end

    expected = "warning: Magic::new() does not take block; use Magic::open() instead\n"
    assert_kind_of(Magic, magic)
    assert_equal(output.split(/\.rb:\d+\:\s+?/).pop, expected)
  end

  def test_magic_instance_methods
    [
      :close,
      :closed?,
      :path,
      :flags,
      :flags_array,
      :file,
      :buffer,
      :descriptor,
      :load,
      :compile,
      :check
    ].each {|i| assert_respond_to(@magic, i) }
  end

  def test_magic_string_integration_methods
    [
      :magic,
      :mime,
      :type
    ].each {|i| assert_respond_to(String.allocate, i) }
  end

  def test_magic_file_integration_singleton_methods
    [
      :magic,
      :mime,
      :type
    ].each {|i| assert_respond_to(File, i) }
  end

  def test_magic_file_integration_instance_methods
    [
      :magic,
      :mime,
      :type
    ].each {|i| assert_respond_to(File.allocate, i) }
  end

  def test_magic_close
    @magic.close
    assert_true(@magic.closed?)
  end

  def test_magic_close_twice
    assert_nothing_raised do
      @magic.close
      @magic.close
    end
  end

  def test_magic_close_error
    @magic.close
    assert_raise Magic::LibraryError do
      @magic.flags
    end
  end

  def test_magic_close_error_message
    @magic.close
    @magic.flags
  rescue Magic::LibraryError => error
    assert_equal(error.message, 'Magic library is not open')
  end

  def test_magic_closed?
    assert_false(@magic.closed?)
    @magic.close
    assert_true(@magic.closed?)
  end

  def test_magic_inspect_when_magic_open
    assert_match(%r{^#<Magic:0x.+>$}, @magic.inspect)
  end

  def test_magic_inspect_when_magic_closed
    @magic.close
    assert_match(%r{^#<Magic:0x.+ \(closed\)>$}, @magic.inspect)
  end

  def test_magic_path
    assert_kind_of(Array, @magic.path)
    assert_not_equal(@magic.path.size, 0)
  end

  def test_magic_path_with_MAGIC_environment_variable
    # XXX(krzysztof): How to override "MAGIC" environment
    # variable so that the C extension will pick it up?
  end

  def test_magic_flags_with_NONE_flag
    @magic.flags = 0x000000 # Flag: NONE
    assert_kind_of(Fixnum, @magic.flags)
    assert_equal(@magic.flags, Magic::NONE)
  end

  def test_magic_flags_with_MIME_TYPE_flag
    @magic.flags = 0x000010 # Flag: MIME_TYPE
    assert_kind_of(Fixnum, @magic.flags)
    assert_equal(@magic.flags, Magic::MIME_TYPE)
  end

  def test_magic_flags_with_MIME_ENCODING_flag
    @magic.flags = 0x000400 # Flag: MIME_ENCODING
    assert_kind_of(Fixnum, @magic.flags)
    assert_equal(@magic.flags, Magic::MIME_ENCODING)
  end

  def test_magic_flags_with_MIME_flag
    @magic.flags = 0x000410 # Flag: MIME_TYPE, MIME_ENCODING
    assert_kind_of(Fixnum, @magic.flags)
    assert_equal(@magic.flags, Magic::MIME)
  end

  def test_magic_flags_to_a_with_NONE_flag
    @magic.flags = 0x000000 # Flag: NONE
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a, [Magic::NONE])
  end

  def test_magic_flags_to_a_with_MIME_TYPE_flag
    @magic.flags = 0x000010 # Flag: MIME_TYPE
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a, [Magic::MIME_TYPE])
  end

  def test_magic_flags_to_a_with_MIME_ENCODING_flag
    @magic.flags = 0x000400 # Flag: MIME_ENCODING
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a, [Magic::MIME_ENCODING])
  end

  def test_magic_flags_to_a_with_MIME_flag
    @magic.flags = 0x000410 # Flag: MIME_TYPE, MIME_ENCODING
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a, [Magic::MIME_TYPE, Magic::MIME_ENCODING])
  end

  def test_magic_flags_to_a_with_NONE_flag_and_argument_true
    @magic.flags = 0x000000 # Flag: NONE
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a(true), ['NONE'])
  end

  def test_magic_flags_to_a_with_MIME_TYPE_flag_and_argument_true
    @magic.flags = 0x000010 # Flag: MIME_TYPE
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a(true), ['MIME_TYPE'])
  end

  def test_magic_flags_to_a_with_MIME_ENCODING_flag_and_argument_true
    @magic.flags = 0x000400 # Flag: MIME_ENCODING
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a(true), ['MIME_ENCODING'])
  end

  def test_magic_flags_to_a_with_MIME_flag_and_argument_true
    @magic.flags = 0x000410 # Flag: MIME_TYPE, MIME_ENCODING
    assert_kind_of(Array, @magic.flags_to_a)
    assert_equal(@magic.flags_to_a(true), ['MIME_TYPE', 'MIME_ENCODING'])
  end

  def test_magic_flags_error_lower_boundary
    assert_raise Magic::FlagsError do
      @magic.flags = -0xffffff
    end
  end

  def test_magic_flags_error_upper_boundary
    assert_raise Magic::FlagsError do
      @magic.flags = 0xffffff
    end
  end

  def test_magic_flags_error_message
    @magic.flags = 0xffffff
  rescue Magic::FlagsError => error
    assert_equal(error.message, 'unknown or invalid flag specified')
  end

  def test_magic_file
  end

  def test_magic_file_with_ERROR_flag
  end

  def test_magic_file_with_MAGIC_CONTINUE_flag
  end

  def test_magic_buffer
  end

  def test_magic_buffer_with_MAGIC_CONTINUE_flag
  end

  def test_magic_descriptor
  end

  def test_magic_descriptor_with_MAGIC_CONTINUE_flag
  end

  def test_magic_load
  end

  def test_magic_load_with_custom_Magic_file_path
  end

  def test_magic_with_new_instance_custom_Magic_file_path
  end

  def test_magic_load_with_MAGIC_environment_variable
  end

  def test_magic_check
  end

  def test_magic_compile
  end

  def test_magic_version
    if @version.nil? || @version < 0
      Magic.stubs(:version).returns(518)
    end

    assert_kind_of(Fixnum, Magic.version)
    assert(Magic.version > 0)
  end

  def test_magic_version_error
    if @version
      Magic.stubs(:version).raises(Magic::NotImplementedError)
    end

    assert_raise Magic::NotImplementedError do
      Magic.version
    end
  end

  def test_magic_version_error_message
    if @version
      Magic.stubs(:version).raises(Magic::NotImplementedError, 'function is not implemented')
    end

    Magic.version
  rescue Magic::NotImplementedError => error
    assert_equal(error.message, 'function is not implemented')
  end

  def test_magic_version_to_a
    if @version.nil? || @version < 0
      Magic.stubs(:version).returns(518)
    end

    expected = [Magic.version / 100, Magic.version % 100]
    assert_equal(Magic.version_to_a, expected)
  end

  def test_magic_version_to_s
    if @version.nil? || @version < 0
      Magic.stubs(:version).returns(518)
    end

    expected = '%d.%02d' % Magic.version_to_a
    assert_equal(Magic.version_to_s, expected)
  end

  def test_magic_singleton_open
  end

  def test_magic_singleton_open_custom_flag
  end

  def test_magic_singleton_open_block
  end

  def test_magic_singleton_open_block_custom_flag
  end

  def test_magic_singleton_mime
  end

  def test_magic_singleton_mime_block
  end

  def test_magic_singleton_type
  end

  def test_magic_singleton_type_block
  end

  def test_magic_singleton_encoding
  end

  def test_magic_singleton_encoding_block
  end

  def test_magic_singleton_compile
  end

  def test_magic_singleton_check
  end

  def test_magic_magic_error
    message = 'The quick brown fox jumps over the lazy dog'
    error = Magic::Error.new(message)

    assert_respond_to(error, :errno)
    assert_equal(error.message, message)
    assert_nil(error.errno)
  end

  def test_magic_error_attribute_errno
    @magic.flags = 0xffffff # Should raise Magic::FlagsError.
  rescue Magic::Error => error
    assert_kind_of(Magic::FlagsError, error)
    assert_equal(error.errno, Errno::EINVAL::Errno)
  end

  def test_magic_new_instance_with_arguments
  end

  def test_file_integration_magic
  end

  def test_file_integration_magic_with_custom_flag
  end

  def test_file_integration_mime
  end

  def test_file_integration_type
  end

  def test_file_integration_singleton_magic
  end

  def test_file_integration_singleton_magic_with_custom_flag
  end

  def test_file_integration_singleton_mime
  end

  def test_file_integration_singleton_type
  end

  def test_string_integration_magic
  end

  def test_string_integration_magic_with_custom_flag
  end

  def test_string_integration_mime
  end

  def test_string_integration_type
  end

  def test_magic_mutex_unlocked
  end
end

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
