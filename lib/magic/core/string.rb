# frozen_string_literal: true

class String
  #
  # call-seq:
  #    string.magic -> string or array
  #
  # See also: String#mime and String#type
  #
  def magic(flags = Magic::NONE)
    Magic.open(flags) {|mgc| mgc.buffer(self) }
  end

  #
  # call-seq:
  #    string.mime -> string or array
  #
  # See also: String#magic and String#type
  #
  def mime
    magic(Magic::MIME)
  end

  #
  # call-seq:
  #    string.type -> string or array
  #
  # See also: String#magic and String#mime
  #
  def type
    magic(Magic::MIME_TYPE)
  end
end
