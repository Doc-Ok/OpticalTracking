/***********************************************************************
SharedJelloServer - Dedicated server program to allow multiple clients
to collaboratively smack around a Jell-O crystal.
Copyright (c) 2007-2014 Oliver Kreylos

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

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <Misc/Timer.h>
#include <Misc/ThrowStdErr.h>
#include <Comm/TCPPipe.h>

#include "SharedJelloServer.h"

/**********************************
Methods of class SharedJelloServer:
**********************************/

void* SharedJelloServer::listenThreadMethod(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Process incoming connections until shut down: */
	while(true)
		{
		/* Wait for the next incoming connection: */
		#ifdef VERBOSE
		std::cout<<"SharedJelloServer: Waiting for client connection"<<std::endl<<std::flush;
		#endif
		Comm::NetPipePtr clientPipe=new Comm::TCPPipe(listenSocket);
		clientPipe->negotiateEndianness();
		
		/* Connect the new client: */
		#ifdef VERBOSE
		std::cout<<"SharedJelloServer: Connecting new client from host "<<clientPipe->getPeerHostName()<<", port "<<clientPipe->getPeerPortId()<<std::endl<<std::flush;
		#endif
		
		/**************************************************************************************
		Connect the new client by creating a new client state object and adding it to the list:
		**************************************************************************************/
		
		{
		/* Lock the client state list: */
		Threads::Mutex::Lock clientStateListLock(clientStateListMutex);
		
		try
			{
			/* Create a new client state object and add it to the list: */
			ClientState* newClientState=new ClientState(clientPipe);
			clientStates.push_back(newClientState);
			
			/* Start a communication thread for the new client: */
			newClientState->communicationThread.start(this,&SharedJelloServer::clientCommunicationThreadMethod,newClientState);
			}
		catch(std::runtime_error err)
			{
			std::cerr<<"SharedJelloServer: Cancelled connecting new client due to exception "<<err.what()<<std::endl<<std::flush;
			}
		}
		}
	
	return 0;
	}

void* SharedJelloServer::clientCommunicationThreadMethod(SharedJelloServer::ClientState* clientState)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	Comm::NetPipe& pipe=*(clientState->pipe);
	Threads::Mutex& pipeMutex=clientState->pipeMutex;
	
	try
		{
		/* Connect the client by sending the size of the Jell-O crystal: */
		{
		Threads::Mutex::Lock pipeLock(pipeMutex);
		writeMessage(CONNECT_REPLY,pipe);
		write(crystal.getDomain().min,pipe);
		write(crystal.getDomain().max,pipe);
		Card numAtoms[3];
		for(int i=0;i<3;++i)
			numAtoms[i]=crystal.getNumAtoms()[i];
		pipe.write(numAtoms,3);
		pipe.flush();
		}
		
		/* Mark the client as connected: */
		{
		Threads::Mutex::Lock clientStateListLock(clientStateListMutex);
		clientState->connected=true;
		}
		
		#ifdef VERBOSE
		std::cout<<"SharedJelloServer: Connection to client from host "<<pipe.getPeerHostName()<<", port "<<pipe.getPeerPortId()<<" established"<<std::endl<<std::flush;
		#endif
		
		/* Run the client communication protocol machine: */
		bool goOn=true;
		while(goOn)
			{
			/* Wait for and handle the next message: */
			switch(readMessage(pipe))
				{
				case CLIENT_PARAMUPDATE:
					/* Update the simulation parameter set: */
					{
					Threads::Mutex::Lock parameterLock(parameterMutex);
					++newParameterVersion;
					newAtomMass=pipe.read<Scalar>();
					newAttenuation=pipe.read<Scalar>();
					newGravity=pipe.read<Scalar>();
					}
					break;
				
				case CLIENT_UPDATE:
					{
					/* Lock the next free client update slot: */
					ClientState::StateUpdate& su=clientState->stateUpdates.startNewValue();
					
					/* Process the client update message: */
					unsigned int newNumDraggers=pipe.read<Card>();
					if(newNumDraggers!=su.numDraggers)
						{
						delete[] su.draggerStates;
						su.numDraggers=newNumDraggers;
						su.draggerStates=su.numDraggers!=0?new ClientState::StateUpdate::DraggerState[su.numDraggers]:0;
						}
					
					for(unsigned int draggerIndex=0;draggerIndex<su.numDraggers;++draggerIndex)
						{
						su.draggerStates[draggerIndex].id=pipe.read<Card>();
						su.draggerStates[draggerIndex].rayBased=pipe.read<Byte>()!=0;
						SharedJelloProtocol::read(su.draggerStates[draggerIndex].ray,pipe);
						SharedJelloProtocol::read(su.draggerStates[draggerIndex].transform,pipe);
						su.draggerStates[draggerIndex].active=pipe.read<Byte>()!=0;
						}

					/* Mark the client update slot as most recent: */
					clientState->stateUpdates.postNewValue();
					break;
					}
				
				case DISCONNECT_REQUEST:
					/* Send a disconnect reply: */
					{
					Threads::Mutex::Lock pipeLock(pipeMutex);
					writeMessage(DISCONNECT_REPLY,pipe);
					pipe.flush();
					}
					
					goOn=false;
					
					break;
				
				default:
					Misc::throwStdErr("Protocol error in client communication");
				}
			}
		}
	catch(std::runtime_error err)
		{
		/* Ignore any connection errors; just disconnect the client */
		std::cerr<<"SharedJelloServer: Disconnecting client due to exception "<<err.what()<<std::endl<<std::flush;
		}
	
	/******************************************************************************************
	Disconnect the client by removing it from the list and deleting the client state structure:
	******************************************************************************************/
	
	#ifdef VERBOSE
	std::cout<<"SharedJelloServer: Disconnecting client from host "<<pipe.getPeerHostName()<<", port "<<pipe.getPeerPortId()<<std::endl<<std::flush;
	#endif
	
	{
	/* Lock the client state list: */
	Threads::Mutex::Lock clientStateListLock(clientStateListMutex);
	
	/* Unlock all atoms held by the client: */
	for(ClientState::AtomLockMap::Iterator alIt=clientState->atomLocks.begin();!alIt.isFinished();++alIt)
		crystal.unlockAtom(alIt->getDest().draggedAtom);
	
	/* Find this client's state in the list: */
	ClientStateList::iterator cslIt;
	for(cslIt=clientStates.begin();cslIt!=clientStates.end()&&*cslIt!=clientState;++cslIt)
		;
	
	/* Remove the client state from the list: */
	clientStates.erase(cslIt);
	
	/* Delete the client state object: */
	delete clientState;
	}
	
	return 0;
	}

SharedJelloServer::SharedJelloServer(const SharedJelloServer::Index& numAtoms,const SharedJelloServer::Box& domain,int listenPortID)
	:newParameterVersion(1),
	 crystal(numAtoms,domain),
	 parameterVersion(1),
	 listenSocket(listenPortID,0)
	{
	/* Start listening thread: */
	listenThread.start(this,&SharedJelloServer::listenThreadMethod);
	}

SharedJelloServer::~SharedJelloServer(void)
	{
	/* Lock client list: */
	Threads::Mutex::Lock clientStateListLock(clientStateListMutex);
	
	/* Stop connection initiating thread: */
	listenThread.cancel();
	listenThread.join();
	
	/* Disconnect all clients: */
	for(ClientStateList::iterator cslIt=clientStates.begin();cslIt!=clientStates.end();++cslIt)
		{
		/* Stop client communication thread: */
		(*cslIt)->communicationThread.cancel();
		(*cslIt)->communicationThread.join();
		
		/* Delete client state object: */
		delete *cslIt;
		}
	}

void SharedJelloServer::simulate(double timeStep)
	{
	{
	Threads::Mutex::Lock parameterLock(parameterMutex);
	if(newParameterVersion!=parameterVersion)
		{
		/* Update the Jell-O crystal's simulation parameters: */
		crystal.setAtomMass(newAtomMass);
		crystal.setAttenuation(newAttenuation);
		crystal.setGravity(newGravity);
		parameterVersion=newParameterVersion;
		}
	}
	
	/*******************************************************************************
	Process all client state updates received since the beginning of the last frame:
	*******************************************************************************/
	
	{
	/* Lock the client state list: */
	Threads::Mutex::Lock clientStateListLock(clientStateListMutex);
	
	/* Check all client states for recent updates: */
	for(ClientStateList::iterator cslIt=clientStates.begin();cslIt!=clientStates.end();++cslIt)
		{
		/* Check if there has been an update since the last frame: */
		ClientState* cs=*cslIt;
		if(cs->stateUpdates.hasNewValue())
			{
			/* Lock the most recent update: */
			cs->stateUpdates.lockNewValue();
			ClientState::StateUpdate& su=cs->stateUpdates.getLockedValue();
			
			/* Update the list of atoms locked by this client: */
			for(unsigned int draggerIndex=0;draggerIndex<su.numDraggers;++draggerIndex)
				{
				if(su.draggerStates[draggerIndex].active)
					{
					/* Check if this dragger has just become active: */
					ClientState::AtomLockMap::Iterator alIt=cs->atomLocks.findEntry(su.draggerStates[draggerIndex].id);
					if(alIt.isFinished())
						{
						/* Find the atom picked by the dragger: */
						ClientState::AtomLock al;
						if(su.draggerStates[draggerIndex].rayBased)
							al.draggedAtom=crystal.pickAtom(su.draggerStates[draggerIndex].ray);
						else
							al.draggedAtom=crystal.pickAtom(su.draggerStates[draggerIndex].transform.getOrigin());
						
						/* Try locking the atom: */
						if(crystal.lockAtom(al.draggedAtom))
							{
							/* Calculate the dragging transformation: */
							al.dragTransformation=su.draggerStates[draggerIndex].transform;
							al.dragTransformation.doInvert();
							al.dragTransformation*=crystal.getAtomState(al.draggedAtom);
							
							/* Store the atom lock in the map: */
							cs->atomLocks.setEntry(ClientState::AtomLockMap::Entry(su.draggerStates[draggerIndex].id,al));
							}
						}
					else
						{
						/* Set the position/orientation of the locked atom: */
						ONTransform transform=su.draggerStates[draggerIndex].transform;
						transform*=alIt->getDest().dragTransformation;
						crystal.setAtomState(alIt->getDest().draggedAtom,transform);
						}
					}
				else
					{
					/* Check if this dragger has just become inactive: */
					ClientState::AtomLockMap::Iterator alIt=cs->atomLocks.findEntry(su.draggerStates[draggerIndex].id);
					if(!alIt.isFinished())
						{
						/* Release the atom lock: */
						crystal.unlockAtom(alIt->getDest().draggedAtom);
						cs->atomLocks.removeEntry(alIt);
						}
					}
				}
			}
		}
	}
	
	/* Simulate the crystal's behavior in this time step: */
	crystal.simulate(timeStep);
	}

void SharedJelloServer::sendServerUpdate(void)
	{
	/* Lock the client state list: */
	Threads::Mutex::Lock clientStateListLock(clientStateListMutex);
	
	/* Go through all client states: */
	for(ClientStateList::iterator cslIt=clientStates.begin();cslIt!=clientStates.end();++cslIt)
		{
		ClientState* cs=*cslIt;
		if(cs->connected)
			{
			try
				{
				Threads::Mutex::Lock pipeLock(cs->pipeMutex);
				
				if(cs->parameterVersion!=parameterVersion)
					{
					/* Send a parameter update message: */
					writeMessage(SERVER_PARAMUPDATE,*cs->pipe);
					cs->pipe->write<Scalar>(crystal.getAtomMass());
					cs->pipe->write<Scalar>(crystal.getAttenuation());
					cs->pipe->write<Scalar>(crystal.getGravity());
					cs->parameterVersion=parameterVersion;
					}
				
				/* Send a server update message: */
				writeMessage(SERVER_UPDATE,*cs->pipe);
				
				/* Send the crystal's state: */
				crystal.writeAtomStates(*cs->pipe);
				
				cs->pipe->flush();
				}
			catch(...)
				{
				/* Ignore write errors; let the client communication thread handle them */
				}
			}
		}
	}

/*********************
Main program function:
*********************/

int main(int argc,char* argv[])
	{
	/* Parse the command line: */
	SharedJelloServer::Index numAtoms(4,4,8);
	SharedJelloServer::Box domain(SharedJelloServer::Box::Point(-60.0,-36.0,0.0),SharedJelloServer::Box::Point(60.0,60.0,96.0));
	int listenPortID=-1; // Assign any free port
	double updateTime=0.02; // Aim for 50 updates/sec
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"numAtoms")==0)
				{
				/* Read the number of atoms: */
				++i;
				for(int j=0;j<3&&i<argc;++j,++i)
					numAtoms[j]=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"domain")==0)
				{
				/* Read the simulation domain: */
				++i;
				for(int j=0;j<3&&i<argc;++j,++i)
					domain.min[j]=SharedJelloServer::Box::Scalar(atof(argv[i]));
				for(int j=0;j<3&&i<argc;++j,++i)
					domain.max[j]=SharedJelloServer::Box::Scalar(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"port")==0)
				{
				/* Read the server listening port: */
				++i;
				if(i<argc)
					listenPortID=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"tick")==0)
				{
				/* Read the server update time interval: */
				++i;
				if(i<argc)
					updateTime=atof(argv[i]);
				}
			}
		}
	
	/* Ignore SIGPIPE and leave handling of pipe errors to TCP sockets: */
	struct sigaction sigPipeAction;
	sigPipeAction.sa_handler=SIG_IGN;
	sigemptyset(&sigPipeAction.sa_mask);
	sigPipeAction.sa_flags=0x0;
	sigaction(SIGPIPE,&sigPipeAction,0);
	
	/* Create a shared Jell-O server: */
	SharedJelloServer sjs(numAtoms,domain,listenPortID);
	std::cout<<"SharedJelloServer::main: Created Jell-O server listening on port "<<sjs.getListenPortID()<<std::endl<<std::flush;
	
	/* Run the simulation loop full speed: */
	Misc::Timer timer;
	double lastFrameTime=timer.peekTime();
	double nextUpdateTime=timer.peekTime()+updateTime;
	int numFrames=0;
	while(true)
		{
		/* Calculate the current time step duration: */
		double newFrameTime=timer.peekTime();
		double timeStep=newFrameTime-lastFrameTime;
		lastFrameTime=newFrameTime;
		
		/* Perform a simulation step: */
		sjs.simulate(timeStep);
		++numFrames;
		
		/* Check if it's time for a state update: */
		if(lastFrameTime>=nextUpdateTime)
			{
			/* Send a state update to all connected clients: */
			sjs.sendServerUpdate();
			
			// std::cout<<"\rFrame rate: "<<double(numFrames)/updateTime<<" fps"<<std::flush;
			nextUpdateTime+=updateTime;
			numFrames=0;
			}
		}
	
	return 0;
	}
