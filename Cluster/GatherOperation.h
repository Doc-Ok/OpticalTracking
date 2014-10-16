/***********************************************************************
GatherOperation - Class containing enumerated type to identify
operations available to compute final values during a gather operation
on a multicast pipe.
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef CLUSTER_GATHEROPERATION_INCLUDED
#define CLUSTER_GATHEROPERATION_INCLUDED

namespace Cluster {

class GatherOperation
	{
	/* Embedded classes: */
	public:
	enum OpCode // Enumerated type for supported gathering accumulation functions
		{
		AND,OR, // Boolean operations
		MIN,MAX, // Range operations
		SUM,PRODUCT // Arithmetic operations
		};
	};

}

#endif
