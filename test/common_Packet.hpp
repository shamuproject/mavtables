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
#include <vector>

#include "macros.hpp"
#include "mavlink.hpp"


namespace
{
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
        uint8_t seq = 0xFE;  // test internal magic byte
        uint8_t sysid = 127;
        uint8_t compid = 1;
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
            uint8_t target_system = 127;
            uint8_t target_component = 1;
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0xFE;  // test internal magic byte
        uint8_t sysid = 192;
        uint8_t compid = 168;
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
            uint8_t base_mode = 0xFE;  // test magic byte in payload
        };
        uint8_t magic = 0xFE;
        uint8_t len = sizeof(payload);
        uint8_t seq = 0xFE;  // test internal magic byte
        uint8_t sysid = 172;
        uint8_t compid = 0;
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
            uint8_t data[253] = {
                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
                13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
                26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
                39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
                52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
                65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
                78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
                91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103,
                104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
                117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
                130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
                143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
                156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
                169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
                195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
                208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
                221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
                234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
                247, 248, 249, 250, 251, 252};
        };
        uint8_t magic = 0xFE;
        uint8_t len = 255;
        uint8_t seq = 0xFE;  // test internal magic byte
        uint8_t sysid = 224;
        uint8_t compid = 255;
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
        uint8_t seq = 0xFD;  // test internal magic byte
        uint8_t sysid = 127;
        uint8_t compid = 1;
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
            uint8_t target_system = 127;
            uint8_t target_component = 1;
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD;  // test internal magic byte
        uint8_t sysid = 192;
        uint8_t compid = 168;
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
            uint8_t base_mode = 0xFD;  // test magic byte in payload
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD;  // test internal magic byte
        uint8_t sysid = 172;
        uint8_t compid = 0;
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
            uint16_t seq = 0xFD;  // test magic byte in payload
            // uint8_t target_system = 0;
            // uint8_t target_component = 0;
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD;  // test internal magic byte
        uint8_t sysid = 255;
        uint8_t compid = 0;
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
            uint8_t data[253] = {
                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
                13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
                26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
                39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
                52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
                65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
                78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
                91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103,
                104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
                117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
                130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
                143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
                156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
                169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
                195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
                208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
                221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
                234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
                247, 248, 249, 250, 251, 252};
        };
        uint8_t magic = 0xFD;
        uint8_t len = sizeof(payload);
        uint8_t incompat_flags = 0;
        uint8_t compat_flags = 0;
        uint8_t seq = 0xFD;  // test internal magic byte
        uint8_t sysid = 224;
        uint8_t compid = 255;
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
        uint8_t seq = 0xFD;  // test internal magic byte
        uint8_t sysid = 0;
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
        data.assign(
            reinterpret_cast<uint8_t *>(&packet),
            reinterpret_cast<uint8_t *>(&packet) + sizeof(packet));
        return data;
    }


    // Convert a MAVLink packet structure to a vector of bytes with signature.
    template <class T>
    static std::vector<uint8_t> to_vector_with_sig(T packet)
    {
        std::vector<uint8_t> data;
        packet.incompat_flags |= MAVLINK_IFLAG_SIGNED;

        data.assign(
            reinterpret_cast<uint8_t *>(&packet),
            reinterpret_cast<uint8_t *>(&packet) + sizeof(packet));
        std::vector<uint8_t> sig = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
        data.insert(std::end(data), std::begin(sig), std::end(sig));
        return data;
    }

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}
