mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

export PYTHONPATH=$(current_dir)

% : .build/Makefile
	@echo "Real build system"
	cd .build/ && env -u MAKELEVEL $(MAKE) $@
all:

test: all

.build/Makefile:
	@echo "Bootstraping build system"
	mkdir -p .build
	touch .build/CMakeCache.txt
	cd .build && env cmake ../

PHONY: all doc test
