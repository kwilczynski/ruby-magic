# -*- encoding: utf-8 -*-

# :stopdoc:

#
# magic.rb
#
# Copyright 2013-2015 Krzysztof Wilczynski
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

require_relative 'magic/magic'
require_relative 'magic/version'
require_relative 'magic/core/file'
require_relative 'magic/core/string'

# :startdoc:

#
# File _Magic_ in Ruby.
#
# Simple interface to _libmagic_ for Ruby Programming Language.
#
class Magic
  #
  # call-seq:
  #    magic.inspect -> string
  #
  # Returns
  #
  # Example:
  #
  #    magic = Magic.new   #=> #<Magic:0x007fd5258a1108>
  #    magic.inspect       #=> "#<Magic:0x007fd5258a1108 @flags=0, @path=[\"/etc/magic\", \"/usr/share/misc/magic\"]>"
  #
  def inspect
    super.insert(-2, self.closed? ? ' (closed)' : '')
  end

  #
  # call-seq:
  #    magic.flags_to_a( names ) -> array
  #
  # Returns an +array+
  #
  # Example:
  #
  # Will raise <i>Magic::LibraryError</i> exception if, or
  #
  # See also: Magic#flags
  #
  def flags_to_a(names = false)
    raise LibraryError, 'Magic library is not open' if closed?
    return [names ? 'NONE' : 0] if @flags.zero?

    n, i = 0, @flags
    flags = []

    @@flags_map ||= flags_as_map if names

    while i > 0
      n = 2 ** (Math.log(i) / Math.log(2)).to_i
      i = i - n
      flags.insert(0, names ? @@flags_map[n] : n)
    end

    flags
  end

  class << self
    #
    # call-seq:
    #    Magic.open( flags )                  -> self
    #    Magic.open( flags ) {|magic| block } -> string or array
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
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
    #    Magic.mime                  -> self
    #    Magic.mime {|magic| block } -> string or array
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also: Magic::open, Magic::type, Magic::encoding, Magic::compile and Magic::check
    #
    def mime(&block)
      open(Magic::MIME, &block)
    end

    #
    # call-seq:
    #    Magic.type                  -> self
    #    Magic.type {|magic| block } -> string or array
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also: Magic::open, Magic::mime, Magic::encoding, Magic::compile and Magic::check
    #
    def type(&block)
      open(Magic::MIME_TYPE, &block)
    end

    #
    # call-seq:
    #    Magic.encoding                  -> self
    #    Magic.encoding {|magic| block } -> string or array
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::compile and Magic::check
    #
    def encoding(&block)
      open(Magic::MIME_ENCODING, &block)
    end

    #
    # call-seq:
    #    Magic.compile( path, ... ) -> true
    #    Magic.compile( array )     -> true
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, and Magic::check
    #
    def compile(path)
      open {|m| m.compile(path) }
    end

    #
    # call-seq:
    #    Magic.check( path, ... ) -> true or false
    #    Magic.check( array )     -> true or false
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::encoding and Magic::compile
    #
    def check(path)
      open {|m| m.check(path) }
    end
  end

  alias_method :flags_array, :flags_to_a

  private

  def flags_as_map
    self.class.constants.inject({}) do |flags,constant|
      value = self.class.const_get(constant)
      flags[value] = constant.to_s if value.is_a?(Fixnum)
      flags
    end
  end
end

# :enddoc:

FileMagic = Magic

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
