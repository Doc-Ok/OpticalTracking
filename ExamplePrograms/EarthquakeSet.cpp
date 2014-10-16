/***********************************************************************
EarthquakeSet - Class to represent and render sets of earthquakes with
3D locations, magnitude and event time.
Copyright (c) 2006-2013 Oliver Kreylos

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

#include "EarthquakeSet.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <Misc/ThrowStdErr.h>
#include <Misc/FileNameExtensions.h>
#include <IO/ValueSource.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Vector.h>
#include <Geometry/HVector.h>
#include <Geometry/ValuedPoint.h>
#include <Geometry/ArrayKdTree.h>
#include <Geometry/Geoid.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayTemplates.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBPointParameters.h>
#include <GL/Extensions/GLARBPointSprite.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLShader.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLGeometryVertex.h>

namespace {

/***********************************************************************
Helper classes and functions to parse spreadsheet files in text format:
***********************************************************************/

inline bool strequal(const std::string& s1,const char* s2)
	{
	std::string::const_iterator s1It=s1.begin();
	const char* s2Ptr=s2;
	while(s1It!=s1.end()&&*s2Ptr!='\0'&&tolower(*s1It)==tolower(*s2Ptr))
		{
		++s1It;
		++s2Ptr;
		}
	return s1It==s1.end()&&*s2Ptr=='\0';
	}

inline double parseDateTime(const char* date,const char* time)
	{
	struct tm dateTime;
	
	/* Parse the date: */
	int d[3];
	char* nextPtr=const_cast<char*>(date); // Ugly, but required by API
	for(int i=0;i<3;++i)
		{
		d[i]=strtol(nextPtr,&nextPtr,10);
		if((i<2&&*nextPtr!='/')||(i==2&&*nextPtr!='\0'))
			Misc::throwStdErr("Format error in date string %s",date);
		++nextPtr;
		}
	if(d[0]>=1&&d[0]<=12&&d[1]>=1&&d[1]<=31)
		{
		/* Format is month, day, year: */
		dateTime.tm_mday=d[1];
		dateTime.tm_mon=d[0]-1;
		dateTime.tm_year=d[2]-1900;
		}
	else if(d[1]>=1&&d[1]<=12&&d[2]>=1&&d[2]<=31)
		{
		/* Format is year, month, day: */
		dateTime.tm_mday=d[2];
		dateTime.tm_mon=d[1]-1;
		dateTime.tm_year=d[0]-1900;
		}
	else
		Misc::throwStdErr("Format error in date string %s",date);
	
	/* Parse the time: */
	int t[3];
	nextPtr=const_cast<char*>(time); // Ugly, but required by API
	for(int i=0;i<3;++i)
		{
		t[i]=strtol(nextPtr,&nextPtr,10);
		if((i<2&&*nextPtr!=':')||(i==2&&*nextPtr!='.'&&*nextPtr!='\0'))
			Misc::throwStdErr("Format error in time string %s",time);
		++nextPtr;
		}
	
	/* Format is hour, minute, second: */
	if(t[0]<0||t[0]>23||t[1]<0||t[1]>59||t[2]<0||t[2]>60)
		Misc::throwStdErr("Format error in time string %s",time);
	dateTime.tm_sec=t[2];
	dateTime.tm_min=t[1];
	dateTime.tm_hour=t[0];
	
	/* Convert the date/time structure to seconds since the epoch: */
	dateTime.tm_isdst=-1;
	return double(mktime(&dateTime));
	}

}

/****************************************
Methods of class EarthquakeSet::DataItem:
****************************************/

EarthquakeSet::DataItem::DataItem(void)
	:vertexBufferObjectId(0),
	 pointRenderer(0),clipPlaneVersion(0),fog(false),
	 scaledPointRadiusLocation(-1),highlightTimeLocation(-1),
	 currentTimeLocation(-1),pointTextureLocation(-1),
	 pointTextureObjectId(0),
	 eyePos(Point::origin),
	 sortedPointIndicesBufferObjectId(0)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		
		/* Check if GLSL shaders are supported: */
		if(GLARBMultitexture::isSupported()&&GLARBPointParameters::isSupported()&&GLARBPointSprite::isSupported()&&GLShader::isSupported())
			{
			/* Initialize the basic OpenGL extensions: */
			GLARBMultitexture::initExtension();
			GLARBPointParameters::initExtension();
			GLARBPointSprite::initExtension();

			/* Create the shader object: */
			pointRenderer=new GLShader;

			/* Create the point texture object: */
			glGenTextures(1,&pointTextureObjectId);

			/* Create the sorted point index buffer: */
			glGenBuffersARB(1,&sortedPointIndicesBufferObjectId);
			}
		}
	}

EarthquakeSet::DataItem::~DataItem(void)
	{
	/* Check if the vertex buffer object extension is supported: */
	if(vertexBufferObjectId>0)
		{
		/* Destroy the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferObjectId);
		
		/* Check if GLSL shaders are supported: */
		if(pointRenderer!=0)
			{
			/* Destroy the shader object: */
			delete pointRenderer;
			
			/* Delete the point texture object: */
			glDeleteTextures(1,&pointTextureObjectId);
			
			/* Delete the sorted point index buffer: */
			glDeleteBuffersARB(1,&sortedPointIndicesBufferObjectId);
			}
		}
	}

/******************************
Methods of class EarthquakeSet:
******************************/

void EarthquakeSet::loadANSSFile(IO::FilePtr earthquakeFile,const Geometry::Geoid<double>& referenceEllipsoid,const Geometry::Vector<double,3>& offset,double scaleFactor)
	{
	/* Wrap a value source around the input file: */
	IO::ValueSource source(earthquakeFile);
	source.setPunctuation("\n");
	source.skipWs();
	
	/* Skip the two header lines: */
	for(int i=0;i<2;++i)
		{
		source.skipLine();
		source.skipWs();
		}
	
	/* Read the rest of the file: */
	while(!source.eof())
		{
		/* Read the next line: */
		std::string line=source.readLine();
		source.skipWs();
		
		/* Skip empty lines: */
		if(line.empty()||line[0]=='\r')
			continue;
		
		/* Parse an event from the line: */
		Event e;
		
		/* Read date and time: */
		std::string date(line,0,10);
		std::string time(line,11,11);
		e.time=parseDateTime(date.c_str(),time.c_str());
		
		/* Read event position: */
		Geometry::Geoid<double>::Point geodeticPosition;
		
		/* Read latitude: */
		std::string lat(line,23,8);
		geodeticPosition[1]=Math::rad(atof(lat.c_str()));
		
		/* Read longitude: */
		std::string lon(line,32,9);
		geodeticPosition[0]=Math::rad(atof(lon.c_str()));
		
		/* Read depth and convert to ellipsoid height in meters: */
		std::string dep(line,42,6);
		geodeticPosition[2]=atof(dep.c_str())*-1000.0;
		
		/* Convert the spherical position to offset and scaled Cartesian: */
		Geometry::Geoid<double>::Point cartesianPosition=referenceEllipsoid.geodeticToCartesian(geodeticPosition);
		for(int i=0;i<3;++i)
			e.position[i]=float((cartesianPosition[i]+offset[i])*scaleFactor);
		
		/* Read magnitude: */
		std::string mag(line,49,5);
		e.magnitude=float(atof(mag.c_str()));
		
		/* Save the event: */
		events.push_back(e);
		}
	}

void EarthquakeSet::loadCSVFile(IO::FilePtr earthquakeFile,const Geometry::Geoid<double>& referenceEllipsoid,const Geometry::Vector<double,3>& offset,double scaleFactor)
	{
	/* Wrap a value source around the input file: */
	IO::ValueSource source(earthquakeFile);
	source.setPunctuation(",\n");
	source.setQuotes("\"");
	source.skipWs();
	
	/*********************************************************************
	Parse the point file's header line:
	*********************************************************************/
	
	/* Remember the column indices of important columns: */
	int latIndex=-1;
	int lngIndex=-1;
	int radiusIndex=-1;
	enum RadiusMode // Enumerated types for radius coordinate modes
		{
		RADIUS,DEPTH,NEGDEPTH
		} radiusMode=RADIUS;
	int dateIndex=-1;
	int timeIndex=-1;
	int magIndex=-1;
	
	/* Read the header line's columns: */
	int column=0;
	while(true)
		{
		/* Read the next column header: */
		std::string header=!source.eof()&&source.peekc()!='\n'&&source.peekc()!=','?source.readString():"";
		
		/* Parse the column header: */
		if(strequal(header,"Latitude")||strequal(header,"Lat"))
			latIndex=column;
		else if(strequal(header,"Longitude")||strequal(header,"Long")||strequal(header,"Lon"))
			lngIndex=column;
		else if(strequal(header,"Radius"))
			{
			radiusIndex=column;
			radiusMode=RADIUS;
			}
		else if(strequal(header,"Depth"))
			{
			radiusIndex=column;
			radiusMode=DEPTH;
			}
		else if(strequal(header,"Negative Depth")||strequal(header,"Neg Depth")||strequal(header,"NegDepth"))
			{
			radiusIndex=column;
			radiusMode=NEGDEPTH;
			}
		else if(strequal(header,"Date"))
			dateIndex=column;
		else if(strequal(header,"Time"))
			timeIndex=column;
		else if(strequal(header,"Magnitude")||strequal(header,"Mag"))
			magIndex=column;
		
		++column;
		
		/* Check for end of line: */
		if(source.eof()||source.peekc()=='\n')
			break;
		
		/* Skip an optional comma: */
		if(source.peekc()==',')
			source.skipString();
		}
	
	/* Determine the number of fields: */
	int maxIndex=latIndex;
	if(maxIndex<lngIndex)
		maxIndex=lngIndex;
	if(maxIndex<radiusIndex)
		maxIndex=radiusIndex;
	if(maxIndex<dateIndex)
		maxIndex=dateIndex;
	if(maxIndex<timeIndex)
		maxIndex=timeIndex;
	if(maxIndex<magIndex)
		maxIndex=magIndex;
	
	/* Skip the newline: */
	source.skipLine();
	source.skipWs();
	
	/* Check if all required portions have been detected: */
	if(latIndex<0)
		throw std::runtime_error("Missing latitude field");
	if(lngIndex<0)
		throw std::runtime_error("Missing longitude field");
	if(radiusIndex<0)
		throw std::runtime_error("Missing radius/depth/negative depth field");
	if(dateIndex<0)
		throw std::runtime_error("Missing date field");
	if(timeIndex<0)
		throw std::runtime_error("Missing time field");
	if(magIndex<0)
		throw std::runtime_error("Missing magnitude field");
	
	/* Read lines from the file: */
	int lineNumber=2;
	while(!source.eof())
		{
		Geometry::Geoid<double>::Point geodeticPosition=Geometry::Geoid<double>::Point::origin; // Only initializing to shut up compiler
		std::string date,time;
		float magnitude=0.0f;
		int column=0;
		try
			{
			while(true)
				{
				/* Read the next field: */
				if(!source.eof()&&source.peekc()!='\n'&&source.peekc()!=',')
					{
					if(column==latIndex)
						geodeticPosition[1]=Math::rad(source.readNumber());
					else if(column==lngIndex)
						geodeticPosition[0]=Math::rad(source.readNumber());
					else if(column==radiusIndex)
						geodeticPosition[2]=source.readNumber()*1000.0;
					else if(column==dateIndex)
						date=source.readString();
					else if(column==timeIndex)
						time=source.readString();
					else if(column==magIndex)
						magnitude=float(source.readNumber());
					else
						source.skipString();
					}
				
				++column;
				
				/* Check for end of line: */
				if(source.eof()||source.peekc()=='\n')
					break;
				
				/* Skip an optional comma: */
				if(source.peekc()==',')
					source.skipString();
				}
			}
		catch(IO::ValueSource::NumberError err)
			{
			/* Ignore the error and the malformed event */
			}
		
		/* Skip the newline: */
		source.skipLine();
		source.skipWs();
		
		/* Check if all fields were read: */
		if(column>maxIndex)
			{
			/* Create an event: */
			Event e;
			
			/* Convert the read spherical coordinates to Cartesian coordinates: */
			Geometry::Geoid<double>::Point cartesianPosition;
			if(radiusMode==RADIUS)
				{
				/* Use a simple formula with a squished sphere: */
				double xy=Math::cos(geodeticPosition[1])*geodeticPosition[2];
				cartesianPosition[0]=xy*Math::cos(geodeticPosition[0]);
				cartesianPosition[1]=xy*Math::sin(geodeticPosition[0]);
				cartesianPosition[2]=Math::sin(geodeticPosition[1])*geodeticPosition[2]*(1.0-referenceEllipsoid.getFlatteningFactor());
				}
			else
				{
				if(radiusMode==DEPTH)
					geodeticPosition[2]=-geodeticPosition[2];
				cartesianPosition=referenceEllipsoid.geodeticToCartesian(geodeticPosition);
				}
			for(int i=0;i<3;++i)
				e.position[i]=float((cartesianPosition[i]+offset[i])*scaleFactor);
			
			/* Calculate the event time: */
			e.time=parseDateTime(date.c_str(),time.c_str());
			
			/* Store the event magnitude: */
			e.magnitude=magnitude;
			
			/* Append the event to the earthquake set: */
			events.push_back(e);
			}
		
		++lineNumber;
		}
	}

#if EARTHQUAKESET_EXPLICIT_RECURSION

void EarthquakeSet::drawBackToFront(const Point& eyePos,GLuint* bufferPtr) const
	{
	/* Set up a traversal stack for explicit recursion: */
	struct TraversalStack
		{
		/* Elements: */
		public:
		int left,right; // Left and right boundaries of the current subtree
		int root; // Root of the current subtree
		int splitDimension; // Split dimension of the current subtree
		bool goLeftFirst; // Flag whether to traverse the left or right subtree in stage 0
		} traversalStack[33]; // 33 is enough for all time! ALL TIME!
	
	/* Initialize the traversal stack: */
	TraversalStack* tsPtr=traversalStack;
	tsPtr->left=0;
	tsPtr->right=int(events.size())-1;
	tsPtr->splitDimension=0;
	
	/*********************************************************************
	Traverse the tree recursively by ping-ponging between stages 0 and 1:
	*********************************************************************/
	
	doTheStage0:
	
	/*****************************************************************
	Stage 0: Traverse into the subtree farther from the eye position.
	*****************************************************************/
	
	/* Calculate the root node index: */
	tsPtr->root=(tsPtr->left+tsPtr->right)>>1;
	
	/* Decide whether to go left or right first: */
	tsPtr->goLeftFirst=eyePos[tsPtr->splitDimension]>events[tsPtr->root].position[tsPtr->splitDimension];
	
	if(tsPtr->goLeftFirst)
		{
		if(tsPtr->left<tsPtr->root)
			{
			/* Traverse left subtree: */
			++tsPtr;
			tsPtr->left=tsPtr[-1].left;
			tsPtr->right=tsPtr[-1].root-1;
			if((tsPtr->splitDimension=tsPtr[-1].splitDimension+1)==3)
				tsPtr->splitDimension=0;
			
			goto doTheStage0;
			}
		}
	else
		{
		if(tsPtr->root<tsPtr->right)
			{
			/* Traverse right subtree: */
			++tsPtr;
			tsPtr->left=tsPtr[-1].root+1;
			tsPtr->right=tsPtr[-1].right;
			if((tsPtr->splitDimension=tsPtr[-1].splitDimension+1)==3)
				tsPtr->splitDimension=0;
			
			goto doTheStage0;
			}
		}
	
	doTheStage1:
	
	/*****************************************************************
	Stage 1: Enter the root node's index into the index buffer.
	*****************************************************************/
	
	*bufferPtr=tsPtr->root;
	++bufferPtr;
	
	/*****************************************************************
	Stage 2: Traverse into the subtree closer to the eye position.
	*****************************************************************/
	
	if(tsPtr->goLeftFirst)
		{
		if(tsPtr->root<tsPtr->right)
			{
			/* Traverse right subtree, but stay in the same stack frame to avoid tail recursion: */
			tsPtr->left=tsPtr->root+1;
			if(++tsPtr->splitDimension==3)
				tsPtr->splitDimension=0;
			
			goto doTheStage0;
			}
		}
	else
		{
		if(tsPtr->left<tsPtr->root)
			{
			/* Traverse left subtree, but stay in the same stack frame to avoid tail recursion: */
			tsPtr->right=tsPtr->root-1;
			if(++tsPtr->splitDimension==3)
				tsPtr->splitDimension=0;
			
			goto doTheStage0;
			}
		}
	
	/* Return to caller: */
	--tsPtr;
	if(tsPtr>=traversalStack)
		goto doTheStage1;
	}

#else

void EarthquakeSet::drawBackToFront(int left,int right,int splitDimension,const Point& eyePos,GLuint*& bufferPtr) const
	{
	/* Get the current node index: */
	int mid=(left+right)>>1;
	
	int childSplitDimension=splitDimension+1;
	if(childSplitDimension==3)
		childSplitDimension=0;
	
	/* Traverse into the subtree on the far side of the split plane first: */
	if(eyePos[splitDimension]>events[treePointIndices[mid]].position[splitDimension])
		{
		/* Traverse left child: */
		if(left<mid)
			drawBackToFront(left,mid-1,childSplitDimension,eyePos,bufferPtr);
		
		/* Draw the point: */
		*bufferPtr=GLuint(mid);
		++bufferPtr;
		
		/* Traverse right child: */
		if(right>mid)
			drawBackToFront(mid+1,right,childSplitDimension,eyePos,bufferPtr);
		}
	else
		{
		/* Traverse right child: */
		if(right>mid)
			drawBackToFront(mid+1,right,childSplitDimension,eyePos,bufferPtr);
		
		/* Draw the point: */
		*bufferPtr=GLuint(mid);
		++bufferPtr;
		
		/* Traverse left child: */
		if(left<mid)
			drawBackToFront(left,mid-1,childSplitDimension,eyePos,bufferPtr);
		}
	}

#endif

void EarthquakeSet::createShader(EarthquakeSet::DataItem* dataItem,const GLClipPlaneTracker& cpt) const
	{
	/* Start creating the point rendering vertex shader: */
	std::string vertexProgram="\
		uniform float scaledPointRadius;\n\
		uniform float highlightTime;\n\
		uniform float currentTime;\n";
	
	if(dataItem->layeredRendering)
		{
		vertexProgram+="\
			uniform vec4 frontSphereCenter;\n\
			uniform float frontSphereRadius2;\n\
			uniform bool frontSphereTest;\n\
			\n\
			void main()\n\
				{\n\
				/* Check if the point is inside the front sphere: */\n\
				bool valid=dot(gl_Vertex-frontSphereCenter,gl_Vertex-frontSphereCenter)>=frontSphereRadius2;\n\
				if(frontSphereTest)\n\
					valid=!valid;\n\
				if(valid)\n\
					{\n";
		}
	else
		{
		vertexProgram+="\
			\n\
			void main()\n\
				{\n";
		}
	
	vertexProgram+="\
				/* Transform the vertex to eye coordinates: */\n\
				vec4 vertexEye=gl_ModelViewMatrix*gl_Vertex;\n\
				\n\
				/* Calculate point size based on vertex' eye distance along z direction and event magnitude: */\n\
				float pointSize=scaledPointRadius*2.0*vertexEye.w/vertexEye.z;\n\
				if(gl_MultiTexCoord0.x>";
	char number[20];
	snprintf(number,sizeof(number),"%f",colorMap.getScalarRangeMin());
	vertexProgram+=number;
	vertexProgram+=")\n\
					pointSize*=gl_MultiTexCoord0.x-(";
	snprintf(number,sizeof(number),"%f",colorMap.getScalarRangeMin()-1.0);
	vertexProgram+=number;
	vertexProgram+=");\n\
				\n\
				/* Adapt point size based on current time and time scale: */\n\
				float highlightFactor=gl_MultiTexCoord0.y-(currentTime-highlightTime);\n\
				if(highlightFactor>0.0&&highlightFactor<=highlightTime)\n\
					pointSize*=2.0*highlightFactor/highlightTime+1.0;\n\
				\n\
				/* Set point size: */\n\
				gl_PointSize=pointSize;\n\
				\n";
	
	/* Check if fog is enabled: */
	if(dataItem->fog)
		{
		/* Add fog attenuation to the vertex shader: */
		vertexProgram+="\
				/* Calculate vertex-eye distance for fog computation: */\n\
				float eyeDist=-vertexEye.z/vertexEye.w;\n\
				\n\
				/* Calculate fog attenuation: */\n\
				float fogFactor=clamp((eyeDist-gl_Fog.start)/(gl_Fog.end-gl_Fog.start),0.0,1.0);\n\
				\n\
				/* Use standard color attenuated by fog: */\n\
				gl_FrontColor=mix(gl_Color,gl_Fog.color,fogFactor);\n\
				\n";
		}
	else
		{
		/* Use unattenuated point colors: */
		vertexProgram+="\
				/* Use standard color: */\n\
				gl_FrontColor=gl_Color;\n\
				\n";
		}
	
	/* Check if any clipping planes are enabled: */
	if(cpt.getNumEnabledClipPlanes()!=0)
		{
		/* Insert code to calculate the point's distance with respect to all enabled clipping planes: */
		vertexProgram+=cpt.createCalcClipDistances("vertexEye");
		}
	
	/* Continue creating the point rendering vertex shader: */
	if(dataItem->layeredRendering)
		{
		vertexProgram+="\
				}\n\
			else\n\
				{\n\
				/* Set point size to zero and color to invisible: */\n\
				gl_PointSize=0.0;\n\
				gl_FrontColor=vec4(0.0,0.0,0.0,0.0);\n\
				}";
		}
	vertexProgram+="\
			\n\
			/* Use standard vertex position for fragment generation: */\n\
			gl_Position=ftransform();\n\
			}\n";
	
	/* Create the point rendering fragment shader: */
	static const char* fragmentProgram="\
		uniform sampler2D pointTexture;\n\
		\n\
		void main()\n\
			{\n\
			/* Modulate the sprite texture map by the point color: */\n\
			gl_FragColor=texture2D(pointTexture,gl_TexCoord[0].xy)*gl_Color;\n\
			}\n";
	
	/* Reset the shader: */
	dataItem->pointRenderer->reset();
	
	/* Compile the vertex and fragment programs: */
	dataItem->pointRenderer->compileVertexShaderFromString(vertexProgram.c_str());
	dataItem->pointRenderer->compileFragmentShaderFromString(fragmentProgram);
	
	/* Link the shader: */
	dataItem->pointRenderer->linkShader();
	
	/* Get the locations of all uniform variables: */
	dataItem->scaledPointRadiusLocation=dataItem->pointRenderer->getUniformLocation("scaledPointRadius");
	dataItem->highlightTimeLocation=dataItem->pointRenderer->getUniformLocation("highlightTime");
	dataItem->scaledPointRadiusLocation=dataItem->pointRenderer->getUniformLocation("scaledPointRadius");
	dataItem->currentTimeLocation=dataItem->pointRenderer->getUniformLocation("currentTime");
	if(dataItem->layeredRendering)
		{
		dataItem->frontSphereCenterLocation=dataItem->pointRenderer->getUniformLocation("frontSphereCenter");
		dataItem->frontSphereRadius2Location=dataItem->pointRenderer->getUniformLocation("frontSphereRadius2");
		dataItem->frontSphereTestLocation=dataItem->pointRenderer->getUniformLocation("frontSphereTest");
		}
	else
		{
		dataItem->frontSphereCenterLocation=-1;
		dataItem->frontSphereRadius2Location=-1;
		dataItem->frontSphereTestLocation=-1;
		}
	dataItem->pointTextureLocation=dataItem->pointRenderer->getUniformLocation("pointTexture");
	}

EarthquakeSet::EarthquakeSet(IO::DirectoryPtr directory,const char* earthquakeFileName,const Geometry::Geoid<double>& referenceEllipsoid,const Geometry::Vector<double,3>& offset,double scaleFactor,const GLColorMap& sColorMap)
	:GLObject(false),
	 colorMap(sColorMap),
	 treePointIndices(0),
	 layeredRendering(false),
	 pointRadius(1.0f),highlightTime(1.0),currentTime(0.0)
	{
	/* Open the earthquake file: */
	IO::FilePtr earthquakeFile=directory->openFile(earthquakeFileName);
	
	try
		{
		/* Check the earthquake file name's extension: */
		if(Misc::hasCaseExtension(earthquakeFileName,".anss"))
			{
			/* Read an earthquake database snapshot in "readable" ANSS format: */
			loadANSSFile(earthquakeFile,referenceEllipsoid,offset,scaleFactor);
			}
		else
			{
			/* Read an earthquake event file in space- or comma-separated format: */
			loadCSVFile(earthquakeFile,referenceEllipsoid,offset,scaleFactor);
			}
		}
	catch(std::runtime_error err)
		{
		/* Wrap and re-throw the exception: */
		Misc::throwStdErr("EarthquakeSet::EarthquakeSet: Error \"%s\" while reading file %s",err.what(),earthquakeFileName);
		}
	
	/* Create a temporary kd-tree to sort the events for back-to-front traversal: */
	Geometry::ArrayKdTree<Geometry::ValuedPoint<Point,int> > sortTree(events.size());
	Geometry::ValuedPoint<Point,int>* stPtr=sortTree.accessPoints();
	int i=0;
	for(std::vector<Event>::const_iterator eIt=events.begin();eIt!=events.end();++eIt,++stPtr,++i)
		{
		*stPtr=eIt->position;
		stPtr->value=i;
		}
	sortTree.releasePoints(8);
	
	/* Retrieve the sorted event indices: */
	treePointIndices=new int[events.size()];
	stPtr=sortTree.accessPoints();
	for(int i=0;i<sortTree.getNumNodes();++i,++stPtr)
		treePointIndices[i]=stPtr->value;
	
	GLObject::init();
	}

EarthquakeSet::~EarthquakeSet(void)
	{
	delete[] treePointIndices;
	}

void EarthquakeSet::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	if(dataItem->vertexBufferObjectId>0)
		{
		typedef GLGeometry::Vertex<float,2,GLubyte,4,void,float,3> Vertex;
		
		/* Create a vertex buffer object to store the events: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,events.size()*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
		Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		int numPoints=int(events.size());
		for(int i=0;i<numPoints;++i,++vPtr)
			{
			/* Get a reference to the event in kd-tree order: */
			const Event& e=events[treePointIndices[i]];
			
			/* Copy the event's magnitude and time: */
			vPtr->texCoord[0]=Vertex::TexCoord::Scalar(e.magnitude);
			vPtr->texCoord[1]=Vertex::TexCoord::Scalar(e.time);
			
			/* Map the event's magnitude to color: */
			vPtr->color=Vertex::Color(colorMap(e.magnitude));
			
			/* Copy the event's position: */
			vPtr->position=e.position;
			}
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		
		/* Protect the vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	
	if(dataItem->pointRenderer!=0)
		{
		/* Create the point rendering shader: */
		dataItem->fog=glIsEnabled(GL_FOG);
		dataItem->layeredRendering=layeredRendering;
		createShader(dataItem,*contextData.getClipPlaneTracker());
		
		/* Create the point rendering texture: */
		GLfloat texImage[32][32][4];
		for(int y=0;y<32;++y)
			for(int x=0;x<32;++x)
				{
				texImage[y][x][0]=texImage[y][x][1]=texImage[y][x][2]=1.0f;
				float r2=Math::sqr((float(x)-15.5f)/15.5f)+Math::sqr((float(y)-15.5f)/15.5f);
				if(r2<1.0f)
					{
					// float l=Math::exp(-r2*2.0f)-Math::exp(-2.0f);
					float l=1.0f-r2;
					texImage[y][x][3]=l;
					}
				else
					texImage[y][x][3]=0.0f;
				}
		
		/* Upload particle texture: */
		glBindTexture(GL_TEXTURE_2D,dataItem->pointTextureObjectId);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_FLOAT,&texImage[0][0][0]);
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Create an index buffer to render points in depth order: */
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->sortedPointIndicesBufferObjectId);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,events.size()*sizeof(GLuint),0,GL_STREAM_DRAW_ARB);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		}
	}

EarthquakeSet::TimeRange EarthquakeSet::getTimeRange(void) const
	{
	TimeRange result=TimeRange::empty;
	for(std::vector<Event>::const_iterator eIt=events.begin();eIt!=events.end();++eIt)
		result.addValue(eIt->time);
	
	return result;
	}

void EarthquakeSet::enableLayeredRendering(const EarthquakeSet::Point& newEarthCenter)
	{
	layeredRendering=true;
	earthCenter=newEarthCenter;
	}

void EarthquakeSet::disableLayeredRendering(void)
	{
	layeredRendering=false;
	}

void EarthquakeSet::setPointRadius(float newPointRadius)
	{
	pointRadius=newPointRadius;
	}

void EarthquakeSet::setHighlightTime(double newHighlightTime)
	{
	highlightTime=newHighlightTime;
	}

void EarthquakeSet::setCurrentTime(double newCurrentTime)
	{
	currentTime=newCurrentTime;
	}

void EarthquakeSet::glRenderAction(GLContextData& contextData) const
	{
	/* Get a pointer to the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Save OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT);
	
	if(dataItem->pointRenderer!=0)
		{
		/* Enable point sprites: */
		glEnable(GL_POINT_SPRITE_ARB);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
		
		/* Bind the point rendering texture: */
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D,dataItem->pointTextureObjectId);
		glTexEnvi(GL_POINT_SPRITE_ARB,GL_COORD_REPLACE_ARB,GL_TRUE);
		
		/* Check if the point renderer program conforms to current OpenGL state: */
		if(contextData.getClipPlaneTracker()->getVersion()!=dataItem->clipPlaneVersion||glIsEnabled(GL_FOG)!=dataItem->fog)
			{
			/* Rebuild the point rendering shader: */
			createShader(dataItem,*contextData.getClipPlaneTracker());
			}
		
		/* Enable the point renderer program: */
		dataItem->pointRenderer->useProgram();
		
		/* Set the uniform variables: */
		glUniform1fARB(dataItem->scaledPointRadiusLocation,pointRadius);
		glUniform1fARB(dataItem->highlightTimeLocation,float(highlightTime));
		glUniform1fARB(dataItem->currentTimeLocation,float(currentTime));
		glUniform1iARB(dataItem->pointTextureLocation,0);
		}
	else
		{
		/* Set up standard point rendering: */
		glDisable(GL_LIGHTING);
		}
	
	if(dataItem->vertexBufferObjectId>0)
		{
		typedef GLGeometry::Vertex<float,2,GLubyte,4,void,float,3> Vertex;
		
		/* Bind the point set's vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		GLVertexArrayParts::enable(Vertex::getPartsMask());
		glVertexPointer(static_cast<Vertex*>(0));
		
		/* Render the vertex array: */
		glDrawArrays(GL_POINTS,0,events.size());
		
		/* Protect the vertex buffer object: */
		GLVertexArrayParts::disable(Vertex::getPartsMask());
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/* Render the earthquake set as a regular vertex array of points: */
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT,sizeof(Event),&events[0].position);
		glDrawArrays(GL_POINTS,0,events.size());
		glDisableClientState(GL_VERTEX_ARRAY);
		}
	
	if(dataItem->pointRenderer!=0)
		{
		/* Unbind the point rendering texture: */
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Disable the point rendering shader: */
		GLShader::disablePrograms();
		}
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

void EarthquakeSet::glRenderAction(const EarthquakeSet::Point& eyePos,bool front,GLContextData& contextData) const
	{
	/* Get a pointer to the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Save OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT);
	
	if(dataItem->pointRenderer!=0)
		{
		/* Enable point sprites: */
		glEnable(GL_POINT_SPRITE_ARB);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
		
		/* Bind the point rendering texture: */
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D,dataItem->pointTextureObjectId);
		glTexEnvi(GL_POINT_SPRITE_ARB,GL_COORD_REPLACE_ARB,GL_TRUE);
		
		/* Check if the point renderer program conforms to current OpenGL state: */
		bool fog=glIsEnabled(GL_FOG);
		if(dataItem->clipPlaneVersion!=contextData.getClipPlaneTracker()->getVersion()||dataItem->fog!=fog||dataItem->layeredRendering!=layeredRendering)
			{
			/* Rebuild the point rendering shader for the new settings: */
			dataItem->clipPlaneVersion=contextData.getClipPlaneTracker()->getVersion();
			dataItem->fog=glIsEnabled(GL_FOG);
			dataItem->layeredRendering=layeredRendering;
			createShader(dataItem,*contextData.getClipPlaneTracker());
			}
		
		/* Enable the point renderer program: */
		dataItem->pointRenderer->useProgram();
		
		/* Set the main uniform variables: */
		glUniform1fARB(dataItem->scaledPointRadiusLocation,pointRadius);
		glUniform1fARB(dataItem->highlightTimeLocation,float(highlightTime));
		glUniform1fARB(dataItem->currentTimeLocation,float(currentTime));
		glUniform1iARB(dataItem->pointTextureLocation,0);
		
		if(layeredRendering)
			{
			/* Calculate the front sphere: */
			Geometry::HVector<Point::Scalar,3> frontSphereCenter=Geometry::mid(eyePos,earthCenter);
			float frontSphereRadius2=Geometry::sqrDist(eyePos,earthCenter)*0.25f;
			
			/* Set the layered rendering uniform variables: */
			glUniform4fvARB(dataItem->frontSphereCenterLocation,1,frontSphereCenter.getComponents());
			glUniform1fARB(dataItem->frontSphereRadius2Location,frontSphereRadius2);
			glUniform1iARB(dataItem->frontSphereTestLocation,front);
			}
		}
	else
		{
		/* Set up standard point rendering: */
		glDisable(GL_LIGHTING);
		}
	
	if(dataItem->vertexBufferObjectId>0)
		{
		typedef GLGeometry::Vertex<float,2,GLubyte,4,void,float,3> Vertex;
		
		/* Bind the point set's vertex buffer object: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		GLVertexArrayParts::enable(Vertex::getPartsMask());
		glVertexPointer(static_cast<Vertex*>(0));
		
		if(dataItem->sortedPointIndicesBufferObjectId>0)
			{
			/* Bind the point indices buffer: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->sortedPointIndicesBufferObjectId);
			
			/* Check if the eye position changed since the last rendering pass: */
			if(dataItem->eyePos!=eyePos)
				{
				/* Re-sort the points according to the new eye position: */
				GLuint* bufferPtr=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
				#if EARTHQUAKESET_EXPLICIT_RECURSION
				drawBackToFront(eyePos,bufferPtr);
				#else
				drawBackToFront(0,int(events.size())-1,0,eyePos,bufferPtr);
				#endif
				glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
				
				dataItem->eyePos=eyePos;
				}
			
			/* Render the vertex array in back-to-front order: */
			glDrawElements(GL_POINTS,events.size(),GL_UNSIGNED_INT,0);
			
			/* Protect the point indices buffer: */
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
			}
		else
			{
			/* Render the points in arbitrary order: */
			glDrawArrays(GL_POINTS,0,events.size());
			}
		
		/* Protect the vertex buffer object: */
		GLVertexArrayParts::disable(Vertex::getPartsMask());
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		/* Render the earthquake set as a regular vertex array of points: */
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT,sizeof(Event),&events[0].position);
		glDrawArrays(GL_POINTS,0,events.size());
		glDisableClientState(GL_VERTEX_ARRAY);
		}
	
	if(dataItem->pointRenderer!=0)
		{
		/* Unbind the point rendering texture: */
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Disable the point rendering shader: */
		GLShader::disablePrograms();
		}
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

const EarthquakeSet::Event* EarthquakeSet::selectEvent(const EarthquakeSet::Point& pos,float maxDist) const
	{
	const Event* result=0;
	
	float minDist2=Math::sqr(maxDist);
	for(std::vector<Event>::const_iterator eIt=events.begin();eIt!=events.end();++eIt)
		{
		float dist2=Geometry::sqrDist(pos,eIt->position);
		if(minDist2>dist2)
			{
			result=&(*eIt);
			minDist2=dist2;
			}
		}
	
	return result;
	}

const EarthquakeSet::Event* EarthquakeSet::selectEvent(const EarthquakeSet::Ray& ray,float coneAngleCos) const
	{
	const Event* result=0;
	
	float coneAngleCos2=Math::sqr(coneAngleCos);
	float lambdaMin=Math::Constants<float>::max;
	for(std::vector<Event>::const_iterator eIt=events.begin();eIt!=events.end();++eIt)
		{
		Ray::Vector sp=eIt->position-ray.getOrigin();
		float x=sp*ray.getDirection();
		if(x>=0.0f&&x<lambdaMin)
			{
			if(Math::sqr(x)>coneAngleCos2*Geometry::sqr(sp))
				{
				result=&(*eIt);
				lambdaMin=x;
				}
			}
		}
	
	return result;
	}
