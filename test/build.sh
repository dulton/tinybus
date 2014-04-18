#!/bin/sh

gcc -g -o test_trace test_trace.c -I/home/zhangsy/usr/x86/debug/tinybus/include -L/home/zhangsy/usr/x86/debug/tinybus/lib -ltinybus -lpthread -lrt
gcc -g -o test_bus test_bus.c -I. -I/home/zhangsy/usr/x86/debug/tinybus/include -L/home/zhangsy/usr/x86/debug/tinybus/lib -ltinybus -lpthread -lrt
