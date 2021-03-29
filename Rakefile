require 'rake'

require 'rake/clean'
require 'rake/extensiontask'
require 'rake/testtask'

require 'rdoc/task'
require 'rubocop/rake_task'

CLEAN.include FileList['**/*{.o,.so,.bundle,.log}'],
              FileList['**/Makefile']

CLOBBER.include FileList['lib/**/*.so'],
                FileList['doc/**/*'],
                FileList['tmp/'],
                FileList['ext/magic/extconf.h'],
                FileList['ext/magic/tmp'],
                FileList['ports/']

gem = eval File.read('ruby-magic.gemspec')

RDoc::Task.new do |d|
  d.title = 'File Magic in Ruby'
  d.main = 'README.md'
  d.options << '--line-numbers'
  d.rdoc_dir = 'doc/rdoc'
  d.rdoc_files.include FileList['ext/**/*.{c,h}', 'lib/**/*.rb']
  d.rdoc_files.include.add(%w[
    AUTHORS
    COPYRIGHT
    LICENSE
    CHANGELOG.md
    README.md
  ])
end

Rake::TestTask.new do |t|
  t.test_files = Dir['test/**/test_*']
  t.verbose = true
  t.warning = true
end

RuboCop::RakeTask.new('lint') do |t|
  t.patterns = FileList['lib/**/*.rb', 'test/**/*.rb']
  t.fail_on_error = false
end

Gem::PackageTask.new(gem) do |p|
  p.need_zip = true
  p.need_tar = true
end

Rake::ExtensionTask.new('magic', gem) do |e|
  e.ext_dir = 'ext/magic'
  e.lib_dir = 'lib/magic'
end

task('default').clear

task default: %w[lint test]
