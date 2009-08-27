/* libcqcam - shared Color Quickcam routines
 * Copyright (C) 1996-1998 by Patrick Reynolds
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// I/O ports wrapper definitions and prototypes
// This file might need tweaking if you're trying to port my code to other
// x86 Unix platforms.  Code is already available for Linux, FreeBSD, and 
// QNX; see the Makefile.
//
// QNX code by: Anders Arpteg <aa11ac@hik.se>
// FreeBSD code by: Patrick Reynolds <reynolds@cs.duke.edu> and Charles 
//                  Henrich <henrich@msu.edu>
// Inlining implemented by: Philip Blundell <philip.blundell@pobox.com>

#ifndef PORT_H
#define PORT_H

//#include "config.h"

#include <unistd.h>

#ifdef LINUX
  #ifndef arm
  #include <sys/io.h>
  #endif /* !arm */
#elif defined(QNX)
#include <conio.h>
#elif defined(FREEBSD)
#include <machine/cpufunc.h>
#include <stdio.h>
#elif defined(BSDI)
#include <machine/inline.h>
#elif defined(OPENBSD)
#include <machine/pio.h>
#elif defined(LYNX)
#include "lynx-io.h"
#elif defined(SOLARIS)
#include "solaris-io.h"
#else
#error Please define a platform in the Makefile
#endif

#ifdef arm
static char ports_temp;

#ifdef inb
#undef inb
#endif /* inb */
#define inb(port) \
  lseek(devport, port, SEEK_SET), \
  read(devport, &ports_temp, 1), \
  ports_temp

#ifdef outb
#undef outb
#endif /* inb */
#define outb(data, port) \
  lseek(devport, port, SEEK_SET); \
  ports_temp = data; \
  write(devport, &ports_temp, 1);

#endif /* arm */

class port_t {
public:
  port_t(int iport);
  ~port_t(void);

  inline int read_data(void) { return inb(port); }
  inline int read_status(void) { return inb(port1); }
  inline int read_control(void) { return inb(port2); }

#if defined(LINUX) || defined(LYNX)
  inline void write_data(int data) { outb(data, port); }
  inline void write_control(int data) { outb(control_reg = data, port2); }
  inline void setbit_control(int data) { outb(control_reg |= data, port2); }
  inline void clearbit_control(int data) { outb(control_reg &= ~data, port2); }
#else // Solaris, QNX, and *BSD use (port, data) instead
  inline void write_data(int data) { outb(port, data); }
  inline void write_control(int data) { outb(port2, control_reg = data); }
  inline void setbit_control(int data) { outb(port2, control_reg |= data); }
  inline void clearbit_control(int data) { outb(port2, control_reg &= ~data); }
#endif

  inline int get_port() { return port; }
  inline operator bool () const { return port != -1; }

private:
  int port;        // number of the base port
  int port1;       // port+1, precalculated for speed
  int port2;       // port+2, precalculated for speed
  int control_reg; // current contents of the control register
#ifdef LOCKING
  int lock_fd;
  int lock(int portnum);
  void unlock(int portnum);
#endif

#ifdef FREEBSD
  FILE *devio;
#endif
#if defined(LINUX) && defined(arm)
  int devport;
#endif
};

#endif
