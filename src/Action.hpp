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


#ifndef ACTION_HPP_
#define ACTION_HPP_


#include <ostream>

#include "Packet.hpp"
#include "MAVAddress.hpp"
#include "RecursionChecker.hpp"


class Action
{
    protected:
        /** Print the action to the given output stream.
         *
         *  \param os The output stream to print to.
         *  \return The output stream.
         */
        virtual std::ostream &print_(std::ostream &os) const = 0;

    public:
        /** Filter rule actions.
         */
        enum Option
        {
            ACCEPT,     //!< Accept the packet.
            REJECT,     //!< Reject the packet.
            CONTINUE,   //!< Continue to next rule (no match).
            DEFAULT     //!< Fall back on global default action.
        };
        virtual ~Action();
        /** Determine what action to take with the given packet.
         *
         *  This is with respect to a given destination address.
         *
         *  \param packet The packet to determine whether to allow or not.
         *  \param address The address the \p packet will be sent out on if the
         *      action allows it.
         *  \param recursion_checker A recursion checker used to ensure infinite
         *      recursion does not occur.
         *  \retval true The packet is allowed to be sent to \p address.
         *  \retval false The packet is not allowed to be sent to \p address.
         */
        virtual Action::Option action(
            const Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const = 0;

        friend std::ostream &operator<<(std::ostream &os, const Action &action);
};


std::ostream &operator<<(std::ostream &os, const Action &action);


#endif // ACTION_HPP_
