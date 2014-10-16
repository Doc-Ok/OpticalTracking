/***********************************************************************
GLExtensions - Functions to query for availability of OpenGL extensions
and query pointers to OpenGL functions by name.
Copyright (c) 2004-2005 Oliver Kreylos

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

#ifndef GLEXTENSIONS_INCLUDED
#define GLEXTENSIONS_INCLUDED

bool glHasExtension(const char* extensionName);
void (*glGetFunctionPtr(const char* functionName))(void);

#endif
