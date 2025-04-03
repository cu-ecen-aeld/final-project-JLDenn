#!/bin/bash

ADDR="90:35:EA:0C:E4:91"

trap printout SIGINT
printout() {
    echo ""
    echo "Terminating"
    exit
}


/usr/bin/expect btpair.exp $ADDR

while [[ 1 ]]
do
	/usr/bin/expect gattctl.exp $ADDR
done

