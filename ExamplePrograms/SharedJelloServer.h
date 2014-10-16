/***********************************************************************
SharedJelloServer - Dedicated server program to allow multiple clients
to collaboratively smack around a Jell-O crystal.
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

#ifndef SHAREDJELLOSERVER_INCLUDED
#define SHAREDJELLOSERVER_INCLUDED

#include <vector>
#include <Misc/HashTable.h>
#include <Threads/Mutex.h>
#include <Threads/Thread.h>
#include <Threads/TripleBuffer.h>
#include <Comm/NetPipe.h>
#include <Comm/ListeningTCPSocket.h>
#include <Geometry/Box.h>

#include "JelloCrystal.h"
#include "SharedJelloProtocol.h"

class SharedJelloServer:private SharedJelloProtocol
	{
	/* Embedded classes: */
	private:
	typedef JelloCrystal::AtomID AtomID;
	public:
	typedef JelloCrystal::Index Index; // Index type for the atom array
	typedef JelloCrystal::Box Box; // Type for the simulation domain of the Jell-O crystal
	
	private:
	struct ClientState // Structure to hold the input device state of a connected client
		{
		/* Embedded classes: */
		public:
		struct StateUpdate // Structure to hold the contents of a state update packet
			{
			/* Embedded classes: */
			public:
			struct DraggerState // Structure describing the state of a dragger
				{
				/* Elements: */
				public:
				Card id; // Unique per-client dragger ID
				bool rayBased; // Flag whether the dragger is ray-based instead of point-based
				Ray ray; // Selection ray for ray-based draggers
				ONTransform transform; // Current dragger transformation
				bool active; // Flag if the dragger is currently active
				};
			
			/* Elements: */
			public:
			unsigned int numDraggers; // Number of dragging tools in the state update
			DraggerState* draggerStates; // Array of dragger states
			
			/* Constructors and destructors: */
			StateUpdate(void)
				:numDraggers(0),draggerStates(0)
				{
				};
			~StateUpdate(void)
				{
				delete[] draggerStates;
				};
			};
		
		struct AtomLock // Structure to connect a client's dragger to a locked Jell-O atom
			{
			/* Elements: */
			public:
			AtomID draggedAtom; // ID of the locked atom
			ONTransform dragTransformation; // The dragging transformation applied to the locked atom
			};
		
		typedef Misc::HashTable<unsigned int,AtomLock> AtomLockMap; // Hash table to map dragger IDs to atom locks
		
		/* Elements: */
		public:
		Threads::Mutex pipeMutex; // Mutex serializing access to the pipe
		Comm::NetPipePtr pipe; // Communication pipe connected to the client
		Threads::Thread communicationThread; // Thread receiving state updates from the client
		bool connected; // Flag if the client's connection protocol has finished
		unsigned int parameterVersion; // Version number of parameter set on the client side
		Threads::TripleBuffer<StateUpdate> stateUpdates; // Triple buffer of state update packets
		AtomLockMap atomLocks; // Map of atom locks held by this client
		
		/* Constructors and destructors: */
		ClientState(Comm::NetPipePtr sPipe)
			:pipe(sPipe),
			 connected(false),
			 parameterVersion(0),
			 atomLocks(17)
			{
			};
		};
	
	typedef std::vector<ClientState*> ClientStateList; // Type for lists of pointers to client state structures
	
	/* Elements: */
	
	/* Jell-O state: */
	Threads::Mutex parameterMutex; // Mutex serializing write access to set of simulation parameters
	unsigned int newParameterVersion; // Version number of current set of simulation parameters
	Scalar newAtomMass;
	Scalar newAttenuation;
	Scalar newGravity;
	JelloCrystal crystal; // The virtual Jell-O crystal
	unsigned int parameterVersion; // Version number of simulation parameters set in Jell-O crystal
	
	/* Client communication state: */
	Comm::ListeningTCPSocket listenSocket; // Listening socket for incoming client connections
	Threads::Thread listenThread; // Thread listening for incoming connections on the listening port
	Threads::Mutex clientStateListMutex; // Mutex protecting the client state list (not the included structures)
	ClientStateList clientStates; // List of client state structures
	
	/* Private methods: */
	void* listenThreadMethod(void); // Thread method accepting connections from new clients
	void* clientCommunicationThreadMethod(ClientState* clientState); // Thread method receiving state updates from connected clients
	
	/* Constructors and destructors: */
	public:
	SharedJelloServer(const Index& numAtoms,const Box& domain,int listenPortID); // Creates a shared Jell-O server with the given crystal size and listen port ID (assigns dynamic port if port ID is negative)
	~SharedJelloServer(void); // Destroys the shared Jell-O server
	
	/* Methods: */
	int getListenPortID(void) const // Returns the port ID assigned to the listening socket
		{
		return listenSocket.getPortId();
		};
	void simulate(double timeStep); // Updates the simulation state based on the real time since the last frame
	void sendServerUpdate(void); // Sends the most recent Jell-O crystal state to all connected clients
	};

#endif
