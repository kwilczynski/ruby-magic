# frozen_string_literal: true

module MagicTestHelpers
  def capture_stderr(children: false)
    require 'thread'
    mutex = Mutex.new

    if children
      require 'tempfile'
      captured_stderr = Tempfile.new('captured_stderr')

      mutex.synchronize do
        backup_stderr = $stderr.dup
        $stderr.reopen(captured_stderr)
        begin
          yield
          $stderr.rewind
          captured_stderr.read
        ensure
          captured_stderr.unlink
          $stderr.reopen(backup_stderr)
        end
      end
    else
      require 'stringio'
      captured_stderr = StringIO.new

      mutex.synchronize do
        backup_stderr = $stderr
        $stderr = captured_stderr
        begin
          yield
          captured_stderr.string
        ensure
          $stderr = backup_stderr
        end
      end
    end
  end

  def with_fixtures(fixtures: 'fixtures', &block)
    cwd = File.join('test', fixtures)
    Dir.chdir(cwd) do
      yield(Dir.pwd)
    end
  end

  def with_env(env, &blk)
    before = ENV.to_h.dup
    env.each { |k, v| ENV[k] = v }
    yield
  ensure
    ENV.replace(before)
  end
end
