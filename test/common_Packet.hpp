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


#include <vector>
#include <cstdint>

#include "Connection.hpp"
#include "macros.hpp"
#include "mavlink.h"


namespace
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    // Subclass of Packet used for testing the abstract class Connection.
    class ConnectionTestClass : public Connection
    {
    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif


    // HEARTBEAT v1.0 structure for testing packets without target
    // system/component.
    struct PACKED HeartbeatV1
    {
        struct PACKED payload
        {
            uint8_t type = 1;
            uint8_t autopilot = 2;
            uint8_t base_mode = 3;
            uint32_t custom_mode = 4;
            uint8_t system_status = 5;
            uint8_t mavlink_version = 6;
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0xFE; // test internal magic byte
        uint8_t sysid = 1;
        uint8_t compid = 0;
        uint8_t msgid = 0;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // PING v1.0 structure for testing target system/compoent.
    struct PACKED PingV1
    {
        struct PACKED payload
        {
            uint64_t time_usec = 295128000000000;
            uint32_t seq = 0xBA5EBA11;
            uint8_t target_system = 255;
            uint8_t target_component = 23;
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0xFE; // test internal magic byte
        uint8_t sysid = 60;
        uint8_t compid = 40;
        uint8_t msgid = 4;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // SET_MODE v1.0 structure for testing target system only.
    struct PACKED SetModeV1
    {
        struct PACKED payload
        {
            uint32_t custom_mode = 2;
            uint8_t target_system = 123;
            uint8_t base_mode = 0xFE; // test magic byte in payload
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0xFE; // test internal magic byte
        uint8_t sysid = 70;
        uint8_t compid = 30;
        uint8_t msgid = 11;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


    // ENCAPSULATED_DATA v1.0 structure for testing maximum length packets.
    struct PACKED EncapsulatedDataV1
    {
        struct PACKED payload
        {
            uint16_t seqnr = 0;
            uint8_t data[253];
        };
        uint8_t magic = 0xFE;
        uint8_t len = 255;
        uint8_t seq = 0xFE; // test internal magic byte
        uint8_t sysid = 255;
        uint8_t compid = 1;
        uint8_t msgid = 131;
        payload payload;
        uint16_t checksum = 0xFACE;
    };


#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-member-function"
#endif

    // HEARTBEAT v2.0 structure for testing packets without target
    // system/component.
    struct PACKED HeartbeatV2
    {
        struct PACKED payload
        {
            uint8_t type = 1;
            uint8_t autopilot = 2;
            uint8_t base_mode = 3;
            uint32_t custom_mode = 4;
            uint8_t system_status = 5;
            uint8_t mavlink_version = 6;
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD; // test internal magic byte
        uint8_t sysid = 1;
        uint8_t compid = 0;
        uint32_t msgid : 24;
        payload payload;
        uint16_t checksum = 0xFACE;
        HeartbeatV2() : msgid(0) {}
    };


    // PING v2.0 structure for testing target system/component.
    struct PACKED PingV2
    {
        struct PACKED payload
        {
            uint64_t time_usec = 295128000000000;
            uint32_t seq = 0xBA5EBA11;
            uint8_t target_system = 255;
            uint8_t target_component = 23;
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD; // test internal magic byte
        uint8_t sysid = 60;
        uint8_t compid = 40;
        uint32_t msgid : 24;
        payload payload;
        uint16_t checksum = 0xFACE;
        PingV2() : msgid(4) {}
    };


    // SET_MODE v2.0 structure for testing target system only.
    struct PACKED SetModeV2
    {
        struct PACKED payload
        {
            uint32_t custom_mode = 2;
            uint8_t target_system = 123;
            uint8_t base_mode = 0xFD; // test magic byte in payload
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD; // test internal magic byte
        uint8_t sysid = 70;
        uint8_t compid = 30;
        uint32_t msgid : 24;
        payload payload;
        uint16_t checksum = 0xFACE;
        SetModeV2() : msgid(11) {}
    };


    // MISSION_SET_CURRENT v2.0 for testing trimmed out target system and
    // components.
    struct PACKED MissionSetCurrentV2
    {
        struct PACKED payload
        {
            uint16_t seq = 0xFD; // test magic byte in payload
            // uint8_t target_system = 0;
            // uint8_t target_component = 0;
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD; // test internal magic byte
        uint8_t sysid = 80;
        uint8_t compid = 20;
        uint32_t msgid : 24;
        payload payload;
        uint16_t checksum = 0xFACE;
        MissionSetCurrentV2() : msgid(41) {}
    };


    // ENCAPSULATED_DATA v2.0 structure for testing maximum length packets.
    struct PACKED EncapsulatedDataV2
    {
        struct PACKED payload
        {
            uint16_t seqnr = 0;
            uint8_t data[253];
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD; // test internal magic byte
        uint8_t sysid = 255;
        uint8_t compid = 1;
        uint32_t msgid : 24;
        payload payload;
        uint16_t checksum = 0xFACE;
        EncapsulatedDataV2() : msgid(131) {}
    };


    // PARAM_EXT_REQUEST_LIST v2.0 structure for testing message ID's beyond
    // 255.
    struct PACKED ParamExtRequestListV2
    {
        struct PACKED payload
        {
            uint8_t target_system = 32;
            uint8_t target_component = 64;
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD; // test internal magic byte
        uint8_t sysid = 1;
        uint8_t compid = 255;
        uint32_t msgid : 24;
        payload payload;
        uint16_t checksum = 0xFACE;
        ParamExtRequestListV2() : msgid(321) {}
    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif


#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-template"
#endif

    // Convert a MAVLink packet structure to a vector of bytes.
    template <class T>
    static std::vector<uint8_t> to_vector(T packet)
    {
        std::vector<uint8_t> data;
        data.assign(reinterpret_cast<uint8_t *>(&packet),
                    reinterpret_cast<uint8_t *>(&packet) + sizeof(packet));
        return data;
    }


    // Convert a MAVLink packet structure to a vector of bytes with signature.
    template <class T>
    static std::vector<uint8_t> to_vector_with_sig(T packet)
    {
        std::vector<uint8_t> data;
        packet.incompat_flags |= MAVLINK_IFLAG_SIGNED;

        data.assign(reinterpret_cast<uint8_t *>(&packet),
                    reinterpret_cast<uint8_t *>(&packet) + sizeof(packet));
        std::vector<uint8_t> sig =
        {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
        };
        data.insert(std::end(data), std::begin(sig), std::end(sig));
        return data;
    }

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

}
