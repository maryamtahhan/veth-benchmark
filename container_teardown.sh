#!/bin/bash

basedir=`dirname $0`
source ${basedir}/lib/bash_functions.sh
# sudo auto detect
root_check_run_with_sudo "$@"

#####################################
# Cleanup
#####################################

docker stop cndp-1 cndp-2; docker rm cndp-1 cndp-2
unset mac_veth1
unset mac_veth2
unset mac_veth3
unset mac_veth4
unset mac_veth5
unset mac_veth6
unset mac_veth7
unset mac_veth8
ip link del veth1; ip link del veth3; ip link del veth5; ip link del veth7
ip link del br0
rm -rf /var/run/netns/*

