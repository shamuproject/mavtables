#!/bin/bash

socat pty,link=./ttyS0,raw,echo=0 pty,link=./ttyS1,raw,echo=0 &
PID=$!

./build/unit_tests

kill -SIGINT $PID
