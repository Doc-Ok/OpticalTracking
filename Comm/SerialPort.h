/***********************************************************************
SerialPort - Class for high-performance reading/writing from/to serial
ports.
Copyright (c) 2001-2013 Oliver Kreylos

This file is part of the Portable Communications Library (Comm).

The Portable Communications Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Portable Communications Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Communications Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef COMM_SERIALPORT_INCLUDED
#define COMM_SERIALPORT_INCLUDED

#include <Comm/Pipe.h>

namespace Comm {

class SerialPort:public Comm::Pipe
	{
	/* Embedded classes: */
	public:
	enum PortSettings
		{
		Blocking=0x0,NonBlocking=0x1
		};
	
	enum Parity
		{
		NoParity,EvenParity,OddParity
		};
	
	/* Elements: */
	private:
	int fd; // File descriptor of the underlying serial port device file
	
	/* Protected methods from IO::File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	SerialPort(const char* deviceName); // Opens the given serial port device file with "DontCare" endianness setting
	virtual ~SerialPort(void);
	
	/* Methods from IO::File: */
	virtual int getFd(void) const;
	
	/* Methods from Pipe: */
	virtual bool waitForData(void) const;
	virtual bool waitForData(const Misc::Time& timeout) const;
	virtual void shutdown(bool read,bool write);
	
	/* New methods: */
	void setPortSettings(int portSettingsMask); // Sets port file descriptor settings
	void setSerialSettings(int bitRate,int charLength,Parity parity,int numStopbits,bool enableHandshake); // Sets serial port parameters
	void setRawMode(int minNumBytes,int timeout); // Switches port to "raw" mode and sets burst parameters
	void setCanonicalMode(void); // Switches port to canonical mode
	void setLineControl(bool respectModemLines,bool hangupOnClose); // Sets line control parameters
	bool getRTS(void); // Returns the current state of the "ready to send" serial port line
	bool setRTS(bool newRTS); // Manually sets or clears the "ready to send" serial port line; returns previous state
	bool getCTS(void); // Returns the current state of the "clear to send" serial port line
	bool setCTS(bool newCTS); // Manually sets or clears the "clear to send" serial port line; returns previous state
	};

}

#endif
