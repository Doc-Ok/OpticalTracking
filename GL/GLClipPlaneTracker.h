/***********************************************************************
GLClipPlaneTracker - Class to keep track of changes to OpenGL's clipping
plane state to support just-in-time compilation of GLSL shaders
depending on the OpenGL context's current clipping state.
Copyright (c) 2012 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLCLIPPLANETRACKER_INCLUDED
#define GLCLIPPLANETRACKER_INCLUDED

#include <string>
#include <GL/gl.h>
#include <GL/GLVector.h>

class GLClipPlaneTracker
	{
	/* Embedded classes: */
	public:
	typedef GLVector<GLdouble,3> Vector; // Type for points and vectors
	typedef GLVector<GLdouble,4> Plane; // Type for plane equations in homogeneous coordinates
	
	class ClipPlaneState // State of an OpenGL clipping plane
		{
		friend class GLClipPlaneTracker;
		
		/* Elements: */
		private:
		bool enabled; // Flag whether this clipping plane is enabled
		Plane plane; // The plane equation of this clipping plane in eye coordinates
		
		/* Methods: */
		public:
		bool isEnabled(void) const // Returns true if the clipping plane is enabled
			{
			return enabled;
			}
		const Plane& getPlane(void) const // Returns the clipping plane's plane equation in eye coordinates
			{
			return plane;
			}
		};
	
	/* Elements: */
	private:
	unsigned int version; // Number incremented each time the tracker detects a change to OpenGL's clipping plane state
	int maxNumClipPlanes; // Maximum number of clipping planes supported by this OpenGL context
	ClipPlaneState* clipPlaneStates; // Array of OpenGL clipping plane states
	
	/* Constructors and destructors: */
	public:
	GLClipPlaneTracker(void); // Creates a clipping plane tracker for the current OpenGL context
	~GLClipPlaneTracker(void); // Destroys the clipping plane tracker
	
	/* Methods: */
	unsigned int getVersion(void) const // Returns the version number of the current clipping plane state
		{
		return version;
		}
	
	/* Methods to query current clipping plane state: */
	int getMaxNumClipPlanes(void) const // Returns the maximum number of clipping planes supported by the OpenGL context
		{
		return maxNumClipPlanes;
		}
	int getNumEnabledClipPlanes(void) const; // Returns the number of currently enabled clipping planes
	const ClipPlaneState& getClipPlaneState(int clipPlaneIndex) const // Returns the current clipping plane state structure of the given clipping plane
		{
		return clipPlaneStates[clipPlaneIndex];
		}
	
	/* Methods to update OpenGL clipping plane state; return true if current clipping plane state changed due to provided values: */
	bool enableClipPlane(int clipPlaneIndex,const Plane& plane); // Enables and sets the given clipping plane in the current OpenGL context using a homogeneous plane equation
	bool enableClipPlane(int clipPlaneIndex,const Vector& planeNormal,const Vector& planePoint); // Enables and sets the given clipping plane in the current OpenGL context using an affine plane equation
	bool disableClipPlane(int clipPlaneIndex); // Disables the given clipping plane in the current OpenGL context
	
	bool update(void); // Updates the tracker by querying the current OpenGL context's state; returns true if state has changed since last check
	std::string createCalcClipDistances(const char* vertexEc) const; // Returns a vertex shader fragment to calculate the clip distances of the eye-coordinate vertex with respect to all enabled clip planes
	
	/* Methods to temporarily disable/enable all clipping planes: */
	void pause(void) const; // Temporarily disables clipping
	void resume(void) const; // Re-enables clipping after a call to pause()
	};

#endif
