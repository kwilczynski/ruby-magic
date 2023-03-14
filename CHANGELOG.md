# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.6.0] - 2023-03-14

### Added

- Add --with-magic-flags option for building libmagic.

## [0.5.5] - 2023-01-23

### Changed

- Update to latest libmagic version (5.44).
- Update mini_portile2 to version 2.8.0.
- Update RDoc in the Ruby gemspec and the Rake task.
- Update PKG_CONFIG_PATH paths to include more platforms.
- Add new library flag called MAGIC_NO_COMPRESS_FORK.
- Add assert to ensure that the internal object is initialised.
- Reduce scope of some string variables.
- Relax rake-compiler and rake-compiler-dock dependencies.
- Only enable support for the GC compaction when there is platform support.
- Remove workaround for linux build per RCD.
- Work around Ruby 3.1 pkg_config limitations.

## [0.5.4] - 2022-01-28

### Added

- Added a pessimistic version pin of `rake-compiler` and `rake-compiler-dock` versions in `Gemfile`.

### Changed

- Removed _RubyGems_ version pin from the Gem gemspec file.
- Simplified the `.gitlab-ci.yml` used to run _CI_ remotely on _GitLab_.
- Enabled verbose log when cross-compiling a native Ruby Gem using the `rake-compiler-dock` Gem. This allows for commands run to be shown.

### Deprecated

- Retired support for Ruby version 2.5 as it has reached _End of Life_ (EOL).

## [0.5.3] - 2021-12-22

### Fixed

- ext: load C extension files from non-native gems via "require"

## [0.5.2] - 2021-11-20

- No changes

## [0.5.1] - 2021-11-20

### Fixed

- Check for pkg-config when building static libraries.

## [0.5.0] - 2021-09-30

### Added

- Introduce support for building native gems.

### Changed

- Relax mini_portile2 dependency.
- Use RB_ALLOC() over calling ruby_xmalloc() directly
- Address warnings reported by cppcheck.
- Address code style that checkpatch.pl script reported.
- Fix how flags are set for the open() function.
- Clean up type definitions.
- Add alias Magic::mime_type for Magic::mime.
 - Make magic_set_flags() return void as its return value was never used.

### Fixed

- Fix truffleruby test failure
- Fix --use-system-libraries not working with Rake install

## [0.2.0] - 2015-03-25
### Added

- Added _Gemnasium_, plus _Coveralls_ and _Code Climate_ integration (and hence improved code test coverage).
- Added functionality to handle releasing the `GVL` for any _file_ and/or _I/O_ operations.
- Added ability to `Magic#flags_array` to return name of each flag that is set.
- Added naïve synchronization via `Mutex#lock` and `Mutex#unlock` methods to make interactions with _libmagic_ more thread-safe.
- Added ability for _Travis CI_ test against multiple versions of vanilla _libmagic_.
- Added _LLVM_ (`clang`) compiler to build with to _Travis CI_, and fixed various issues reported by `clang` compiler during build-time.
- Added ability for `Magic#load` to take nil as valid argument.
- Added support for the `MAGIC_CONTINUE` flag so that `Magic#file`, `Magic#buffer` and `Magic#descriptor` methods will return an _array_ for multiple results when appropriate.
- Added rudimentary `Vagrantfile` that can be used to build a development environment.
- Added the `Guard` Ruby gem for convenience, with an appropriate `Guardfile`.

### Changed

- Re-factored the _API_ and cleaned up small portions of code and documentation.
- Changed `Magic#version_array` and `Magic#version_string` methods to be a singleton methods.
- Renamed the `Magic#flags_array` method to `Magic#flags_to_a`.
- Changed the `Magic::new` method, so that it accepts an array of paths from which to load the _Magic_ database.
- Moved integration with _File_ and _String_ core classes into separate namespace.
- Changed the behavior not to catch the generic `Magic::Error`, plus always to raise on errors coming from _libmagic_. This is to make it more aligned with the standard library, where _file_ and _I/O_ related errors would raise an appropriate exception.
- Changed behavior consistent among various versions of _libmagic_ adhering to the _POSIX_ standard. This concerns the following _IEEE 1003.1_ standards:
  - http://pubs.opengroup.org/onlinepubs/007904975/utilities/file.html
  - http://pubs.opengroup.org/onlinepubs/9699919799/utilities/file.html

### Fixed

- Fixed formatting and white spaces.
- Fixed _Travis CI_ build against _Rubinius_.
- Fixed setting of global `errno` value to avoid race conditions.
- Fixed issue with _libmagic_'s regular expression (`regex`) library not working with _UTF-8_ (or any other wide-character encoding).
- Fixed build to make it work with _C++_ compilers.
- Fixed any _C90_ standard related build-time warnings.
- Fixed version number to comply with _Semantic Versioning 2_ (http://semver.org/).

### Deprecated

- Retired support for Ruby 1.8.x (no support for _MRI_, _Ruby Enterprise Edition_ and _Rubinius_).
- Retired testing with Ruby _1.9.2_ and _2.1.0_, and added _2.2.0_ on _Travis CI_.
- Removed forward declaration of `errno` as it's not needed on systems with modern _C/C++_ libraries and compilers.
- Removed the `-Wl,--no-undefined` option from `LDFLAGS`, as they might break on some systems.

## [0.1.0] - 2014-03-21
### Added

- First version of Magic.

[Unreleased]: https://github.com/kwilczynski/ruby-magic/compare/v0.5.4...HEAD
[0.5.4]: https://github.com/kwilczynski/ruby-magic/compare/v0.5.3...v0.5.4
[0.5.3]: https://github.com/kwilczynski/ruby-magic/compare/v0.5.2...v0.5.3
[0.5.2]: https://github.com/kwilczynski/ruby-magic/compare/v0.5.1...v0.5.2
[0.5.1]: https://github.com/kwilczynski/ruby-magic/compare/v0.5.0...v0.5.1
[0.5.0]: https://github.com/kwilczynski/ruby-magic/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/kwilczynski/ruby-magic/compare/v0.3.2...v0.4.0
[0.3.2]: https://github.com/kwilczynski/ruby-magic/compare/v0.3.1...v0.3.2
[0.3.1]: https://github.com/kwilczynski/ruby-magic/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/kwilczynski/ruby-magic/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/kwilczynski/ruby-magic/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/kwilczynski/ruby-magic/compare/29e6c26...v0.1.0
