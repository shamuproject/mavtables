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


#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>

#include "Logger.hpp"


/** Log a message with timestamp (at level 1).
 *
 *  This will log a message with the current date and time as the timestamp if
 *  the loglevel is set to at least 1.
 *
 *  \note This is the only method (along with \ref log(unsigned int,
 *      std::string)) that is threadsafe.
 *
 *  \param message The message to log.
 */
void Logger::log(std::string message)
{
    Logger::log(1, std::move(message));
}


/** Log a message with timestamp at the given level.
 *
 *  This will log a message with the current date and time as the timestamp if
 *  the loglevel is at least \p level.
 *
 *  \note This is the only method (along with \ref log(std::string)) that is
 *      threadsafe.
 *
 *  \param level The level to log the \p message at.  If the logger's level is
 *      lower than this, the message will be discarded.  The valid range is 1 to
 *      65535, a value of 0 will be corrected to 1.
 *  \param message The message to log.
 *  \remarks
 *      Threadsafe (locking).
 */
void Logger::log(unsigned int level, std::string message)
{
    if (level < 1)
    {
        level = 1;
    }
    if (level_ >= level)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "  "
                  << message << std::endl;
    }
}


/** Set the logging level.
 *
 *  A higher level indicates a higher verbosity of logging.  A level of 0 will
 *  completely disable logging.
 *
 *  \param level The new logging level, valid values are 0 to 65535.
 */
void Logger::level(unsigned int level)
{
    level_ = level;
}


/** Get the logging level.
 *
 *  \note It is recommended to check the level before constructing a log message
 *      if the message is expensive to construct.
 *
 *  \returns The current logging level.
 */
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
