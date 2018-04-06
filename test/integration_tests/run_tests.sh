#!/bin/bash


function dir() {
    DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    echo "$DIR"
}


source "$(dir)/../ansi_codes.sh"


function run_test() {
    PAD=$(printf '%0.1s' "."{1..80})
    printf "${_BOLD}%s" "$1  "
    $2
    DIFF=$(diff "$(dir)/$4" "$(dir)/$3")
    if [ "$DIFF" != "" ]; then
        printf "%*.*s" 0 $((67 - ${#1})) "$PAD"
        printf "  ${_RED}%s${ANSI_RESET}\n" "[FAILED]"
        echo "$DIFF" | tail -n +2
        return 1
    else
        printf "%*.*s" 0 $((66 - ${#1})) "$PAD"
        printf "  ${_GREEN}%s${ANSI_RESET}\n" "[SUCCESS]"
        return 0
    fi
}


function shutdown_background() {
    array=($(jobs -p))
    for ((i = ${#array[@]} - 1; i >= 0; i--)); do
        echo $i
        ps -p $i -o comm=
        kill -SIGINT ${array[i]} >/dev/null
        sleep 1.0
    done
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
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_udp.conf" &
    "$(dir)/logger.py" 20 10 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v1_packets_udp_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 \
        > "$(dir)/all_v1_packets_udp_to_serial.log" &
    sleep 1.0
    "$(dir)/packet_scripter.py" 3 10 "$(dir)/all_v1_packets.pks" \
        --udp 127.0.0.1:14500 --mavlink1
    sleep 1.0
    shutdown_background
}


function test_all_v2_packets_udp() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_udp.conf" &
    "$(dir)/logger.py" 20 10 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v2_packets_udp_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 \
        > "$(dir)/all_v2_packets_udp_to_serial.log" &
    sleep 1.0
    "$(dir)/packet_scripter.py" 3 10 "$(dir)/all_v2_packets.pks" \
        --udp 127.0.0.1:14500
    sleep 1.0
    shutdown_background
}


function test_all_v1_packets_serial() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_serial.conf" &
    "$(dir)/logger.py" 20 10 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v1_packets_serial_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 \
        > "$(dir)/all_v1_packets_serial_to_serial.log" &
    sleep 1.0
    "$(dir)/packet_scripter.py" 3 10 "$(dir)/all_v1_packets.pks" \
        --serial ./ttyS3 --mavlink1
    sleep 1.0
    shutdown_background
}


function test_all_v2_packets_serial() {
    socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
    socat pty,link=./ttyS2,raw,echo=0 pty,link=./ttyS3,raw,echo=0 &
    "$(dir)/../../build/mavtables" --conf "$(dir)/all_serial.conf" &
    "$(dir)/logger.py" 20 10 --udp 127.0.0.1:14500 \
        > "$(dir)/all_v2_packets_serial_to_udp.log" &
    "$(dir)/logger.py" 10 20 --serial ./ttyS1 \
        > "$(dir)/all_v2_packets_serial_to_serial.log" &
    sleep 1.0
    "$(dir)/packet_scripter.py" 3 10 "$(dir)/all_v2_packets.pks" \
        --serial ./ttyS3
    sleep 1.0
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
FAIL=0


function check_fail() {
    if [ "$?" -ne "0" ]; then
        FAIL=1
        # exit $FAIL;
    fi
}


run_test "Abstract Syntax Tree printing with --ast flag" \
    test_ast_printing \
    mavtables.cmp \
    mavtables.log
check_fail
run_test "Complex Abstract Syntax Tree printing with --ast flag" \
    test_complex_ast_printing \
    complex_config.cmp \
    complex_config.log
check_fail
run_test "MAVLink v1.0 packets from UDP to UDP" \
    test_all_v1_packets_udp \
    all_v1_packets.pks \
    all_v1_packets_udp_to_udp.log
check_fail
run_test "MAVLink v1.0 packets from UDP to serial port" \
    test_all_v1_packets_udp \
    all_v1_packets.pks \
    all_v1_packets_udp_to_serial.log
check_fail
run_test "MAVLink v2.0 packets from UDP to UDP" \
    test_all_v2_packets_udp \
    all_v2_packets.pks \
    all_v2_packets_udp_to_udp.log
check_fail
run_test "MAVLink v2.0 packets from UDP to serial port" \
    test_all_v2_packets_udp \
    all_v2_packets.pks \
    all_v2_packets_udp_to_serial.log
check_fail


run_test "MAVLink v1.0 packets from serial port to UDP" \
    test_all_v1_packets_serial \
    all_v1_packets.pks \
    all_v1_packets_serial_to_udp.log
check_fail
run_test "MAVLink v1.0 packets from serial port to serial port" \
    test_all_v1_packets_serial \
    all_v1_packets.pks \
    all_v1_packets_serial_to_serial.log
check_fail
run_test "MAVLink v2.0 packets from serial port to UDP" \
    test_all_v2_packets_serial \
    all_v2_packets.pks \
    all_v2_packets_serial_to_udp.log
check_fail
run_test "MAVLink v2.0 packets from serial port to serial port" \
    test_all_v2_packets_serial \
    all_v2_packets.pks \
    all_v2_packets_serial_to_serial.log
check_fail


if [ "$FAIL" -ne "0" ]; then
    exit $FAIL;
fi
