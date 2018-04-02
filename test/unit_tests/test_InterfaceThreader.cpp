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


#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

#include <catch.hpp>

#include "ConnectionPool.hpp"
#include "Interface.hpp"
#include "InterfaceThreader.hpp"

#include <iostream>
#include <thread>


using namespace std::chrono_literals;


namespace
{

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    // Subclass of Interface used for testing the abstract class Interface.
    class InterfaceTestClass : public Interface
    {
        public:
            InterfaceTestClass(
                std::atomic<unsigned int> &tx_counter_,
                std::atomic<unsigned int> &rx_counter_)
                : tx_counter(tx_counter_), rx_counter(rx_counter_)
            {
            }
            void send_packet(
                const std::chrono::nanoseconds &timeout =
                    std::chrono::nanoseconds(100000)) final
            {
                std::this_thread::sleep_for(timeout);
                ++tx_counter;
            }
            void receive_packet(
                const std::chrono::nanoseconds &timeout =
                    std::chrono::nanoseconds(100000)) final
            {
                std::this_thread::sleep_for(timeout);
                ++rx_counter;
            }

        protected:
            // No point in testing this.
            // LCOV_EXCL_START
            std::ostream &print_(std::ostream &os) const final
            {
                os << "interface test class";
                return os;
            }
            // LCOV_EXCL_STOP

        private:
            std::atomic<unsigned int> &tx_counter;
            std::atomic<unsigned int> &rx_counter;

    };

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

}


TEST_CASE("InterfaceThreader's can be constructed.", "[InterfaceThreader]")
{
    std::atomic<unsigned int> tx_count(0);
    std::atomic<unsigned int> rx_count(0);
    SECTION("With delayed start.")
    {
        REQUIRE_NOTHROW(
            InterfaceThreader(
                std::make_unique<InterfaceTestClass>(tx_count, rx_count),
                1ms, InterfaceThreader::DELAY_START));
    }
    SECTION("With immediate start.")
    {
        REQUIRE_NOTHROW(
            InterfaceThreader(
                std::make_unique<InterfaceTestClass>(tx_count, rx_count)));
    }
}


TEST_CASE("InterfaceThreader's run Interface::send_packet and "
          "Interface::receive_packet methods of the contained "
          "Interface repeatedly.", "[InterfaceThreader]")
{
    std::atomic<unsigned int> tx_count(0);
    std::atomic<unsigned int> rx_count(0);
    SECTION("With delayed start.")
    {
        REQUIRE(tx_count == 0);
        REQUIRE(rx_count == 0);
        InterfaceThreader threader(
            std::make_unique<InterfaceTestClass>(tx_count, rx_count),
            1us, InterfaceThreader::DELAY_START);
        REQUIRE(tx_count == 0);
        REQUIRE(rx_count == 0);
        threader.start();
        std::this_thread::sleep_for(10ms);
        threader.shutdown();
        auto tx_count_ = tx_count.load();
        auto rx_count_ = rx_count.load();
        REQUIRE(tx_count > 0);
        REQUIRE(rx_count > 0);
        std::this_thread::sleep_for(10ms);
        REQUIRE(tx_count == tx_count_);
        REQUIRE(rx_count == rx_count_);
    }
    SECTION("With immediate start (manual shutdown).")
    {
        REQUIRE(tx_count == 0);
        REQUIRE(rx_count == 0);
        InterfaceThreader threader(
            std::make_unique<InterfaceTestClass>(tx_count, rx_count), 1us);
        std::this_thread::sleep_for(10ms);
        threader.shutdown();
        auto tx_count_ = tx_count.load();
        auto rx_count_ = rx_count.load();
        REQUIRE(tx_count > 0);
        REQUIRE(rx_count > 0);
        std::this_thread::sleep_for(10ms);
        REQUIRE(tx_count == tx_count_);
        REQUIRE(rx_count == rx_count_);
    }
    SECTION("With immediate start (RAII shutdown).")
    {
        REQUIRE(tx_count == 0);
        REQUIRE(rx_count == 0);
        {
            InterfaceThreader threader(
                std::make_unique<InterfaceTestClass>(tx_count, rx_count), 1us);
            std::this_thread::sleep_for(10ms);
        }
        auto tx_count_ = tx_count.load();
        auto rx_count_ = rx_count.load();
        REQUIRE(tx_count > 0);
        REQUIRE(rx_count > 0);
        std::this_thread::sleep_for(10ms);
        REQUIRE(tx_count == tx_count_);
        REQUIRE(rx_count == rx_count_);
    }
}
