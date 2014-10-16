/***********************************************************************
VruiSoundTest - Small application to illustrate principles of spatial
audio programming using Vrui's OpenAL interface.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <AL/Config.h>
#include <AL/ALGeometryWrappers.h>
#include <AL/ALObject.h>
#include <AL/ALContextData.h>
#include <Vrui/Application.h>
#include <Vrui/Vrui.h>

class VruiSoundTest:public Vrui::Application,public GLObject,public ALObject
	{
	/* Embedded classes: */
	private:
	struct GLDataItem:public GLObject::DataItem // Structure to hold application state associated with a particular OpenGL context
		{
		/* Elements: */
		public:
		GLuint displayListId; // Display list to render a sphere
		
		/* Constructors and destructors: */
		public:
		GLDataItem(void)
			:displayListId(glGenLists(1))
			{
			}
		virtual ~GLDataItem(void)
			{
			glDeleteLists(displayListId,1);
			}
		};
	
	struct ALDataItem:public ALObject::DataItem // Structure to hold application state associated with a particular OpenAL context
		{
		/* Elements: */
		public:
		#if ALSUPPORT_CONFIG_HAVE_OPENAL
		ALuint sources[3]; // Three sound sources
		ALuint buffers[3]; // Sample buffers for the three sound sources
		#endif
		
		/* Constructors and destructors: */
		public:
		ALDataItem(void)
			{
			#if ALSUPPORT_CONFIG_HAVE_OPENAL
			/* Generate buffers and sources: */
			alGenSources(3,sources);
			alGenBuffers(3,buffers);
			#endif
			}
		virtual ~ALDataItem(void)
			{
			#if ALSUPPORT_CONFIG_HAVE_OPENAL
			/* Destroy buffers and sources: */
			alDeleteSources(3,sources);
			alDeleteBuffers(3,buffers);
			#endif
			}
		};
	
	/* Elements: */
	Vrui::Point positions[3]; // Positions of the three spheres
	
	/* Constructors and destructors: */
	public:
	VruiSoundTest(int& argc,char**& argv);
	virtual ~VruiSoundTest(void);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* Methods from ALObject: */
	virtual void initContext(ALContextData& contextData) const;
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void sound(ALContextData& contextData) const;
	};

/******************************
Methods of class VruiSoundTest:
******************************/

VruiSoundTest::VruiSoundTest(int& argc,char**& argv)
	:Vrui::Application(argc,argv)
	{
	/* Request sound processing: */
	Vrui::requestSound();
	
	/* Initialize the sphere positions: */
	positions[0]=Vrui::Point(0,0,0);
	positions[1]=Vrui::Point(5,0,0);
	positions[2]=Vrui::Point(0,10,0);
	
	/* Initialize the navigation transformation: */
	Vrui::setNavigationTransformation(Vrui::Point::origin,Vrui::Scalar(10),Vrui::Vector(0,0,1));
	}

VruiSoundTest::~VruiSoundTest(void)
	{
	}

void VruiSoundTest::initContext(GLContextData& contextData) const
	{
	/* Create a new context data item: */
	GLDataItem* dataItem=new GLDataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create a display list to render a sphere: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	
	/* Draw the sphere: */
	glDrawSphereIcosahedron(1.0f,10);
	
	glEndList();
	}

void VruiSoundTest::initContext(ALContextData& contextData) const
	{
	/* Create a new context data item: */
	ALDataItem* dataItem=new ALDataItem;
	contextData.addDataItem(this,dataItem);
	
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Upload sound data into the sound buffers: */
	const int pcmFreq=44100;
	ALubyte* pcmData=new ALubyte[pcmFreq];
	
	/* Create a 400 Hz sine wave: */
	for(int i=0;i<pcmFreq;++i)
		{
		double angle=400.0*2.0*Math::Constants<double>::pi*double(i)/double(pcmFreq);
		pcmData[i]=ALubyte(Math::sin(angle)*127.5+128.0);
		}
	alBufferData(dataItem->buffers[0],AL_FORMAT_MONO8,pcmData,pcmFreq,pcmFreq);
	
	/* Create an 800 Hz sawtooth wave: */
	for(int i=0;i<pcmFreq;++i)
		{
		double step=800.0*double(i)/double(pcmFreq);
		pcmData[i]=ALubyte((step-Math::floor(step))*255.0+0.5);
		}
	alBufferData(dataItem->buffers[1],AL_FORMAT_MONO8,pcmData,pcmFreq,pcmFreq);
	
	/* Create a 600 Hz rectangle wave: */
	for(int i=0;i<pcmFreq;++i)
		{
		double step=600.0*double(i)/double(pcmFreq);
		pcmData[i]=ALubyte((step-Math::floor(step))>=0.5?255:0);
		}
	alBufferData(dataItem->buffers[2],AL_FORMAT_MONO8,pcmData,pcmFreq,pcmFreq);
	
	delete[] pcmData;
	
	/* Create three source objects: */
	for(int i=0;i<3;++i)
		{
		alSourceBuffer(dataItem->sources[i],dataItem->buffers[i]);
		alSourceLooping(dataItem->sources[i],AL_TRUE);
		alSourcePitch(dataItem->sources[i],1.0f);
		alSourceGain(dataItem->sources[i],1.0f);
		alSourcePlay(dataItem->sources[i]);
		}
	#endif
	}

void VruiSoundTest::frame(void)
	{
	}

void VruiSoundTest::display(GLContextData& contextData) const
	{
	/* Get the data item: */
	GLDataItem* dataItem=contextData.retrieveDataItem<GLDataItem>(this);
	
	/* Set up OpenGL state: */
	glPushAttrib(GL_LIGHTING_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	
	/* Draw several spheres at "random" positions: */
	glPushMatrix();
	glTranslate(positions[0]-Vrui::Point::origin);
	glColor3f(0.0f,1.0f,0.0f);
	glCallList(dataItem->displayListId);
	glPopMatrix();
	
	glPushMatrix();
	glTranslate(positions[1]-Vrui::Point::origin);
	glColor3f(1.0f,0.0f,0.0f);
	glCallList(dataItem->displayListId);
	glPopMatrix();
	
	glPushMatrix();
	glTranslate(positions[2]-Vrui::Point::origin);
	glColor3f(0.0f,0.0f,1.0f);
	glCallList(dataItem->displayListId);
	glPopMatrix();
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

void VruiSoundTest::sound(ALContextData& contextData) const
	{
	typedef ALContextData::Point ALPoint;
	typedef ALContextData::Transform ALTransform;
	
	/* Get the data item: */
	ALDataItem* dataItem=contextData.retrieveDataItem<ALDataItem>(this);
	
	/* Get the current modelview matrix from the OpenAL context: */
	const ALTransform& transform=contextData.getMatrix();
	
	/* Update the positions and gains of the sources: */
	for(int i=0;i<3;++i)
		{
		#if ALSUPPORT_CONFIG_HAVE_OPENAL
		/* Set the source position transformed to physical coordinates: */
		alSourcePosition(dataItem->sources[i],ALPoint(positions[i]),transform);
		
		/* Set the source attenuation factors in physical coordinates: */
		alSourceReferenceDistance(dataItem->sources[i],Vrui::getDisplaySize());
		alSourceRolloffFactor(dataItem->sources[i],1.0);
		#endif
		}
	}

VRUI_APPLICATION_RUN(VruiSoundTest)
