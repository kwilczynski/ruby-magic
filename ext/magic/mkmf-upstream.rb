# -*- coding: us-ascii -*-
# frozen-string-literal: false

module MakeMakefile
  # Returns compile/link information about an installed library in a
  # tuple of <code>[cflags, ldflags, libs]</code>, by using the
  # command found first in the following commands:
  #
  # 1. If <code>--with-{pkg}-config={command}</code> is given via
  #    command line option: <code>{command} {options}</code>
  #
  # 2. <code>{pkg}-config {options}</code>
  #
  # 3. <code>pkg-config {options} {pkg}</code>
  #
  # Where {options} is, for instance, <code>--cflags</code>.
  #
  # The values obtained are appended to +$INCFLAGS+, +$CFLAGS+, +$LDFLAGS+ and
  # +$libs+.
  #
  # If one or more <code>options</code> argument is given, the config command is
  # invoked with the options and a stripped output string is returned without
  # modifying any of the global values mentioned above.
  def pkg_config(pkg, *options)
    _, ldir = dir_config(pkg)
    if ldir
      pkg_config_path = "#{ldir}/pkgconfig"
      if File.directory?(pkg_config_path)
        Logging.message("PKG_CONFIG_PATH = %s\n", pkg_config_path)
        envs = ["PKG_CONFIG_PATH"=>[pkg_config_path, ENV["PKG_CONFIG_PATH"]].compact.join(File::PATH_SEPARATOR)]
      end
    end
    if pkgconfig = with_config("#{pkg}-config") and find_executable0(pkgconfig)
      # if and only if package specific config command is given
    elsif ($PKGCONFIG ||=
           (pkgconfig = with_config("pkg-config", ("pkg-config" unless CROSS_COMPILING))) &&
           find_executable0(pkgconfig) && pkgconfig) and
        xsystem([*envs, $PKGCONFIG, "--exists", pkg])
      # default to pkg-config command
      pkgconfig = $PKGCONFIG
      get = proc {|opts|
        opts = Array(opts).map { |o| "--#{o}" }
        opts = xpopen([*envs, $PKGCONFIG, *opts, pkg], err:[:child, :out], &:read)
        Logging.open {puts opts.each_line.map{|s|"=> #{s.inspect}"}}
        opts.strip if $?.success?
      }
    elsif find_executable0(pkgconfig = "#{pkg}-config")
      # default to package specific config command, as a last resort.
    else
      pkgconfig = nil
    end
    if pkgconfig
      get ||= proc {|opts|
        opts = Array(opts).map { |o| "--#{o}" }
        opts = xpopen([*envs, pkgconfig, *opts], err:[:child, :out], &:read)
        Logging.open {puts opts.each_line.map{|s|"=> #{s.inspect}"}}
        opts.strip if $?.success?
      }
    end
    orig_ldflags = $LDFLAGS
    if get and !options.empty?
      get[options]
    elsif get and try_ldflags(ldflags = get['libs'])
      if incflags = get['cflags-only-I']
        $INCFLAGS << " " << incflags
        cflags = get['cflags-only-other']
      else
        cflags = get['cflags']
      end
      libs = get['libs-only-l']
      if cflags
        $CFLAGS += " " << cflags
        $CXXFLAGS += " " << cflags
      end
      if libs
        ldflags = (Shellwords.shellwords(ldflags) - Shellwords.shellwords(libs)).quote.join(" ")
      else
        libs, ldflags = Shellwords.shellwords(ldflags).partition {|s| s =~ /-l([^ ]+)/ }.map {|l|l.quote.join(" ")}
      end
      $libs += " " << libs

      $LDFLAGS = [orig_ldflags, ldflags].join(' ')
      Logging::message "package configuration for %s\n", pkg
      Logging::message "incflags: %s\ncflags: %s\nldflags: %s\nlibs: %s\n\n",
                       incflags, cflags, ldflags, libs
      [[incflags, cflags].join(' '), ldflags, libs]
    else
      Logging::message "package configuration for %s is not found\n", pkg
      nil
    end
  end
end
