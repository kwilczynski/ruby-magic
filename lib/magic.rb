# -*- encoding: utf-8 -*-

# :stopdoc:

#
# magic.rb
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

require 'magic/magic'
require 'magic/file'
require 'magic/string'
require 'magic/version'

# :startdoc:

#
#
#
class Magic
  #
  # call-seq:
  #
  # Example:
  #
  def inspect
    super.insert(-2, self.closed? ? ' (closed)' : '')
  end

  #
  # call-seq:
  #
  # Example:
  #
  def flags_array
    n, i = 0, @flags

    flags = []

    while i > 0
      n = 2**(Math.log(i) / Math.log(2)).to_i
      i = i - n
      flags.insert(0, n)
    end

    flags
  end

  #
  # call-seq:
  #
  # Example:
  #
  def version_array
    [self.version / 100, self.version % 100]
  end

  class << self
    #
    # call-seq:
    #    Magic.open( flags )                  ->
    #    Magic.open( flags ) {|magic| block } ->
    #
    # Example:
    #
    # See also: Magic::mime, Magic::type, Magic::encoding, Magic::compile and Magic::check
    #
    def open(flags = Magic::NONE)
      magic = Magic.new
      magic.flags = flags

      if block_given?
        begin
          yield magic
        ensure
          magic.close
        end
      else
        magic
      end
    end

    #
    # call-seq:
    #    Magic.mime                  ->
    #    Magic.mime {|magic| block } ->
    #
    # Example:
    #
    # See also: Magic::open, Magic::type, Magic::encoding, Magic::compile and Magic::check
    #
    def mime(&block)
      open(Magic::MIME, &block)
    end

    #
    # call-seq:
    #    Magic.type                  ->
    #    Magic.type {|magic| block } ->
    #
    # Example:
    #
    # See also: Magic::open, Magic::mime, Magic::encoding, Magic::compile and Magic::check
    #
    def type(&block)
      open(Magic::MIME_TYPE, &block)
    end

    #
    # call-seq:
    #    Magic.encoding                  ->
    #    Magic.encoding {|magic| block } ->
    #
    # Example:
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::compile and Magic::check
    #
    def encoding(&block)
      open(Magic::MIME_ENCODING, &block)
    end

    #
    # call-seq:
    #    Magic.compile( path, ... ) ->
    #    Magic.compile( array )     ->
    #
    # Example:
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, and Magic::check
    #
    def compile(path)
      open {|m| m.compile(path) }
    end

    #
    # call-seq:
    #    Magic.check( path, ... ) ->
    #    Magic.check( array )     ->
    #
    # Example:
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::encoding and Magic::compile
    #
    def check(path)
      open {|m| m.check(path) }
    end
  end

  alias_method :flags_ary,   :flags_array
  alias_method :version_ary, :version_array
end

# :enddoc:

FileMagic = Magic

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
