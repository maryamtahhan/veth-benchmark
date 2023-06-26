# SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)

ifeq ("$(origin V)", "command line")
VERBOSE = $(V)
endif
ifndef VERBOSE
VERBOSE = 0
endif

ifeq ($(VERBOSE),0)
MAKEFLAGS += --no-print-directory
Q = @
endif

APPS = $(wildcard host-side-bpf*)
APPS_CLEAN = $(addsuffix _clean,$(APPS))
.PHONY: clean clobber distclean $(APPS) $(APPS_CLEAN)

all: lib $(APPS) docker-image
clean: $(APPS_CLEAN)
	@echo; echo common; $(MAKE) -C common clean
	@echo; echo lib; $(MAKE) -C lib clean

lib: config.mk check_submodule
	@echo; echo $@; $(MAKE) -C $@

$(APPS):
	@echo; echo $@; $(MAKE) -C $@

$(APPS_CLEAN):
	@echo; echo $@; $(MAKE) -C $(subst _clean,,$@) clean

config.mk: configure
	@sh configure

clobber:
	@touch config.mk
	$(Q)$(MAKE) clean
	$(Q)rm -f config.mk

distclean:	clobber

check_submodule:
	@if [ -d .git ] && `git submodule status lib/libbpf | grep -q '^+'`; then \
		echo "" ;\
		echo "** WARNING **: git submodule SHA-1 out-of-sync" ;\
		echo " consider running: git submodule update"  ;\
		echo "" ;\
	fi\

docker-image:
	docker build -t cndp-veth-bench -f containerization/Dockerfile .
