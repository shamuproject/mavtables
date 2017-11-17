CMAKE = cmake
INSTALL_PREFIX ?= /usr/local
COVERAGE = Off

default: release

install:
	$(MAKE) -j8 -C build install

release:
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=$(COVERAGE) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -j8 -C build

debug: 
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=$(COVERAGE) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -j8 -C build

test: unit_tests

unit_tests: debug
	./build/unit_tests

coverage: COVERAGE = On
coverage: test
	$(MAKE) -j8 -C build lcov-geninfo
	$(MAKE) -j8 -C build lcov-genhtml
	gcovr -r . -e '.*catch.hpp' -e '.*fakeit.hpp'

style:
	astyle --options=.astylerc "src/*.cpp" "src/*.hpp" "test/*.cpp"

clean:
	rm -rf build
	find . -name "*.orig" -delete

remove-subs:
	$(MAKE) -C lib clean

.PHONY: style debug release test unit_tests coverage style clean remove-subs
.SILENT:
