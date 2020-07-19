# frozen_string_literal: true

module MagicTestHelpers
  def capture_stderr(children: false)
    require 'thread'
    semaphore = Mutex.new
    if children
      require 'tempfile'
      captured_stderr = Tempfile.new('captured_stderr')
      semaphore.synchronize do
        backup_stderr = $stderr.dup
        $stderr.reopen captured_stderr
        begin
          yield
          $stderr.rewind
          captured_stderr.read
        ensure
          captured_stderr.unlink
          $stderr.reopen backup_stderr
        end
      end
    else
      require 'stringio'
      captured_stderr = StringIO.new
      semaphore.synchronize do
        backup_stderr = $stderr
        $stderr = captured_stderr
        begin
          yield
        ensure
          $stderr = backup_stderr
        end
      end
      captured_stderr.string
    end
  end

  def with_fixtures(fixtures: 'fixtures', &block)
    new = @version && @version >= 519
    cwd = File.join('test', fixtures)
    Dir.chdir(cwd) do
      yield(Dir.pwd, File.join(new ? 'new-format' : 'old-format'))
    end
  end

  def with_attribute_override(attribute, value:,  &block)
    writer = "#{attribute}="
    old = @magic.send(attribute)
    @magic.send(writer, value)
  ensure
    @magic.send(writer, old)
  end
end
