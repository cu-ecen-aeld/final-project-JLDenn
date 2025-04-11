#!/bin/sh

DATACTRL_PATH="."
# $2 is data received

# output the new tbstate to the correct file
$DATACTRL_PATH/datactrl scan "$2"
