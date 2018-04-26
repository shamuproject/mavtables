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


#ifndef MAVLINK_H_
#define MAVLINK_H_


extern "C"
{

#define MAVLINK_USE_MESSAGE_INFO
#include "mavlink.h"

}

#include <cstdint>
#include "macros.hpp"

namespace mavlink
{

    /** \defgroup mavlink MAVLink Library and Helpers
     *
     *  MAVLink utility macros, defines, types and functions.
     *
     *  Including this file also includes the main MAVLink C library.  This will
     *  be the reference implementation using the common dialect by default.
     *  Set the `MAVLINK_INCLUDE_PATH` environment variable to the path
     *  containing the desired `mavlink.h` file to override this.
     */


    /** MAVLink packet v1.0 header.
     *
     *  \ingroup mavlink
     */
    struct PACKED v1_header
    {
        uint8_t magic;  //!< Protocol magic marker (0xFE).
        uint8_t len;    //!< Length of payload.
        uint8_t seq;    //!< Sequence of packet.
        uint8_t sysid;  //!< ID of message sender system/aircraft.
        uint8_t compid; //!< ID of the message sneder component.
        uint8_t msgid;  //!< ID of message in payload.
    };


    /** MAVLink packet v2.0 header.
     *
     *  \ingroup mavlink
     */
    struct PACKED v2_header
    {
        uint8_t magic;          //!< Protocol magic marker (0xFD).
        uint8_t len;            //!< Length of payload.
        uint8_t incompat_flags; //!< Flags that must be understood.
        uint8_t compat_flags;   //!< Flags that can be ignored if not known.
        uint8_t seq;            //!< Sequence of packet.
        uint8_t sysid;          //!< ID of message sender system/aircraft.
        uint8_t compid;         //!< ID of the message sender component.
        uint32_t msgid : 24;    //!< ID of message in payload (3 bytes).
    };


    std::string name(unsigned long id);
    unsigned long id(std::string name);

}

#endif // MAVLINK_H_
