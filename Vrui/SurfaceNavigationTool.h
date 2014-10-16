/***********************************************************************
SurfaceNavigationTool - Base class for navigation tools that are limited
to navigate along an application-defined surface.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef VRUI_SURFACENAVIGATIONTOOL_INCLUDED
#define VRUI_SURFACENAVIGATIONTOOL_INCLUDED

#include <Math/Constants.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

/* Forward declarations: */
namespace Misc {
template <class ParameterParam>
class FunctionCall;
}

namespace Vrui {
class ToolManager;
}

namespace Vrui {

class SurfaceNavigationToolFactory:public ToolFactory
	{
	/* Constructors and destructors: */
	public:
	SurfaceNavigationToolFactory(ToolManager& toolManager);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	};

class SurfaceNavigationTool:public NavigationTool
	{
	/* Embedded classes: */
	public:
	class AlignmentState // Base class for state that alignment objects can attach to an alignment data structure between calls
		{
		/* Constructors and destructors: */
		public:
		virtual ~AlignmentState(void); // Destroys the alignment state object
		};
	
	struct AlignmentData // Structure to hold data required to align a surface frame
		{
		/* Elements: */
		public:
		const NavTransform& prevSurfaceFrame; // The aligned surface frame from a previous call
		AlignmentState* alignmentState; // Alignment object's state; stays valid while the navigation tool stays active
		NavTransform& surfaceFrame; // The surface frame to be aligned
		Scalar probeSize; // Size of a "probe" around the current surface frame's origin; probe shape is defined by application
		Scalar maxClimb; // Height above the surface frame base point at which the alignment function will start searching
		
		/* Constructors and destructors: */
		AlignmentData(const NavTransform& sPrevSurfaceFrame,NavTransform& sSurfaceFrame,Scalar sProbeSize,Scalar sMaxClimb) // Probe size and maximum climb are given in physical coordinate units
			:prevSurfaceFrame(sPrevSurfaceFrame),alignmentState(0),surfaceFrame(sSurfaceFrame),
			 probeSize(sProbeSize*surfaceFrame.getScaling()),maxClimb(sMaxClimb*surfaceFrame.getScaling())
			{
			}
		};
	
	typedef Misc::FunctionCall<AlignmentData&> AlignFunction; // Type for alignment function objects
	
	/* Elements: */
	private:
	AlignFunction* alignFunction; // Function call that aligns the passed local navigation frame to the application-defined surface
	AlignmentState* alignmentState; // Alignment object's most recent state
	protected:
	NavTransform physicalFrame; // Local navigation coordinate frame (x: right, y: forward, z: up) in physical coordinates
	
	/* Protected methods: */
	protected:
	static Scalar wrapAngle(Scalar angle) // Helper function to wrap an angle to the -pi...pi range
		{
		if(angle<-Math::Constants<Scalar>::pi)
			angle+=Scalar(2)*Math::Constants<Scalar>::pi;
		else if(angle>Math::Constants<Scalar>::pi)
			angle-=Scalar(2)*Math::Constants<Scalar>::pi;
		return angle;
		}
	static Point projectToFloor(const Point& p); // Projects the given point to the environment's floor plane along the up direction
	NavTransform& calcPhysicalFrame(const Point& basePoint); // Calculates a default physical navigation frame at the given base point in physical coordinates
	static Scalar calcAzimuth(const Rotation& orientation); // Helper function to calculate the azimuth angle of the given orientation with respect to a standard physical frame
	static void calcEulerAngles(const Rotation& orientation,Scalar angles[3]); // Helper function to calculate the full set of Euler angles of the given orientation with respect to a standard physical frame
	void align(AlignmentData& alignmentData); // Aligns the given navigation frame with an application-defined surface
	
	/*********************************************************************
	Method to align a navigation frame with an application-defined surface
	and return the Euler angles of the original frame with respect to the
	aligned frame. Order of angle application:
	1. Azimuth is rotation around the aligned frame's Z axis,
	   from -pi to +pi.
	2. Elevation is rotation around the new X axis from step 1,
	   from -pi/2 to +pi/2.
	3. Roll is rotation around the new Y axis from step 2,
	   from -pi to +pi.
	*********************************************************************/
	
	void align(AlignmentData& alignmentData,Scalar& azimuth,Scalar& elevation,Scalar& roll); // Ditto, and calculates Euler angles of the original surface frame with respect to the aligned surface frame
	
	/* Constructors and destructors: */
	public:
	SurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~SurfaceNavigationTool(void);
	
	/* Override methods from NavigationTool: */
	void deactivate(void);
	
	/* New methods: */
	void setAlignFunction(AlignFunction* newAlignFunction); // Sets a new align function; inherits function call object
	};

}

#endif
