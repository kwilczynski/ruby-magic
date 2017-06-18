module MagicTestHelpers
  def capture_stderr
    require 'stringio'
    saved_stderr, $stderr = $stderr, StringIO.new
    [yield, $stderr.string]
  ensure
    $stderr = saved_stderr
  end

  def with_fixtures(fixtures = 'fixtures', &block)
    new = @version && @version >= 519
    cwd = File.join('test', fixtures)
    Dir.chdir(cwd) do
      yield(Dir.pwd, File.join(new ? 'new-format' : 'old-format'))
    end
  end
end
