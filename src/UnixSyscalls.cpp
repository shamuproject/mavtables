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


#include <termios.h> // terminal control
#include <sys/poll.h> // poll
#include <sys/stat.h> // fstat
#include <fcntl.h> // open, fcntl
#include <unistd.h>     // close
#include <sys/types.h>  // socklen_t type on old BSD systems
#include <sys/socket.h> // socket, bind, sendto, recvfrom
#include <netinet/in.h> // sockaddr_in

#include "UnixSyscalls.hpp"


/** Bind a name to a socket.
 *
 *  See [man 2 bind](http://man7.org/linux/man-pages/man2/bind.2.html) for
 *  documentation.
 */
int UnixSyscalls::bind(
    int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return ::bind(sockfd, addr, addrlen);
}


/** Get the input baud rate from the given termios structure.
 *
 *  See [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html) for
 *  documentation.
 */
speed_t UnixSyscalls::cfgetispeed(const struct termios *termios_p)
{
    return ::cfgetispeed(termios_p);
}


/** Get the output baud rate from the given termios structure.
 *
 *  See [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html) for
 *  documentation.
 */
speed_t UnixSyscalls::cfgetospeed(const struct termios *termios_p)
{
    return ::cfgetospeed(termios_p);
}


/** Set the input baud rate in the given termios structure.
 *
 *  See [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html) for
 *  documentation.
 */
int UnixSyscalls::cfsetispeed(struct termios *termios_p, speed_t speed)
{
    return ::cfsetispeed(termios_p, speed);
}


/** Set the output baud rate in the given termios structure.
 *
 *  See [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html) for
 *  documentation.
 */
int UnixSyscalls::cfsetospeed(struct termios *termios_p, speed_t speed)
{
    return ::cfsetospeed(termios_p, speed);
}


/** Close a file descriptor.
 *
 *  See [man 2 close](http://man7.org/linux/man-pages/man2/close.2.html) for
 *  documentation.
 */
int UnixSyscalls::close(int fd)
{
    return ::close(fd);
}


/** Open and possibly create a file.
 *
 *  See [man 2 open](http://man7.org/linux/man-pages/man2/open.2.html) for
 *  documentation.
 */
int UnixSyscalls::open(const char *pathname, int flags)
{
    return ::open(pathname, flags);
}


/** Wait for some event on a file descriptor.
 *
 *  See [man 2 poll](http://man7.org/linux/man-pages/man2/poll.2.html) for
 *  documentation.
 */
int UnixSyscalls::poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    return ::poll(fds, nfds, timeout);
}


/** Read from a file descriptor.
 *
 *  See [man 2 read](http://man7.org/linux/man-pages/man2/read.2.html) for
 *  documentation.
 */
ssize_t UnixSyscalls::read(int fd, void *buf, size_t count)
{
    return ::read(fd, buf, count);
}


/** Receive a message from a socket.
 *
 *  See [man 2 recvfrom](http://man7.org/linux/man-pages/man2/recv.2.html) for
 *  documentation.
 */
ssize_t UnixSyscalls::recvfrom(
    int sockfd, void *buf, size_t len, int flags,
    struct sockaddr *src_addr, socklen_t *addrlen)
{
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}


/** Send a message on a socket.
 *
 *  See [man 2 sendto](http://man7.org/linux/man-pages/man2/send.2.html) for
 *  documentation.
 */
ssize_t UnixSyscalls::sendto(
    int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen)
{
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}


/** Create an endpoint for communication.
 *
 *  See [man 2 socket](http://man7.org/linux/man-pages/man2/socket.2.html) for
 *  documentation.
 */
int UnixSyscalls::socket(int domain, int type, int protocol)
{
    return ::socket(domain, type, protocol);
}


/** Get serial port parameters associated with the given file descriptor.
 *
 *  See [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html) for
 *  documentation.
 */
int UnixSyscalls::tcgetattr(int fd, struct termios *termios_p)
{
    return ::tcgetattr(fd, termios_p);
}


/** Set serial port parameters for given file descriptor.
 *
 *  See [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html) for
 *  documentation.
 */
int UnixSyscalls::tcsetattr(
    int fd, int optional_actions, const struct termios *termios_p)
{
    return ::tcsetattr(fd, optional_actions, termios_p);
}


/** Write to a file descriptor.
 *
 *  See [man 2 write](http://man7.org/linux/man-pages/man2/write.2.html) for
 *  documentation.
 */
ssize_t UnixSyscalls::write(int fd, const void *buf, size_t count)
{
    return ::write(fd, buf, count);
}
