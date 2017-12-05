CMAKE = cmake
PREFIX ?= /usr/local
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
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) ..
	$(MAKE) -j8 -C build

debug: 
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=$(COVERAGE) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) ..
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

doc:
	mkdir -p build
	cd build && $(CMAKE) ..
	$(MAKE) -j8 -C build doc

gh-pages: doc
	mkdir -p build
	-git clone -b gh-pages --single-branch git@github.com:shamuproject/mavtables.git build/gh-pages
	rm -R build/gh-pages/*
	cp -R build/doc/html/* build/gh-pages/
	cd build/gh-pages/ && git add --all
	cd build/gh-pages/ && git commit -m "Publish documentation to GitHub Pages."
	cd build/gh-pages/ && git push

clean:
	rm -rf build
	find . -name "*.orig" -delete

remove-subs:
	$(MAKE) -C lib remove-subs
	$(MAKE) -C lib remove-subs

.PHONY: style debug release test unit_tests coverage style doc gh-pages clean remove-subs
.SILENT:
