// MAVLink router and firewall.
// Copyright (C) 2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <mutex>
#include <iostream>
#include <iomanip>
#include <string>

#include "Logger.hpp"


void Logger::log(std::string message)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "  "
              << message << std::endl;
}


void Logger::level(unsigned int level)
{
    level_ = level;
}


unsigned int Logger::level()
{
    return level_;
}


unsigned int Logger::level_ = 0;


#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

std::mutex Logger::mutex_;

#ifdef __clang__
    #pragma clang diagnostic pop
#endif
