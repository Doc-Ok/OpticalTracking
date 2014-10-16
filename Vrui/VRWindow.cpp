/***********************************************************************
VRWindow - Class for OpenGL windows that are used to map one or two eyes
of a viewer onto a VR screen.
Copyright (c) 2004-2014 Oliver Kreylos
ZMap stereo mode additions copyright (c) 2011 Matthias Deller.

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

#define GLX_GLXEXT_PROTOTYPES 1
#define SAVE_SCREENSHOT_PROJECTION 0
#define RENDERFRAMETIMES 0

#include <Vrui/VRWindow.h>

#include <unistd.h>
#include <iostream>
#include <X11/keysym.h>
#include <Misc/PrintInteger.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/CallbackData.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#if SAVE_SCREENSHOT_PROJECTION
#include <IO/File.h>
#endif
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/AffineTransformation.h>
#include <Geometry/Plane.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
// #include <GL/glxext.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLPrintError.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBTextureRectangle.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLEXTFramebufferBlit.h>
#include <GL/Extensions/GLEXTFramebufferObject.h>
#include <GL/Extensions/GLEXTFramebufferMultisample.h>
#include <GL/Extensions/GLEXTPackedDepthStencil.h>
#include <GL/GLShader.h>
#include <GL/GLContextData.h>
#include <GL/GLFont.h>
#include <GL/GLTransformationWrappers.h>
#include <Images/Config.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>
#include <Images/WriteImageFile.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>
#if SAVE_SCREENSHOT_PROJECTION
#include <Vrui/OpenFile.h>
#endif
#include <Vrui/InputDevice.h>
#include <Vrui/Internal/InputDeviceAdapterMouse.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/WindowProperties.h>
#include <Vrui/ViewSpecification.h>
#include <Vrui/Tool.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Internal/LensCorrector.h>
#include <Vrui/Internal/ToolKillZone.h>
#include <Vrui/Internal/MovieSaver.h>
#include <Vrui/Internal/Vrui.h>
#include <Vrui/Internal/Config.h>
#if VRUI_INTERNAL_CONFIG_HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif

#if SAVE_SCREENSHOT_PROJECTION
#include <Misc/File.h>
#include <GL/GLTransformationWrappers.h>
#endif

namespace Misc {

/***********************************
Helper class to decode window types:
***********************************/

template <>
class ValueCoder<Vrui::VRWindow::WindowType>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::VRWindow::WindowType& value)
		{
		switch(value)
			{
			case Vrui::VRWindow::MONO:
				return "Mono";
			
			case Vrui::VRWindow::LEFT:
				return "LeftEye";
			
			case Vrui::VRWindow::RIGHT:
				return "RightEye";
			
			case Vrui::VRWindow::QUADBUFFER_STEREO:
				return "QuadbufferStereo";
			
			case Vrui::VRWindow::ANAGLYPHIC_STEREO:
				return "AnaglyphicStereo";
			
			case Vrui::VRWindow::SPLITVIEWPORT_STEREO:
				return "SplitViewportStereo";
			
			case Vrui::VRWindow::INTERLEAVEDVIEWPORT_STEREO:
				return "InterleavedViewportStereo";
			
			case Vrui::VRWindow::AUTOSTEREOSCOPIC_STEREO:
				return "AutoStereoscopicStereo";
			}
		}
	static Vrui::VRWindow::WindowType decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		if(end-start>=4&&strncasecmp(start,"Mono",4)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+4;
			return Vrui::VRWindow::MONO;
			}
		else if(end-start>=7&&strncasecmp(start,"LeftEye",7)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+7;
			return Vrui::VRWindow::LEFT;
			}
		else if(end-start>=8&&strncasecmp(start,"RightEye",8)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+8;
			return Vrui::VRWindow::RIGHT;
			}
		else if(end-start>=16&&strncasecmp(start,"QuadbufferStereo",16)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+16;
			return Vrui::VRWindow::QUADBUFFER_STEREO;
			}
		else if(end-start>=16&&strncasecmp(start,"AnaglyphicStereo",16)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+16;
			return Vrui::VRWindow::ANAGLYPHIC_STEREO;
			}
		else if(end-start>=18&&strncasecmp(start,"SplitViewportStereo",18)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+18;
			return Vrui::VRWindow::SPLITVIEWPORT_STEREO;
			}
		else if(end-start>=25&&strncasecmp(start,"InterleavedViewportStereo",25)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+25;
			return Vrui::VRWindow::INTERLEAVEDVIEWPORT_STEREO;
			}
		else if(end-start>=22&&strncasecmp(start,"AutoStereoscopicStereo",22)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+22;
			return Vrui::VRWindow::AUTOSTEREOSCOPIC_STEREO;
			}
		else
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to VRWindow::WindowType"));
		}
	};

/*************************************************
Helper class to decode window positions and sizes:
*************************************************/

template <>
class ValueCoder<GLWindow::WindowPos>
	{
	/* Methods: */
	public:
	static std::string encode(const GLWindow::WindowPos& value)
		{
		std::string result;
		
		result.append(CFixedArrayValueCoder<int,2>::encode(value.origin));
		result.push_back(',');
		result.push_back(' ');
		result.append(CFixedArrayValueCoder<int,2>::encode(value.size));
		return result;
		}
	static GLWindow::WindowPos decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		try
			{
			GLWindow::WindowPos result;
			const char* cPtr=start;
			
			/* Parse the origin: */
			CFixedArrayValueCoder<int,2>(result.origin).decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			
			/* Check for separating comma: */
			cPtr=checkSeparator(',',cPtr,end);
			
			/* Parse the size: */
			CFixedArrayValueCoder<int,2>(result.size).decode(cPtr,end,&cPtr);
			
			if(decodeEnd!=0)
				*decodeEnd=cPtr;
			return result;
			}
		catch(std::runtime_error err)
			{
			throw DecodingError(std::string("Unable to convert ")+std::string(start,end)+std::string(" to GLWindow::WindowPos due to ")+err.what());
			}
		}
	};

}

namespace Vrui {

#if RENDERFRAMETIMES
const int numFrameTimes=800;
extern double frameTimes[];
extern int frameTimeIndex;
#endif

/*************************
Methods of class VRWindow:
*************************/

void VRWindow::render(const GLWindow::WindowPos& viewportPos,int screenIndex,const Point& eye)
	{
	/*********************************************************************
	First step: Re-initialize OpenGL state and clear all buffers.
	*********************************************************************/
	
	/* Set up lens distortion correction if requested: */
	if(lensCorrector!=0)
		{
		/* Prepare the lens corrector for this eye: */
		lensCorrector->prepare(screenIndex,*displayState);
		}
	else
		{
		/* Set the viewport: */
		glViewport(viewportPos.origin[0],viewportPos.origin[1],viewportPos.size[0],viewportPos.size[1]);
		for(int i=0;i<2;++i)
			{
			displayState->viewport[i]=viewportPos.origin[i];
			displayState->viewport[2+i]=viewportPos.size[i];
			}
		for(int i=0;i<2;++i)
			displayState->frameSize[i]=getWindowSize()[i];
		}
	
	/* Clear all relevant buffers: */
	glClearColor(getBackgroundColor());
	glClearDepth(1.0); // Clear depth is "infinity"
	if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
		glClearStencil(0x0);
	if(clearBufferMask&GL_ACCUM_BUFFER_BIT)
		glClearAccum(0.0f,0.0f,0.0f,0.0f);
	glClear(clearBufferMask);
	
	/* Initialize standard OpenGL settings: */
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_ALWAYS,0.0f);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
		{
		glDisable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS,0,~0x0U);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		glStencilMask(~0x0U);
		}
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	
	/*********************************************************************
	Second step: Set up the projection and modelview matrices to project
	from the given eye onto the given screen.
	*********************************************************************/
	
	/* Get the inverse of the current screen transformation: */
	ONTransform invScreenT=screens[screenIndex]->getScreenTransformation();
	invScreenT.doInvert();
	
	/* Transform the eye position to screen coordinates: */
	Point screenEyePos=invScreenT.transform(eye);
	
	/* Calculate the screen's frustum transformation: */
	double near=getFrontplaneDist();
	double far=getBackplaneDist();
	double left=(viewports[screenIndex][0]-screenEyePos[0])/screenEyePos[2]*near;
	double right=(viewports[screenIndex][1]-screenEyePos[0])/screenEyePos[2]*near;
	double bottom=(viewports[screenIndex][2]-screenEyePos[1])/screenEyePos[2]*near;
	double top=(viewports[screenIndex][3]-screenEyePos[1])/screenEyePos[2]*near;
	
	/* Adjust the frustum transformation if lens correction is active: */
	if(lensCorrector!=0)
		lensCorrector->adjustProjection(screenIndex,screenEyePos,near,left,right,bottom,top);
	
	PTransform projection;
	PTransform::Matrix& pm=projection.getMatrix();
	pm(0,0)=2.0*near/(right-left);
	pm(0,2)=(right+left)/(right-left);
	pm(1,1)=2.0*near/(top-bottom);
	pm(1,2)=(top+bottom)/(top-bottom);
	pm(2,2)=-(far+near)/(far-near);
	pm(2,3)=-2.0*far*near/(far-near);
	pm(3,2)=-1.0;
	pm(3,3)=0.0;
	
	/* Check if the screen is projected off-axis: */
	if(screens[screenIndex]->isOffAxis())
		{
		/* Apply the screen's off-axis correction homography: */
		projection.leftMultiply(screens[screenIndex]->getInverseClipHomography());
		}
	
	/* Upload the projection matrix to OpenGL: */
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(projection);
	
	/* Calculate the base modelview matrix: */
	OGTransform modelview=OGTransform::translateToOriginFrom(screenEyePos);
	modelview*=OGTransform(invScreenT);
	
	/*********************************************************************
	Third step: Render Vrui state.
	*********************************************************************/
	
	/* Update the window's display state object: */
	displayState->resized=resizeViewport;
	displayState->viewer=viewers[screenIndex];
	displayState->eyePosition=eye;
	displayState->screen=screens[screenIndex];
	
	/* Store the projection and physical and navigational modelview matrices: */
	displayState->projection=projection;
	displayState->modelviewPhysical=modelview;
	modelview*=getNavigationTransformation();
	modelview.renormalize();
	displayState->modelviewNavigational=modelview;
	
	#if 0 // Don't do this; it's a bad idea
	
	/*********************************************************************
	Fudge the navigational modelview transformation such that the point
	that should end up being transformed to the display center actually
	does get transformed to the display center, given OpenGL's limited
	precision.
	*********************************************************************/
	
	/* Get the display center point both in eye and navigational coordinates: */
	Point dcEye=displayState->modelviewPhysical.transform(getDisplayCenter());
	Point dcNav=getNavigationTransformation().inverseTransform(getDisplayCenter());
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrix(displayState->modelviewNavigational);
	typedef Geometry::ProjectiveTransformation<double,3> PTransform;
	PTransform oglMv=glGetModelviewMatrix<double>();
	// PTransform oglMv=PTransform::translate(displayState->modelviewNavigational.getTranslation());
	// oglMv*=PTransform::rotate(displayState->modelviewNavigational.getRotation());
	// oglMv*=PTransform::scale(displayState->modelviewNavigational.getScaling());
	
	/* Transform the navigational-space display center with the truncated modelview matrix: */
	Point oglDcEye=oglMv.transform(dcNav);
	
	/* Multiply the error correction translation vector onto the modelview matrix: */
	displayState->modelviewNavigational.leftMultiply(OGTransform::translate(dcEye-oglDcEye));
	
	#endif
	
	/* Call Vrui's main rendering function: */
	vruiState->display(displayState,getContextData());
	
	if(lensCorrector!=0)
		lensCorrector->finish(screenIndex);
	
	/*********************************************************************
	Fourth step: Render screen protectors and fps counter.
	*********************************************************************/
	
	if(protectScreens&&vruiState->numProtectors>0)
		{
		/* Check if any monitored input device is dangerously close to the screen: */
		bool renderProtection=false;
		for(int i=0;i<vruiState->numProtectors;++i)
			{
			VruiState::ScreenProtector& sp=vruiState->protectors[i];
			
			/* Transform device protection sphere center to screen coordinates: */
			Point p=sp.inputDevice->getTransformation().transform(sp.center);
			p=invScreenT.transform(p);
			if(p[2]>-sp.radius&&p[2]<sp.radius)
				if(p[0]>-sp.radius&&p[0]<screens[screenIndex]->getWidth()+sp.radius&&p[1]>-sp.radius&&p[1]<screens[screenIndex]->getHeight()+sp.radius)
					{
					renderProtection=true;
					break;
					}
			}
		
		if(renderProtection)
			{
			/* Save and set up OpenGL state: */
			glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
			glDisable(GL_LIGHTING);
			glLineWidth(1.0f);
			
			/* Set OpenGL matrices to pixel-based: */
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0,viewportPos.size[0],0,viewportPos.size[1],0,1);
			
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			
			/* Render grid onto screen: */
			glBegin(GL_LINES);
			glColor3f(0.0f,1.0f,0.0f);
			for(int x=0;x<=10;++x)
				{
				int pos=x*(viewportPos.size[0]-1)/10;
				glVertex2i(pos,0);
				glVertex2i(pos,viewportPos.size[1]);
				}
			for(int y=0;y<=10;++y)
				{
				int pos=y*(viewportPos.size[1]-1)/10;
				glVertex2i(0,pos);
				glVertex2i(viewportPos.size[0],pos);
				}
			glEnd();
			
			/* Reset the OpenGL matrices: */
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			
			/* Restore OpenGL state: */
			glPopAttrib();
			}
		}
	
	if(showFps&&burnMode)
		{
		/* Set OpenGL matrices to pixel-based: */
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0,viewportPos.size[0],0,viewportPos.size[1],0,1);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
		#if RENDERFRAMETIMES
		
		/* Save and set up OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		
		/* Render EKG of recent frame rates: */
		glBegin(GL_LINES);
		glColor3f(0.0f,1.0f,0.0f);
		for(int i=0;i<numFrameTimes;++i)
			if(i!=frameTimeIndex)
				{
				glVertex2i(i,0);
				glVertex2i(i,int(floor(frameTimes[i]*1000.0+0.5)));
				}
		glColor3f(1.0f,0.0f,0.0f);
		glVertex2i(frameTimeIndex,0);
		glVertex2i(frameTimeIndex,int(floor(frameTimes[frameTimeIndex]*1000.0+0.5)));
		glEnd();
		
		/* Restore OpenGL state: */
		glPopAttrib();
		
		#else
		
		/* Save and set up OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		
		/* Print the current frame time: */
		unsigned int fps=(unsigned int)(10.0/vruiState->currentFrameTime+0.5);
		char buffer[20];
		char* bufPtr=buffer+15;
		*(--bufPtr)=char(fps%10+'0');
		fps/=10;
		*(--bufPtr)='.';
		do
			{
			*(--bufPtr)=char(fps%10+'0');
			fps/=10;
			}
		while(bufPtr>buffer&&fps!=0);
		buffer[15]=' ';
		buffer[16]='f';
		buffer[17]='p';
		buffer[18]='s';
		buffer[19]='\0';
		
		/* Draw the current frame time: */
		showFpsFont->drawString(GLFont::Vector(showFpsFont->getCharacterWidth()*9.5f+2.0f,2.0f,0.0f),bufPtr);
		
		/* Restore OpenGL state: */
		glPopAttrib();
		
		#endif
		
		/* Reset the OpenGL matrices: */
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		}
	}

GLContext* VRWindow::createContext(const WindowProperties& properties,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Query flags that determine the window's required visual type: */
	bool vsync=configFileSection.retrieveValue<bool>("./vsync",false);
	bool lensCorrection=configFileSection.hasTag("./lensCorrectorName");
	
	bool frontBufferRendering=vsync&&lensCorrection&&!configFileSection.retrieveValue<bool>("./useBackBuffer",false);
	bool renderToBuffer=lensCorrection;
	
	/* Create a list of desired visual properties: */
	int visualPropertyList[256];
	int numProperties=0;
	
	/* Add standard properties first: */
	visualPropertyList[numProperties++]=GLX_RGBA;
	
	/* Check if the requested rendering mode requires double buffering: */
	if(!frontBufferRendering)
		visualPropertyList[numProperties++]=GLX_DOUBLEBUFFER;
	
	/* Ask for the requested main buffer channel sizes: */
	visualPropertyList[numProperties++]=GLX_RED_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[0];
	visualPropertyList[numProperties++]=GLX_GREEN_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[1];
	visualPropertyList[numProperties++]=GLX_BLUE_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[2];
	visualPropertyList[numProperties++]=GLX_ALPHA_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[3];
	
	/* All other properties apply to the render buffer, not necessarily the window's visual: */
	if(!renderToBuffer)
		{
		/* Ask for the requested depth buffer size: */
		visualPropertyList[numProperties++]=GLX_DEPTH_SIZE;
		visualPropertyList[numProperties++]=properties.depthBufferSize;
		
		if(properties.numAuxBuffers>0)
			{
			/* Ask for auxiliary buffers: */
			visualPropertyList[numProperties++]=GLX_AUX_BUFFERS;
			visualPropertyList[numProperties++]=properties.numAuxBuffers;
			}
		
		if(properties.stencilBufferSize>0)
			{
			/* Ask for a stencil buffer: */
			visualPropertyList[numProperties++]=GLX_STENCIL_SIZE;
			visualPropertyList[numProperties++]=properties.stencilBufferSize;
			}
		
		/* Check for multisample requests: */
		int multisamplingLevel=configFileSection.retrieveValue<int>("./multisamplingLevel",1);
		if(multisamplingLevel>1)
			{
			visualPropertyList[numProperties++]=GLX_SAMPLE_BUFFERS_ARB;
			visualPropertyList[numProperties++]=1;
			visualPropertyList[numProperties++]=GLX_SAMPLES_ARB;
			visualPropertyList[numProperties++]=multisamplingLevel;
			}
		}
	
	if(properties.accumBufferSize[0]>0||properties.accumBufferSize[1]>0||properties.accumBufferSize[2]>0||properties.accumBufferSize[3]>0)
		{
		/* Ask for an accumulation buffer of the requested channel sizes: */
		visualPropertyList[numProperties++]=GLX_ACCUM_RED_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[0];
		visualPropertyList[numProperties++]=GLX_ACCUM_GREEN_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[1];
		visualPropertyList[numProperties++]=GLX_ACCUM_BLUE_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[2];
		visualPropertyList[numProperties++]=GLX_ACCUM_ALPHA_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[3];
		}
	
	/* Check for quad buffering (active stereo) requests: */
	if(configFileSection.retrieveValue<WindowType>("./windowType")==QUADBUFFER_STEREO)
		visualPropertyList[numProperties++]=GLX_STEREO;
	
	/* Terminate the property list: */
	visualPropertyList[numProperties]=None;
	
	/* Retrieve the display connection name: */
	const char* defaultDisplay=getenv("DISPLAY");
	if(defaultDisplay==0)
		defaultDisplay="";
	std::string displayName=configFileSection.retrieveString("./display",defaultDisplay);
	
	/* Create and return an OpenGL context: */
	return new GLContext(displayName.empty()?0:displayName.c_str(),visualPropertyList);
	}

namespace {

/****************
Helper functions:
****************/

GLWindow::WindowPos getInitialWindowPos(Display* display,const Misc::ConfigurationFileSection& configFileSection)
	{
	GLWindow::WindowPos windowPos;
	
	/* Get the configuration of either the requested or the default output: */
	OutputConfiguration outputConfiguration=getOutputConfiguration(display,configFileSection.retrieveString("./outputName",std::string()).c_str());
	
	/* Read or create a window size: */
	typedef Geometry::ComponentArray<int,2> WindowSize;
	WindowSize windowSize=configFileSection.retrieveValue<WindowSize>("./windowSize",WindowSize(outputConfiguration.domainSize[0]/2,outputConfiguration.domainSize[1]/2));
	
	/* Create a default window position on the found output: */
	for(int i=0;i<2;++i)
		{
		windowPos.size[i]=windowSize[i];
		if(windowPos.size[i]>outputConfiguration.domainSize[i])
			windowPos.size[i]=outputConfiguration.domainSize[i];
		windowPos.origin[i]=outputConfiguration.domainOrigin[i]+(outputConfiguration.domainSize[i]-windowPos.size[i])/2;
		}
	
	/* Override the default window position: */
	windowPos=configFileSection.retrieveValue<GLWindow::WindowPos>("./windowPos",windowPos);
	
	return windowPos;
	}

}

VRWindow::VRWindow(GLContext* sContext,int sScreen,const char* windowName,const Misc::ConfigurationFileSection& configFileSection,VruiState* sVruiState,InputDeviceAdapterMouse* sMouseAdapter)
	:GLWindow(sContext,sScreen,windowName,
	          getInitialWindowPos(sContext->getDisplay(),configFileSection),
	          configFileSection.retrieveValue<bool>("./decorate",true)),
	 vruiState(sVruiState),windowGroup(0),
	 mouseAdapter(sMouseAdapter),
	 clearBufferMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT),
	 vsync(configFileSection.retrieveValue<bool>("./vsync",false)),
	 frontBufferRendering(false),
	 displayState(0),
	 outputName(configFileSection.retrieveString("./outputName",std::string())),
	 xrandrEventBase(0),
	 windowType(configFileSection.retrieveValue<WindowType>("./windowType")),
	 multisamplingLevel(configFileSection.retrieveValue<int>("./multisamplingLevel",1)),
	 panningViewport(configFileSection.retrieveValue<bool>("./panningViewport",false)),
	 navigate(configFileSection.retrieveValue<bool>("./navigate",false)),
	 movePrimaryWidgets(configFileSection.retrieveValue<bool>("./movePrimaryWidgets",false)),
	 hasFramebufferObjectExtension(false),
	 exitKey(KeyMapper::getQualifiedKey(configFileSection.retrieveString("./exitKey","Esc"))),
	 screenshotKey(KeyMapper::getQualifiedKey(configFileSection.retrieveString("./screenshotKey","Super+Print"))),
	 burnModeToggleKey(KeyMapper::getQualifiedKey(configFileSection.retrieveString("./burnModeToggleKey","Super+ScrollLock"))),
	 ivEyeIndexOffset(0),
	 ivRightViewportTextureID(0),ivRightDepthbufferObjectID(0),ivRightFramebufferObjectID(0),
	 asNumViewZones(0),asViewZoneOffset(0),
	 asViewMapTextureID(0),asViewZoneTextureID(0),
	 asDepthBufferObjectID(0),asFrameBufferObjectID(0),
	 asInterzigShader(0),asQuadSizeUniformIndex(-1),
	 lensCorrector(0),
	 mouseScreen(0),
	 showFpsFont(0),
	 showFps(configFileSection.retrieveValue<bool>("./showFps",false)),burnMode(false),
	 protectScreens(configFileSection.retrieveValue<bool>("./protectScreens",true)),
	 trackToolKillZone(false),
	 dirty(true),
	 resizeViewport(true),
	 saveScreenshot(false),
	 movieSaver(0)
	{
	/* Update the X window's event mask: */
	{
	XWindowAttributes wa;
	XGetWindowAttributes(getContext().getDisplay(),getWindow(),&wa);
	XSetWindowAttributes swa;
	swa.event_mask=wa.your_event_mask|FocusChangeMask;
	XChangeWindowAttributes(getContext().getDisplay(),getWindow(),CWEventMask,&swa);
	}
	
	/* Initialize the window mouse position: */
	windowMousePos[0]=windowMousePos[1]=0;
	
	/* Update the clear buffer bit mask: */
	if(vruiState->windowProperties.stencilBufferSize>0)
		clearBufferMask|=GL_STENCIL_BUFFER_BIT;
	if(vruiState->windowProperties.accumBufferSize[0]>0||vruiState->windowProperties.accumBufferSize[1]>0||vruiState->windowProperties.accumBufferSize[2]>0||vruiState->windowProperties.accumBufferSize[3]>0)
		clearBufferMask|=GL_ACCUM_BUFFER_BIT;
	
	/* Check if the window can render directly to the front buffer: */
	bool lensCorrection=configFileSection.hasTag("./lensCorrectorName");
	frontBufferRendering=vsync&&lensCorrection&&!configFileSection.retrieveValue<bool>("./useBackBuffer",false);
	
	/* Get the screen(s) onto which this window projects: */
	screens[0]=findScreen(configFileSection.retrieveString("./leftScreenName","").c_str());
	screens[1]=findScreen(configFileSection.retrieveString("./rightScreenName","").c_str());
	if(screens[0]==0||screens[1]==0)
		{
		/* Get the common screen: */
		screens[0]=screens[1]=findScreen(configFileSection.retrieveString("./screenName").c_str());
		}
	if(screens[0]==0||screens[1]==0)
		Misc::throwStdErr("VRWindow::VRWindow: No screen(s) provided");
	
	/* Get the viewer(s) observing this window: */
	viewers[0]=findViewer(configFileSection.retrieveString("./leftViewerName","").c_str());
	viewers[1]=findViewer(configFileSection.retrieveString("./rightViewerName","").c_str());
	if(viewers[0]==0||viewers[1]==0)
		{
		/* Get the common viewer: */
		viewers[0]=viewers[1]=findViewer(configFileSection.retrieveString("./viewerName").c_str());
		}
	if(viewers[0]==0||viewers[1]==0)
		Misc::throwStdErr("VRWindow::VRWindow: No viewer(s) provided");
	
	/* Get the configuration of the requested or default output: */
	outputConfiguration=getOutputConfiguration(getContext().getDisplay(),outputName.c_str());
	
	/* Override the panning domain from the configuration file: */
	WindowPos panningDomain=configFileSection.retrieveValue<WindowPos>("./panningDomain",WindowPos(outputConfiguration.domainOrigin,outputConfiguration.domainSize));
	for(int i=0;i<2;++i)
		{
		outputConfiguration.domainOrigin[i]=panningDomain.origin[i];
		outputConfiguration.domainSize[i]=panningDomain.size[i];
		}
	
	#if VRUI_INTERNAL_CONFIG_HAVE_XRANDR
	if(!outputName.empty())
		{
		/* Determine whether XRANDR is supported on the X server: */
		int xrandrErrorBase;
		if(XRRQueryExtension(getContext().getDisplay(),&xrandrEventBase,&xrandrErrorBase))
			{
			/* Request XRANDR events to keep the window in the screen area assigned to the requested output: */
			XRRSelectInput(getContext().getDisplay(),getWindow(),RRScreenChangeNotifyMask);
			}
		else
			xrandrEventBase=0;
		}
	#endif
	
	/* Initialize other window state: */
	for(int i=0;i<2;++i)
		{
		ivTextureSize[i]=0;
		ivTexCoord[i]=0.0f;
		asNumTiles[i]=0;
		asTextureSize[i]=0;
		}
	for(int i=0;i<4;++i)
		ivRightStipplePatterns[i]=0;
	
	/* Check if the window's screen size should be defined based on the X display's real size: */
	if(configFileSection.retrieveValue<bool>("./autoScreenSize",false))
		{
		/* Convert the output's advertised display size from mm to physical units: */
		Scalar w=Scalar(outputConfiguration.sizeMm[0])*getInchFactor()/Scalar(25.4);
		Scalar h=Scalar(outputConfiguration.sizeMm[1])*getInchFactor()/Scalar(25.4);
		
		/* Query the screen's configured size (use mean of both screens, assuming they're the same): */
		Scalar oldSize=Scalar(1);
		for(int i=0;i<2;++i)
			oldSize*=Math::sqrt(Math::sqr(screens[i]->getWidth())+Math::sqr(screens[i]->getHeight()));
		oldSize=Math::sqrt(oldSize);
		
		/* Adjust the size of the screen used by this window: */
		screens[0]->setSize(w,h);
		if(screens[1]!=screens[0])
			screens[1]->setSize(w,h);
		Scalar newSize=Math::sqrt(Math::sqr(w)+Math::sqr(h));
		
		/* Adjust the size of the display environment: */
		setDisplayCenter(getDisplayCenter(),getDisplaySize()*newSize/oldSize);
		
		/* Try activating a fake navigation tool: */
		if(activateNavigationTool(reinterpret_cast<Tool*>(this)))
			{
			/* Adjust the navigation transformation to the new display size: */
			NavTransform nav=NavTransform::translateFromOriginTo(getDisplayCenter());
			nav*=NavTransform::scale(newSize/oldSize);
			nav*=NavTransform::translateToOriginFrom(getDisplayCenter());
			concatenateNavigationTransformationLeft(nav);
			
			/* Deactivate the fake navigation tool: */
			deactivateNavigationTool(reinterpret_cast<Tool*>(this));
			}
		}
	
	/* Bypass the compositor if requested: */
	if(configFileSection.retrieveValue<bool>("./bypassCompositor",false))
		bypassCompositor();
	
	/* Make the window full screen if requested: */
	if(configFileSection.retrieveValue<bool>("./windowFullscreen",false))
		makeFullscreen();
	
	/* Force vertical retrace synchronization on or off: */
	if(vsync)
		{
		if(canVsync(frontBufferRendering))
			{
			if(!frontBufferRendering)
				setVsyncInterval(1);
			}
		else
			std::cerr<<"VRWindow::VRWindow: Vertical retrace synchronization requested but not supported"<<std::endl;
		}
	else if(!frontBufferRendering)
		setVsyncInterval(0);
	
	if(windowType==SPLITVIEWPORT_STEREO)
		{
		/* Read split viewport positions: */
		splitViewportPos[0]=configFileSection.retrieveValue<GLWindow::WindowPos>("./leftViewportPos");
		splitViewportPos[1]=configFileSection.retrieveValue<GLWindow::WindowPos>("./rightViewportPos");
		}
	
	/* Initialize the window's panning viewport state: */
	if(panningViewport)
		{
		/* Adapt the viewports to the size of the window in relation to the size of the display: */
		for(int i=0;i<2;++i)
			{
			viewports[i][0]=Scalar(getWindowOrigin()[0]-panningDomain.origin[0])*screens[i]->getWidth()/Scalar(panningDomain.size[0]);
			viewports[i][1]=Scalar(getWindowOrigin()[0]-panningDomain.origin[0]+getWindowWidth())*screens[i]->getWidth()/Scalar(panningDomain.size[0]);
			viewports[i][2]=Scalar(panningDomain.origin[1]+panningDomain.size[1]-getWindowOrigin()[1]-getWindowHeight())*screens[i]->getHeight()/Scalar(panningDomain.size[1]);
			viewports[i][3]=Scalar(panningDomain.origin[1]+panningDomain.size[1]-getWindowOrigin()[1])*screens[i]->getHeight()/Scalar(panningDomain.size[1]);
			}
		
		/* Calculate the window center and size in physical coordinates: */
		Point windowCenter;
		Scalar windowSize(0);
		for(int i=0;i<2;++i)
			{
			windowCenter[i]=Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
			windowSize+=Math::sqr(viewports[0][i*2+1]-viewports[0][i*2+0]);
			}
		windowCenter[2]=Scalar(0);
		ONTransform screenT=screens[0]->getScreenTransformation();
		windowSize=Math::div2(Math::sqrt(windowSize));
		windowCenter=screenT.transform(windowCenter);
		
		if(navigate)
			{
			/* Navigate such that the previous transformation, which assumed a full-screen window, fits into the actual window: */
			Point screenCenter(Math::div2(screens[0]->getWidth()),Math::div2(screens[0]->getHeight()),Scalar(0));
			screenCenter=screenT.transform(screenCenter);
			
			/* Try activating a fake navigation tool: */
			if(activateNavigationTool(reinterpret_cast<Tool*>(this)))
				{
				/* Scale to fit the old viewport into the new viewport: */
				NavTransform nav=NavTransform::scaleAround(windowCenter,windowSize/getDisplaySize());
				
				/* Translate to move to the new viewport center: */
				nav*=NavTransform::translate(windowCenter-screenCenter);
				
				/* Apply the transformation: */
				concatenateNavigationTransformationLeft(nav);
				
				/* Deactivate the fake navigation tool: */
				deactivateNavigationTool(reinterpret_cast<Tool*>(this));
				}
			}
		
		/* Update the display center and size: */
		setDisplayCenter(windowCenter,windowSize);
		}
	else
		{
		/* Set the viewport dimensions to the full screen: */
		for(int i=0;i<2;++i)
			screens[i]->getViewport(viewports[i]);
		
		/* Tell the window manager that this window should not be resized: */
		XSizeHints normalHints;
		memset(&normalHints,0,sizeof(normalHints));
		normalHints.flags=PMinSize|PMaxSize;
		normalHints.min_width=getWindowWidth();
		normalHints.min_height=getWindowHeight();
		normalHints.max_width=getWindowWidth();
		normalHints.max_height=getWindowHeight();
		XSetWMNormalHints(getContext().getDisplay(),getWindow(),&normalHints);
		}
	
	/* Check if the window is supposed to track the tool manager's tool kill zone: */
	if(configFileSection.hasTag("./toolKillZonePos"))
		{
		/* Read the tool kill zone's relative window position: */
		Geometry::Point<Scalar,2> tkzp=configFileSection.retrieveValue<Geometry::Point<Scalar,2> >("./toolKillZonePos");
		for(int i=0;i<2;++i)
			toolKillZonePos[i]=tkzp[i];
		trackToolKillZone=true;
		
		/* Move the tool kill zone to the desired position: */
		ToolKillZone* toolKillZone=getToolManager()->getToolKillZone();
		Vector toolKillZoneSize=screens[0]->getScreenTransformation().inverseTransform(Vector(toolKillZone->getSize()));
		Point screenPos;
		for(int i=0;i<2;++i)
			{
			Scalar min=viewports[0][2*i+0]+toolKillZoneSize[i]*Scalar(0.5);
			Scalar max=viewports[0][2*i+1]-toolKillZoneSize[i]*Scalar(0.5);
			screenPos[i]=min+(max-min)*toolKillZonePos[i];
			}
		screenPos[2]=Scalar(0);
		toolKillZone->setCenter(screens[0]->getScreenTransformation().transform(screenPos));
		vruiState->navigationTransformationChangedMask|=0x4;
		}
	
	/* Hide mouse cursor and ignore mouse events if the mouse is not used as an input device: */
	if(mouseAdapter==0||!mouseAdapter->needMouseCursor())
		{
		hideCursor();
		if(mouseAdapter==0)
			disableMouseEvents();
		}
	
	/* Initialize the window's OpenGL context: */
	makeCurrent();
	displayState=vruiState->registerContext(getContextData());
	displayState->window=this;
	displayState->eyeIndex=0;
	glViewport(0,0,getWindowWidth(),getWindowHeight());
	glClearColor(getBackgroundColor());
	glClearDepth(1.0);
	if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
		glClearStencil(0x0);
	if(clearBufferMask&GL_ACCUM_BUFFER_BIT)
		glClearAccum(0.0f,0.0f,0.0f,0.0f);
	if(multisamplingLevel>1)
		glEnable(GL_MULTISAMPLE_ARB);
	
	if(windowType==INTERLEAVEDVIEWPORT_STEREO)
		{
		/* Create the viewport buffer texture for the right viewport rendering pass: */
		for(int i=0;i<2;++i)
			{
			for(ivTextureSize[i]=1;ivTextureSize[i]<GLWindow::getWindowSize()[i];ivTextureSize[i]<<=1)
				;
			ivTexCoord[i]=float(GLWindow::getWindowSize()[i])/float(ivTextureSize[i]);
			}
		ivEyeIndexOffset=(GLWindow::getWindowOrigin()[1]%2)*2+(GLWindow::getWindowOrigin()[0]%2);
		glGenTextures(1,&ivRightViewportTextureID);
		glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,ivTextureSize[0],ivTextureSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Check if the local OpenGL supports frame buffer objects: */
		hasFramebufferObjectExtension=GLEXTFramebufferObject::isSupported();
		if(hasFramebufferObjectExtension)
			{
			/* Initialize the extension: */
			GLEXTFramebufferObject::initExtension();
			
			/* Create a depthbuffer object for the right viewport rendering pass: */
			glGenRenderbuffersEXT(1,&ivRightDepthbufferObjectID);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,ivRightDepthbufferObjectID);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,ivTextureSize[0],ivTextureSize[1]);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
			
			/* Create a framebuffer object for the right viewport rendering pass: */
			glGenFramebuffersEXT(1,&ivRightFramebufferObjectID);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,ivRightFramebufferObjectID);
			
			/* Attach the viewport texture and the depthbuffer to the framebuffer: */
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,ivRightViewportTextureID,0);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,ivRightDepthbufferObjectID);
			
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			}
		
		/* Read the target display's interleave pattern: */
		std::string pattern=configFileSection.retrieveValue<std::string>("./interleavePattern","LRRL");
		bool patRight[2][2];
		bool patternValid=pattern.size()==4;
		for(int y=0;patternValid&&y<2;++y)
			for(int x=0;patternValid&&x<2;++x)
				{
				char pat=toupper(pattern[y*2+x]);
				patternValid=pat=='L'||pat=='R';
				patRight[y][x]=pat=='R';
				}
		if(!patternValid)
			Misc::throwStdErr("VRWindow::VRWindow: Invalid interleave pattern %s ",pattern.c_str());
		
		/* Initialize the interleave stipple patterns: */
		for(int yoff=0;yoff<2;++yoff)
			for(int xoff=0;xoff<2;++xoff)
				{
				GLubyte* stipPat=ivRightStipplePatterns[yoff*2+xoff]=new GLubyte[128];
				for(int i=0;i<128;++i)
					stipPat[i]=GLubyte(0);
				for(int y=0;y<32;++y)
					for(int x=0;x<32;++x)
						if(patRight[(y+yoff)%2][(x+xoff)%2])
							stipPat[y*4+x/8]|=0x1U<<(x%8);
				}
		}
	else if(windowType==AUTOSTEREOSCOPIC_STEREO)
		{
		try
			{
			/* Initialize the required OpenGL extensions: */
			GLARBMultitexture::initExtension();
			GLShader::initExtensions();
			}
		catch(std::runtime_error err)
			{
			Misc::throwStdErr("VRWindow::VRWindow: Unable to set mode AutoStereoscopicStereo due to exception %s",err.what());
			}
		
		/* Read the number of view zones and the view zone offset: */
		asNumViewZones=configFileSection.retrieveValue<int>("./autostereoNumViewZones");
		asViewZoneOffset=configFileSection.retrieveValue<Scalar>("./autostereoViewZoneOffset");
		
		/* Get the number of view zone tile columns: */
		asNumTiles[0]=configFileSection.retrieveValue<int>("./autostereoNumTileColumns");
		asNumTiles[1]=(asNumViewZones+asNumTiles[0]-1)/asNumTiles[0];
		
		/* Determine the texture size required to cover the entire screen: */
		WindowPos rootPos=getRootWindowPos();
		for(int i=0;i<2;++i)
			for(asTextureSize[i]=1;asTextureSize[i]<rootPos.size[i];asTextureSize[i]<<=1)
				;
		
		/* Get the name of the view map image: */
		std::string viewMapImageName=configFileSection.retrieveValue<std::string>("./autostereoViewMapImageName");
		
		/* Go to the standard directory if none specified: */
		if(viewMapImageName[0]!='/')
			viewMapImageName=std::string(VRUI_INTERNAL_CONFIG_SHAREDIR)+"/Textures/"+viewMapImageName;
		
		/* Load the view map: */
		Images::RGBImage viewMap=Images::readImageFile(viewMapImageName.c_str());
		if(int(viewMap.getSize(0))!=rootPos.size[0]||int(viewMap.getSize(1))!=rootPos.size[1])
			Misc::throwStdErr("VRWindow::VRWindow: View map image size does not match display size");
		
		/* Upload the view map texture (pad to power of two size): */
		glGenTextures(1,&asViewMapTextureID);
		glBindTexture(GL_TEXTURE_2D,asViewMapTextureID);
		viewMap.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,true);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		
		/* Create the view zone texture: */
		glGenTextures(1,&asViewZoneTextureID);
		glBindTexture(GL_TEXTURE_2D,asViewZoneTextureID);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,asTextureSize[0],asTextureSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Check if the local OpenGL supports frame buffer objects: */
		hasFramebufferObjectExtension=GLEXTFramebufferObject::isSupported();
		if(hasFramebufferObjectExtension)
			{
			/* Initialize the extension: */
			GLEXTFramebufferObject::initExtension();
			
			/* Generate a depth buffer object for the view zone rendering pass: */
			glGenRenderbuffersEXT(1,&asDepthBufferObjectID);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,asDepthBufferObjectID);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,asTextureSize[0],asTextureSize[1]);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
			
			/* Generate a frame buffer object for the view zone rendering pass: */
			glGenFramebuffersEXT(1,&asFrameBufferObjectID);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,asFrameBufferObjectID);
			
			/* Attach the view zone texture and the depth buffer to the frame buffer: */
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,asViewZoneTextureID,0);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,asDepthBufferObjectID);
			
			/* Unbind the frame buffer object for now: */
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			}
		
		/* Initialize the interzigging shader: */
		asInterzigShader=new GLShader;
		std::string asVertexShaderName=VRUI_INTERNAL_CONFIG_SHAREDIR;
		asVertexShaderName.append("/Textures/InterzigShader.vs");
		asInterzigShader->compileVertexShader(asVertexShaderName.c_str());
		std::string asFragmentShaderName=VRUI_INTERNAL_CONFIG_SHAREDIR;
		asFragmentShaderName.append("/Textures/InterzigShader.fs");
		asInterzigShader->compileFragmentShader(asFragmentShaderName.c_str());
		asInterzigShader->linkShader();
		asQuadSizeUniformIndex=asInterzigShader->getUniformLocation("quadSize");
		if(asQuadSizeUniformIndex<0)
			Misc::throwStdErr("VRWindow::VRWindow: Interzigging shader does not define quadSize variable");
		}
	
	/* Check if the window is supposed to perform post-rendering lens distortion correction: */
	if(lensCorrection)
		{
		/* Create a lens distortion correction object: */
		lensCorrector=new LensCorrector(*this,vruiState->windowProperties,multisamplingLevel,splitViewportPos,configFileSection.getSection(configFileSection.retrieveString("./lensCorrectorName").c_str()));
		}
	
	/* Check if the window has a dedicated mouse mapping screen: */
	if(configFileSection.hasTag("./mouseScreenName"))
		{
		std::string mouseScreenName=configFileSection.retrieveString("./mouseScreenName");
		mouseScreen=findScreen(mouseScreenName.c_str());
		if(mouseScreen==0)
			Misc::throwStdErr("VRWindow::VRWindow: Screen %s does not exist",mouseScreenName.c_str());
		}
	
	if(showFps)
		{
		/* Load font: */
		showFpsFont=loadFont(configFileSection.retrieveString("./showFpsFontName","HelveticaMediumUpright").c_str());
		GLfloat textHeight=showFpsFont->getTextPixelHeight()-1.0f;
		if(textHeight>16.0f)
			textHeight=16.0f;
		showFpsFont->setTextHeight(textHeight);
		GLFont::Color bg=getBackgroundColor();
		showFpsFont->setBackgroundColor(bg);
		GLFont::Color fg;
		for(int i=0;i<3;++i)
			fg[i]=1.0f-bg[i];
		fg[3]=bg[3];
		showFpsFont->setForegroundColor(fg);
		showFpsFont->setHAlignment(GLFont::Right);
		showFpsFont->setVAlignment(GLFont::Bottom);
		showFpsFont->setAntialiasing(false);
		// showFpsFont->createCharTextures(*contextData);
		}
	
	#ifdef VRWINDOW_USE_SWAPGROUPS
	/* Join a swap group if requested: */
	if(configFileSection.retrieveValue<bool>("./joinSwapGroup",false))
		{
		GLuint maxSwapGroupName,maxSwapBarrierName;
		glXQueryMaxSwapGroupsNV(getContext().getDisplay(),getScreen(),&maxSwapGroupName,&maxSwapBarrierName);
		GLuint swapGroupName=configFileSection.retrieveValue<GLuint>("./swapGroupName",0);
		if(swapGroupName>maxSwapGroupName)
			Misc::throwStdErr("VRWindow::VRWindow: Swap group name %u larger than maximum %u",swapGroupName,maxSwapGroupName);
		GLuint swapBarrierName=configFileSection.retrieveValue<GLuint>("./swapBarrierName",0);
		if(swapBarrierName>maxSwapBarrierName)
			Misc::throwStdErr("VRWindow::VRWindow: Swap barrier name %u larger than maximum %u",swapBarrierName,maxSwapBarrierName);
		if(!glXJoinSwapGroupNV(getContext().getDisplay(),getWindow(),swapGroupName))
			Misc::throwStdErr("VRWindow::VRWindow: Unable to join swap group %u",swapGroupName);
		if(!glXBindSwapBarrierNV(getContext().getDisplay(),swapGroupName,swapBarrierName))
			Misc::throwStdErr("VRWindow::VRWindow: Unable to bind swap barrier %u",swapBarrierName);
		}
	#endif
	
	/* Check if the window is supposed to save a movie: */
	if(configFileSection.retrieveValue<bool>("./saveMovie",false))
		{
		/* Create a movie saver object: */
		movieSaver=MovieSaver::createMovieSaver(configFileSection);
		}
	}

VRWindow::~VRWindow(void)
	{
	delete movieSaver;
	}

void VRWindow::setWindowGroup(VruiWindowGroup* newWindowGroup)
	{
	/* Store the window group association: */
	windowGroup=newWindowGroup;
	
	/* Immediately advertise the current viewport and frame buffer size: */
	int viewportSize[2];
	if(windowType==SPLITVIEWPORT_STEREO)
		{
		for(int i=0;i<2;++i)
			viewportSize[i]=Math::max(splitViewportPos[0].size[i],splitViewportPos[1].size[i]);
		}
	else
		{
		for(int i=0;i<2;++i)
			viewportSize[i]=getWindowSize()[i];
		}
	int frameSize[2];
	if(lensCorrector!=0)
		{
		for(int i=0;i<2;++i)
			frameSize[i]=lensCorrector->getPredistortionFrameSize()[i];
		}
	else
		{
		for(int i=0;i<2;++i)
			frameSize[i]=getWindowSize()[i];
		}
	
	resizeWindow(windowGroup,this,viewportSize,frameSize);
	}

void VRWindow::setVRScreen(int screenIndex,VRScreen* newScreen)
	{
	screens[screenIndex]=newScreen;
	}

void VRWindow::setVRScreen(VRScreen* newScreen)
	{
	/* Set both screens to the given screen: */
	screens[0]=screens[1]=newScreen;
	}

void VRWindow::setScreenViewport(const Scalar newViewport[4])
	{
	/* Update both viewports: */
	for(int i=0;i<4;++i)
		viewports[0][i]=viewports[1][i]=newViewport[i];
	}

void VRWindow::setViewer(int viewerIndex,Viewer* newViewer)
	{
	viewers[viewerIndex]=newViewer;
	}

void VRWindow::setViewer(Viewer* newViewer)
	{
	/* Set both viewers to the given viewer: */
	viewers[0]=viewers[1]=newViewer;
	}

void VRWindow::deinit(void)
	{
	makeCurrent();
	if(windowType==INTERLEAVEDVIEWPORT_STEREO)
		{
		if(hasFramebufferObjectExtension)
			{
			glDeleteFramebuffersEXT(1,&ivRightFramebufferObjectID);
			glDeleteRenderbuffersEXT(1,&ivRightDepthbufferObjectID);
			}
		glDeleteTextures(1,&ivRightViewportTextureID);
		for(int i=0;i<4;++i)
			delete[] ivRightStipplePatterns[i];
		}
	else if(windowType==AUTOSTEREOSCOPIC_STEREO)
		{
		delete asInterzigShader;
		if(hasFramebufferObjectExtension)
			{
			glDeleteFramebuffersEXT(1,&asFrameBufferObjectID);
			glDeleteRenderbuffersEXT(1,&asDepthBufferObjectID);
			}
		glDeleteTextures(1,&asViewZoneTextureID);
		glDeleteTextures(1,&asViewMapTextureID);
		}
	delete lensCorrector;
	delete showFpsFont;
	}

int VRWindow::getNumEyes(void) const
	{
	switch(windowType)
		{
		case MONO:
		case LEFT:
		case RIGHT:
			return 1;
		
		case AUTOSTEREOSCOPIC_STEREO:
			return asNumViewZones;
		
		default:
			return 2;
		}
	}

Point VRWindow::getEyePosition(int eyeIndex) const
	{
	switch(windowType)
		{
		case MONO:
			return viewers[0]->getEyePosition(Viewer::MONO);
		
		case LEFT:
			return viewers[0]->getEyePosition(Viewer::LEFT);
		
		case RIGHT:
			return viewers[1]->getEyePosition(Viewer::RIGHT);
		
		case AUTOSTEREOSCOPIC_STEREO:
			{
			Point asEye=viewers[0]->getEyePosition(Viewer::MONO);
			Vector asViewZoneOffsetVector=screens[0]->getScreenTransformation().inverseTransform(Vector(asViewZoneOffset,0,0));
			asEye+=asViewZoneOffsetVector*(Scalar(eyeIndex)-Math::div2(Scalar(asNumViewZones-1)));
			return asEye;
			}
		
		default:
			if(eyeIndex==0)
				return viewers[0]->getEyePosition(Viewer::LEFT);
			else
				return viewers[1]->getEyePosition(Viewer::RIGHT);
		}
	}

void VRWindow::updateMouseDevice(const int windowPos[2],InputDevice* mouse) const
	{
	/* Find the screen and viewer responsible for the given window position and transform the window position to screen coordinates: */
	int viewport=0;
	const VRScreen* screen;
	Geometry::Point<Scalar,2> screenPos;
	if(windowType==SPLITVIEWPORT_STEREO)
		{
		/* Check which viewport contains the given window position: */
		if(splitViewportPos[1].contains(windowPos))
			viewport=1;
		
		/* Convert the window position to screen coordinates: */
		if(mouseScreen!=0)
			{
			screen=mouseScreen;
			screenPos[0]=(Scalar(windowPos[0])+Scalar(0.5))*mouseScreen->getWidth()/getWindowWidth();
			screenPos[1]=(Scalar(getWindowHeight()-windowPos[1])-Scalar(0.5))*mouseScreen->getHeight()/getWindowHeight();
			}
		else
			{
			screen=screens[viewport];
			screenPos[0]=(Scalar(windowPos[0]-splitViewportPos[viewport].origin[0])+Scalar(0.5))*screen->getWidth()/Scalar(splitViewportPos[viewport].size[0]);
			screenPos[1]=(Scalar(splitViewportPos[viewport].origin[1]+splitViewportPos[viewport].size[1]-windowPos[1])-Scalar(0.5))*screen->getHeight()/Scalar(splitViewportPos[viewport].size[1]);
			}
		}
	else
		{
		/* Convert the window position to screen coordinates: */
		if(mouseScreen!=0)
			{
			screen=mouseScreen;
			screenPos[0]=(Scalar(windowPos[0])+Scalar(0.5))*mouseScreen->getWidth()/getWindowWidth();
			screenPos[1]=(Scalar(getWindowHeight()-windowPos[1])-Scalar(0.5))*mouseScreen->getHeight()/getWindowHeight();
			}
		else if(panningViewport)
			{
			screen=screens[viewport];
			screenPos[0]=(Scalar(getWindowOrigin()[0]-outputConfiguration.domainOrigin[0]+windowPos[0])+Scalar(0.5))*screen->getWidth()/Scalar(outputConfiguration.domainSize[0]);
			screenPos[1]=(Scalar(outputConfiguration.domainOrigin[1]+outputConfiguration.domainSize[1]-getWindowOrigin()[1]-windowPos[1])-Scalar(0.5))*screen->getHeight()/Scalar(outputConfiguration.domainSize[1]);
			}
		else
			{
			screen=screens[viewport];
			screenPos[0]=(Scalar(windowPos[0])+Scalar(0.5))*screen->getWidth()/getWindowWidth();
			screenPos[1]=(Scalar(getWindowHeight()-windowPos[1])-Scalar(0.5))*screen->getHeight()/getWindowHeight();
			}
		}
	
	/* Check if the screen is projected off-axis: */
	if(screen->isOffAxis())
		{
		/* Transform the window position from rectified screen coordinates to projected screen coordinates: */
		screenPos=screen->getScreenHomography().transform(screenPos);
		}
	
	/* Get the current screen transformation: */
	ONTransform screenT=screen->getScreenTransformation();
	
	/* Set the mouse device's position and orientation: */
	ONTransform mouseT(screenT.transform(Point(screenPos[0],screenPos[1],Scalar(0)))-Point::origin,screenT.getRotation()*Rotation::rotateX(Math::rad(-90.0)));
	
	/* Transform the eye position to screen coordinates: */
	Point screenEyePos=screenT.inverseTransform(viewers[viewport]->getEyePosition(Viewer::MONO));
	
	/* Calculate the mouse device's ray direction in device, i.e., rotated screen coordinates: */
	Vector mouseRayDir(screenPos[0]-screenEyePos[0],screenEyePos[2],screenPos[1]-screenEyePos[1]);
	Scalar mouseRayLen=Geometry::mag(mouseRayDir);
	mouseRayDir/=mouseRayLen;
	Scalar mouseRayStart=-mouseRayLen;
	
	/* Set the mouse device: */
	mouse->setDeviceRay(mouseRayDir,mouseRayStart);
	mouse->setTransformation(mouseT);
	}

ViewSpecification VRWindow::calcViewSpec(int eyeIndex) const
	{
	/* Prepare the result viewing specification: */
	ViewSpecification result;
	
	/* Set the viewport size: */
	result.setViewportSize(getViewportSize());
	
	/* Get the screen's coordinate system: */
	ATransform screenT=screens[eyeIndex]->getScreenTransformation();
	
	/* Calculate helper vectors/points: */
	Scalar l=viewports[eyeIndex][0];
	Scalar r=viewports[eyeIndex][1];
	Scalar b=viewports[eyeIndex][2];
	Scalar t=viewports[eyeIndex][3];
	Scalar lr=Math::mid(l,r);
	Scalar bt=Math::mid(b,t);
	Vector screenX=screenT.getDirection(0);
	Vector screenY=screenT.getDirection(1);
	Vector screenZ=screenT.getDirection(2);
	Point left=screenT.transform(Point(l,bt,0));
	Point right=screenT.transform(Point(r,bt,0));
	Point bottom=screenT.transform(Point(lr,b,0));
	Point top=screenT.transform(Point(lr,t,0));
	
	/* Set the screen plane: */
	result.setScreenPlane(Plane(screenZ,screenT.getOrigin()));
	
	/* Set the screen size: */
	Scalar screenSize[2];
	screenSize[0]=r-l;
	screenSize[1]=t-b;
	result.setScreenSize(screenSize);
	
	/* Get the eye position in physical coordinates: */
	Point eye=getEyePosition(eyeIndex);
	result.setEye(eye);
	
	/* Get the z coordinate of the eye in screen space: */
	Scalar eyeZ=(eye-left)*screenZ;
	result.setEyeScreenDistance(eyeZ);
	
	/* Calculate the six frustum face planes: */
	result.setFrustumPlane(0,Plane(screenY^(eye-left),left));
	result.setFrustumPlane(1,Plane((eye-right)^screenY,right));
	result.setFrustumPlane(2,Plane((eye-bottom)^screenX,bottom));
	result.setFrustumPlane(3,Plane(screenX^(eye-top),top));
	result.setFrustumPlane(4,Plane(-screenZ,eye-screenZ*getFrontplaneDist()));
	result.setFrustumPlane(5,Plane(screenZ,eye-screenZ*getBackplaneDist()));
	
	/* Calculate the eight frustum corner vertices: */
	Point vertex0=screenT.transform(Point(l,b,0));
	Point vertex1=screenT.transform(Point(r,b,0));
	Point vertex2=screenT.transform(Point(l,t,0));
	Point vertex3=screenT.transform(Point(r,t,0));
	Scalar frontLambda=getFrontplaneDist()/eyeZ;
	result.setFrustumVertex(0,Geometry::affineCombination(eye,vertex0,frontLambda));
	result.setFrustumVertex(1,Geometry::affineCombination(eye,vertex1,frontLambda));
	result.setFrustumVertex(2,Geometry::affineCombination(eye,vertex2,frontLambda));
	result.setFrustumVertex(3,Geometry::affineCombination(eye,vertex3,frontLambda));
	Scalar backLambda=getBackplaneDist()/eyeZ;
	result.setFrustumVertex(4,Geometry::affineCombination(eye,vertex0,backLambda));
	result.setFrustumVertex(5,Geometry::affineCombination(eye,vertex1,backLambda));
	result.setFrustumVertex(6,Geometry::affineCombination(eye,vertex2,backLambda));
	result.setFrustumVertex(7,Geometry::affineCombination(eye,vertex3,backLambda));
	
	return result;
	}

bool VRWindow::processEvent(const XEvent& event)
	{
	bool stopProcessing=false;
	
	#if VRUI_INTERNAL_CONFIG_HAVE_XRANDR
	if(xrandrEventBase!=0&&event.type==xrandrEventBase+RRScreenChangeNotify)
		{
		/* Tell Xlib that the screen resolution or layout changed: */
		XRRUpdateConfiguration(const_cast<XEvent*>(&event));
		
		/* Query the new screen layout: */
		OutputConfiguration newOc=getOutputConfiguration(getContext().getDisplay(),outputName.c_str());
		
		/* Do something clever by moving the window from the old to the new display position etc.: */
		WindowPos oldWp=getWindowPos();
		WindowPos newWp;
		for(int i=0;i<2;++i)
			{
			newWp.size[i]=(oldWp.size[i]*newOc.domainSize[i]+outputConfiguration.domainSize[i]/2)/outputConfiguration.domainSize[i];
			newWp.origin[i]=((oldWp.origin[i]-outputConfiguration.domainOrigin[i])*newOc.domainSize[i]+outputConfiguration.domainSize[i]/2)/outputConfiguration.domainSize[i]+newOc.domainOrigin[i];
			}
		
		/* Store the new output configuration: */
		outputConfiguration=newOc;
		
		/* Request the window's new position and size: */
		setWindowPos(newWp);
		}
	#endif
	
	switch(event.type)
		{
		case Expose:
		case GraphicsExpose:
			/* Window must be redrawn: */
			dirty=true;
			break;
		
		case ConfigureNotify:
			{
			/* Call the base class' event handler: */
			GLWindow::processEvent(event);
			
			if(panningViewport)
				{
				/* Compute a translation from the old viewport center to the new viewport center: */
				Vector translate;
				for(int i=0;i<2;++i)
					translate[i]=-Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
				translate[2]=Scalar(0);
				
				/* Update the window's viewport: */
				for(int i=0;i<2;++i)
					{
					viewports[i][0]=Scalar(getWindowOrigin()[0]-outputConfiguration.domainOrigin[0])*screens[i]->getWidth()/Scalar(outputConfiguration.domainSize[0]);
					viewports[i][1]=Scalar(getWindowOrigin()[0]-outputConfiguration.domainOrigin[0]+getWindowWidth())*screens[i]->getWidth()/Scalar(outputConfiguration.domainSize[0]);
					viewports[i][2]=Scalar(outputConfiguration.domainOrigin[1]+outputConfiguration.domainSize[1]-getWindowOrigin()[1]-getWindowHeight())*screens[i]->getHeight()/Scalar(outputConfiguration.domainSize[1]);
					viewports[i][3]=Scalar(outputConfiguration.domainOrigin[1]+outputConfiguration.domainSize[1]-getWindowOrigin()[1])*screens[i]->getHeight()/Scalar(outputConfiguration.domainSize[1]);
					}
				
				/* Compute the new viewport center and size: */
				ONTransform screenT=screens[0]->getScreenTransformation();
				Point newCenter;
				Scalar newSize(0);
				for(int i=0;i<2;++i)
					{
					newCenter[i]=Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
					translate[i]+=newCenter[i];
					newSize+=Math::sqr(viewports[0][i*2+1]-viewports[0][i*2+0]);
					}
				newCenter[2]=Scalar(0);
				newCenter=screenT.transform(newCenter);
				translate=screenT.transform(translate);
				newSize=Math::div2(Math::sqrt(newSize));
				
				if(navigate)
					{
					/* Try activating a fake navigation tool: */
					if(activateNavigationTool(reinterpret_cast<Tool*>(this)))
						{
						/* Scale to fit the old viewport into the new viewport: */
						NavTransform nav=NavTransform::scaleAround(newCenter,newSize/getDisplaySize());
						
						/* Translate navigation coordinates to move the display with the window: */
						nav*=NavTransform::translate(translate);
						
						/* Apply the transformation: */
						concatenateNavigationTransformationLeft(nav);
						
						/* Deactivate the fake navigation tool: */
						deactivateNavigationTool(reinterpret_cast<Tool*>(this));
						}
					}
				
				if(movePrimaryWidgets)
					{
					/* Move all primary popped-up widgets with the window: */
					GLMotif::WidgetManager* wm=getWidgetManager();
					GLMotif::WidgetManager::Transformation wt=GLMotif::WidgetManager::Transformation::translate(translate);
					for(GLMotif::WidgetManager::PoppedWidgetIterator wIt=wm->beginPrimaryWidgets();wIt!=wm->endPrimaryWidgets();++wIt)
						wIt.setWidgetToWorld(wt*wIt.getWidgetToWorld());
					}
				
				/* Update the display center and size: */
				setDisplayCenter(newCenter,newSize);
				
				requestUpdate();
				}
			
			/* Remember to resize the window's viewport on the next draw() call: */
			resizeViewport=true;
			
			if(trackToolKillZone)
				{
				/* Move the tool kill zone to its intended position in relative window coordinates: */
				ToolKillZone* toolKillZone=getToolManager()->getToolKillZone();
				Vector toolKillZoneSize=screens[0]->getScreenTransformation().inverseTransform(Vector(toolKillZone->getSize()));
				Point screenPos;
				for(int i=0;i<2;++i)
					{
					Scalar min=viewports[0][2*i+0]+toolKillZoneSize[i]*Scalar(0.5);
					Scalar max=viewports[0][2*i+1]-toolKillZoneSize[i]*Scalar(0.5);
					screenPos[i]=min+(max-min)*toolKillZonePos[i];
					}
				screenPos[2]=Scalar(0);
				toolKillZone->setCenter(screens[0]->getScreenTransformation().transform(screenPos));
				vruiState->navigationTransformationChangedMask|=0x4;
				}
			
			if(windowType==INTERLEAVEDVIEWPORT_STEREO)
				{
				/* Reallocate the off-screen buffers: */
				bool mustReallocate=false;
				for(int i=0;i<2;++i)
					{
					int newTextureSize;
					for(newTextureSize=1;newTextureSize<GLWindow::getWindowSize()[i];newTextureSize<<=1)
						;
					ivTexCoord[i]=float(GLWindow::getWindowSize()[i])/float(newTextureSize);
					if(ivTextureSize[i]!=newTextureSize)
						mustReallocate=true;
					ivTextureSize[i]=newTextureSize;
					}
				ivEyeIndexOffset=(GLWindow::getWindowOrigin()[1]%2)*2+(GLWindow::getWindowOrigin()[0]%2);
				
				if(mustReallocate)
					{
					/* Reallocate the right viewport texture: */
					glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
					glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,ivTextureSize[0],ivTextureSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
					glBindTexture(GL_TEXTURE_2D,0);
					
					if(hasFramebufferObjectExtension)
						{
						/* Reallocate the depthbuffer: */
						glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,ivRightDepthbufferObjectID);
						glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,ivTextureSize[0],ivTextureSize[1]);
						glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
						}
					}
				}
			
			if(windowGroup!=0)
				{
				/* Notify the Vrui run-time that the window size has changed: */
				setWindowGroup(windowGroup); // Lazy, lazy...
				}
			
			break;
			}
		
		case MotionNotify:
			if(mouseAdapter!=0)
				{
				/* Remember window-coordinate mouse position: */
				windowMousePos[0]=event.xmotion.x;
				windowMousePos[1]=event.xmotion.y;
				
				/* Set mouse position in input device adapter: */
				mouseAdapter->setMousePosition(this,windowMousePos);
				}
			break;
		
		case ButtonPress:
		case ButtonRelease:
			if(mouseAdapter!=0)
				{
				/* Remember window-coordinate mouse position: */
				windowMousePos[0]=event.xbutton.x;
				windowMousePos[1]=event.xbutton.y;
				
				/* Set mouse position in input device adapter: */
				mouseAdapter->setMousePosition(this,windowMousePos);
				
				/* Set the state of the appropriate button in the input device adapter: */
				bool newState=event.type==ButtonPress;
				if(event.xbutton.button<4)
					stopProcessing=mouseAdapter->setButtonState(event.xbutton.button-1,newState);
				else if(event.xbutton.button==4)
					{
					if(newState)
						mouseAdapter->incMouseWheelTicks();
					}
				else if(event.xbutton.button==5)
					{
					if(newState)
						mouseAdapter->decMouseWheelTicks();
					}
				else if(event.xbutton.button>5)
					stopProcessing=mouseAdapter->setButtonState(event.xbutton.button-3,newState);
				}
			break;
		
		case KeyPress:
		case KeyRelease:
			{
			if(mouseAdapter!=0)
				{
				/* Remember window-coordinate mouse position: */
				windowMousePos[0]=event.xbutton.x;
				windowMousePos[1]=event.xbutton.y;
				
				/* Set mouse position in input device adapter: */
				mouseAdapter->setMousePosition(this,windowMousePos);
				}
			
			/* Convert event key index to keysym: */
			char keyString[20];
			KeySym keySym;
			XKeyEvent keyEvent=event.xkey;
			
			/* Use string lookup method to get proper key value for text events: */
			int keyStringLen=XLookupString(&keyEvent,keyString,sizeof(keyString),&keySym,0);
			keyString[keyStringLen]='\0';
			
			/* Use keysym lookup a second time to get raw key code ignoring modifier keys: */
			keySym=XLookupKeysym(&keyEvent,0);
			
			if(event.type==KeyPress)
				{
				/* Handle Vrui application keys: */
				if(exitKey.matches(keySym,keyEvent.state))
					{
					/* Call the window close callbacks: */
					Misc::CallbackData cbData;
					getCloseCallbacks().call(&cbData);
					stopProcessing=true;
					}
				else if(screenshotKey.matches(keySym,keyEvent.state))
					{
					saveScreenshot=true;
					char numberedFileName[256];
					#if IMAGES_CONFIG_HAVE_PNG
					/* Save the screenshot as a PNG file: */
					screenshotImageFileName=Misc::createNumberedFileName("VruiScreenshot.png",4,numberedFileName);
					#else
					/* Save the screenshot as a PPM file: */
					screenshotImageFileName=Misc::createNumberedFileName("VruiScreenshot.ppm",4,numberedFileName);
					#endif
					
					/* Write a confirmation message: */
					std::cout<<"Saving window contents as "<<screenshotImageFileName<<std::endl;
					}
				else if(burnModeToggleKey.matches(keySym,keyEvent.state))
					{
					/* Enter or leave burn mode: */
					if(burnMode)
						{
						double burnModeTime=getApplicationTime()-burnModeStartTime;
						std::cout<<"Leaving burn mode: "<<burnModeNumFrames<<" frames in "<<burnModeTime*1000.0<<" ms, averaging "<<double(burnModeNumFrames)/burnModeTime<<" fps"<<std::endl;
						burnMode=false;
						}
					else
						{
						std::cout<<"Entering burn mode"<<std::endl;
						burnMode=true;
						burnModeNumFrames=~0U;
						}
					}
				
				if(mouseAdapter!=0)
					{
					/* Send key event to input device adapter: */
					stopProcessing=mouseAdapter->keyPressed(keySym,keyEvent.state,keyString);
					}
				}
			else
				{
				if(mouseAdapter!=0)
					{
					/* Send key event to input device adapter: */
					stopProcessing=mouseAdapter->keyReleased(keySym);
					}
				}
			break;
			}
		
		case FocusIn:
			if(panningViewport)
				{
				/* Compute the new viewport center and size: */
				ONTransform screenT=screens[0]->getScreenTransformation();
				Point newCenter;
				Scalar newSize(0);
				for(int i=0;i<2;++i)
					{
					newCenter[i]=Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
					newSize+=Math::sqr(viewports[0][i*2+1]-viewports[0][i*2+0]);
					}
				newCenter[2]=Scalar(0);
				newCenter=screenT.transform(newCenter);
				newSize=Math::div2(Math::sqrt(newSize));
				
				/* Update the display center and size: */
				setDisplayCenter(newCenter,newSize);
				
				requestUpdate();
				}
			
			if(trackToolKillZone)
				{
				/* Move the tool kill zone to its intended position in relative window coordinates: */
				ToolKillZone* toolKillZone=getToolManager()->getToolKillZone();
				Vector toolKillZoneSize=screens[0]->getScreenTransformation().inverseTransform(Vector(toolKillZone->getSize()));
				Point screenPos;
				for(int i=0;i<2;++i)
					{
					Scalar min=viewports[0][2*i+0]+toolKillZoneSize[i]*Scalar(0.5);
					Scalar max=viewports[0][2*i+1]-toolKillZoneSize[i]*Scalar(0.5);
					screenPos[i]=min+(max-min)*toolKillZonePos[i];
					}
				screenPos[2]=Scalar(0);
				toolKillZone->setCenter(screens[0]->getScreenTransformation().transform(screenPos));
				vruiState->navigationTransformationChangedMask|=0x4;
				}
			
			if(mouseAdapter!=0)
				{
				/* Create a fake XKeymap event: */
				XKeymapEvent keymapEvent;
				keymapEvent.type=KeymapNotify;
				keymapEvent.serial=event.xcrossing.serial;
				keymapEvent.send_event=event.xcrossing.send_event;
				keymapEvent.display=event.xcrossing.display;
				keymapEvent.window=event.xcrossing.window;
				
				/* Query the current key map: */
				XQueryKeymap(getContext().getDisplay(),keymapEvent.key_vector);
				
				/* Reset the input device adapter's key states: */
				mouseAdapter->resetKeys(keymapEvent);
				}
			break;
		
		default:
			/* Call base class method: */
			GLWindow::processEvent(event);
		}
	
	return stopProcessing;
	}

void VRWindow::requestScreenshot(const char* sScreenshotImageFileName)
	{
	/* Set the screenshot flag and remember the given image file name: */
	saveScreenshot=true;
	screenshotImageFileName=sScreenshotImageFileName;
	}

void VRWindow::draw(void)
	{
	/* Update the window's display state: */
	getMaxWindowSizes(windowGroup,displayState->maxViewportSize,displayState->maxFrameSize);
	
	/* Activate the window's OpenGL context: */
	makeCurrent();
	
	/* Check if the window's viewport needs to be resized: */
	if(resizeViewport)
		{
		/* Resize the GL viewport: */
		glViewport(0,0,getWindowWidth(),getWindowHeight());
		
		/* Clear the entire window (important if the usual rendering mode masks part of the frame buffer): */
		glDisable(GL_SCISSOR_TEST);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		glDepthMask(GL_TRUE);
		if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
			glStencilMask(~0x0U);
		glClear(clearBufferMask);
		}
	
	/* Update things in the window's GL context data: */
	getContextData().updateThings();
	
	/* Draw the window's contents: */
	GLWindow::WindowPos windowViewport(getWindowWidth(),getWindowHeight());
	GLenum drawBuffer=frontBufferRendering?GL_FRONT:GL_BACK;
	switch(windowType)
		{
		case MONO:
			/* Render both-eyes view: */
			glDrawBuffer(drawBuffer);
			glReadBuffer(drawBuffer);
			render(windowViewport,0,viewers[0]->getEyePosition(Viewer::MONO));
			break;
		
		case LEFT:
			/* Render left-eye view: */
			glDrawBuffer(drawBuffer);
			glReadBuffer(drawBuffer);
			render(windowViewport,0,viewers[0]->getEyePosition(Viewer::LEFT));
			break;
		
		case RIGHT:
			/* Render right-eye view: */
			glDrawBuffer(drawBuffer);
			glReadBuffer(drawBuffer);
			render(windowViewport,1,viewers[1]->getEyePosition(Viewer::RIGHT));
			break;
		
		case QUADBUFFER_STEREO:
			/* Render left-eye view: */
			glDrawBuffer(frontBufferRendering?GL_FRONT_LEFT:GL_BACK_LEFT);
			glReadBuffer(frontBufferRendering?GL_FRONT_LEFT:GL_BACK_LEFT);
			displayState->eyeIndex=0;
			render(windowViewport,0,viewers[0]->getEyePosition(Viewer::LEFT));
			
			/* Render right-eye view: */
			glDrawBuffer(frontBufferRendering?GL_FRONT_RIGHT:GL_BACK_RIGHT);
			glReadBuffer(frontBufferRendering?GL_FRONT_RIGHT:GL_BACK_RIGHT);
			displayState->eyeIndex=1;
			render(windowViewport,1,viewers[1]->getEyePosition(Viewer::RIGHT));
			break;
		
		case ANAGLYPHIC_STEREO:
			glDrawBuffer(drawBuffer);
			glReadBuffer(drawBuffer);
			
			/* Render left-eye view: */
			glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
			displayState->eyeIndex=0;
			render(windowViewport,0,viewers[0]->getEyePosition(Viewer::LEFT));
			
			/* Render right-eye view: */
			glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE);
			displayState->eyeIndex=1;
			render(windowViewport,1,viewers[1]->getEyePosition(Viewer::RIGHT));
			break;
		
		case SPLITVIEWPORT_STEREO:
			{
			glDrawBuffer(drawBuffer);
			glReadBuffer(drawBuffer);
			
			if(lensCorrector!=0)
				{
				/* Render the pre-distortion views: */
				for(int eye=0;eye<2;++eye)
					{
					displayState->eyeIndex=eye;
					render(splitViewportPos[eye],eye,viewers[eye]->getEyePosition(eye==0?Viewer::LEFT:Viewer::RIGHT));
					}
				
				if(!frontBufferRendering)
					{
					/* Warp the pre-distortion views into the final drawable: */
					lensCorrector->warp();
					}
				}
			else
				{
				/* Render both views into the split viewport: */
				glEnable(GL_SCISSOR_TEST);
				for(int eye=0;eye<2;++eye)
					{
					glScissor(splitViewportPos[eye].origin[0],splitViewportPos[eye].origin[1],
				          	splitViewportPos[eye].size[0],splitViewportPos[eye].size[1]);
					displayState->eyeIndex=eye;
					render(splitViewportPos[eye],eye,viewers[eye]->getEyePosition(eye==0?Viewer::LEFT:Viewer::RIGHT));
					}
				glDisable(GL_SCISSOR_TEST);
				}
			
			break;
			}
		
		case INTERLEAVEDVIEWPORT_STEREO:
			glDrawBuffer(drawBuffer);
			glReadBuffer(drawBuffer);
			
			if(hasFramebufferObjectExtension)
				{
				/* Render the left-eye view into the window's default framebuffer: */
				displayState->eyeIndex=0;
				render(windowViewport,0,viewers[0]->getEyePosition(Viewer::LEFT));
				
				/* Render the right-eye view into the right viewport framebuffer: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,ivRightFramebufferObjectID);
				displayState->eyeIndex=1;
				render(windowViewport,1,viewers[1]->getEyePosition(Viewer::RIGHT));
				
				/* Re-bind the default framebuffer to get access to the right viewport image as a texture: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
				}
			else
				{
				/* Render the right-eye view into the window's default framebuffer: */
				displayState->eyeIndex=1;
				render(windowViewport,1,viewers[1]->getEyePosition(Viewer::RIGHT));
				
				/* Copy the rendered view into the viewport texture: */
				glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
				glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,GLWindow::getWindowSize()[0],GLWindow::getWindowSize()[1]);
				glBindTexture(GL_TEXTURE_2D,0);
				
				/* Render the left-eye view into the window's default framebuffer: */
				displayState->eyeIndex=0;
				render(windowViewport,0,viewers[0]->getEyePosition(Viewer::LEFT));
				}
			
			/* Set up matrices to render a full-screen quad: */
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glDisable(GL_DEPTH_TEST);
			
			/* Set up polygon stippling: */
			glEnable(GL_POLYGON_STIPPLE);
			
			/* Bind the right viewport texture: */
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			
			/* Set the polygon stippling pattern: */
			glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glPolygonStipple(ivRightStipplePatterns[ivEyeIndexOffset]);
				
			/* Render the quad: */
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);
			glVertex2f(-1.0f,-1.0f);
			
			glTexCoord2f(ivTexCoord[0],0.0f);
			glVertex2f(1.0f,-1.0f);
			
			glTexCoord2f(ivTexCoord[0],ivTexCoord[1]);
			glVertex2f(1.0f,1.0f);
			
			glTexCoord2f(0.0f,ivTexCoord[1]);
			glVertex2f(-1.0f,1.0f);
			glEnd();
			
			/* Reset OpenGL state: */
			glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_POLYGON_STIPPLE);
			glEnable(GL_DEPTH_TEST);
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			break;
		
		case AUTOSTEREOSCOPIC_STEREO:
			{
			/* Set up the view zone mapping: */
			int asTileSize[2];
			float asTileTexCoord[2];
			int asQuadSize[2];
			for(int i=0;i<2;++i)
				{
				asTileSize[i]=GLWindow::getWindowSize()[i]/asNumTiles[i];
				asTileTexCoord[i]=float(asTileSize[i])/float(asTextureSize[i]);
				asQuadSize[i]=asTileSize[i]*asNumTiles[i];
				}
			
			if(hasFramebufferObjectExtension)
				{
				/* Bind the framebuffer for view zone rendering: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,asFrameBufferObjectID);
				}
			
			/* Calculate the central eye position and the view zone offset vector: */
			Point asEye=viewers[0]->getEyePosition(Viewer::MONO);
			Vector asViewZoneOffsetVector=screens[0]->getScreenTransformation().inverseTransform(Vector(asViewZoneOffset,0,0));
			
			/* Render the view zones: */
			glEnable(GL_SCISSOR_TEST);
			for(int zoneIndex=0;zoneIndex<asNumViewZones;++zoneIndex)
				{
				int row=zoneIndex/asNumTiles[0];
				int col=zoneIndex%asNumTiles[0];
				GLWindow::WindowPos asTile(asTileSize[0]*col,asTileSize[1]*row,asTileSize[0],asTileSize[1]);
				glScissor(asTileSize[0]*col,asTileSize[1]*row,asTileSize[0],asTileSize[1]);
				Point eyePos=asEye;
				eyePos+=asViewZoneOffsetVector*(Scalar(zoneIndex)-Math::div2(Scalar(asNumViewZones-1)));
				displayState->eyeIndex=zoneIndex;
				render(asTile,0,eyePos);
				}
			glDisable(GL_SCISSOR_TEST);
			
			/* Read the view zone image into a texture: */
			if(hasFramebufferObjectExtension)
				{
				/* Unbind the frame buffer to get access to the texture image: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
				
				/* Bind the view zone texture to texture unit 0: */
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glBindTexture(GL_TEXTURE_2D,asViewZoneTextureID);
				}
			else
				{
				/* Copy the view zone image from the back buffer into the texture image and bind it to texture 0: */
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glBindTexture(GL_TEXTURE_2D,asViewZoneTextureID);
				glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,GLWindow::getWindowSize()[0],GLWindow::getWindowSize()[1]);
				}
			
			/* Bind the view map image to texture unit 1: */
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D,asViewMapTextureID);
			
			/* Enable the interzigging shader: */
			asInterzigShader->useProgram();
			glUniformARB(asInterzigShader->getUniformLocation("viewZonesTexture"),0);
			glUniformARB(asInterzigShader->getUniformLocation("viewMapTexture"),1);
			glUniformARB<2>(asQuadSizeUniformIndex,1,asTileTexCoord);
			
			/* Set up matrices to render a full-screen quad: */
			glViewport(0,0,asQuadSize[0],asQuadSize[1]);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0,GLdouble(asQuadSize[0]),0.0,GLdouble(asQuadSize[1]),-1.0,1.0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			
			/* Render the quad: */
			glBegin(GL_QUADS);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0f,0.0f);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.0f,0.0f);
			glVertex2i(0,0);
			
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,asTileTexCoord[0],0.0f);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,asTileTexCoord[0]*3.0f,0.0f);
			glVertex2i(asQuadSize[0],0);
			
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,asTileTexCoord[0],asTileTexCoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,asTileTexCoord[0]*3.0f,asTileTexCoord[1]*3.0f);
			glVertex2i(asQuadSize[0],asQuadSize[1]);
			
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0f,asTileTexCoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.0f,asTileTexCoord[1]*3.0f);
			glVertex2i(0,asQuadSize[1]);
			glEnd();
			
			/* Reset OpenGL state: */
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			GLShader::disablePrograms();
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D,0);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glBindTexture(GL_TEXTURE_2D,0);
			break;
			}
		}
	
	/* Flush the OpenGL pipeline to force completion: */
	glFlush();
	
	/* Check for OpenGL errors: */
	glPrintError(std::cerr);
	
	/* Take a screen shot if requested: */
	if(saveScreenshot)
		{
		/* Create an RGB image of the same size as the window: */
		Images::RGBImage image(getWindowWidth(),getWindowHeight());
		
		/* Read the window contents into an RGB image: */
		image.glReadPixels(0,0);
		
		/* Save the image buffer to the given image file: */
		Images::writeImageFile(image,screenshotImageFileName.c_str());
		
		#if SAVE_SCREENSHOT_PROJECTION
		
		/* Temporarily load the navigation-space modelview matrix: */
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glMultMatrix(displayState->modelviewNavigational);
		
		/* Query the current projection and modelview matrices: */
		GLdouble proj[16],mv[16];
		glGetDoublev(GL_PROJECTION_MATRIX,proj);
		glGetDoublev(GL_MODELVIEW_MATRIX,mv);
		
		glPopMatrix();
		
		/* Write the matrices to a projection file: */
		{
		IO::FilePtr projFile(Vrui::openFile((screenshotImageFileName+".proj").c_str(),Misc::BufferedFile::WriteOnly));
		projFile->setEndianness(IO::File::LittleEndian);
		projFile->write(proj,16);
		projFile->write(mv,16);
		}
		
		#endif
		
		saveScreenshot=false;
		}
	
	/* Check if the window is supposed to save a movie: */
	if(movieSaver!=0)
		{
		/* Get a fresh frame buffer: */
		MovieSaver::FrameBuffer& frameBuffer=movieSaver->startNewFrame();
		
		/* Update the frame buffer's size and prepare it for writing: */
		frameBuffer.setFrameSize(getWindowWidth(),getWindowHeight());
		frameBuffer.prepareWrite();
		
		/* Read the window contents into the movie saver's frame buffer: */
		glPixelStorei(GL_PACK_ALIGNMENT,1);
		glPixelStorei(GL_PACK_SKIP_PIXELS,0);
		glPixelStorei(GL_PACK_ROW_LENGTH,0);
		glPixelStorei(GL_PACK_SKIP_ROWS,0);
		glReadPixels(0,0,getWindowWidth(),getWindowHeight(),GL_RGB,GL_UNSIGNED_BYTE,frameBuffer.getBuffer());
		
		/* Post the new frame: */
		movieSaver->postNewFrame();
		}
	
	/* Window is now up-to-date: */
	resizeViewport=false;
	dirty=false;
	
	if(burnMode)
		{
		if(++burnModeNumFrames==0)
			burnModeStartTime=getApplicationTime();
		
		/* Request another Vrui frame immediately: */
		requestUpdate();
		}
	}

void VRWindow::swapBuffers(void)
	{
	if(frontBufferRendering)
		{
		if(lensCorrector!=0)
			{
			/* Wait for the next vertical blanking period: */
			waitForVsync();
			
			/* Warp the pre-distortion views into the final drawable: */
			glXWaitX();
			lensCorrector->warp();
			glFlush();
			}
		}
	else
		{
		GLWindow::swapBuffers();
		if(vsync)
			glFinish();
		}
	}

}
