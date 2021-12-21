# frozen_string_literal: true

begin
  ::RUBY_VERSION =~ /(\d+\.\d+)/
  require_relative "magic/#{Regexp.last_match(1)}/magic"
rescue LoadError
  # use "require" instead of "require_relative" because non-native gems will place C extension files
  # in Gem::BasicSpecification#extension_dir after compilation (during normal installation), which
  # is in $LOAD_PATH but not necessarily relative to this file (see #21 and nokogiri#2300)
  require 'magic/magic'
end

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
  # Example:
  #
  #    magic = Magic.new
  #    magic.inspect     #=> "#<Magic:0x007fd5258a1108 @flags=0, @paths=[\"/etc/magic\", \"/usr/share/misc/magic\"]>"
  #    magic.close
  #    magic.inspect     #=> "#<Magic:0x007fd5258a1108 @flags=0, @paths=[\"/etc/magic\", \"/usr/share/misc/magic\"] (closed)>"
  #
  def inspect
    super.insert(-2, self.closed? ? ' (closed)' : '')
  end

  #
  # call-seq:
  #    magic.flags_list( boolean ) -> array
  #
  # See also: Magic#flags and Magic#flags_names
  #
  def flags_list(names = false)
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

  alias_method :flags_to_a, :flags_list

  # call-seq:
  #    magic.flags_names -> array
  #
  # See also: Magic#flags and Magic#flags_list
  #
  def flags_names
    flags_list(true)
  end

  class << self
    #
    # call-seq:
    #    Magic.open( integer )                  -> self
    #    Magic.open( integer ) {|magic| block } -> string or array
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
    # See also: Magic::open, Magic::type, Magic::encoding, Magic::compile and Magic::check
    #
    def mime(&block)
      open(Magic::MIME, &block)
    end

    alias_method :mime_type, :mime

    #
    # call-seq:
    #    Magic.type                  -> self
    #    Magic.type {|magic| block } -> string or array
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
    # See also: Magic::open, Magic::mime, Magic::type, Magic::compile and Magic::check
    #
    def encoding(&block)
      open(Magic::MIME_ENCODING, &block)
    end

    #
    # call-seq:
    #    Magic.compile( string, ... ) -> true
    #    Magic.compile( array )       -> true
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::encoding, and Magic::check
    #
    def compile(*paths)
      open {|m| m.compile(paths) }
    end

    #
    # call-seq:
    #    Magic.check( string, ... ) -> true or false
    #    Magic.check( array )       -> true or false
    #
    # See also: Magic::open, Magic::mime, Magic::type, Magic::encoding and Magic::compile
    #
    def check(*paths)
      open {|m| m.check(paths) }
    end

    #
    # call-seq:
    #    Magic.file( object )          -> string or array
    #    Magic.file( string )          -> string or array
    #    Magic.file( string, integer ) -> string or array
    #
    # See also:
    #
    def file(path, flags = Magic::MIME)
      open(flags).file(path)
    end

    #
    # call-seq:
    #    Magic.buffer( string )          -> string or array
    #    Magic.buffer( string, integer ) -> string or array
    #
    # See also:
    #
    def buffer(buffer, flags = Magic::MIME)
      open(flags).buffer(buffer)
    end

    #
    # call-seq:
    #    Magic.descriptor( object )           -> string or array
    #    Magic.descriptor( integer )          -> string or array
    #    Magic.descriptor( integer, integer ) -> string or array
    #
    # See also:
    #
    def descriptor(fd, flags = Magic::MIME)
      open(flags).descriptor(fd)
    end

    alias_method :fd, :descriptor

    private

    def default_paths
      paths = Dir.glob(File.expand_path(File.join(File.dirname(__FILE__), "../ext/magic/share/*.mgc")))
      paths.empty? ? nil : paths
    end
  end

  private

  def power_of_two?(number)
    number > 0 && Math.log2(number) % 1 == 0
  end

  def flags_as_map
    klass = self.class

    klass.constants.each_with_object({}) do |constant, flags|
      constant = constant.to_s

      next if constant.start_with?('PARAM_')

      value = klass.const_get(constant)

      if value.is_a?(Integer) && power_of_two?(value)
        flags[value] = constant
      end
    end
  end
end

FileMagic = Magic
