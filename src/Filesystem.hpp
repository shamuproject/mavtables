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


#ifndef FILESYSTEM_HPP_
#define FILESYSTEM_HPP_


#include <memory>
#include <optional>
#include <string>

#include <boost/filesystem.hpp>

#include "config.hpp"


/** A class containing filesystem operations.
 *
 *  \note This class exists for testing purposes and to provide a level of
 *      indirection so the underlying filesystem library can be changed to
 *      std::filesystem once it is moved out of experimental.
 */
class Filesystem
{
    public:
        using path = boost::filesystem::path;
        TEST_VIRTUAL ~Filesystem();
        TEST_VIRTUAL bool exists(const path &p) const;
};


#endif  // FILESYSTEM_HPP_
