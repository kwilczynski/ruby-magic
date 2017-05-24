#
#
#
class File
  class << self
    #
    # call-seq:
    #    File.magic( path ) -> string or array
    #
    # Returns
    #
    # Example:
    #
    # See also: File::mime and File::type
    #
    def magic(path, flags = Magic::NONE)
      path   = path.path    if path.respond_to?(:path)
      path ||= path.to_path if path.respond_to?(:to_path)
      path ||= path.to_s

      Magic.open(flags | Magic::ERROR) {|mgc| mgc.file(path) }
    end

    #
    # call-seq:
    #    File.mime( path ) -> string or array
    #
    # Returns
    #
    # Example:
    #
    # See also: File::magic and File::type
    #
    def mime(path)
      magic(path, Magic::MIME)
    end

    #
    # call-seq:
    #    File.type( path ) -> string or array
    #
    # Returns
    #
    # Example:
    #
    # See also: File::magic and File::mime
    #
    def type(path)
      magic(path, Magic::MIME_TYPE)
    end
  end

  #
  # call-seq:
  #    File.magic( path ) -> string or array
  #
  # Returns
  #
  # Example:
  #
  # See also: File#mime and File#type
  #
  def magic
    self.class.magic(self)
  end

  #
  # call-seq:
  #    File.mime( path ) -> string or array
  #
  # Returns
  #
  # Example:
  #
  # See also: File#magic and File#type
  #
  def mime
    self.class.mime(self)
  end

  #
  # call-seq:
  #    File.type( path ) -> string or array
  #
  # Returns
  #
  # Example:
  #
  # See also: File#magic and File#mime
  #
  def type
    self.class.type(self)
  end
end
