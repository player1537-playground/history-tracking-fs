MAKEFLAGS += --warn-undefined-variables --no-builtin-variables
SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
.SUFFIXES:

################
# Utilities

# Used for backups
date := $(shell date +%Y%m%d%H%M%S)

# Used for debugging
.PHONY: echo.%
echo.%:
	@echo $*=$($*)

# Used to filter based on specific substrings. e.g.
#   $(call containing foo,bing bingfoobaz dingfoo foobang)
#     == bingfoobaz dingfoo foobang
containing = $(filter %,$(foreach v,$2,$(if $(findstring $1,$v),$v)))
not-containing = $(filter %,$(foreach v,$2,$(if $(findstring $1,$v),,$v)))

################
# Environment variables

T :=
debug := off
gdb := off
valgrind := off
profile := off
CFLAGS := -std=gnu11 -pedantic -Wall -Werror -Wno-missing-braces -DFUSE_USE_VERSION=30 -D_FILE_OFFSET_BITS=64 $(shell PKG_CONFIG_PATH=$(HOME)/downloads/fuse-3.0.0 pkg-config fuse3 --cflags)
LDFLAGS := -lm $(shell PKG_CONFIG_PATH=$(HOME)/downloads/fuse-3.0.0 pkg-config fuse3 --libs)
CC := gcc
AR := ar
VALGRIND := valgrind
VALGRINDFLAGS := --leak-check=yes --track-origins=yes --error-exitcode=1
COV := gcov
COVFLAGS := --object-directory build
LCOV := lcov
LCOVFLAGS := -c -d build/

ifeq ($(gdb), on)
debug = on
MAKEFLAGS += -B
endif

ifeq ($(valgrind), on)
debug := on
MAKEFLAGS += -B
endif

ifeq ($(valgrind), on)
ifeq ($(gdb), on)
$(error Use just one of the valgrind or gdb flags)
endif
endif

ifeq ($(debug), on)
CFLAGS += -g
endif

ifeq ($(profile), on)
CFLAGS += -fprofile-arcs -ftest-coverage -O0
MAKEFLAGS += -B
else
CFLAGS += -O3
endif

################
# Sanity checks and local variables

sources := $(wildcard src/*.c)
headers := $(wildcard include/*.h)
tests := $(wildcard tests/test_*.c)
executables := $(patsubst src/main_%.c,%,$(wildcard src/main_*.c))

################
# Exported variables

export DATE := $(date)

################
# Includes

################
# Standard targets

.PHONY: all
all: $(addprefix build/,$(executables))

.PHONY: run
run: $(addprefix build/,$(executables)) | mnt data
	build/foo -f mnt

.PHONY: test
test: $(patsubst tests/%,%,$(tests:.c=))

.PHONY: valgrind
valgrind: clean
	+$(MAKE) test valgrind=on
	rm -rf build

.PHONY: profile
profile: clean | cov
	+$(MAKE) test profile=on
	$(COV) $(COVFLAGS) $(sources)
	mv *.c.gcov cov/
	$(LCOV) $(LCOVFLAGS) -o cov/out.lcov
	rm -rf build

.PHONY: depend
depend: .libtap.secondary

.PHONY: check
check:

.PHONY: help
help:

.PHONY: clean
clean:
	rm -- $(wildcard *~ **/*~ *.orig *.d **/*.d **/*.orig .*.secondary)
	rm -rf -- tmp libtap build lib cov

.PHONY: prepare
prepare:
	+$(MAKE) clean
	+$(MAKE) indent
	+$(MAKE) test
	+$(MAKE) valgrind
	+$(MAKE) profile

################
# Application specific targets

.PHONY: indent
indent:
	indent $(sources) $(tests) $(headers)

$(patsubst tests/%,%,$(tests:.c=)): %: build/%
ifeq ($(gdb), on)
	gdb $<
else ifeq ($(valgrind), on)
	$(VALGRIND) $(VALGRINDFLAGS) $< $(T)
else
	$< $(T)
endif

################
# Source transformations

mnt:
	mkdir -p $@

data:
	mkdir -p $@

build:
	mkdir -p $@

cov:
	mkdir -p $@

build/%: build/%.o | build
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.a: build/%.o | build
	$(AR) rcs $@ $^

$(addprefix build/,$(executables)): build/%: build/main_%.a build/%.a

$(patsubst tests/%.c,build/%,$(tests)): build/test_%: build/%.a build/tap.a

%.d: %.c
	$(CC) $(CFLAGS) -MM $< | sed -e 's!^\([^:]*\).o: \([^ ]*\).c!build/\1.o \2.d: \2.c!' > $@

-include $(sources:.c=.d) $(tests:.c=.d)
