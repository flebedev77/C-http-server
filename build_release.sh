#!/bin/sh

make clean
make CC=gcc -j4 CFLAGS="-Wall -Wextra -Werror -O2"
