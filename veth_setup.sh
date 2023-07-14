# !/bin/bash

basedir=`dirname $0`
source ${basedir}/lib/bash_functions.sh
# sudo auto detect
root_check_run_with_sudo "$@"

HOOK=N

while getopts "ns" flag; do
  case $flag in
    n) HOOK=N      ;;
    s) HOOK=S ;;
    *) echo 'error unkown flag' >&2
       exit 1
  esac
done
xdp-loader unload veth1 --all
xdp-loader unload veth7 --all
xdp-loader unload veth5 --all
xdp-loader unload veth3 --all
rm -rf /sys/fs/bpf/veth1
rm -rf /sys/fs/bpf/veth5
rm -rf /sys/fs/bpf/veth3
rm -rf /sys/fs/bpf/veth7
cd host-side-bpf/
./xdp_redirect_user -d veth1 -$HOOK
./xdp_pass_user -d veth7 -$HOOK
./xdp_redirect_user -d veth5 -$HOOK
./xdp_pass_user -d veth3 -$HOOK
# ./xdp_prog_user -d veth1 -r veth7 --src-mac (veth2) --dest-mac (veth8)
./xdp_prog_user -d veth1 -r veth7 --src-mac 02:a7:a2:bc:51:30 --dest-mac 3a:22:35:f4:7c:f5
# ./xdp_prog_user -d veth5 -r veth3 --src-mac (veth6) --dest-mac (veth4)
./xdp_prog_user -d veth5 -r veth3 --src-mac 2e:c3:a4:7f:18:b9 --dest-mac 36:da:e7:35:a9:bc
cd -
