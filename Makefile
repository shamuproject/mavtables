CMAKE = cmake
INSTALL_PREFIX ?= /usr/local

default: release

install:
	$(MAKE) -j8 -C build install

release:
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=Off \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -j8 -C build

debug: 
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=On \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -j8 -C build

test: unit_tests

unit_tests: debug
	./build/unit_tests

coverage: test
	$(MAKE) -C build lcov-geninfo
	$(MAKE) -C build lcov-genhtml
	gcovr -r . -e '.*catch.hpp'

style:
	astyle --options=.astylerc "src/*.cpp" "src/*.hpp"

clean:
	rm -rf build
	find . -name "*.orig" -delete

remove-subs:
	$(MAKE) -C lib clean

.PHONY: style debug release test unit_tests coverage style clean remove-subs
.SILENT:
