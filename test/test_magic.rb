# -*- encoding: utf-8 -*-

# :enddoc:

#
# test_magic.rb
#
# Copyright 2013 Krzysztof Wilczynski
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
  def test_magic_alias
    assert(FileMagic == Magic)
  end

  def test_magic_singleton_methods
    mgc = Magic
    assert_block do
      DEFAULT_SINGLETON_METHODS.all? {|i| mgc.respond_to?(i) }
    end
  end

  def test_magic_new_instance
    mgc = Magic.new
    assert(mgc.class == Magic)
  end

  def test_magic_instance_methods
    mgc = Magic.new
    assert_block do
      DEFAULT_INSTANCE_METHODS.all? {|i| mgc.respond_to?(i) }
    end
  end

  def test_magic_close
    mgc = Magic.new
    mgc.close

    assert(mgc.closed?)
    assert_raise Magic::LibraryError do
      mgc.file('')
    end
  end

  def test_magic_closed?
    mgc = Magic.new
    assert_equal(mgc.closed?, false)
    mgc.close
    assert_equal(mgc.closed?, true)
  end

  def test_magic_inspect
    mgc = Magic.new
    assert(mgc.inspect.match(%r{^#<Magic:0x.+>$}))
    mgc.close
    assert(mgc.inspect.match(%r{^#<Magic:0x.+ \(closed\)>$}))
  end

  def test_magic_path
  end

  def test_magic_flags
  end

  def test_magic_flags_to_a
  end

  def test_magic_file
  end

  def test_magic_buffer
  end

  def test_magic_descriptor
  end

  def test_magic_load
  end

  def test_magic_check
  end

  def test_magic_valid?
  end

  def test_magic_compile
  end

  def test_magic_version
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
  end

  def test_magic_magic_error
  end

  def test_magic_library_error
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
