# -*- encoding: utf-8 -*-

# :stopdoc:

#
# string.rb
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
class String
  #
  # call-seq:
  #    string.magic -> string, array or nil
  #
  # Returns
  #
  # Example:
  #
  # See also: String#mime and String#type
  #
  def magic(flags = Magic::NONE)
    Magic.open(flags) {|mgc| mgc.buffer(self) }
  rescue Magic::Error
  end

  #
  # call-seq:
  #    string.mime -> string, array or nil
  #
  # Returns
  #
  # Example:
  #
  # See also: String#magic and String#type
  #
  def mime
    magic(Magic::MIME)
  end

  #
  # call-seq:
  #    string.type -> string, array or nil
  #
  # Returns
  #
  # Example:
  #
  # See also: String#magic and String#mime
  #
  def type
    magic(Magic::MIME_TYPE)
  end
end

# :enddoc:

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
