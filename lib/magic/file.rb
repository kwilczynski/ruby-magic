# -*- encoding: utf-8 -*-

# :stopdoc:

#
# file.rb
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

# :startdoc:

#
#
#
class File
  class << self
    #
    # call-seq:
    #    File.magic( path ) -> string, array or nil
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

      Magic.open(flags) {|mgc| mgc.file(path) }
    rescue Magic::Error
    end

    #
    # call-seq:
    #    File.mime( path ) -> string, array or nil
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
    #    File.type( path ) -> string, array or nil
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
  #    File.magic( path ) -> string, array or nil
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
  #    File.mime( path ) -> string, array or nil
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
  #    File.type( path ) -> string, array or nil
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

# :enddoc:

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
