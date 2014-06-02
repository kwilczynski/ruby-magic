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
    @version = begin
      Magic.version
    rescue Magic::NotImplementedError
      # Do not care.
    end
  end


  def test_constants_defiend
    # Sorted in an ascending order by value.
    [
      :NONE,
      :NO_CHECK_FORTRAN,
      :NO_CHECK_TROFF,
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
      :NO_CHECK_ASCII,
      :NO_CHECK_CDF,
      :NO_CHECK_TOKENS,
      :NO_CHECK_ENCODING,
      :NO_CHECK_BUILTIN
    ].each {|i| assert_const_defined(Magic, i) }
  end

  def test_constants_values
    # Any recent version of libmagic will have 0x37b000 by default.
    custom_NO_CHECK_BUILTIN = Magic::NO_CHECK_COMPRESS | Magic::NO_CHECK_TAR |
      Magic::NO_CHECK_APPTYPE | Magic::NO_CHECK_ELF | Magic::NO_CHECK_TEXT |
      Magic::NO_CHECK_CDF | Magic::NO_CHECK_TOKENS | Magic::NO_CHECK_ENCODING

    # Older versions of libmagic will have 0x3fb000 here historically ...
    if @version.nil? && Magic::NO_CHECK_BUILTIN != 0x37b000
      custom_NO_CHECK_BUILTIN ^= 0x080000 # 0x37b000 ^ 0x080000 is 0x3fb000
    end

    # Check if underlaying constants coming from libmagic are sane.
    [{
       :given    => Magic::MIME,
       :expected => Magic::MIME_TYPE | Magic::MIME_ENCODING,
     },
     {
       :given    => Magic::NO_CHECK_BUILTIN,
       :expected => custom_NO_CHECK_BUILTIN,
     },
     {
       :given    => Magic::NO_CHECK_ASCII,
       :expected => Magic::NO_CHECK_TEXT,
     }].each {|t| assert_equal(t[:given], t[:expected]) }
  end
end

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
