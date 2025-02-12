#!/bin/bash
LD_PRELOAD=/lib/arm-linux-gnueabihf/libasan.so.6 ./elf_injector $*
