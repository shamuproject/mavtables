mavtables {#mainpage}
=====================

## Introduction

A MAVLink router and firewall.  It can connect over serial and UDP with 2 or
more MAVLink endpoints such as autopilots, ground control software, loggers,
image capture systems, etc.  MAVLink packets will be routed when they are
addressed and can be filtered based on source system/component and message type.

* [README](\ref readme)
* [LICENSE](\ref license)
* [CONTRIBUTING](\ref contributing)
* [Makefile Targets](\ref makefile_targets)


## Links

* [Documentation](https://shamuproject.github.io/mavtables)
* [Download](https://github.com/shamuproject/mavtables/archive/master.zip)
* [GitHub](https://github.com/shamuproject/mavtables)


## Install

In order to compile you will need the following packages:

* GCC 7+ or Clang 5+ (needed for C++17 support)
* [CMake](https://cmake.org/)

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
