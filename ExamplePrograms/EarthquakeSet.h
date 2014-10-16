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

#ifndef EARTHQUAKESET_INCLUDED
#define EARTHQUAKESET_INCLUDED

#include <vector>
#include <IO/File.h>
#include <IO/Directory.h>
#include <Math/Interval.h>
#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLColorMap.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Vector;
template <class ScalarParam>
class Geoid;
}
class GLClipPlaneTracker;
class GLShader;

#define EARTHQUAKESET_EXPLICIT_RECURSION 1

class EarthquakeSet:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Math::Interval<double> TimeRange; // Range for earthquake event times
	typedef Geometry::Point<float,3> Point; // Type for points
	typedef Geometry::Ray<float,3> Ray; // Type for rays
	
	struct Event // Structure for events (earthquakes)
		{
		/* Elements: */
		public:
		Point position; // 3D earthquake position in Cartesian coordinates
		double time; // Earthquake time in seconds since the epoch (UTC)
		float magnitude; // Earthquake magnitude
		};
	
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer object that contains the earthquake set (0 if extension not supported)
		GLShader* pointRenderer; // Pointer to GLSL shader to render properly scaled, texture-mapped points (0 if extension not supported)
		unsigned int clipPlaneVersion; // Version number of clipping plane state compiled into the current shader object
		bool fog; // Flag whether fog blending is enabled in the current shader object
		bool layeredRendering; // Flag whether layered rendering is enabled in the current shader object
		GLint scaledPointRadiusLocation; // Location of point radius uniform variable in shader program
		GLint highlightTimeLocation; // Location of highlight time uniform variable in shader program
		GLint currentTimeLocation; // Location of current time uniform variable in shader programs
		GLint frontSphereCenterLocation;
		GLint frontSphereRadius2Location;
		GLint frontSphereTestLocation;
		GLint pointTextureLocation; // Location of texture sample uniform variable in shader program
		GLuint pointTextureObjectId; // ID of the point texture object
		Point eyePos; // The eye position for which the points have been sorted in depth order
		GLuint sortedPointIndicesBufferObjectId; // ID of index buffer containing the indices of points, sorted in depth order from the current eye position
		
		/* Constructors and destructors: */
		public:
		DataItem(void); // Creates a data item
		virtual ~DataItem(void); // Destroys a data item
		};
	
	/* Elements: */
	GLColorMap colorMap; // A color map for event magnitudes
	std::vector<Event> events; // Vector of earthquakes
	int* treePointIndices; // Array of event indices in kd-tree order
	bool layeredRendering; // Flag whether layered rendering is requested
	Point earthCenter; // Position of earth's center point for layered rendering
	float pointRadius; // Point radius in model space
	double highlightTime; // Time span (in real time) for which earthquake events are highlighted during animation
	double currentTime; // Current event time during animation
	
	/* Private methods: */
	void loadANSSFile(IO::FilePtr earthquakeFile,const Geometry::Geoid<double>& referenceEllipsoid,const Geometry::Vector<double,3>& offset,double scaleFactor); // Loads an earthquake event file in ANSS readable database snapshot format
	void loadCSVFile(IO::FilePtr earthquakeFile,const Geometry::Geoid<double>& referenceEllipsoid,const Geometry::Vector<double,3>& offset,double scaleFactor); // Loads an earthquake event file in space- or comma-separated format
	#if EARTHQUAKESET_EXPLICIT_RECURSION
	void drawBackToFront(const Point& eyePos,GLuint* indexBuffer) const; // Creates an index buffer for the earthquake set in back-to-front order for the given eye position
	#else
	void drawBackToFront(int left,int right,int splitDimension,const Point& eyePos,GLuint*& bufferPtr) const; // Renders the given kd-tree subtree in back-to-front order
	#endif
	void createShader(DataItem* dataItem,const GLClipPlaneTracker& cpt) const; // Creates the particle rendering shader based on current OpenGL settings
	
	/* Constructors and destructors: */
	public:
	EarthquakeSet(IO::DirectoryPtr directory,const char* earthquakeFileName,const Geometry::Geoid<double>& referenceEllipsoid,const Geometry::Vector<double,3>& offset,double scaleFactor,const GLColorMap& sColorMap); // Creates an earthquake set by reading a file; transforms lon/lat/ellipsoid height to Cartesian coordinates using given reference ellipsoid, adds offset vector and scales afterwards
	~EarthquakeSet(void);
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	TimeRange getTimeRange(void) const; // Returns the range of event times
	void enableLayeredRendering(const Point& newEarthCenter); // Enables layered rendering where earthquakes properly blend with the Earth's inner and outer core and surface
	void disableLayeredRendering(void); // Disables layered rendering
	void setPointRadius(float newPointRadius); // Sets the point radius in model space
	void setHighlightTime(double newHighlightTime); // Sets the time span for which events are highlighted during animation
	void setCurrentTime(double newCurrentTime); // Sets the current event time during animation
	void glRenderAction(GLContextData& contextData) const; // Renders the earthquake set
	void glRenderAction(const Point& eyePos,bool front,GLContextData& contextData) const; // Renders the earthquake set in blending order from the given eye point
	void glRenderAction(const Point& eyePos,GLContextData& contextData) const // Shortcut method to render the front and back halves of the earthquake set whether or not layered rendering is enabled
		{
		/* Render the back half (or both halves if layered rendering is disabled): */
		glRenderAction(eyePos,false,contextData);
		if(layeredRendering)
			{
			/* Render the front half: */
			glRenderAction(eyePos,true,contextData);
			}
		}
	const Event* selectEvent(const Point& pos,float maxDist) const; // Returns the event closest to the given query point (or null pointer)
	const Event* selectEvent(const Ray& ray,float coneAngleCos) const; // Ditto, for query ray
	};

#endif
