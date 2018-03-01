mavtables {#mainpage}
=====================

* [Build Status](https://travis-ci.org/shamuproject/mavtables) ![Build Status](https://travis-ci.org/shamuproject/mavtables.svg?branch=master) 
* [Coverage Status](https://coveralls.io/github/shamuproject/mavtables?branch=master) ![Coverage Status](https://coveralls.io/repos/github/shamuproject/mavtables/badge.svg?branch=master)

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

* [HTML Documentation](https://shamuproject.github.io/mavtables)
* [PDF Documentation](mavtables.pdf)
* [Download](https://github.com/shamuproject/mavtables/archive/master.zip)
* [GitHub](https://github.com/shamuproject/mavtables)
* [Theory of MAVLink Routing](http://ardupilot.org/dev/docs/mavlink-routing-in-ardupilot.html)


## Install

In order to compile you will need the following packages:

* GCC 7+ or Clang 5+ (needed for C++17 support)
* [CMake v3.3+](https://cmake.org/)
* [Boost v1.54+](https://www.boost.org/)

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
