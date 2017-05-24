signing_key = File.expand_path('~/.gem/kwilczynski-private.pem')

Gem::Specification.new do |s|
  s.name    = 'ruby-magic'
  s.summary = 'File Magic in Ruby'

  s.description = (<<-EOS).gsub(/^[ ]+/, '')
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
            %w(Rakefile Gemfile Guardfile Vagrantfile
               AUTHORS CHANGES CHANGES.rdoc COPYRIGHT
               LICENSE README README.rdoc TODO VERSION
               ruby-magic.gemspec kwilczynski.asc
               kwilczynski-public.pem)

  s.executables   << 'magic'
  s.require_paths << 'lib'
  s.extensions    << 'ext/magic/extconf.rb'

  s.signing_key = signing_key if File.exist?(signing_key)
end
