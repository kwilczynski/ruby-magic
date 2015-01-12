# -*- encoding: utf-8 -*-

# :enddoc:

#
# extconf.rb
#
# Copyright 2013-2015 Krzysztof Wilczynski
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

require 'mkmf'

RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

$CFLAGS << ' -std=c99 -g -Wall -Wextra -pedantic'

unless RbConfig::CONFIG['host_os'][/darwin/]
  $LDFLAGS << ' -Wl,--as-needed'
end

$LDFLAGS << " %s" % ENV['LDFLAGS'] if ENV['LDFLAGS']

%w(CFLAGS CXXFLAGS CPPFLAGS).each do |variable|
  $CFLAGS << " %s" % ENV[variable] if ENV[variable]
end

have_header('locale.h')
have_header('xlocale.h')
have_header('utime.h')
have_header('sys/types.h')
have_header('sys/time.h')

have_ruby_h = have_header('ruby.h')
have_magic_h = have_header('magic.h')

have_func('newlocale', ['locale.h', 'xlocale.h'])
have_func('uselocale', ['locale.h', 'xlocale.h'])
have_func('freelocale', ['locale.h', 'xlocale.h'])

have_func('utime', ['utime.h', 'sys/types.h'])
have_func('utimes', 'sys/time.h')

unless have_ruby_h
  abort <<-EOS

  You appear to be missing Ruby development libraries and/or header
  files. You can install missing compile-time dependencies in one of
  the following ways:

  - Debian / Ubuntu

      apt-get install ruby-dev

  - Red Hat / CentOS / Fedora

      yum install ruby-devel


  Alternatively, you can use either of the following Ruby version
  managers in order to install Ruby locally (for your user only)
  and/or system-wide:

  - Ruby Version Manager (for RVM, see http://rvm.io/)
  - Ruby Environment (for rbenv, see http://github.com/sstephenson/rbenv)

  EOS
end

have_func('rb_thread_call_without_gvl')
have_func('rb_thread_blocking_region')

unless have_magic_h
  abort <<-EOS

  You appear to be missing libmagic(3) library and/or necessary header
  files. You can install missing compile-time dependencies in one of
  the following ways:

  - Debian / Ubuntu

      apt-get install libmagic-dev

  - Red Hat / CentOS / Fedora

       yum install file-devel

  - Mac OS X (Darwin)

       brew install libmagic (for Homebrew, see http://brew.sh/)
       port install libmagic (for MacPorts, see http://www.macports.org/)


  Alternatively, you can download recent release of the file(1) package
  from the following web site and attempt to compile libmagic(3) manually:

    http://www.darwinsys.com/file/

  EOS
end

unless have_library('magic', 'magic_getpath')
  abort <<-EOS

  Your version of libmagic(3) appears to be too old.  Please, consider
  upgrading to at least version 5.09 or newer if possible ...

  For more information about file(1) command and libmagic(3) please
  visit the following web site:

    http://www.darwinsys.com/file/

  EOS
end

have_func('magic_version', 'magic.h')

dir_config('magic')

create_header
create_makefile('magic/magic')

# vim: set ts=2 sw=2 sts=2 et :
# encoding: utf-8
