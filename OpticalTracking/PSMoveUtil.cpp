/***********************************************************************
PSMoveUtil - Utility program to detect, list, and configure PlayStation
Move input devices via USB.
Copyright (c) 2013-2017 Oliver Kreylos

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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <libusb-1.0/libusb.h>
#include <USB/Device.h>
#include <USB/DeviceList.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <dbus/dbus.h>

inline char toHex(int value,bool upperCase)
	{
	if(value<10)
		return char('0'+value);
	else if(upperCase)
		return char('A'+value-10);
	else
		return char('a'+value-10);
	}

const char* btAddrToString(const unsigned char btAddr[6],char string[18],bool upperCase =false)
	{
	/* Convert address bytes to hexadecimal in reverse order: */
	char* sPtr=string;
	for(int i=5;i>=0;--i,sPtr+=3)
		{
		sPtr[0]=toHex(int(btAddr[i])/16,upperCase);
		sPtr[1]=toHex(int(btAddr[i])%16,upperCase);
		sPtr[2]=':';
		}
	sPtr[-1]='\0';
	
	return string;
	}

const unsigned char* stringToBtAddr(const char string[18],unsigned char btAddr[6])
	{
	/* Convert hex numbers from string to bytes in reverse order: */
	const char* sPtr=string;
	for(int i=5;i>=0;--i)
		{
		/* Convert the 2-digit hex code to a byte: */
		unsigned int byte=0x00U;
		for(int j=0;j<2;++j,++sPtr)
			{
			byte<<=4;
			if(*sPtr>='0'&&*sPtr<='9')
				byte|=(unsigned int)(*sPtr-'0');
			else if(*sPtr>='A'&&*sPtr<='F')
				byte|=(unsigned int)(*sPtr-'A'+10);
			else if(*sPtr>='a'&&*sPtr<='f')
				byte|=(unsigned int)(*sPtr-'a'+10);
			}
		btAddr[i]=(unsigned char)byte;
		
		/* Skip the ":" separator: */
		++sPtr;
		}
	
	return btAddr;
	}

bool restartBluetoothDaemon(void)
	{
	/* Set up message bus error handling: */
	DBusError error;
	dbus_error_init(&error);
	
	/* Connect to the system message bus: */
	DBusConnection* connection;
	connection=dbus_bus_get(DBUS_BUS_SYSTEM,&error);
	if(dbus_error_is_set(&error))
		{
		std::cerr<<"Error: Unable to connect to system message bus due to error "<<error.message<<std::endl;
		dbus_error_free(&error);
		return false;
		}
	else if(connection==0)
		{
		std::cerr<<"Error: Unable to connect to system message bus due to unknown error"<<std::endl;
		dbus_error_free(&error);
		return false;
		}
	#if 0 // Not required: DBUS automatically assigns a unique name for throw-away connections
	int result=dbus_bus_request_name(connection,"org.doc-ok.PSMoveUtil",DBUS_NAME_FLAG_REPLACE_EXISTING,&error);
	if(dbus_error_is_set(&error))
		{
		std::cerr<<"Error: Unable to connect to system message bus due to error "<<error.message<<std::endl;
		dbus_connection_unref(connection);
		dbus_error_free(&error);
		return false;
		}
	else if(result!=DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		{
		std::cerr<<"Error: Unable to connect to system message bus due to unknown error"<<std::endl;
		dbus_connection_unref(connection);
		dbus_error_free(&error);
		return false;
		}
	#endif
	
	/* Create a new method call object: */
	DBusMessage* message=dbus_message_new_method_call("org.freedesktop.systemd1","/org/freedesktop/systemd1","org.freedesktop.systemd1.Manager","ReloadOrRestartUnit");
	if(message==0)
		{
		std::cerr<<"Error: Unable to create system bus message"<<std::endl;
		dbus_connection_unref(connection);
		dbus_error_free(&error);
		return false;
		}
	
	/* Systemd unit name of the bluetooth daemon: */
	static const char* unitName="bluetooth.service";
	static const char* reloadMode="fail";
	
	/* Set method arguments: */
	DBusMessageIter arguments;
	dbus_message_iter_init_append(message,&arguments);
	if(!dbus_message_iter_append_basic(&arguments,DBUS_TYPE_STRING,&unitName)||!dbus_message_iter_append_basic(&arguments,DBUS_TYPE_STRING,&reloadMode))
		{
		std::cerr<<"Error: Unable to create system bus message"<<std::endl;
		dbus_message_unref(message);
		dbus_connection_unref(connection);
		dbus_error_free(&error);
		return false;
		}
	
	/* Send the message: */
	std::cout<<"Restarting bluetooth daemon..."<<std::flush;
	DBusPendingCall* pending=0;
	if(!dbus_connection_send_with_reply(connection,message,&pending,-1)||pending==0)
		{
		std::cout<<" failed"<<std::endl;
		std::cerr<<"Error: Unable to send system bus message"<<std::endl;
		dbus_message_unref(message);
		dbus_connection_unref(connection);
		dbus_error_free(&error);
		return false;
		}
	dbus_connection_flush(connection);
	
	/* Release the message object: */
	dbus_message_unref(message);
	
	/* Block until the message reply arrives: */
	dbus_pending_call_block(pending);
	
	/* Get the message reply: */
	message=dbus_pending_call_steal_reply(pending);
	if(message==0)
		{
		std::cout<<" failed"<<std::endl;
		std::cerr<<"Error: Unable to retrieve system bus message reply"<<std::endl;
		dbus_pending_call_unref(pending);
		dbus_connection_unref(connection);
		dbus_error_free(&error);
		return false;
		}
	
	/* Release the reply message object and the message completion object: */
	dbus_message_unref(message);
	dbus_pending_call_unref(pending);
	
	std::cout<<" OK"<<std::endl;
	
	/* Clean up and return: */
	dbus_connection_unref(connection);
	dbus_error_free(&error);
	return true;
	}

bool configureBluetoothDaemon(const unsigned char btHostAddr[6],const unsigned char btControllerAddr[6])
	{
	/* Find the bluetooth daemon's configuration directory: */
	std::string configDir="/var/lib/bluetooth";
	configDir.push_back('/');
	char btAddrString[18];
	configDir.append(btAddrToString(btHostAddr,btAddrString,true));
	
	/* Create a new configuration directory for the PS Move controller: */
	char psMoveName[18];
	btAddrToString(btControllerAddr,psMoveName,true);
	std::string psMoveDir=configDir;
	psMoveDir.push_back('/');
	psMoveDir.append(psMoveName);
	if(mkdir(psMoveDir.c_str(),S_IRWXU)!=0&&errno!=EEXIST)
		{
		int error=errno;
		std::cerr<<"Error: Unable to create PS Move configuration directory "<<psMoveDir<<" due to error "<<strerror(error)<<std::endl;
		return false;
		}
	
	/* Create an info file for the PS Move controller: */
	std::string infoFileName=psMoveDir;
	infoFileName.append("/info");
	{
	std::ofstream info(infoFileName.c_str(),std::ios_base::trunc);
	if(!info.is_open())
		{
		std::cerr<<"Error: Unable to create PS Move configuration file "<<infoFileName<<std::endl;
		return false;
		}
	info<<"[General]"<<std::endl;
	info<<"Name=Motion Controller"<<std::endl;
	info<<"Class=0x002508"<<std::endl;
	info<<"SupportedTechnologies=BR/EDR;"<<std::endl;
	info<<"Trusted=true"<<std::endl;
	info<<"Blocked=false"<<std::endl;
	info<<"Services=00001124-0000-1000-8000-00805f9b34fb;"<<std::endl;
	info<<"Alias=Playstation Move"<<std::endl;
	info<<std::endl;
	info<<"[DeviceID]"<<std::endl;
	info<<"Source=1"<<std::endl;
	info<<"Vendor=1356"<<std::endl;
	info<<"Product=981"<<std::endl;
	info<<"Version=1"<<std::endl;
	info.close();
	}
	
	/* Change the info file's mode: */
	if(chmod(infoFileName.c_str(),S_IRUSR|S_IWUSR)!=0)
		{
		int error=errno;
		std::cerr<<"Error: Unable to set permissions of PS Move configuration file "<<infoFileName<<" due to error "<<strerror(error)<<std::endl;
		return false;
		}
	
	/* Create a configuration cache directory if it does not exist yet: */
	std::string cacheDir=configDir;
	cacheDir.append("/cache");
	if(mkdir(cacheDir.c_str(),S_IRWXU)!=0&&errno!=EEXIST)
		{
		int error=errno;
		std::cerr<<"Error: Unable to create configuration cache directory "<<cacheDir<<" due to error "<<strerror(error)<<std::endl;
		return false;
		}
	
	/* Create a cache file for the PS Move controller: */
	std::string cacheFileName=cacheDir;
	cacheFileName.push_back('/');
	cacheFileName.append(psMoveName);
	{
	std::ofstream cache(cacheFileName.c_str(),std::ios_base::trunc);
	if(!cache.is_open())
		{
		std::cerr<<"Error: Unable to create PS Move configuration file "<<cacheFileName<<std::endl;
		return false;
		}
	cache<<"[General]"<<std::endl;
	cache<<"Name=Motion Controller"<<std::endl;
	cache<<""<<std::endl;
	cache<<"[ServiceRecords]"<<std::endl;
	cache<<"0x00010000=3601920900000A000100000900013503191124090004350D35061901000900113503190011090006350909656E09006A0901000900093508350619112409010009000D350F350D350619010009001335031900110901002513576972656C65737320436F6E74726F6C6C65720901012513576972656C65737320436F6E74726F6C6C6572090102251B536F6E7920436F6D707574657220456E7465727461696E6D656E740902000901000902010901000902020800090203082109020428010902052801090206359A35980822259405010904A101A102850175089501150026FF00810375019513150025013500450105091901291381027501950D0600FF8103150026FF0005010901A10075089504350046FF0009300931093209358102C0050175089527090181027508953009019102750895300901B102C0A1028502750895300901B102C0A10285EE750895300901B102C0A10285EF750895300901B102C0C0090207350835060904090901000902082800090209280109020A280109020B09010009020C093E8009020D280009020E2800"<<std::endl;
	cache.close();
	}
	
	/* Change the cache file's mode: */
	if(chmod(cacheFileName.c_str(),S_IRUSR|S_IWUSR)!=0)
		{
		int error=errno;
		std::cerr<<"Error: Unable to set permissions of PS Move configuration file "<<cacheFileName<<" due to error "<<strerror(error)<<std::endl;
		return false;
		}
	
	/* Restart the bluetooth daemon: */
	if(!restartBluetoothDaemon())
		return false;
	
	return true;
	}

const unsigned short psmoveVendorId=0x054c;
const unsigned short psmoveProductId=0x03d5;

bool configureUdev(const unsigned char btControllerAddr[6])
	{
	/* Create a udev rule file for the PS Move controller of the given bluetooth address: */
	std::string rulesFileName="/etc/udev/rules.d/69-PSMove-";
	char psMoveName[18];
	btAddrToString(btControllerAddr,psMoveName);
	rulesFileName.append(psMoveName);
	rulesFileName.append(".rules");
	std::cout<<"Creating UDev rules file "<<rulesFileName<<"..."<<std::flush;
	
	{
	std::ofstream rules(rulesFileName.c_str(),std::ios_base::trunc);
	if(!rules.is_open())
		{
		std::cout<<" failed"<<std::endl;
		std::cerr<<"Error: Unable to create PS Move UDev rules file "<<rulesFileName<<std::endl;
		return false;
		}
	rules<<"# Rule for Playstation Move motion controller connected via bluetooth"<<std::endl;
	rules<<"SUBSYSTEMS==\"bluetooth\", ATTRS{address}==\""<<psMoveName<<"\", TAG+=\"uaccess\""<<std::endl;
	rules.close();
	}
	
	std::cout<<" OK"<<std::endl;
	
	return true;
	}

void list(void)
	{
	/* Get the list of all USB devices: */
	USB::DeviceList deviceList;
	
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
		
		move.close();
		}
	}

void pair(USB::Device& move,const char* btHostAddrString)
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
	unsigned char btControllerAddr[6];
	memcpy(btControllerAddr,featureRequest+1,6);
	char btAddrString[18];
	std::cout<<"Controller bluetooth address:   "<<btAddrToString(btControllerAddr,btAddrString)<<std::endl;
	
	/* Print the PS Move's host bluetooth address: */
	std::cout<<"Current host bluetooth address: "<<btAddrToString(featureRequest+10,btAddrString)<<std::endl;
	
	/* Bail out if no new host address is provided: */
	if(btHostAddrString==0)
		return;
	
	/* Auto-detect bluetooth host controller's address if requested: */
	unsigned char btHostAddr[6];
	if(strcasecmp(btHostAddrString,"auto")==0)
		{
		/* Open a connection to the first bluetooth host controller: */
		int btDeviceId=hci_get_route(0);
		if(btDeviceId<0)
			{
			std::cerr<<"Error: Could not get handle to default bluetooth host controller"<<std::endl;
			return;
			}
		
		/* Query the controller's bluetooth address: */
		bdaddr_t addr;
		if(hci_devba(btDeviceId,&addr)>=0)
			memcpy(btHostAddr,addr.b,6);
		else
			{
			std::cerr<<"Error: Could not query default bluetooth host controller's address"<<std::endl;
			return;
			}
		}
	else
		{
		/* Convert the new host bluetooth address to binary: */
		stringToBtAddr(btHostAddrString,btHostAddr);
		}
	
	/* Print the new host bluetooth address: */
	std::cout<<"New host bluetooth address:     "<<btAddrToString(btHostAddr,btAddrString)<<std::endl;
	
	/* Check if the new host address is different from the current: */
	if(memcmp(featureRequest+10,btHostAddr,6)!=0)
		{
		std::cout<<"Configuring new bluetooth host address "<<btAddrToString(btHostAddr,btAddrString)<<std::endl;
		
		/* Assemble the feature request to set the PS Move's host bluetooth address: */
		memset(featureRequest,0x00U,sizeof(featureRequest));
		featureRequest[0]=0x05U;
		for(int i=0;i<6;++i)
			featureRequest[1+i]=btHostAddr[i];
		featureRequest[7]=0x10U;
		featureRequest[8]=0x01U;
		featureRequest[9]=0x02U;
		featureRequest[10]=0x12U;
		
		/* Send the HID feature request: */
		move.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,0x09U,(0x03U<<8)|featureRequest[0],0x0000U,featureRequest,11);
		}
	else
		std::cout<<"Bluetooth host address already configured"<<std::endl;
	
	{
	/* Add the PS Move device to the bluetooth daemon's configuration files: */
	std::cout<<"Do you want to add this PS Move to the bluetooth daemon's configuration (yes/no): "<<std::flush;
	std::string answer;
	std::cin>>answer;
	if(answer=="yes")
		configureBluetoothDaemon(btHostAddr,btControllerAddr);
	else
		std::cout<<"Bluetooth daemon not configured"<<std::endl;
	}
	
	{
	std::cout<<"Do you want to make this PS Move controller user-accessible (yes/no): "<<std::flush;
	std::string answer;
	std::cin>>answer;
	if(answer=="yes")
		configureUdev(btControllerAddr);
	else
		std::cout<<"UDev not configured"<<std::endl;
	}
	
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
		/* Parse the command line: */
		if(argc<2)
			{
			std::cerr<<"Missing command. Usage:"<<std::endl;
			std::cerr<<"PSMoveUtil list | ( pair ( <device index> | all ) [ <bluetooth host address> | auto ] ) | ( setLED <device index> <red> <green> <blue> )"<<std::endl;
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
			if(argc<3)
				{
				std::cerr<<"Malformed pair command. Usage:"<<std::endl;
				std::cerr<<"PSMoveUtil pair ( <device index> | all ) <bluetooth host address>"<<std::endl;
				return 1;
				}
			else if(strcasecmp(argv[2],"all")==0)
				{
				/* Get the list of all USB devices: */
				USB::DeviceList deviceList;
				
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
				USB::DeviceList deviceList;
				
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
			USB::DeviceList deviceList;
			
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
			std::cerr<<"PSMoveUtil list | ( pair ( <device index> | all ) [ <bluetooth host address> ] ) | ( setLED <device index> <red> <green> <blue> )"<<std::endl;
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
