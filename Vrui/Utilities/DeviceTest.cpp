/***********************************************************************
DeviceTest - Program to test the connection to a Vrui VR Device Daemon
and to dump device positions/orientations and button states.
Copyright (c) 2002-2014 Oliver Kreylos

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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <Misc/Timer.h>
#include <Misc/ConfigurationFile.h>
#include <Realtime/Time.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/OutputOperators.h>
#include <Vrui/Internal/VRDeviceDescriptor.h>
#include <Vrui/Internal/VRDeviceClient.h>

typedef Vrui::VRDeviceState::TrackerState TrackerState;
typedef TrackerState::PositionOrientation PositionOrientation;
typedef PositionOrientation::Scalar Scalar;
typedef PositionOrientation::Point Point;
typedef PositionOrientation::Vector Vector;
typedef PositionOrientation::Rotation Rotation;

class LatencyHistogram // Helper class to collect and print tracker data latency histograms
	{
	/* Elements: */
	private:
	unsigned int binSize; // Size of a histogram bin in microseconds
	unsigned int maxBinLatency; // Maximum latency to expect in microseconds
	unsigned int numBins; // Number of bins in the histogram
	unsigned int* bins; // Array of histogram bins
	unsigned int numSamples; // Number of samples in current observation period
	double latencySum; // Sum of all latencies to calculate average latency
	unsigned int minLatency,maxLatency; // Latency range in current observation period in microseconds
	unsigned int maxBinSize; // Maximum number of samples in any bin
	
	/* Constructors and destructors: */
	public:
	LatencyHistogram(unsigned int sBinSize,unsigned int sMaxBinLatency)
		:binSize(sBinSize),maxBinLatency(sMaxBinLatency),
		 numBins(maxBinLatency/binSize+2),bins(new unsigned int[numBins])
		{
		/* Initialize the histogram: */
		reset();
		}
	~LatencyHistogram(void)
		{
		delete[] bins;
		}
	
	/* Methods: */
	void reset(void) // Resets the histogram for the next observation period
		{
		/* Clear the histogram: */
		for(unsigned int i=0;i<numBins;++i)
			bins[i]=0;
		
		/* Reset the latency counter and range: */
		numSamples=0;
		latencySum=0.0;
		minLatency=~0x0U;
		maxLatency=0U;
		maxBinSize=0U;
		}
	void addSample(unsigned int latency) // Adds a latency sample
		{
		/* Update the histogram: */
		unsigned int binIndex=latency/binSize;
		if(binIndex>numBins-1)
			binIndex=numBins-1; // All outliers go into the last bin
		++bins[binIndex];
		if(maxBinSize<bins[binIndex])
			maxBinSize=bins[binIndex];
		
		/* Update sample counter and range: */
		++numSamples;
		latencySum+=double(latency);
		if(minLatency>latency)
			minLatency=latency;
		if(maxLatency<latency)
			maxLatency=latency;
		}
	unsigned int getNumSamples(void) const
		{
		return numSamples;
		}
	void printHistogram(void) const // Prints the histogram
		{
		/* Calculate the range of non-empty bins: */
		unsigned int firstBinIndex=minLatency/binSize;
		if(firstBinIndex>numBins-1)
			firstBinIndex=numBins-1;
		unsigned int lastBinIndex=maxLatency/binSize;
		if(lastBinIndex>numBins-1)
			lastBinIndex=numBins-1;
		
		std::cout<<"Histogram of "<<numSamples<<" latency samples:"<<std::endl;
		for(unsigned int i=firstBinIndex;i<=lastBinIndex;++i)
			{
			if(i<numBins-1)
				std::cout<<std::setw(8)<<i*binSize<<' ';
			else
				std::cout<<"Outliers ";
			unsigned int width=(bins[i]*71+maxBinSize-1)/maxBinSize;
			for(unsigned int j=0;j<width;++j)
				std::cout<<'*';
			std::cout<<std::endl;
			}
		
		std::cout<<"Average latency: "<<latencySum/double(numSamples)<<" us"<<std::endl;
		}
	};

void printTrackerPos(const Vrui::VRDeviceState& state,int trackerIndex)
	{
	const TrackerState& ts=state.getTrackerState(trackerIndex);
	Point pos=ts.positionOrientation.getOrigin();
	std::cout.setf(std::ios::fixed);
	std::cout.precision(3);
	std::cout<<"("<<std::setw(9)<<pos[0]<<" "<<std::setw(9)<<pos[1]<<" "<<std::setw(9)<<pos[2]<<")";
	}

void printTrackerPosOrient(const Vrui::VRDeviceState& state,int trackerIndex)
	{
	const TrackerState& ts=state.getTrackerState(trackerIndex);
	Point pos=ts.positionOrientation.getOrigin();
	Rotation rot=ts.positionOrientation.getRotation();
	Vector axis=rot.getScaledAxis();
	Scalar angle=Math::deg(rot.getAngle());
	std::cout.setf(std::ios::fixed);
	std::cout.precision(3);
	std::cout<<"("<<std::setw(8)<<pos[0]<<" "<<std::setw(8)<<pos[1]<<" "<<std::setw(8)<<pos[2]<<") ";
	std::cout<<"("<<std::setw(8)<<axis[0]<<" "<<std::setw(8)<<axis[1]<<" "<<std::setw(8)<<axis[2]<<") ";
	std::cout<<std::setw(8)<<angle;
	}

void printTrackerFrame(const Vrui::VRDeviceState& state,int trackerIndex)
	{
	const TrackerState& ts=state.getTrackerState(trackerIndex);
	Point pos=ts.positionOrientation.getOrigin();
	Rotation rot=ts.positionOrientation.getRotation();
	Vector x=rot.getDirection(0);
	Vector y=rot.getDirection(1);
	Vector z=rot.getDirection(2);
	std::cout.setf(std::ios::fixed);
	std::cout.precision(3);
	std::cout<<"("<<std::setw(8)<<pos[0]<<" "<<std::setw(8)<<pos[1]<<" "<<std::setw(8)<<pos[2]<<") ";
	std::cout<<"("<<std::setw(6)<<x[0]<<" "<<std::setw(6)<<x[1]<<" "<<std::setw(6)<<x[2]<<") ";
	std::cout<<"("<<std::setw(6)<<y[0]<<" "<<std::setw(6)<<y[1]<<" "<<std::setw(6)<<y[2]<<") ";
	std::cout<<"("<<std::setw(6)<<z[0]<<" "<<std::setw(6)<<z[1]<<" "<<std::setw(6)<<z[2]<<")";
	}

void printButtons(const Vrui::VRDeviceState& state)
	{
	for(int i=0;i<state.getNumButtons();++i)
		{
		if(i>0)
			std::cout<<" ";
		if(state.getButtonState(i))
			std::cout<<"X";
		else
			std::cout<<'.';
		}
	}

void printValuators(const Vrui::VRDeviceState& state)
	{
	std::cout.setf(std::ios::fixed);
	std::cout.precision(3);
	for(int i=0;i<state.getNumValuators();++i)
		{
		if(i>0)
			std::cout<<" ";
		std::cout<<std::setw(6)<<state.getValuatorState(i);
		}
	}

int main(int argc,char* argv[])
	{
	/* Parse command line: */
	char* serverName=0;
	bool printDevices=false;
	int trackerIndex=0;
	int printMode=0;
	bool printButtonStates=false;
	bool printNewlines=false;
	bool savePositions=false;
	std::string saveFileName;
	int triggerIndex=0;
	int latencyIndex=-1;
	unsigned int latencyBinSize=250;
	unsigned int latencyMaxLatency=20000;
	unsigned int latencyNumSamples=1000;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i],"-listDevices")==0||strcasecmp(argv[i],"-ld")==0)
				printDevices=true;
			else if(strcasecmp(argv[i],"-t")==0||strcasecmp(argv[i],"--trackerIndex")==0)
				{
				++i;
				trackerIndex=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i],"-alltrackers")==0)
				trackerIndex=-1;
			else if(strcasecmp(argv[i],"-p")==0)
				printMode=0;
			else if(strcasecmp(argv[i],"-o")==0)
				printMode=1;
			else if(strcasecmp(argv[i],"-f")==0)
				printMode=2;
			else if(strcasecmp(argv[i],"-v")==0)
				printMode=3;
			else if(strcasecmp(argv[i],"-b")==0)
				printButtonStates=true;
			else if(strcasecmp(argv[i],"-n")==0)
				printNewlines=true;
			else if(strcasecmp(argv[i],"-save")==0)
				{
				savePositions=true;
				++i;
				saveFileName=argv[i];
				}
			else if(strcasecmp(argv[i],"-trigger")==0)
				{
				++i;
				triggerIndex=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i],"-latency")==0)
				{
				++i;
				latencyIndex=atoi(argv[i]);
				++i;
				latencyBinSize=(unsigned int)(atoi(argv[i]));
				++i;
				latencyMaxLatency=(unsigned int)(atoi(argv[i]));
				++i;
				latencyNumSamples=(unsigned int)(atoi(argv[i]));
				}
			}
		else
			serverName=argv[i];
		}
	
	if(serverName==0)
		{
		std::cerr<<"Usage: "<<argv[0]<<" [-ld | -listDevices] [(-t | --trackerIndex) <trackerIndex>] [-alltrackers] [-p | -o | -f | -v] [-b] [-n] [-save <save file name>] [-trigger <trigger index>] [-latency <trackerIndex> <bin size> <max latency> <num samples>] <serverName:serverPort>"<<std::endl;
		return 1;
		}
	
	/* Split the server name into hostname:port: */
	char* colonPtr=0;
	for(char* cPtr=serverName;*cPtr!='\0';++cPtr)
		if(*cPtr==':')
			colonPtr=cPtr;
	int portNumber=0;
	if(colonPtr!=0)
		{
		portNumber=atoi(colonPtr+1);
		*colonPtr='\0';
		}
	
	/* Initialize device client: */
	Vrui::VRDeviceClient* deviceClient=0;
	try
		{
		deviceClient=new Vrui::VRDeviceClient(serverName,portNumber);
		}
	catch(std::runtime_error error)
		{
		std::cerr<<"Caught exception "<<error.what()<<" while initializing VR device client"<<std::endl;
		return 1;
		}
	
	if(printDevices)
		{
		/* Print information about the server's virtual input devices: */
		std::cout<<"Device server at "<<serverName<<":"<<portNumber<<" defines "<<deviceClient->getNumVirtualDevices()<<" virtual input devices."<<std::endl;
		for(int deviceIndex=0;deviceIndex<deviceClient->getNumVirtualDevices();++deviceIndex)
			{
			const Vrui::VRDeviceDescriptor& vd=deviceClient->getVirtualDevice(deviceIndex);
			std::cout<<"Virtual device "<<vd.name<<":"<<std::endl;
			std::cout<<"  Track type: ";
			if(vd.trackType&Vrui::VRDeviceDescriptor::TRACK_ORIENT)
				std::cout<<"6-DOF";
			else if(vd.trackType&Vrui::VRDeviceDescriptor::TRACK_DIR)
				std::cout<<"Ray-based";
			else if(vd.trackType&Vrui::VRDeviceDescriptor::TRACK_POS)
				std::cout<<"3-DOF";
			else
				std::cout<<"None";
			std::cout<<std::endl;
			
			if(vd.trackType&Vrui::VRDeviceDescriptor::TRACK_DIR)
				std::cout<<"  Device ray direction: "<<vd.rayDirection<<", start: "<<vd.rayStart<<std::endl;
			
			if(vd.trackType&Vrui::VRDeviceDescriptor::TRACK_POS)
				std::cout<<"  Tracker index: "<<vd.trackerIndex<<std::endl;
			
			if(vd.numButtons>0)
				{
				std::cout<<"  "<<vd.numButtons<<" buttons:";
				for(int i=0;i<vd.numButtons;++i)
					std::cout<<" ("<<vd.buttonNames[i]<<", "<<vd.buttonIndices[i]<<")";
				std::cout<<std::endl;
				}
			
			if(vd.numValuators>0)
				{
				std::cout<<"  "<<vd.numValuators<<" valuators:";
				for(int i=0;i<vd.numValuators;++i)
					std::cout<<" ("<<vd.valuatorNames[i]<<", "<<vd.valuatorIndices[i]<<")";
				std::cout<<std::endl;
				}
			}
		std::cout<<std::endl;
		}
	
	/* Disable printing of tracking information if there are no trackers: */
	deviceClient->lockState();
	if(printMode==0&&deviceClient->getState().getNumTrackers()==0)
		printMode=-1;
	deviceClient->unlockState();
	
	/* Open the save file: */
	FILE* saveFile=0;
	if(savePositions)
		saveFile=fopen(saveFileName.c_str(),"wt");
	
	/* Print output header line: */
	switch(printMode)
		{
		case 0:
			std::cout<<"     Pos X     Pos Y     Pos Z "<<std::endl;
			break;
		
		case 1:
			std::cout<<"    Pos X    Pos Y    Pos Z     Axis X   Axis Y   Axis Z     Angle"<<std::endl;
			break;
		
		case 2:
			std::cout<<"    Pos X    Pos Y    Pos Z     XA X   XA Y   XA Z     YA X   YA Y   YA Z     ZA X   ZA Y   ZA Z "<<std::endl;
			break;
		}
	
	LatencyHistogram* latencyHistogram=0;
	if(latencyIndex>=0)
		{
		/* Create a latency histogram: */
		latencyHistogram=new LatencyHistogram(latencyBinSize,latencyMaxLatency);
		}
	
	/* Run main loop: */
	Misc::Timer t;
	int numPackets=0;
	try
		{
		deviceClient->activate();
		deviceClient->startStream(0);
		bool loop=true;
		bool oldTriggerState=false;
		while(loop)
			{
			/* Wait for next packet: */
			deviceClient->getPacket();
			Realtime::TimePointMonotonic now;
			Vrui::VRDeviceState::TimeStamp nowTs=Vrui::VRDeviceState::TimeStamp(now.tv_sec*1000000+(now.tv_nsec+500)/1000);
			++numPackets;
			
			/* Print new device state: */
			if(!printNewlines)
				std::cout<<"\r";
			deviceClient->lockState();
			const Vrui::VRDeviceState& state=deviceClient->getState();
			
			if(latencyHistogram!=0)
				{
				latencyHistogram->addSample(nowTs-state.getTrackerTimeStamp(latencyIndex));
				if(latencyHistogram->getNumSamples()>=latencyNumSamples)
					{
					latencyHistogram->printHistogram();
					latencyHistogram->reset();
					}
				}
			
			if(savePositions&&saveFile!=0)
				{
				if(oldTriggerState==false&&state.getButtonState(triggerIndex))
					{
					/* Save the current position: */
					Point::AffineCombiner pc;
					for(int i=0;i<50;++i)
						{
						/* Accumulate the current position: */
						const TrackerState& ts=state.getTrackerState(trackerIndex);
						pc.addPoint(ts.positionOrientation.getOrigin());

						/* Wait for the next packet: */
						deviceClient->unlockState();
						deviceClient->getPacket();
						deviceClient->lockState();
						}

					/* Save the accumulated position: */
					Point p=pc.getPoint();
					fprintf(saveFile,"%14.8f %14.8f %14.8f\n",p[0],p[1],p[2]);
					}
				oldTriggerState=state.getButtonState(triggerIndex);
				}

			switch(printMode)
				{
				case 0:
					if(trackerIndex<0)
						{
						printTrackerPos(state,0);
						for(int i=1;i<state.getNumTrackers();++i)
							{
							std::cout<<" ";
							printTrackerPos(state,i);
							}
						}
					else
						printTrackerPos(state,trackerIndex);
					break;

				case 1:
					printTrackerPosOrient(state,trackerIndex);
					break;

				case 2:
					printTrackerFrame(state,trackerIndex);
					break;

				case 3:
					printValuators(state);
					break;

				default:
					; // Print nothing; nothing, I say!
				}
			if(printButtonStates)
				{
				std::cout<<" ";
				printButtons(state);
				}
			deviceClient->unlockState();
			if(printNewlines)
				std::cout<<std::endl;
			else
				std::cout<<std::flush;

			/* Check for a key press event: */
			fd_set readFdSet;
			FD_ZERO(&readFdSet);
			FD_SET(fileno(stdin),&readFdSet);
			struct timeval timeout;
			timeout.tv_sec=0;
			timeout.tv_usec=0;
			bool dataWaiting=select(fileno(stdin)+1,&readFdSet,0,0,&timeout)>=0&&FD_ISSET(fileno(stdin),&readFdSet);
			if(dataWaiting)
				loop=false;
			}
		std::cout<<std::endl;
		}
	catch(std::runtime_error err)
		{
		if(!printNewlines)
			std::cout<<std::endl;
		std::cerr<<"Caught exception "<<err.what()<<" while reading tracking data"<<std::endl;
		}
	t.elapse();
	std::cout<<"Received "<<numPackets<<" device data packets in "<<t.getTime()*1000.0<<" ms ("<<double(numPackets)/t.getTime()<<" packets/s)"<<std::endl;
	deviceClient->stopStream();
	deviceClient->deactivate();
	
	/* Clean up and terminate: */
	delete latencyHistogram;
	if(saveFile!=0)
		fclose(saveFile);
	delete deviceClient;
	return 0;
	}
