/***********************************************************************
Client to read tracking data from a NaturalPoint OptiTrack tracking
system.
Copyright (c) 2010-2012 Oliver Kreylos

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

#ifndef NATURALPOINTCLIENT_INCLUDED
#define NATURALPOINTCLIENT_INCLUDED

#include <string>
#include <vector>
#include <Threads/Thread.h>
#include <Threads/MutexCond.h>
#include <Threads/TripleBuffer.h>
#include <Comm/UDPSocket.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>

#include "PacketBuffer.h"

class NaturalPointClient
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::Rotation<Scalar,3> Rotation;
	
	struct MarkerSetDef // Definition of a marker set
		{
		/* Elements: */
		public:
		std::string name; // Name of the marker set
		std::vector<std::string> markerNames; // Names of all markers in the set
		};
	
	struct RigidBodyDef // Definition of a rigid body
		{
		/* Elements: */
		public:
		std::string name; // Name of the rigid body; only defined in protocol version >= 2.0.0.0
		int id; // Unique ID of rigid body
		int parentId; // ID of rigid body's parent
		Vector offset; // Offset of rigid body relative to its parent
		
		/* Constructors and destructors: */
		RigidBodyDef(void) // Creates invalid rigid body definition
			:id(-1),parentId(-1),offset(Vector::zero)
			{
			}
		};
	
	struct SkeletonDef // Definition of a skeleton
		{
		/* Elements: */
		public:
		std::string name; // Name of the skeleton; only defined in protocol version >= 2.0.0.0
		int id; // Unique ID of skeleton
		std::vector<RigidBodyDef> rigidBodies; // List of rigid bodies associated with the skeleton
		
		/* Constructors and destructors: */
		SkeletonDef(void) // Creates invalid skeleton definition
			:id(-1)
			{
			}
		};
	
	struct ModelDef // Structure to hold a definition of all models defined in the tracking engine
		{
		/* Constructors and destructors: */
		public:
		std::vector<MarkerSetDef> markerSets; // List of defined marker sets
		std::vector<RigidBodyDef> rigidBodies; // List of defined rigid bodies
		std::vector<SkeletonDef> skeletons; // List of defined skeletons; only defined in protocol version >= 2.1.0.0
		};
	
	struct MarkerSet // Structure to hold the current state of a defined set of markers
		{
		/* Elements: */
		public:
		std::string name; // Name of the marker set
		std::vector<Point> markers; // Positions of all markers in the set
		};
	
	struct RigidBody // Structure to hold the current state of a defined rigid body
		{
		/* Elements: */
		public:
		int id; // Unique ID of rigid body
		Point position; // Position of rigid body's center point
		Rotation orientation; // Orientation of rigid body
		std::vector<Point> markers; // Positions of all markers in the rigid body
		std::vector<int> markerIds; // Unique IDs of all markers; only defined in protocol version >= 2.0.0.0
		std::vector<Scalar> markerSizes; // Sizes of all markers; only defined in protocol version >= 2.0.0.0
		Scalar meanMarkerError; // Mean fitting error for all markers; only defined in protocol version >= 2.0.0.0
		
		/* Constructors and destructors: */
		RigidBody(void) // Creates invalid rigid body
			:id(-1),
			 position(Point::origin),orientation(Rotation::identity),
			 meanMarkerError(0)
			{
			}
		};
	
	struct Skeleton // Structure to hold the current state of a defined skeleton
		{
		/* Elements: */
		public:
		int id; // Unique ID of skeleton
		std::vector<RigidBody> rigidBodies; // Rigid bodies comprising the skeleton
		
		/* Constructors and destructors: */
		Skeleton(void) // Creates invalid skeleton
			:id(-1)
			{
			}
		};
	
	struct Frame // Structure to hold a frame of tracking data
		{
		/* Elements: */
		public:
		int number; // Monotonically increasing frame number
		std::vector<MarkerSet> markerSets; // List of marker sets
		std::vector<Point> otherMarkers; // List of unidentified markers
		std::vector<RigidBody> rigidBodies; // List of rigid bodies
		std::vector<Skeleton> skeletons; // List of skeletons; only defined in protocol version >= 2.1.0.0
		int latency; // Frame latency
		
		/* Constructors and destructors: */
		Frame(void) // Creates empty frame
			:number(-1),latency(-1)
			{
			}
		};
	
	/* Elements: */
	private:
	Comm::UDPSocket commandSocket; // UDP socket to send query requests to the server
	PacketBuffer commandBuffer; // Buffer to pack command request packets
	PacketBuffer commandReplyBuffer; // Buffer to unpack command reply packets
	Threads::Thread commandHandlingThread; // Thread receiving command responses from server
	int dataSocketFd; // UDP multicast socket to receive streaming data from the server
	PacketBuffer dataBuffer; // Buffer to unpack data packets
	Threads::Thread dataHandlingThread; // Thread receiving tracking data from server
	std::string serverName; // Name of the server application
	int serverVersion[4]; // Server application version number
	int protocolVersion[4]; // Protocol version number
	Threads::MutexCond pingCond; // Condition variable to allow a caller to block until a ping response
	ModelDef* nextModelDef; // Pointer to next model definition structure to be filled by a ModelDef reply packet
	Threads::MutexCond modelDefCond; // Condition variable to allow a caller to block until a model definition arrives
	Threads::TripleBuffer<Frame> frames; // Triple buffer of frame states
	Threads::MutexCond frameCond; // Condition variable to allow a caller to block until a data frame arrives
	
	/* Private methods: */
	void handlePacket(PacketBuffer& packet);
	void* commandHandlingThreadMethod(void);
	void* dataHandlingThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	NaturalPointClient(const char* serverHostName,int commandPort,const char* dataMulticastGroup,int dataPort);
	~NaturalPointClient(void);
	
	/* Methods: */
	const std::string& getServerName(void) const // Returns the server application's name
		{
		return serverName;
		}
	const int* getServerVersion(void) const // Returns the server application's version
		{
		return serverVersion;
		}
	const int* getProtocolVersion(void) const // Returns the protocol version
		{
		return protocolVersion;
		}
	ModelDef& queryModelDef(ModelDef& modelDef); // Queries the models currently defined in the tracking engine and fills in the given structure
	const Frame& requestFrame(void); // Requests a data frame and blocks until it arrives
	const Frame& waitForNextFrame(void); // Waits until a requested frame arrives
	};

#endif
