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


#ifndef UNIXSYSCALLS_HPP_
#define UNIXSYSCALLS_HPP_


#include <fcntl.h>       // open, fnctl
#include <netinet/in.h>  // sockaddr_in
#include <sys/ioctl.h>   // ioctl
#include <sys/poll.h>    // poll
#include <sys/socket.h>  // socket, bind, sendto, recvfrom
#include <sys/stat.h>    // fstat
#include <sys/types.h>   // socklen_t type on old BSD systems
#include <termios.h>     // terminal control
#include <unistd.h>      // read, write, close

#include "config.hpp"


/** This is a thin wrapper around Unix system calls.
 *
 *  The purpose of this is to allow system calls to be mocked during testing.
 *
 *  See the following man pages for documentation:
 *  * [man 2 bind](http://man7.org/linux/man-pages/man2/bind.2.html)
 *  * [man 2 close](http://man7.org/linux/man-pages/man2/close.2.html)
 *  * [man 2 socket](http://man7.org/linux/man-pages/man2/socket.2.html)
 *  * [man 2 ioctl](http://man7.org/linux/man-pages/man2/ioctl.2.html)
 *  * [man 7 ip](http://man7.org/linux/man-pages/man7/ip.7.html)
 *  * [man 2 open](http://man7.org/linux/man-pages/man2/open.2.html)
 *  * [man 2 poll](http://man7.org/linux/man-pages/man2/poll.2.html)
 *  * [man 2 read](http://man7.org/linux/man-pages/man2/read.2.html)
 *  * [man 2 recvfrom](http://man7.org/linux/man-pages/man2/recv.2.html)
 *  * [man 2 sendto](http://man7.org/linux/man-pages/man2/send.2.html)
 *  * [man 2 termios](http://man7.org/linux/man-pages/man3/termios.3.html)
 *  * [man 2 write](http://man7.org/linux/man-pages/man2/write.2.html)
 *
 */
class UnixSyscalls
{
  public:
    TEST_VIRTUAL ~UnixSyscalls() = default;
    TEST_VIRTUAL int
    bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    TEST_VIRTUAL speed_t cfgetispeed(const struct termios *termios_p);
    TEST_VIRTUAL speed_t cfgetospeed(const struct termios *termios_p);
    TEST_VIRTUAL int cfsetispeed(struct termios *termios_p, speed_t speed);
    TEST_VIRTUAL int cfsetospeed(struct termios *termios_p, speed_t speed);
    TEST_VIRTUAL int close(int fd);
    TEST_VIRTUAL int ioctl(int fd, unsigned long request, void *argp);
    TEST_VIRTUAL int open(const char *pathname, int flags);
    TEST_VIRTUAL int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    TEST_VIRTUAL ssize_t read(int fd, void *buf, size_t count);
    TEST_VIRTUAL ssize_t recvfrom(
        int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr,
        socklen_t *addrlen);
    TEST_VIRTUAL ssize_t sendto(
        int sockfd, const void *buf, size_t len, int flags,
        const struct sockaddr *dest_addr, socklen_t addrlen);
    TEST_VIRTUAL int socket(int domain, int type, int protocol);
    TEST_VIRTUAL int tcgetattr(int fd, struct termios *termios_p);
    TEST_VIRTUAL int
    tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
    TEST_VIRTUAL ssize_t write(int fd, const void *buf, size_t count);
};


#endif  // UNIXSYSCALLS_HPP_
