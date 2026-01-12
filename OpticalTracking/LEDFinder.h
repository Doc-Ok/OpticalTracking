/***********************************************************************
LEDFinder - A simple viewer for live video from a video source
connected to the local computer.
Copyright (c) 2013-2022 Oliver Kreylos

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

#ifndef LEDFINDER_INCLUDED
#define LEDFINDER_INCLUDED

#include <Misc/SizedTypes.h>
#include <Realtime/Time.h>
#include <Threads/Thread.h>
#include <Threads/MutexCond.h>
#include <Threads/TripleBuffer.h>
#include <RawHID/Device.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLNumberRenderer.h>
#include <Images/RGBImage.h>
#include <Video/Types.h>
#include <Video/VideoDataFormat.h>
#include <Vrui/Application.h>

#include "LensDistortionParameters.h"
#include "HMDModel.h"
#include "ModelTracker.h"

/* Forward declarations: */
namespace GLMotif {
class Widget;
class PopupMenu;
}
namespace Video {
class FrameBuffer;
class VideoDevice;
class ImageExtractor;
}

class LEDFinder:public Vrui::Application,public GLObject
	{
	/* Embedded classes: */
	private:
	struct NumberedGreyscaleFrame // Structure for numbered greyscale image frames
		{
		/* Elements: */
		public:
		unsigned int index; // Frame index
		Misc::UInt8* frame; // Pointer to the allocated frame buffer
		
		/* Constructors and destructors: */
		NumberedGreyscaleFrame(void)
			:index(0),frame(0)
			{
			}
		~NumberedGreyscaleFrame(void)
			{
			delete[] frame;
			}
		};
	
	typedef Geometry::Point<float,2> Point2; // Type for points in image space
	
	struct LEDPoint:public Point2 // Structure for identified LEDs in image space
		{
		/* Elements: */
		public:
		unsigned int blobSize; // Blob size of the LED point in the current frame
		unsigned int numBits; // Number of bits that have been shoved in since this blob was detected
		unsigned int ledId; // Current value of the decoded LED ID
		unsigned int markerIndex; // Index of the LED's associated marker in the HMD model
		
		/* Constructors and destructors: */
		LEDPoint(void)
			:blobSize(0),numBits(0),ledId(0),markerIndex(~0)
			{
			}
		};
	
	typedef Geometry::ArrayKdTree<LEDPoint> LEDTree; // Type for kd-trees to match LEDs in image space between frames
	
	typedef ModelTracker::Point Point;
	typedef ModelTracker::ImgPoint ImgPoint;
	
	struct ModelTransform // Structure to hold reconstructed model transformations with valid flag
		{
		/* Elements: */
		public:
		bool valid; // Flag whether the reconstructed transformation is valid
		ModelTracker::Transform transform; // The reconstructed transformation
		
		/* Constructors and destructors: */
		ModelTransform(void) // Creates an invalid model transformation
			:valid(false)
			{
			}
		ModelTransform(const ModelTracker::Transform& sTransform) // Creates a valid model transformation
			:valid(true),transform(sTransform)
			{
			}
		};
	
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
	RawHID::Device rift; // The Rift's raw HID device
	HMDModel riftModel; // A 3D model of the Rift's tracking LEDs
	Video::VideoDevice* videoDevice; // Pointer to the video recording device
	Video::VideoDataFormat videoFormat; // Configured video format of the video device
	Video::ImageExtractor* videoExtractor; // Helper object to convert video frames to RGB
	LensDistortionParameters ldp; // The video recording device's lens distortion parameters
	ModelTracker modelTracker; // Object to reconstruct the pose of the tracked 3D model
	unsigned int frameIndex; // Index of the next incoming video frame
	Realtime::TimePointMonotonic frameTimes[13]; // Array of recent video frame arrival times to calculate an accurate frame rate
	Video::Size frameSize; // Size of incoming video frames
	Threads::TripleBuffer<NumberedGreyscaleFrame> videoFrames; // Triple buffer to pass video frames from the video callback to the blob extractor
	Threads::MutexCond videoFrameCond; // Condition variable to signal arrival of a new video frame
	volatile bool runBlobExtractorThread; // Flag to terminate the blob extraction thread
	Threads::Thread blobExtractorThread; // Thread extracting blobs from video frames
	unsigned int* blobIdImage; // Image of per-pixel blob IDs
	LEDTree lastFrameLeds; // Kd-tree containing LEDs extracted from the previous frame
	Threads::TripleBuffer<std::vector<LEDPoint> > identifiedLeds; // Triple buffer of lists of identified LEDs
	Threads::TripleBuffer<ModelTransform> modelTransforms; // Triple buffer of reconstructed 3D model transformations
	Threads::TripleBuffer<Images::RGBImage> blobbedFrames; // Triple buffer to pass blob-extracted frames to the main loop
	unsigned int blobbedFrameVersion; // Version number of the most recent blobbed video frame in the triple buffer
	GLNumberRenderer numberRenderer; // Helper object to draw LED labels
	GLMotif::Widget* videoControlPanel; // The video device's control panel
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	void videoFrameCallback(const Video::FrameBuffer* frameBuffer); // Callback receiving incoming video frames
	void* blobExtractorThreadMethod(void); // Method run by the blob extractor thread
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	void resetNavigationCallback(Misc::CallbackData* cbData); // Method to reset the Vrui navigation transformation to its default
	void showControlPanelCallback(Misc::CallbackData* cbData); // Method to pop up the video device's control panel
	
	/* Constructors and destructors: */
	public:
	LEDFinder(int& argc,char**& argv);
	virtual ~LEDFinder(void);
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

#endif
