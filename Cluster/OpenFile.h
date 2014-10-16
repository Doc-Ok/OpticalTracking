/***********************************************************************
OpenFile - Convenience function to open files of several types using the
IO::File abstraction and distribute among a cluster via a multicast
pipe.
Copyright (c) 2011 Oliver Kreylos

This file is part of the Cluster Abstraction Library (Cluster).

The Cluster Abstraction Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Cluster Abstraction Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Cluster Abstraction Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef CLUSTER_OPENFILE_INCLUDED
#define CLUSTER_OPENFILE_INCLUDED

#include <IO/OpenFile.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}

namespace Cluster {

IO::FilePtr openFile(Multiplexer* multiplexer,const char* fileName,IO::File::AccessMode accessMode =IO::File::ReadOnly); // Opens a file of the given name and distributes it over a new multicast pipe
IO::SeekableFilePtr openSeekableFile(Multiplexer* multiplexer,const char* fileName,IO::File::AccessMode accessMode =IO::File::ReadOnly); // Opens a seekable file of the given name and distributes it over a new multicast pipe
IO::DirectoryPtr openDirectory(Multiplexer* multiplexer,const char* directoryName); // Opens a directory of the given name and distributes it over a new multicast pipe

}

#endif
