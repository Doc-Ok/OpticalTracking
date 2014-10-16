/***********************************************************************
OculusRiftDK2VideoDevice - Custom subclass of V4L2VideoDevice to correct
the DK2's tracking cameras quirks, and allow for LED strobe
synchronization.
Copyright (c) 2014 Oliver Kreylos
***********************************************************************/

#include <Video/Linux/OculusRiftDK2VideoDevice.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/usb/video.h>
#include <linux/uvcvideo.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <Misc/SizedTypes.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextFieldSlider.h>
#include <Video/ImageExtractor.h>
#include <Video/BayerPattern.h>
#include <Video/ImageExtractorBA81.h>

namespace Video {

namespace {

/**************
Helper classes:
**************/

class ImageExtractorY8:public ImageExtractor // Custom image extractor class to treat the camera's IR image as straight 8-bit values
	{
	/* Elements: */
	private:
	unsigned int size[2]; // Frame width and height
	
	/* Constructors and destructors: */
	public:
	ImageExtractorY8(const unsigned int sSize[2]); // Constructs an extractor for the given frame size
	
	/* Methods from ImageExtractor: */
	virtual void extractGrey(const FrameBuffer* frame,void* image);
	virtual void extractRGB(const FrameBuffer* frame,void* image);
	virtual void extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride);
	};

/*********************************
Methods of class ImageExtractorY8:
*********************************/

ImageExtractorY8::ImageExtractorY8(const unsigned int sSize[2])
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	}

void ImageExtractorY8::extractGrey(const FrameBuffer* frame,void* image)
	{
	/* Do a straight-up copy: */
	memcpy(image,frame->start,size[1]*size[0]*sizeof(unsigned char));
	}

void ImageExtractorY8::extractRGB(const FrameBuffer* frame,void* image)
	{
	/* Convert pixels to RBG: */
	const unsigned char* fPtr=frame->start;
	unsigned char* iPtr=static_cast<unsigned char*>(image);
	for(unsigned int y=0;y<size[1];++y)
		for(unsigned int x=0;x<size[0];++x,++fPtr,iPtr+=3)
			iPtr[2]=iPtr[1]=iPtr[0]=*fPtr;
	}

void ImageExtractorY8::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Copy pixels directly to Y': */
	const unsigned char* fPtr=frame->start;
	unsigned char* yPtr=static_cast<unsigned char*>(yp);
	for(unsigned int y=0;y<size[1];++y)
		for(unsigned int x=0;x<size[0];++x,++fPtr,++yPtr)
			*yPtr=*fPtr;
	
	/* Reset the Cb and Cr planes to zero: */
	unsigned char* cbRowPtr=static_cast<unsigned char*>(cb);
	unsigned char* crRowPtr=static_cast<unsigned char*>(cr);
	for(unsigned int y=0;y<size[1];y+=2,cbRowPtr+=cbStride,crRowPtr+=crStride)
		{
		/* Reset the two planes' pixel row to zero: */
		memset(cbRowPtr,0,size[0]/2);
		memset(crRowPtr,0,size[0]/2);
		}
	}

}

/***************************************************
Methods of class OculusRiftDK2VideoDevice::DeviceId:
***************************************************/

VideoDevice* OculusRiftDK2VideoDevice::DeviceId::createDevice(void) const
	{
	return new OculusRiftDK2VideoDevice(deviceFileName.c_str());
	}

namespace {

class MT9V034 // Helper class to communicate with the Oculus Rift DK2 camera's imaging sensor
	{
	/* Embedded classes: */
	public:
	enum Registers // Enumerated type for registers on the imaging sensor
		{
		ChipVersion=0x00U,
		ColumnStart=0x01U,
		RowStart=0x02U,
		WindowHeight=0x03U,
		WindowWidth=0x04U,
		HorizontalBlanking=0x05U,
		VerticalBlanking=0x06U,
		ChipControl=0x07U,
		CoarseShutterWidthTotal=0x0bU,
		ReadMode=0x0dU,
		LEDOutControl=0x1bU,
		Companding=0x1cU,
		AnalogGain=0x35U,
		BlacklevelCalibControl=0x47U,
		BlacklevelCalibValue=0x48U,
		BlacklevelCalibStepSize=0x4cU,
		MaxAnalogGain=0xabU,
		AecMinimumExposure=0xacU,
		AecMaximumExposure=0xadU,
		AecAgcControl=0xafU,
		MaxTotalShutterWidth=0xbdU,
		FineShutterWidthTotal=0xd5
		};
	enum ChipControl // Enumerated type for chip control flags
		{
		ChipControlScanModeMask=0x0003U,
		ChipControlScanModeProgressive=0x0000U,
		ChipControlScanModeInterlacedTwoField=0x0002U,
		ChipControlScanModeInterlacedOneField=0x0003U,
		ChipControlModeMask=0x0018U,
		ChipControlModeSlave=0x0000U,
		ChipControlModeMaster=0x0008U,
		ChipControlModeSnapshot=0x0018U,
		ChipControlParallelOutEnable=0x0080U,
		ChipControlReadoutMask=0x0100U,
		ChipControlReadoutSequential=0x0000U,
		ChipControlReadoutSimultaneous=0x0100U
		};
	enum ReadMode // Enumerated type for read modes
		{
		ReadModeRowBinMask=0x0003U,
		ReadModeRowBin1=0x0000U,
		ReadModeRowBin2=0x0001U,
		ReadModeRowBin4=0x0002U,
		ReadModeColumnBinMask=0x000cU,
		ReadModeColumnBin1=0x0000U,
		ReadModeColumnBin2=0x0004U,
		ReadModeColumnBin4=0x0008U,
		ReadModeRowFlip=0x0010U,
		ReadModeColumnFlip=0x0020U,
		ReadModeShowDarkRows=0x0040U,
		ReadModeShowDarkColumns=0x0080U,
		ReadModeReserved=0x0300U
		};
	enum LEDOutControl // Enumerated type for LED control modes
		{
		LEDOutControlDisable=0x0001U,
		LEDOutControlInvert=0x0002U
		};
	enum BlacklevelCalibControl // Enumerated type for blacklevel control modes
		{
		BlacklevelCalibControlOverride=0x0001U,
		BlacklevelCalibControlFilterLengthMask=0x00e0U,
		BlacklevelCalibControlFilterLengthShift=5
		};
	enum AecAgcControl // Enumerated type for automatic exposure/gain control
		{
		AecAgcControlAecMask=0x0001U,
		AecAgcControlAecDisable=0x0000U,
		AecAgcControlAecEnable=0x0001U,
		AecAgcControlAgcMask=0x0002U,
		AecAgcControlAgcDisable=0x0000U,
		AecAgcControlAgcEnable=0x0002U
		};
	enum Constants // Enumerated type for imaging sensor constants
		{
		Address=0x98U,
		ESP570SelectorI2C=2,
		ESP570SelectorUnkown=3,
		ESP570SelectorEEPROM=5,
		ESP570ExtensionUnitID=4
		};
	
	/* Elements: */
	private:
	int fd; // File handle of the V4L2 device node
	
	/* Private methods: */
	void uvcSetGetCurrent(Misc::UInt8 selector,Misc::UInt8* packet,size_t packetSize)
		{
		/* Set up the UVC control query: */
		uvc_xu_control_query query;
		memset(&query,0,sizeof(query));
		query.unit=ESP570ExtensionUnitID;
		query.selector=selector;
		query.size=packetSize;
		query.data=packet;
		
		int result;
		
		/* Send a set current request: */
		query.query=UVC_SET_CUR;
		result=ioctl(fd,UVCIOC_CTRL_QUERY,&query);
		if(result<0)
			throw std::runtime_error("Video::MT9V034::uvcSetGetCurrent: Error during UVC_SET_CUR request");
		
		/* Send a get current request: */
		query.query=UVC_GET_CUR;
		result=ioctl(fd,UVCIOC_CTRL_QUERY,&query);
		if(result<0)
			throw std::runtime_error("Video::MT9V034::uvcSetGetCurrent: Error during UVC_GET_CUR request");
		}
	
	/* Constructors and destructors: */
	public:
	MT9V034(int sFd) // Creates a wrapper for an imaging sensor on the given V4L2 device node
		:fd(sFd)
		{
		}
	
	/* Methods: */
	Misc::UInt16 read(Misc::UInt8 registerIndex) // Reads a value from the given register
		{
		/* Read a 16-bit value via the sensor controller's I2C bus: */
		Misc::UInt8 packet[6];
		packet[0]=0x04U|0x80U;
		packet[1]=Address;
		packet[2]=registerIndex;
		packet[5]=packet[4]=packet[3]=0x00U;
		
		/* Perform the I2C operation: */
		uvcSetGetCurrent(ESP570SelectorI2C,packet,sizeof(packet));
		
		/* Check the returned packet for validity: */
		if(packet[0]!=0x84U||packet[4]!=0x00U||packet[5]!=0x00U)
			throw std::runtime_error("Video::MT9V034::read: I2C communication error");
		
		/* Return the result value: */
		return (Misc::UInt16(packet[1])<<8)|Misc::UInt16(packet[2]);
		}
	void write(Misc::UInt8 address,Misc::UInt8 registerIndex,Misc::UInt16 value) // Writes a value to the given register on the given I2C address
		{
		/* Write a 16-bit value via the sensor controller's I2C bus: */
		Misc::UInt8 packet[6];
		packet[0]=0x04U;
		packet[1]=address;
		packet[2]=registerIndex;
		packet[3]=Misc::UInt8(value>>8);
		packet[4]=Misc::UInt8(value&0xffU);
		packet[5]=0x00U;
		
		/* Perform the I2C operation: */
		uvcSetGetCurrent(ESP570SelectorI2C,packet,sizeof(packet));
		
		/* Check the returned packet for validity: */
		if(packet[0]!=0x04U||packet[1]!=Address||packet[2]!=registerIndex||packet[5]!=0x00U)
			throw std::runtime_error("Video::MT9V034::write: I2C communication error");
		}
	void write(Misc::UInt8 registerIndex,Misc::UInt16 value) // Writes a value to the given register on the imaging sensor's I2C address
		{
		/* Call the full function: */
		write(Address,registerIndex,value);
		}
	void readEeprom(Misc::UInt16 address,Misc::UInt8 size,Misc::UInt8* buffer) // Reads a block from the controller's EEPROM
		{
		if(size>32)
			throw std::runtime_error("Video::MT9V034::readEeprom: Block size larger than 32");
		
		/* Prepare the request data packet: */
		Misc::UInt8 packet[59];
		memset(packet,0,sizeof(packet));
		packet[0]=0x82U;
		packet[1]=0xa0U;
		packet[2]=Misc::UInt8(address>>8);
		packet[3]=Misc::UInt8(address&0xffU);
		packet[4]=size;
		
		/* Perform the EEPROM operation: */
		uvcSetGetCurrent(ESP570SelectorEEPROM,packet,sizeof(packet));
		
		/* Check the returned packet for validity: */
		if(packet[0]!=0x82U||packet[1]!=size)
			throw std::runtime_error("Video::MT9V034::readEeprom: I2C communication error");
		
		/* Copy the returned data: */
		memcpy(buffer,packet+2,size);
		}
	void setupSequence(void) // Performs a setup sequence of unknown meaning
		{
		Misc::UInt8 packet[3];
		
		/* Set up the first data packet: */
		packet[0]=0x80U;
		packet[1]=0x14U;
		packet[2]=0x00U;
		
		/* Perform the I2C operation: */
		uvcSetGetCurrent(ESP570SelectorUnkown,packet,sizeof(packet));
		
		/* Check the returned packet for validity: */
		if(packet[0]!=0x80U||packet[1]!=0xdcU||packet[2]!=0x00U)
			throw std::runtime_error("Video::MT9V034::setupSequence: I2C communication error during first exchange");
		
		/* Set up the second data packet, this might determine the address of the MT9V034 imaging sensor: */
		packet[0]=0xa0U;
		packet[1]=0xf0U;
		packet[2]=0x00U;
		
		/* Perform the I2C operation: */
		uvcSetGetCurrent(ESP570SelectorUnkown,packet,sizeof(packet));
		
		/* Check the returned packet for validity: */
		if(packet[0]!=0xa0U||packet[1]!=0x98U||packet[2]!=0x00U)
			throw std::runtime_error("Video::MT9V034::setupSequence: I2C communication error during second exchange");
		}
	};

}

/*****************************************
Methods of class OculusRiftDK2VideoDevice:
*****************************************/

void OculusRiftDK2VideoDevice::getControlValues(void)
	{
	/* Open an I2C connection to the imaging sensor: */
	MT9V034 sensor(videoFd);
	
	/* Read the imaging sensor's chip version: */
	Misc::UInt16 chipVersion=sensor.read(MT9V034::ChipVersion);
	if(chipVersion!=0x1324U)
		throw std::runtime_error("Video::OculusRiftDK2VideoDevice::getControlValues: Unsupported imaging sensor version");
	
	/* Get current imaging sensor parameters: */
	horizontalBlanking=sensor.read(MT9V034::HorizontalBlanking);
	verticalBlanking=sensor.read(MT9V034::VerticalBlanking);
	unsigned int chipControl=sensor.read(MT9V034::ChipControl);
	synched=(chipControl&MT9V034::ChipControlModeMask)==MT9V034::ChipControlModeSnapshot;
	coarseShutterWidth=sensor.read(MT9V034::CoarseShutterWidthTotal);
	unsigned int readMode=sensor.read(MT9V034::ReadMode);
	flipColumns=readMode&MT9V034::ReadModeColumnFlip;
	flipRows=readMode&MT9V034::ReadModeRowFlip;
	analogGain=sensor.read(MT9V034::AnalogGain);
	unsigned int blacklevelCalibControl=sensor.read(MT9V034::BlacklevelCalibControl);
	automaticBlacklevelCalibration=!(blacklevelCalibControl&MT9V034::BlacklevelCalibControlOverride);
	blacklevelCalibrationValue=Misc::SInt8(sensor.read(MT9V034::BlacklevelCalibValue));
	unsigned int aecAgcControl=sensor.read(MT9V034::AecAgcControl);
	automaticExposureGain=aecAgcControl&(MT9V034::AecAgcControlAecMask|MT9V034::AecAgcControlAgcMask);
	fineShutterWidth=sensor.read(MT9V034::FineShutterWidthTotal);
	}

void OculusRiftDK2VideoDevice::setControlValues(void)
	{
	/* Open an I2C connection to the imaging sensor: */
	MT9V034 sensor(videoFd);
	
	/* Read the imaging sensor's chip version: */
	Misc::UInt16 chipVersion=sensor.read(MT9V034::ChipVersion);
	if(chipVersion!=0x1324U)
		throw std::runtime_error("Video::OculusRiftDK2VideoDevice::setControlValues: Unsupported imaging sensor version");
	
	/* Set current imaging sensor parameters: */
	sensor.write(MT9V034::HorizontalBlanking,horizontalBlanking);
	sensor.write(MT9V034::VerticalBlanking,verticalBlanking);
	unsigned int chipControl=MT9V034::ChipControlParallelOutEnable|MT9V034::ChipControlReadoutSimultaneous;
	if(synched)
		chipControl|=MT9V034::ChipControlModeSnapshot;
	else
		chipControl|=MT9V034::ChipControlModeMaster;
	sensor.write(MT9V034::ChipControl,chipControl);
	sensor.write(MT9V034::CoarseShutterWidthTotal,coarseShutterWidth);
	unsigned int readMode=MT9V034::ReadModeReserved;
	if(flipColumns)
		readMode|=MT9V034::ReadModeColumnFlip;
	if(flipRows)
		readMode|=MT9V034::ReadModeRowFlip;
	sensor.write(MT9V034::ReadMode,readMode);
	sensor.write(MT9V034::AnalogGain,analogGain);
	sensor.write(MT9V034::BlacklevelCalibControl,automaticBlacklevelCalibration?((4<<MT9V034::BlacklevelCalibControlFilterLengthShift)&MT9V034::BlacklevelCalibControlFilterLengthMask):MT9V034::BlacklevelCalibControlOverride);
	sensor.write(MT9V034::BlacklevelCalibValue,blacklevelCalibrationValue); // Is actually a Misc::SInt8
	sensor.write(MT9V034::AecAgcControl,automaticExposureGain?(MT9V034::AecAgcControlAecEnable|MT9V034::AecAgcControlAgcEnable):(MT9V034::AecAgcControlAecDisable|MT9V034::AecAgcControlAgcDisable));
	sensor.write(MT9V034::FineShutterWidthTotal,fineShutterWidth);
	}

void OculusRiftDK2VideoDevice::textFieldSliderCallback(Misc::CallbackData* cbData,const int& index)
	{
	GLMotif::TextFieldSlider::ValueChangedCallbackData* myCbData=static_cast<GLMotif::TextFieldSlider::ValueChangedCallbackData*>(cbData);
	
	/* Open an I2C connection to the camera's imaging sensor: */
	MT9V034 sensor(videoFd);
	
	/* Handle the control event: */
	switch(index)
		{
		case 0: // Horizontal blank
			sensor.write(MT9V034::HorizontalBlanking,Misc::UInt16(myCbData->value+0.5));
			break;
		
		case 1: // Vertical blank
			sensor.write(MT9V034::VerticalBlanking,Misc::UInt16(myCbData->value+0.5));
			break;
		
		case 2: // Gain
			sensor.write(MT9V034::AnalogGain,Misc::UInt16(myCbData->value+0.5));
			break;
		
		case 3: // Coarse shutter
			sensor.write(MT9V034::CoarseShutterWidthTotal,Misc::UInt16(myCbData->value+0.5));
			break;
		
		case 4: // Fine shutter
			sensor.write(MT9V034::FineShutterWidthTotal,Misc::UInt16(myCbData->value+0.5));
			break;
		
		case 5: // Black level value
			{
			sensor.write(MT9V034::BlacklevelCalibValue,Misc::UInt16(Misc::SInt8(Math::floor(myCbData->value+0.5))));
			break;
			}
		}
	}

void OculusRiftDK2VideoDevice::toggleButtonCallback(Misc::CallbackData* cbData,const int& index)
	{
	GLMotif::ToggleButton::ValueChangedCallbackData* myCbData=static_cast<GLMotif::ToggleButton::ValueChangedCallbackData*>(cbData);
	
	/* Open an I2C connection to the camera's imaging sensor: */
	MT9V034 sensor(videoFd);
	
	/* Handle the control event: */
	switch(index)
		{
		case 0: // Horizontal flipping
			{
			Misc::UInt16 readMode=sensor.read(MT9V034::ReadMode);
			if(myCbData->set)
				readMode|=MT9V034::ReadModeColumnFlip;
			else
				readMode&=~MT9V034::ReadModeColumnFlip;
			sensor.write(MT9V034::ReadMode,readMode);
			break;
			}
		
		case 1: // Vertical flipping
			{
			Misc::UInt16 readMode=sensor.read(MT9V034::ReadMode);
			if(myCbData->set)
				readMode|=MT9V034::ReadModeRowFlip;
			else
				readMode&=~MT9V034::ReadModeRowFlip;
			sensor.write(MT9V034::ReadMode,readMode);
			break;
			}
		
		case 2: // Automatic gain and exposure control
			sensor.write(MT9V034::AecAgcControl,myCbData->set?(MT9V034::AecAgcControlAecEnable|MT9V034::AecAgcControlAgcEnable):(MT9V034::AecAgcControlAecDisable|MT9V034::AecAgcControlAgcDisable));
			break;
		
		case 3: // Automatic black level
			sensor.write(MT9V034::BlacklevelCalibControl,myCbData->set?0x80U:0x01U);
			break;
		
		case 4: // Synchronization
			if(myCbData->set)
				sensor.write(MT9V034::ChipControl,MT9V034::ChipControlModeSnapshot|MT9V034::ChipControlParallelOutEnable|MT9V034::ChipControlReadoutSimultaneous);
			else
				sensor.write(MT9V034::ChipControl,MT9V034::ChipControlModeMaster|MT9V034::ChipControlParallelOutEnable|MT9V034::ChipControlReadoutSimultaneous);
			break;
		}
	}

OculusRiftDK2VideoDevice::OculusRiftDK2VideoDevice(const char* videoDeviceName)
	:V4L2VideoDevice(videoDeviceName),
	 colorMode(false),
	 horizontalBlanking(94),verticalBlanking(5),
	 synched(true),
	 coarseShutterWidth(11),
	 flipColumns(true),flipRows(false),
	 analogGain(16),
	 automaticBlacklevelCalibration(false),blacklevelCalibrationValue(-127),
	 automaticExposureGain(false),
	 fineShutterWidth(111)
	{
	/* Run the setup sequence of unknown meaning: */
	MT9V034 sensor(videoFd);
	sensor.setupSequence();
	
	#if 0
	
	/* Read the camera version and serial number string: */
	Misc::UInt8 buffer[33];
	sensor.readEeprom(0x0ff0U,16U,buffer);
	buffer[16]='\0';
	std::cout<<"Camera version: "<<buffer<<std::endl;
	
	sensor.readEeprom(0x2800U,32U,buffer);
	buffer[32]='\0';
	std::cout<<"Camera serial number: "<<buffer<<std::endl;
	
	#endif
	}

OculusRiftDK2VideoDevice::~OculusRiftDK2VideoDevice(void)
	{
	}

std::vector<VideoDataFormat> OculusRiftDK2VideoDevice::getVideoFormatList(void) const
	{
	std::vector<VideoDataFormat> result;
	
	/* Return the DK2 camera's only four video formats: */
	VideoDataFormat format;
	format.setPixelFormat("Y8  ");
	format.size[0]=752;
	format.size[1]=480;
	format.lineSize=format.size[0];
	format.frameSize=format.size[1]*format.lineSize;
	
	/* Add the 30Hz greyscale video format: */
	format.frameIntervalCounter=1;
	format.frameIntervalCounter=30;
	result.push_back(format);
	
	/* Add the 30Hz Bayer-encoded color video format: */
	format.setPixelFormat("BGGR");
	result.push_back(format);
	
	/* Add the 60Hz Bayer-encoded color video format: */
	format.frameIntervalCounter=1;
	format.frameIntervalCounter=60;
	result.push_back(format);
	
	/* Add the 60Hz greyscale video format: */
	format.setPixelFormat("Y8  ");
	result.push_back(format);
	
	return result;
	}

VideoDataFormat OculusRiftDK2VideoDevice::getVideoFormat(void) const
	{
	/* Get the camera's advertised current video format: */
	VideoDataFormat result=V4L2VideoDevice::getVideoFormat();
	
	/* Adjust the pixel format and resolution: */
	result.setPixelFormat(colorMode?"BGGR":"Y8  ");
	result.size[0]*=2;
	
	return result;
	}

VideoDataFormat& OculusRiftDK2VideoDevice::setVideoFormat(VideoDataFormat& newFormat)
	{
	/* Check if the requested format uses a color pixel format: */
	colorMode=newFormat.isPixelFormat("BGGR");
	
	/* Adjust the given format's pixel format and resolution: */
	newFormat.setPixelFormat("YUYV");
	newFormat.size[0]/=2;
	
	/* Set the format in the real camera: */
	V4L2VideoDevice::setVideoFormat(newFormat);
	
	/* Adjust the pixel format and resolution: */
	newFormat.setPixelFormat(colorMode?"BGGR":"Y8  ");
	newFormat.size[0]*=2;
	
	return newFormat;
	}

ImageExtractor* OculusRiftDK2VideoDevice::createImageExtractor(void) const
	{
	unsigned int frameSize[2]={752,480};
	
	if(colorMode)
		{
		/* Create an extractor for 8-bit Bayer-encoded RGB pixels: */
		return new ImageExtractorBA81(frameSize,BAYER_BGGR);
		}
	else
		{
		/* Create an extractor for 8-bit greyscale pixels: */
		return new ImageExtractorY8(frameSize);
		}
	}

GLMotif::Widget* OculusRiftDK2VideoDevice::createControlPanel(GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the control panel dialog window: */
	GLMotif::PopupWindow* controlPanelPopup=new GLMotif::PopupWindow("V4L2VideoDeviceControlPanelPopup",widgetManager,"Video Source Control");
	controlPanelPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* controlPanel=new GLMotif::RowColumn("ControlPanel",controlPanelPopup,false);
	controlPanel->setNumMinorWidgets(2);
	
	new GLMotif::Label("HblankLabel",controlPanel,"Horz Blank");
	
	GLMotif::TextFieldSlider* hblankSlider=new GLMotif::TextFieldSlider("HblankSlider",controlPanel,6,ss->fontHeight*10.0f);
	hblankSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	hblankSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	hblankSlider->setValueRange(61.0,1000.0,1.0);
	hblankSlider->setValue(horizontalBlanking);
	hblankSlider->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::textFieldSliderCallback,0);
	
	new GLMotif::Label("VblankLabel",controlPanel,"Vert Blank");
	
	GLMotif::TextFieldSlider* vblankSlider=new GLMotif::TextFieldSlider("VblankSlider",controlPanel,6,ss->fontHeight*10.0f);
	vblankSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	vblankSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	vblankSlider->setValueRange(5.0,1000.0,1.0);
	vblankSlider->setValue(verticalBlanking);
	vblankSlider->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::textFieldSliderCallback,1);
	
	new GLMotif::Label("GainLabel",controlPanel,"Gain");
	
	GLMotif::TextFieldSlider* gainSlider=new GLMotif::TextFieldSlider("GainSlider",controlPanel,6,ss->fontHeight*10.0f);
	gainSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	gainSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	gainSlider->setValueRange(16.0,64.0,1.0);
	gainSlider->setValue(analogGain);
	gainSlider->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::textFieldSliderCallback,2);
	
	new GLMotif::Label("CoarseShutterLabel",controlPanel,"Coarse Shutter");
	
	GLMotif::TextFieldSlider* coarseShutterSlider=new GLMotif::TextFieldSlider("CoarseShutterSlider",controlPanel,6,ss->fontHeight*10.0f);
	coarseShutterSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	coarseShutterSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	coarseShutterSlider->setValueRange(1.0,480.0,1.0);
	coarseShutterSlider->setValue(coarseShutterWidth);
	coarseShutterSlider->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::textFieldSliderCallback,3);
	
	new GLMotif::Label("FineShutterLabel",controlPanel,"Fine Shutter");
	
	GLMotif::TextFieldSlider* fineShutterSlider=new GLMotif::TextFieldSlider("FineShutterSlider",controlPanel,6,ss->fontHeight*10.0f);
	fineShutterSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	fineShutterSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	fineShutterSlider->setValueRange(0.0,1774.0,1.0);
	fineShutterSlider->setValue(fineShutterWidth);
	fineShutterSlider->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::textFieldSliderCallback,4);
	
	new GLMotif::Label("FlippingLabel",controlPanel,"Flipping");
	
	GLMotif::Margin* flippingMargin=new GLMotif::Margin("FlippingMargin",controlPanel,false);
	flippingMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	GLMotif::RowColumn* flippingBox=new GLMotif::RowColumn("FlippingBox",flippingMargin,false);
	flippingBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	flippingBox->setNumMinorWidgets(1);
	
	GLMotif::ToggleButton* hflippingToggle=new GLMotif::ToggleButton("HflippingToggle",flippingBox,"Horizontal");
	hflippingToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	hflippingToggle->setToggle(flipColumns);
	hflippingToggle->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::toggleButtonCallback,0);
	
	GLMotif::ToggleButton* vflippingToggle=new GLMotif::ToggleButton("VflippingToggle",flippingBox,"Vertical");
	vflippingToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	vflippingToggle->setToggle(flipRows);
	vflippingToggle->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::toggleButtonCallback,1);
	
	flippingBox->manageChild();
	
	flippingMargin->manageChild();
	
	new GLMotif::Label("AutomaticGainControlLabel",controlPanel,"Automatic Exposure/Gain");
	
	GLMotif::Margin* automaticGainControlMargin=new GLMotif::Margin("AutomaticGainControlMargin",controlPanel,false);
	automaticGainControlMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	GLMotif::ToggleButton* automaticGainControlToggle=new GLMotif::ToggleButton("AutomaticGainControlToggle",automaticGainControlMargin,"Enable");
	automaticGainControlToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	automaticGainControlToggle->setToggle(automaticExposureGain);
	automaticGainControlToggle->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::toggleButtonCallback,2);
	
	automaticGainControlMargin->manageChild();
	
	new GLMotif::Label("AutomaticBlackLevelLabel",controlPanel,"Automatic Black Level");
	
	GLMotif::Margin* automaticBlackLevelMargin=new GLMotif::Margin("AutomaticBlackLevelMargin",controlPanel,false);
	automaticBlackLevelMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	GLMotif::ToggleButton* automaticBlackLevelToggle=new GLMotif::ToggleButton("AutomaticBlackLevelToggle",automaticBlackLevelMargin,"Enable");
	automaticBlackLevelToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	automaticBlackLevelToggle->setToggle(automaticBlacklevelCalibration);
	automaticBlackLevelToggle->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::toggleButtonCallback,3);
	
	automaticBlackLevelMargin->manageChild();
	
	new GLMotif::Label("BlackLevelValueLabel",controlPanel,"Black Level Value");
	
	GLMotif::TextFieldSlider* blackLevelValueSlider=new GLMotif::TextFieldSlider("BlackLevelValueSlider",controlPanel,6,ss->fontHeight*10.0f);
	blackLevelValueSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	blackLevelValueSlider->setValueType(GLMotif::TextFieldSlider::INT);
	blackLevelValueSlider->setValueRange(-128.0,127.0,1.0);
	blackLevelValueSlider->setValue(blacklevelCalibrationValue);
	blackLevelValueSlider->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::textFieldSliderCallback,5);
	
	new GLMotif::Label("SynchLabel",controlPanel,"Synch");
	
	GLMotif::Margin* synchMargin=new GLMotif::Margin("SynchMargin",controlPanel,false);
	synchMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	GLMotif::ToggleButton* synchToggle=new GLMotif::ToggleButton("SynchToggle",synchMargin,"Enable");
	synchToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	synchToggle->setToggle(synched);
	synchToggle->getValueChangedCallbacks().add(this,&OculusRiftDK2VideoDevice::toggleButtonCallback,4);
	
	synchMargin->manageChild();
	
	controlPanel->manageChild();
	
	return controlPanelPopup;
	}

void OculusRiftDK2VideoDevice::startStreaming(void)
	{
	V4L2VideoDevice::startStreaming();
	
	/* Download the current control values from the imaging sensor: */
	getControlValues();
	}

void OculusRiftDK2VideoDevice::startStreaming(VideoDevice::StreamingCallback* newStreamingCallback)
	{
	V4L2VideoDevice::startStreaming(newStreamingCallback);
	
	/* Download the current control values from the imaging sensor: */
	getControlValues();
	}

void OculusRiftDK2VideoDevice::stopStreaming(void)
	{
	V4L2VideoDevice::stopStreaming();
	}

void OculusRiftDK2VideoDevice::enumerateDevices(std::vector<VideoDevice::DeviceIdPtr>& devices)
	{
	/* Enumerate all /dev/videoXXX device file nodes: */
	for(int videoDeviceIndex=0;;++videoDeviceIndex)
		{
		/* Create the device file name: */
		char deviceFileName[40];
		snprintf(deviceFileName,sizeof(deviceFileName),"/dev/video%d",videoDeviceIndex);
		
		/* Try opening the device file: */
		int videoFd=open(deviceFileName,O_RDWR); // Read/write access is required, even for capture only!
		if(videoFd<0)
			{
			if(errno==ENOENT) // Device node doesn't exist
				break;
			else
				continue; // Some other error
			}
		
		/* Check if the device can capture video in streaming mode: */
		v4l2_capability videoCap;
		if(ioctl(videoFd,VIDIOC_QUERYCAP,&videoCap)==0)
			if((videoCap.capabilities&V4L2_CAP_VIDEO_CAPTURE)!=0&&(videoCap.capabilities&V4L2_CAP_STREAMING)!=0)
				{
				/* Query the device's name: */
				std::string name(reinterpret_cast<const char*>(videoCap.card));
				if(name=="Camera DK2")
					{
					/* Create a device ID: */
					DeviceId* newDeviceId=new DeviceId(name);
					
					/* Store the device file name: */
					newDeviceId->deviceFileName=deviceFileName;
					
					/* Store the device ID: */
					devices.push_back(newDeviceId);
					}
				}
		
		/* Close the device file: */
		close(videoFd);
		}
	}

void OculusRiftDK2VideoDevice::setTrackingMode(bool enabled)
	{
	if(enabled)
		{
		/* Open an I2C connection to the imaging sensor: */
		MT9V034 sensor(videoFd);

		/* Override control values with those optimal for DK2 tracking: */
		sensor.write(MT9V034::HorizontalBlanking,94);
		sensor.write(MT9V034::VerticalBlanking,5);
		sensor.write(MT9V034::ChipControl,MT9V034::ChipControlParallelOutEnable|MT9V034::ChipControlReadoutSimultaneous|MT9V034::ChipControlModeSnapshot);
		sensor.write(MT9V034::CoarseShutterWidthTotal,11);
		sensor.write(MT9V034::ReadMode,MT9V034::ReadModeReserved|MT9V034::ReadModeColumnFlip);
		sensor.write(MT9V034::AnalogGain,16);
		sensor.write(MT9V034::BlacklevelCalibControl,MT9V034::BlacklevelCalibControlOverride);
		sensor.write(MT9V034::BlacklevelCalibValue,0x80); // Is actually a Misc::SInt8
		sensor.write(MT9V034::AecAgcControl,MT9V034::AecAgcControlAecDisable|MT9V034::AecAgcControlAgcDisable);
		sensor.write(MT9V034::FineShutterWidthTotal,111);
		
		/* Invert the output LED control; not sure what this does: */
		sensor.write(MT9V034::LEDOutControl,MT9V034::LEDOutControlInvert);
		
		/* Execute unknown command sequence (both lock up on execution, must investigate further): */
		// sensor.write(0x60U,0x05U,0x0001U);
		// sensor.write(0x60U,0x06U,0x0020U);
		}
	else
		{
		/* Upload the current control values to the imaging sensor: */
		setControlValues();
		}
	}

}
