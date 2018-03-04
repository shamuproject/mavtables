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


#include <fcntl.h>      // open, fnctl
#include <netinet/in.h> // sockaddr_in
#include <sys/ioctl.h>  // ioctl
#include <sys/poll.h>   // poll
#include <sys/socket.h> // socket, bind, sendto, recvfrom
#include <sys/stat.h>   // fstat
#include <sys/types.h>  // socklen_t type on old BSD systems
#include <termios.h>    // terminal control
#include <unistd.h>     // read, write, close

#include "UnixSyscalls.hpp"


/** Bind a name to a socket.
 *
 *  See [man 2 bind](http://man7.org/linux/man-pages/man2/bind.2.html) for
 *  documentation.
 *
 *  \param sockfd Socket file descriptor.
 *  \param addr Address to assign to socket.
 *  \param addrlen Size of address structure in bytes.
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
 *
 *  \param fd The file descriptor to close.
 */
int UnixSyscalls::close(int fd)
{
    return ::close(fd);
}


/** Control device.
 *
 *  See [man 2 ioctl](http://man7.org/linux/man-pages/man2/ioctl.2.html) for
 *  documentation.
 *
 *  \param fd The file descriptor to control.
 *  \param request Request code, defined in <sys/ioctl.h>
 *  \param argp Pointer to input/output, dependent on request code.
 */
int UnixSyscalls::ioctl(int fd, unsigned long request, void *argp)
{
    return ::ioctl(fd, request, argp);
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
 *
 *  \param fds File descriptor event structures.
 *  \param nfds Number of file descriptor event structures.
 *  \param timeout The timeout in milliseconds.
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
 *
 *  \param sockfd Socket file descriptor to receive data on.
 *  \param buf The buffer to write the data into.
 *  \param len The length of the buffer.
 *  \param flags Option flags.
 *  \param src_addr Source address buffer.
 *  \param addrlen Before call, length of source address buffer.  After call,
 *      actual length of address data.
 *  \returns The number of bytes written to \p or -1 if an error occurred.
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
 *
 *  \param domain Protocol family to use for communication.
 *  \param type Socket type.
 *  \param protocol Protocol to use for socket.
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
