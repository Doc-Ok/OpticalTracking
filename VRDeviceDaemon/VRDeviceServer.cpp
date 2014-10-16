/***********************************************************************
VRDeviceServer - Class encapsulating the VR device protocol's server
side.
Copyright (c) 2002-2014 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <VRDeviceDaemon/VRDeviceServer.h>

#include <stdio.h>
#include <stdexcept>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Vrui/Internal/VRDeviceDescriptor.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/*******************************
Methods of class VRDeviceServer:
*******************************/

void* VRDeviceServer::listenThreadMethod(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Wait for the next incoming connection: */
		#ifdef VERBOSE
		printf("VRDeviceServer: Waiting for client connection\n");
		fflush(stdout);
		#endif
		ClientData* newClient=new ClientData(listenSocket);
		
		/* Connect the new client: */
		#ifdef VERBOSE
		printf("VRDeviceServer: Connecting new client from %s, port %d\n",newClient->pipe.getPeerHostName().c_str(),newClient->pipe.getPeerPortId());
		fflush(stdout);
		#endif
		{
		Threads::Mutex::Lock clientListLock(clientListMutex);
		clientList.push_back(newClient);
		newClient->communicationThread.start(this,&VRDeviceServer::clientCommunicationThreadMethod,newClient);
		}
		}
	
	return 0;
	}

void* VRDeviceServer::clientCommunicationThreadMethod(VRDeviceServer::ClientData* clientData)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	Vrui::VRDevicePipe& pipe=clientData->pipe;
	
	enum State
		{
		START,CONNECTED,ACTIVE,STREAMING,FINISH
		};
	
	try
		{
		/* Execute client communication protocol state machine: */
		State state=START; // Current client state
		while(state!=FINISH)
			{
			/* Read the next message from the client: */
			Vrui::VRDevicePipe::MessageIdType message=pipe.readMessage();
			
			/* Handle the message based on the current state: */
			switch(state)
				{
				case START:
					switch(message)
						{
						case Vrui::VRDevicePipe::CONNECT_REQUEST:
							{
							/* Read client's protocol version number: */
							clientData->protocolVersion=pipe.read<unsigned int>();
							
							/* Lock the pipe for writing: */
							Threads::Mutex::Lock pipeLock(clientData->pipeMutex);
							
							/* Send connect reply message: */
							pipe.writeMessage(Vrui::VRDevicePipe::CONNECT_REPLY);
							if(clientData->protocolVersion>Vrui::VRDevicePipe::protocolVersionNumber)
								clientData->protocolVersion=Vrui::VRDevicePipe::protocolVersionNumber;
							pipe.write<unsigned int>(clientData->protocolVersion);
							
							/* Send server layout: */
							deviceManager->getState().writeLayout(pipe);
							
							/* Check if the client expects virtual device descriptors: */
							if(clientData->protocolVersion>=2U)
								{
								/* Send the layout of all virtual devices: */
								pipe.write<int>(deviceManager->getNumVirtualDevices());
								for(int deviceIndex=0;deviceIndex<deviceManager->getNumVirtualDevices();++deviceIndex)
									deviceManager->getVirtualDevice(deviceIndex).write(pipe);
								}
							
							/* Check if the client expects tracker state time stamps: */
							clientData->clientExpectsTimeStamps=clientData->protocolVersion>=3U;
							
							pipe.flush();
							}
							
							/* Go to connected state: */
							state=CONNECTED;
							break;
						
						default:
							state=FINISH;
						}
					break;
				
				case CONNECTED:
					switch(message)
						{
						case Vrui::VRDevicePipe::ACTIVATE_REQUEST:
							{
							/* Lock the client list: */
							Threads::Mutex::Lock clientListLock(clientListMutex);
							
							/* Start VR devices if this is the first active client: */
							if(numActiveClients==0)
								deviceManager->start();
							
							/* Activate the client: */
							clientData->active=true;
							++numActiveClients;
							}
							
							/* Go to active state: */
							state=ACTIVE;
							break;
						
						default:
							state=FINISH;
						}
					break;
				
				case ACTIVE:
					switch(message)
						{
						case Vrui::VRDevicePipe::PACKET_REQUEST:
						case Vrui::VRDevicePipe::STARTSTREAM_REQUEST:
							deviceManager->lockState();
							try
								{
								/* Lock the pipe for writing: */
								Threads::Mutex::Lock pipeLock(clientData->pipeMutex);
								
								if(message==Vrui::VRDevicePipe::STARTSTREAM_REQUEST)
									{
									/* Enable streaming: */
									clientData->streaming=true;
									}
								
								/* Send packet reply message: */
								pipe.writeMessage(Vrui::VRDevicePipe::PACKET_REPLY);
								
								/* Send server state: */
								deviceManager->getState().write(pipe,clientData->clientExpectsTimeStamps);
								pipe.flush();
								}
							catch(...)
								{
								/* Unlock the device manager's state and throw the exception again: */
								deviceManager->unlockState();
								throw;
								}
							deviceManager->unlockState();
							
							if(message==Vrui::VRDevicePipe::STARTSTREAM_REQUEST)
								state=STREAMING;
							
							break;
						
						case Vrui::VRDevicePipe::DEACTIVATE_REQUEST:
							{
							/* Lock the client list: */
							Threads::Mutex::Lock clientListLock(clientListMutex);
							
							/* Deactivate client: */
							clientData->active=false;
							--numActiveClients;
							
							/* Stop VR devices if this was the last active client: */
							if(numActiveClients==0)
								deviceManager->stop();
							}
							
							/* Go to connected state: */
							state=CONNECTED;
							break;
						
						default:
							state=FINISH;
						}
					break;
				
				case STREAMING:
					switch(message)
						{
						case Vrui::VRDevicePipe::PACKET_REQUEST:
							/* Ignore message: */
							break;
						
						case Vrui::VRDevicePipe::STOPSTREAM_REQUEST:
							{
							/* Lock the pipe for writing: */
							Threads::Mutex::Lock pipeLock(clientData->pipeMutex);
							
							/* Disable streaming: */
							clientData->streaming=false;
							
							/* Send stopstream reply message: */
							pipe.writeMessage(Vrui::VRDevicePipe::STOPSTREAM_REPLY);
							pipe.flush();
							}
							
							/* Go to active state: */
							state=ACTIVE;
							break;
						
						default:
							state=FINISH;
						}
					break;
				
				default:
					/* Just to make g++ happy... */
					;
				}
			}
		}
	catch(std::runtime_error err)
		{
		/* Print error message to stderr, but ignore exception otherwise: */
		fprintf(stderr,"VRDeviceServer: Terminating client connection due to exception\n  %s\n",err.what());
		fflush(stderr);
		}
	
	/* Cleanly deactivate client: */
	{
	Threads::Mutex::Lock clientListLock(clientListMutex);
	if(clientData->streaming)
		{
		/* Leave streaming mode: */
		clientData->streaming=false;
		}
	if(clientData->active)
		{
		/* Deactivate client: */
		clientData->active=false;
		--numActiveClients;
		
		/* Stop VR devices if this was the last active client: */
		if(numActiveClients==0)
			deviceManager->stop();
		}
	
	/* Remove client from list: */
	ClientList::iterator clIt;
	for(clIt=clientList.begin();clIt!=clientList.end()&&*clIt!=clientData;++clIt)
		;
	clientList.erase(clIt);
	
	/* Disconnect client: */
	delete clientData;
	}
	
	/* Terminate: */
	#ifdef VERBOSE
	printf("VRDeviceServer: Disconnected client\n");
	fflush(stdout);
	#endif
	
	return 0;
	}

void* VRDeviceServer::streamingThreadMethod(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Wait for the next update notification from the device manager: */
		trackerUpdateCompleteCond.wait();
		
		/* Lock client list: */
		{
		Threads::Mutex::Lock clientListLock(clientListMutex);
		
		/* Lock the device manager's current state: */
		deviceManager->lockState();
		
		/* Iterate through all clients in streaming mode: */
		std::vector<ClientList::iterator> deadClients;
		for(ClientList::iterator clIt=clientList.begin();clIt!=clientList.end();++clIt)
			{
			/* Lock the client's pipe: */
			Threads::Mutex::Lock clientPipeLock((*clIt)->pipeMutex);
			
			if((*clIt)->streaming)
				{
				try
					{
					/* Send packet reply message: */
					(*clIt)->pipe.writeMessage(Vrui::VRDevicePipe::PACKET_REPLY);
					
					/* Send server state: */
					deviceManager->getState().write((*clIt)->pipe,(*clIt)->clientExpectsTimeStamps);
					(*clIt)->pipe.flush();
					}
				catch(std::runtime_error err)
					{
					/* Print error message to stderr and mark client for termination: */
					fprintf(stderr,"VRDeviceServer: Terminating client connection due to exception\n  %s\n",err.what());
					fflush(stderr);
					deadClients.push_back(clIt);
					}
				catch(...)
					{
					/* Print error message to stderr and mark client for termination: */
					fprintf(stderr,"VRDeviceServer: Terminating client connection due to spurious exception\n");
					fflush(stderr);
					deadClients.push_back(clIt);
					
					/* Re-throw exception to let the thread cancel: */
					throw;
					}
				}
			}
		
		/* Unlock the device manager's state: */
		deviceManager->unlockState();
		
		/* Disconnect all dead clients: */
		for(std::vector<ClientList::iterator>::iterator dcIt=deadClients.begin();dcIt!=deadClients.end();++dcIt)
			{
			/* Stop client communication thread: */
			(**dcIt)->communicationThread.cancel();
			(**dcIt)->communicationThread.join();
			
			/* Cleanly deactivate client: */
			if((**dcIt)->streaming)
				{
				/* Leave streaming mode: */
				(**dcIt)->streaming=false;
				}
			if((**dcIt)->active)
				{
				/* Deactivate client: */
				(**dcIt)->active=false;
				--numActiveClients;
				
				/* Stop VR devices if this was the last active client: */
				if(numActiveClients==0)
					deviceManager->stop();
				}
			delete **dcIt;
			
			/* Remove client from list: */
			clientList.erase(*dcIt);
			}
		}
		}
	
	return 0;
	}

VRDeviceServer::VRDeviceServer(VRDeviceManager* sDeviceManager,const Misc::ConfigurationFile& configFile)
	:deviceManager(sDeviceManager),
	 listenSocket(configFile.retrieveValue<int>("./serverPort"),-1),
	 numActiveClients(0)
	{
	/* Enable tracker update notification: */
	deviceManager->enableTrackerUpdateNotification(&trackerUpdateCompleteCond);
	
	/* Start connection initiating thread: */
	listenThread.start(this,&VRDeviceServer::listenThreadMethod);
	
	/* Start streaming thread: */
	streamingThread.start(this,&VRDeviceServer::streamingThreadMethod);
	}

VRDeviceServer::~VRDeviceServer(void)
	{
	/* Lock client list: */
	{
	Threads::Mutex::Lock clientListLock(clientListMutex);
	
	/* Stop streaming thread: */
	streamingThread.cancel();
	streamingThread.join();
	
	/* Stop connection initiating thread: */
	listenThread.cancel();
	listenThread.join();
	
	/* Disconnect all clients: */
	deviceManager->lockState();
	for(ClientList::iterator clIt=clientList.begin();clIt!=clientList.end();++clIt)
		{
		/* Stop client communication thread: */
		(*clIt)->communicationThread.cancel();
		(*clIt)->communicationThread.join();
		
		/* Delete client data object (closing TCP socket): */
		delete *clIt;
		}
	deviceManager->unlockState();
	
	/* Stop VR devices: */
	if(numActiveClients>0)
		deviceManager->stop();
	}
	
	/* Disable tracker update notification: */
	deviceManager->disableTrackerUpdateNotification();
	}
