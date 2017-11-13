CMAKE = cmake
INSTALL_PREFIX ?= /usr/local

default: release

release:
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -j8 -C build

debug: 
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -j8 -C build

test: unit_tests

unit_tests: debug
	./build/unit_tests

style:
	astyle --options=.astylerc "src/*.cpp" "src/*.hpp"

clean:
	rm -rf build
	find . -name "*.orig" -delete

remove-subs:
	$(MAKE) -C lib clean

.PHONY: style debug release test unit_tests style clean remove-subs
.SILENT:
