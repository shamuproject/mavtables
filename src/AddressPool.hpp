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


#ifndef ADDRESSPOOL_HPP_
#define ADDRESSPOOL_HPP_


#include <chrono>
#include <map>
#include <mutex>
#include <vector>

#include "Logger.hpp"
#include "config.hpp"
#include "MAVAddress.hpp"
#include "util.hpp"


/** A threadsafe container for addresses that expire after a given time.
 */
template <class TC = std::chrono::steady_clock>
class AddressPool
{
    public:
        AddressPool(std::chrono::milliseconds timeout =
                        std::chrono::milliseconds(120000));
        // LCOV_EXCL_START
        TEST_VIRTUAL ~AddressPool() = default;
        // LCOV_EXCL_STOP
        TEST_VIRTUAL void add(MAVAddress address);
        TEST_VIRTUAL std::vector<MAVAddress> addresses();
        TEST_VIRTUAL bool contains(const MAVAddress &address);

    private:
        std::map<MAVAddress, std::chrono::time_point<TC>> addresses_;
        std::chrono::milliseconds timeout_;
        std::mutex mutex_;
};


/** Construct a new address pool.
 *
 *  \param timeout The amount of time (in milliseconds) before a component will
 *      be considered offline and removed from the pool, unless its time is
 *      updated with \ref add.
 */
template <class TC>
AddressPool<TC>::AddressPool(std::chrono::milliseconds timeout)
    : timeout_(std::move(timeout))
{
}


/** Add a MAVLink address to the pool.
 *
 *  \note Addresses will be removed after the timeout (set in the
 *      constructor) has run out.  Re-adding the address (even before this time
 *      runs out) will reset the timeout.
 *
 *  \param address The MAVLink address to add or update the timeout for.
 *  \remarks
 *      Threadsafe (locking).
 */
template <class TC>
void AddressPool<TC>::add(MAVAddress address)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (Logger::level() == 1 && addresses_.find(address) == addresses_.end())
    {
        Logger::log("New component " + str(address));
    }
    addresses_.insert_or_assign(std::move(address), TC::now());
}


/** Get a vector of all the addresses in the pool.
 *
 *  \note A copy is returned instead of using iterators in order to make the
 *      call thread safe.
 *
 *  \returns A vector of the addresses in the pool.
 *  \remarks
 *      Threadsafe (locking).
 */
template <class TC>
std::vector<MAVAddress> AddressPool<TC>::addresses()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MAVAddress> addresses;
    addresses.reserve(addresses_.size());
    auto current_time = TC::now();

    // Loop over addresses.
    for (auto it = addresses_.cbegin(); it != addresses_.cend();)
    {
        // Remove the address if it has expired.
        if ((current_time - it->second) > timeout_)
        {
            it = addresses_.erase(it);
        }
        // Store the address.
        else
        {
            addresses.push_back((it++)->first);
        }
    }

    return addresses;
}


/** Determine if the pool contains a given MAVLink address.
 *
 *  \param address The MAVLink address to test for.
 *  \retval true If the pool contains \p address.
 *  \retval false If the pool does not contain \p address.
 *  \remarks
 *      Threadsafe (locking).
 */
template <class TC>
bool AddressPool<TC>::contains(const MAVAddress &address)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = addresses_.find(address);

    if (it != addresses_.end())
    {
        auto current_time = TC::now();

        if (current_time - it->second > timeout_)
        {
            addresses_.erase(it);
            return false;
        }

        return true;
    }

    return false;
}


#endif // ADDRESSPOOL_HPP_
