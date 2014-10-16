/***********************************************************************
Clusterize - Helper functions to distribute an application across a
cluster, and establish communications between nodes using a multicast
pipe multiplexer.
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

#include <Cluster/Clusterize.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <Misc/StringMarshaller.h>
#include <Misc/GetCurrentDirectory.h>
#include <Cluster/Multiplexer.h>
#include <Cluster/MulticastPipe.h>

namespace Cluster {

namespace {

/****************
Helper functions:
****************/

void removeArg(int& argc,char**& argv,int arg)
	{
	--argc;
	for(int i=arg;i<argc;++i)
		argv[i]=argv[i+1];
	argv[argc]=0;
	}

/***************
Static elements:
***************/

int slaveArgc=0;
char** slaveArgv=0;
int numSlaves=0;
pid_t* slavePids=0;

}

Multiplexer* clusterize(int& argc,char**& argv)
	{
	Multiplexer* result=0;
	
	/* Determine whether this instance is on the master or a slave node: */
	if(argc==8&&strcmp(argv[1],"-clusterizeSlaveInstance")==0)
		{
		/********************
		This is a slave node:
		********************/
		
		/* Read multipipe settings from the command line: */
		int numSlaves=atoi(argv[2]);
		int nodeIndex=atoi(argv[3]);
		char* master=argv[4];
		int masterPort=atoi(argv[5]);
		char* multicastGroup=argv[6];
		int multicastPort=atoi(argv[7]);
		
		/* Connect back to the master: */
		try
			{
			/* Create the multicast multiplexer: */
			result=new Multiplexer(numSlaves,nodeIndex,master,masterPort,multicastGroup,multicastPort);
			
			/* Wait until the entire cluster is connected: */
			result->waitForConnection();
			
			{
			/* Read the application's command line via a multicast pipe: */
			MulticastPipe argPipe(result);
			slaveArgc=argPipe.read<int>();
			slaveArgv=new char*[slaveArgc+1];
			for(int i=0;i<=slaveArgc;++i)
				slaveArgv[i]=0;
			for(int i=0;i<slaveArgc;++i)
				slaveArgv[i]=Misc::readCString(argPipe);
			}
			
			/* Override the actual command line provided by the caller: */
			argc=slaveArgc;
			argv=slaveArgv;
			}
		catch(std::runtime_error error)
			{
			std::cerr<<"Node "<<nodeIndex<<": Caught exception "<<error.what()<<" while initializing cluster communication"<<std::endl;
			delete result;
			result=0;
			}
		}
	else
		{
		/* Read and remove clusterization arguments from the command line: */
		const char* hostname=getenv("HOSTNAME");
		if(hostname==0)
			hostname=getenv("HOST");
		std::string master=hostname;
		int masterPort=26000;
		std::vector<std::string> slaves;
		std::string multicastGroup;
		int multicastPort=26000;
		std::string remoteCommand="ssh";
		for(int i=1;i<argc;++i)
			{
			if(argv[i][0]=='-')
				{
				if(strcasecmp(argv[i],"-master")==0)
					{
					removeArg(argc,argv,i);
					if(i<argc)
						{
						master=argv[i];
						removeArg(argc,argv,i);
						}
					--i;
					}
				else if(strcasecmp(argv[i],"-masterPort")==0)
					{
					removeArg(argc,argv,i);
					if(i<argc)
						{
						masterPort=atoi(argv[i]);
						removeArg(argc,argv,i);
						}
					--i;
					}
				else if(strcasecmp(argv[i],"-slaves")==0)
					{
					removeArg(argc,argv,i);
					if(i<argc)
						{
						int numSlaves=atoi(argv[i]);
						removeArg(argc,argv,i);
						for(int j=0;j<numSlaves&&i<argc;++j)
							{
							slaves.push_back(argv[i]);
							removeArg(argc,argv,i);
							}
						}
					--i;
					}
				else if(strcasecmp(argv[i],"-multicastGroup")==0)
					{
					removeArg(argc,argv,i);
					if(i<argc)
						{
						multicastGroup=argv[i];
						removeArg(argc,argv,i);
						}
					--i;
					}
				else if(strcasecmp(argv[i],"-multicastPort")==0)
					{
					removeArg(argc,argv,i);
					if(i<argc)
						{
						multicastPort=atoi(argv[i]);
						removeArg(argc,argv,i);
						}
					--i;
					}
				else if(strcasecmp(argv[i],"-remoteCommand")==0)
					{
					removeArg(argc,argv,i);
					if(i<argc)
						{
						remoteCommand=argv[i];
						removeArg(argc,argv,i);
						}
					--i;
					}
				}
			}
		
		if(!slaves.empty()&&!multicastGroup.empty())
			{
			try
				{
				/* Create the multicast multiplexer: */
				result=new Multiplexer(slaves.size(),0,master.c_str(),masterPort,multicastGroup.c_str(),multicastPort);
				masterPort=result->getLocalPortNumber();
				
				/* Start the multipipe slaves on all slave nodes: */
				numSlaves=int(slaves.size());
				slavePids=new pid_t[numSlaves];
				std::string cwd=Misc::getCurrentDirectory();
				size_t rcLen=cwd.length()+strlen(argv[0])+master.length()+multicastGroup.length()+512;
				char* rc=new char[rcLen];
				for(int i=0;i<numSlaves;++i)
					{
					pid_t childPid=fork();
					if(childPid==0)
						{
						/* Delete the multicast pipe multiplexer in the child instance: */
						delete result;
						result=0;
						
						/* Create a command line to run the program from the current working directory: */
						int ai=0;
						ai+=snprintf(rc+ai,rcLen-ai,"cd %s ;",cwd.c_str());
						ai+=snprintf(rc+ai,rcLen-ai," %s",argv[0]);
						ai+=snprintf(rc+ai,rcLen-ai," -clusterizeSlaveInstance");
						ai+=snprintf(rc+ai,rcLen-ai," %d %d",numSlaves,i+1);
						ai+=snprintf(rc+ai,rcLen-ai," %s %d",master.c_str(),masterPort);
						ai+=snprintf(rc+ai,rcLen-ai," %s %d",multicastGroup.c_str(),multicastPort);
						
						/* Create command line for the ssh (or other remote login) program: */
						char* sshArgv[20];
						int sshArgc=0;
						sshArgv[sshArgc++]=const_cast<char*>(remoteCommand.c_str());
						sshArgv[sshArgc++]=const_cast<char*>(slaves[i].c_str());
						sshArgv[sshArgc++]=rc;
						sshArgv[sshArgc]=0;
						
						/* Run the remote login program: */
						execvp(sshArgv[0],sshArgv);
						}
					else
						{
						/* Store PID of ssh process for later: */
						slavePids[i]=childPid;
						}
					}
				
				/* Clean up: */
				delete[] rc;
				
				/* Wait until the entire cluster is connected: */
				result->waitForConnection();
				
				{
				/* Write the application's command line to a multicast pipe: */
				MulticastPipe argPipe(result);
				argPipe.write<int>(argc);
				for(int i=0;i<argc;++i)
					Misc::writeCString(argv[i],argPipe);
				}
				}
			catch(std::runtime_error error)
				{
				std::cerr<<"Master node: Caught exception "<<error.what()<<" while initializing cluster communication"<<std::endl;
				delete result;
				result=0;
				}
			}
		}
	
	return result;
	}

void unclusterize(Multiplexer* multiplexer)
	{
	if(multiplexer!=0)
		{
		bool master=multiplexer->isMaster();
		
		/* Destroy the multiplexer: */
		delete multiplexer;
		
		if(master&&numSlaves>0&&slavePids!=0)
			{
			/* Wait for all slaves to terminate: */
			for(int i=0;i<numSlaves;++i)
				waitpid(slavePids[i],0,0);
			delete[] slavePids;
			numSlaves=0;
			slavePids=0;
			}
		if(!master&&slaveArgc>0&&slaveArgv!=0)
			{
			/* Delete the slave command line: */
			for(int i=0;i<slaveArgc;++i)
				delete[] slaveArgv[i];
			delete[] slaveArgv;
			slaveArgc=0;
			slaveArgv=0;
			}
		}
	}

}
