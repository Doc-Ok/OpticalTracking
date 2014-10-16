/***********************************************************************
ColorInterpolatorNode - Class to represent color maps.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef COLORINTERPOLATORNODE_INCLUDED
#define COLORINTERPOLATORNODE_INCLUDED

#include <vector>
#include <Math/Math.h>

#include "Types.h"

#include "VRMLNode.h"

class ColorInterpolatorNode:public VRMLNode
	{
	/* Elements: */
	private:
	std::vector<Float> key; // Array of knot values
	std::vector<Color> keyValue; // Array of color values for knot values
	
	/* Constructors and destructors: */
	public:
	ColorInterpolatorNode(VRMLParser& parser); // Creates color interpolator node by parsing VRML file
	
	/* Methods: */
	Color interpolate(Float value) const // Evaluates the color map for the given value
		{
		/* Check the value against the key value range: */
		if(value<=key.front())
			return keyValue.front();
		else if(value>=key.back())
			return keyValue.back();
		else
			{
			/* Find the knot interval containing the given value: */
			int l=0;
			int r=key.size()-1;
			while(r-l>1)
				{
				int m=(l+r)>>1;
				if(value<key[m])
					r=m;
				else
					l=m;
				}
			
			/* Interpolate linearly between l and r: */
			Float wr=(value-key[l])/(key[r]-key[l]);
			Float wl=1.0-wr;
			Color result;
			for(int i=0;i<4;++i)
				result[i]=GLubyte(Math::floor(float(keyValue[l][i])*wl+float(keyValue[r][i])*wr+0.5f));
			
			return result;
			}
		}
	};

#endif
