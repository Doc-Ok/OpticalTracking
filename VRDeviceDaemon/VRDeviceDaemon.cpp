/***********************************************************************
VRDeviceDaemon - Daemon for distributed VR device driver architecture.
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <iostream>
#include <stdexcept>
#include <Misc/ConfigurationFile.h>
#include <Threads/MutexCond.h>

#include <VRDeviceDaemon/VRDeviceManager.h>
#include <VRDeviceDaemon/VRDeviceServer.h>

bool shutdown;
Threads::MutexCond shutdownCond;

void signalHandler(int signalId)
	{
	switch(signalId)
		{
		case SIGHUP:
			/* Restart server: */
			{
			Threads::MutexCond::Lock shutdownLock(shutdownCond);
			shutdown=false;
			shutdownCond.broadcast();
			}
			break;
		
		case SIGINT:
		case SIGTERM:
			/* Shut down server: */
			{
			Threads::MutexCond::Lock shutdownLock(shutdownCond);
			shutdown=true;
			shutdownCond.broadcast();
			}
			break;
		}
	return;
	}

int main(int argc,char* argv[])
	{
	/* Parse command line: */
	bool daemonize=false;
	const char* configFileName=VRDEVICEDAEMON_CONFIG_CONFIGFILENAME;
	const char* rootSectionName=0;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"D")==0)
				daemonize=true;
			else if(strcasecmp(argv[i]+1,"rootSection")==0)
				{
				++i;
				if(i<argc)
					rootSectionName=argv[i];
				}
			}
		else
			configFileName=argv[i];
		}
	
	if(daemonize)
		{
		/***************************
		Daemonize the device daemon:
		***************************/
		
		/* Fork once (and exit) to notify shell or caller that program is done: */
		int childPid=fork();
		if(childPid<0)
			{
			std::cerr<<"Error during fork"<<std::endl<<std::flush;
			return 1; // Fork error
			}
		else if(childPid>0)
			{
			/* Save daemon's process ID to pid file: */
			int pidFd=open("/var/run/VRDeviceDaemon.pid",O_RDWR|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
			if(pidFd>=0)
				{
				/* Write process ID: */
				char pidBuffer[20];
				snprintf(pidBuffer,sizeof(pidBuffer),"%d\n",childPid);
				size_t pidLen=strlen(pidBuffer);
				if(write(pidFd,pidBuffer,pidLen)!=ssize_t(pidLen))
					std::cerr<<"Could not write PID to PID file"<<std::endl;
				close(pidFd);
				}
			return 0; // Parent process exits
			}
		
		/* Set new session ID to become independent process without controlling tty: */
		setsid();
		
		/* Close all inherited file descriptors: */
		for(int i=getdtablesize()-1;i>=0;--i)
			close(i);
		
		#if 0
		/* Redirect stdin, stdout and stderr to /dev/null: */
		// int nullFd=open("/dev/null",O_RDWR); // Ugly hack; descriptors are assigned sequentially
		// dup(nullFd);
		// dup(nullFd);
		#else
		/* Redirect stdin, stdout and stderr to log file (this is ugly, but works because descriptors are assigned sequentially): */
		int logFd=open("/var/log/VRDeviceDaemon.log",O_RDWR|O_CREAT|O_TRUNC,S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
		if(logFd!=0||dup(logFd)!=1||dup(logFd)!=2)
			std::cerr<<"Error while rerouting output to log file"<<std::endl;
		#endif
		
		/* Ignore most signals: */
		struct sigaction sigIgnore;
		sigIgnore.sa_handler=SIG_IGN;
		sigemptyset(&sigIgnore.sa_mask);
		sigIgnore.sa_flags=0x0;
		sigaction(SIGCHLD,&sigIgnore,0);
		sigaction(SIGTSTP,&sigIgnore,0);
		sigaction(SIGTTOU,&sigIgnore,0);
		sigaction(SIGTTIN,&sigIgnore,0);
		}
	
	/* Ignore SIGPIPE and leave handling of pipe errors to TCP sockets: */
	struct sigaction sigPipeAction;
	sigPipeAction.sa_handler=SIG_IGN;
	sigemptyset(&sigPipeAction.sa_mask);
	sigPipeAction.sa_flags=0x0;
	sigaction(SIGPIPE,&sigPipeAction,0);
	
	/* Install signal handler for SIGHUP to re-start the daemon: */
	struct sigaction sigHupAction;
	sigHupAction.sa_handler=signalHandler;
	sigemptyset(&sigHupAction.sa_mask);
	sigHupAction.sa_flags=0x0;
	sigaction(SIGHUP,&sigHupAction,0);
	
	/* Install signal handlers for SIGINT and SIGTERM to exit cleanly: */
	struct sigaction sigIntAction;
	sigIntAction.sa_handler=signalHandler;
	sigemptyset(&sigIntAction.sa_mask);
	sigIntAction.sa_flags=0x0;
	sigaction(SIGINT,&sigIntAction,0);
	struct sigaction sigTermAction;
	sigTermAction.sa_handler=signalHandler;
	sigemptyset(&sigTermAction.sa_mask);
	sigTermAction.sa_flags=0x0;
	sigaction(SIGTERM,&sigTermAction,0);
	
	while(true)
		{
		/* Open configuration file: */
		#ifdef VERBOSE
		std::cout<<"VRDeviceDaemon: Reading configuration file"<<std::endl<<std::flush;
		#endif
		Misc::ConfigurationFile* configFile=0;
		try
			{
			configFile=new Misc::ConfigurationFile(const_cast<const char*>(configFileName));
			}
		catch(std::runtime_error error)
			{
			std::cerr<<"VRDeviceDaemon: Caught exception "<<error.what()<<" while reading configuration file"<<std::endl<<std::flush;
			return 1;
			}
		
		/* Set current section to given root section or name of current machine: */
		if(rootSectionName==0||rootSectionName[0]=='\0')
			rootSectionName=getenv("HOSTNAME");
		if(rootSectionName==0||rootSectionName[0]=='\0')
			rootSectionName=getenv("HOST");
		if(rootSectionName==0||rootSectionName[0]=='\0')
			rootSectionName="localhost";
		configFile->setCurrentSection(rootSectionName);
		
		/* Initialize devices: */
		#ifdef VERBOSE
		std::cout<<"VRDeviceDaemon: Initializing device manager"<<std::endl<<std::flush;
		#endif
		VRDeviceManager* deviceManager=0;
		configFile->setCurrentSection("./DeviceManager");
		try
			{
			deviceManager=new VRDeviceManager(*configFile);
			}
		catch(std::runtime_error error)
			{
			std::cerr<<"VRDeviceDaemon: Caught exception "<<error.what()<<" while initializing VR devices"<<std::endl<<std::flush;
			delete configFile;
			return 1;
			}
		configFile->setCurrentSection("..");
		
		/* Initialize device server: */
		#ifdef VERBOSE
		std::cout<<"VRDeviceDaemon: Initializing device server"<<std::endl<<std::flush;
		#endif
		VRDeviceServer* deviceServer=0;
		configFile->setCurrentSection("./DeviceServer");
		try
			{
			deviceServer=new VRDeviceServer(deviceManager,*configFile);
			}
		catch(std::runtime_error error)
			{
			std::cerr<<"VRDeviceDaemon: Caught exception "<<error.what()<<" while initializing VR device server"<<std::endl<<std::flush;
			delete configFile;
			delete deviceManager;
			return 1;
			}
		configFile->setCurrentSection("..");
		
		/* Go back to root section: */
		configFile->setCurrentSection("..");
		
		/* Create shutdown condition variable: */
		shutdown=false;
		
		/* Wait for restart or shutdown: */
		shutdownCond.wait();
		
		/* Clean up: */
		delete deviceServer;
		deviceServer=0;
		delete deviceManager;
		deviceManager=0;
		delete configFile;
		configFile=0;
		
		/* Shut down the device daemon if SIGINT or SIGTERM were caught: */
		if(!daemonize||shutdown)
			{
			#ifdef VERBOSE
			std::cout<<"VRDeviceDaemon: Shutting down daemon"<<std::endl<<std::flush;
			#endif
			break;
			}
		else
			{
			#ifdef VERBOSE
			std::cout<<"VRDeviceDaemon: Restarting daemon"<<std::endl<<std::flush;
			#endif
			}
		}
	
	/* Exit: */
	#if 0
	if(daemonize)
		{
		/* Close all file descriptors and remove the log file: */
		for(int i=getdtablesize()-1;i>=0;--i)
			close(i);
		// unlink("/var/log/DeviceDaemon.log");
		}
	#endif
	return 0;
	}
