#!/bin/bash

# Copyright 2017  Michael R. Shannon <mrshannon.aerospace@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# BASH library that provides variables for the ANSI escape codes.
#
# To use this library simply source the file:
#
#       $ source ansi_codes
#
# Codes to turn on a feature begin with and underscore while those to disable
# the feature end in an underscore.
#
# Example:
#
#       $ echo "This is ${_BOLD}bold${BOLD_}."
#       $ echo "This is ${_RED}red${RED_}."
#       $ echo "This is ${_BOLD}${_RED}bold and red${RED_}${BOLD_}."


# Reset all ANSI codes.
ANSI_RESET="\e[0m"

# Font properties.
_BOLD="\e[1m"
BOLD_="\e[21m"
_ITALICS="\e[3m"
ITALICS_="\e[23m"
_UNDERLINE_="\e[4m"
UNDERLINE_="\e[24m"
_INVERSE="\e[7m"
INVERSE_="\e[27m"
_STRIKE="\e[9m"
STRIKE_="\e[29m"

# Forground colors.
_BLACK="\e[30m"
BLACK_="\e[39m"
_RED="\e[31m"
RED_="\e[39m"
_GREEN="\e[32m"
GREEN_="\e[39m"
_YELLOW="\e[33m"
YELLOW_="\e[39m"
_BLUE="\e[34m"
BLUE_="\e[39m"
_MAGENTA="\e[35m"
MAGENTA_="\e[39m"
_CYAN="\e[36m"
CYAN_="\e[39m"
_WHITE="\e[37m"
WHITE_="\e[39m"
FG_RESET="\e[39m"

# Background colors.
_BG_BLACK="\e[40m"
BG_BLACK_="\e[49m"
_BG_RED="\e[41m"
BG_RED_="\e[49m"
_BG_GREEN="\e[42m"
BG_GREEN_="\e[49m"
_BG_YELLOW="\e[43m"
BG_YELLOW_="\e[49m"
_BG_BLUE="\e[44m"
BG_BLUE_="\e[49m"
_BG_MAGENTA="\e[45m"
BG_MAGENTA_="\e[49m"
_BG_CYAN="\e[46m"
BG_CYAN_="\e[49m"
_BG_WHITE="\e[47m"
BG_WHITE_="\e[49m"
BG_RESET="\e[49m"
