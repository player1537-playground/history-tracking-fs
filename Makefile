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

python := python3.6

################
# Sanity checks and local variables

################
# Exported variables

export DATE := $(date)
export VIRTUAL_ENV_DISABLE_PROMPT := yes

################
# Includes

################
# Standard targets

.PHONY: all
all:

.PHONY: run
run: .depend.secondary | mnt data
	source venv/bin/activate && $(python) main.py mnt data

.PHONY: test
test:

.PHONY: depend
depend: .depend.secondary

.PHONY: check
check:

.PHONY: help
help:

.PHONY: clean
clean:

################
# Application specific targets

.venv.secondary:
	$(python) -m virtualenv venv
	touch $@

.depend.secondary: requirements.txt .venv.secondary
	source venv/bin/activate && $(python) -m pip install -r requirements.txt
	touch $@

################
# Source transformations

mnt:
	mkdir -p $@

data:
	mkdir -p $@
