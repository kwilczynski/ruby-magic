# frozen_string_literal: true

#
#
#
class Magic
  #
  # Current version of _Magic_.
  #
  VERSION = '0.2.0'.freeze

  class << self
    #
    # call-seq:
    #    Magic.version_to_a -> array
    #
    # Returns
    #
    # Example:
    #
    #    Magic.version_to_a   #=> [5, 17]
    #
    # Will raise <i>Magic::NotImplementedError</i> exception if, or
    #
    # See also: Magic::version_to_s
    #
    def version_to_a
      [self.version / 100, self.version % 100]
    end

    #
    # call-seq:
    #    Magic.version_to_s -> string
    #
    # Returns
    #
    # Example:
    #
    #    Magic.version_to_s   #=> "5.17"
    #
    # Will raise <i>Magic::NotImplementedError</i> exception if, or
    #
    # See also: Magic::version_to_a
    #
    def version_to_s
      '%d.%02d' % self.version_to_a
    end

    alias_method :version_array,  :version_to_a
    alias_method :version_string, :version_to_s
  end
end
