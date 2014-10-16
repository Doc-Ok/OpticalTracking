/***********************************************************************
PSMoveUtil - Utility program to detect, list, and configure PlayStation
Move input devices via USB.
Copyright (c) 2013 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <libusb-1.0/libusb.h>
#include <USB/Context.h>
#include <USB/Device.h>
#include <USB/DeviceList.h>

USB::Context usbContext;

const unsigned short psmoveVendorId=0x054c;
const unsigned short psmoveProductId=0x03d5;

void list(void)
	{
	/* Get the list of all USB devices: */
	USB::DeviceList deviceList(usbContext);
	
	/* Get the number of PS Move devices: */
	size_t numMoves=deviceList.getNumDevices(psmoveVendorId,psmoveProductId);
	
	/* Print information about all PS Move devices: */
	for(size_t i=0;i<numMoves;++i)
		{
		/* Get and open the i-th PS Move device: */
		USB::Device move=deviceList.getDevice(psmoveVendorId,psmoveProductId,i);
		move.open();
		
		/* Print the device's information: */
		std::cout<<"PS Move "<<i<<": USB address ";
		std::cout<<std::setfill('0')<<std::setw(3)<<move.getBusNumber()<<":"<<std::setfill('0')<<std::setw(3)<<move.getAddress();
		std::cout<<", device serial number "<<move.getSerialNumber()<<std::endl;
		}
	}

void pair(USB::Device& move,const char* btHostAddr)
	{
	/* Open the USB device and claim the first interface: */
	move.open();
	// move.setConfiguration(1);
	move.claimInterface(0,true); // Detach kernel driver if one is present
	
	/* Assemble the HID feature request to query the PS Move's current destination bluetooth address: */
	unsigned char featureRequest[23];
	memset(featureRequest,0x00U,sizeof(featureRequest));
	featureRequest[0]=0x04U;
	
	/* Send the HID feature request: */
	if(move.readControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x01U,(0x03U<<8)|featureRequest[0],0x0000U,featureRequest,sizeof(featureRequest))!=16)
		{
		std::cerr<<"Error while reading PS Move's current bluetooth addresses"<<std::endl;
		return;
		}
	
	/* Print the PS Move's controller bluetooth address: */
	std::cout<<"Controller bluetooth address:   ";
	std::cout<<std::hex;
	for(int i=0;i<6;++i)
		{
		if(i>0)
			std::cout<<':';
		std::cout<<std::setfill('0')<<std::setw(2)<<(unsigned int)(featureRequest[6-i]);
		}
	std::cout<<std::dec<<std::endl;
	
	/* Print the PS Move's host bluetooth address: */
	std::cout<<"Current host bluetooth address: ";
	std::cout<<std::hex;
	for(int i=0;i<6;++i)
		{
		if(i>0)
			std::cout<<':';
		std::cout<<std::setfill('0')<<std::setw(2)<<(unsigned int)(featureRequest[15-i]);
		}
	std::cout<<std::dec<<std::endl;
	
	/* Convert the new host bluetooth address to binary: */
	unsigned char btAddr[6];
	for(int i=0;i<6;++i)
		{
		if(i>0)
			++btHostAddr;
		unsigned int byte=0x00U;
		for(int j=0;j<2;++j,++btHostAddr)
			{
			byte<<=4;
			if(*btHostAddr>='0'&&*btHostAddr<='9')
				byte|=(unsigned int)(*btHostAddr-'0');
			else if(*btHostAddr>='A'&&*btHostAddr<='F')
				byte|=(unsigned int)(*btHostAddr-'A'+10);
			else if(*btHostAddr>='a'&&*btHostAddr<='f')
				byte|=(unsigned int)(*btHostAddr-'a'+10);
			}
		btAddr[5-i]=(unsigned char)byte;
		}
	
	/* Print the new host bluetooth address: */
	std::cout<<"New host bluetooth address:     ";
	std::cout<<std::hex;
	for(int i=0;i<6;++i)
		{
		if(i>0)
			std::cout<<':';
		std::cout<<std::setfill('0')<<std::setw(2)<<(unsigned int)(btAddr[5-i]);
		}
	std::cout<<std::dec<<std::endl;
	
	#if 1
	
	/* Check if the new host address is different from the current: */
	int compare;
	for(compare=0;compare<6&&btAddr[5-compare]==featureRequest[15-compare];++compare)
		;
	
	if(compare<6)
		{
		std::cout<<"Configuring new bluetooth host address "<<btHostAddr<<std::endl;
		
		/* Assemble the feature request to set the PS Move's host bluetooth address: */
		memset(featureRequest,0x00U,sizeof(featureRequest));
		featureRequest[0]=0x05U;
		for(int i=0;i<6;++i)
			featureRequest[1+i]=btAddr[i];
		featureRequest[7]=0x10U;
		featureRequest[8]=0x01U;
		featureRequest[9]=0x02U;
		featureRequest[10]=0x12U;
		
		/* Send the HID feature request: */
		move.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x09U,(0x03U<<8)|featureRequest[0],0x0000U,featureRequest,11);
		}
	else
		{
		std::cout<<"Bluetooth host address already configured"<<std::endl;
		}
	
	#endif
	
	/* Close the device: */
	move.close();
	}

void setLED(USB::Device& move,int red,int green,int blue)
	{
	/* Open the USB device and claim the first interface: */
	move.open();
	// move.setConfiguration(1);
	move.claimInterface(0,true); // Detach kernel driver if one is present
	
	/* Assemble the HID feature request to set the PS Move's LED color: */
	unsigned char featureRequest[23];
	memset(featureRequest,0x00U,sizeof(featureRequest));
	featureRequest[0]=0xfaU;
	featureRequest[1]=(unsigned char)red;
	featureRequest[2]=(unsigned char)red;
	featureRequest[3]=(unsigned char)green;
	featureRequest[4]=(unsigned char)green;
	featureRequest[5]=(unsigned char)blue;
	featureRequest[6]=(unsigned char)blue;
	
	/* Send the HID feature request: */
	move.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x09U,(0x03U<<8)|featureRequest[0],0x0000U,featureRequest+1,sizeof(featureRequest)-1);
	
	/* Close the device: */
	move.close();
	}

int main(int argc,char* argv[])
	{
	try
		{
		/* Initialize the USB context: */
		usbContext.setDebugLevel(3);
		usbContext.startEventHandling();
		
		/* Parse the command line: */
		if(argc<2)
			{
			std::cerr<<"Missing command. Usage:"<<std::endl;
			std::cerr<<"PSMoveUtil list | ( pair ( <device index> | all ) <bluetooth host address> )"<<std::endl;
			return 1;
			}
		if(strcasecmp(argv[1],"list")==0)
			{
			/* List all USB-connected PS Move devices: */
			list();
			}
		else if(strcasecmp(argv[1],"pair")==0)
			{
			/* Get the index of the connected PS Move device which to pair: */
			if(argc<4)
				{
				std::cerr<<"Malformed pair command. Usage:"<<std::endl;
				std::cerr<<"PSMoveUtil pair ( <device index> | all ) <bluetooth host address>"<<std::endl;
				return 1;
				}
			else if(strcasecmp(argv[2],"all")==0)
				{
				/* Get the list of all USB devices: */
				USB::DeviceList deviceList(usbContext);
				
				/* Get the number of PS Move devices: */
				size_t numMoves=deviceList.getNumDevices(psmoveVendorId,psmoveProductId);
				
				/* Pair all PS Move devices: */
				for(size_t i=0;i<numMoves;++i)
					{
					/* Get and pair the i-th PS Move device: */
					USB::Device move=deviceList.getDevice(psmoveVendorId,psmoveProductId,i);
					pair(move,argv[3]);
					}
				}
			else
				{
				/* Get the device index: */
				size_t deviceIndex=atoi(argv[2]);
				
				/* Get the list of all USB devices: */
				USB::DeviceList deviceList(usbContext);
				
				/* Get the number of PS Move devices: */
				size_t numMoves=deviceList.getNumDevices(psmoveVendorId,psmoveProductId);
				
				if(deviceIndex>=numMoves)
					{
					std::cerr<<"Less than "<<deviceIndex+1<<" PlayStation Move devices connected to USB bus"<<std::endl;
					return 1;
					}
				else
					{
					/* Get and pair the index-th PS Move device: */
					USB::Device move=deviceList.getDevice(psmoveVendorId,psmoveProductId,deviceIndex);
					pair(move,argv[3]);
					}
				}
			}
		else if(strcasecmp(argv[1],"setLED")==0)
			{
			/* Get the index of the connected PS Move device which to pair: */
			if(argc<6)
				{
				std::cerr<<"Malformed setLED command. Usage:"<<std::endl;
				std::cerr<<"PSMoveUtil setLED <device index> <red> <green> <blue>"<<std::endl;
				return 1;
				}
			
			/* Get the device index: */
			size_t deviceIndex=atoi(argv[2]);
			
			/* Get the list of all USB devices: */
			USB::DeviceList deviceList(usbContext);
			
			/* Get the number of PS Move devices: */
			size_t numMoves=deviceList.getNumDevices(psmoveVendorId,psmoveProductId);
			
			if(deviceIndex>=numMoves)
				{
				std::cerr<<"Less than "<<deviceIndex+1<<" PlayStation Move devices connected to USB bus"<<std::endl;
				return 1;
				}
			else
				{
				/* Get and set the LED color of the index-th PS Move device: */
				USB::Device move=deviceList.getDevice(psmoveVendorId,psmoveProductId,deviceIndex);
				setLED(move,atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
				}
			}
		else
			{
			std::cerr<<"Unknown command "<<argv[1]<<". Usage:"<<std::endl;
			std::cerr<<"PSMoveUtil list | ( pair ( <device index> | all ) <bluetooth host address> )"<<std::endl;
			return 1;
			}
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<std::endl;
		return 1;
		}
	
	return 0;
	}
