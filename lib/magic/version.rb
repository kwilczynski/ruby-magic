# frozen_string_literal: true

class Magic
  #
  # Current version of _Magic_.
  #
  VERSION = '0.6.0'.freeze

  class << self
    #
    # call-seq:
    #    Magic.version_array -> array
    #
    # Example:
    #
    #    Magic.version_array #=> [5, 42]
    #
    # See also: Magic::version and Magic::version_string
    #
    def version_array
      [self.version / 100, self.version % 100]
    end

    alias_method :version_to_a, :version_array

    #
    # call-seq:
    #    Magic.version_string -> string
    #
    # Example:
    #
    #    Magic.version_string #=> "5.42"
    #
    # See also: Magic::version and Magic::version_array
    #
    def version_string
      '%d.%02d' % self.version_array
    end

    alias_method :version_to_s, :version_string
  end
end
