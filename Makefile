CMAKE = cmake
INSTALL_PREFIX ?= /usr/local

default: release
debug: 
	mkdir -p build
	cd build && $(CMAKE) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -C build
release:
	mkdir -p build
	cd build && $(CMAKE) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ..
	$(MAKE) -C build
style:
	astyle --options=.astylerc "src/*.cpp" "src/*.hpp"
clean:
	rm -rf build
	find . -name "*.orig" -delete

.PHONY: style
.SILENT:
