#!/bin/bash

#####################################
# Cleanup
#####################################

docker stop cndp-1 cndp-2; docker rm cndp-1 cndp-2
ip link del veth1; ip link del veth3; ip link del veth5; ip link del veth7
ip link del br0
rm -rf /var/run/netns/*
