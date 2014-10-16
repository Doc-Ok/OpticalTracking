/***********************************************************************
ConvexInterpolator - Generic class to perform convex interpolation of
data values.
Copyright (c) 2003-2010 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef CONVEXINTERPOLATOR_INCLUDED
#define CONVEXINTERPOLATOR_INCLUDED

namespace Visualization {

template <class ValueParam,class WeightParam>
class ConvexInterpolator
	{
	/* Methods: */
	public:
	inline static ValueParam interpolate(const ValueParam& v0,const ValueParam& v1,WeightParam w1)
		{
		return v0*(WeightParam(1)-w1)+v1*w1;
		};
	};

}

#endif
