/* SPDX-License-Identifier: GPL-2.0 */
#include <stddef.h>
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>

struct {
    __uint(type, BPF_MAP_TYPE_XSKMAP);
    __type(key, __u32);
    __type(value, __u32);
    __uint(max_entries, 64);
} xsks_map SEC(".maps");

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

static __always_inline int parse_iphdr(struct hdr_cursor *nh,
                       void *data_end,
                       struct iphdr **iphdr)
{
    struct iphdr *iph = nh->pos;
    int hdrsize;

    if (iph + 1 > data_end)
        return -1;

    hdrsize = iph->ihl * 4;
    /* Sanity check packet field is valid */
    if(hdrsize < sizeof(*iph))
        return -1;

    /* Variable-length IPv4 header, need to use byte-based arithmetic */
    if (nh->pos + hdrsize > data_end)
        return -1;

    nh->pos += hdrsize;
    *iphdr = iph;

    return iph->protocol;
}

SEC("xdp")
int  xdp_filter_udp(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct ethhdr *eth;


    struct hdr_cursor nh;
    int nh_type, ip_type;
    struct iphdr *iph;
    nh.pos = data;

    nh_type = parse_ethhdr(&nh, data_end, &eth);
    if (nh_type == bpf_htons(ETH_P_IP)) {
        ip_type = parse_iphdr(&nh, data_end, &iph);
        if (ip_type == IPPROTO_UDP) {
            int index = ctx->rx_queue_index;

            /* A set entry here means that the corresponding queue_id
            * has an active AF_XDP socket bound to it. */
            if (bpf_map_lookup_elem(&xsks_map, &index))
                return bpf_redirect_map(&xsks_map, index, 0);
            }
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
