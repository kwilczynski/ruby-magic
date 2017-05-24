guard 'bundler' do
  watch('Gemfile')
end

guard 'rake', :task => 'test' do
  watch(%r{^ext/(.+)\.(c|h)})
  watch(%r{^lib/(.+)\.rb})
  watch(%r{^test/.+_test\.rb$})
end
