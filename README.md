# veth-benchmark

The test case run when trying to benchmark the performance of veth with
native AF_XDP is as follows:

```bash
#    +-------------------+                         +------------------+
#    |      cndp-1       |                         |      cndp-2      |
#    | +--------------+  |                         | +--------------+ |
#    | |              |  |                         | |              | |
#    | |    TXGEN     |  |                         | |    CNET      | |
#    | |              |  |                         | |              | |
#    | +=====+--+=====+  |                         | +=====+--+=====+ |
#    | |veth2|  |veth4|  |                         | |veth6|  |veth8| |
#    | +==|==+  +==|==+  |                         | +==|==+  +==|==+ |
#    +----|--------|-----+                         +----|--------|----+
#      +==|==+  +==|==+                              +==|==+  +==|==+
#      |veth1|  |veth3|                              |veth5|  |veth7|
#      +==|==+  +==^==+                              +==|==+  +==^==+
#         |        |______redirect veth 5 to veth3______|        |
#         |______________redirect veth 1 to veth7________________|
```

Where:

- Txgen: is an af_xdp based traffic generator
- Cnet(-graph) is a lightweight AF_XDP based networking stack.

## veth benchmarking setup

### Building the bpf progs and the container image

Start by [ensuring Docker is installed](https://docs.docker.com/engine/install/)

Build the relevant xdp-progs, applications and container image by running

```cmd
# ./configure
# make
```

> **_NOTE:_** Tool `xdp-loader` is available in `xdp-tools` package.

### Setting up the containers

Setup the containers by running the `container_setup.sh` script. This will
create the following setup:

```bash
#    +-------------------+                         +------------------+
#    |      cndp-1       |                         |      cndp-2      |
#    |                   |                         |                  |
#    | +=====+  +=====+  |                         | +=====+ +=====+  |
#    | |veth2|  |veth4|  |                         | |veth6| |veth8|  |
#    | +==|==+  +==|==+  |                         | +==|==+ +==|==+  |
#    +----|--------|-----+                         +----|-------|-----+
#      +==|==+  +==|==+                              +==|==+ +==|==+
#      |veth1|  |veth3|                              |veth5| |veth7|
#      +==|==+  +==|==+                              +==|==+ +==|==+
#         |        |                                    |       |
#         |        |                                    |       |
#       +-|--------|------------------------------------|-------|-+
#       |                     br-0                                |
#       +---------------------------------------------------------+
```

### Setting xdp progs on the host side veths

Run the `veth_setup.sh` script to:

- Install the xdp-redirection program on veth1 and veth5.
- Install the xdp-pass program on veth7 and veth3.

> **_NOTE:_** Modify the MAC addresses in the script as appropriate.

```cmd
# ./veth_setup.sh -n
```

```bash
#    +-------------------+                         +------------------+
#    |      cndp-1       |                         |    cndp-2     |
#    |                   |                         |                  |
#    | +=====+  +=====+  |                         | +=====+ +=====+  |
#    | |veth2|  |veth4|  |                         | |veth6| |veth8|  |
#    | +==|==+  +==|==+  |                         | +==|==+ +==|==+  |
#    +----|--------|-----+                         +----|-------|-----+
#      +==|==+  +==|==+                              +==|==+ +==|==+
#      |veth1|  |veth3|                              |veth5| |veth7|
#      +==|==+  +==^==+                              +==|==+ +==^==+
#         |        |______redirect veth 5 to veth3______|       |
#         |______________redirect veth 1 to veth7_______________|
```

### Run the applications in the containers

Connect to both containers in separate terminals and run the cndp applications.

#### txgen

```cmd
# docker exec -ti cndp-1 /cndp/builddir/usrtools/txgen/app/txgen -c /cndp/builddir/usrtools/txgen/app/txgen.jsonc
```

Configure the txgen app by inputting the following at the `TXGen:/>` prompt:

```bash
set 0 dst mac 3a:22:35:f4:7c:f5


```

> **_NOTE:_** Modify the MAC addresses as appropriate. Set the DST mac to veth8 mac address

To start traffic use:

```cmd
TXGen:/> start 0
```

To stop traffic use:

```cmd
TXGen:/> stp
```

#### cnet-graph

```cmd
$ docker exec -ti cndp-2 ./run_cnet.sh
(mmap_alloc              : 203) WARNING: unable to allocate 2MB hugepages, trying 4KB pages
parse_sysfs_value(): Warning cannot open sysfs value /sys/class/net/veth6/device/numa_node
(xskdev_load_custom_xdp_prog: 791) INFO: Successfully loaded XDP program xdp_filter_udp_prog_kern.o with fd 5
(xskdev_socket_create    : 953) INFO: UMEM shared memory is enabled for veth6:0
parse_sysfs_value(): Warning cannot open sysfs value /sys/class/net/veth8/device/numa_node
(xskdev_load_custom_xdp_prog: 791) INFO: Successfully loaded XDP program xdp_filter_udp_prog_kern.o with fd 9
(xskdev_socket_create    : 953) INFO: UMEM shared memory is enabled for veth8:0
(parse_args              : 339) INFO: *** Mode type was not set in json file or command line, use drop mode ***

*** CNET-GRAPH Application, Mode: Drop, Burst Size: 128

*** cnet-graph, PID: 35 lcore: 0
(thread_func             : 310) ERR: Unable to find graph:0 option name

** Version: CNDP 22.08.0, Command Line Interface
CNDP-cli:/>
```

to see the stats for 5 seconds use the gstats command:

```bash
*** CNET-GRAPH Application, Mode: Drop, Burst Size: 128

*** cnet-graph, PID: 49 lcore: 0
(chnl_open               : 163) ERR:  TCP is disabled

** Version: CNDP 22.08.0, Command Line Interface
CNDP-cli:/> gstats 5
+------------------+---------------+---------------+--------+--------+----------+------------+
|Node              |          Calls|        Objects| Realloc|  Objs/c|   KObjs/c|    Cycles/c|
+------------------+---------------+---------------+--------+--------+----------+------------+
|ip4_input         |              0|              0|       1|     0.0|       0.0|         0.0|
|ip4_output        |              0|              0|       1|     0.0|       0.0|         0.0|
|ip4_forward       |              0|              0|       1|     0.0|       0.0|         0.0|
|ip4_proto         |              0|              0|       1|     0.0|       0.0|         0.0|
|udp_input         |              0|              0|       1|     0.0|       0.0|         0.0|
|udp_output        |              0|              0|       1|     0.0|       0.0|         0.0|
|pkt_drop          |              0|              0|       1|     0.0|       0.0|         0.0|
|chnl_callback     |              0|              0|       1|     0.0|       0.0|         0.0|
|chnl_recv         |              0|              0|       1|     0.0|       0.0|         0.0|
|kernel_recv       |        9981680|              0|       2|     0.0|       0.0|      1685.0|
|eth_rx-0          |        9981726|              0|       2|     0.0|       0.0|        74.0|
|eth_rx-1          |        9981767|              0|       2|     0.0|       0.0|       138.0|
|arp_request       |              0|              0|       1|     0.0|       0.0|         0.0|
|eth_tx-0          |              0|              0|       1|     0.0|       0.0|         0.0|
|eth_tx-1          |              0|              0|       1|     0.0|       0.0|         0.0|
|punt_kernel       |              0|              0|       1|     0.0|       0.0|         0.0|
|ptype             |              0|              0|       1|     0.0|       0.0|         0.0|
|gtpu_input        |              0|              0|       1|     0.0|       0.0|         0.0|
+------------------+---------------+---------------+--------+--------+----------+------------+
```

> **_NOTE:_** you might need to modify the lcore stanzas in the cndp jsonc files.

### Debug

If you are seeing the cnet stats incrementing but no throughput on the second txgen port
you can always check the dst mac address on frames being sent out veth5 using:

```cmd
$ xdpdump -i veth5 -w - | tcpdump -r - -en
```

then adjust the `veth_setup.sh` to modify the section that redirects from veth5 to veth3:

```cmd
$ ./xdp_prog_user -d veth5 -r veth3 --src-mac 2e:c3:a4:7f:18:b9 --dest-mac <update-this-value>
```

### ip link setup for example

```cmd
$ docker exec -ti cndp-1 ip link
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
1094: eth0@if1095: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT group default
    link/ether 02:42:ac:11:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
1098: veth2@if1099: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether 02:a7:a2:bc:51:30 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    prog/xdp id 1133 name  tag 03b13f331978c78c jited
1100: veth4@if1101: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether 36:da:e7:35:a9:bc brd ff:ff:ff:ff:ff:ff link-netnsid 0
    prog/xdp id 1136 name  tag 03b13f331978c78c jited
```

```cmd
$ docker exec -ti cndp-2 ip link
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
1096: eth0@if1097: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT group default
    link/ether 02:42:ac:11:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 0
1103: veth6@if1104: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether 2e:c3:a4:7f:18:b9 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    prog/xdp id 1126 name xdp_filter_udp tag d7ed399a3651d5bb jited
1105: veth8@if1106: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue state UP mode DEFAULT group default qlen 1000
    link/ether 3a:22:35:f4:7c:f5 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    prog/xdp id 1129 name xdp_filter_udp tag d7ed399a3651d5bb jited
```

```cmd
1099: veth1@if1098: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue master br0 state UP mode DEFAULT group default qlen 1000
    link/ether 66:61:74:2a:ef:76 brd ff:ff:ff:ff:ff:ff link-netns 252529
    prog/xdp id 1079 name xdp_dispatcher tag 94d5f00c20184d17 jited
1101: veth3@if1100: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue master br0 state UP mode DEFAULT group default qlen 1000
    link/ether 96:b6:82:62:fd:0c brd ff:ff:ff:ff:ff:ff link-netns 252529
    prog/xdp id 1115 name xdp_dispatcher tag 94d5f00c20184d17 jited
1104: veth5@if1103: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue master br0 state UP mode DEFAULT group default qlen 1000
    link/ether 62:66:51:4a:47:9d brd ff:ff:ff:ff:ff:ff link-netns 252843
    prog/xdp id 1103 name xdp_dispatcher tag 94d5f00c20184d17 jited
1106: veth7@if1105: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 xdp qdisc noqueue master br0 state UP mode DEFAULT group default qlen 1000
    link/ether 82:4a:64:0a:11:16 brd ff:ff:ff:ff:ff:ff link-netns 252843
    prog/xdp id 1091 name xdp_dispatcher tag 94d5f00c20184d17 jite
```
