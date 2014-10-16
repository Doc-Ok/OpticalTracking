/***********************************************************************
JelloCrystal - Class to simulate the behavior of crystals of Jell-O
atoms using a real-time ODE solver based on a fourth-order Runge-Kutta-
Nystrom method.
Copyright (c) 2007-2014 Oliver Kreylos

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

#ifndef JELLOCRYSTAL_INCLUDED
#define JELLOCRYSTAL_INCLUDED

#include <Misc/Array.h>
#include <Geometry/Ray.h>
#include <Geometry/Box.h>
#include <Geometry/OrthonormalTransformation.h>

#include "JelloAtom.h"

/* Forward declarations: */
class JelloRenderer;

class JelloCrystal
	{
	friend class JelloRenderer; // A class to render Jell-O crystals using OpenGL
	
	/* Embedded classes: */
	public:
	typedef JelloAtom::Scalar Scalar; // Scalar type
	typedef JelloAtom::Point Point; // Point type
	typedef JelloAtom::Vector Vector; // Vector type
	typedef JelloAtom::Rotation Rotation; // Rotation type
	typedef Geometry::Ray<Scalar,3> Ray; // Type for rays
	typedef Geometry::Box<Scalar,3> Box; // Type for axis-aligned bounding boxes
	typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform; // Type for atom positions/orientations
	typedef Misc::Array<JelloAtom,3> Crystal; // Type to represent "Jell-O crystals" as 3D arrays of Jell-O atoms
	typedef Crystal::Index Index; // Type for indices into 3D arrays and array sizes
	typedef Crystal::const_iterator AtomID; // Atom handle type used by class clients
	
	private:
	struct AtomState // Structure to buffer the state of an atom during Runge-Kutta-Nystrom integration
		{
		/* Elements: */
		public:
		Point position;
		Rotation orientation;
		Vector linearAcceleration[3];
		Vector angularAcceleration[3];
		};
	
	/* Elements: */
	Scalar atomMass; // Mass of a single Jell-O atom
	Scalar attenuation; // The velocity attenuation factor
	Scalar gravity; // The gravity acceleration constant
	Crystal crystal; // The virtual Jell-O crystal
	Box domain; // The box containing the Jell-O crystal
	AtomState* atomStates; // Buffer of atom states for Runge-Kutta-Nystrom integration
	
	/* Constructors and destructors: */
	public:
	JelloCrystal(void); // Creates invalid Jell-O crystal
	JelloCrystal(const Index& numAtoms); // Creates a Jell-O crystal of the given size
	JelloCrystal(const Index& numAtoms,const Box& sDomain); // Creates a Jell-O crystal of the given size inside the given domain
	~JelloCrystal(void);
	
	/* Methods: */
	void setNumAtoms(const Index& newNumAtoms); // Changes the size of an existing Jell-O crystal
	Scalar getAtomMass(void) const // Returns the current Jell-O atom mass
		{
		return atomMass;
		};
	Scalar getAttenuation(void) const // Returns the current attenuation setting
		{
		return attenuation;
		};
	Scalar getGravity(void) const // Returns the current gravity setting
		{
		return gravity;
		};
	const Index& getNumAtoms(void) const // Returns the size of the Jell-O crystal
		{
		return crystal.getSize();
		};
	const Box& getDomain(void) const // Returns the domain box of the Jell-O simulation
		{
		return domain;
		};
	void setAtomMass(Scalar newAtomMass); // Sets the atom mass
	void setAttenuation(Scalar newAttenuation); // Sets the attenuation
	void setGravity(Scalar newGravity); // Sets the gravity
	void setDomain(const Box& newDomain); // Sets the simulation domain; resets the position and orientation of the Jell-O crystal
	AtomID pickAtom(const Point& p) const; // Picks a Jell-O atom based on a 3D position
	AtomID pickAtom(const Ray& r) const; // Picks a Jell-O atom based on a 3D ray
	bool isValid(AtomID atom) const // Checks if an atom ID is valid
		{
		return atom!=crystal.end();
		};
	bool lockAtom(AtomID atom); // Tries locking the given atom; returns true if the atom is valid and was locked
	ONTransform getAtomState(AtomID atom) const // Returns the position and orientation of the given atom; atom must be locked by caller (fails on invalid atom)
		{
		return ONTransform(atom->position-Point::origin,atom->orientation);
		};
	void setAtomState(AtomID atom,const ONTransform& newAtomState); // Sets the state of an atom; atom must be locked by caller (fails on invalid atom)
	void unlockAtom(AtomID atom); // Unlocks an atom; atom must be locked by caller (fails on invalid atom)
	void simulate(Scalar timeStep); // Advances the simulation by the given time step
	template <class PipeParam>
	void writeAtomStates(PipeParam& pipe) const // Writes the states of all atoms to a pipe that supports typed writes
		{
		/* Write the positions of all atoms: */
		for(Crystal::const_iterator aIt=crystal.begin();aIt!=crystal.end();++aIt)
			pipe.write(aIt->position.getComponents(),3);
		};
	template <class PipeParam>
	void readAtomStates(PipeParam& pipe) // Reads the states of all atoms from a pipe that supports typed reads
		{
		/* Read the positions of all atoms: */
		for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt)
			pipe.read(aIt->position.getComponents(),3);
		};
	void copyAtomStates(const JelloCrystal& source) // Reads the states of all atoms from another Jell-O crystal of the same size
		{
		/* Copy the positions of all atoms: */
		Crystal::const_iterator scIt;
		Crystal::iterator dcIt;
		for(scIt=source.crystal.begin(),dcIt=crystal.begin();scIt!=source.crystal.end();++scIt,++dcIt)
			dcIt->position=scIt->position;
		};
	};

#endif
