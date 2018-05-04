MAVTables User Manual {#user_manual}
====================================

* [Project](https://github.com/shamuproject/mavtables)
* [Installing](#installing)
  * [Requirements](#requirements)
  * [CMake Installation](#cmake-installation)
  * [Manual Installation](#manual-installation)
* [Usage](#usage)
  * [Running](#running)
  * [Log Level](#log-level)
    * [--loglevel 0](#loglevel-0)
    * [--loglevel 1](#loglevel-1)
    * [--loglevel 2](#loglevel-2)
    * [--loglevel 3](#loglevel-3)
  * [Abstract Syntax Tree](#abstract-syntax-tree)
* [Configuration](configuration.md)


# Installing

## Requirements

In order to compile you will need the following packages:

* GCC 7+ or Clang 5+ (needed for C++17 support)
* [CMake v3.3+](https://cmake.org/)
* [Boost v1.54+](https://www.boost.org/)

on either

* Linux
* Mac OS X
* BSD

mavtables is tested on both Linux and Mac OS X and should work on any unix
compatible system.


## CMake Installation

To install via CMake simple run:
```
$ make
$ make install
```

The installation prefix is `/usr/local` by default but can be changed with
```
$ make PREFIX=/desired/install/path
# make DESDIR=/desired/install/path install
```
`PREFIX` changes where mavtables expects to be installed while `DESTDIR` changes
where it will actually be installed.

To change the MAVLink dialect use the `DIALECT` environment variable.  For
example, to use the `common` dialect:
```
$ make DIALECT=common
# make install
```

To change the MAVLink implementation completely use the `MDIR` environment
variable to set the path to the MAVLink library instead of downloading the
upstream implementation.  For example, if a custom implementation is at
`/tmp/mavlink/v2` then:
```
$ make MDIR=/tmp/mavlink/v2
# make install
```
Both the `MDIR` and `DIALECT` variables can be used together.


## Manual Installation

To manually install mavtables first make a release build:
```
$ make
```
Remembering to use the `PREFIX` option if the final installation destination is
not `/usr/local`.

Copy binary, configuration, and systemd unit files to their proper locations
```
# cp build/mavtables /usr/local/bin/
# cp examples/mavtables.conf /usr/local/etc/
# cp build/mavtables.service /usr/local/lib/systemd/system/
```


# Usage

## Running

To run mavtables and begin routing packets
```
$ mavtables
```
This will use the first configuration file it finds in the configuration file
priority order:

1. The target of the `MAVTABLES_CONFIG_PATH` environment variable.
2. `.mavtablesrc` in the current directory.
3. `.mavtablesrc` at `$HOME/.mavtablesrc`.
4. The main configuration file at `/etc/mavtables.conf`.

To specify the configuration file use
```
$ mavtables --config path/to/config/file
```

If the configuration file contains an error, mavtables will exit immediately and
print an error message to stderr.

The installer will install a system wide configuration file at
`PREFIX/etc/mavtables.conf` and a systemd unit file at
`PREFIX/lib/systemd/system/mavtables.service`.  Therefore, on Linux (with
systemd) mavtables can be run as a daemon with
```
# systemctl start mavtables
```
or enabled for startup on each boot with
```
# systemctl enable mavtables
```

systemd will keep mavtables running (even if it crashes) and will use the
`PREFIX/etc/mavtables.conf` configuration file.  It will also use `loglevel` 1,
discussed in the [Log Level](#log-level) section below.


## Log Level

The `--loglevel` flag can be use to set the logging level.  This is the
verbosity to print to stdout when mavtables is running.  Each loglevel,
0 through 3, is documented below.

### --loglevel 0

Do not log anything to stdcout.

### --loglevel 1

Log each new component.  Therefore, every time a new system/component address is
connected to the router it will be printed to stdcout.  An example log output
is:
```
new component 127.1 on 127.0.0.1
new component 192.168 on 127.0.0.1
new component 172.128 on ./ttyS0
new component 10.10 on 127.0.0.1
```

### --loglevel 2

In addition to everything in `--loglevel 1` this will log received packets to
stdcout.  An example log output is:
```
new component 127.1 on 127.0.0.1
received HEARTBEAT (#0) from 127.1 (v2.0) source 127.0.0.1
new component 192.168 on 127.0.0.1
received HEARTBEAT (#0) from 192.168 (v2.0) source 127.0.0.1
new component 172.128 on ./ttyS0
received HEARTBEAT (#0) from 172.128 (v2.0) source ./ttyS0
new component 10.10 on 127.0.0.1
received HEARTBEAT (#0) from 10.10 (v2.0) source 127.0.0.1
received POWER_STATUS (#125) from 10.10 (v2.0) source 127.0.0.1
received SYS_STATUS (#1) from 10.10 (v2.0) source 127.0.0.1
received GPS_RAW_INT (#24) from 10.10 (v2.0) source 127.0.0.1
received LOCAL_POSITION_NED_COV (#64) from 10.10 (v2.0) source 127.0.0.1
received MISSION_REQUEST_PARTIAL_LIST (#37) from 10.10 to 172.168 (v2.0) source 127.0.0.1
received LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET (#89) from 10.10 (v2.0) source 127.0.0.1
received MANUAL_CONTROL (#69) from 10.10 (v2.0) source 127.0.0.1
received GPS_INPUT (#232) from 10.10 (v2.0) source 127.0.0.1
received VFR_HUD (#74) from 10.10 (v2.0) source 127.0.0.1
received LANDING_TARGET (#149) from 10.10 (v2.0) source 127.0.0.1
received SCALED_IMU (#26) from 10.10 (v2.0) source 127.0.0.1
received PARAM_REQUEST_READ (#20) from 10.10 to 192.168 (v2.0) source 127.0.0.1
```

### --loglevel 3

In addition to everything in `--loglevel 1` and `--loglevel 2` this will log
routed packets, indicating whether they are `accepted` or `rejected`, to
stdcout.  An example log output is:
```
new component 127.1 on 127.0.0.1
received HEARTBEAT (#0) from 127.1 (v2.0) source 127.0.0.1
rejected HEARTBEAT (#0) from 127.1 (v2.0) source 127.0.0.1 dest ./ttyS0
new component 192.168 on 127.0.0.1
received HEARTBEAT (#0) from 192.168 (v2.0) source 127.0.0.1
rejected HEARTBEAT (#0) from 192.168 (v2.0) source 127.0.0.1 dest ./ttyS0
rejected HEARTBEAT (#0) from 192.168 (v2.0) source 127.0.0.1 dest 127.0.0.1
new component 172.128 on ./ttyS0
received HEARTBEAT (#0) from 172.128 (v2.0) source ./ttyS0
rejected HEARTBEAT (#0) from 172.128 (v2.0) source ./ttyS0 dest 127.0.0.1
rejected HEARTBEAT (#0) from 172.128 (v2.0) source ./ttyS0 dest 127.0.0.1
new component 10.10 on 127.0.0.1
received HEARTBEAT (#0) from 10.10 (v2.0) source 127.0.0.1
rejected HEARTBEAT (#0) from 10.10 (v2.0) source 127.0.0.1 dest ./ttyS0
rejected HEARTBEAT (#0) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
rejected HEARTBEAT (#0) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
received POWER_STATUS (#125) from 10.10 (v2.0) source 127.0.0.1
rejected POWER_STATUS (#125) from 10.10 (v2.0) source 127.0.0.1 dest ./ttyS0
accepted POWER_STATUS (#125) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
rejected POWER_STATUS (#125) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
received SYS_STATUS (#1) from 10.10 (v2.0) source 127.0.0.1
accepted SYS_STATUS (#1) from 10.10 (v2.0) source 127.0.0.1 dest ./ttyS0
accepted SYS_STATUS (#1) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
accepted SYS_STATUS (#1) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
received GPS_RAW_INT (#24) from 10.10 (v2.0) source 127.0.0.1
rejected GPS_RAW_INT (#24) from 10.10 (v2.0) source 127.0.0.1 dest ./ttyS0
accepted GPS_RAW_INT (#24) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
accepted GPS_RAW_INT (#24) from 10.10 (v2.0) source 127.0.0.1 dest 127.0.0.1
```

## Abstract Syntax Tree

mavtables can print the abstract syntax tree of the configuration file instead
of routing packets.  When run in this mode it will return immediately  This is
accomplished with:
```
$ mavtables --ast
```
or
```
$ mavtables  --ast --config path/to/config/file
```
which will print something similar to
```
===== /etc/mavtables.conf =====
:005:  default_action
:005:  |  reject
:008:  udp
:009:  |  port 14500
:010:  |  address 127.0.0.1
:022:  chain default
:023:  |  accept
```

This feature can be used to debug configuration files.


For and explanation of configuration files see
[Configuration](configuration.md).
