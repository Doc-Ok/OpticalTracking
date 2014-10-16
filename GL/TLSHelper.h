/***********************************************************************
TLSHelper.h - Macro definition to simplify supporting thread-local
storage types inside the GLSupport library.
Copyright (c) 2006-2010 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLTLSHELPER_INCLUDED
#define GLTLSHELPER_INCLUDED

#include <GL/Config.h>

#if GLSUPPORT_CONFIG_USE_TLS

#if GLSUPPORT_CONFIG_HAVE_BUILTIN_TLS

/* Use the compiler's built-in support for thread-local storage: */
#define GL_THREAD_LOCAL(VariableType) __thread VariableType

#else

/* Use pthreads' thread-local storage mechanism: */
#define GL_THREAD_LOCAL(VariableType) Threads::Local<VariableType>

#endif

#else

/* Ignore the THREAD_LOCAL modifier: */
#define GL_THREAD_LOCAL(VariableType) VariableType

#endif

#endif
