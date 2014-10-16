/***********************************************************************
ReadOptiTrackMarkerFile - Helper functions to read marker files in XML
format as written by the NaturalPoint OptiTrack rigid body tracking
toolkit.
Copyright (c) 2008-2010 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef READOPTITRACKMARKERFILE_INCLUDED
#define READOPTITRACKMARKERFILE_INCLUDED

#include <vector>
#include <Geometry/Point.h>

void readOptiTrackMarkerFile(const char* fileName,const char* bodyName,double scale,bool flipZ,std::vector<Geometry::Point<double,3> >& markers);

#endif
