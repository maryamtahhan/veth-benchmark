/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/bpf.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <netinet/ether.h>

// Comment out or #undef to disable debugging bpf_printk
//#define _DEBUG 1
//#undef _DEBUG

/* Header cursor to keep track of current parsing position */
struct hdr_cursor {
    void *pos;
};


static __always_inline int parse_ethhdr(struct hdr_cursor *nh,
                    void *data_end,
                    struct ethhdr **ethhdr)
{
    struct ethhdr *eth = nh->pos;
    int hdrsize = sizeof(*eth);
    __u16 h_proto;

    /* Byte-count bounds check; check if current pointer + size of header
     * is after data_end.
     */
    if (nh->pos + hdrsize > data_end)
        return -1;

    nh->pos += hdrsize;
    *ethhdr = eth;
    h_proto = eth->h_proto;

    return h_proto; /* network-byte-order */
}


struct {
	__uint(type, BPF_MAP_TYPE_DEVMAP);
	__type(key, int);
	__type(value, int);
	__uint(max_entries, 256);
	__uint(pinning, LIBBPF_PIN_BY_NAME);
} tx_port SEC(".maps");


struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__type(key,  unsigned char [ETH_ALEN]);
	__type(value, unsigned char [ETH_ALEN]);
	__uint(max_entries, 64);
	__uint(pinning, LIBBPF_PIN_BY_NAME);
} redirect_params SEC(".maps");

/* Solution to packet03/assignment-3 */
SEC("xdp")
int xdp_prog_redirect(struct xdp_md *ctx)
{
	void *data_end = (void *)(long)ctx->data_end;
	void *data = (void *)(long)ctx->data;
	struct hdr_cursor nh;
	struct ethhdr *eth;
	int eth_type;
	unsigned char *dst;
    int *value;
	int index = 0;

	/* These keep track of the next header type and iterator pointer */
	nh.pos = data;

	/* Parse Ethernet and IP/IPv6 headers */
	eth_type = parse_ethhdr(&nh, data_end, &eth);
	if (eth_type == -1){
#ifdef _DEBUG
//      cat /sys/kernel/_DEBUG/tracing/trace_pipe
		bpf_printk("Dont know the ethtype");
#endif
		goto out;
	}

	/* Do we know where to redirect this packet? */
	dst = bpf_map_lookup_elem(&redirect_params, eth->h_source);
	if (!dst) {
#ifdef _DEBUG
		bpf_printk("bpf_map_lookup_elem failed");
#endif
		goto out;
	}

	value = bpf_map_lookup_elem(&tx_port, &index);
	if(!value){
#ifdef _DEBUG
		bpf_printk("bpf_map_lookup_elem tx_port failed");
#endif
		goto out;
	}

#ifdef _DEBUG
	bpf_printk("REDIRECTING PACKET to ifindex %d", *value);
#endif

	return bpf_redirect_map(&tx_port, 0, 0);

out:
#ifdef _DEBUG
    bpf_printk("XDP_PASS");
#endif
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
