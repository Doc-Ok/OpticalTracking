/***********************************************************************
TransferBuffer - Class for sets of USB transfer buffers for asynchronous
I/O from/to USB devices.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef USB_TRANSFERBUFFER_INCLUDED
#define USB_TRANSFERBUFFER_INCLUDED

#include <stddef.h>

/* Forward declarations: */
struct libusb_transfer;

namespace USB {

class TransferBuffer
	{
	/* Elements: */
	private:
	unsigned int numTransfers; // Number of transfer objects in the pool
	unsigned int numPackets; // Number of packets per transfer objects (0 for non-isochronous transfers)
	size_t packetSize; // Size of one transfer packet in bytes (0 for non-isochronous transfers)
	size_t transferSize; // Size of buffer associated with each transfer object
	unsigned char* buffer; // Pointer to memory buffer backing all allocated USB transfer objects
	libusb_transfer** transfers; // Array of pointers to USB transfer objects
	volatile unsigned int numActiveTransfers; // Number of currently active transfers, to support proper cancellation
	
	/* Private methods: */
	bool initTransfers(void); // Initializes USB transfer objects; returns false on error
	
	/* Constructors and destructors: */
	public:
	TransferBuffer(unsigned int sNumTransfers,size_t sTransferSize); // Creates a transfer buffer for control, interrupt, or bulk I/O
	TransferBuffer(unsigned int sNumTransfers,unsigned int sNumPackets,size_t sPacketSize); // Creates a transfer buffer for isochronous I/O
	private:
	TransferBuffer(const TransferBuffer& source); // Prohibit copy constructor
	TransferBuffer& operator=(const TransferBuffer& source); // Prohibit assignment operator
	public:
	~TransferBuffer(void); // Releases all allocated resources
	
	/* Methods: */
	};

}

#endif
