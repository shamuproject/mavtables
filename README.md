# mavtables

MAVLink router and firewall.




## Building

Even though `mavtables` uses CMake for building a simple Makefile (which uses
CMake) is included for convenience.

### Release

```
$ make
```

### Install

```
$ make
# make install
```

### Debug

```
$ make debug
```




## Testing

### Test

Testing uses [Catch2](https://github.com/catchorg/Catch2) for unit testing.  It
will be automatically downloaded into `lib/catch`.

```
$ make test
```

### Test Coverage

__NOTE: This currently requires GCC to be the default compiler and for
[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and
[gcovr](http://gcovr.com/) to be installed.__

```
$ make coverage
```

This will run all tests and report on the code coverage to the terminal as well
as placing a coverage report in html format at
[build/lcov/html/selected_targets/index.html]()




### Style

Astyle is used to enforce consistent style across `mavtables`.  It can be ran
(assuming it is installed) with
```
$ make style
```
which will fix and replace files based on style and store the original files
with the `.orig` extension.
