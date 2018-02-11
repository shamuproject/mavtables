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


#ifndef RECURSIONDATA_HPP_
#define RECURSIONDATA_HPP_


#include <mutex>
#include <set>
#include <thread>


/** A data structure used by RecursionGuard to detect unwanted recursion.
 *
 *  \note While RecursionData supports copy and move semantics both is
 *      constructors and assignment, a recursion data structure should never
 *      change instance.  One way to deal with this would have been to delete
 *      these operators but this would force users to manually implement copy
 *      and move semantics for their classes.  Therefore, RecursionData will
 *      always make a new data structure on copy, move or assignment, allowed
 *      default copy and move constructors/assignment operators to be created
 *      for classes using RecursionData.
 */
class RecursionData
{
        friend class RecursionGuard;

    private:
        std::set<std::thread::id> calling_threads_;
        std::mutex mutex_;
    public:
        RecursionData() = default;
        RecursionData(const RecursionData &other)
        {
            (void)other;
        }
        RecursionData(RecursionData &&other)
        {
            (void)other;
        }
        RecursionData &operator=(const RecursionData &other)
        {
            (void)other;
            calling_threads_.clear();
            return *this;
        }
        RecursionData &operator=(RecursionData &&other)
        {
            (void)other;
            calling_threads_.clear();
            return *this;
        }
};


#endif // RECURSIONDATA_HPP_
