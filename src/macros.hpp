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


/** \defgroup macros Macros
 *
 *  Macros used elsewhere.
 */


/** Enforce a packed structure.
 *
 *  A packed structure will not have any padding regardless of the typical
 *  alignment restrictions.
 *
 *  Example:
 *  ```
 *  struct PACKED a_packed_structure
 *  {
 *      uint8_t a;
 *      uint16_t b;
 *      uint32_t c;
 *  }
 *  ```
 *
 *  \ingroup macros
 */
#define PACKED __attribute__((packed))
