/***********************************************************************
Alignment - Structure describing the alignment of a smaller object
inside a larger object.
Copyright (c) 2008 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_ALIGNMENT_INCLUDED
#define GLMOTIF_ALIGNMENT_INCLUDED

namespace GLMotif {

struct Alignment
	{
	/* Embedded classes: */
	public:
	enum HAlignment // Enumerated type for horizontal alignments
		{
		HFILL,LEFT,HCENTER,RIGHT
		};
	enum VAlignment // Enumerated type for vertical alignments
		{
		VFILL,BOTTOM,VCENTER,TOP
		};
	
	/* Elements: */
	HAlignment hAlignment; // Horizontal alignment
	VAlignment vAlignment; // Vertical alignment
	
	/* Constructors and destructors: */
	Alignment(HAlignment sHAlignment,VAlignment sVAlignment) // Elementwise constructor
		:hAlignment(sHAlignment),vAlignment(sVAlignment)
		{
		}
	Alignment(HAlignment sHAlignment) // Constructor with default vertical alignment VFILL
		:hAlignment(sHAlignment),vAlignment(VFILL)
		{
		}
	Alignment(VAlignment sVAlignment) // Constructor with default horizontal alignment HFILL
		:hAlignment(HFILL),vAlignment(sVAlignment)
		{
		}
	};

}

#endif
