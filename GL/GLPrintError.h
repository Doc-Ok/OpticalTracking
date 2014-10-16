/***********************************************************************
GLPrintError - Helper function to print a plain-text OpenGL error
message.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef GLPRINTERROR_INCLUDED
#define GLPRINTERROR_INCLUDED

#include <ostream>

void glPrintError(std::ostream& os); // Prints an error message to the given output stream
void glPrintError(std::ostream& os,const char* messageTag); // Ditto, but prints the given message tag before the error message

#endif
