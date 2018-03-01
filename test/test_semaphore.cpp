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
#include <future>


#include <catch.hpp>

#include <semaphore.hpp>


using namespace std::chrono_literals;


TEST_CASE("semaphore's can be constructed.", "[semaphore]")
{
    REQUIRE_NOTHROW(semaphore());
    REQUIRE_NOTHROW(semaphore(10));
}


TEST_CASE("semaphore's 'wait' method waits until the semaphore can be "
          "decremented.", "[semaphore]")
{
    SECTION("Single wait.")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            sp.wait();
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        future.wait();
    }
    SECTION("Multiple wait.")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            sp.wait();
            sp.wait();
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(10ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        future.wait();
    }
    SECTION("Single wait, with initial value by notification.")
    {
        semaphore sp;
        sp.notify();
        auto future = std::async(std::launch::async, [&]()
        {
            sp.wait();
        });
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        future.wait();
    }
    SECTION("Multiple wait, with initial value by notification.")
    {
        semaphore sp;
        sp.notify();
        auto future = std::async(std::launch::async, [&]()
        {
            sp.wait();
            sp.wait();
        });
        REQUIRE(future.wait_for(10ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        future.wait();
    }
    SECTION("Single wait, with initial value in constructor.")
    {
        semaphore sp(1);
        auto future = std::async(std::launch::async, [&]()
        {
            sp.wait();
        });
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        future.wait();
    }
    SECTION("Multiple wait, with initial value in constructor.")
    {
        semaphore sp(2);
        auto future = std::async(std::launch::async, [&]()
        {
            sp.wait();
            sp.wait();
        });
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        future.wait();
    }
}


TEST_CASE("semaphore's 'wait_for' method waits until the semaphore can be "
          "decremented, or the timeout is reached (returning false if it "
          "timed out).", "[semaphore]")
{
    SECTION("Single wait (no timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_for(20ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(1ms) == std::future_status::ready);
        REQUIRE(future.get());
    }
    SECTION("Single wait (timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_for(1ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        REQUIRE_FALSE(future.get());
    }
    SECTION("Multiple wait (no timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_for(20ms) && sp.wait_for(20ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(1ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(1ms) == std::future_status::ready);
        REQUIRE(future.get());
    }
    SECTION("Multiple wait (timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_for(1ms) && sp.wait_for(1ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        REQUIRE_FALSE(future.get());
    }
}


TEST_CASE("semaphore's 'wait_until' method waits until the semaphore can be "
          "decremented, or the timeout timepoint is reached (returning false "
          "if it timed out).", "[semaphore]")
{
    SECTION("Single wait (no timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_until(std::chrono::steady_clock::now() + 20ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        REQUIRE(future.get());
    }
    SECTION("Single wait (timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_until(std::chrono::steady_clock::now() + 1ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        REQUIRE_FALSE(future.get());
    }
    SECTION("Multiple wait (no timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_until(std::chrono::steady_clock::now() + 20ms) &&
                   sp.wait_until(std::chrono::steady_clock::now() + 20ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(1ms) != std::future_status::ready);
        sp.notify();
        REQUIRE(future.wait_for(1ms) == std::future_status::ready);
        REQUIRE(future.get());
    }
    SECTION("Multiple wait (timeout).")
    {
        semaphore sp;
        auto future = std::async(std::launch::async, [&]()
        {
            return sp.wait_until(std::chrono::steady_clock::now() + 1ms) &&
                   sp.wait_until(std::chrono::steady_clock::now() + 1ms);
        });
        REQUIRE(future.wait_for(0ms) != std::future_status::ready);
        REQUIRE(future.wait_for(10ms) == std::future_status::ready);
        REQUIRE_FALSE(future.get());
    }
}
