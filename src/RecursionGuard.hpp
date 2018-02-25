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


#ifndef RECURSIONGUARD_HPP_
#define RECURSIONGUARD_HPP_


#include <mutex>
#include <thread>

#include "RecursionData.hpp"


/** Guard against recursion.
 *
 *  A recursion guard is an RAII (Resource Acquisition Is Initialization) data
 *  structure used to raise an error upon recursion.  The constructor marks a
 *  \ref RecursionData structure, acquiring ownership of the containing function
 *  (within the given thread).  Recursion guards treat calls from different
 *  threads and different, therefore, it will not guard against reentrancy.
 *
 *  An example of how to use this is:
 *
 *  ```
 *  #include "RecursionGuard.hpp"
 *
 *  int a_function(int value)
 *  {
 *      // shared data between calls
 *      static RecursionData rdata;
 *      // take ownership of the call
 *      RecursionGuard rguard(rdata);
 *      return b_function(value);
 *      // the recursion guard is released upon destruction of rguard
 *  }
 *  ```
 *
 *  If `b_function`, or any function it calls, ever calls `a_function` then this
 *  will throw \ref RecursionError.  However, if multiple threads call
 *  `a_function` (possibly at the same time) but `b_function` does not call
 *  `a_function` then no error will be thrown.
 *
 *  \sa RecursionError
 */
class RecursionGuard
{
    public:
        RecursionGuard(RecursionData &data);
        ~RecursionGuard();

    private:
        RecursionData &data_;
};


#endif // RECURSIONGUARD_HPP_
