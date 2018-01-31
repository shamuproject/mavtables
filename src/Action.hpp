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


/** Base class of all rule actions.
 *
 *  These are used as the actions of a \ref Rule to determine what to do with a
 *  \ref Packet if it matches the \ref Rule.
 */
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
            ACCEPT,     //!< The packet has been accepted and should be
            //!< delivered to the given \p address.
            REJECT,     //!< The packet has been rejected and should not be
            //!< delivered to the given \p address.
            CONTINUE,   //!< Whether the packet will be accepted or rejected
            //!< has not yet been decided and rule checking should
            //!< continue.
            DEFAULT     //!< Whether the packet will be accepted or rejected
            //!< should be decided by the global default action.
        };
        virtual ~Action();  // Clang does not like pure virtual destructors.
        /** Decide what to do with a \ref Packet.
         *
         *  Determine what action to take with the given \p packet sent to the
         *  given \p address.  The possible actions are documented in the \ref
         *  Option enum.
         *
         *  \param packet The packet to determine whether to allow or not.
         *  \param address The address the \p packet will be sent out on if the
         *      action allows it.
         *  \param recursion_checker A recursion checker used to ensure infinite
         *      recursion does not occur.
         *  \retval Action::ACCEPT The packet is allowed to be sent to \p
         *      address.
         *  \retval Action::REJECT The packet is not allowed to be sent to \p
         *      address.
         *  \retval Action::CONTINUE The action to take on packet is not decided
         *      yet.
         *  \retval Action::DEFAULT Use the global default action.
         *
         *  The packet is not allowed to be sent to \p address.
         */
        virtual Action::Option action(
            const Packet &packet, const MAVAddress &address,
            RecursionChecker &recursion_checker) const = 0;

        friend std::ostream &operator<<(std::ostream &os, const Action &action);
};


std::ostream &operator<<(std::ostream &os, const Action &action);


#endif // ACTION_HPP_
