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


#ifndef SEMAPHORE_HPP_
#define SEMAPHORE_HPP_


#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>


/** A weak semaphore implementation.
 *
 *  \note This uses a different naming scheme than the rest of this project
 *      because it is intended to have the same feel as the standard library.
 *
 *  \note This semaphore implementation is based on
 *      https://gist.github.com/sguzman/9594227
 */
class semaphore
{
    public:
        semaphore(const semaphore &other) = delete;
        semaphore(semaphore &&other) = delete;
        semaphore(size_t initial_value = 0);
        void notify();
        void wait();
        template<class Rep, class Period>
        bool wait_for(const std::chrono::duration<Rep, Period> &rel_time);
        template<class Clock, class Duration>
        bool wait_until(
            const std::chrono::time_point<Clock, Duration> &timeout_time);
        semaphore &operator=(const semaphore &other) = delete;
        semaphore &operator=(semaphore &&other) = delete;

    private:
        size_t value_;
        std::mutex mutex_;
        std::condition_variable cv_;
};


/** Wait on the semaphore, or a given duration timeout.
 *
 *  \param rel_time The duration of the timeout.
 *  \retval true The semaphore has been successfully decremented.
 *  \retval false The semaphore timed out.
 */
template<class Rep, class Period>
bool semaphore::wait_for(const std::chrono::duration<Rep, Period> &rel_time)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, rel_time, [this]()
    {
        return value_ > 0;
    });

    if (result)
    {
        --value_;
    }

    return result;
}


/** Wait on the semaphore, or a given timepoint timeout.
 *
 *  \param timeout_time The timepoint for the timeout.
 *  \retval true The semaphore has been successfully decremented.
 *  \retval false The semaphore timed out.
 */
template<class Clock, class Duration>
bool semaphore::wait_until(
    const std::chrono::time_point<Clock, Duration> &timeout_time)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_until(lock, timeout_time, [this]()
    {
        return value_ > 0;
    });

    if (result)
    {
        --value_;
    }

    return result;
}


#endif // SEMAPHORE_HPP_
