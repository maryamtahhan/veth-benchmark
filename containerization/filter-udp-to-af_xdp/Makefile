# SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)

LLC ?= llc
CLANG ?= clang

all: myprog

myprog:
	$(CLANG) -S \
	-target bpf \
	-D __BPF_TRACING__ \
	-I/usr/include/bpf \
	-Wall \
	-Wno-unused-value \
	-Wno-pointer-sign \
	-Wno-compare-distinct-pointer-types \
	-Werror \
	-O2 -emit-llvm -c -g -o xdp_filter_udp_prog_kern.ll xdp_filter_udp_prog_kern.c
	$(LLC) -march=bpf -filetype=obj -mattr=dwarfris -o xdp_filter_udp_prog_kern.o xdp_filter_udp_prog_kern.ll

clean:
	rm -rf xdp_filter_udp_prog_kern.o xdp_filter_udp_prog_kern.ll
