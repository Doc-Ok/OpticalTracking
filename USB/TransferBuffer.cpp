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

#include <USB/TransferBuffer.h>

#include <libusb-1.0/libusb.h>
#include <Misc/ThrowStdErr.h>

namespace USB {

/*******************************
Methods of class TransferBuffer:
*******************************/

bool TransferBuffer::initTransfers(void)
	{
	/* Allocate the transfer backing buffer: */
	buffer=new unsigned char[numTransfers*transferSize];
	
	/* Allocate all USB transfer objects: */
	transfers=new libusb_transfer*[numTransfers];
	for(unsigned int i=0;i<numTransfers;++i)
		transfers[i]=0;
	unsigned char* bufPtr=buffer;
	bool ok=true;
	for(unsigned int i=0;i<numTransfers&&ok;++i,bufPtr+=transferSize)
		ok=(transfers[i]=libusb_alloc_transfer(numPackets))!=0;
	
	/* Check for errors: */
	if(!ok)
		{
		/* Delete all transfers that were successfully allocated: */
		for(unsigned int i=0;i<numTransfers;++i)
			libusb_free_transfer(transfers[i]);
		delete[] transfers;
		
		/* Delete the backing buffer: */
		delete[] buffer;
		}
	
	return ok;
	}

TransferBuffer::TransferBuffer(unsigned int sNumTransfers,size_t sTransferSize)
	:numTransfers(sNumTransfers),
	 numPackets(0),packetSize(0),
	 transferSize(sTransferSize),
	 buffer(0),transfers(0),
	 numActiveTransfers(0)
	{
	/* Initialize the transfer objects: */
	if(!initTransfers())
		Misc::throwStdErr("USB::TransferBuffer: Unable to allocate %u transfer objects of size %u each",numTransfers,(unsigned int)(transferSize));
	}

TransferBuffer::TransferBuffer(unsigned int sNumTransfers,unsigned int sNumPackets,size_t sPacketSize)
	:numTransfers(sNumTransfers),
	 numPackets(sNumPackets),packetSize(sPacketSize),
	 transferSize(numPackets*packetSize),
	 buffer(0),transfers(0),
	 numActiveTransfers(0)
	{
	/* Initialize the transfer objects: */
	if(!initTransfers())
		Misc::throwStdErr("USB::TransferBuffer: Unable to allocate %u transfer objects of size %u each",numTransfers,(unsigned int)(transferSize));
	}

TransferBuffer::~TransferBuffer(void)
	{
	}

}
