#!/bin/bash


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $DIR

# socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
"$DIR/../../build/mavtables" --conf "$DIR/all.conf" &
"$DIR/logger.py" 20 10 --udp 127.0.0.1:14500 > all_v2_packets_udp.log &
# "$DIR/logger.py" 20 10 --serial ./ttyS1 > all_v2_packets_serial.log &
"$DIR/packet_scripter.py" 3 10 all_v2_packets.pks --udp 127.0.0.1:14500
sleep 1
kill $(jobs -p)


# Kill background jobs.

# kill -SIGINT $MAVTABLES_PID
# kill -SIGINT $LOGGER_PID
# kill -SIGINT $SCRIPTER_PID
# socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
# PID=$!
#
# ./build/unit_tests
# UNIT_TESTS_EXIT=$?
#

# if [ "$UNIT_TESTS_EXIT" -ne "0" ]; then
#     exit 1;
# fi
