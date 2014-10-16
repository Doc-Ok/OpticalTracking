/***********************************************************************
ThrowStdErr - Helper function to create std::runtime_error error
descriptions using the printf() interface. The function obviously never
returns...
Copyright (c) 2005 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef MISC_THROWSTDERR_INCLUDED
#define MISC_THROWSTDERR_INCLUDED

namespace Misc {

const char* printStdErrMsg(const char* formatString,...);

void throwStdErr(const char* formatString,...);

}

#endif
