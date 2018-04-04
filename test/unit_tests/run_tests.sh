#!/bin/bash

socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
PID=$!
sleep 1

./build/unit_tests
UNIT_TESTS_EXIT=$?

kill -SIGINT $PID

if [ "$UNIT_TESTS_EXIT" -ne "0" ]; then
    exit 1;
fi
