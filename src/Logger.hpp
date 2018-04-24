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


#ifndef LOGGER_HPP_
#define LOGGER_HPP_


#include <mutex>
#include <string>


class Logger
{
    public:
        static void log(std::string message);
        static void level(unsigned int level);
        static unsigned int level();
        Logger(const Logger &logger) = delete;
        void operator=(const Logger &logger) = delete;

    private:
        Logger() = default;
        static unsigned int level_;
        static std::mutex mutex_;
};


#endif // LOGGER_HPP_
