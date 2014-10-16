/***********************************************************************
JelloAtom - Class for "Jell-O atoms" forming virtual Jell-O molecules.
Copyright (c) 2006-2007 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef JELLOATOM_INCLUDED
#define JELLOATOM_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>

/* Forward declarations: */
class GLContextData;

class JelloAtom
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type
	typedef Geometry::Point<Scalar,3> Point; // Point type
	typedef Geometry::Vector<Scalar,3> Vector; // Vector type
	typedef Geometry::Rotation<Scalar,3> Rotation; // Rotation type
	
	private:
	struct Bond // Structure to form bonds between multiple atoms
		{
		/* Elements: */
		public:
		JelloAtom* atom; // Pointer to bonded atom
		int vertexIndex; // Index of bonded vertex in bonded atom
		
		/* Constructors and destructors: */
		Bond(void) // Creates an unbonded bond
			:atom(0),vertexIndex(-1)
			{
			};
		};
	
	/* Elements: */
	private:
	static Scalar vertexForceRadius,vertexForceRadius2; // Radius and squared radius of vertex force field
	static Scalar vertexForceStrength; // Strength of vertex attraction force
	static Scalar radius,radius2; // Radius and squared radius of atom's circumsphere
	static Scalar centralForceOvershoot; // Factor of how much centroid repelling force overshoots units' radii
	static Scalar centralForceRadius,centralForceRadius2; // Radius and squared radius of central force field
	static Scalar centralForceStrength; // Strength of centroid repelling force
	static Scalar mass; // Mass of an atom
	static Scalar inertia; // Moment of inertia of an atom (assumed to be isotropic)
	static Vector vertexOffsets[6]; // Offsets of bond vertices in local atom coordinates
	
	Bond bonds[6]; // Bonds with up to six other atoms
	public:
	bool locked; // Flag if the atom is currently locked (by a dragger)
	Point position; // Atoms's current position
	Rotation orientation; // Atoms's current orientation
	Vector linearVelocity,angularVelocity; // Atoms's current linear and angular velocities
	Vector linearAcceleration,angularAcceleration; // Atom's current linear and angular accelerations
	
	/* Private methods: */
	private:
	void removeBond(int vertexIndex); // Removes the bond on the atom's given vertex
	
	/* Constructors and destructors: */
	public:
	static void initClass(void); // Initializes the Jell-O atom class
	JelloAtom(void); // Creates an unbonded Jell-O atom with undefined position and orientation
	JelloAtom(const Point& position,const Rotation& orientation); // Creates an unbonded Jell-O atom
	private:
	JelloAtom(const JelloAtom& source); // Prohibit copy constructor
	JelloAtom& operator=(const JelloAtom& source); // Prohibit assignment operator
	public:
	~JelloAtom(void); // Destroys Jell-O atom
	
	/* Methods: */
	static Scalar getRadius(void) // Returns an atom's radius
		{
		return radius;
		};
	static void setMass(Scalar newMass); // Sets the mass (and moment of inertia) of all Jell-O atoms
	friend void bondAtoms(JelloAtom& atom1,int vertexIndex1,JelloAtom& atom2,int vertexIndex2); // Bonds the two given atoms
	void calculateForces(void); // Calculates interaction forces working on this atom
	void glRenderAction(GLContextData& contextData) const; // Renders the atom and its bonds
	};

#endif
