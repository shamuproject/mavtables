#!/bin/bash


function dir() {
    DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    echo "$DIR"
}


source "$(dir)/../ansi_codes.sh"


function shutdown_background() {
    array=($(jobs -p))
    for ((i = ${#array[@]} - 1; i >= 0; i--)); do
        kill -SIGINT ${array[i]} >/dev/null
        sleep 0.5
    done
}


echo -en "${_BOLD}${_BLUE}*---------------------------------------"
echo -en "--------------------------------------*\n"
echo -en "${_BOLD}${_BLUE}|                                  "
echo -en "Unit Tests"
echo -en "                                 |\n"
echo -en "${_BOLD}${_BLUE}*---------------------------------------"
echo -en "--------------------------------------*\n"
echo -en "${ANSI_RESET}"


socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
sleep 3

./build/unit_tests
UNIT_TESTS_EXIT=$?
sleep 1

shutdown_background

if [ "$UNIT_TESTS_EXIT" -ne "0" ]; then
    exit 1;
fi
