require 'rake'

require 'rake/clean'
require 'rake/extensiontask'
require 'rake/testtask'

require 'rdoc/task'
require 'rubocop/rake_task'

RakeFileUtils.verbose_flag = false

CLEAN.include FileList['**/*{.o,.so,.dylib,.bundle}'],
              FileList['**/extconf.h'],
              FileList['**/Makefile'],
              FileList['pkg/']

CLOBBER.include FileList['**/tmp'],
                FileList['**/*.log'],
                FileList['doc/**'],
                FileList['ports/'],
                FileList['tmp/']

RUBY_MAGIC_GEM_SPEC = Gem::Specification.load('ruby-magic.gemspec')

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
    NOTICE
    CHANGELOG.md
    CONTRIBUTORS.md
    README.md
  ])
end

Rake::TestTask.new do |t|
  t.test_files = FileList['test/**/test_*']
  t.warning = true
end

RuboCop::RakeTask.new('lint') do |t|
  t.patterns = FileList['lib/**/*.rb', 'test/**/*.rb']
  t.fail_on_error = false
end

Gem::PackageTask.new(RUBY_MAGIC_GEM_SPEC) do |p|
  p.need_zip = false
  p.need_tar = false
end

CROSS_RUBY_PLATFORMS = ["x86_64-linux", "x86-linux"]
CROSS_RUBY_VERSIONS = ["2.5.0", "2.6.0", "2.7.0", "3.0.0"].join(":")

require "rake_compiler_dock"

ENV["RUBY_CC_VERSION"] = CROSS_RUBY_VERSIONS

Rake::ExtensionTask.new('magic', RUBY_MAGIC_GEM_SPEC) do |e|
  e.source_pattern = '*.{c,h}'
  e.ext_dir = 'ext/magic'
  e.lib_dir = 'lib/magic'

  e.cross_compile = true
  e.cross_config_options << "--enable-cross-build"
  e.cross_platform = CROSS_RUBY_PLATFORMS
end

namespace "gem" do
  def gem_builder(platform)
    # use Task#invoke because the pkg/*gem task is defined at runtime
    Rake::Task["native:#{platform}"].invoke
    Rake::Task["pkg/#{RUBY_MAGIC_GEM_SPEC.full_name}-#{Gem::Platform.new(platform).to_s}.gem"].invoke
  end

  CROSS_RUBY_PLATFORMS.each do |platform|
    desc "build native gem for #{platform} platform"
    task platform do
      RakeCompilerDock.sh <<~EOT, platform: platform
        gem install bundler --no-document &&
        bundle &&
        bundle exec rake gem:#{platform}:builder MAKE='nice make -j`nproc`'
      EOT
    end

    namespace platform do
      desc "build native gem for #{platform} platform (guest container)"
      task "builder" do
        gem_builder(platform)
      end
    end
  end

  desc "build all native gems"
  task "native" => CROSS_RUBY_PLATFORMS
end

task('default').clear

task default: %w[lint test]
