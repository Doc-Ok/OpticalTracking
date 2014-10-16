/***********************************************************************
HIDDevice - VR device driver class for generic input devices supported
by the MacOS X HID event interface. Reports buttons and absolute axes.
Copyright (c) 2006-2010 Braden Pellett, Oliver Kreylos

Portions of this code were adapted from examples given on Apple's
developer website, and can be found at the following URL:
http://developer.apple.com/documentation/DeviceDrivers/Conceptual/HID/

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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/errno.h>
#include <sysexits.h>

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDKeys.h>

#include <CoreFoundation/CoreFoundation.h>


/*
 * Static Members
 */

unsigned HIDDevice::queueDepth ( 32 ) ;

/*
 * Public Methods
 */

HIDDevice::HIDDevice ( VRDevice::Factory* sFactory,
                       VRDeviceManager* sDeviceManager,
                       Misc::ConfigurationFile& configFile )
	: VRDevice ( sFactory, sDeviceManager, configFile ),
	  hidDeviceInterface ( NULL ),
	  hidQueueInterface ( NULL ),
	  cfRunLoop ( NULL )
{
	// Get the desired HID device
	io_object_t hidDevice ( getHIDDevice ( configFile ) ) ;

	// Create a device interface to the found HID device
	hidDeviceInterface = createHIDDeviceInterface ( hidDevice ) ;
	IOObjectRelease ( hidDevice ) ;
	
	// Setup the button and axis maps
	setupButtonAndAxisMaps ( configFile ) ;

	// Set number of trackers, buttons, and valuators on device
	setNumTrackers ( 0, configFile ) ;
	setNumButtons ( buttonMap . size ( ), configFile ) ;
	setNumValuators ( absAxisMap . size ( ), configFile ) ;

	// Open the HID device
	if ( (*hidDeviceInterface) -> open ( hidDeviceInterface, 0 ) != kIOReturnSuccess )
		Misc::throwStdErr ( "HIDDevice: Unable to open HID device" ) ;

	// Setup the event queue
	setupEventQueue ( ) ;
}

HIDDevice::~HIDDevice ( void )
{
	{
		CFRunLoopStop ( cfRunLoop ) ;
		Threads::Mutex::Lock stateLock ( runLoopMutex ) ;
		stopDeviceThread ( ) ;
	}

	(*hidQueueInterface) -> stop ( hidQueueInterface ) ;
	(*hidQueueInterface) -> dispose ( hidQueueInterface ) ;
	(*hidQueueInterface) -> Release ( hidQueueInterface ) ;

	(*hidDeviceInterface) -> close ( hidDeviceInterface ) ;
	(*hidDeviceInterface) -> Release ( hidDeviceInterface ) ;
}

void HIDDevice::deviceThreadMethod ( void )
{
	Threads::Mutex::Lock stateLock ( runLoopMutex ) ;
	cfRunLoop = CFRunLoopGetCurrent ( ) ;
	CFRunLoopAddSource ( cfRunLoop, eventSource, kCFRunLoopDefaultMode ) ;
	CFRunLoopRun ( ) ;
	cfRunLoop = NULL ;
}

void HIDDevice::start(void)
{
	if ( ! cfRunLoop )
	{
		// Initialize button and valuator states to the device's current values
		IOHIDEventStruct hidEvent;
		for ( CookieIndexMap::iterator pos ( buttonMap . begin ( ) ) ;
				pos != buttonMap . end ( ) ; ++pos )
		{
			if ( (*hidDeviceInterface) -> getElementValue ( hidDeviceInterface, pos -> first, &hidEvent )
					== kIOReturnSuccess )
			{
				setButtonState ( pos -> second, hidEvent . value ) ;
			}
		}
		for ( CookieAxisInfoMap::iterator pos ( absAxisMap . begin ( ) ) ;
				pos != absAxisMap . end ( ) ; ++pos )
		{
			if ( (*hidDeviceInterface) ->
				 getElementValue ( hidDeviceInterface, pos -> first, &hidEvent ) == kIOReturnSuccess )
			{
				setValuatorState ( pos -> second . index,
				                   pos -> second . converter . map ( hidEvent . value ) ) ;
			}
		}
		updateState ( ) ;
	
		if ( (*hidQueueInterface) -> start ( hidQueueInterface ) != kIOReturnSuccess )
			Misc::throwStdErr ( "HIDDevice: Unable to start HID device event queue" ) ;
	
		startDeviceThread ( ) ;
	}
}

void HIDDevice::stop(void)
{
	if ( cfRunLoop ) {
		CFRunLoopStop ( cfRunLoop ) ;
		Threads::Mutex::Lock stateLock ( runLoopMutex ) ;
		(*hidQueueInterface) -> stop ( hidQueueInterface ) ;
		stopDeviceThread ( ) ;
	}
}

/*
 * Private Methods
 */

io_object_t HIDDevice::findHIDDeviceByVendorIdAndProductId ( int targetVendorId, int targetProductId )
{
	CFMutableDictionaryRef subDictionary (
		CFDictionaryCreateMutable ( kCFAllocatorDefault, 0,
                                    &kCFTypeDictionaryKeyCallBacks,
                                    &kCFTypeDictionaryValueCallBacks ) ) ;
	CFNumberRef targetVendorIdValue (
		CFNumberCreate ( kCFAllocatorDefault, kCFNumberIntType, &targetVendorId ) ) ;
	CFNumberRef targetProductIdValue (
		CFNumberCreate ( kCFAllocatorDefault, kCFNumberIntType, &targetProductId ) ) ;
	CFDictionarySetValue ( subDictionary, CFSTR ( kIOHIDVendorIDKey ), targetVendorIdValue ) ;
	CFDictionarySetValue ( subDictionary, CFSTR ( kIOHIDProductIDKey ), targetProductIdValue ) ;
	
	CFMutableDictionaryRef matchingDictionary ( IOServiceMatching ( kIOHIDDeviceKey ) ) ;
	CFDictionarySetValue ( matchingDictionary, CFSTR ( kIOPropertyMatchKey ), subDictionary ) ;

	CFRelease ( targetProductIdValue ) ;
	CFRelease ( targetVendorIdValue ) ;
	CFRelease ( subDictionary ) ;

	return IOServiceGetMatchingService ( kIOMasterPortDefault, matchingDictionary ) ;
}

io_object_t HIDDevice::findHIDDeviceByName ( const char* targetDeviceName )
{
	CFMutableDictionaryRef subDictionary (
		CFDictionaryCreateMutable ( kCFAllocatorDefault, 0,
                                    &kCFTypeDictionaryKeyCallBacks,
                                    &kCFTypeDictionaryValueCallBacks ) ) ;
	CFStringRef targetDeviceNameValue (
		CFStringCreateWithCString (	kCFAllocatorDefault, targetDeviceName, kCFStringEncodingASCII ) ) ;
	CFDictionarySetValue ( subDictionary, CFSTR ( kIOHIDProductKey ), targetDeviceNameValue ) ;
	
	CFMutableDictionaryRef matchingDictionary ( IOServiceMatching ( kIOHIDDeviceKey ) ) ;
	CFDictionarySetValue ( matchingDictionary, CFSTR ( kIOPropertyMatchKey ), subDictionary ) ;

	CFRelease ( targetDeviceNameValue ) ;
	CFRelease ( subDictionary ) ;
	
	return IOServiceGetMatchingService ( kIOMasterPortDefault, matchingDictionary ) ;
}

io_object_t HIDDevice::getHIDDevice ( Misc::ConfigurationFile& configFile )
{
	io_object_t device ( NULL );

	// First option - Open device by vendor ID / product ID
	if ( ! device )
	{
		std::string deviceVendorProductId ( configFile . retrieveString ( "./deviceVendorProductId", "" ) ) ;
		if ( deviceVendorProductId != "" )
		{
			// Split ID string into vendor ID and product ID
			char* colonPtr ;
			int vendorId ( strtol ( deviceVendorProductId . c_str ( ) , &colonPtr , 16 ) ) ;
			char* endPtr ;
			int productId ( strtol ( colonPtr + 1, &endPtr, 16 ) ) ;
			if ( *colonPtr != ':' || *endPtr != '\0' || vendorId < 0 || productId < 0 )
				Misc::throwStdErr ( "HIDDevice: Malformed vendorId:productId string \"%s\"",
				                    deviceVendorProductId.c_str ( ) ) ;
			#ifdef VERBOSE
			printf ( "HIDDevice: Searching for device %04x:%04x\n", vendorId, productId ) ;
			fflush ( stdout ) ;
			#endif
			device = findHIDDeviceByVendorIdAndProductId ( vendorId, productId ) ;
			if ( ! device )
				Misc::throwStdErr ( "HIDDevice: No device with vendorId:productId %04x:%04x found",
				                    vendorId, productId ) ;
		}
	}

	// Second option - Open device by device name
	if ( ! device )
	{
		std::string deviceName ( configFile . retrieveString ( "./deviceName", "" ) ) ;
		if ( deviceName != "" )
		{
			#ifdef VERBOSE
			printf ( "HIDDevice: Searching for device \"%s\"\n", deviceName . c_str ( ) ) ;
			fflush ( stdout ) ;
			#endif
			device = findHIDDeviceByName ( deviceName . c_str ( ) ) ;
			if ( ! device )
				Misc::throwStdErr ( "HIDDevice: No device with name \"%s\" found",
				                    deviceName . c_str ( ) ) ;
		}
	}

	// Third option - Open device by explicit event device file name
	// NOTE: This is not supported on Mac OS X
	if ( ! device )
	{
		std::string deviceFileName ( configFile . retrieveString ( "./deviceFileName", "" ) ) ;
		if ( deviceFileName != "" )
		{
			Misc::throwStdErr ( "HIDDevice: Unable to open device file \"%s\"",
			                    deviceFileName . c_str ( ) ) ;
		}
	}

	// Bail out if no device was found
	if ( ! device )
		Misc::throwStdErr ( "HIDDevice: No device specified" ) ;

	return device ;
}

IOHIDDeviceInterface** HIDDevice::createHIDDeviceInterface ( io_object_t hidDevice )
{
	IOHIDDeviceInterface** hidDeviceInterface ( NULL ) ;
	IOCFPlugInInterface** plugInInterface ( NULL ) ;
	SInt32 score ( 0 ) ;
	IOReturn ioReturnValue ( kIOReturnSuccess ) ;

	ioReturnValue = IOCreatePlugInInterfaceForService ( hidDevice,
														kIOHIDDeviceUserClientTypeID,
														kIOCFPlugInInterfaceID,
														&plugInInterface,
														&score ) ;

	if ( ioReturnValue == kIOReturnSuccess )
	{
		// Call a method of the intermediate plug-in to create the device 
		// interface
		HRESULT plugInResult (
			(*plugInInterface) -> QueryInterface ( plugInInterface,
												   CFUUIDGetUUIDBytes ( kIOHIDDeviceInterfaceID ),
												   (LPVOID*) (&hidDeviceInterface) ) ) ;
		if ( plugInResult != S_OK )
			hidDeviceInterface = NULL ;

		(*plugInInterface) -> Release ( plugInInterface ) ;
	} else {
		printf ( "error: 0x%x\n", ioReturnValue ) ;
	}

	if ( ! hidDeviceInterface )
		Misc::throwStdErr ( "HIDDevice: Unable to create HID device interface" ) ;

	return hidDeviceInterface ;
}

void HIDDevice::setupButtonAndAxisMaps ( Misc::ConfigurationFile& configFile )
{
	IOHIDDeviceInterface122** handle ( (IOHIDDeviceInterface122**) hidDeviceInterface ) ;
	CFArrayRef elements;
	IOReturn success;

	#ifdef VERBOSE
	printf ( "HIDDevice: Setting up button and axis maps\n" ) ;
	fflush ( stdout ) ;
	#endif

	success = (*handle) -> copyMatchingElements ( handle, NULL, &elements ) ;

	if ( success != kIOReturnSuccess )
		Misc::throwStdErr ( "HIDDevice: copyMatchingElements failed with error %d\n", success ) ;

	typedef std::multimap < long, IOHIDElementCookie > usageMap ;
	usageMap buttonCookies ;
	usageMap axisCookies ;

	#ifdef VERBOSE
	printf ( "HIDDevice: Output type elements found --\n" );
	printf ( "HIDDevice:    Usage Page   Usage    Cookie   Min   Max   Use?\n" );
	#endif
	
	for ( CFIndex i ( 0 ) ; i < CFArrayGetCount ( elements ) ; i++ )
	{
		CFTypeRef object ;
		CFDictionaryRef element ( (CFDictionaryRef) CFArrayGetValueAtIndex ( elements, i ) ) ;

		// Get element type
		object = CFDictionaryGetValue ( element, CFSTR ( kIOHIDElementTypeKey ) ) ;
		if ( object == 0 || CFGetTypeID ( object ) != CFNumberGetTypeID ( ) )
			continue ;
		IOHIDElementType elementType ;
		if ( ! CFNumberGetValue ( (CFNumberRef) object, kCFNumberLongType, &elementType ) )
			continue ;
		// Skip the element if it isn't a button or valuator
		if ( ! ( elementType == kIOHIDElementTypeInput_Misc ||
		         elementType == kIOHIDElementTypeInput_Axis || 
		         elementType == kIOHIDElementTypeInput_Button ||
		         elementType == kIOHIDElementTypeInput_ScanCodes ) )
			continue ;

		// Get cookie
		object = CFDictionaryGetValue ( element, CFSTR ( kIOHIDElementCookieKey ) ) ;
		if ( object == 0 || CFGetTypeID ( object ) != CFNumberGetTypeID ( ) )
			continue ;
		IOHIDElementCookie cookie ;
		if ( ! CFNumberGetValue ( (CFNumberRef) object, kCFNumberLongType, &cookie ) )
			continue ;

		// Get usage
		object = CFDictionaryGetValue ( element, CFSTR ( kIOHIDElementUsageKey ) ) ;
		if ( object == 0 || CFGetTypeID ( object ) != CFNumberGetTypeID ( ) )
			continue ;
		long usage ;
		if ( ! CFNumberGetValue ( (CFNumberRef) object, kCFNumberLongType, &usage ) )
			continue ;

		// Get usage page
		object = CFDictionaryGetValue ( element, CFSTR ( kIOHIDElementUsagePageKey ) ) ;
		if ( object == 0 || CFGetTypeID ( object ) != CFNumberGetTypeID ( ) )
			continue ;
		long usagePage ;
		if ( ! CFNumberGetValue ( (CFNumberRef) object, kCFNumberLongType, &usagePage ) )
			continue ;

		#ifdef VERBOSE
		printf ( "HIDDevice:    0x%-4X       0x%-4X   0x%-4X   ", (unsigned int)usagePage, (unsigned int)usage, (unsigned int)cookie ) ;
		fflush ( stdout ) ;
		#endif

		// Check for a button
		if ( usagePage == 0x09 && usage > 0x0 )
		{
			buttonCookies . insert ( std::make_pair ( usage, cookie ) ) ;
			#ifdef VERBOSE
			printf ( "            *\n" ) ;
			#endif
		}
		// Check for an axis
		else if ( usagePage == 0x01 && usage >= 0x30 && usage <= 0x39 )
		{
			axisCookies . insert ( std::make_pair ( usage, cookie ) ) ;
			// Set up the converter for the axis
			AxisConverter& converter ( absAxisMap [ cookie ] . converter ) ;
			long min ( 0 ) ;
			object = CFDictionaryGetValue ( element, CFSTR ( kIOHIDElementMinKey ) ) ;
			if ( object == 0 ||
			     CFGetTypeID ( object ) != CFNumberGetTypeID ( ) ||
			     ! CFNumberGetValue ( (CFNumberRef) object, kCFNumberLongType, &min ) )
			{
				min = 0 ;
				#ifdef VERBOSE
				printf ( "n/a   " ) ;
			}
			else
			{
				printf ( "%-6d", int(min) ) ;
				#endif
			}
			long max ( 1 ) ;
			object = CFDictionaryGetValue ( element, CFSTR ( kIOHIDElementMaxKey ) ) ;
			if ( object == 0 ||
			     CFGetTypeID ( object ) != CFNumberGetTypeID ( ) ||
			     ! CFNumberGetValue ( (CFNumberRef) object, kCFNumberLongType, &max ) )
			{
				max = 1 ;
				#ifdef VERBOSE
				printf ( "n/a   *\n" ) ;
			}
			else
			{
				printf ( "%-6d*\n", int(max) ) ;
				#endif
			}
			converter=AxisConverter(min,max,0.0f);
		}
		#ifdef VERBOSE
		else
		{
			printf ( "\n" ) ;
		}
		#endif
	}
	
	CFRelease ( elements ) ;

	// Map buttons to button indicies
	int index ( 0 ) ;
	for ( usageMap::iterator pos ( buttonCookies . begin ( ) ) ;
			pos != buttonCookies . end ( ) ; ++pos, ++index )
	{
		buttonMap [ pos -> second ] = index ;
	}

	#ifdef VERBOSE
		printf ( "HIDDevice: Axis setup --\n" ) ;
		printf ( "HIDDevice:    Axis  Usage  Cookie    NegMax   NegMin   NegRange   PosMin   PosMax   PosRange\n" ) ;
	#endif
	
	// Map axes to axis indicies
	index = 0 ;
	for ( usageMap::iterator pos ( axisCookies . begin ( ) ) ;
			pos != axisCookies . end ( ) ; ++pos, ++index )
	{
		absAxisMap [ pos -> second ] . index = index ;
		// Now that we have the index, see if the config file specifies axis settings.
		// If so, set up the converter from those settings.
		char absAxisSettingsTag [20] ;
		snprintf ( absAxisSettingsTag, sizeof ( absAxisSettingsTag ), "axis%dSettings", index ) ;
		AxisConverter& converter=absAxisMap[pos->second].converter;
		converter=configFile.retrieveValue<AxisConverter>(absAxisSettingsTag,converter);
		#ifdef VERBOSE
			printf("HIDDevice:    %-5d 0x%-4X 0x%-4X %s\n",int(index),(unsigned int)pos->first,(unsigned int)pos->second,Misc::ValueCoder<AxisConverter>::encode(converter).c_str());
		#endif
	}
}

void HIDDevice::setupEventQueue ( void )
{
	#ifdef VERBOSE
	printf ( "HIDDevice: Setting up the event queue\n" ) ;
	fflush ( stdout ) ;
	#endif

	hidQueueInterface = (*hidDeviceInterface) -> allocQueue ( hidDeviceInterface ) ;
	if ( ! hidQueueInterface )
		Misc::throwStdErr ( "HIDDevice: Unable to allocate HID device event queue" ) ;

	if ( (*hidQueueInterface) -> create ( hidQueueInterface, 0, queueDepth ) != kIOReturnSuccess )
		Misc::throwStdErr ( "HIDDevice: Unable to create HID device event queue" ) ;

	if ( (*hidQueueInterface) -> createAsyncEventSource ( hidQueueInterface, &eventSource) != kIOReturnSuccess )
		Misc::throwStdErr ( "HIDDevice: Unable to create an async event source" ) ;

	if ( (*hidQueueInterface) -> setEventCallout ( hidQueueInterface, HIDDevice::queueCallbackFunction,
	                                               this, NULL ) )
		Misc::throwStdErr ( "HIDDevice: Unable to set the event callout" ) ;

	for ( CookieIndexMap::iterator pos ( buttonMap . begin ( ) ) ;
			pos != buttonMap . end ( ) ; ++pos )
	{
		(*hidQueueInterface) -> addElement ( hidQueueInterface, pos -> first, 0 ) ;
	}

	for ( CookieAxisInfoMap::iterator pos ( absAxisMap . begin ( ) ) ;
			pos != absAxisMap . end ( ) ; ++pos )
	{
		(*hidQueueInterface) -> addElement ( hidQueueInterface, pos -> first, 0 ) ;
	}
}

void HIDDevice::handleEvents ( void )
{
	static const AbsoluteTime zeroTime = {0,0} ;
	IOReturn result ;
	IOHIDEventStruct hidEvent;
	bool changed ( false ) ;

	while ( ( result = (*hidQueueInterface) -> getNextEvent ( hidQueueInterface, &hidEvent, zeroTime, 0 ) )
				== kIOReturnSuccess )
	{
		CookieIndexMap::iterator buttonPos ( buttonMap . find ( hidEvent . elementCookie ) ) ;
		if ( buttonPos != buttonMap . end ( ) ) {
			setButtonState ( buttonPos -> second, hidEvent . value ) ;
			changed = true ;
			continue ;
		}
		CookieAxisInfoMap::iterator axisPos ( absAxisMap . find ( hidEvent . elementCookie ) ) ;
		if ( axisPos != absAxisMap . end ( ) ) {
			setValuatorState ( axisPos -> second . index,
			                   axisPos -> second . converter . map ( hidEvent . value ) ) ;
			changed = true ;
			continue ;
		}
	}

	// If events have been processed, mark manager state as complete
	if ( changed ) {
		updateState ( ) ;
		changed = false ;
	}

	if ( result != kIOReturnUnderrun )
		Misc::throwStdErr ( "HIDDevice: event queue error ( 0x%x )", result ) ;
}

/*
 * Static Private Methods
 */

void HIDDevice::queueCallbackFunction ( void* target, IOReturn result, void* refcon, void* sender )
{
	if ( result != kIOReturnSuccess )
		Misc::throwStdErr ( "HIDDevice: event queue callback error ( 0x%x )", result ) ;

	if ( target )
		static_cast < HIDDevice* > ( target ) -> handleEvents ( ) ;
}
