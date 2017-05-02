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

################
# Sanity checks and local variables

################
# Exported variables

export DATE := $(date)

################
# Includes

################
# Standard targets

.PHONY: all
all:

.PHONY: run
run:

.PHONY: test
test:

.PHONY: check
check:

.PHONY: help
help:

.PHONY: clean
clean:

################
# Application specific targets

################
# Source transformations
