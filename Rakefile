gem 'rdoc', '>= 4.1.1'

require 'rake'
require 'rake/testtask'
require 'rake/extensiontask'
require 'rdoc/task'

CLEAN.include '*{.h,.o,.log,.so}', 'ext/**/*{.o,.log,.so}', 'Makefile', 'ext/**/Makefile'
CLOBBER.include 'lib/**/*.so', 'doc/**/*'

gem = eval File.read('ruby-magic.gemspec')

RDoc::Task.new do |d|
  files = %w(AUTHORS CHANGELOG.md COPYRIGHT LICENSE README.md TODO)

  d.title = 'File Magic in Ruby'
  d.main = 'README.md'

  d.rdoc_dir = 'doc/rdoc'

  d.rdoc_files.include 'ext/**/*.{c,h}', 'lib/**/*.rb'
  d.rdoc_files.include.add(files)

  d.options << '--line-numbers'
end

Rake::TestTask.new do |t|
  t.verbose = true
  t.warning = true
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
