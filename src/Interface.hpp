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


#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_


#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "ConnectionPool.hpp"


class Interface
{
    private:
        // Variables
        std::shared_ptr<ConnectionPool> connection_pool_;
        std::thread tx_thread_;
        std::thread rx_thread_;
        // Methods
        void tx_runner_();
        void rx_runner_();

    protected:
        // Variables
        /** True while interface is running.
         *
         *  %If this becomes cleared then the overloaded \ref tx_ and \ref rx_
         *  methods must return within 250 ms.
         *
         *  \remarks
         *      This is an atomic variable and is thus threadsafe.
         */
        std::atomic<bool> running_;
        // Methods
        /** Transmit packets on the interface (from the contained connections).
         *
         *  Must be defined by any derived class and should only return if the
         *  \ref running_ flag is set to false, and should do so no later than
         *  250 ms after the \ref running_ flag is cleared.
         */
        virtual void tx_() = 0;
        /** Read next packet.
         *
         *  Must be defined by any derived class.  It should return upon
         *  receiving a complete packet or if the \ref running_ flag is set to
         *  false, and should do so no later than 250 ms after the \ref running_
         *  flag is cleared.  In the later case, the pointer should be nullptr.
         *
         *  This method is also responsible for managing any of the interface's
         *  connections.
         *
         *  \returns A packet received on the interface.
         */
        virtual std::unique_ptr<Packet> rx_() = 0;

    public:
        enum Threads {
            START,  //!< Start the interface (and worker threads) immediately.
            DELAY_START //!< Delay starting, use \ref start to launch threads.
        };
        Interface(
            std::shared_ptr<ConnectionPool> connection_pool,
            Threads start_threads = Interface::START);
        // LCOV_EXCL_START
        virtual ~Interface();
        // LCOV_EXCL_STOP
        void start();
        void shutdown();
};


#endif // INTERFACE_HPP_
