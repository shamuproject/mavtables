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
#include <utility>

#include "Interface.hpp"


#include <iostream>


/** The transmitting thread runner.
 *
 *  This handles all transmission related tasks in a separate thread.
 */
void Interface::tx_runner_()
{
    while (running_.load())
    {
        tx_();
    }
}


/** The receiving thread runner.
 *
 *  This handles all receiving related tasks in a separate thread.
 */
void Interface::rx_runner_()
{
    while (running_.load())
    {
    }

    // while (running_.load())
    // {
    //     // std::cout << "In rx" << std::endl;
    //     // auto packet = rx_();
    //     // if (packet != nullptr)
    //     // {
    //     //     connection_pool_->send(std::move(packet));
    //     // }
    // }
}


/** Construct and optionally start an interface.
 *
 *  \param connection_pool The connection pool to use for registering
 *      connections and sending packets.
 *  \param start_threads Set to \ref Interface::START (the default value) to
 *      start the interface (including worker threads) on construction.  Set to
 *      \ref Interface::DELAY_START to delay starting the interface (and worker
 *      threads) until the \ref start method is called.
 */
Interface::Interface(
    std::shared_ptr<ConnectionPool> connection_pool, Threads start_threads)
    : connection_pool_(std::move(connection_pool)), running_(false)
{
    if (start_threads == Interface::START)
    {
        start();
    }
}


/** Shutdown the interface and its associated worker threads.
 */
Interface::~Interface()
{
    std::cout << "shutdown" << std::endl;

    if (running_.load())
    {
        running_.store(false);
        std::cout << "wait for join" << std::endl;
        tx_thread_.join();
        std::cout << "wait for join" << std::endl;
        // rx_thread_.join();
    }
}


#include <chrono>

using namespace std::chrono_literals;

/** Start the worker threads for the interface.
 *
 *  This starts a receiver and transmitter thread.
 */
void Interface::start()
{
    // TODO: Calls pure virtual (in constructor).  Need to make constructor
    // private and use a factory method.
    // Care to avoid: https://stackoverflow.com/q/25145994/8093188
    running_.store(true);
    // tx_thread_ = std::thread(&Interface::tx_runner_, this);
    // rx_thread_ = std::thread(&Interface::rx_runner_, this);
    tx_thread_ = std::thread([this] {this->tx_runner_();});
    // rx_thread_ = std::thread([this]{this->rx_runner_();});
    std::cout << " Threads running." << std::endl;
    std::this_thread::sleep_for(4s);
    shutdown();
}


/** Shutdown the interface and its associated worker threads.
 *
 *  \note This will always be called by the interface's destructor.
 *
 */
void Interface::shutdown()
{
    std::cout << "shutdown" << std::endl;

    if (running_.load())
    {
        running_.store(false);
        std::cout << "wait for join" << std::endl;
        tx_thread_.join();
        std::cout << "wait for join" << std::endl;
        rx_thread_.join();
    }
}
