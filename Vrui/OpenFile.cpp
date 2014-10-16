/***********************************************************************
OpenFile - Convenience function to open files of several types using the
IO::File abstraction inside a Vrui runtime environment.
Copyright (c) 2011 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/OpenFile.h>

#include <Cluster/OpenFile.h>
#include <Vrui/Vrui.h>

namespace Vrui {

IO::FilePtr openFile(const char* fileName,IO::File::AccessMode accessMode)
	{
	return Cluster::openFile(getClusterMultiplexer(),fileName,accessMode);
	}

IO::SeekableFilePtr openSeekableFile(const char* fileName,IO::File::AccessMode accessMode)
	{
	return Cluster::openSeekableFile(getClusterMultiplexer(),fileName,accessMode);
	}

IO::DirectoryPtr openDirectory(const char* directoryName)
	{
	return Cluster::openDirectory(getClusterMultiplexer(),directoryName);
	}

}
