THIS_FILE := $(lastword $(MAKEFILE_LIST))
CMAKE = cmake
PREFIX ?= /usr/local
COVERAGE = Off

default: release

install: release
	$(MAKE) -C build install

release:
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=$(COVERAGE) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) ..
	$(MAKE) -C build

debug: tags
	mkdir -p build
	cd build && $(CMAKE) \
		-G"Unix Makefiles" \
		-DENABLE_COVERAGE=$(COVERAGE) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) ..
	$(MAKE) -C build

tags:
	ctags -R .

unit_tests: debug
	./test/unit_tests/run_tests.sh

integration_tests: debug
	./test/integration_tests/run_tests.sh

test: debug
	./test/unit_tests/run_tests.sh
	./test/integration_tests/run_tests.sh

coverage: COVERAGE = On
coverage: test
	$(MAKE) -C build lcov-geninfo
	$(MAKE) -C build lcov-genhtml
	gcovr -r . -e 'lib.*'

linecheck:
	-grep -rPIn --color=always '(.{81})' src | grep -v '\\copydoc'
	-grep -rPIn --color=always '(.{81})' test | grep -v '\\copydoc'

style:
	astyle --options=.astylerc "src/*.cpp" "src/*.hpp" "test/*.cpp" | grep 'Formatted' || true
	$(MAKE) -f $(THIS_FILE) linecheck

html:
	mkdir -p build
	cd build && $(CMAKE) ..
	$(MAKE) -C build doc

doc: html
	$(MAKE) -C build/doc/latex
	cp build/doc/latex/refman.pdf build/doc/html/mavtables.pdf

gh-pages: doc
	mkdir -p build
	-git clone -b gh-pages --single-branch git@github.com:shamuproject/mavtables.git build/gh-pages
	rm -R build/gh-pages/*
	cp -R build/doc/html/* build/gh-pages/
	cd build/gh-pages/ && git add --all
	cd build/gh-pages/ && git commit --amend -m "Publish documentation to GitHub Pages."
	cd build/gh-pages/ && git push --force

clean:
	rm -rf build
	find . -name "*.orig" -delete
	$(MAKE) -C test/integration_tests clean

remove-subs:
	$(MAKE) -C lib remove-subs
	$(MAKE) -C cmake remove-subs

.PHONY: debug release test unit_tests integration_tests coverage
.PHONY: style doc gh-pages clean remove-subs 
.SILENT:
