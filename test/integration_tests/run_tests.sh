#!/bin/bash


FAIL=0


function dir() {
    DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    echo "$DIR"
}


source "$(dir)/../ansi_codes.sh"


function run_test() {
    PAD=$(printf '%0.1s' "."{1..80})
    printf "${_BOLD}%s${ANSI_RESET}" "$1  "
    $2
    DIFF=$(diff "$(dir)/$4" "$(dir)/$3")
    if [ "$DIFF" != "" ]; then
        printf "%*.*s" 0 $((67 - ${#1})) "$PAD"
        printf "  ${_BOLD}${_RED}%s${ANSI_RESET}\n" "[FAILED]"
        echo "$DIFF"
        FAIL=1
    else
        printf "%*.*s" 0 $((66 - ${#1})) "$PAD"
        printf "  ${_BOLD}${_GREEN}%s${ANSI_RESET}\n" "[SUCCESS]"
    fi
}


function shutdown_background() {
    array=($(jobs -p))
    for ((i = ${#array[@]} - 1; i >= 0; i--)); do
        kill -SIGINT ${array[i]} 2>/dev/null
        sleep 2
    done
}


function do_nothing() {
    echo "do nothing" >/dev/null
}


function test_ast_printing() {
    "$(dir)/../../build/mavtables" --ast --conf "$(dir)/../mavtables.conf" \
        | tail -n +2 > "$(dir)/mavtables.log"
}


function test_complex_ast_printing() {
    "$(dir)/../../build/mavtables" --ast \
        --conf "$(dir)/complex_config.conf" \
        | tail -n +2 > "$(dir)/complex_config.log"
}


function test_all_v1_packets_udp() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_1serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v1_packets_udp_to_udp.log" &
    "$(dir)/logger.py" 29 39 --serial ./ttyS1 \
        > "$(dir)/all_v1_packets_udp_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/all_v1_packets.pks" \
        --udp 127.0.0.1:14500 --mavlink1
    sleep 2
    shutdown_background
}


function test_all_v2_packets_udp() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_1serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v2_packets_udp_to_udp.log" &
    "$(dir)/logger.py" 29 39 --serial ./ttyS1 \
        > "$(dir)/all_v2_packets_udp_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/all_v2_packets.pks" \
        --udp 127.0.0.1:14500
    sleep 2
    shutdown_background
}


function test_all_v1_packets_serial() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_2serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v1_packets_serial_to_udp.log" &
    "$(dir)/logger.py" 29 39 --serial ./ttyS1 \
        > "$(dir)/all_v1_packets_serial_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/all_v1_packets.pks" \
        --serial ./ttyS3 --mavlink1
    sleep 2
    shutdown_background
}


function test_all_v2_packets_serial() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_2serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v2_packets_serial_to_udp.log" &
    "$(dir)/logger.py" 29 39 --serial ./ttyS1 \
        > "$(dir)/all_v2_packets_serial_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/all_v2_packets.pks" \
        --serial ./ttyS3
    sleep 2
    shutdown_background
}


function test_multiple_senders_v1_packets() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_2serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 --verbose \
        | sort > "$(dir)/multiple_senders_v1_packets_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 --verbose \
        | sort > "$(dir)/multiple_senders_v1_packets_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/all_v1_packets.pks" \
        --udp 127.0.0.1:14500 --mavlink1 &
    "$(dir)/packet_scripter.py" 172 128 "$(dir)/all_v1_packets.pks" \
        --udp 127.0.0.1:14500 --mavlink1 &
    "$(dir)/packet_scripter.py" 127 1 "$(dir)/all_v1_packets.pks" \
        --serial ./ttyS3 --mavlink1
    sleep 2
    shutdown_background
}


function test_multiple_senders_v2_packets() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_2serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 --verbose \
        | sort > "$(dir)/multiple_senders_v2_packets_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 --verbose \
        | sort > "$(dir)/multiple_senders_v2_packets_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/all_v2_packets.pks" \
        --udp 127.0.0.1:14500 &
    "$(dir)/packet_scripter.py" 172 128 "$(dir)/all_v2_packets.pks" \
        --udp 127.0.0.1:14500 &
    "$(dir)/packet_scripter.py" 127 1 "$(dir)/all_v2_packets.pks" \
        --serial ./ttyS3
    sleep 2
    shutdown_background
}


function test_large_packets() {
    perl -e 'for$i(1..5000){print "ENCAPSULATED_DATA\n"}' \
        > "$(dir)/large_packets.tmp"
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_2serial.conf" &
    "$(dir)/logger.py" 12 26 --udp 127.0.0.1:14500 \
        > "$(dir)/large_packets_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 \
        > "$(dir)/large_packets_to_serial.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 192 168 "$(dir)/large_packets.tmp" \
        --udp 127.0.0.1:14500 &
    "$(dir)/packet_scripter.py" 172 128 "$(dir)/large_packets.tmp" \
        --serial ./ttyS3
    sleep 2
    perl -e 'for$i(1..5000){print "ENCAPSULATED_DATA\n"}' \
        >> "$(dir)/large_packets.tmp"
    shutdown_background
}


function test_routing_v1_packets() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/routing.conf" &
    "$(dir)/logger.py" 127 1 --udp 127.0.0.1:14500 \
        > "$(dir)/routing_v1_packets_127.1.log" &
    "$(dir)/logger.py" 192 168 --udp 127.0.0.1:14500 \
        > "$(dir)/routing_v1_packets_192.168.log" &
    "$(dir)/logger.py" 172 128 --serial ./ttyS1 \
        > "$(dir)/routing_v1_packets_172.128.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 10 10 "$(dir)/routing.pks" \
        --udp 127.0.0.1:14500 --mavlink1
    sleep 2
    shutdown_background
}


function test_routing_v2_packets() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    sleep 0.5
    "$(dir)/../../build/mavtables" --conf "$(dir)/routing.conf" &
    "$(dir)/logger.py" 127 1 --udp 127.0.0.1:14500 \
        > "$(dir)/routing_v2_packets_127.1.log" &
    "$(dir)/logger.py" 192 168 --udp 127.0.0.1:14500 \
        > "$(dir)/routing_v2_packets_192.168.log" &
    "$(dir)/logger.py" 172 128 --serial ./ttyS1 \
        > "$(dir)/routing_v2_packets_172.128.log" &
    sleep 0.5
    "$(dir)/packet_scripter.py" 10 10 "$(dir)/routing.pks" \
        --udp 127.0.0.1:14500
    sleep 2
    shutdown_background
}


echo -en "${_BOLD}${_BLUE}*---------------------------------------"
echo -en "--------------------------------------*\n"
echo -en "${_BOLD}${_BLUE}|                              "
echo -en "Integration Tests"
echo -en "                              |\n"
echo -en "${_BOLD}${_BLUE}*---------------------------------------"
echo -en "--------------------------------------*\n"
echo -en "${ANSI_RESET}"


rm "$(dir)"/*.log 2>/dev/null


case "$OSTYPE" in
  solaris*)
      ;;
  darwin*)
      echo -en "${_BOLD}${_ITALICS}${_RED}Integration testing is not currently "
      echo -en "supported on Mac OS X.${ANSI_RESET}\n\n"
      exit 0
      ;;
  linux*)
      ;;
  bsd*)
      ;;
  *)
      echo "unknown operating system: $OSTYPE"
      exit 1
      ;;
esac


function check_fail() {
    if [ "$?" -ne "0" ]; then
        FAIL=1
    fi
}


run_test "Abstract Syntax Tree printing with --ast flag" \
    test_ast_printing \
    mavtables.cmp \
    mavtables.log
run_test "Complex Abstract Syntax Tree printing with --ast flag" \
    test_complex_ast_printing \
    complex_config.cmp \
    complex_config.log


run_test "All MAVLink v1.0 packets from UDP to UDP" \
    test_all_v1_packets_udp \
    all_v1_packets.pks \
    all_v1_packets_udp_to_udp.log
run_test "All MAVLink v1.0 packets from UDP to serial port" \
    do_nothing \
    all_v1_packets.pks \
    all_v1_packets_udp_to_serial.log
run_test "All MAVLink v2.0 packets from UDP to UDP" \
    test_all_v2_packets_udp \
    all_v2_packets.pks \
    all_v2_packets_udp_to_udp.log
run_test "All MAVLink v2.0 packets from UDP to serial port" \
    do_nothing \
    all_v2_packets.pks \
    all_v2_packets_udp_to_serial.log


run_test "All MAVLink v1.0 packets from serial port to UDP" \
    test_all_v1_packets_serial \
    all_v1_packets.pks \
    all_v1_packets_serial_to_udp.log
run_test "All MAVLink v1.0 packets from serial port to serial port" \
    do_nothing \
    all_v1_packets.pks \
    all_v1_packets_serial_to_serial.log
run_test "All MAVLink v2.0 packets from serial port to UDP" \
    test_all_v2_packets_serial \
    all_v2_packets.pks \
    all_v2_packets_serial_to_udp.log
run_test "All MAVLink v2.0 packets from serial port to serial port" \
    do_nothing \
    all_v2_packets.pks \
    all_v2_packets_serial_to_serial.log


run_test "Multiple senders with MAVLink v1.0 packets to UDP" \
    test_multiple_senders_v1_packets \
    multiple_senders_v1_packets.cmp \
    multiple_senders_v1_packets_to_udp.log
run_test "Multiple senders with MAVLink v1.0 packets to serial port" \
    do_nothing \
    multiple_senders_v1_packets.cmp \
    multiple_senders_v1_packets_to_serial.log
run_test "Multiple senders with MAVLink v2.0 packets to UDP" \
    test_multiple_senders_v2_packets \
    multiple_senders_v2_packets.cmp \
    multiple_senders_v2_packets_to_udp.log
run_test "Multiple senders with MAVLink v2.0 packets to serial port" \
    do_nothing \
    multiple_senders_v2_packets.cmp \
    multiple_senders_v2_packets_to_serial.log


run_test "Many large packets with multiple senders to UDP" \
    test_large_packets large_packets.tmp large_packets_to_udp.log
run_test "Many large packets with multiple senders to serial port" \
    do_nothing large_packets.tmp large_packets_to_serial.log


run_test "Routing MAVLink v1.0 packets (127.1)" \
    test_routing_v1_packets routing_127.1.cmp routing_v1_packets_127.1.log
run_test "Routing MAVLink v1.0 packets (192.168)" \
    do_nothing routing_192.168.cmp routing_v1_packets_192.168.log
run_test "Routing MAVLink v1.0 packets (172.128)" \
    do_nothing routing_172.128.cmp routing_v1_packets_172.128.log


run_test "Routing MAVLink v2.0 packets (127.1)" \
    test_routing_v2_packets routing_127.1.cmp routing_v2_packets_127.1.log
run_test "Routing MAVLink v2.0 packets (192.168)" \
    do_nothing routing_192.168.cmp routing_v2_packets_192.168.log
run_test "Routing MAVLink v2.0 packets (172.128)" \
    do_nothing routing_172.128.cmp routing_v2_packets_172.128.log

# test_routing_v1_packets
# run_test "Many large packets with multiple senders to serial port" \
#     test_large_packets large_packets.tmp large_packets_to_serial.log

if [ "$FAIL" -ne "0" ]; then
    echo -en "\n"
    exit $FAIL
fi
