/***********************************************************************
Config - Configuration header file for Portable Threading Library.
Copyright (c) 2011-2012 Oliver Kreylos

This file is part of the Portable Threading Library (Threads).

The Portable Threading Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Portable Threading Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Threading Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef THREADS_CONFIG_INCLUDED
#define THREADS_CONFIG_INCLUDED

#define THREADS_CONFIG_HAVE_BUILTIN_TLS 1
#define THREADS_CONFIG_HAVE_BUILTIN_ATOMICS 1
#define THREADS_CONFIG_HAVE_SPINLOCKS 1
#define THREADS_CONFIG_CAN_CANCEL 1

#define THREADS_CONFIG_DEBUG 0

#endif
