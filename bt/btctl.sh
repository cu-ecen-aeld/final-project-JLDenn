#!/bin/sh

PATH=`dirname $0`
if ! test -f "$PATH/bt.conf"; then
	exit 1;
fi 
source "$PATH/bt.conf"

trap printout SIGINT
printout() {
    echo ""
    echo "Terminating"
    exit
}

#indicate we've started
echo "0" > /tmp/btconn

/usr/bin/expect btpair.exp $ADDR

while [[ 1 ]]
do
	/usr/bin/expect gattctl.exp $ADDR
done

