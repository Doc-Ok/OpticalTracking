/***********************************************************************
GetCurrentDirectory - Helper function to retrieve the fully-qualified
path name of the calling process' current directory.
Copyright (c) 2010 Oliver Kreylos

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

#include <Misc/GetCurrentDirectory.h>

#include <unistd.h>

namespace Misc {

std::string getCurrentDirectory(void)
	{
	/* Use a safe method to query the current directory name: */
	size_t cwdLen=512;
	char* cwd=new char[cwdLen];
	while(getcwd(cwd,cwdLen)==0)
		{
		cwdLen=(cwdLen*3)/2;
		delete[] cwd;
		cwd=new char[cwdLen];
		}
	
	/* Return the queried name: */
	std::string result(cwd);
	delete[] cwd;
	return result;
	}

}
