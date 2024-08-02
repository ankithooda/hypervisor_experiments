#!/bin/bash

make all && bash build_disk.sh && qemu-system-x86_64 -hda image.hdd
