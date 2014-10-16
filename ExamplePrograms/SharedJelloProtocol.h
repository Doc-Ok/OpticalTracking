/***********************************************************************
SharedJelloProtocol - Class defining the communication protocol between
a shared Jell-O server and its clients.
Copyright (c) 2007-2011 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SHAREDJELLOPROTOCOL_INCLUDED
#define SHAREDJELLOPROTOCOL_INCLUDED

#include <Misc/SizedTypes.h>
#include <Misc/StandardMarshallers.h>
#include <IO/File.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryMarshallers.h>

#include "JelloCrystal.h"

class SharedJelloProtocol
	{
	/* Embedded classes: */
	public:
	typedef Misc::UInt16 MessageIdType; // Network type for protocol messages
	typedef Misc::UInt8 Byte; // Network type for raw bytes
	typedef Misc::UInt32 Card; // Network type for size or index values
	typedef JelloCrystal::Scalar Scalar;
	typedef JelloCrystal::Point Point;
	typedef JelloCrystal::Vector Vector;
	typedef JelloCrystal::Rotation Rotation;
	typedef JelloCrystal::Ray Ray;
	typedef JelloCrystal::Box Box;
	typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform;
	
	enum MessageId // Enumerated type for protocol messages
		{
		CONNECT_REPLY=0, // Initiates connection by sending the Jell-O crystal's parameters to the client
		CLIENT_UPDATE, // Updates the connected client's state on the server side
		SERVER_UPDATE, // Sends current state of all other connected clients to a connected client
		CLIENT_PARAMUPDATE, // Sends new simulation parameters from client to server
		SERVER_PARAMUPDATE, // Sends new simulation parameters from server to client
		DISCONNECT_REQUEST, // Polite request to disconnect from the server
		DISCONNECT_REPLY, // Reply to a disconnect request
		MESSAGES_END
		};
	
	/* Methods: */
	static void writeMessage(MessageId messageId,IO::File& sink) // Writes a protocol message to the given sink
		{
		sink.write<MessageIdType>(messageId);
		};
	static MessageIdType readMessage(IO::File& source) // Reads a protocol message from the given source
		{
		return source.read<MessageIdType>();
		};
	template <class ValueParam>
	static ValueParam read(IO::File& source) // Reads a value from the given source
		{
		return Misc::Marshaller<ValueParam>::read(source);
		}
	template <class ValueParam>
	static void read(ValueParam& value,IO::File& source) // Ditto, by reference
		{
		value=Misc::Marshaller<ValueParam>::read(source);
		}
	template <class ValueParam>
	static void write(const ValueParam& value,IO::File& sink) // Writes a value to the given sink
		{
		Misc::Marshaller<ValueParam>::write(value,sink);
		}
	};

#endif
