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
#include <thread>

#include <RecursionError.hpp>
#include <RecursionData.hpp>
#include <RecursionGuard.hpp>


/** Construct a RecursionGuard.
 *
 *  This marks the given \ref RecursionData structure, ensuring it cannot
 *  be used to construct another guard.
 *
 *  \param data The object used to prevent recursion.
 *  \throws RecursionError if the given data has already been marked by a
 *      RecursionGuard that is still in scope.
 */
RecursionGuard::RecursionGuard(RecursionData &data)
    : data_(data)
{
    std::lock_guard<std::mutex> lock(data_.mutex_);
    std::thread::id id = std::this_thread::get_id();

    if (!(data_.calling_threads_.insert(id).second))
    {
        throw RecursionError("Recursion detected.");
    }
}


RecursionGuard::~RecursionGuard()
{
    std::lock_guard<std::mutex> lock(data_.mutex_);
    data_.calling_threads_.erase(std::this_thread::get_id());
}
