# -*- encoding: utf-8 -*-

# :enddoc:

#
# test_constants.rb
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

class MagicConstantsTest < Test::Unit::TestCase
  def setup
    @version = Magic.version rescue nil
  end

  def test_constants_defiend
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
    ].each {|i| assert_const_defined(Magic, i) }
  end

  def test_MIME_constant
    assert_equal(Magic::MIME, Magic::MIME_TYPE | Magic::MIME_ENCODING)
  end

  def test_NO_CHECK_BUILTIN_constat
    # Any recent version of libmagic will have 0x37b000 by default.
    custom_NO_CHECK_BUILTIN = Magic::NO_CHECK_COMPRESS | Magic::NO_CHECK_TAR |
      Magic::NO_CHECK_APPTYPE | Magic::NO_CHECK_ELF | Magic::NO_CHECK_TEXT |
      Magic::NO_CHECK_CDF | Magic::NO_CHECK_TOKENS | Magic::NO_CHECK_ENCODING

    # Older versions of libmagic will have 0x3fb000 here historically ...
    if @version.nil? && Magic::NO_CHECK_BUILTIN != 0x37b000
      custom_NO_CHECK_BUILTIN ^= 0x080000 # 0x37b000 ^ 0x080000 is 0x3fb000
    end

    assert_equal(Magic::NO_CHECK_BUILTIN, custom_NO_CHECK_BUILTIN)
  end

  def check_NO_CHECK_ASCII_constant
    assert_equal(Magic::NO_CHECK_ASCII, Magic::NO_CHECK_TEXT)
  end
end

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
