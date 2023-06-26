# SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)

XDP_TARGETS  := xdp_prog_kern_redirect xdp_prog_kern_pass
USER_TARGETS := xdp_prog_user xdp_redirect_user xdp_pass_user

LLC ?= llc
CLANG ?= clang
CC := gcc

COMMON_DIR = ../common

EXTRA_DEPS  += $(COMMON_DIR)/parsing_helpers.h

COMMON_OBJS += $(COMMON_DIR)/common_user_bpf_xdp.o
include $(COMMON_DIR)/common.mk
