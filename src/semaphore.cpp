// MAVLink router and firewall.
// Copyright (C) 2017-2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
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


#include <cstdint>
#include <mutex>

#include <semaphore.hpp>


/** Construct a semaphore with the given initial value.
 *
 *  \param initial_value The initial value of the semaphore.
 */
semaphore::semaphore(size_t initial_value) : value_(initial_value) {}


/** Signal the semaphore.
 */
void semaphore::notify()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++value_;
    }
    cv_.notify_one();
}


/** Wait on the semaphore.
 */
void semaphore::wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return value_ > 0; });
    --value_;
}
