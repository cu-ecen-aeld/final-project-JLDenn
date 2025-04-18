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

echo "Starting $PATH/btpair.exp" >> /var/log/btctl.log
/usr/bin/expect "$PATH/btpair.exp" $ADDR

while [[ 1 ]]
do
	/usr/bin/expect "$PATH/gattctl.exp" $ADDR
	echo "retrying $PATH/gattctl.exp" >> /var/log/btctl.log
done

