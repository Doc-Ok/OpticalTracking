/***********************************************************************
VideoViewer - A simple viewer for live video from a video source
connected to the local computer.
Copyright (c) 2013-2014 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/FunctionCalls.h>
#include <Misc/Timer.h>
#include <Threads/TripleBuffer.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <Images/RGBImage.h>
#include <Images/WriteImageFile.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Button.h>
#include <Video/VideoDataFormat.h>
#include <Video/VideoDevice.h>
#include <Video/ImageExtractor.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>

class VideoViewer:public Vrui::Application,public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint videoTextureId; // ID of image texture object
		bool haveNpotdt; // Flag whether OpenGL supports non-power-of-two dimension textures
		GLfloat texMin[2],texMax[2]; // Texture coordinate rectangle to render the image texture (to account for power-of-two only textures)
		unsigned int videoTextureVersion; // Version number of the video frame in the video texture
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	Video::VideoDevice* videoDevice; // Pointer to the video recording device
	Video::VideoDataFormat videoFormat; // Configured video format of the video device
	Video::ImageExtractor* videoExtractor; // Helper object to convert video frames to RGB
	volatile bool saveVideoFrames; // Flag to save video frames to disk as they arrive
	Misc::Timer saveVideoTimer; // A free-running timer to time-stamp saved video frames
	std::string saveVideoFrameNameTemplate; // Printf-style template to save video frames
	unsigned int saveVideoNextFrameIndex; // Index for the next video frame to be saved
	Threads::TripleBuffer<Images::RGBImage> videoFrames; // Triple buffer to pass video frames from the video callback to the main loop
	unsigned int videoFrameVersion; // Version number of the most recent video frame in the triple buffer
	GLMotif::Widget* videoControlPanel; // The video device's control panel
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	void videoFrameCallback(const Video::FrameBuffer* frameBuffer); // Callback receiving incoming video frames
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	void resetNavigationCallback(Misc::CallbackData* cbData); // Method to reset the Vrui navigation transformation to its default
	void showControlPanelCallback(Misc::CallbackData* cbData); // Method to pop up the video device's control panel
	
	/* Constructors and destructors: */
	public:
	VideoViewer(int& argc,char**& argv);
	virtual ~VideoViewer(void);
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

/**************************************
Methods of class VideoViewer::DataItem:
**************************************/

VideoViewer::DataItem::DataItem(void)
	:videoTextureId(0)
	{
	glGenTextures(1,&videoTextureId);
	}

VideoViewer::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&videoTextureId);
	}

/****************************
Methods of class VideoViewer:
****************************/

void VideoViewer::videoFrameCallback(const Video::FrameBuffer* frameBuffer)
	{
	double timeStamp=saveVideoTimer.peekTime();
	
	/* Start a new value in the input triple buffer: */
	Images::RGBImage& image=videoFrames.startNewValue();
	
	/* Extract an RGB image from the provided frame buffer into the new value: */
	videoExtractor->extractRGB(frameBuffer,image.modifyPixels());
	
	/* Finish the new value in the triple buffer and wake up the main loop: */
	videoFrames.postNewValue();
	Vrui::requestUpdate();
	
	if(saveVideoFrames)
		{
		/* Create a filename for the new video frame: */
		char videoFrameFileName[1024];
		snprintf(videoFrameFileName,sizeof(videoFrameFileName),saveVideoFrameNameTemplate.c_str(),saveVideoNextFrameIndex);
		
		/* Save the new video frame: */
		Images::writeImageFile(image,videoFrameFileName);
		
		std::cout<<"Saving frame "<<videoFrameFileName<<" at "<<timeStamp*1000.0<<" ms"<<std::endl;
		
		/* Increment the frame counter: */
		++saveVideoNextFrameIndex;
		}
	}

GLMotif::PopupMenu* VideoViewer::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Video Viewer");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create a button to reset navigation back to full-frame view: */
	GLMotif::Button* resetNavigationButton=new GLMotif::Button("ResetNavigationButton",mainMenu,"Reset Navigation");
	resetNavigationButton->getSelectCallbacks().add(this,&VideoViewer::resetNavigationCallback);
	
	if(videoControlPanel!=0)
		{
		/* Create a button to pop up the video control panel: */
		GLMotif::Button* showControlPanelButton=new GLMotif::Button("ShowControlPanelButton",mainMenu,"Show Video Device Controls");
		showControlPanelButton->getSelectCallbacks().add(this,&VideoViewer::showControlPanelCallback);
		}
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void VideoViewer::resetNavigationCallback(Misc::CallbackData* cbData)
	{
	/* Calculate the center point and diagonal size of the video frame: */
	Vrui::Point center=Vrui::Point::origin;
	Vrui::Scalar size=Vrui::Scalar(0);
	for(int i=0;i<2;++i)
		{
		/* Calculate the center point and width/height of the video frame: */
		Vrui::Scalar s=Math::div2(Vrui::Scalar(videoFormat.size[i]));
		center[i]=s;
		size+=Math::sqr(s);
		}
	
	/* Center and size the video frame, and rotate it so that Y points up: */
	Vrui::setNavigationTransformation(center,Math::sqrt(size),Vrui::Vector(0,1,0));
	}

void VideoViewer::showControlPanelCallback(Misc::CallbackData* cbData)
	{
	/* Pop up the video control panel: */
	Vrui::popupPrimaryWidget(videoControlPanel);
	}

VideoViewer::VideoViewer(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 videoDevice(0),videoExtractor(0),
	 saveVideoFrames(false),saveVideoFrameNameTemplate("Frame%06u.ppm"),saveVideoNextFrameIndex(0),
	 videoFrameVersion(0),
	 videoControlPanel(0),mainMenu(0)
	{
	/* Parse the command line: */
	const char* videoDeviceName=0;
	bool requestSize=false;
	int videoSize[2];
	bool requestRate=false;
	int videoRate;
	const char* pixelFormat=0;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			/* Parse a command line option: */
			if(strcasecmp(argv[i]+1,"size")==0||strcasecmp(argv[i]+1,"S")==0)
				{
				/* Parse the desired video frame size: */
				i+=2;
				if(i<argc)
					{
					for(int j=0;j<2;++j)
						videoSize[j]=atoi(argv[i-1+j]);
					requestSize=true;
					}
				else
					std::cerr<<"Ignoring dangling -size option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"rate")==0||strcasecmp(argv[i]+1,"R")==0)
				{
				/* Parse the desired video frame rate: */
				++i;
				if(i<argc)
					{
					videoRate=atoi(argv[i]);
					requestRate=true;
					}
				else
					std::cerr<<"Ignoring dangling -rate option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"format")==0||strcasecmp(argv[i]+1,"F")==0)
				{
				/* Parse the desired pixel format: */
				++i;
				if(i<argc)
					pixelFormat=argv[i];
				else
					std::cerr<<"Ignoring dangling -format option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"saveName")==0||strcasecmp(argv[i]+1,"SN")==0)
				{
				++i;
				if(i<argc)
					saveVideoFrameNameTemplate=argv[i];
				else
					std::cerr<<"Ignoring dangling -saveName option"<<std::endl;
				}
			else
				std::cerr<<"Ignoring unknown command line option "<<argv[i]<<std::endl;
			}
		else if(strcasecmp(argv[i],"list")==0)
			{
			/* Print a list of all connected video devices: */
			std::vector<Video::VideoDevice::DeviceIdPtr> videoDevices=Video::VideoDevice::getVideoDevices();
			std::cout<<"Connected video devices:"<<std::endl;
			for(std::vector<Video::VideoDevice::DeviceIdPtr>::iterator vdIt=videoDevices.begin();vdIt!=videoDevices.end();++vdIt)
				std::cout<<(*vdIt)->getName()<<std::endl;
			
			/* Exit from the program: */
			Vrui::shutdown();
			}
		else if(videoDeviceName==0)
			{
			/* Treat the argument as the name of a video device: */
			videoDeviceName=argv[i];
			}
		else
			std::cerr<<"Ignoring extra device name argument "<<argv[i]<<std::endl;
		}
	
	/* Open the video device: */
	std::vector<Video::VideoDevice::DeviceIdPtr> videoDevices=Video::VideoDevice::getVideoDevices();
	if(videoDeviceName==0&&!videoDevices.empty())
		{
		/* Select the first video device: */
		videoDevice=Video::VideoDevice::createVideoDevice(videoDevices.front());
		}
	else
		{
		/* Find a video device whose name matches the given name: */
		for(std::vector<Video::VideoDevice::DeviceIdPtr>::iterator vdIt=videoDevices.begin();vdIt!=videoDevices.end();++vdIt)
			if(strcasecmp((*vdIt)->getName().c_str(),videoDeviceName)==0)
				{
				/* Open the matching video device and bail out: */
				videoDevice=Video::VideoDevice::createVideoDevice(*vdIt);
				break;
				}
		}
	if(videoDevice==0)
		Misc::throwStdErr("VideoViewer: Could not find requested video device");
	
	/* Get and modify the video device's current video format: */
	videoFormat=videoDevice->getVideoFormat();
	if(requestSize)
		for(int i=0;i<2;++i)
			videoFormat.size[i]=(unsigned int)videoSize[i];
	if(requestRate)
		{
		/* Convert from frame rate in Hz to frame interval as a rational number: */
		videoFormat.frameIntervalCounter=1;
		videoFormat.frameIntervalDenominator=videoRate;
		}
	if(pixelFormat!=0)
		videoFormat.setPixelFormat(pixelFormat);
	videoDevice->setVideoFormat(videoFormat);
	
	/* Print the actual video format after adaptation: */
	std::cout<<"Selected video format on video device "<<(videoDeviceName!=0?videoDeviceName:"Default")<<":"<<std::endl;
	std::cout<<"Frame size "<<videoFormat.size[0]<<"x"<<videoFormat.size[1]<<" at "<<double(videoFormat.frameIntervalDenominator)/double(videoFormat.frameIntervalCounter)<<" Hz"<<std::endl;
	char videoPixelFormatBuffer[5];
	std::cout<<"Pixel format "<<videoFormat.getFourCC(videoPixelFormatBuffer)<<std::endl;
	
	/* Create an image extractor to convert from the video device's raw image format to RGB: */
	videoExtractor=videoDevice->createImageExtractor();
	
	/* Initialize the incoming video frame triple buffer: */
	for(int i=0;i<3;++i)
		{
		Images::RGBImage img(videoFormat.size[0],videoFormat.size[1]);
		img.clear(Images::RGBImage::Color(128,128,128));
		videoFrames.getBuffer(i)=img;
		}
	
	/* Create the video device's control panel: */
	videoControlPanel=videoDevice->createControlPanel(Vrui::getWidgetManager());
	
	/* Check if the control panel is a pop-up window; if so, add a close button: */
	GLMotif::PopupWindow* vcp=dynamic_cast<GLMotif::PopupWindow*>(videoControlPanel);
	if(vcp!=0)
		{
		/* Add a close button: */
		vcp->setCloseButton(true);
		
		/* Set it so that the popup window will pop itself down, but not destroy itself, when closed: */
		vcp->popDownOnClose();
		}
	
	/* Create and install the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	
	/* Create an event tool to start/stop saving video frames: */
	addEventTool("Save Video Frames",0,0);
	
	/* Initialize the navigation transformation to show the entire video image: */
	resetNavigationCallback(0);
	
	/* Start capturing video from the video device: */
	videoDevice->allocateFrameBuffers(5);
	videoDevice->startStreaming(Misc::createFunctionCall(this,&VideoViewer::videoFrameCallback));
	}

VideoViewer::~VideoViewer(void)
	{
	/* Stop streaming: */
	videoDevice->stopStreaming();
	videoDevice->releaseFrameBuffers();
	
	/* Close the video device: */
	delete videoExtractor;
	delete videoDevice;
	
	delete videoControlPanel;
	delete mainMenu;
	}

void VideoViewer::frame(void)
	{
	/* Lock the most recent video frame in the input triple buffer: */
	if(videoFrames.lockNewValue())
		{
		/* Bump up the video frame's version number to invalidate the cached texture: */
		++videoFrameVersion;
		}
	}

void VideoViewer::display(GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->videoTextureId);
	
	/* Check if the cached texture is outdated: */
	if(dataItem->videoTextureVersion!=videoFrameVersion)
		{
		/* Upload the most recent texture image: */
		videoFrames.getLockedValue().glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,!dataItem->haveNpotdt);
		dataItem->videoTextureVersion=videoFrameVersion;
		}
	
	glBegin(GL_QUADS);
	glTexCoord2f(dataItem->texMin[0],dataItem->texMin[1]);
	glVertex2i(0,0);
	glTexCoord2f(dataItem->texMax[0],dataItem->texMin[1]);
	glVertex2i(videoFormat.size[0],0);
	glTexCoord2f(dataItem->texMax[0],dataItem->texMax[1]);
	glVertex2i(videoFormat.size[0],videoFormat.size[1]);
	glTexCoord2f(dataItem->texMin[0],dataItem->texMax[1]);
	glVertex2i(0,videoFormat.size[1]);
	glEnd();
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Draw the video's backside: */
	glDisable(GL_TEXTURE_2D);
	glMaterial(GLMaterialEnums::FRONT,GLMaterial(GLMaterial::Color(0.7f,0.7f,0.7f)));
	
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex2i(0,0);
	glVertex2i(0,videoFormat.size[1]);
	glVertex2i(videoFormat.size[0],videoFormat.size[1]);
	glVertex2i(videoFormat.size[0],0);
	glEnd();
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

void VideoViewer::eventCallback(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(eventId==0)
		{
		/* Save video frames while the tool button is pressed: */
		saveVideoFrames=cbData->newButtonState;
		}
	}

void VideoViewer::initContext(GLContextData& contextData) const
	{
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Check whether non-power-of-two-dimension textures are supported: */
	dataItem->haveNpotdt=GLARBTextureNonPowerOfTwo::isSupported();
	if(dataItem->haveNpotdt)
		GLARBTextureNonPowerOfTwo::initExtension();
	
	/* Calculate the texture coordinate rectangle: */
	unsigned int texSize[2];
	if(dataItem->haveNpotdt)
		{
		for(int i=0;i<2;++i)
			texSize[i]=videoFormat.size[i];
		}
	else
		{
		/* Find the next larger power-of-two texture size: */
		for(int i=0;i<2;++i)
			for(texSize[i]=1U;texSize[i]<videoFormat.size[i];texSize[i]<<=1)
				;
		}
	
	/* Calculate texture coordinates to map the (padded) texture onto the geometry: */
	for(int i=0;i<2;++i)
		{
		dataItem->texMin[i]=0.0f;
		dataItem->texMax[i]=GLfloat(videoFormat.size[i])/GLfloat(texSize[i]);
		}
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->videoTextureId);
	
	/* Initialize basic texture settings: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
	/* Upload the initial texture image: */
	videoFrames.getLockedValue().glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,!dataItem->haveNpotdt);
	dataItem->videoTextureVersion=videoFrameVersion;
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(VideoViewer)
