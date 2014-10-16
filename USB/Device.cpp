/***********************************************************************
Device - Class representing a USB device and optionally a handle
resulting from opening the device.
Copyright (c) 2010-2014 Oliver Kreylos

This file is part of the USB Support Library (USB).

The USB Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The USB Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the USB Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <USB/Device.h>

#include <libusb-1.0/libusb.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>

namespace USB {

/***********************
Methods of class Device:
***********************/

Device::Device(libusb_device* sDevice)
	:device(sDevice),handle(0)
	{
	/* Reference the device: */
	if(device!=0)
		libusb_ref_device(device);
	}

Device::Device(const Device& source)
	:device(source.device),handle(0)
	{
	/* Reference the device: */
	if(device!=0)
		libusb_ref_device(device);
	}

Device& Device::operator=(libusb_device* sDevice)
	{
	if(device!=sDevice)
		{
		/* Close and unrefence the device: */
		if(handle!=0)
			close();
		if(device!=0)
			libusb_unref_device(device);
		
		/* Copy the device pointer and reset the handle: */
		device=sDevice;
		handle=0;
		
		/* Reference the device: */
		if(device!=0)
			libusb_ref_device(device);
		}
	return *this;
	}

Device& Device::operator=(const Device& source)
	{
	if(device!=source.device)
		{
		/* Close and unrefence the device: */
		if(handle!=0)
			close();
		if(device!=0)
			libusb_unref_device(device);
		
		/* Copy the device pointer and reset the handle: */
		device=source.device;
		handle=0;
		
		/* Reference the device: */
		if(device!=0)
			libusb_ref_device(device);
		}
	return *this;
	}

Device::~Device(void)
	{
	/* Close and unreference the device: */
	if(handle!=0)
		close();
	if(device!=0)
		libusb_unref_device(device);
	}

unsigned int Device::getBusNumber(void) const
	{
	return libusb_get_bus_number(device);
	}

unsigned int Device::getAddress(void) const
	{
	return libusb_get_device_address(device);
	}

int Device::getSpeedClass(void) const
	{
	return libusb_get_device_speed(device);
	}

libusb_device_descriptor Device::getDeviceDescriptor(void)
	{
	libusb_device_descriptor result;
	if(libusb_get_device_descriptor(device,&result)!=0)
		throw std::runtime_error("USB::Device::getDeviceDescriptor: Error while querying device descriptor");
	return result;
	}

VendorProductId Device::getVendorProductId(void)
	{
	libusb_device_descriptor dd;
	if(libusb_get_device_descriptor(device,&dd)==0)
		return VendorProductId(dd.idVendor,dd.idProduct);
	else
		throw std::runtime_error("USB::Device::getDeviceDescriptor: Error while querying device descriptor");
	}

std::string Device::getDescriptorString(unsigned int stringIndex)
	{
	/* Temporarily open the device if it is not open already: */
	bool tempOpen=handle==0;
	if(tempOpen)
		open();
	
	/* Retrieve the descriptor string: */
	unsigned char stringBuffer[256];
	int stringLength=libusb_get_string_descriptor_ascii(handle,stringIndex,stringBuffer,sizeof(stringBuffer));
	if(stringLength<0)
		{
		/* Close the device again if it wasn't open to begin with: */
		if(tempOpen)
			close();
		
		throw std::runtime_error("USB::Device::getDescriptorString: Error while querying descriptor string");
		}
	
	/* Close the device again if it wasn't open to begin with: */
	if(tempOpen)
		close();
	
	/* Return the descriptor string: */
	return std::string(stringBuffer,stringBuffer+stringLength);
	}

std::string Device::getSerialNumber(void)
	{
	/* Get the device descriptor: */
	libusb_device_descriptor dd;
	if(libusb_get_device_descriptor(device,&dd)!=0)
		throw std::runtime_error("USB::Device::getSerialNumber: Error while querying device descriptor");
	
	/* Check if the serial number is set: */
	if(dd.iSerialNumber==0)
		return "";
	
	/* Temporarily open the device if it is not open already: */
	bool tempOpen=handle==0;
	if(tempOpen)
		open();
	
	/* Retrieve the serial number string: */
	unsigned char stringBuffer[256];
	int stringLength=libusb_get_string_descriptor_ascii(handle,dd.iSerialNumber,stringBuffer,sizeof(stringBuffer));
	if(stringLength<0)
		{
		/* Close the device again if it wasn't open to begin with: */
		if(tempOpen)
			close();
		
		throw std::runtime_error("USB::Device::getSerialNumber: Error while querying serial number string");
		}
	
	/* Close the device again if it wasn't open to begin with: */
	if(tempOpen)
		close();
	
	/* Return the serial number: */
	return std::string(stringBuffer,stringBuffer+stringLength);
	}

libusb_config_descriptor* Device::getActiveConfigDescriptor(void)
	{
	libusb_config_descriptor* result;
	int getActiveConfigResult=libusb_get_active_config_descriptor(device,&result);
	switch(getActiveConfigResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			throw std::runtime_error("USB::Device::getActiveConfigDescriptor: Device is not configured");
		
		default:
			throw std::runtime_error("USB::Device::getActiveConfigDescriptor: Error while querying active configuration descriptor");
		}
	
	return result;
	}

libusb_config_descriptor* Device::getConfigDescriptorByIndex(unsigned int index)
	{
	libusb_config_descriptor* result;
	int getConfigResult=libusb_get_config_descriptor(device,index,&result);
	switch(getConfigResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			Misc::throwStdErr("USB::Device::getConfigDescriptorByIndex: Configuration of index %d does not exist",index);
		
		default:
			Misc::throwStdErr("USB::Device::getConfigDescriptorByIndex: Error while querying configuration descriptor of index %d",index);
		}
	
	return result;
	}

libusb_config_descriptor* Device::getConfigDescriptorByValue(unsigned int configurationValue)
	{
	libusb_config_descriptor* result;
	int getConfigResult=libusb_get_config_descriptor_by_value(device,configurationValue,&result);
	switch(getConfigResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			Misc::throwStdErr("USB::Device::getConfigDescriptorByValue: Configuration of value %d does not exist",configurationValue);
		
		default:
			Misc::throwStdErr("USB::Device::getConfigDescriptorByValue: Error while querying configuration descriptor of value %d",configurationValue);
		}
	
	return result;
	}

void Device::open(void)
	{
	/* Bail out if device is already open: */
	if(handle!=0)
		return;
	
	/* Open the device: */
	int openResult=libusb_open(device,&handle);
	switch(openResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_ACCESS:
			handle=0;
			throw std::runtime_error("USB::Device::open: Insufficient device permissions");
		
		case LIBUSB_ERROR_NO_DEVICE:
			handle=0;
			throw std::runtime_error("USB::Device::open: Device has been disconnected");
		
		default:
			handle=0;
			throw std::runtime_error("USB::Device::open: Error while opening device");
		}
	}

int Device::getConfiguration(void) const
	{
	int result;
	int getConfigurationResult=libusb_get_configuration(handle,&result);
	switch(getConfigurationResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NO_DEVICE:
			throw std::runtime_error("USB::Device::getConfiguration: Device has been disconnected");
		
		default:
			throw std::runtime_error("USB::Device::getConfiguration: Error while querying current configuration");
		}
	return result;
	}

void Device::setConfiguration(int newConfiguration)
	{
	int setConfigurationResult=libusb_set_configuration(handle,newConfiguration);
	switch(setConfigurationResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			Misc::throwStdErr("USB::Device::setConfiguration: Configuration %d does not exist on device",newConfiguration);
		
		case LIBUSB_ERROR_BUSY:
			throw std::runtime_error("USB::Device::setConfiguration: Device has claimed interfaces");
		
		case LIBUSB_ERROR_NO_DEVICE:
			throw std::runtime_error("USB::Device::setConfiguration: Device has been disconnected");
		
		default:
			Misc::throwStdErr("USB::Device::setConfiguration: Error while setting configuration %d",newConfiguration);
		}
	}

void Device::claimInterface(int interfaceNumber,bool detachKernelDriver)
	{
	/* Bail out if the interface is already claimed: */
	for(std::vector<ClaimedInterface>::iterator ciIt=claimedInterfaces.begin();ciIt!=claimedInterfaces.end();++ciIt)
		if(ciIt->interfaceNumber==interfaceNumber)
			return;
	
	/* Prepare a claim record: */
	ClaimedInterface ci;
	ci.interfaceNumber=interfaceNumber;
	ci.detachedKernelDriver=false;
	
	if(detachKernelDriver&&libusb_kernel_driver_active(handle,interfaceNumber)>0)
		{
		/* Detach a kernel driver: */
		int detachResult=libusb_detach_kernel_driver(handle,interfaceNumber);
		switch(detachResult)
			{
			case 0:
				break;
			
			case LIBUSB_ERROR_NOT_FOUND:
				Misc::throwStdErr("USB::Device::claimInterface: No kernel driver attached to interface %d",interfaceNumber);
			
			case LIBUSB_ERROR_INVALID_PARAM:
				Misc::throwStdErr("USB::Device::claimInterface: Interface %d does not exist",interfaceNumber);
			
			case LIBUSB_ERROR_NO_DEVICE:
				throw std::runtime_error("USB::Device::claimInterface: Device has been disconnected");
			
			default:
				Misc::throwStdErr("USB::Device::claimInterface: Error while detaching kernel driver from interface %d",interfaceNumber);
			}
		ci.detachedKernelDriver=true;
		}
	
	/* Claim the interface: */
	int claimResult=libusb_claim_interface(handle,interfaceNumber);
	switch(claimResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			Misc::throwStdErr("USB::Device::claimInterface: Interface %d does not exist",interfaceNumber);
		
		case LIBUSB_ERROR_BUSY:
			Misc::throwStdErr("USB::Device::claimInterface: Interface %d is already claimed",interfaceNumber);
		
		case LIBUSB_ERROR_NO_DEVICE:
			throw std::runtime_error("USB::Device::claimInterface: Device has been disconnected");
		
		default:
			Misc::throwStdErr("USB::Device::claimInterface: Error while claiming interface %d",interfaceNumber);
		}
	
	/* Append the claim record to the list: */
	claimedInterfaces.push_back(ci);
	}

void Device::setAlternateSetting(int interfaceNumber,int alternateSettingNumber)
	{
	int setAltSettingResult=libusb_set_interface_alt_setting(handle,interfaceNumber,alternateSettingNumber);
	switch(setAltSettingResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			Misc::throwStdErr("USB::Device::setAlternateSetting: Interface %d does not have alternate setting %d",interfaceNumber,alternateSettingNumber);
		
		case LIBUSB_ERROR_NO_DEVICE:
			throw std::runtime_error("USB::Device::setAlternateSetting: Device has been disconnected");
		
		default:
			Misc::throwStdErr("USB::Device::setAlternateSetting: Error while setting alternate setting %d for interface %d",alternateSettingNumber,interfaceNumber);
		}
	}

void Device::writeControl(unsigned int requestType,unsigned int request,unsigned int value,unsigned int index,const unsigned char* data,size_t dataSize,unsigned int timeOut)
	{
	/* Issue the request: */
	int transferResult=libusb_control_transfer(handle,requestType&~0x80U,request,value,index,const_cast<unsigned char*>(data),dataSize,timeOut);
	if(transferResult<0)
		{
		switch(transferResult)
			{
			case LIBUSB_ERROR_TIMEOUT:
				throw std::runtime_error("USB::Device::writeControl: Timeout during write");
			
			case LIBUSB_ERROR_PIPE:
				Misc::throwStdErr("USB::Device::writeControl: Unsupported control request %d",int(request));
			
			case LIBUSB_ERROR_NO_DEVICE:
				throw std::runtime_error("USB::Device::writeControl: Device has been disconnected");
			
			default:
				throw std::runtime_error("USB::Device::writeControl: Error while writing");
			}
		}
	else if(size_t(transferResult)!=dataSize)
		Misc::throwStdErr("USB::Device::writeControl: Overflow during write; sent %d bytes instead of %d",transferResult,int(dataSize));
	}

size_t Device::readControl(unsigned int requestType,unsigned int request,unsigned int value,unsigned int index,unsigned char* data,size_t maxDataSize,unsigned int timeOut)
	{
	/* Issue the request: */
	int transferResult=libusb_control_transfer(handle,requestType|0x80U,request,value,index,data,maxDataSize,timeOut);
	if(transferResult<0)
		{
		switch(transferResult)
			{
			case LIBUSB_ERROR_TIMEOUT:
				throw std::runtime_error("USB::Device::readControl: Timeout during read");
			
			case LIBUSB_ERROR_PIPE:
				Misc::throwStdErr("USB::Device::readControl: Unsupported control request %d",int(request));
			
			case LIBUSB_ERROR_NO_DEVICE:
				throw std::runtime_error("USB::Device::readControl: Device has been disconnected");
			
			default:
				throw std::runtime_error("USB::Device::readControl: Error while reading");
			}
		}
	
	return size_t(transferResult);
	}

size_t Device::interruptTransfer(unsigned char endpoint,unsigned char* data,size_t dataSize,unsigned int timeOut)
	{
	/* Issue the request: */
	int transferred=0;
	int transferResult=libusb_interrupt_transfer(handle,endpoint,data,int(dataSize),&transferred,timeOut);
	if(transferResult<0&&transferResult!=LIBUSB_ERROR_TIMEOUT)
		{
		switch(transferResult)
			{
			case LIBUSB_ERROR_PIPE:
				Misc::throwStdErr("USB::Device::interruptTransfer: Endpoint %u is halted",(unsigned int)endpoint);
			
			case LIBUSB_ERROR_OVERFLOW:
				Misc::throwStdErr("USB::Device::interruptTransfer: Overflow on endpoint %u",(unsigned int)endpoint);
			
			case LIBUSB_ERROR_NO_DEVICE:
				throw std::runtime_error("USB::Device::interruptTransfer: Device has been disconnected");
			
			default:
				Misc::throwStdErr("USB::Device::interruptTransfer: Error %d during interrupt transfer on endpoint %u",transferResult,(unsigned int)endpoint);
			}
		}
	
	return size_t(transferred);
	}

size_t Device::bulkTransfer(unsigned char endpoint,unsigned char* data,size_t dataSize,unsigned int timeOut)
	{
	/* Issue the request: */
	int transferred=0;
	int transferResult=libusb_bulk_transfer(handle,endpoint,data,int(dataSize),&transferred,timeOut);
	if(transferResult<0&&transferResult!=LIBUSB_ERROR_TIMEOUT)
		{
		switch(transferResult)
			{
			case LIBUSB_ERROR_PIPE:
				Misc::throwStdErr("USB::Device::bulkTransfer: Endpoint %u is halted",(unsigned int)endpoint);
			
			case LIBUSB_ERROR_OVERFLOW:
				Misc::throwStdErr("USB::Device::bulkTransfer: Overflow on endpoint %u",(unsigned int)endpoint);
			
			case LIBUSB_ERROR_NO_DEVICE:
				throw std::runtime_error("USB::Device::bulkTransfer: Device has been disconnected");
			
			default:
				Misc::throwStdErr("USB::Device::bulkTransfer: Error %d during bulk transfer on endpoint %u",transferResult,(unsigned int)endpoint);
			}
		}
	
	return size_t(transferred);
	}

void Device::releaseInterface(int interfaceNumber)
	{
	/* Find the interface's claim record: */
	std::vector<ClaimedInterface>::iterator claimIt=claimedInterfaces.end();
	for(std::vector<ClaimedInterface>::iterator ciIt=claimedInterfaces.begin();ciIt!=claimedInterfaces.end();++ciIt)
		if(ciIt->interfaceNumber==interfaceNumber)
			claimIt=ciIt;
	
	/* Bail out if the interface was not actually claimed: */
	if(claimIt==claimedInterfaces.end())
		return;
	
	/* Remove the claim record (even if release fails, the interface is still released): */
	bool detachedKernelDriver=claimIt->detachedKernelDriver;
	claimedInterfaces.erase(claimIt);
	
	/* Release the interface: */
	int releaseResult=libusb_release_interface(handle,interfaceNumber);
	switch(releaseResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			Misc::throwStdErr("USB::Device::releaseInterface: Interface %d does not exist or was not claimed",interfaceNumber);
		
		case LIBUSB_ERROR_NO_DEVICE:
			throw std::runtime_error("USB::Device::releaseInterface: Device has been disconnected");
		
		default:
			Misc::throwStdErr("USB::Device::releaseInterface: Error while releasing interface %d",interfaceNumber);
		}
	if(detachedKernelDriver)
		if(libusb_attach_kernel_driver(handle,interfaceNumber)!=0)
			{
			/* This can theoretically not fail; if it does, we're fucked: */
			Misc::throwStdErr("USB::Device::releaseInterface: Error while reattaching kernel driver to interface %d",interfaceNumber);
			}
	}

bool Device::reset(void)
	{
	int resetResult=libusb_reset_device(handle);
	switch(resetResult)
		{
		case 0:
			break;
		
		case LIBUSB_ERROR_NOT_FOUND:
			return true;
		
		default:
			throw std::runtime_error("USB::Device::reset: Error while resetting device");
		}
	
	return false;
	}

void Device::close(void)
	{
	/* Bail out if device is already closed: */
	if(handle==0)
		return;
	
	/* Release all still-claimed interfaces: */
	for(std::vector<ClaimedInterface>::iterator ciIt=claimedInterfaces.begin();ciIt!=claimedInterfaces.end();++ciIt)
		{
		libusb_release_interface(handle,ciIt->interfaceNumber);
		if(ciIt->detachedKernelDriver)
			libusb_attach_kernel_driver(handle,ciIt->interfaceNumber);
		}
	claimedInterfaces.clear();
	
	/* Close the device: */
	libusb_close(handle);
	handle=0;
	}

}
