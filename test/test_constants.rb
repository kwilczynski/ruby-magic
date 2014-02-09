# -*- encoding: utf-8 -*-

# :enddoc:

#
# test_constants.rb
#
# Copyright 2013 Krzysztof Wilczynski
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

begin
  require 'coveralls'
  Coveralls.wear!
rescue LoadError
  STDERR.puts 'The Coveralls gem is not installed, skipping ...'
end

gem 'test-unit', '>= 2.5.2'

require 'test/unit'
require 'magic'

class MagicConstantsTest < Test::Unit::TestCase
  def setup
  end

  def teardown
  end

  def test_constants
  end
end

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
