require 'stringio'

module MagicTestHelpers
  def capture_stderr
    saved_stderr, $stderr = $stderr, StringIO.new
    [yield, $stderr.string]
  ensure
    $stderr = saved_stderr
  end
end
