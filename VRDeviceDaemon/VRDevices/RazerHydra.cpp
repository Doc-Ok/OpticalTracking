/***********************************************************************
RazerHydra - Class to represent a Razer / Sixense Hydra dual-sensor
desktop 6-DOF tracking device.
Copyright (c) 2011-2013 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/RazerHydra.h>

#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <Misc/SizedTypes.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FunctionCalls.h>
#include <USB/DeviceList.h>
#include <Math/Math.h>

namespace {

/****************
Helper functions:
****************/

bool isInMotionTrackingMode(USB::Device& device)
	{
	/* Initialize the HID control packet: */
	unsigned char controlPacket[90];
	memset(controlPacket,0x00U,sizeof(controlPacket));
	
	/* Read the feature request via a control transfer: */
	device.readControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x01U,0x0300U,0x0001U,controlPacket,sizeof(controlPacket));
	
	/* Check for the motion tracking mode pattern: */
	bool result=true;
	result=result&&controlPacket[0]==0x02U; // Progress flag: 0x01U: still "warming up" coils; 0x02U: coils have stabilized, tracking can commence
	result=result&&controlPacket[1]==0x00U;
	result=result&&controlPacket[2]==0x00U;
	result=result&&controlPacket[3]==0x00U;
	result=result&&controlPacket[4]==0x00U;
	result=result&&controlPacket[5]==0x01U;
	result=result&&controlPacket[6]==0x00U;
	result=result&&controlPacket[7]==0x04U;
	result=result&&controlPacket[8]==0x03U;
	result=result&&controlPacket[9]==0x00U;
	result=result&&controlPacket[10]==0x00U;
	result=result&&controlPacket[86]==0x00U;
	result=result&&controlPacket[87]==0x00U;
	result=result&&controlPacket[88]==0x06U;
	result=result&&controlPacket[89]==0x00U;
	
	return result;
	}

void goToMotionTrackingMode(USB::Device& device)
	{
	/* Assemble the HID control packet: */
	unsigned char controlPacket[90];
	memset(controlPacket,0x00U,sizeof(controlPacket));
	controlPacket[5]=0x01U;
	controlPacket[7]=0x04U;
	controlPacket[8]=0x03U;
	controlPacket[88]=0x06U;
	
	/* Send the feature request via a control transfer: */
	device.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x09U,0x0300U,0x0001U,controlPacket,sizeof(controlPacket));
	}

void goToGamepadMode(USB::Device& device)
	{
	/* Assemble the HID control packet: */
	unsigned char controlPacket[90];
	memset(controlPacket,0x00U,sizeof(controlPacket));
	controlPacket[5]=0x01U;
	controlPacket[7]=0x04U;
	controlPacket[8]=0x00U;
	controlPacket[88]=0x05U;
	
	/* Send the feature request via a control transfer: */
	device.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x09U,0x0300U,0x0001U,controlPacket,sizeof(controlPacket));
	}

}

/***************************
Methods of class RazerHydra:
***************************/

bool RazerHydra::processUpdatePacket(RazerHydra::SensorState states[2])
	{
	/* Read the next packet: */
	packetBuffer.setDataSize(device.interruptTransfer(0x81U,static_cast<unsigned char*>(packetBuffer.getBuffer()),packetBuffer.getBufferSize()));
	if(packetBuffer.getUnread()==52)
		{
		/***************************
		Process the packet contents:
		***************************/
		
		/* Skip the packet header: */
		packetBuffer.skip<Misc::UInt8>(6);
		
		/* Process both sensors: */
		Scalar rawValues[14];
		for(int sensor=0;sensor<2;++sensor)
			{
			/* Skip sensor ID and packet counter: */
			packetBuffer.skip<Misc::UInt8>(2);
			states[sensor].sensorIndex=sensor;
			
			/* Read the raw sensor position: */
			for(int i=0;i<3;++i)
				rawValues[sensor*7+i]=Scalar(packetBuffer.read<Misc::SInt16>());
			
			/* Read the sensor orientation quaternion (quaternion in packet is w, x, y, z instead of x, y, z, w): */
			rawValues[sensor*7+3+3]=Scalar(packetBuffer.read<Misc::SInt16>())/Scalar(32767); // Convert to -1.0...+1.0 range
			for(int i=0;i<3;++i)
				rawValues[sensor*7+3+i]=Scalar(packetBuffer.read<Misc::SInt16>())/Scalar(32767); // Convert to -1.0...+1.0 range
			
			/* Process the button bit mask: */
			unsigned int buttonMask=packetBuffer.read<Misc::UInt8>();
			unsigned int buttonBit=0x1U;
			for(int i=0;i<7;++i,buttonBit<<=1)
				states[sensor].buttonStates[i]=(buttonMask&buttonBit)!=0x0U;
			
			/* Read the joystick axes: */
			for(int i=0;i<2;++i)
				states[sensor].valuatorStates[i]=Scalar(packetBuffer.read<Misc::SInt16>())/Scalar(32767); // Convert to -1.0...+1.0 range
			
			/* Read the analog trigger button: */
			states[sensor].valuatorStates[2]=Scalar(packetBuffer.read<Misc::UInt8>())/Scalar(255); // Convert to 0.0...1.0 range
			}
		
		if(applyInterleaveFilter)
			{
			/* Interleave-filter the raw values: */
			for(int value=0;value<14;++value)
				{
				/* Update the coefficients of the current formula based on the new measurement: */
				Scalar p0=valueBuffers[value][valueBufferIndex];
				Scalar p1=rawValues[value];
				smoothingCoeffs[value][valueBufferIndex][0]=p0;
				smoothingCoeffs[value][valueBufferIndex][1]=(p1-p0)*Scalar(0.25);
				
				/* Update the value buffer: */
				valueBuffers[value][valueBufferIndex]=p1;
				
				/* Evaluate the smoothing formulae: */
				Scalar pSum=Scalar(0);
				for(int formula=0;formula<4;++formula)
					{
					Scalar t=Scalar((valueBufferIndex-formula+4)%4+4);
					pSum+=smoothingCoeffs[value][formula][0]+smoothingCoeffs[value][formula][1]*t;
					}
				rawValues[value]=pSum*Scalar(0.25);
				}
			valueBufferIndex=(valueBufferIndex+1)%4;
			}
		
		if(applyLowpassFilter)
			{
			/* Low-pass filter the raw or interleave-filtered values: */
			for(int sensor=0;sensor<2;++sensor)
				{
				/* Calculate sensor's distance from base station in mm: */
				Point accumPos(accumulators+(sensor*7+0));
				Scalar baseDist2=accumPos.sqr();
				
				/* Accumulate position values: */
				for(int i=0;i<3;++i)
					{
					Scalar invWeight=lowpassFilterStrength*baseDist2/Scalar(1.0e6);
					Scalar weight=invWeight>Scalar(1)?Scalar(1)-Scalar(1)/invWeight:Scalar(0);
					accumulators[sensor*7+i]=accumulators[sensor*7+i]*weight+rawValues[sensor*7+i]*(Scalar(1)-weight);
					}
				
				/* Accumulate orientation values: */
				for(int i=0;i<4;++i)
					{
					Scalar invWeight=lowpassFilterStrength*baseDist2/Scalar(1.0e6);
					Scalar weight=invWeight>Scalar(1)?Scalar(1)-Scalar(1)/invWeight:Scalar(0);
					accumulators[sensor*7+3+i]=accumulators[sensor*7+3+i]*weight+rawValues[sensor*7+3+i]*(Scalar(1)-weight);
					}
				}
			
			/* Store low-pass-filtered values in the sensor state structures: */
			for(int sensor=0;sensor<2;++sensor)
				{
				for(int i=0;i<3;++i)
					states[sensor].position[i]=accumulators[sensor*7+i]*positionConversionFactor;
				states[sensor].orientation=Orientation::fromQuaternion(accumulators+(sensor*7+3));
				}
			}
		else
			{
			/* Store the raw or interleave-filtered values in the sensor state structures: */
			for(int sensor=0;sensor<2;++sensor)
				{
				for(int i=0;i<3;++i)
					states[sensor].position[i]=rawValues[sensor*7+i]*positionConversionFactor;
				states[sensor].orientation=Orientation::fromQuaternion(rawValues+(sensor*7+3));
				}
			}
		
		return true;
		}
	else
		return false;
	}

void* RazerHydra::streamingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Receive packets while in streaming mode: */
	SensorState states[2];
	while(streaming)
		{
		/* Wait for and process the next update packet: */
		if(processUpdatePacket(states)) // Only call the callback with valid packets
			{
			/* Call the streaming callback with each sensor state in turn: */
			for(int sensor=0;sensor<2;++sensor)
				(*streamingCallback)(states[sensor]);
			}
		}
	
	return 0;
	}

RazerHydra::RazerHydra(USB::Context& usbContext,unsigned int index)
	:wasInGamepadMode(false),
	 positionConversionFactor(1),
	 packetBuffer(64), // 64 is maximum packet size advertised in USB device descriptor; actual packets are 52 bytes
	 applyInterleaveFilter(true), // Should always be applied
	 applyLowpassFilter(true), // Should probably be applied
	 lowpassFilterStrength(24),
	 streaming(false),streamingCallback(0)
	{
	{
	/* Get the list of all USB devices: */
	USB::DeviceList deviceList(usbContext);
	
	/* Get the index-th device with the Razer Hydra's vendor/product ID: */
	device=deviceList.getDevice(0x1532U,0x0300U,index);
	}
	
	if(!device.isValid())
		Misc::throwStdErr("RazerHydra::RazerHydra: Less than %u Razer Hydra devices detected on USB bus",index+1);
	
	/* Open the device and claim both interfaces: */
	device.open();
	for(unsigned int interface=0;interface<2;++interface)
		device.claimInterface(interface,true); // Detach kernel HID driver if necessary
	
	/* Send the control packet sequence to initialize the device: */
	unsigned char replyBuffer[256];
	for(unsigned int interface=0;interface<2;++interface)
		{
		device.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x0aU,0x0000U,interface,0,0);
		device.readControl(LIBUSB_REQUEST_TYPE_STANDARD|LIBUSB_RECIPIENT_INTERFACE,0x06U,0x2200U,interface,replyBuffer,sizeof(replyBuffer));
		try
			{
			/* This request always fails, but seems to be necessary: */
			device.readControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x01U,0x0100U,interface,replyBuffer,sizeof(replyBuffer));
			}
		catch(std::runtime_error)
			{
			/* Just ignore the error and carry on */
			}
		}
	
	/* Check if the device is not in motion tracking mode: */
	if(!isInMotionTrackingMode(device))
		{
		/* Remember for later: */
		wasInGamepadMode=true;
		
		/* This may take a while, so print a warning message: */
		std::cout<<"RazerHydra::RazerHydra: Switching Razer Hydra device "<<index<<" to motion tracking mode."<<std::endl;
		
		/* Request motion tracking mode until the device obliges: */
		std::cout<<"RazerHydra::RazerHydra: Warming up magnetic coils..."<<std::flush;
		do
			{
			goToMotionTrackingMode(device);
			sleep(1);
			}
		while(!isInMotionTrackingMode(device));
		std::cout<<" done."<<std::endl;
		}
	
	if(applyInterleaveFilter)
		{
		/* Initialize the smoothing functions: */
		for(int value=0;value<14;++value)
			{
			for(int i=0;i<4;++i)
				{
				valueBuffers[value][i]=Scalar(0);
				smoothingCoeffs[value][i][0]=smoothingCoeffs[value][i][1]=Scalar(0);
				}
			}
		valueBufferIndex=0;
		}
	
	if(applyLowpassFilter)
		{
		/* Initialize the accumulation buffers: */
		for(int value=0;value<14;++value)
			accumulators[value]=Scalar(0);
		}
	}

RazerHydra::~RazerHydra(void)
	{
	if(streaming)
		{
		/* Shut down streaming mode: */
		streaming=false; // This will tell streaming thread to terminate at next opportunity
		streamingThread.join();
		
		/* Release the streaming callback: */
		delete streamingCallback;
		}
	
	/* Check if the device was in gamepad emulation mode when initially opened: */
	if(wasInGamepadMode)
		{
		/* Reset it to initial state: */
		std::cout<<"RazerHydra::~RazerHydra: Switching Razer Hydra device back to game pad mode."<<std::endl;
		goToGamepadMode(device);
		}
	}

void RazerHydra::setPositionConversionFactor(RazerHydra::Scalar newPositionConversionFactor)
	{
	/* Throw an exception if the device is currently streaming: */
	if(streaming)
		Misc::throwStdErr("RazerHydra::setPositionConversionFactor: Cannot change position conversion factor while streaming");
	
	/* Simply set it: */
	positionConversionFactor=newPositionConversionFactor;
	}

void RazerHydra::setApplyInterleaveFilter(bool newApplyInterleaveFilter)
	{
	applyInterleaveFilter=newApplyInterleaveFilter;
	}

void RazerHydra::setApplyLowpassFilter(bool newApplyLowpassFilter)
	{
	applyLowpassFilter=newApplyLowpassFilter;
	}

void RazerHydra::setLowpassFilterStrength(RazerHydra::Scalar newLowpassFilterStrength)
	{
	lowpassFilterStrength=newLowpassFilterStrength;
	}

void RazerHydra::pollSensors(RazerHydra::SensorState states[2])
	{
	/* Throw an exception if the device is currently streaming: */
	if(streaming)
		Misc::throwStdErr("RazerHydra::pollSensors: Cannot poll sensors while streaming");
	
	/* Wait for and process the next valid update packet: */
	while(!processUpdatePacket(states))
		;
	}

void RazerHydra::startStreaming(RazerHydra::StreamingCallback* newStreamingCallback)
	{
	/* Throw an exception if the device is currently streaming: */
	if(streaming)
		Misc::throwStdErr("RazerHydra::startStreaming: Device is already streaming");
	
	/* Install the streaming callback: */
	streamingCallback=newStreamingCallback;
	
	/* Start the streaming thread: */
	streaming=true;
	streamingThread.start(this,&RazerHydra::streamingThreadMethod);
	}

void RazerHydra::stopStreaming(void)
	{
	/* Throw an exception if the device is not currently streaming: */
	if(streaming)
		Misc::throwStdErr("RazerHydra::stopStreaming: Device is not streaming");
	
	/* Shut down the streaming thread: */
	streaming=false; // This will tell streaming thread to terminate at next opportunity
	streamingThread.join();
	
	/* Release the streaming callback: */
	delete streamingCallback;
	}
