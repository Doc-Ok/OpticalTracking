/***********************************************************************
LEDFinder - A simple viewer for live video from a video source
connected to the local computer.
Copyright (c) 2013-2014 Oliver Kreylos

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

#include "LEDFinder.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <Misc/ThrowStdErr.h>
#include <Misc/FunctionCalls.h>
#include <Misc/Timer.h>
#include <Misc/CreateNumberedFileName.h>
#include <Realtime/Time.h>
#include <IO/File.h>
#include <RawHID/BusType.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/ProjectiveTransformation.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <Images/WriteImageFile.h>
#include <Images/ExtractBlobs.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Button.h>
#include <Video/VideoDevice.h>
#include <Video/Linux/OculusRiftDK2VideoDevice.h>
#include <Video/ImageExtractor.h>
#include <Vrui/Vrui.h>
#include <Vrui/OpenFile.h>
#include <Vrui/VisletManager.h>

#include "RiftLEDControl.h"

#define HISTOFRAMETIMES 0
#if HISTOFRAMETIMES
#include <Math/Histogram.h>
#endif

#define SAVEFRAMES 0

#define SAVEBLOBS 1
#if SAVEBLOBS
std::ofstream blobFile;
#endif

/************************************
Methods of class LEDFinder::DataItem:
************************************/

LEDFinder::DataItem::DataItem(void)
	:videoTextureId(0)
	{
	glGenTextures(1,&videoTextureId);
	}

LEDFinder::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&videoTextureId);
	}

/**************************
Methods of class LEDFinder:
**************************/

void LEDFinder::videoFrameCallback(const Video::FrameBuffer* frameBuffer)
	{
	/* Store the frame's arrival time: */
	frameTimes[frameIndex%13].set();
	
	/* Check for dropped frames: */
	static Realtime::TimePointMonotonic frameTimer;
	Realtime::TimeVector delta=frameTimer.setAndDiff();
	if(delta.tv_nsec>=100000000) // First frame after tracking was disabled
		frameIndex=9;
	else if(delta.tv_nsec>=25000000) // One frame was dropped
		++frameIndex;
	
	#if HISTOFRAMETIMES
	static Math::Histogram<unsigned int> frameRateHist(500000,10000000,40000000);
	static Realtime::TimePointMonotonic lastFrame;
	Realtime::TimeVector diff=lastFrame.setAndDiff();
	frameRateHist.addSample((unsigned int)(diff.tv_sec*1000000000L+diff.tv_nsec));
	if(frameRateHist.getNumSamples()==1000)
		{
		size_t i0=frameRateHist.getFirstBinIndex();
		size_t i1=frameRateHist.getLastBinIndex();
		std::cout<<frameRateHist.getBinMin(i0)<<std::endl;
		for(size_t i=i0;i<i1;++i)
			{
			std::cout<<frameRateHist.getBinMax(i)<<": ";
			size_t numStars=(frameRateHist.getBinSize(i)*100+frameRateHist.getMaxBinSize()-1)/frameRateHist.getMaxBinSize();
			for(size_t i=0;i<numStars;++i)
				std::cout<<'#';
			std::cout<<std::endl;
			}
		std::cout<<"Average frame time: "<<double(frameRateHist.getAverageValue())*1.0e-6<<"ms"<<std::endl;
		
		frameRateHist.reset();
		}
	#endif
	
	/* Start a new frame in the input triple buffer: */
	NumberedGreyscaleFrame& frame=videoFrames.startNewValue();
	frame.index=frameIndex;
	
	/* Extract a greyscale image from the provided frame buffer into the new frame: */
	videoExtractor->extractGrey(frameBuffer,frame.frame);
	
	/* Finish the new frame in the triple buffer and wake up the blob extraction thread: */
	{
	Threads::MutexCond::Lock videoFrameLock(videoFrameCond);
	videoFrames.postNewValue();
	++frameIndex;
	videoFrameCond.signal();
	}
	
	#if SAVEFRAMES
	
	/* Write the just-extracted frame to a greyscale PPM image: */
	char fileName[256];
	snprintf(fileName,sizeof(fileName),"Frames/Frame%04u.pgm",frame.frameIndex);
	IO::FilePtr ppmFile=Vrui::openFile(fileName,IO::File::WriteOnly);
	static char header[]="P5\n752 480\n255\n";
	ppmFile->write(header,strlen(header));
	for(int y=0;y<videoFormat.size[1];++y)
		ppmFile->write(frame.frame+(videoFormat.size[1]-1-y)*videoFormat.size[0],videoFormat.size[0]);
	
	#endif
	}

namespace {

class BlobForegroundSelector // Functor class to select foreground pixels
	{
	/* Elements: */
	private:
	int threshold; // Maximum deviation from foreground pixels to pure red
	
	/* Constructors and destructors: */
	public:
	BlobForegroundSelector(int sThreshold)
		:threshold(sThreshold)
		{
		}
	
	/* Methods: */
	bool operator()(unsigned int x,unsigned int y,const Misc::UInt8& pixel) const
		{
		/* Return bright pixels in a greyscale image: */
		return pixel>=threshold;
		}
	};

}

void* LEDFinder::blobExtractorThreadMethod(void)
	{
	unsigned int lastFrameIndex=~0x0U;
	ModelTransform lastTransform;
	while(true)
		{
		/* Wait for the arrival of the next video frame: */
		{
		Threads::MutexCond::Lock videoFrameLock(videoFrameCond);
		while(runBlobExtractorThread&&!videoFrames.lockNewValue())
			videoFrameCond.wait(videoFrameLock);
		}
		if(!runBlobExtractorThread)
			break;
		
		// Realtime::TimePointMonotonic processingTimer;
		
		/* Process the most recent video frame: */
		unsigned int lastMask=0x200U>>(lastFrameIndex%10);
		lastFrameIndex=videoFrames.getLockedValue().index;
		
		typedef Images::CentroidBlob<Images::BboxBlob<Images::Blob<Misc::UInt8> > > Blob;
		BlobForegroundSelector bfs(112);
		std::vector<Blob> blobs=Images::extractBlobs<Blob>(frameSize,videoFrames.getLockedValue().frame,bfs,Blob::Creator(),blobIdImage);
		
		/* Create the next blobbed video frame: */
		Images::RGBImage& bFrame=blobbedFrames.startNewValue();
		const Misc::UInt8* sPtr=videoFrames.getLockedValue().frame;
		const unsigned int* biPtr=blobIdImage;
		Images::RGBImage::Color* dPtr=bFrame.modifyPixels();
		for(unsigned int y=0;y<frameSize[1];++y)
			for(unsigned int x=0;x<frameSize[0];++x,++sPtr,++biPtr,++dPtr)
				if(*biPtr!=~0x0U)
					*dPtr=Images::RGBImage::Color(0,255,0);
				else
					*dPtr=Images::RGBImage::Color(*sPtr,*sPtr,*sPtr);
		
		/* Create an array of all circle-like blobs and match them with blobs from the previous frame: */
		unsigned int currentMask=0x200U>>(lastFrameIndex%10);
		LEDPoint* leds=new LEDPoint[Math::max(blobs.size(),size_t(riftModel.getNumMarkers()))]; // Somewhat bigger than needed, but who cares...
		unsigned int numLeds=0;
		std::vector<LEDPoint>& idedLeds=identifiedLeds.startNewValue();
		idedLeds.clear();
		for(std::vector<Blob>::iterator bIt=blobs.begin();bIt!=blobs.end();++bIt)
			{
			/* Check if the blob is mostly circle-like: */
			unsigned int w=bIt->bbMax[0]+1-bIt->bbMin[0];
			unsigned int h=bIt->bbMax[1]+1-bIt->bbMin[1];
			if(bIt->numPixels>=10&&Math::max(w,h)*3<=Math::min(w,h)*4&&bIt->numPixels*10>=w*h*5) // 0.5 is somewhat smaller than pi/4...
				{
				/* Lens-correct this blob and create an LED structure for it: */
				Point2 lp=ldp.transform(Point2(bIt->cx/bIt->cw,bIt->cy/bIt->cw));
				leds[numLeds][0]=lp[0];
				leds[numLeds][1]=lp[1];
				leds[numLeds].blobSize=bIt->numPixels;
				leds[numLeds].numBits=0;
				leds[numLeds].ledId=0;
				leds[numLeds].markerIndex=~0;
				
				if(lastFrameLeds.getNumNodes()>0)
					{
					/* Find a matching blob in last frame's LED set: */
					const LEDPoint& closest=lastFrameLeds.findClosestPoint(leds[numLeds]);
					
					if(Geometry::sqrDist(leds[numLeds],closest)<Math::sqr(10)) // Some random cut-off value
						{
						leds[numLeds].numBits=closest.numBits;
						
						/* Check if the closest LED is a "fake" LED: */
						if(closest.blobSize==0)
							{
							/* Copy the state of the fake LED: */
							leds[numLeds].ledId=closest.ledId;
							}
						else
							{
							/* Compare the blob's current size to the previous one: */
							if(bIt->numPixels*12>closest.blobSize*13) // Definitely a '1' bit
								{
								/* Set the bit corresponding to the current frame counter: */
								leds[numLeds].ledId=closest.ledId|currentMask;
								++leds[numLeds].numBits;
								}
							else if(bIt->numPixels*13<closest.blobSize*12) // Definitely a '0' bit
								{
								/* Reset the bit corresponding to the current frame counter: */
								leds[numLeds].ledId=closest.ledId&~currentMask;
								++leds[numLeds].numBits;
								}
							else // No change; keep value of most-recently set bit
								{
								if((closest.ledId&lastMask)!=0x0U)
									{
									/* Set the bit corresponding to the current frame counter: */
									leds[numLeds].ledId=closest.ledId|currentMask;
									}
								else
									{
									/* Reset the bit corresponding to the current frame counter: */
									leds[numLeds].ledId=closest.ledId&~currentMask;
									}
								}
							}
						
						/* Check if the LED has been fully identified: */
						if(leds[numLeds].numBits>=10)
							{
							leds[numLeds].markerIndex=riftModel.getMarkerIndex(leds[numLeds].ledId);
							if(leds[numLeds].markerIndex<40)
								idedLeds.push_back(leds[numLeds]);
							}
						}
					}
				
				++numLeds;
				}
			}
		
		/* Check if there are enough identified LEDs to run model pose estimation: */
		size_t numIdedLeds=idedLeds.size();
		if(numIdedLeds>=4)
			{
			#if SAVEBLOBS
			
			/* Write all identified LEDs to the blob file: */
			blobFile<<lastFrameIndex<<' '<<numIdedLeds<<std::endl;
			for(std::vector<LEDPoint>::iterator ilIt=idedLeds.begin();ilIt!=idedLeds.end();++ilIt)
				{
				/* Write the blob centroid: */
				blobFile<<ilIt->markerIndex<<' '<<(*ilIt)[0]<<' '<<(*ilIt)[1];
				
				/* Write the associated LED's 3D position: */
				const HMDModel::Point& markerPos=riftModel.getMarkerPos(ilIt->markerIndex);
				blobFile<<' '<<markerPos[0]<<' '<<markerPos[1]<<' '<<markerPos[2]<<std::endl;
				}
			
			#endif
			
			/* Set the tracker's model to the set of currently identified LEDs and collect the lens-corrected blob centroid positions: */
			Point* modelPoints=new Point[numIdedLeds];
			Point* mpPtr=modelPoints;
			ImgPoint* imagePoints=new ImgPoint[numIdedLeds];
			ImgPoint* ipPtr=imagePoints;
			for(std::vector<LEDPoint>::iterator ilIt=idedLeds.begin();ilIt!=idedLeds.end();++ilIt,++mpPtr,++ipPtr)
				{
				*mpPtr=riftModel.getMarkerPos(ilIt->markerIndex);
				*ipPtr=*ilIt;
				}
			modelTracker.setModel(numIdedLeds,modelPoints);
			delete[] modelPoints;
			
			/* Calculate the new model transformation: */
			ModelTransform& newTransform=modelTransforms.startNewValue();
			
			/* If there is no valid transformation from the previous frame; start from scratch: */
			if(!lastTransform.valid)
				newTransform.transform=modelTracker.epnp(imagePoints);
			
			/* Refine the new transformation via iterative optimization: */
			newTransform.transform=modelTracker.levenbergMarquardt(imagePoints,newTransform.transform,50);
			
			/* Calculate the total squared reprojection error: */
			double error2=modelTracker.calcReprojectionError(imagePoints,newTransform.transform);
			
			/* Invalidate the pose if the reprojection error is too large: */
			newTransform.valid=error2<=2.0*double(modelTracker.getNumModelPoints());
			
			/* Post the result: */
			modelTransforms.postNewValue();
			delete[] imagePoints;
			
			if(newTransform.valid)
				{
				/* Add fake blobs for all visible model LEDs to the kd-tree for the next frame to stabilize and speed up LED identification: */
				numLeds=0;
				idedLeds.clear();
				for(unsigned int mi=0;mi<riftModel.getNumMarkers();++mi)
					{
					/* Check if the LED should be visible: */
					Point mp=newTransform.transform.transform(riftModel.getMarkerPos(mi));
					Point::Vector md=newTransform.transform.transform(riftModel.getMarkerDir(mi));
					if(mp*md<Point::Scalar(0))
						{
						/* Create a "fake" LED point by projecting the LED into the image: */
						ImgPoint ip=modelTracker.project(mp);
						for(int i=0;i<2;++i)
							leds[numLeds][i]=float(ip[i]);
						leds[numLeds].blobSize=0;
						leds[numLeds].numBits=10;
						leds[numLeds].ledId=riftModel.getMarkerPattern(mi);
						leds[numLeds].markerIndex=mi;
						idedLeds.push_back(leds[numLeds]);
						++numLeds;
						}
					}
				}
			}
		else
			modelTransforms.postNewValue(ModelTransform());
		
		/* Post the list of identified LEDs and the new blobbed video frame: */
		identifiedLeds.postNewValue();
		blobbedFrames.postNewValue();
		Vrui::requestUpdate();
		
		/* Store the new array of LEDs as the association kd-tree for the next frame: */
		lastFrameLeds.donatePoints(numLeds,leds); // Kd-tree now owns LED array and will delete it
		
		// double processingTime=double(processingTimer.setAndDiff())*1000.0;
		// std::cout<<processingTime<<std::endl;
		}
	
	return 0;
	}

GLMotif::PopupMenu* LEDFinder::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Video Viewer");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create a button to reset navigation back to full-frame view: */
	GLMotif::Button* resetNavigationButton=new GLMotif::Button("ResetNavigationButton",mainMenu,"Reset Navigation");
	resetNavigationButton->getSelectCallbacks().add(this,&LEDFinder::resetNavigationCallback);
	
	if(videoControlPanel!=0)
		{
		/* Create a button to pop up the video control panel: */
		GLMotif::Button* showControlPanelButton=new GLMotif::Button("ShowControlPanelButton",mainMenu,"Show Video Device Controls");
		showControlPanelButton->getSelectCallbacks().add(this,&LEDFinder::showControlPanelCallback);
		}
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void LEDFinder::resetNavigationCallback(Misc::CallbackData* cbData)
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

void LEDFinder::showControlPanelCallback(Misc::CallbackData* cbData)
	{
	/* Pop up the video control panel: */
	Vrui::popupPrimaryWidget(videoControlPanel);
	}

LEDFinder::LEDFinder(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 rift(RawHID::BUSTYPE_USB,0x2833U,0x0021U,0),
	 videoDevice(0),videoExtractor(0),
	 frameIndex(0),
	 runBlobExtractorThread(true),
	 blobIdImage(0),
	 blobbedFrameVersion(0),
	 numberRenderer(10,false),
	 videoControlPanel(0),mainMenu(0)
	{
	#if SAVEBLOBS
	blobFile.open("BlobFile.txt");
	#endif
	
	/* Create the Rift's 3D LED model: */
	riftModel.readFromRiftDK2(rift);
	
	/* Create an event tool class: */
	addEventTool("Save Frame",0,0);
	
	/* Parse the command line: */
	const char* videoDeviceName=0;
	bool requestSize=false;
	int videoSize[2];
	bool requestRate=false;
	int videoRate;
	const char* pixelFormat=0;
	const char* cameraName=0;
	const char* modelFileName=0;
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
			else if(strcasecmp(argv[i]+1,"camera")==0||strcasecmp(argv[i]+1,"C")==0)
				{
				/* Read the camera name: */
				++i;
				if(i<argc)
					cameraName=argv[i];
				else
					std::cerr<<"Ignoring dangling -camera option"<<std::endl;
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
			return;
			}
		else if(videoDeviceName==0)
			{
			/* Treat the argument as the name of a video device: */
			videoDeviceName=argv[i];
			}
		else if(modelFileName==0)
			{
			/* Treat the argument as the name of a 3D model file: */
			modelFileName=argv[i];
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
		Misc::throwStdErr("LEDFinder: Could not find requested video device");
	Video::OculusRiftDK2VideoDevice* ordk2vd=dynamic_cast<Video::OculusRiftDK2VideoDevice*>(videoDevice);
	
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
	
	/* Initialize the video source's lens distortion parameters: */
	int ldpFrameSize[2];
	for(int i=0;i<2;++i)
		ldpFrameSize[i]=int(videoFormat.size[i]);
	ldp=LensDistortionParameters(ldpFrameSize);
	if(videoDeviceName!=0)
		{
		try
			{
			/* Load the lens distortion parameters: */
			std::string ldpFileName=videoDeviceName;
			ldpFileName.append(".ldp");
			ldp.read(ldpFileName.c_str());
			}
		catch(std::runtime_error err)
			{
			/* Print a warning: */
			std::cerr<<"Could not load lens distortion parameters due to exception "<<err.what()<<"; using defaults"<<std::endl;
			}
		
		try
			{
			/* Load the intrinsic camera parameters: */
			std::string icpFileName=videoDeviceName;
			icpFileName.append(".icp");
			modelTracker.loadCameraIntrinsics(*Vrui::openDirectory("."),icpFileName.c_str());
			}
		catch(std::runtime_error err)
			{
			/* Print a warning: */
			std::cerr<<"Could not load intrinsic camera parameters due to exception "<<err.what()<<"; using defaults"<<std::endl;
			}
		}
	
	if(modelFileName!=0)
		{
		/* Load a 3D model file: */
		std::ifstream modelFile(modelFileName);
		unsigned int numModelPoints;
		modelFile>>numModelPoints;
		ModelTracker::Point* modelPoints=new ModelTracker::Point[numModelPoints];
		for(unsigned int i=0;i<numModelPoints;++i)
			modelFile>>modelPoints[i][0]>>modelPoints[i][1]>>modelPoints[i][2];
		modelTracker.setModel(numModelPoints,modelPoints);
		delete[] modelPoints;
		}
	
	modelTracker.setMaxMatchDist(5.0);
	
	/* Initialize the incoming video frame triple buffer: */
	for(int i=0;i<2;++i)
		frameSize[i]=videoFormat.size[i];
	for(int i=0;i<3;++i)
		videoFrames.getBuffer(i).frame=new Misc::UInt8[frameSize[1]*frameSize[0]];
	
	/* Create the blob ID image: */
	blobIdImage=new unsigned int[frameSize[1]*frameSize[0]];  
	
	/* Initialize the blobbed video frame triple buffer: */
	for(int i=0;i<3;++i)
		{
		Images::RGBImage img(frameSize[0],frameSize[1]);
		img.clear(Images::RGBImage::Color(128,128,128));
		blobbedFrames.getBuffer(i)=img;
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
	
	if(ordk2vd!=0)
		{
		/* Instantiate a Rift LED control vislet: */
		RiftLEDControlFactory* factory=new RiftLEDControlFactory(*Vrui::getVisletManager());
		Vrui::getVisletManager()->createVislet(factory,0,0);
		}
	
	/* Create and install the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	
	/* Initialize the navigation transformation to show the entire video image: */
	resetNavigationCallback(0);
	
	/* Start the blob extractor thread: */
	blobExtractorThread.start(this,&LEDFinder::blobExtractorThreadMethod);
	
	/* Start capturing video from the video device: */
	videoDevice->allocateFrameBuffers(5);
	videoDevice->startStreaming(Misc::createFunctionCall(this,&LEDFinder::videoFrameCallback));
	
	if(ordk2vd!=0)
		{
		/* Set the camera to IR tracking mode: */
		ordk2vd->setTrackingMode(true);
		}
	}

LEDFinder::~LEDFinder(void)
	{
	if(videoDevice!=0)
		{
		Video::OculusRiftDK2VideoDevice* ordk2vd=dynamic_cast<Video::OculusRiftDK2VideoDevice*>(videoDevice);
		if(ordk2vd!=0)
			{
			/* Set the camera back to regular mode: */
			ordk2vd->setTrackingMode(false);
			}
		
		/* Stop streaming: */
		videoDevice->stopStreaming();
		videoDevice->releaseFrameBuffers();
		}
	
	/* Close the video device: */
	delete videoExtractor;
	delete videoDevice;
	
	if(!blobExtractorThread.isJoined())
		{
		/* Shut down the blob extractor thread: */
		{
		Threads::MutexCond::Lock videoFrameLock(videoFrameCond);
		runBlobExtractorThread=false;
		videoFrameCond.signal();
		}
		blobExtractorThread.join();
		}
	delete[] blobIdImage;
	
	delete videoControlPanel;
	delete mainMenu;
	}

void LEDFinder::frame(void)
	{
	/* Lock the most recent list of identified LEDs: */
	identifiedLeds.lockNewValue();
	
	/* Lock the most recent video frame in the input triple buffer: */
	if(blobbedFrames.lockNewValue())
		{
		/* Bump up the video frame's version number to invalidate the cached texture: */
		++blobbedFrameVersion;
		}
	
	/* Lock the most recent model transformation: */
	modelTransforms.lockNewValue();
	}

void LEDFinder::display(GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_POINT_BIT|GL_TEXTURE_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->videoTextureId);
	
	/* Check if the cached texture is outdated: */
	if(dataItem->videoTextureVersion!=blobbedFrameVersion)
		{
		/* Upload the most recent texture image: */
		blobbedFrames.getLockedValue().glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,!dataItem->haveNpotdt);
		dataItem->videoTextureVersion=blobbedFrameVersion;
		}
	
	#if 1
	
	/* Map the texture onto a distortion-correcting surface: */
	for(int v=1;v<=48;++v)
		{
		double y0=double(v-1)*double(videoFormat.size[1])/48.0;
		double y1=double(v)*double(videoFormat.size[1])/48.0;
		glBegin(GL_QUAD_STRIP);
		for(int h=0;h<=64;++h)
			{
			double x=double(h)*double(videoFormat.size[0])/64.0;
			LensDistortionParameters::Point p1=ldp.transformNonNormalized(LensDistortionParameters::Point(x,y1));
			glTexCoord2f(float(h)/64.0f,float(v)/48.0f);
			glVertex2d(p1[0],p1[1]);
			LensDistortionParameters::Point p0=ldp.transformNonNormalized(LensDistortionParameters::Point(x,y0));
			glTexCoord2f(float(h)/64.0f,float(v-1)/48.0f);
			glVertex2d(p0[0],p0[1]);
			}
		glEnd();
		}
	
	#else
	
	/* Map the texture onto a quad: */
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
	
	#endif
	
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
	
	glDisable(GL_LIGHTING);
	glLineWidth(3.0f);
	glPointSize(3.0f);
	glColor3f(1.0f,0.0f,1.0f);
	
	#if 1
	
	/* Draw the list of identified LEDs: */
	for(std::vector<LEDPoint>::const_iterator lIt=identifiedLeds.getLockedValue().begin();lIt!=identifiedLeds.getLockedValue().end();++lIt)
		{
		GLNumberRenderer::Vector pos;
		pos[0]=(*lIt)[0];
		pos[1]=(*lIt)[1];
		pos[2]=0.01f;
		numberRenderer.drawNumber(pos,lIt->markerIndex,contextData,0,0);
		}
	
	#endif
	
	/* Draw the current camera frame interval: */
	double videoFrameInterval=double(frameTimes[(frameIndex+12)%13]-frameTimes[frameIndex%13])*1000.0/12.0;
	numberRenderer.drawNumber(GLNumberRenderer::Vector(-0.5,-0.5,0.01),videoFrameInterval,2,contextData,1,-1);
	
	/* Check if there is a valid reconstructed model transformation: */
	static const unsigned int lineIndices[]=
		{
		10,9,9,7,7,6,6,33,33,32,32,30,30,29,29,10,
		10,13,13,12,12,4,4,6,
		29,26,26,27,27,35,35,33,
		13,17,17,22,22,26,
		4,2,2,19,19,37,37,35,
		10,11,11,13,9,11,
		29,28,28,26,30,28,
		3,18,18,14,14,3,36,25,25,21,21,36,
		18,20,20,21,13,14,4,3,26,25,35,36,
		17,14,22,25,37,36,2,3,19,20
		};
	glLineWidth(1.0f);
	if(modelTransforms.getLockedValue().valid)
		{
		/* Project the 3D model onto the camera image: */
		glBegin(GL_LINES);
		for(unsigned int i=0;i<sizeof(lineIndices)/sizeof(lineIndices[0]);++i)
			{
			ImgPoint ip=modelTracker.project(modelTransforms.getLockedValue().transform.transform(riftModel.getMarkerPos(lineIndices[i])));
			glVertex3f(ip[0],ip[1],0.01f);
			}
		glEnd();
		glBegin(GL_POINTS);
		for(unsigned int i=0;i<riftModel.getNumMarkers();++i)
			{
			ImgPoint ip=modelTracker.project(modelTransforms.getLockedValue().transform.transform(riftModel.getMarkerPos(i)));
			glVertex3f(ip[0],ip[1],0.01f);
			}
		glEnd();
		}
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

void LEDFinder::eventCallback(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Save the most recent frame: */
		char fileNameBuffer[256];
		Images::writeImageFile(blobbedFrames.getLockedValue(),Misc::createNumberedFileName("VideoFrame.png",4,fileNameBuffer));
		}
	}

void LEDFinder::initContext(GLContextData& contextData) const
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
	blobbedFrames.getLockedValue().glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,!dataItem->haveNpotdt);
	dataItem->videoTextureVersion=blobbedFrameVersion;
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(LEDFinder)
