# frozen_string_literal: true

require 'mkmf'

def darwin?
  RbConfig::CONFIG['target_os'] =~ /darwin/
end

def windows?
  RbConfig::CONFIG['target_os'] =~ /mswin|mingw32|windows/
end

if ENV['CC']
  RbConfig::CONFIG['CC'] = RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC']
end

ENV['CC'] = RbConfig::CONFIG['CC']

$CFLAGS += ' -std=c99 -fPIC'
$CFLAGS += ' -Wall -Wextra -pedantic'

if RbConfig::CONFIG['CC'] =~ /gcc/
  $CFLAGS += ' -O2' unless $CFLAGS =~ /-O\d/
  $CFLAGS += ' -Wcast-qual -Wwrite-strings -Wconversion -Wmissing-noreturn -Winline'
end

unless darwin?
  $LDFLAGS += ' -Wl,--as-needed -Wl,--no-undefined'
end

if windows?
  $LDFLAGS += ' -static-libgcc'
end

%w[
  CFLAGS
  CXXFLAGS
  CPPFLAGS
].each do |variable|
  $CFLAGS += format(' %s', ENV[variable]) if ENV[variable]
end

$LDFLAGS += format(' %s', ENV['LDFLAGS']) if ENV['LDFLAGS']

unless have_header('ruby.h')
  abort "\n" + (<<-EOS).gsub(/^[ ]{,3}/, '') + "\n"
    You appear to be missing Ruby development libraries and/or header
    files. You can install missing compile-time dependencies in one of
    the following ways:

    - Debian / Ubuntu

        apt-get install ruby-dev

    - Red Hat / CentOS / Fedora

        yum install ruby-devel or dnf install ruby-devel

    - Mac OS X (Darwin)

        brew install ruby (for Homebrew, see https://brew.sh)
        port install ruby2.6 (for MacPorts, see https://www.macports.org)

    - OpenBSD / NetBSD

        pkg_add ruby (for pkgsrc, see https://www.pkgsrc.org)

    - FreeBSD

        pkg install ruby (for FreeBSD Ports, see https://www.freebsd.org/ports)

    Alternatively, you can use either of the following Ruby version
    managers in order to install Ruby locally (for your user only)
    and/or system-wide:

    - Ruby Version Manager (for RVM, see https://rvm.io)
    - Ruby Environment (for rbenv, see https://github.com/sstephenson/rbenv)
    - Change Ruby (for chruby, see https://github.com/postmodern/chruby)

    More information about how to install Ruby on various platforms
    available at the following web site:

      https://www.ruby-lang.org/en/documentation/installation
  EOS
end

have_library('ruby')

have_func('rb_thread_call_without_gvl')
have_func('rb_thread_blocking_region')

unless have_header('magic.h')
  abort "\n" + (<<-EOS).gsub(/^[ ]{,3}/, '') + "\n"
    You appear to be missing libmagic(3) library and/or necessary header
    files. You can install missing compile-time dependencies in one of
    the following ways:

    - Debian / Ubuntu

        apt-get install libmagic-dev

    - Red Hat / CentOS / Fedora

        yum install file-devel or dns install file-devel

    - Mac OS X (Darwin)

        brew install libmagic (for Homebrew, see https://brew.sh)
        port install libmagic (for MacPorts, see https://www.macports.org)

    - OpenBSD / NetBSD

        pkg_add file (for pkgsrc, see https://www.pkgsrc.org)

    - FreeBSD

        pkg install file (for FreeBSD Ports, see https://www.freebsd.org/ports)

    Alternatively, you can download recent release of the file(1) package
    from the following web site and attempt to compile libmagic(3) manually:

      https://www.darwinsys.com/file
  EOS
end

have_library('magic')

unless have_func('magic_getpath')
  abort "\n" + (<<-EOS).gsub(/^[ ]{,3}/, '') + "\n"
    Your version of libmagic(3) appears to be too old.

    Please, consider upgrading to at least version 5.29 or newer,
    if possible. For more information about file(1) command and
    libmagic(3) please visit the following web site:

      https://www.darwinsys.com/file
  EOS
end

have_func('magic_getflags')

%w[
  utime.h
  sys/types.h
  sys/time.h
].each do |h|
  have_header(h)
end

%w[
  utime
  utimes
].each do |f|
  have_func(f)
end

dir_config('magic')

create_header
create_makefile('magic/magic')
