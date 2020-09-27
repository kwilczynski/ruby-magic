guard 'bundler' do
  watch('Gemfile')
end

guard 'rake', task: 'compile' do
  watch(%r{^ext/(.+)\.(c|h)})
  watch(%r{^lib/(.+)\.rb})
end

guard 'rake', task: 'test' do
  watch(%r{^ext/(.+)\.(c|h)})
  watch(%r{^lib/(.+)\.rb})
  watch(%r{^test/test_(.+)\.rb})
end
