mavtables {#readme}
===================

[![Build Status](https://travis-ci.org/shamuproject/mavtables.svg?branch=master)](https://travis-ci.org/shamuproject/mavtables) [![Coverage Status](https://coveralls.io/repos/github/shamuproject/mavtables/badge.svg?branch=master)](https://coveralls.io/github/shamuproject/mavtables?branch=master)

A MAVLink router and firewall.  It can connect 2 or more MAVLink endpoints such
as autopilots, ground control software, loggers, image capture systems, etc over
serial and UDP. MAVLink packets will be routed to specific components when they
have a destination address.  Any packet, targeted or broadcasted, can be
filtered based on source system/component, destination system/component and
message type.  The filter can also apply a priority to packets allowing more
important packets to take priority over lower priority packets when an endpoint
is choked.


## Links

* [User Documentation](doc/user_manual.md)
* [HTML Developer Documentation](https://shamuproject.github.io/mavtables)
* [PDF Developer Documentation](https://shamuproject.github.io/mavtables/mavtables.pdf)
* [Download](https://github.com/shamuproject/mavtables/archive/master.zip)
* [GitHub](https://github.com/shamuproject/mavtables)
* [Theory of MAVLink Routing](http://ardupilot.org/dev/docs/mavlink-routing-in-ardupilot.html)


## Compilation and Installation

In order to compile you will need the following packages:

* GCC 7+ or Clang 5+ (needed for C++17 support)
* [CMake v3.3+](https://cmake.org/)
* [Boost v1.54+](https://www.boost.org/)

__Clang is recommended when contributing to mavtables as it's warnings are
more comprehensive.  However, GCC must be used when generating code coverage
reports.__

The following packages are only needed for development work:

* [Artistic Style](http://astyle.sourceforge.net/) (used for
  checking/fixing the code style)
* [Gcovr](http://gcovr.com/) (coverage report)
* [LCOV](http://ltp.sourceforge.net/coverage/lcov.php) (detailed coverage html
  report)
* [socat](http://www.dest-unreach.org/socat/) (for testing serial port
  communications)

`mavtables` can be easily installed using the standard procedure of
```
$ make
# make install
```
The installation prefix is `/usr/local` by default but can be changed with
```
$ make
# make PREFIX=/desired/install/path install
```
The makefile will download and use the default MAVLink implementation with the
ArduPilot dialect.  This can be overridden by setting the `MDIR` environment
variable to the library path (containing the `DIALECT`) and/or the `DIALECT`
environment variable to the MAVLink dialect to use.  For instance the default
value of `DIALECT` is `ardupilotmega`.


## Running

To run mavtables and begin routing packets
```
$ mavtables
```
This will use the first configuration file it finds in the configuration file
priority order given in the next section.  To force a specific configuration
file the `--config` flag may be used.
```
$ mavtables --config <path/to/config>
```
The inbuilt help may be accessed with the `-h` or `--help` flags.
```
$ mavtalbes --help
usage: mavtables:
  -h [ --help ]         print this message
  --config arg          specify configuration file
  --ast                 print AST of configuration file (do not run)
  --version             print version and license information
  --loglevel arg        level of logging, between 0 and 3
```


## Configuration File

Both interfaces and filter rules are defined in a configuration file.  The
format of this configuration file is documented in
[`doc/configuration.md`](doc/configuration.md) and an example is located at
`examples/mavtables.conf` which is the same file that is installed at
`/etc/mavtables.conf` when using `make install`.  The configuration file used is
the first one found in the following order:

1. The target of the `MAVTABLES_CONFIG_PATH` environment variable.
2. `.mavtablesrc` in the current directory.
3. `.mavtablesrc` at `$HOME/.mavtablesrc`.
4. The main configuration file at `/etc/mavtables.conf`.

If the `--config` flag is given then mavtables will only look for the given
configuration file.


## Contributing

Before contributing read the `CONTRIBUTING.md` file which gives guidelines that
must be followed by all developers working on mavtables.
