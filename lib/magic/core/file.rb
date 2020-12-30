# frozen_string_literal: true

class File
  class << self
    #
    # call-seq:
    #    File.magic( object ) -> string or array
    #    File.magic( string ) -> string or array
    #
    # See also: File::mime and File::type
    #
    def magic(path, flags = Magic::NONE)
      Magic.open(flags) {|mgc| mgc.file(path) }
    end

    #
    # call-seq:
    #    File.mime( string ) -> string or array
    #
    # See also: File::magic and File::type
    #
    def mime(path)
      magic(path, Magic::MIME)
    end

    #
    # call-seq:
    #    File.type( string ) -> string or array
    #
    # See also: File::magic and File::mime
    #
    def type(path)
      magic(path, Magic::MIME_TYPE)
    end
  end

  #
  # call-seq:
  #    File.magic -> string or array
  #
  # See also: File#mime and File#type
  #
  def magic
    self.class.magic(self)
  end

  #
  # call-seq:
  #    File.mime -> string or array
  #
  # See also: File#magic and File#type
  #
  def mime
    self.class.mime(self)
  end

  #
  # call-seq:
  #    File.type -> string or array
  #
  # See also: File#magic and File#mime
  #
  def type
    self.class.type(self)
  end
end
