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
#include <stdexcept>
#include <utility>

#include "Interface.hpp"
#include "InterfaceThreader.hpp"
#include "PartialSendError.hpp"


/** The transmitting thread runner.
 *
 *  This handles all transmission related tasks in a separate thread.
 */
void InterfaceThreader::tx_runner_()
{
    while (running_.load())
    {
        try
        {
            interface_->send_packet(timeout_);
        }
        // Ignore partial write errors on shutdown.
        // TODO: A better way to handle this error might be in order.  Not even
        //       sure exactly why it happens on close.
        catch (const PartialSendError &)
        {
            if (running_.load())
            {
                throw;
            }
        }
    }
}


/** The receiving thread runner.
 *
 *  This handles all receiving related tasks in a separate thread.
 */
void InterfaceThreader::rx_runner_()
{
    while (running_.load())
    {
        interface_->receive_packet(timeout_);
    }
}


/** Construct and optionally start an interface threader.
 *
 *  \param interface The \ref Interface to run in TX/RX threads.  It's \ref
 *      Interface::send_packet and \ref Interface::receive_packet methods will
 *      be called repeatedly in two separate worker threads.
 *  \param timeout The maximum amount of time to wait for incoming data or a
 *      packet to transmit.  The default value is 100000 us (100 ms).
 *  \param start_threads Set to \ref InterfaceThreader::START (the default
 *      value) to start the interface (including worker threads) on
 *      construction.  Set to \ref InterfaceThreader::DELAY_START to delay
 *      starting the interface (and worker threads) until the \ref start method
 *      is called.
 */
InterfaceThreader::InterfaceThreader(
    std::unique_ptr<Interface> interface,
    std::chrono::microseconds timeout,
    Threads start_threads)
    : interface_(std::move(interface)),
      timeout_(std::move(timeout)),
      running_(false)
{
    if (start_threads == InterfaceThreader::START)
    {
        start();
    }
}


/** Shutdown the interface and its associated worker threads.
 */
InterfaceThreader::~InterfaceThreader()
{
    shutdown();
}


/** Start the worker threads for the interface.
 *
 *  This starts the receiver and transmitter threads.
 */
void InterfaceThreader::start()
{
    running_.store(true);
    tx_thread_ = std::thread(&InterfaceThreader::tx_runner_, this);
    rx_thread_ = std::thread(&InterfaceThreader::rx_runner_, this);
}


/** Shutdown the interface and its associated worker threads.
 *
 *  \note This will always be called by the interface's destructor.
 */
void InterfaceThreader::shutdown()
{
    if (running_.load())
    {
        running_.store(false);
        tx_thread_.join();
        rx_thread_.join();
    }
}
