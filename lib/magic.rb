# frozen_string_literal: true

require_relative 'magic/magic'
require_relative 'magic/version'
require_relative 'magic/core/file'
require_relative 'magic/core/string'

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
  #    magic = Magic.new    #=> #<Magic:0x007fd5258a1108>
  #    magic.inspect        #=> "#<Magic:0x007fd5258a1108 @flags=0, @paths=[\"/etc/magic\", \"/usr/share/misc/magic\"]>"
  #    magic.close
  #    magic.inspect        #=> "#<Magic:0x007fd5258a1108 @flags=0, @paths=[\"/etc/magic\", \"/usr/share/misc/magic\"] (closed)>"
  #
  def inspect
    super.insert(-2, self.closed? ? ' (closed)' : '')
  end

  #
  # call-seq:
  #    magic.fd( object )  -> string
  #    magic.fd( integer ) -> string
  #
  # Returns
  #
  # Example:
  #
  # Will raise <i>Magic::LibraryError</i> exception if, or
  #
  # See also: Magic#descriptor, Magic#buffer and Magic#file
  #
  def fd(fd)
    descriptor(fd.respond_to?(:fileno) ? fd.fileno : fd)
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

    #
    # call-seq:
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also:
    #
    def file(path, flags = Magic::MIME)
      open(flags).file(path)
    end

    #
    # call-seq:
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also:
    #
    def buffer(buffer, flags = Magic::MIME)
      open(flags).buffer(buffer)
    end

    #
    # call-seq:
    #
    # Returns
    #
    # Example:
    #
    # Will raise <i></i>
    #
    # See also:
    #
    def descriptor(fd, flags = Magic::MIME)
      open(flags).fd(fd)
    end

    alias_method :fd, :descriptor
  end

  alias_method :flags_array, :flags_to_a

  private

  def flags_as_map
    klass = self.class

    klass.constants.each_with_object({}) do |constant, flags|
      next if constant.start_with?('PARAM_')

      value = klass.const_get(constant)

      if value.is_a?(Integer) && (value & (value - 1)) == 0
        flags[value] = constant.to_s
      end
    end
  end
end

FileMagic = Magic
