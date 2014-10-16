/***********************************************************************
VendorProductId - Helper structure to represent USB device identifiers.
Copyright (c) 2013 Oliver Kreylos

This file is part of the USB Support Library (USB).

The USB Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The USB Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the USB Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef USB_VENDORPRODUCTID_INCLUDED
#define USB_VENDORPRODUCTID_INCLUDED

#include <stddef.h>
#include <Misc/SizedTypes.h>

namespace USB {

struct VendorProductId
	{
	/* Elements: */
	public:
	Misc::UInt16 vendorId;
	Misc::UInt16 productId;
	
	/* Constructors and destructors: */
	VendorProductId(void)
		{
		}
	VendorProductId(Misc::UInt16 sVendorId,Misc::UInt16 sProductId)
		:vendorId(sVendorId),productId(sProductId)
		{
		}
	
	/* Methods: */
	bool is(Misc::UInt16 otherVendorId,Misc::UInt16 otherProductId) const
		{
		return vendorId==otherVendorId&&productId==otherProductId;
		}
	bool operator==(const VendorProductId& other)
		{
		return vendorId==other.vendorId&&productId==other.productId;
		}
	bool operator!=(const VendorProductId& other)
		{
		return vendorId!=other.vendorId||productId!=other.productId;
		}
	static size_t rawHash(const VendorProductId& source)
		{
		return (size_t(source.productId)<<16)|size_t(source.vendorId);
		}
	static size_t hash(const VendorProductId& source,size_t tableSize)
		{
		return ((size_t(source.productId)<<16)|size_t(source.vendorId))%tableSize;
		}
	};

}

#endif
