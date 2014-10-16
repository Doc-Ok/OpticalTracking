/***********************************************************************
Environment-dependent part of Vrui virtual reality development toolkit.
Copyright (c) 2000-2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <Misc/StringHashFunctions.h>
#include <Misc/HashTable.h>
#include <Misc/FdSet.h>
#include <Misc/File.h>
#include <Misc/Timer.h>
#include <Misc/StringMarshaller.h>
#include <Misc/GetCurrentDirectory.h>
#include <Misc/FileNameExtensions.h>
#include <Misc/FileTests.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/TimerEventScheduler.h>
#include <Realtime/Time.h>
#include <Threads/Thread.h>
#include <Threads/Mutex.h>
#include <Threads/Barrier.h>
#include <Cluster/Multiplexer.h>
#include <Cluster/MulticastPipe.h>
#include <Cluster/ThreadSynchronizer.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Plane.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/AffineTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/Config.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <X11/keysym.h>
#include <GLMotif/Event.h>
#include <GLMotif/Popup.h>
#include <AL/Config.h>
#include <AL/ALContextData.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Internal/InputDeviceAdapterMouse.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/VRWindow.h>
#include <Vrui/SoundContext.h>
#include <Vrui/ToolManager.h>
#include <Vrui/VisletManager.h>
#include <Vrui/ViewSpecification.h>

#include <Vrui/Internal/Vrui.h>
#include <Vrui/Internal/Config.h>

namespace Vrui {

struct VruiWindowGroup
	{
	/* Embedded classes: */
	public:
	struct Window
		{
		/* Elements: */
		public:
		VRWindow* window; // Pointer to window
		int viewportSize[2]; // Window's current maximal viewport size
		int frameSize[2]; // Window's current maximal frame buffer size
		};
	
	/* Elements: */
	Display* display; // Display connection shared by all windows in the window group
	int displayFd; // File descriptor for the display connection
	std::vector<Window> windows; // List of pointers to windows in the window group
	int maxViewportSize[2]; // Maximum current viewport size of all windows in the group
	int maxFrameSize[2]; // Maximum current frame buffer size of all windows in the group
	};

namespace {

/***********************************
Workbench-specific global variables:
***********************************/

bool vruiVerbose=false;
int vruiEventPipe[2]={-1,-1};
Threads::Mutex vruiEventPipeMutex;
volatile unsigned int vruiNumSignaledEvents=0;
Misc::ConfigurationFile* vruiConfigFile=0;
char* vruiApplicationName=0;
int vruiNumWindows=0;
VRWindow** vruiWindows=0;
int vruiNumWindowGroups=0;
VruiWindowGroup* vruiWindowGroups=0;
int vruiTotalNumWindows=0;
VRWindow** vruiTotalWindows=0;
#if GLSUPPORT_CONFIG_USE_TLS
Threads::Thread* vruiRenderingThreads=0;
Threads::Barrier vruiRenderingBarrier;
volatile bool vruiStopRenderingThreads=false;
#endif
int vruiNumSoundContexts=0;
SoundContext** vruiSoundContexts=0;
Cluster::Multiplexer* vruiMultiplexer=0;
Cluster::MulticastPipe* vruiPipe=0;
int vruiNumSlaves=0;
pid_t* vruiSlavePids=0;
int vruiSlaveArgc=0;
char** vruiSlaveArgv=0;
char** vruiSlaveArgvShadow=0;
volatile bool vruiAsynchronousShutdown=false;

/*****************************************
Workbench-specific private Vrui functions:
*****************************************/

/* Signal handler to shut down Vrui if something goes wrong: */
void vruiTerminate(int)
	{
	/* Request an asynchronous shutdown: */
	vruiAsynchronousShutdown=true;
	}

/* Generic cleanup function called in case of an error: */
void vruiErrorShutdown(bool signalError)
	{
	if(signalError)
		{
		if(vruiMultiplexer!=0)
			{
			/* Signal a fatal error to all nodes and let them die: */
			//vruiMultiplexer->fatalError();
			}
		
		/* Return with an error condition: */
		exit(1);
		}
	
	/* Clean up: */
	vruiState->finishMainLoop();
	GLContextData::shutdownThingManager();
	#if GLSUPPORT_CONFIG_USE_TLS
	if(vruiRenderingThreads!=0)
		{
		/* Cancel all rendering threads: */
		for(int i=0;i<vruiNumWindowGroups;++i)
			{
			vruiRenderingThreads[i].cancel();
			vruiRenderingThreads[i].join();
			}
		delete[] vruiRenderingThreads;
		vruiRenderingThreads=0;
		}
	#endif
	if(vruiWindows!=0)
		{
		/* Release all OpenGL state: */
		for(int i=0;i<vruiNumWindowGroups;++i)
			{
			for(std::vector<VruiWindowGroup::Window>::iterator wgIt=vruiWindowGroups[i].windows.begin();wgIt!=vruiWindowGroups[i].windows.end();++wgIt)
				wgIt->window->deinit();
			vruiWindowGroups[i].windows.front().window->getContext().deinit();
			}
		
		/* Delete all windows: */
		for(int i=0;i<vruiNumWindows;++i)
			delete vruiWindows[i];
		delete[] vruiWindows;
		vruiWindows=0;
		delete[] vruiWindowGroups;
		vruiWindowGroups=0;
		delete[] vruiTotalWindows;
		vruiTotalWindows=0;
		}
	ALContextData::shutdownThingManager();
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	if(vruiSoundContexts!=0)
		{
		/* Destroy all sound contexts: */
		for(int i=0;i<vruiNumSoundContexts;++i)
			delete vruiSoundContexts[i];
		delete[] vruiSoundContexts;
		}
	#endif
	delete[] vruiApplicationName;
	delete vruiState;
	
	if(vruiMultiplexer!=0)
		{
		bool master=vruiMultiplexer->isMaster();
		
		/* Destroy the multiplexer: */
		delete vruiPipe;
		delete vruiMultiplexer;
		
		if(master&&vruiSlavePids!=0)
			{
			/* Wait for all slaves to terminate: */
			for(int i=0;i<vruiNumSlaves;++i)
				waitpid(vruiSlavePids[i],0,0);
			delete[] vruiSlavePids;
			vruiSlavePids=0;
			}
		if(!master&&vruiSlaveArgv!=0)
			{
			/* Delete the slave command line: */
			for(int i=0;i<vruiSlaveArgc;++i)
				delete[] vruiSlaveArgv[i];
			delete[] vruiSlaveArgv;
			vruiSlaveArgv=0;
			delete[] vruiSlaveArgvShadow;
			vruiSlaveArgvShadow=0;
			}
		}
	
	/* Close the configuration file: */
	delete vruiConfigFile;
	
	if(vruiEventPipe[0]>=0)
		{
		/* Close the event pipe: */
		close(vruiEventPipe[0]);
		close(vruiEventPipe[1]);
		}
	}

std::string vruiCreateConfigurationFilePath(const char* fullName,const char* configFileName)
	{
	/* Find the path prefix of the fully-qualified name: */
	const char* dirEndPtr=fullName;
	for(const char* fnPtr=fullName;*fnPtr!='\0';++fnPtr)
		if(*fnPtr=='/')
			dirEndPtr=fnPtr+1;
	
	/* Prepend the path prefix to the given configuration file name: */
	std::string result(fullName,dirEndPtr);
	result.append(configFileName);
	return result;
	}

bool vruiMergeConfigurationFile(const char* configFileName)
	{
	try
		{
		/* Merge in the given configuration file: */
		if(vruiVerbose)
			std::cout<<"Vrui: Merging configuration file "<<configFileName<<"..."<<std::flush;
		vruiConfigFile->merge(configFileName);
		if(vruiVerbose)
			std::cout<<" Ok"<<std::endl;
		
		return true;
		}
	catch(Misc::File::OpenError err)
		{
		/* Ignore the error and continue */
		if(vruiVerbose)
			std::cout<<" does not exist"<<std::endl;
		
		return false;
		}
	catch(std::runtime_error error)
		{
		/* Bail out on errors in user configuration file: */
		if(vruiVerbose)
			std::cout<<" error"<<std::endl;
		std::cerr<<"Caught exception "<<error.what()<<" while merging configuration file "<<configFileName<<std::endl;
		vruiErrorShutdown(true);
		
		return false;
		}
	}

void vruiOpenConfigurationFile(const char* userConfigFileName)
	{
	try
		{
		/* Open the system-wide configuration file: */
		if(vruiVerbose)
			std::cout<<"Vrui: Reading system-wide configuration file "<<VRUI_INTERNAL_CONFIG_SYSCONFIGFILE<<std::endl;
		vruiConfigFile=new Misc::ConfigurationFile(VRUI_INTERNAL_CONFIG_SYSCONFIGFILE);
		}
	catch(std::runtime_error error)
		{
		/* Bail out: */
		std::cerr<<"Caught exception "<<error.what()<<" while reading system-wide configuration file "<<VRUI_INTERNAL_CONFIG_SYSCONFIGFILE<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Merge the global per-user configuration file if given: */
	if(userConfigFileName!=0)
		vruiMergeConfigurationFile(userConfigFileName);
	
	/* Get the name of the local per-application configuration file: */
	const char* localConfigFileName=getenv("VRUI_CONFIGFILE");
	if(localConfigFileName==0||localConfigFileName[0]=='\0')
		localConfigFileName="./Vrui.cfg";
	
	/* Merge in the local per-application configuration file: */
	vruiMergeConfigurationFile(localConfigFileName);
	}

void vruiGoToRootSection(const char*& rootSectionName)
	{
	try
		{
		/* Fall back to simulator mode if the root section does not exist: */
		bool rootSectionFound=false;
		if(rootSectionName==0)
			rootSectionName=VRUI_INTERNAL_CONFIG_DEFAULTROOTSECTION;
		Misc::ConfigurationFile::SectionIterator rootIt=vruiConfigFile->getRootSection().getSection("/Vrui");
		for(Misc::ConfigurationFile::SectionIterator sIt=rootIt.beginSubsections();sIt!=rootIt.endSubsections();++sIt)
			if(sIt.getName()==rootSectionName)
				{
				rootSectionFound=true;
				break;
				}
		if(!rootSectionFound)
			{
			if(vruiVerbose)
				std::cout<<"Vrui: Requested root section /Vrui/"<<rootSectionName<<" does not exist"<<std::endl;
			rootSectionName=VRUI_INTERNAL_CONFIG_DEFAULTROOTSECTION;
			}
		}
	catch(...)
		{
		/* Bail out if configuration file does not contain the Vrui section: */
		std::cerr<<"Configuration file does not contain /Vrui section"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Go to the given root section: */
	if(vruiVerbose)
		std::cout<<"Vrui: Going to root section /Vrui/"<<rootSectionName<<std::endl;
	vruiConfigFile->setCurrentSection("/Vrui");
	vruiConfigFile->setCurrentSection(rootSectionName);
	}

struct VruiWindowGroupCreator // Structure defining a group of windows rendered sequentially by the same thread
	{
	/* Embedded classes: */
	public:
	struct VruiWindow // Structure defining a window inside a window group
		{
		/* Elements: */
		public:
		int windowIndex; // Index of the window in Vrui's main window array
		Misc::ConfigurationFileSection windowConfigFileSection; // Configuration file section for the window
		};
	
	/* Elements: */
	public:
	std::vector<VruiWindow> windows; // List of the windows in this group
	InputDeviceAdapterMouse* mouseAdapter; // Pointer to the mouse input device adapter to be used for this window group
	};

bool vruiCreateWindowGroup(const VruiWindowGroupCreator& group)
	{
	VRWindow* firstWindow=0;
	bool allWindowsOk=true;
	for(std::vector<VruiWindowGroupCreator::VruiWindow>::const_iterator wIt=group.windows.begin();wIt!=group.windows.end();++wIt)
		{
		try
			{
			/* Create a unique name for the window: */
			char windowName[256];
			if(vruiNumWindows>1)
				snprintf(windowName,sizeof(windowName),"%s - %d",vruiApplicationName,wIt->windowIndex);
			else
				snprintf(windowName,sizeof(windowName),"%s",vruiApplicationName);
			
			if(firstWindow!=0)
				{
				/* Get the window's screen number: */
				int screen=wIt->windowConfigFileSection.retrieveValue<int>("./screen",firstWindow->getScreen());
				
				/* Create the new window: */
				vruiWindows[wIt->windowIndex]=new VRWindow(&firstWindow->getContext(),screen,windowName,wIt->windowConfigFileSection,vruiState,group.mouseAdapter);
				}
			else
				{
				/* Create a new OpenGL context: */
				GLContextPtr context(VRWindow::createContext(vruiState->windowProperties,wIt->windowConfigFileSection));
				
				/* Get the window's screen number: */
				int screen=wIt->windowConfigFileSection.retrieveValue<int>("./screen",context->getDefaultScreen());
				
				/* Create the window: */
				vruiWindows[wIt->windowIndex]=new VRWindow(context.getPointer(),screen,windowName,wIt->windowConfigFileSection,vruiState,group.mouseAdapter);
				
				firstWindow=vruiWindows[wIt->windowIndex];
				}
			
			vruiWindows[wIt->windowIndex]->getCloseCallbacks().add(vruiState,&VruiState::quitCallback);
			}
		catch(std::runtime_error err)
			{
			std::cerr<<"Caught exception "<<err.what()<<" while initializing rendering window "<<wIt->windowIndex<<std::endl;
			delete vruiWindows[wIt->windowIndex];
			vruiWindows[wIt->windowIndex]=0;
			
			/* Bail out: */
			allWindowsOk=false;
			break;
			}
		}
	
	/* Initialize all GLObjects for the first window's context data: */
	if(allWindowsOk)
		{
		firstWindow->makeCurrent();
		firstWindow->getContextData().updateThings();
		}
	
	return allWindowsOk;
	}

#if GLSUPPORT_CONFIG_USE_TLS

void* vruiRenderingThreadFunction(VruiWindowGroupCreator group)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Create all windows in this thread's group: */
	bool allWindowsOk=vruiCreateWindowGroup(group);
	
	/* Synchronize with the other rendering threads: */
	vruiRenderingBarrier.synchronize();
	
	/* Terminate early if there was a problem creating any rendering window: */
	if(!allWindowsOk)
		return 0;
	
	/* Enter the rendering loop and redraw all windows until interrupted: */
	while(true)
		{
		/* Wait for the start of the rendering cycle: */
		vruiRenderingBarrier.synchronize();
		
		/* Check for shutdown: */
		if(vruiStopRenderingThreads)
			break;
		
		/* Draw all windows' contents: */
		for(std::vector<VruiWindowGroupCreator::VruiWindow>::iterator wIt=group.windows.begin();wIt!=group.windows.end();++wIt)
			vruiWindows[wIt->windowIndex]->draw();
		
		/* Wait until all threads are done rendering: */
		glFinish();
		vruiRenderingBarrier.synchronize();
		
		if(vruiState->multiplexer)
			{
			/* Wait until all other nodes are done rendering: */
			vruiRenderingBarrier.synchronize();
			}
		
		/* Swap all windows' buffers: */
		for(std::vector<VruiWindowGroupCreator::VruiWindow>::iterator wIt=group.windows.begin();wIt!=group.windows.end();++wIt)
			{
			vruiWindows[wIt->windowIndex]->makeCurrent();
			vruiWindows[wIt->windowIndex]->swapBuffers();
			}
		
		/* Wait until all threads are done swapping buffers: */
		vruiRenderingBarrier.synchronize();
		}
	
	return 0;
	}

#endif

}

/**********************************
Call-in functions for user program:
**********************************/

void init(int& argc,char**& argv,char**&)
	{
	/* Determine whether this node is the master or a slave: */
	if(argc==8&&strcmp(argv[1],"-vruiMultipipeSlave")==0)
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
			vruiMultiplexer=new Cluster::Multiplexer(numSlaves,nodeIndex,master,masterPort,multicastGroup,multicastPort);
			
			/* Wait until the entire cluster is connected: */
			vruiMultiplexer->waitForConnection();
			
			/* Open a multicast pipe: */
			vruiPipe=new Cluster::MulticastPipe(vruiMultiplexer);
			
			/* Read the entire configuration file and the root section name: */
			vruiConfigFile=new Misc::ConfigurationFile(*vruiPipe);
			char* rootSectionName=Misc::readCString(*vruiPipe);
			
			/* Go to the given root section: */
			vruiConfigFile->setCurrentSection("/Vrui");
			vruiConfigFile->setCurrentSection(rootSectionName);
			delete[] rootSectionName;
			
			/* Read the application's command line: */
			vruiSlaveArgc=vruiPipe->read<int>();
			vruiSlaveArgv=new char*[vruiSlaveArgc+1];
			vruiSlaveArgvShadow=new char*[vruiSlaveArgc+1];
			for(int i=0;i<=vruiSlaveArgc;++i)
				vruiSlaveArgv[i]=0;
			for(int i=0;i<vruiSlaveArgc;++i)
				vruiSlaveArgv[i]=Misc::readCString(*vruiPipe);
			for(int i=0;i<=vruiSlaveArgc;++i)
				vruiSlaveArgvShadow[i]=vruiSlaveArgv[i];
			
			/* Override the actual command line provided by the caller: */
			argc=vruiSlaveArgc;
			argv=vruiSlaveArgvShadow;
			}
		catch(std::runtime_error error)
			{
			std::cerr<<"Node "<<nodeIndex<<": Caught exception "<<error.what()<<" while initializing cluster communication"<<std::endl;
			vruiErrorShutdown(true);
			}
		}
	else
		{
		/***********************
		This is the master node:
		***********************/
		
		/* Check the command line for -vruiVerbose and -vruiHelp flags: */
		for(int i=1;i<argc;++i)
			{
			if(strcasecmp(argv[i],"-vruiVerbose")==0)
				{
				std::cout<<"Vrui: Entering verbose mode"<<std::endl;
				vruiVerbose=true;
				
				/* Print information about the Vrui run-time installation: */
				std::cout<<"Vrui: Run-time version ";
				char prevFill=std::cout.fill('0');
				std::cout<<VRUI_INTERNAL_CONFIG_VERSION/1000000<<'.'<<(VRUI_INTERNAL_CONFIG_VERSION/1000)%1000<<'-'<<std::setw(3)<<VRUI_INTERNAL_CONFIG_VERSION%1000;
				std::cout.fill(prevFill);
				std::cout<<" installed in:"<<std::endl;
				std::cout<<"        libraries   : "<<VRUI_INTERNAL_CONFIG_LIBDIR<<std::endl;
				std::cout<<"        executables : "<<VRUI_INTERNAL_CONFIG_EXECUTABLEDIR<<std::endl;
				std::cout<<"        plug-ins    : "<<VRUI_INTERNAL_CONFIG_PLUGINDIR<<std::endl;
				std::cout<<"        config files: "<<VRUI_INTERNAL_CONFIG_ETCDIR<<std::endl;
				std::cout<<"        shared files: "<<VRUI_INTERNAL_CONFIG_SHAREDIR<<std::endl;
				
				/* Remove parameter from argument list: */
				argc-=1;
				for(int j=i;j<argc;++j)
					argv[j]=argv[j+1];
				--i;
				}
			else if(strcasecmp(argv[i]+1,"vruiHelp")==0)
				{
				/* Print information about Vrui command line options: */
				std::cout<<"Vrui-wide command line options:"<<std::endl;
				std::cout<<"  -vruiHelp"<<std::endl;
				std::cout<<"     Prints this help message"<<std::endl;
				std::cout<<"  -vruiVerbose"<<std::endl;
				std::cout<<"     Logs details about Vrui's startup and shutdown procedures to stdout"<<std::endl;
				std::cout<<"  -mergeConfig <configuration file name>"<<std::endl;
				std::cout<<"     Merges the configuration file of the given name into Vrui's"<<std::endl;
				std::cout<<"     configuration space"<<std::endl;
				std::cout<<"  -dumpConfig <configuration file name>"<<std::endl;
				std::cout<<"     Writes the current state of Vrui's configuration space, including"<<std::endl;
				std::cout<<"     all previously merged configuration files, to the configuration"<<std::endl;
				std::cout<<"     file of the given name"<<std::endl;
				std::cout<<"  -rootSection <root section name>"<<std::endl;
				std::cout<<"     Overrides the default root section name"<<std::endl;
				std::cout<<"  -loadInputGraph <input graph file name>"<<std::endl;
				std::cout<<"     Loads the input graph contained in the given file after initialization"<<std::endl;
				std::cout<<"  -addToolClass <tool class name>"<<std::endl;
				std::cout<<"     Adds the tool class of the given name to the tool manager and the"<<std::endl;
				std::cout<<"     tool selection menu"<<std::endl;
				std::cout<<"  -addTool <tool configuration file section name>"<<std::endl;
				std::cout<<"     Adds the tool defined in the given tool configuration section"<<std::endl;
				std::cout<<"  -vislet <vislet class name> [vislet option 1] ... [vislet option n] ;"<<std::endl;
				std::cout<<"     Loads a vislet of the given class name, with the given vislet"<<std::endl;
				std::cout<<"     arguments. Argument list must be terminated with a semicolon"<<std::endl;
				std::cout<<"  -setLinearUnit <unit name> <unit scale factor>"<<std::endl;
				std::cout<<"     Sets the coordinate unit of the Vrui application's navigation space"<<std::endl;
				std::cout<<"     to the given unit name and scale factor"<<std::endl;
				std::cout<<"  -loadView <viewpoint file name>"<<std::endl;
				std::cout<<"     Loads the initial viewing position from the given viewpoint file"<<std::endl;
				
				/* Remove parameter from argument list: */
				argc-=1;
				for(int j=i;j<argc;++j)
					argv[j]=argv[j+1];
				--i;
				}
			}
		
		/* Open the Vrui event pipe: */
		if(pipe(vruiEventPipe)!=0)
			{
			/* This is bad; need to shut down: */
			std::cerr<<"Error while opening event pipe"<<std::endl;
			vruiErrorShutdown(true);
			}
		
		/* Set both ends of the pipe to non-blocking I/O: */
		for(int i=0;i<2;++i)
			{
			long flags=fcntl(vruiEventPipe[i],F_GETFL);
			fcntl(vruiEventPipe[i],F_SETFL,flags|O_NONBLOCK);
			}
		
		/* Get the full name of the global per-user configuration file: */
		const char* userConfigFileName=0;
		
		#if VRUI_INTERNAL_CONFIG_HAVE_USERCONFIGFILE
		std::string userConfigFileNameString;
		
		const char* home=getenv("HOME");
		if(home!=0&&home[0]!='\0')
			{
			userConfigFileNameString=home;
			userConfigFileNameString.push_back('/');
			userConfigFileNameString.append(VRUI_INTERNAL_CONFIG_USERCONFIGFILE);
			
			userConfigFileName=userConfigFileNameString.c_str();
			}
		
		#endif
		
		/* Open the global and user configuration files: */
		vruiOpenConfigurationFile(userConfigFileName);
		
		/* Get the root section name: */
		const char* rootSectionName=getenv("VRUI_ROOTSECTION");
		if(rootSectionName==0||rootSectionName[0]=='\0')
			rootSectionName=getenv("HOSTNAME");
		if(rootSectionName==0||rootSectionName[0]=='\0')
			rootSectionName=getenv("HOST");
		
		/* Apply configuration-related arguments from the command line: */
		for(int i=1;i<argc;++i)
			if(argv[i][0]=='-')
				{
				if(strcasecmp(argv[i]+1,"mergeConfig")==0)
					{
					/* Next parameter is name of another configuration file to merge: */
					if(i+1<argc)
						{
						/* Assemble the full name of the configuration file to merge: */
						std::string configFileName=argv[i+1];
						
						/* Ensure that the configuration file name has the .cfg suffix: */
						if(!Misc::hasExtension(argv[i+1],".cfg"))
							configFileName.append(".cfg");
						
						/* Check if the configuration file name is a relative path: */
						if(argv[i+1][0]!='/')
							{
							/* Try loading the configuration file from the global systemwide configuration directory: */
							vruiMergeConfigurationFile(vruiCreateConfigurationFilePath(VRUI_INTERNAL_CONFIG_SYSCONFIGFILE,configFileName.c_str()).c_str());
							
							if(userConfigFileName!=0)
								{
								/* Try loading the configuration file from the global per-user configuration directory: */
								vruiMergeConfigurationFile(vruiCreateConfigurationFilePath(userConfigFileName,configFileName.c_str()).c_str());
								}
							}
						
						/* Try loading the configuration file as given: */
						vruiMergeConfigurationFile(configFileName.c_str());
						
						/* Remove parameters from argument list: */
						argc-=2;
						for(int j=i;j<argc;++j)
							argv[j]=argv[j+2];
						--i;
						}
					else
						{
						/* Ignore the mergeConfig parameter: */
						std::cerr<<"Vrui::init: No configuration file name given after -mergeConfig option"<<std::endl;
						--argc;
						}
					}
				else if(strcasecmp(argv[i]+1,"dumpConfig")==0)
					{
					/* Next parameter is name of configuration file to create: */
					if(i+1<argc)
						{
						/* Save the current configuration to the given configuration file: */
						if(vruiVerbose)
							std::cout<<"Vrui: Dumping current configuration space to configuration file "<<argv[i+1]<<"..."<<std::flush;
						vruiConfigFile->saveAs(argv[i+1]);
						if(vruiVerbose)
							std::cout<<" Ok"<<std::endl;
						
						/* Remove parameters from argument list: */
						argc-=2;
						for(int j=i;j<argc;++j)
							argv[j]=argv[j+2];
						--i;
						}
					else
						{
						/* Ignore the dumpConfig parameter: */
						std::cerr<<"Vrui::init: No configuration file name given after -dumpConfig option"<<std::endl;
						--argc;
						}
					}
				else if(strcasecmp(argv[i]+1,"rootSection")==0)
					{
					/* Next parameter is name of root section to use: */
					if(i+1<argc)
						{
						/* Save root section name: */
						rootSectionName=argv[i+1];
						
						/* Remove parameters from argument list: */
						argc-=2;
						for(int j=i;j<argc;++j)
							argv[j]=argv[j+2];
						--i;
						}
					else
						{
						/* Ignore the rootSection parameter: */
						std::cerr<<"Vrui::init: No root section name given after -rootSection option"<<std::endl;
						--argc;
						}
					}
				}
		
		/* Go to the configuration's root section: */
		vruiGoToRootSection(rootSectionName);
		
		/* Check if this is a multipipe environment: */
		if(vruiConfigFile->retrieveValue<bool>("./enableMultipipe",false))
			{
			typedef std::vector<std::string> StringList;
			
			try
				{
				if(vruiVerbose)
					std::cout<<"Vrui: Entering cluster mode"<<std::endl;
				
				/* Read multipipe settings from configuration file: */
				std::string master=vruiConfigFile->retrieveString("./multipipeMaster");
				int masterPort=vruiConfigFile->retrieveValue<int>("./multipipeMasterPort",0);
				StringList slaves=vruiConfigFile->retrieveValue<StringList>("./multipipeSlaves");
				vruiNumSlaves=slaves.size();
				std::string multicastGroup=vruiConfigFile->retrieveString("./multipipeMulticastGroup");
				int multicastPort=vruiConfigFile->retrieveValue<int>("./multipipeMulticastPort");
				unsigned int multicastSendBufferSize=vruiConfigFile->retrieveValue<unsigned int>("./multipipeSendBufferSize",16);
				
				/* Create the multicast multiplexer: */
				vruiMultiplexer=new Cluster::Multiplexer(vruiNumSlaves,0,master.c_str(),masterPort,multicastGroup.c_str(),multicastPort);
				vruiMultiplexer->setSendBufferSize(multicastSendBufferSize);
				
				/* Start the multipipe slaves on all slave nodes: */
				std::string multipipeRemoteCommand=vruiConfigFile->retrieveString("./multipipeRemoteCommand","ssh");
				masterPort=vruiMultiplexer->getLocalPortNumber();
				vruiSlavePids=new pid_t[vruiNumSlaves];
				std::string cwd=Misc::getCurrentDirectory();
				size_t rcLen=cwd.length()+strlen(argv[0])+master.length()+multicastGroup.length()+512;
				char* rc=new char[rcLen];
				if(vruiVerbose)
					std::cout<<"Vrui: Spawning slave processes..."<<std::flush;
				for(int i=0;i<vruiNumSlaves;++i)
					{
					if(vruiVerbose)
						std::cout<<' '<<slaves[i]<<std::flush;
					pid_t childPid=fork();
					if(childPid==0)
						{
						/* Create a command line to run the program from the current working directory: */
						int ai=0;
						ai+=snprintf(rc+ai,rcLen-ai,"cd '%s' ;",cwd.c_str());
						ai+=snprintf(rc+ai,rcLen-ai," %s",argv[0]);
						ai+=snprintf(rc+ai,rcLen-ai," -vruiMultipipeSlave");
						ai+=snprintf(rc+ai,rcLen-ai," %d %d",vruiNumSlaves,i+1);
						ai+=snprintf(rc+ai,rcLen-ai," %s %d",master.c_str(),masterPort);
						ai+=snprintf(rc+ai,rcLen-ai," %s %d",multicastGroup.c_str(),multicastPort);
						
						/* Create command line for the ssh (or other remote login) program: */
						char* sshArgv[20];
						int sshArgc=0;
						sshArgv[sshArgc++]=const_cast<char*>(multipipeRemoteCommand.c_str());
						sshArgv[sshArgc++]=const_cast<char*>(slaves[i].c_str());
						sshArgv[sshArgc++]=rc;
						sshArgv[sshArgc]=0;
						
						/* Run the remote login program: */
						execvp(sshArgv[0],sshArgv);
						}
					else
						{
						/* Store PID of ssh process for later: */
						vruiSlavePids[i]=childPid;
						}
					}
				if(vruiVerbose)
					std::cout<<" Ok"<<std::endl;
				
				/* Clean up: */
				delete[] rc;
				
				/* Wait until the entire cluster is connected: */
				if(vruiVerbose)
					std::cout<<"Vrui: Waiting for cluster to connect..."<<std::flush;
				vruiMultiplexer->waitForConnection();
				if(vruiVerbose)
					std::cout<<" Ok"<<std::endl;
				
				if(vruiVerbose)
					std::cout<<"Vrui: Distributing configuration and command line..."<<std::flush;
				
				/* Open a multicast pipe: */
				vruiPipe=new Cluster::MulticastPipe(vruiMultiplexer);
				
				/* Send the entire Vrui configuration file and the root section name across the pipe: */
				vruiConfigFile->writeToPipe(*vruiPipe);
				Misc::writeCString(rootSectionName,*vruiPipe);
				
				/* Write the application's command line: */
				vruiPipe->write<int>(argc);
				for(int i=0;i<argc;++i)
					Misc::writeCString(argv[i],*vruiPipe);
				
				/* Flush the pipe: */
				vruiPipe->flush();
				
				if(vruiVerbose)
					std::cout<<" Ok"<<std::endl;
				}
			catch(std::runtime_error error)
				{
				if(vruiVerbose)
					std::cout<<" error"<<std::endl;
				std::cerr<<"Master node: Caught exception "<<error.what()<<" while initializing cluster communication"<<std::endl;
				vruiErrorShutdown(true);
				}
			}
		}
	
	/* Synchronize threads between here and end of function body: */
	Cluster::ThreadSynchronizer threadSynchronizer(vruiPipe);
	
	/* Initialize Vrui state object: */
	try
		{
		if(vruiVerbose)
			std::cout<<"Vrui: Initializing Vrui environment..."<<std::flush;
		vruiState=new VruiState(vruiMultiplexer,vruiPipe);
		vruiState->initialize(vruiConfigFile->getCurrentSection());
		if(vruiVerbose)
			std::cout<<" Ok"<<std::endl;
		}
	catch(std::runtime_error error)
		{
		if(vruiVerbose)
			std::cout<<" error"<<std::endl;
		std::cerr<<"Caught exception "<<error.what()<<" while initializing Vrui state object"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Process additional command line arguments: */
	for(int i=1;i<argc;++i)
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"loadInputGraph")==0)
				{
				/* Next parameter is name of input graph file to load: */
				if(i+1<argc)
					{
					/* Save input graph file name: */
					vruiState->loadInputGraph=true;
					vruiState->inputGraphFileName=argv[i+1];
					
					/* Remove parameters from argument list: */
					argc-=2;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+2];
					--i;
					}
				else
					{
					/* Ignore the loadInputGraph parameter: */
					std::cerr<<"Vrui::init: No input graph file name given after -loadInputGraph option"<<std::endl;
					--argc;
					}
				}
			else if(strcasecmp(argv[i]+1,"addToolClass")==0)
				{
				/* Next parameter is name of tool class to load: */
				if(i+1<argc)
					{
					try
						{
						/* Load the tool class: */
						if(vruiVerbose)
							std::cout<<"Vrui: Adding requested tool class "<<argv[i+1]<<"..."<<std::flush;
						threadSynchronizer.sync();
						vruiState->toolManager->loadClass(argv[i+1]);
						if(vruiVerbose)
							std::cout<<" Ok"<<std::endl;
						}
					catch(std::runtime_error err)
						{
						/* Print a warning and carry on: */
						if(vruiVerbose)
							std::cout<<" error"<<std::endl;
						std::cerr<<"Vrui::init: Ignoring tool class "<<argv[i+1]<<" due to exception "<<err.what()<<std::endl;
						}
					
					/* Remove parameters from argument list: */
					argc-=2;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+2];
					--i;
					}
				else
					{
					/* Ignore the addToolClass parameter: */
					std::cerr<<"Vrui::init: No tool class name given after -addToolClass option"<<std::endl;
					--argc;
					}
				}
			else if(strcasecmp(argv[i]+1,"addTool")==0)
				{
				/* Next parameter is name of tool binding configuration file section: */
				if(i+1<argc)
					{
					try
						{
						/* Load the tool: */
						if(vruiVerbose)
							std::cout<<"Vrui: Adding requested tool from configuration section "<<argv[i+1]<<"..."<<std::flush;
						threadSynchronizer.sync();
						vruiState->toolManager->loadToolBinding(argv[i+1]);
						if(vruiVerbose)
							std::cout<<" Ok"<<std::endl;
						}
					catch(std::runtime_error err)
						{
						/* Print a warning and carry on: */
						if(vruiVerbose)
							std::cout<<" error"<<std::endl;
						std::cerr<<"Vrui::init: Ignoring tool binding "<<argv[i+1]<<" due to exception "<<err.what()<<std::endl;
						}
					
					/* Remove parameters from argument list: */
					argc-=2;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+2];
					--i;
					}
				else
					{
					/* Ignore the addTool parameter: */
					std::cerr<<"Vrui::init: No tool binding section name given after -addTool option"<<std::endl;
					--argc;
					}
				}
			else if(strcasecmp(argv[i]+1,"vislet")==0)
				{
				if(i+1<argc)
					{
					/* First parameter is name of vislet class: */
					const char* className=argv[i+1];
					
					/* Find semicolon terminating vislet parameter list: */
					int argEnd;
					for(argEnd=i+2;argEnd<argc&&(argv[argEnd][0]!=';'||argv[argEnd][1]!='\0');++argEnd)
						;
					
					if(vruiState->visletManager!=0)
						{
						try
							{
							/* Initialize the vislet: */
							if(vruiVerbose)
								std::cout<<"Vrui: Loading vislet of class "<<className<<"..."<<std::flush;
							threadSynchronizer.sync();
							VisletFactory* factory=vruiState->visletManager->loadClass(className);
							vruiState->visletManager->createVislet(factory,argEnd-(i+2),argv+(i+2));
							if(vruiVerbose)
								std::cout<<" Ok"<<std::endl;
							}
						catch(std::runtime_error err)
							{
							/* Print a warning and carry on: */
							if(vruiVerbose)
								std::cout<<" error"<<std::endl;
							std::cerr<<"Vrui::init: Ignoring vislet of type "<<className<<" due to exception "<<err.what()<<std::endl;
							}
						}
					
					/* Remove all vislet parameters from the command line: */
					if(argEnd<argc)
						++argEnd;
					int numArgs=argEnd-i;
					argc-=numArgs;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+numArgs];
					--i;
					}
				else
					{
					/* Ignore the vislet parameter: */
					std::cerr<<"Vrui::init: No vislet class name given after -vislet option"<<std::endl;
					argc=i;
					}
				}
			else if(strcasecmp(argv[i]+1,"loadView")==0)
				{
				/* Next parameter is name of viewpoint file to load: */
				if(i+1<argc)
					{
					/* Save viewpoint file name: */
					vruiState->viewpointFileName=argv[i+1];
					
					/* Remove parameters from argument list: */
					argc-=2;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+2];
					--i;
					}
				else
					{
					/* Ignore the loadView parameter: */
					std::cerr<<"Vrui::init: No viewpoint file name given after -loadView option"<<std::endl;
					--argc;
					}
				}
			else if(strcasecmp(argv[i]+1,"setLinearUnit")==0)
				{
				/* Next two parameters are unit name and scale factor: */
				if(i+2<argc)
					{
					/* Set the linear unit: */
					getCoordinateManager()->setUnit(Geometry::LinearUnit(argv[i+1],atof(argv[i+2])));
					
					/* Remove parameters from argument list: */
					argc-=3;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+3];
					--i;
					}
				else
					{
					/* Ignore the setLinearUnit parameter: */
					std::cerr<<"Vrui::init: No unit name and scale factor given after -setLinearUnit option"<<std::endl;
					--argc;
					}
				}
			}
	
	if(vruiVerbose&&vruiState->master)
		{
		std::cout<<"Vrui: Command line passed to application:";
		for(int i=1;i<argc;++i)
			std::cout<<" \""<<argv[i]<<'"';
		std::cout<<std::endl;
		}
	
	/* Extract the application name: */
	const char* appNameStart=argv[0];
	const char* cPtr;
	for(cPtr=appNameStart;*cPtr!='\0';++cPtr)
		if(*cPtr=='/')
			appNameStart=cPtr+1;
	vruiApplicationName=new char[cPtr-appNameStart+1];
	memcpy(vruiApplicationName,appNameStart,cPtr-appNameStart);
	vruiApplicationName[cPtr-appNameStart]='\0';
	}

void startDisplay(void)
	{
	/* Synchronize threads between here and end of function body: */
	Cluster::ThreadSynchronizer threadSynchronizer(vruiState->pipe);
	
	/* Wait for all nodes in the multicast group to reach this point: */
	if(vruiState->multiplexer!=0)
		{
		if(vruiVerbose&&vruiState->master)
			std::cout<<"Vrui: Waiting for cluster before graphics initialization..."<<std::flush;
		vruiState->pipe->barrier();
		if(vruiVerbose&&vruiState->master)
			std::cout<<" Ok"<<std::endl;
		}
	
	if(vruiVerbose&&vruiState->master)
		std::cout<<"Vrui: Starting graphics subsystem..."<<std::endl;
	
	/* Find the mouse adapter listed in the input device manager (if there is one): */
	InputDeviceAdapterMouse* mouseAdapter=0;
	for(int i=0;i<vruiState->inputDeviceManager->getNumInputDeviceAdapters()&&mouseAdapter==0;++i)
		mouseAdapter=dynamic_cast<InputDeviceAdapterMouse*>(vruiState->inputDeviceManager->getInputDeviceAdapter(i));
	
	try
		{
		/* Retrieve the list of VR windows: */
		typedef std::vector<std::string> StringList;
		StringList windowNames;
		if(vruiState->multiplexer!=0)
			{
			char windowNamesTag[40];
			snprintf(windowNamesTag,sizeof(windowNamesTag),"./node%dWindowNames",vruiState->multiplexer->getNodeIndex());
			windowNames=vruiConfigFile->retrieveValue<StringList>(windowNamesTag);
			}
		else
			windowNames=vruiConfigFile->retrieveValue<StringList>("./windowNames");
		
		/* Ready the GLObject manager to initialize its objects per-window: */
		GLContextData::resetThingManager();
		
		/* Initialize the window list: */
		vruiNumWindows=windowNames.size();
		vruiWindows=new VRWindow*[vruiNumWindows];
		for(int i=0;i<vruiNumWindows;++i)
			vruiWindows[i]=0;
		
		/* Sort the windows into groups based on their group IDs: */
		typedef Misc::HashTable<unsigned int,VruiWindowGroupCreator> WindowGroupMap;
		WindowGroupMap windowGroups(7);
		typedef Misc::HashTable<std::string,unsigned int> DisplayGroupMap;
		DisplayGroupMap displayGroups(7);
		const char* defaultDisplayName=getenv("DISPLAY");
		if(defaultDisplayName==0)
			defaultDisplayName="";
		unsigned int nextGroupId=0;
		for(int windowIndex=0;windowIndex<vruiNumWindows;++windowIndex)
			{
			/* Go to the window's configuration section: */
			Misc::ConfigurationFileSection windowSection=vruiConfigFile->getSection(windowNames[windowIndex].c_str());
			
			/* Read the window's display string: */
			std::string displayName=windowSection.retrieveString("./display",defaultDisplayName);
			
			/* Create a default group ID for the window: */
			DisplayGroupMap::Iterator dgIt=displayGroups.findEntry(displayName);
			unsigned int groupId;
			if(dgIt.isFinished())
				{
				/* Start a new window group: */
				groupId=nextGroupId;
				}
			else
				{
				/* Use the ID of the group on the same display: */
				groupId=dgIt->getDest();
				}
			
			/* Read the window's group ID: */
			groupId=windowSection.retrieveValue<unsigned int>("./groupId",groupId);
			
			/* Look for the group ID in the window groups hash table: */
			WindowGroupMap::Iterator wgIt=windowGroups.findEntry(groupId);
			if(wgIt.isFinished())
				{
				/* Start a new window group: */
				VruiWindowGroupCreator newGroup;
				VruiWindowGroupCreator::VruiWindow newWindow;
				newWindow.windowIndex=windowIndex;
				newWindow.windowConfigFileSection=windowSection;
				newGroup.windows.push_back(newWindow);
				newGroup.mouseAdapter=mouseAdapter;
				windowGroups[groupId]=newGroup;
				
				/* Associate the new group with the display name: */
				displayGroups[displayName]=groupId;
				if(nextGroupId<=groupId)
					nextGroupId=groupId+1;
				}
			else
				{
				/* Add the window to the existing window group: */
				VruiWindowGroupCreator::VruiWindow newWindow;
				newWindow.windowIndex=windowIndex;
				newWindow.windowConfigFileSection=windowSection;
				wgIt->getDest().windows.push_back(newWindow);
				}
			}
		
		/* Check if there are multiple window groups, so multiple threads can be used: */
		vruiNumWindowGroups=int(windowGroups.getNumEntries());
		bool allWindowsOk=true;
		if(vruiNumWindowGroups>1)
			{
			#if GLSUPPORT_CONFIG_USE_TLS
			
			/* Initialize the rendering barrier: */
			vruiRenderingBarrier.setNumSynchronizingThreads(vruiNumWindowGroups+1);
			
			/* Create one rendering thread for each window group (which will in turn create the windows in their respective groups themselves): */
			vruiRenderingThreads=new Threads::Thread[vruiNumWindowGroups];
			{
			int i=0;
			for(WindowGroupMap::Iterator wgIt=windowGroups.begin();!wgIt.isFinished();++wgIt,++i)
				vruiRenderingThreads[i].start(vruiRenderingThreadFunction,wgIt->getDest());
			}
			
			/* Wait until all threads have created their windows: */
			vruiRenderingBarrier.synchronize();
			
			/* Check if all windows have been properly created: */
			allWindowsOk=true;
			for(int i=0;i<vruiNumWindows;++i)
				if(vruiWindows[i]==0)
					allWindowsOk=false;
			
			#else
			
			/* Create all windows in all window groups: */
			for(WindowGroupMap::Iterator wgIt=windowGroups.begin();allWindowsOk&&!wgIt.isFinished();++wgIt)
				allWindowsOk=vruiCreateWindowGroup(wgIt->getDest());
			
			#endif
			}
		else if(vruiNumWindowGroups==1)
			{
			/* Create all windows in the only window group: */
			allWindowsOk=vruiCreateWindowGroup(windowGroups.begin()->getDest());
			}
		
		if(vruiVerbose)
			{
			if(vruiState->multiplexer!=0)
				std::cout<<"Vrui (node "<<vruiState->multiplexer->getNodeIndex()<<"): ";
			else
				std::cout<<"Vrui: ";
			std::cout<<"Opened "<<vruiNumWindows<<(vruiNumWindows>1?" windows":" window");
			if(vruiNumWindowGroups>1)
				{
				std::cout<<" in "<<vruiNumWindowGroups<<" window groups";
				#if GLSUPPORT_CONFIG_USE_TLS
				std::cout<<" (rendering in parallel)";
				#else
				std::cout<<" (rendering serially)";
				#endif
				}
			std::cout<<std::endl;
			if(vruiState->master)
				std::cout<<"Vrui: Graphics subsystem "<<(allWindowsOk?" Ok":" failed")<<std::endl;
			}
		if(!allWindowsOk)
			Misc::throwStdErr("Vrui::startDisplay: Could not create all rendering windows");
		
		/* Initialize the window groups array: */
		vruiWindowGroups=new VruiWindowGroup[vruiNumWindowGroups];
		{
		int i=0;
		for(WindowGroupMap::Iterator wgIt=windowGroups.begin();!wgIt.isFinished();++wgIt,++i)
			{
			vruiWindowGroups[i].display=vruiWindows[wgIt->getDest().windows.front().windowIndex]->getContext().getDisplay();
			vruiWindowGroups[i].displayFd=ConnectionNumber(vruiWindowGroups[i].display);
			vruiWindowGroups[i].maxViewportSize[0]=vruiWindowGroups[i].maxViewportSize[1]=0;
			vruiWindowGroups[i].maxFrameSize[0]=vruiWindowGroups[i].maxFrameSize[1]=0;
			for(std::vector<VruiWindowGroupCreator::VruiWindow>::iterator wIt=wgIt->getDest().windows.begin();wIt!=wgIt->getDest().windows.end();++wIt)
				{
				VruiWindowGroup::Window newWindow;
				newWindow.window=vruiWindows[wIt->windowIndex];
				newWindow.viewportSize[0]=newWindow.viewportSize[1]=0;
				newWindow.frameSize[0]=newWindow.frameSize[1]=0;
				vruiWindowGroups[i].windows.push_back(newWindow);
				newWindow.window->setWindowGroup(&vruiWindowGroups[i]);
				}
			}
		}
		}
	catch(std::runtime_error error)
		{
		std::cerr<<"Caught exception "<<error.what()<<" while initializing rendering windows"<<std::endl;
		vruiErrorShutdown(true);
		}
	catch(...)
		{
		std::cerr<<"Caught spurious exception while initializing rendering windows"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Create the total list of all windows on the cluster: */
	vruiTotalNumWindows=0;
	int localWindowsStart=0;
	if(vruiMultiplexer!=0)
		{
		/* Count the number of windows on all cluster nodes: */
		for(unsigned int nodeIndex=0;nodeIndex<vruiMultiplexer->getNumNodes();++nodeIndex)
			{
			if(nodeIndex==vruiMultiplexer->getNodeIndex())
				localWindowsStart=vruiTotalNumWindows;
			char windowNamesTag[40];
			snprintf(windowNamesTag,sizeof(windowNamesTag),"./node%uWindowNames",nodeIndex);
			typedef std::vector<std::string> StringList;
			StringList windowNames=vruiConfigFile->retrieveValue<StringList>(windowNamesTag);
			vruiTotalNumWindows+=int(windowNames.size());
			}
		}
	else
		{
		/* On a single-machine environment, total windows are local windows: */
		vruiTotalNumWindows=vruiNumWindows;
		localWindowsStart=0;
		}
	vruiTotalWindows=new VRWindow*[vruiTotalNumWindows];
	for(int i=0;i<localWindowsStart;++i)
		vruiTotalWindows[i]=0;
	for(int i=0;i<vruiNumWindows;++i)
		vruiTotalWindows[localWindowsStart+i]=vruiWindows[i];
	for(int i=localWindowsStart+vruiNumWindows;i<vruiTotalNumWindows;++i)
		vruiTotalWindows[i]=0;
	}

void startSound(void)
	{
	/* Synchronize threads between here and end of function body: */
	Cluster::ThreadSynchronizer threadSynchronizer(vruiState->pipe);
	
	/* Wait for all nodes in the multicast group to reach this point: */
	if(vruiState->multiplexer!=0)
		{
		if(vruiVerbose&&vruiState->master)
			std::cout<<"Vrui: Waiting for cluster before sound initialization..."<<std::flush;
		vruiState->pipe->barrier();
		if(vruiVerbose&&vruiState->master)
			std::cout<<" Ok"<<std::endl;
		}
	else if(vruiVerbose)
		std::cout<<"Vrui: Starting sound subsystem"<<std::endl;
	
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Retrieve the name of the sound context: */
	std::string soundContextName;
	if(vruiState->multiplexer!=0)
		{
		char soundContextNameTag[40];
		snprintf(soundContextNameTag,sizeof(soundContextNameTag),"./node%dSoundContextName",vruiState->multiplexer->getNodeIndex());
		soundContextName=vruiConfigFile->retrieveValue<std::string>(soundContextNameTag,"");
		}
	else
		soundContextName=vruiConfigFile->retrieveValue<std::string>("./soundContextName","");
	if(soundContextName=="")
		return;
	
	/* Ready the ALObject manager to initialize its objects per-context: */
	ALContextData::resetThingManager();
	
	try
		{
		/* Create a new sound context: */
		SoundContext* sc=new SoundContext(vruiConfigFile->getSection(soundContextName.c_str()),vruiState);
		
		/* Install the Vrui sound context: */
		vruiNumSoundContexts=1;
		vruiSoundContexts=new SoundContext*[1];
		vruiSoundContexts[0]=sc;
		
		/* Initialize all ALObjects for this sound context's context data: */
		vruiSoundContexts[0]->makeCurrent();
		vruiSoundContexts[0]->getContextData().updateThings();
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Disabling OpenAL sound due to exception "<<err.what()<<std::endl;
		if(vruiSoundContexts[0]!=0)
			{
			delete vruiSoundContexts[0];
			vruiSoundContexts[0]=0;
			}
		}
	catch(...)
		{
		std::cerr<<"Disabling OpenAL sound due to spurious exception"<<std::endl;
		if(vruiSoundContexts[0]!=0)
			{
			delete vruiSoundContexts[0];
			vruiSoundContexts[0]=0;
			}
		}
	#endif
	}

bool vruiHandleAllEvents(bool allowBlocking,bool checkStdin)
	{
	bool handledEvents=false;
	
	/* Check if there are pending events on the event pipe or any windows' X event queues: */
	Misc::FdSet readFds;
	bool mustBlock=allowBlocking;
	#ifdef DEBUG
	{
	Threads::Mutex::Lock vruiEventPipeLock(vruiEventPipeMutex);
	#endif
	if(vruiNumSignaledEvents>0&&vruiEventPipe[0]>=0)
		{
		readFds.add(vruiEventPipe[0]);
		mustBlock=false;
		}
	#ifdef DEBUG
	}
	#endif
	for(int i=0;i<vruiNumWindowGroups;++i)
		if(XPending(vruiWindowGroups[i].display))
			{
			readFds.add(vruiWindowGroups[i].displayFd);
			mustBlock=false;
			}
	
	/* If there are no pending events, and blocking is allowed, block until something happens: */
	if(mustBlock)
		{
		/* Fill the file descriptor set to wait for events: */
		if(checkStdin)
			readFds.add(fileno(stdin)); // Return on input on stdin, as well
		if(vruiEventPipe[0]>=0)
			readFds.add(vruiEventPipe[0]);
		for(int i=0;i<vruiNumWindowGroups;++i)
			readFds.add(vruiWindowGroups[i].displayFd);
		
		/* Block until any events arrive: */
		if(vruiState->nextFrameTime!=0.0||vruiState->timerEventScheduler->hasPendingEvents())
			{
			/* Calculate the time interval until the next scheduled event: */
			double nextFrameTime=Math::Constants<double>::max;
			if(vruiState->timerEventScheduler->hasPendingEvents())
				nextFrameTime=vruiState->timerEventScheduler->getNextEventTime();
			if(vruiState->nextFrameTime!=0.0&&nextFrameTime>vruiState->nextFrameTime)
				nextFrameTime=vruiState->nextFrameTime;
			double dtimeout=nextFrameTime-vruiState->appTime.peekTime();
			struct timeval timeout;
			if(dtimeout>0.0)
				{
				timeout.tv_sec=long(Math::floor(dtimeout));
				timeout.tv_usec=long(Math::floor((dtimeout-double(timeout.tv_sec))*1000000.0+0.5));
				}
			else
				{
				timeout.tv_sec=0;
				timeout.tv_usec=0;
				}
			
			/* Block until the next scheduled timer event comes due: */
			if(Misc::select(&readFds,0,0,&timeout)==0)
				handledEvents=true; // Must stop waiting if a timer event is due
			}
		else
			{
			/* Block until kingdom come: */
			Misc::select(&readFds,0,0);
			}
		}
	
	/* Process any pending X events: */
	for(int i=0;i<vruiNumWindowGroups;++i)
		if(readFds.isSet(vruiWindowGroups[i].displayFd))
			{
			/* Process all pending events for this display connection: */
			bool isKeyRepeat=false; // Flag if the next event is a key repeat event
			do
				{
				/* Get the next event: */
				XEvent event;
				XNextEvent(vruiWindowGroups[i].display,&event);
				
				/* Check for key repeat events (a KeyRelease immediately followed by a KeyPress with the same time stamp): */
				if(event.type==KeyRelease&&XPending(vruiWindowGroups[i].display))
					{
					/* Check if the next event is a KeyPress with the same time stamp: */
					XEvent nextEvent;
					XPeekEvent(vruiWindowGroups[i].display,&nextEvent);
					if(nextEvent.type==KeyPress&&nextEvent.xkey.window==event.xkey.window&&nextEvent.xkey.time==event.xkey.time&&nextEvent.xkey.keycode==event.xkey.keycode)
						{
						/* Mark the next event as a key repeat: */
						isKeyRepeat=true;
						continue;
						}
					}
				
				/* Pass it to all windows interested in it: */
				bool finishProcessing=false;
				for(std::vector<VruiWindowGroup::Window>::iterator wIt=vruiWindowGroups[i].windows.begin();wIt!=vruiWindowGroups[i].windows.end();++wIt)
					if(wIt->window->isEventForWindow(event))
						finishProcessing=wIt->window->processEvent(event)||finishProcessing;
				handledEvents=!isKeyRepeat||finishProcessing;
				isKeyRepeat=false;
				
				/* Stop processing events if something significant happened: */
				if(finishProcessing)
					goto doneWithEvents;
				}
			while(XPending(vruiWindowGroups[i].display));
			}
	doneWithEvents:
	
	/* Read pending data from stdin and exit if escape key is pressed: */
	if(checkStdin)
		{
		if(!mustBlock)
			{
			/* Check for pending key presses real quick: */
			struct timeval timeout;
			timeout.tv_sec=0;
			timeout.tv_usec=0;
			readFds.add(fileno(stdin));
			Misc::select(&readFds,0,0,&timeout);
			}
		if(readFds.isSet(fileno(stdin)))
			{
			/* Read the next character from stdin and check if it's ESC: */
			char in='\0';
			if(read(fileno(stdin),&in,sizeof(char))>0)
				{
				if(in==27)
					{
					/* Call the quit callback: */
					Misc::CallbackData cbData;
					vruiState->quitCallback(&cbData);
					}
				handledEvents=true;
				}
			}
		}
	
	if(vruiEventPipe[0]>=0)
		{
		/* Flush the event pipe no matter what: */
		Threads::Mutex::Lock eventPipeLock(vruiEventPipeMutex);
		
		/* Read accumulated bytes from the event pipe (it's nonblocking): */
		char readBuffer[16]; // More than enough; there should only be one byte in the pipe
		if(read(vruiEventPipe[0],readBuffer,sizeof(readBuffer))>0)
			handledEvents=true;
		
		/* Reset the number of accumulated events: */
		vruiNumSignaledEvents=0;
		}
	
	return handledEvents;
	}

void vruiInnerLoopMultiWindow(void)
	{
	bool keepRunning=true;
	bool firstFrame=true;
	while(keepRunning)
		{
		/* Handle all events, blocking if there are none unless in continuous mode: */
		if(firstFrame||vruiState->updateContinuously)
			{
			/* Check for and handle events without blocking: */
			vruiHandleAllEvents(false,vruiNumWindows==0&&vruiState->master);
			}
		else
			{
			/* Wait for and process events until something actually happens: */
			while(!vruiHandleAllEvents(true,vruiNumWindows==0&&vruiState->master))
				;
			}
		
		/* Check for asynchronous shutdown: */
		keepRunning=keepRunning&&!vruiAsynchronousShutdown;
		
		/* Run a single Vrui frame: */
		if(vruiState->multiplexer!=0)
			vruiState->pipe->broadcast(keepRunning);
		if(!keepRunning)
			{
			if(vruiState->multiplexer!=0&&vruiState->master)
				vruiState->pipe->flush();
			
			/* Bail out of the inner loop: */
			break;
			}
		
		/* Update the Vrui state: */
		vruiState->update();
		
		/* Reset the AL thing manager: */
		ALContextData::resetThingManager();
		
		#if ALSUPPORT_CONFIG_HAVE_OPENAL
		/* Update all sound contexts: */
		for(int i=0;i<vruiNumSoundContexts;++i)
			vruiSoundContexts[i]->draw();
		#endif
		
		/* Reset the GL thing manager: */
		GLContextData::resetThingManager();
		
		if(vruiNumWindowGroups>1)
			{
			#if GLSUPPORT_CONFIG_USE_TLS
			
			/* Start the rendering cycle by synchronizing with the render threads: */
			vruiRenderingBarrier.synchronize();
			
			/* Wait until all threads are done rendering: */
			vruiRenderingBarrier.synchronize();
			
			if(vruiState->multiplexer!=0)
				{
				/* Synchronize with other nodes: */
				vruiState->pipe->barrier();
				
				/* Notify the render threads to swap buffers: */
				vruiRenderingBarrier.synchronize();
				}
			
			/* Wait until all threads are done swapping buffers: */
			vruiRenderingBarrier.synchronize();
			
			#else
			
			/* Render to all window groups in turn: */
			for(int i=0;i<vruiNumWindowGroups;++i)
				{
				for(std::vector<VruiWindowGroup::Window>::iterator wgIt=vruiWindowGroups[i].windows.begin();wgIt!=vruiWindowGroups[i].windows.end();++wgIt)
					wgIt->window->draw();
				}
			
			if(vruiState->multiplexer!=0)
				{
				/* Synchronize with other nodes: */
				glFinish();
				vruiState->pipe->barrier();
				}
			
			/* Swap all buffers at once: */
			for(int i=0;i<vruiNumWindowGroups;++i)
				{
				for(std::vector<VruiWindowGroup::Window>::iterator wgIt=vruiWindowGroups[i].windows.begin();wgIt!=vruiWindowGroups[i].windows.end();++wgIt)
					{
					wgIt->window->makeCurrent();
					wgIt->window->swapBuffers();
					}
				}
			
			#endif
			}
		else if(vruiNumWindows>0)
			{
			/* Update rendering: */
			for(int i=0;i<vruiNumWindows;++i)
				vruiWindows[i]->draw();
			
			if(vruiState->multiplexer!=0)
				{
				/* Synchronize with other nodes: */
				glFinish();
				vruiState->pipe->barrier();
				}
			
			/* Swap all buffers at once: */
			for(int i=0;i<vruiNumWindows;++i)
				{
				vruiWindows[i]->makeCurrent();
				vruiWindows[i]->swapBuffers();
				}
			}
		else if(vruiState->multiplexer!=0)
			{
			/* Synchronize with other nodes: */
			vruiState->pipe->barrier();
			}
		
		/* Print current frame rate on head node's console for window-less Vrui processes: */
		if(vruiNumWindows==0&&vruiState->master)
			{
			printf("Current frame rate: %8.3f fps\r",1.0/vruiState->currentFrameTime);
			fflush(stdout);
			}
		
		firstFrame=false;
		}
	if(vruiNumWindows==0&&vruiState->master)
		{
		printf("\n");
		fflush(stdout);
		}
	}

// DEBUGGING
Realtime::TimePointMonotonic vruiLoopTimer;
const Realtime::TimeVector vruiLoopDelay(0,11*1000000L);

void vruiInnerLoopSingleWindow(void)
	{
	bool keepRunning=true;
	bool firstFrame=true;
	while(true)
		{
		// DEBUGGING
		//Realtime::TimeVector loopTime=vruiLoopTimer.setAndDiff();
		//std::cout<<"Loop time: "<<double(loopTime)*1000.0<<" ms"<<std::endl;
		
		// TESTING
		//vruiLoopTimer.sleepFromCurrent(vruiLoopDelay);
		
		/* Handle all events, blocking if there are none unless in continuous mode: */
		if(firstFrame||vruiState->updateContinuously)
			{
			/* Check for and handle events without blocking: */
			vruiHandleAllEvents(false,false);
			}
		else
			{
			/* Wait for and process events until something actually happens: */
			while(!vruiHandleAllEvents(true,false))
				;
			}
		
		/* Check for asynchronous shutdown: */
		keepRunning=keepRunning&&!vruiAsynchronousShutdown;
		
		/* Run a single Vrui frame: */
		if(vruiState->multiplexer!=0)
			vruiState->pipe->broadcast(keepRunning);
		if(!keepRunning)
			{
			if(vruiState->multiplexer!=0&&vruiState->master)
				vruiState->pipe->flush();
			
			/* Bail out of the inner loop: */
			break;
			}
		
		/* Update the Vrui state: */
		vruiState->update();
		
		/* Reset the AL thing manager: */
		ALContextData::resetThingManager();
		
		#if ALSUPPORT_CONFIG_HAVE_OPENAL
		/* Update all sound contexts: */
		for(int i=0;i<vruiNumSoundContexts;++i)
			vruiSoundContexts[i]->draw();
		#endif
		
		/* Reset the GL thing manager: */
		GLContextData::resetThingManager();
		
		/* Update rendering: */
		vruiWindows[0]->draw();
		
		if(vruiState->multiplexer!=0)
			{
			/* Synchronize with other nodes: */
			glFinish();
			vruiState->pipe->barrier();
			}
		
		/* Swap buffer: */
		vruiWindows[0]->swapBuffers();
		
		firstFrame=false;
		}
	}

void mainLoop(void)
	{
	/* Bail out if someone requested a shutdown during the initialization procedure: */
	if(vruiAsynchronousShutdown)
		{
		if(vruiVerbose&&vruiState->master)
			std::cout<<"Vrui: Shutting down due to shutdown request during initialization"<<std::flush;
		return;
		}
	
	/* Start the display subsystem: */
	startDisplay();
	
	if(vruiState->useSound)
		{
		/* Start the sound subsystem: */
		startSound();
		}
	
	/* Wait for all nodes in the multicast group to reach this point: */
	if(vruiState->multiplexer!=0)
		{
		if(vruiVerbose&&vruiState->master)
			std::cout<<"Vrui: Waiting for cluster before preparing main loop..."<<std::flush;
		vruiState->pipe->barrier();
		if(vruiVerbose&&vruiState->master)
			std::cout<<" Ok"<<std::endl;
		}
	
	/* Prepare Vrui state for main loop: */
	if(vruiVerbose&&vruiState->master)
		std::cout<<"Vrui: Preparing main loop..."<<std::flush;
	vruiState->prepareMainLoop();
	
	#if 0
	/* Turn off the screen saver: */
	int screenSaverTimeout,screenSaverInterval;
	int screenSaverPreferBlanking,screenSaverAllowExposures;
	XGetScreenSaver(vruiWindow->getDisplay(),&screenSaverTimeout,&screenSaverInterval,&screenSaverPreferBlanking,&screenSaverAllowExposures);
	XSetScreenSaver(vruiWindow->getDisplay(),0,0,DefaultBlanking,DefaultExposures);
	XResetScreenSaver(vruiWindow->getDisplay());
	#endif
	
	if(vruiState->master&&vruiNumWindows==0)
		{
		/* Disable line buffering on stdin to detect key presses in the inner loop: */
		termios term;
		tcgetattr(fileno(stdin),&term);
		term.c_lflag&=~ICANON;
		tcsetattr(fileno(stdin),TCSANOW,&term);
		setbuf(stdin,0);
		
		printf("Press Esc to exit...\n");
		}
	
	if(vruiVerbose&&vruiState->master)
		std::cout<<" Ok"<<std::endl;
	
	/* Perform the main loop until the ESC key is hit: */
	if(vruiVerbose&&vruiState->master)
		std::cout<<"Vrui: Entering main loop"<<std::endl;
	if(vruiNumWindows!=1)
		vruiInnerLoopMultiWindow();
	else
		vruiInnerLoopSingleWindow();
	
	/* Perform first clean-up steps: */
	if(vruiVerbose&&vruiState->master)
		std::cout<<"Vrui: Exiting main loop..."<<std::flush;
	vruiState->finishMainLoop();
	if(vruiVerbose&&vruiState->master)
		std::cout<<" Ok"<<std::endl;
	
	/* Shut down the rendering system: */
	if(vruiVerbose&&vruiState->master)
		std::cout<<"Vrui: Shutting down graphics subsystem..."<<std::flush;
	GLContextData::shutdownThingManager();
	#if GLSUPPORT_CONFIG_USE_TLS
	if(vruiRenderingThreads!=0)
		{
		/* Shut down all rendering threads: */
		vruiStopRenderingThreads=true;
		vruiRenderingBarrier.synchronize();
		for(int i=0;i<vruiNumWindowGroups;++i)
			{
			// vruiRenderingThreads[i].cancel();
			vruiRenderingThreads[i].join();
			}
		delete[] vruiRenderingThreads;
		vruiRenderingThreads=0;
		}
	#endif
	if(vruiWindows!=0)
		{
		/* Release all OpenGL state: */
		for(int i=0;i<vruiNumWindowGroups;++i)
			{
			for(std::vector<VruiWindowGroup::Window>::iterator wgIt=vruiWindowGroups[i].windows.begin();wgIt!=vruiWindowGroups[i].windows.end();++wgIt)
				wgIt->window->deinit();
			vruiWindowGroups[i].windows.front().window->getContext().deinit();
			}
		
		/* Delete all windows: */
		for(int i=0;i<vruiNumWindows;++i)
			delete vruiWindows[i];
		delete[] vruiWindows;
		vruiWindows=0;
		delete[] vruiWindowGroups;
		vruiWindowGroups=0;
		delete[] vruiTotalWindows;
		vruiTotalWindows=0;
		}
	if(vruiVerbose&&vruiState->master)
		std::cout<<" Ok"<<std::endl;
	
	/* Shut down the sound system: */
	if(vruiVerbose&&vruiState->master&&vruiSoundContexts!=0)
		std::cout<<"Vrui: Shutting down sound subsystem..."<<std::flush;
	ALContextData::shutdownThingManager();
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	if(vruiSoundContexts!=0)
		{
		/* Destroy all sound contexts: */
		for(int i=0;i<vruiNumSoundContexts;++i)
			delete vruiSoundContexts[i];
		delete[] vruiSoundContexts;
		}
	#endif
	if(vruiVerbose&&vruiState->master&&vruiSoundContexts!=0)
		std::cout<<" Ok"<<std::endl;
	
	#if 0
	/* Turn the screen saver back on: */
	XSetScreenSaver(vruiWindow->getDisplay(),screenSaverTimeout,screenSaverInterval,screenSaverPreferBlanking,screenSaverAllowExposures);
	#endif
	}

void deinit(void)
	{
	/* Clean up: */
	if(vruiVerbose&&vruiState->master)
		std::cout<<"Vrui: Shutting down Vrui environment"<<std::endl;
	delete[] vruiApplicationName;
	delete vruiState;
	
	if(vruiMultiplexer!=0)
		{
		bool master=vruiMultiplexer->isMaster();
		if(vruiVerbose&&master)
			std::cout<<"Vrui: Exiting cluster mode"<<std::endl;
		
		/* Destroy the multiplexer: */
		if(vruiVerbose&&master)
			std::cout<<"Vrui: Shutting down intra-cluster communication..."<<std::flush;
		delete vruiPipe;
		delete vruiMultiplexer;
		if(vruiVerbose&&master)
			std::cout<<" Ok"<<std::endl;
		
		if(master&&vruiSlavePids!=0)
			{
			/* Wait for all slaves to terminate: */
			if(vruiVerbose)
				std::cout<<"Vrui: Waiting for slave processes to terminate..."<<std::flush;
			for(int i=0;i<vruiNumSlaves;++i)
				waitpid(vruiSlavePids[i],0,0);
			delete[] vruiSlavePids;
			vruiSlavePids=0;
			if(vruiVerbose)
				std::cout<<" Ok"<<std::endl;
			}
		if(!master&&vruiSlaveArgv!=0)
			{
			/* Delete the slave command line: */
			for(int i=0;i<vruiSlaveArgc;++i)
				delete[] vruiSlaveArgv[i];
			delete[] vruiSlaveArgv;
			vruiSlaveArgv=0;
			delete[] vruiSlaveArgvShadow;
			vruiSlaveArgvShadow=0;
			}
		}
	
	/* Close the configuration file: */
	delete vruiConfigFile;
	
	if(vruiEventPipe[0]>=0)
		{
		/* Close the vrui event pipe: */
		close(vruiEventPipe[0]);
		close(vruiEventPipe[1]);
		}
	}

void shutdown(void)
	{
	/* Signal asynchronous shutdown if this node is the master node: */
	if(vruiState->master)
		{
		vruiAsynchronousShutdown=true;
		requestUpdate();
		}
	}

int getNumWindows(void)
	{
	return vruiTotalNumWindows;
	}

VRWindow* getWindow(int index)
	{
	return vruiTotalWindows[index];
	}

int getNumSoundContexts(void)
	{
	return vruiNumSoundContexts;
	}

SoundContext* getSoundContext(int index)
	{
	return vruiSoundContexts[index];
	}

ViewSpecification calcViewSpec(int windowIndex,int eyeIndex)
	{
	/* Return bogus view specification if the window is non-local: */
	if(vruiTotalWindows[windowIndex]==0)
		return ViewSpecification();
	
	/* Get the view specification in physical coordinates: */
	ViewSpecification viewSpec=vruiTotalWindows[windowIndex]->calcViewSpec(eyeIndex);
	
	if(vruiState->navigationTransformationEnabled)
		{
		/* Transform the view specification to navigation coordinates: */
		ATransform invNav=vruiState->inverseNavigationTransformation;
		Scalar invNavScale=vruiState->inverseNavigationTransformation.getScaling();
		Plane newScreenPlane=viewSpec.getScreenPlane();
		newScreenPlane.transform(invNav);
		newScreenPlane.normalize();
		viewSpec.setScreenPlane(newScreenPlane);
		Scalar newScreenSize[2];
		for(int i=0;i<2;++i)
			{
			newScreenSize[i]=viewSpec.getScreenSize(i);
			newScreenSize[i]*=invNavScale;
			}
		viewSpec.setScreenSize(newScreenSize);
		viewSpec.setEye(invNav.transform(viewSpec.getEye()));
		viewSpec.setEyeScreenDistance(viewSpec.getEyeScreenDistance()*invNavScale);
		for(int i=0;i<8;++i)
			viewSpec.setFrustumVertex(i,invNav.transform(viewSpec.getFrustumVertex(i)));
		for(int i=0;i<6;++i)
			{
			Plane newFrustumPlane=viewSpec.getFrustumPlane(i);
			newFrustumPlane.transform(invNav);
			newFrustumPlane.normalize();
			viewSpec.setFrustumPlane(i,newFrustumPlane);
			}
		}
	
	return viewSpec;
	}

void requestUpdate(void)
	{
	if(vruiState->master)
		{
		Threads::Mutex::Lock eventPipeLock(vruiEventPipeMutex);
		
		/* Send a byte to the event pipe if nothing has been written yet: */
		if(vruiNumSignaledEvents==0)
			{
			char byte=1;
			if(write(vruiEventPipe[1],&byte,sizeof(char))<0)
				{
				/* There's nothing to do! Stupid gcc! */
				}
			}
		
		/* Count the number of pending events: */
		++vruiNumSignaledEvents;
		}
	}

void resizeWindow(VruiWindowGroup* windowGroup,const VRWindow* window,const int newViewportSize[2],const int newFrameSize[2])
	{
	/* Find the window in the window group's list: */
	for(std::vector<VruiWindowGroup::Window>::iterator wIt=windowGroup->windows.begin();wIt!=windowGroup->windows.end();++wIt)
		if(wIt->window==window)
			{
			/* Check if the window's viewport got bigger: */
			bool viewportBigger=wIt->viewportSize[0]<=newViewportSize[0]&&wIt->viewportSize[1]<=newViewportSize[1];
			
			/* Update the window's viewport size: */
			for(int i=0;i<2;++i)
				wIt->viewportSize[i]=newViewportSize[i];
			
			if(viewportBigger)
				{
				/* Update the window group's maximum viewport size: */
				for(int i=0;i<2;++i)
					if(windowGroup->maxViewportSize[i]<newViewportSize[i])
						windowGroup->maxViewportSize[i]=newViewportSize[i];
				}
			else
				{
				/* Recalculate the window group's maximum viewport size from scratch: */
				std::vector<VruiWindowGroup::Window>::iterator w2It=windowGroup->windows.begin();
				for(int i=0;i<2;++i)
					windowGroup->maxViewportSize[i]=w2It->viewportSize[i];
				for(++w2It;w2It!=windowGroup->windows.end();++w2It)
					for(int i=0;i<2;++i)
						if(windowGroup->maxViewportSize[i]<w2It->viewportSize[i])
							windowGroup->maxViewportSize[i]=w2It->viewportSize[i];
				}
			
			/* Check if the window's frame buffer got bigger: */
			bool frameBigger=wIt->frameSize[0]<=newFrameSize[0]&&wIt->frameSize[1]<=newFrameSize[1];
			
			/* Update the window's frame buffer size: */
			for(int i=0;i<2;++i)
				wIt->frameSize[i]=newFrameSize[i];
			
			if(frameBigger)
				{
				/* Update the window group's maximum frame buffer size: */
				for(int i=0;i<2;++i)
					if(windowGroup->maxFrameSize[i]<newFrameSize[i])
						windowGroup->maxFrameSize[i]=newFrameSize[i];
				}
			else
				{
				/* Recalculate the window group's maximum frame buffer size from scratch: */
				std::vector<VruiWindowGroup::Window>::iterator w2It=windowGroup->windows.begin();
				for(int i=0;i<2;++i)
					windowGroup->maxFrameSize[i]=w2It->frameSize[i];
				for(++w2It;w2It!=windowGroup->windows.end();++w2It)
					for(int i=0;i<2;++i)
						if(windowGroup->maxFrameSize[i]<w2It->frameSize[i])
							windowGroup->maxFrameSize[i]=w2It->frameSize[i];
				}
			
			break;
			}
	}

void getMaxWindowSizes(VruiWindowGroup* windowGroup,int viewportSize[2],int frameSize[2])
	{
	for(int i=0;i<2;++i)
		{
		viewportSize[i]=windowGroup->maxViewportSize[i];
		frameSize[i]=windowGroup->maxFrameSize[i];
		}
	}

}
