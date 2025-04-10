#!/bin/sh

# $2 is data received

# output the new tbstate to the correct file
echo -n "34 00 00 01 00 00 02 00 00 00 41 42 41 42 41 42 43" > tbstate
