source 'https://rubygems.org'
gemspec

platforms :rbx do
  gem 'racc'
  gem 'rubysl', '~> 2.0'
  gem 'psych'
end

gem 'rake', '~> 11.2', '>= 11.2.2'
gem 'rdoc', '~> 4.1', '>= 4.1.1'
gem 'rake-compiler', '~> 1.0', '>= 1.0.1'

group :test do
  gem 'mocha', '~> 1.1', '>= 1.1.0'
  gem 'test-unit', '~> 3.0', '>= 3.0.0'
  gem 'coveralls', '>= 0.7.0', :require => false
end

group :development do
  gem 'guard', '~> 2.6', '>= 2.6.0'
  gem 'guard-rake', '>= 0.0.9'
  gem 'guard-bundler', '~> 2.0', '>= 2.0.0'
end
