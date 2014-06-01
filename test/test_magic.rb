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
require 'magic'

DEFAULT_SINGLETON_METHODS = [
  :open,
  :mime,
  :type,
  :encoding,
  :compile,
  :check,
  :version,
  :version_to_a,
  :version_to_s
]

DEFAULT_INSTANCE_METHODS = [
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
]

DEFAULT_INTEGRATION_METHODS = [
  :magic,
  :mime,
  :type
]

class MagicTest < Test::Unit::TestCase
  def setup
    @magic = Magic.new
    @version = begin
      Magic.version
    rescue Magic::NotImplementedError
    end
  end

  def test_magic_alias
    assert(FileMagic == Magic)
  end

  def test_magic_singleton_methods
    assert_block do
      DEFAULT_SINGLETON_METHODS.all? {|i| Magic.respond_to?(i) }
    end
  end

  def test_magic_new_instance
    assert(@magic.class == Magic)
  end

  def test_magic_instance_methods
    assert_block do
      DEFAULT_INSTANCE_METHODS.all? {|i| @magic.respond_to?(i) }
    end
  end

  def test_magic_close
    @magic.close
    assert_true(@magic.closed?)
  end

  def test_magic_close_error
    assert_raise Magic::LibraryError do
      @magic.close
      @magic.file('') # Should raise Magic::LibraryError.
    end
  end

  def test_magic_close_error_message
    @magic.close
    @magic.file('') # Should raise Magic::LibraryError.
  rescue Magic::LibraryError => error
    assert_equal(error.message, 'Magic library is not open')
  end

  def test_magic_closed?
    assert_false(@magic.closed?)
    @magic.close
    assert_true(@magic.closed?)
  end

  def test_magic_inspect
    assert(@magic.inspect.match(%r{^#<Magic:0x.+>$}))
    @magic.close
    assert(@magic.inspect.match(%r{^#<Magic:0x.+ \(closed\)>$}))
  end

  def test_magic_path
  end

  def test_magic_path_MAGIC_environment_variable
  end

  def test_magic_flags
    [
      {
        :given    => 0x000000, # Flag: NONE
        :expected => Magic::NONE,
      },
      {
        :given    => 0x000010, # Flag: MIME_TYPE
        :expected => Magic::MIME_TYPE,
      },
      {
        :given    => 0x000400, # Flag: MIME_ENCODING
        :expected => Magic::MIME_ENCODING,
      },
      {
        :given    => 0x000410, # Flag: MIME_TYPE, MIME_ENCODING
        :expected => Magic::MIME,
      },
    ].each do |t|
        @magic.flags = t[:given]
        obtained = @magic.flags
        assert_equal(obtained, t[:expected])
    end
  end

  def test_magic_flags_to_a
    [
      {
        :given    => 0x000000, # Flag: NONE
        :expected => [Magic::NONE],
      },
      {
        :given    => 0x000001, # Flag: DEBUG
        :expected => [Magic::DEBUG],
      },
      {
        :given    => 0x000201, # Flag: DEBUG, ERROR
        :expected => [Magic::DEBUG, Magic::ERROR],
      },
      {
        :given    => 0x000022, # Flag: SYMLINK, CONTINUE
        :expected => [Magic::SYMLINK, Magic::CONTINUE],
      },
      {
        :given    => 0x000410, # Flag: MIME_TYPE, MIME_ENCODING
        :expected => [Magic::MIME_TYPE, Magic::MIME_ENCODING],
      },
    ].each do |t|
        @magic.flags = t[:given]
        obtained = @magic.flags_to_a
        assert_equal(obtained, t[:expected])
    end
  end

  def test_magic_flags_to_a_with_boolean_argument_true
    [
      {
        :given    => 0x000000, # Flag: NONE
        :expected => ['NONE'],
      },
      {
        :given    => 0x000001, # Flag: DEBUG
        :expected => ['DEBUG'],
      },
      {
        :given    => 0x000201, # Flag: DEBUG, ERROR
        :expected => ['DEBUG', 'ERROR'],
      },
      {
        :given    => 0x000022, # Flag: SYMLINK, CONTINUE
        :expected => ['SYMLINK', 'CONTINUE'],
      },
      {
        :given    => 0x000410, # Flag: MIME_TYPE, MIME_ENCODING
        :expected => ['MIME_TYPE', 'MIME_ENCODING'],
      },
    ].each do |t|
        @magic.flags = t[:given]
        obtained = @magic.flags_to_a(true)
        assert_equal(obtained, t[:expected])
    end
  end

  def test_magic_file
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

  def test_magic_load_MAGIC_environment_variable
  end

  def test_magic_check
  end

  def test_magic_valid?
  end

  def test_magic_compile
  end

  def test_magic_version
    omit_unless(@version, 'Older version of libmagic present. Nothing to do.')

    obtained = Magic.version
    assert(obtained.class == Fixnum)
    assert(obtained > 0)
  end

  def test_magic_version_to_a
    omit_unless(@version, 'Older version of libmagic present. Nothing to do.')

    obtained = Magic.version_to_a
    expected = [Magic.version / 100, Magic.version % 100]
    assert_equal(obtained, expected)
  end

  def test_magic_version_to_s
    omit_unless(@version, 'Older version of libmagic present. Nothing to do.')

    obtained = Magic.version_to_s
    expected = '%d.%02d' % [Magic.version / 100, Magic.version % 100]
    assert_equal(obtained, expected)
  end

  def test_magic_version_error
    omit_if(@version, 'Up to date version of libmagic present. Nothing to do.')

    assert_raise Magic::NotImplementedError do
      Magic.version
    end
  end

  def test_magic_version_error_message
    omit_if(@version, 'Up to date version of libmagic present. Nothing to do.')

    Magic.version
  rescue Magic::NotImplementedError => error
    assert_equal(error.message, 'function is not implemented')
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

  def test_magic_flags_error
    # Test lower boundary limit.
    assert_raise Magic::FlagsError do
      @magic.flags = -0xffffff
    end

    # Test upper boundary limit.
    assert_raise Magic::FlagsError do
      @magic.flags = 0xffffff
    end
  end

  def test_magic_flags_error_message
    @magic.flags = 0xffffff
  rescue Magic::FlagsError => error
    assert_equal(error.message, 'unknown or invalid flag specified')
  end

  def test_magic_magic_error
  end

  def test_magic_library_error
  end

  def test_magic_new_instance_with_arguments
  end

  def test_file_integration
  end

  def test_file_integration_magic
  end

  def test_file_integration_magic_custom_flag
  end

  def test_file_integration_mime
  end

  def test_file_integration_type
  end

  def test_file_integration_singleton_magic
  end

  def test_file_integration_singleton_magic_custom_flag
  end

  def test_file_integration_singleton_mime
  end

  def test_file_integration_singleton_type
  end

  def test_string_integration
  end

  def test_string_integration_magic
  end

  def test_string_integration_magic_custom_flag
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
