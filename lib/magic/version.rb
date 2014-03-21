# -*- encoding: utf-8 -*-

# :stopdoc:

#
# version.rb
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

class Magic
  #
  # Current version of _Magic_.
  #
  VERSION = '0.0.1'

  class << self
    #
    # call-seq:
    #
    # Example:
    #
    def version_to_a
      [self.version / 100, self.version % 100]
    end

    #
    # call-seq:
    #
    # Example:
    #
    def version_to_s
      '%d.%02d' % self.version_to_a
    end

    alias_method :version_array,  :version_to_a
    alias_method :version_string, :version_to_s
  end
end

# :enddoc:

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
