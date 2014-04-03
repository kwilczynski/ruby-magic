# -*- encoding: utf-8 -*-

# :enddoc:

#
# ruby-magic.gemspec
#
# Copyright 2013-2014 Krzysztof Wilczynski
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

signing_key = File.expand_path('~/.gem/kwilczynski-private.pem')

Gem::Specification.new do |s|
  s.name    = 'ruby-magic'
  s.summary = 'File Magic in Ruby'

  s.description = <<-EOS
File Magic in Ruby.

Simple interface to libmagic for Ruby Programming Language.
  EOS

  s.platform = Gem::Platform::RUBY
  s.version  = File.read('VERSION').strip
  s.license  = 'Apache License, Version 2.0'
  s.author   = 'Krzysztof Wilczynski'
  s.email    = 'krzysztof.wilczynski@linux.com'
  s.homepage = 'http://about.me/kwilczynski'
  s.has_rdoc = true

  s.required_ruby_version = '>= 1.9.2'
  s.rubygems_version      = '~> 2.2.0'

  s.files = Dir['ext/**/*.{c,h,rb}'] +
            Dir['lib/**/*.rb']       +
            Dir['benchmark/**/*.rb'] +
            Dir['test/**/*.rb']      +
            %w(Rakefile ruby-magic.gemspec AUTHORS
               CHANGES CHANGES.rdoc COPYRIGHT LICENSE
               README README.rdoc TODO VERSION)

  s.executables   << 'magic'
  s.require_paths << 'lib'
  s.extensions    << 'ext/magic/extconf.rb'

  s.signing_key = signing_key if File.exists?(signing_key)
end

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
