/***********************************************************************
Doom3MD5Anim - Class to represent animation sequences for animated mesh
models in Doom3's MD5 format.
Copyright (c) 2007-2010 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_INTERNAL_DOOM3MD5ANIM_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3MD5ANIM_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>

/* Forward declarations: */
namespace SceneGraph {
class Doom3FileManager;
class Doom3MD5Mesh;
}

namespace SceneGraph {

class Doom3MD5Anim
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	private:
	typedef Geometry::OrthonormalTransformation<Scalar,3> Transform; // Type for joint transformations
	
	struct Joint // Structure to represent individual joints in the mesh's skeleton
		{
		/* Elements: */
		public:
		char* name; // Joint's name as read from the model file
		int parentIndex; // Index of joint's parent (-1 if root joint)
		int flags;
		int startIndex;
		Vector baseTranslation; // Base translation of joint
		Scalar baseRotation[3]; // Base rotation of joint as reduced quaternion
		
		/* Constructors and destructors: */
		Joint(void)
			:name(0)
			{
			}
		~Joint(void)
			{
			delete[] name;
			}
		};
	
	/* Elements: */
	int numFrames; // Number of frames in the animation sequence
	int numJoints; // Total number of joints in the mesh's skeleton
	Joint* joints; // Array containing the skeleton's joint tree
	Scalar frameRate; // Frame rate of the animation sequence
	Scalar frameTime; // Frame time (1.0 / frameRate)
	int numAnimatedComponents; // Number of components of the skeleton affected by this animation sequence
	Scalar* frameComponents; // Array of animation component values for each frame
	
	/* Constructors and destructors: */
	public:
	Doom3MD5Anim(Doom3FileManager& fileManager,const char* animFileName); // Creates an animation sequence by parsing an animation file in Doom3's MD5 format
	~Doom3MD5Anim(void);
	
	/* Methods: */
	int getNumFrames(void) const // Returns the number of frames in the animation sequence
		{
		return numFrames;
		}
	Scalar getFrameRate(void) const // Returns the frame rate of the animation sequence
		{
		return frameRate;
		}
	Scalar getFrameTime(void) const // Returns the frame time of the animation sequence
		{
		return frameTime;
		}
	void animateMesh(Doom3MD5Mesh* mesh,int frameIndex) const; // Applies an animation frame to the given target mesh (mesh must match)
	};

}

#endif
