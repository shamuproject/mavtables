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


#ifndef FILTER_HPP_
#define FILTER_HPP_


#include <memory>
#include <utility>

#include "Action.hpp"
#include "Chain.hpp"
#include "MAVAddress.hpp"
#include "Packet.hpp"
#include "config.hpp"


/** The filter used to determin wheather to accept or reject a packet.
 *
 *  \sa Chain
 *  \sa Rule
 *  \sa If
 */
class Filter
{
  public:
    /** Copy constructor.
     *
     *  \param other Filter to copy.
     */
    Filter(const Filter &other) = default;
    /** Move constructor.
     *
     *  \param other Filter to move from.
     */
    Filter(Filter &&other) = default;
    Filter(Chain default_chain, bool accept_by_default = false);
    // LCOV_EXCL_START
    TEST_VIRTUAL ~Filter() = default;
    // LCOV_EXCL_STOP
    TEST_VIRTUAL std::pair<bool, int>
    will_accept(const Packet &packet, const MAVAddress &address);
    /** Assignment operator.
     *
     * \param other Filter to copy.
     */
    Filter &operator=(const Filter &other) = default;
    /** Assignment operator (by move semantics).
     *
     * \param other Filter to move from.
     */
    Filter &operator=(Filter &&other) = default;

    friend bool operator==(const Filter &lhs, const Filter &rhs);
    friend bool operator!=(const Filter &lhs, const Filter &rhs);

  private:
    Chain default_chain_;
    bool accept_by_default_;
};


bool operator==(const Filter &lhs, const Filter &rhs);
bool operator!=(const Filter &lhs, const Filter &rhs);


#endif  // FILTER_HPP_
