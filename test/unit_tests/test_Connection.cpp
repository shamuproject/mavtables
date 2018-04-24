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


#include <chrono>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>
#include <utility>

#include <catch.hpp>
#include <fakeit.hpp>

#include "AddressPool.hpp"
#include "Connection.hpp"
#include "MAVAddress.hpp"
#include "Filter.hpp"
#include "Packet.hpp"
#include "PacketVersion2.hpp"
#include "PacketQueue.hpp"
#include "util.hpp"

#include "common.hpp"
#include "common_Packet.hpp"


using namespace std::chrono_literals;


// NOTE: This testing file is not the best at not testing the implementation.
//       If a better solution is found it should be rewritten.


TEST_CASE("Connection's can be constructed.", "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    SECTION("With default arguments.")
    {
        REQUIRE_NOTHROW(Connection("name", filter));
    }
    SECTION("As a regular connection.")
    {
        REQUIRE_NOTHROW(
            Connection(
                "name", filter, false, std::move(pool), std::move(queue)));
    }
    SECTION("As a mirror connection.")
    {
        REQUIRE_NOTHROW(
            Connection(
                "name", filter, true, std::move(pool), std::move(queue)));
    }
    SECTION("Ensures the filter pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            Connection(
                "name", nullptr, false, std::move(pool), std::move(queue)),
            std::invalid_argument);
        pool = mock_unique(mock_pool);
        queue = mock_unique(mock_queue);
        REQUIRE_THROWS_WITH(
            Connection(
                "name", nullptr, false, std::move(pool), std::move(queue)),
            "Given filter pointer is null.");
    }
    SECTION("Ensures the pool pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            Connection("name", filter, false, nullptr, std::move(queue)),
            std::invalid_argument);
        queue = mock_unique(mock_queue);
        REQUIRE_THROWS_WITH(
            Connection("name", filter, false, nullptr, std::move(queue)),
            "Given pool pointer is null.");
    }
    SECTION("Ensures the queue pointer is not null.")
    {
        REQUIRE_THROWS_AS(
            Connection("name", filter, false, std::move(pool), nullptr),
            std::invalid_argument);
        pool = mock_unique(mock_pool);
        REQUIRE_THROWS_WITH(
            Connection("name", filter, false, std::move(pool), nullptr),
            "Given queue pointer is null.");
    }
}


TEST_CASE("Connection's 'add_address' method adds/updates addresses.",
          "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_pool, add));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    conn.add_address(MAVAddress("192.168"));
    fakeit::Verify(Method(mock_pool, add).Matching([](auto a)
    {
        return a == MAVAddress("192.168");
    })).Once();
}


TEST_CASE("Connection's 'next_packet' method.", "[Connection]")
{
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Returns the next packet.")
    {
        fakeit::When(
            OverloadedMethod(
                mock_queue, pop,
                std::shared_ptr<const Packet>(
                    const std::chrono::nanoseconds &))).Return(ping);
        std::chrono::nanoseconds timeout = 1ms;
        auto packet = conn.next_packet(timeout);
        REQUIRE(packet != nullptr);
        REQUIRE(*packet == *ping);
        fakeit::Verify(
            OverloadedMethod(
                mock_queue, pop,
                std::shared_ptr<const Packet>(
                    const std::chrono::nanoseconds &)).Matching([](auto a)
        {
            return a == 1ms;
        })).Once();
    }
    SECTION("Or times out and returns nullptr.")
    {
        fakeit::When(
            OverloadedMethod(
                mock_queue, pop,
                std::shared_ptr<const Packet>(
                    const std::chrono::nanoseconds &))).Return(nullptr);
        std::chrono::nanoseconds timeout = 0ms;
        REQUIRE(conn.next_packet(timeout) == nullptr);
        fakeit::Verify(
            OverloadedMethod(
                mock_queue, pop,
                std::shared_ptr<const Packet>(
                    const std::chrono::nanoseconds &)).Matching([](auto a)
        {
            return a == 0ms;
        })).Once();
    }
}


TEST_CASE("Connection's 'send' method ensures the given packet is not "
          "nullptr.", "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Ensures the given packet is not nullptr.")
    {
        REQUIRE_THROWS_AS(conn.send(nullptr), std::invalid_argument);
        REQUIRE_THROWS_WITH(
            conn.send(nullptr), "Given packet pointer is null.");
    }
}


TEST_CASE("Connection's 'send' method (with destination address).",
          "[Connection]")
{
    // Packets for testing.
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_queue, push));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the destination can be "
            "reached on this connection and the filter allows it.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(true, 2);
        });
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto & a)
        {
            return a != MAVAddress("192.168");
        });
        conn.send(ping);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *ping && b == 2;
        })).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(false, 0);
        });
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto & a)
        {
            return a != MAVAddress("192.168");
        });
        conn.send(ping);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
    SECTION("Silently drops the packet if the destination cannot be "
            "reached on this connection.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(true, 0);
        });
        fakeit::When(Method(mock_pool, addresses)).AlwaysReturn(
            std::vector<MAVAddress>());
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto & a)
        {
            (void)a;
            return false;
        });
        conn.send(ping);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
}


TEST_CASE("Connection's 'send' method (without destination address).",
          "[Connection]")
{
    // Packets for testing.
    auto heartbeat =
        std::make_shared<packet_v2::Packet>(to_vector(HeartbeatV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
        [&](auto & a, auto & b)
    {
        (void)a;
        if (b == MAVAddress("192.168"))
        {
            return std::pair<bool, int>(true, 2);
        }
        if (b == MAVAddress("172.16"))
        {
            return std::pair<bool, int>(true, -3);
        }
        return std::pair<bool, int>(false, 0);
    });
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::When(Method(mock_pool, contains)).AlwaysDo([](MAVAddress addr)
    {
        if (addr == MAVAddress("10.10"))
        {
            return true;
        }
        if (addr == MAVAddress("172.16"))
        {
            return true;
        }
        if (addr == MAVAddress("192.168"))
        {
            return true;
        }
        return false;
    });
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_queue, push));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(increasing priority).")
    {
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("10.10"),
                MAVAddress("172.16"),
                MAVAddress("192.168")
            };
            return addr;
        });
        conn.send(heartbeat);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *heartbeat && b == 2;
        })).Once();
    }
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(decreasing priority).")
    {
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("192.168"),
                MAVAddress("172.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(heartbeat);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *heartbeat && b == 2;
        })).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it for all "
            "reachable addresses.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(false, 0);
        });
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("10.10"),
                MAVAddress("172.16"),
                MAVAddress("192.168")
            };
            return addr;
        });
        conn.send(heartbeat);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
}


TEST_CASE("Connection's 'send' method (with broadcast address 0.0).",
          "[Connection]")
{
    // Packets for testing.
    auto mission_set_current =
        std::make_shared<packet_v2::Packet>(to_vector(MissionSetCurrentV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
        [&](auto & a, auto & b)
    {
        (void)a;
        if (b == MAVAddress("192.168"))
        {
            return std::pair<bool, int>(true, 2);
        }
        if (b == MAVAddress("172.16"))
        {
            return std::pair<bool, int>(true, -3);
        }
        return std::pair<bool, int>(false, 0);
    });
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::When(Method(mock_pool, contains)).AlwaysDo([](MAVAddress addr)
    {
        if (addr == MAVAddress("10.10"))
        {
            return true;
        }
        if (addr == MAVAddress("172.16"))
        {
            return true;
        }
        if (addr == MAVAddress("192.168"))
        {
            return true;
        }
        return false;
    });
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_queue, push));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(increasing priority).")
    {
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("10.10"),
                MAVAddress("172.16"),
                MAVAddress("192.168")
            };
            return addr;
        });
        conn.send(mission_set_current);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *mission_set_current && b == 2;
        })).Once();
    }
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any of the reachable addresses and favors the higher priority "
            "(decreasing priority).")
    {
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("192.168"),
                MAVAddress("172.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(mission_set_current);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *mission_set_current && b == 2;
        })).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it for all "
            "reachable addresses.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(false, 0);
        });
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("10.10"),
                MAVAddress("192.168"),
                MAVAddress("172.16")
            };
            return addr;
        });
        conn.send(mission_set_current);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
}


TEST_CASE("Connection's 'send' method (with component broadcast address x.0).",
          "[Connection]")
{
    // Packets for testing.
    auto set_mode = std::make_shared<packet_v2::Packet>(to_vector(SetModeV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::When(Method(mock_pool, contains)).AlwaysDo([](MAVAddress addr)
    {
        if (addr == MAVAddress("10.10"))
        {
            return true;
        }
        if (addr == MAVAddress("123.16"))
        {
            return true;
        }
        if (addr == MAVAddress("123.17"))
        {
            return true;
        }
        if (addr == MAVAddress("123.168"))
        {
            return true;
        }
        return false;
    });
    fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
        [&](auto & a, auto & b)
    {
        (void)a;
        if (b == MAVAddress("123.168"))
        {
            return std::pair<bool, int>(true, 2);
        }
        if (b == MAVAddress("123.16"))
        {
            return std::pair<bool, int>(true, -3);
        }
        return std::pair<bool, int>(false, 0);
    });
    fakeit::Mock<PacketQueue> mock_queue;
    fakeit::Fake(Method(mock_queue, push));
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any reachable component address of the given system address and "
            "favors the higher priority (increasing priority).")
    {
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("10.10"),
                MAVAddress("123.16"),
                MAVAddress("123.17"),
                MAVAddress("123.168")
            };
            return addr;
        });
        conn.send(set_mode);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *set_mode && b == 2;
        })).Once();
    }
    SECTION("Adds the packet to the PacketQueue if the filter allows it for "
            "any reachable component address of the given system address and "
            "favors the higher priority (decreasing priority).")
    {
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("123.168"),
                MAVAddress("123.17"),
                MAVAddress("123.16"),
                MAVAddress("10.10")
            };
            return addr;
        });
        conn.send(set_mode);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *set_mode && b == 2;
        })).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it for all "
            "matching addresses.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(false, 0);
        });
        fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
        {
            std::vector<MAVAddress> addr =
            {
                MAVAddress("10.10"),
                MAVAddress("123.16"),
                MAVAddress("123.17"),
                MAVAddress("123.168")
            };
            return addr;
        });
        conn.send(set_mode);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
    SECTION("Silently drops the packet if the destination system cannot be "
            "reached on this connection.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            (void)b;
            return std::pair<bool, int>(true, 0);
        });
        fakeit::When(Method(mock_pool, addresses)).AlwaysReturn(
            std::vector<MAVAddress>());
        fakeit::When(Method(mock_pool, contains)).AlwaysDo([&](auto & a)
        {
            (void)a;
            return false;
        });
        conn.send(set_mode);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
}


TEST_CASE("Connection's 'send' method (destination address, system reachable, "
          "component unreachable.", "[Connection]")
{
    // Packets for testing.
    auto ping = std::make_shared<packet_v2::Packet>(to_vector(PingV2()));
    // Mocked objects.
    fakeit::Mock<Filter> mock_filter;
    fakeit::Mock<AddressPool<>> mock_pool;
    fakeit::When(Method(mock_pool, addresses)).AlwaysDo([]()
    {
        std::vector<MAVAddress> addr =
        {
            MAVAddress("10.10"),
            MAVAddress("127.16"),
            MAVAddress("127.17"),
            MAVAddress("127.168")
        };
        return addr;
    });
    fakeit::When(Method(mock_pool, contains)).AlwaysDo([](MAVAddress addr)
    {
        if (addr == MAVAddress("10.10"))
        {
            return true;
        }
        if (addr == MAVAddress("127.16"))
        {
            return true;
        }
        if (addr == MAVAddress("127.17"))
        {
            return true;
        }
        if (addr == MAVAddress("127.168"))
        {
            return true;
        }
        return false;
    });
    fakeit::Mock<PacketQueue> mock_queue;
    auto filter = mock_shared(mock_filter);
    auto pool = mock_unique(mock_pool);
    auto queue = mock_unique(mock_queue);
    // Connection for testing.
    Connection conn("name", filter, false, std::move(pool), std::move(queue));
    SECTION("Adds the packet to the PacketQueue if any component of the "
            "system is reachable on the connection and the filter allows it "
            "to the original component.")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            if (b == MAVAddress("127.1"))
            {
                return std::pair<bool, int>(true, 2);
            }
            return std::pair<bool, int>(false, 0);
        });
        fakeit::Fake(Method(mock_queue, push));
        conn.send(ping);
        fakeit::Verify(Method(mock_queue, push)).Once();
        fakeit::Verify(Method(mock_queue, push).Matching([&](auto a, auto b)
        {
            return a != nullptr && *a == *ping && b == 2;
        })).Once();
    }
    SECTION("Silently drops the packet if the filter rejects it (using "
            "it's original address).")
    {
        fakeit::When(Method(mock_filter, will_accept)).AlwaysDo(
            [&](auto & a, auto & b)
        {
            (void)a;
            if (b == MAVAddress("127.1"))
            {
                return std::pair<bool, int>(false, 0);
            }
            return std::pair<bool, int>(true, 2);
        });
        fakeit::Fake(Method(mock_queue, push));
        conn.send(ping);
        fakeit::Verify(Method(mock_queue, push)).Exactly(0);
    }
}


TEST_CASE("Connection's are printable", "[Connection]")
{
    fakeit::Mock<Filter> mock_filter;
    auto filter = mock_shared(mock_filter);
    REQUIRE(str(Connection("some_name", filter)) == "some_name");
    REQUIRE(str(Connection("/dev/ttyUSB0", filter)) == "/dev/ttyUSB0");
    REQUIRE(str(Connection("127.0.0.1:14555", filter)) == "127.0.0.1:14555");
}
