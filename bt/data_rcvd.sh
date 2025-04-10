#!/bin/sh

DATACTRL="."
# $2 is data received

# output the new tbstate to the correct file
$DATACTRL_PATH/datctrl "$2" > tbstate
