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


#ifndef INTERFACETHREADER_HPP_
#define INTERFACETHREADER_HPP_


#include <atomic>
#include <memory>
#include <thread>

#include "Interface.hpp"


class InterfaceThreader
{
    public:
        enum Threads
        {
            START,  //!< Start the interface (and worker threads) immediately.
            DELAY_START //!< Delay starting, use \ref start to launch threads.
        };
        InterfaceThreader(
            std::shared_ptr<Interface> interface,
            std::chrono::microseconds = std::chrono::microseconds(100000),
            Threads start_threads = InterfaceThreader::START);
        InterfaceThreader(const InterfaceThreader &other) = delete;
        InterfaceThreader(InterfaceThreader &&other) = delete;
        ~InterfaceThreader();
        void start();
        void shutdown();
        InterfaceThreader &operator=(const InterfaceThreader &other) = delete;
        InterfaceThreader &operator=(InterfaceThreader &&other) = delete;

    private:
        // Variables
        std::shared_ptr<Interface> interface_;
        std::thread tx_thread_;
        std::thread rx_thread_;
        std::chrono::microseconds timeout_;
        std::atomic<bool> running_;
        // Methods
        void tx_runner_();
        void rx_runner_();
};


#endif // INTERFACETHREADER_HPP_
