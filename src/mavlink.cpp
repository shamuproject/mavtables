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


#include <cstdint>
#include <stdexcept>
#include <string>

#include "mavlink.hpp"


namespace mavlink
{

    /** Get message name from numeric ID.
     *
     *  \ingroup mavlink
     *  \param id The ID of the MAVLink message to get the name of.
     *  \returns The name of the message.
     *  \throws std::invalid_argument if the given \p id is not valid.
     */
    std::string name(unsigned long id)
    {
        const mavlink_message_info_t *info =
            mavlink_get_message_info_by_id(static_cast<uint32_t>(id));

        if (info)
        {
            return std::string(info->name);
        }

        throw std::invalid_argument(
            "Invalid packet ID (#" + std::to_string(id) + ").");
    }


    /** Get message ID from message name.
     *
     *  \ingroup mavlink
     *  \param name The name of the MAVLink message to get the numeric ID of.
     *  \returns The numeric ID of the message.
     *  \throws std::invalid_argument if the given message \p name is not valid.
     */
    unsigned long id(std::string name)
    {
        const mavlink_message_info_t *info =
            mavlink_get_message_info_by_name(name.c_str());

        if (info)
        {
            return info->msgid;
        }

        throw std::invalid_argument("Invalid packet name (\"" + name + "\").");
    }

}
