# -*- encoding: utf-8 -*-

# :enddoc:

#
# Rakefile
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

gem 'rdoc', '>= 4.1.1'

begin
  require 'rake'
  require 'rdoc/task'
  require 'rake/testtask'
  require 'rake/extensiontask'
rescue LoadError
  require 'rubygems'
  require 'rake'
  require 'rdoc/task'
  require 'rake/testtask'
  require 'rake/extensiontask'
end

CLEAN.include   '*{.h,.o,.log,.so}', 'ext/**/*{.o,.log,.so}', 'Makefile', 'ext/**/Makefile'
CLOBBER.include 'lib/**/*.so', 'doc/**/*'

gem = eval File.read('ruby-magic.gemspec')

RDoc::Task.new do |d|
  files = %w(AUTHORS CHANGES.rdoc COPYRIGHT LICENSE README.rdoc TODO)

  d.title = 'File Magic in Ruby'
  d.main  = 'README.rdoc'

  d.rdoc_dir = 'doc/rdoc'

  d.rdoc_files.include 'ext/**/*.{c,h}', 'lib/**/*.rb'
  d.rdoc_files.include.add(files)

  d.options << '--line-numbers'
end

Rake::TestTask.new do |t|
  t.verbose    = true
  t.warning    = true
  t.test_files = Dir['test/**/test_*']
end

Gem::PackageTask.new(gem) do |p|
  p.need_zip = true
  p.need_tar = true
end

Rake::ExtensionTask.new('magic', gem) do |e|
  e.ext_dir = 'ext/magic'
  e.lib_dir = 'lib/magic'
end

Rake::Task[:test].prerequisites << :clobber
Rake::Task[:test].prerequisites << :compile

task :default => :test

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
