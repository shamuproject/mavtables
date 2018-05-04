Makefile Targets {#makefile_targets}
====================================

While mavtables uses [CMake](https://cmake.org/) for building a simple Makefile
(which calls `cmake`) is included for convenience.  The available targets are
listed below.


## release (default)

Build `mavtables` using the _Release_ option which generates an optimized (for
speed) executable without debug symbols.  `mavtables` is placed at
`build/mavtables`.

Unit tests are built as well under `build/unit_tests` and are built with
optimizations.


## debug

Build `mavtables` and unit tests using the _Debug_ option which will enable
debug flags and turn off optimizations.  The executables are located at
`build/mavtables` and `build/unit_tests`.


## test

Build and run all tests.  Runs both `test/unit_tests/run_tests.sh` and
`test/integration_tests/run_tests.sh`.  The latter requires Python with the
packages located in `test/integration_tests/requirements.txt`.


## unit_tests

Build and run unit tests only.


## integration_tests

Build and run integration tests only.  Requires Python with the packages located
in `test/integration_tests/requirements.txt`.


## coverage

Build and run all tests and compute test coverage.  This option requires
[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and
[gcovr](http://gcovr.com/) to be installed and will only work when the compiler
is `g++`.

The coverage per file will be printed to the terminal and a detailed, line by
line, report generated with [lcov](http://ltp.sourceforge.net/coverage/lcov.php)
at `build/lcov/html/selected_targets/index.html`.


## linecheck

Prints all lines exceeding 80 characters.


## style

Fix the style of C++ source code and header files.  The original files are
backed up with a `.orig` extension.  [Artistic
Style](http://astyle.sourceforge.net/) is required to use this target.  This
will also call the linecheck target.


## html

Generate html documentation with
[Doxygen](http://www.stack.nl/~dimitri/doxygen/) and place it at
`build/doc/html/index.html`.


## doc

Generate html and pdf documentation with
[Doxygen](http://www.stack.nl/~dimitri/doxygen/) and place it at
`build/doc/html/index.html` and `build/doc/html/mavtables.pdf` respectively.


## gh-pages

Generate html and pdf documentation and publish to
[https://shamuproject.github.io/mavtables](https://shamuproject.github.io/mavtables).


## clean

Clean up the project directory.  This does not remove git submodules.


## remove-subs

Remove all git submodules.  They will be re-downloaded if needed.
