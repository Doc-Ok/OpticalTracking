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

#include "JelloCrystal.h"

#include <Math/Math.h>
#include <Math/Random.h>
#include <Math/Constants.h>
#include <Geometry/Sphere.h>

/*****************************
Methods of class JelloCrystal:
*****************************/

JelloCrystal::JelloCrystal(void)
	:atomMass(1.0),
	 attenuation(0.5),
	 gravity(20.0),
	 domain(Point(-60.0,-36.0,0.0),Point(60.0,60.0,96.0)),
	 atomStates(0)
	{
	/* Initialize the Jell-O crystal: */
	JelloAtom::initClass();
	JelloAtom::setMass(atomMass);
	}

JelloCrystal::JelloCrystal(const JelloCrystal::Index& numAtoms)
	:atomMass(1.0),
	 attenuation(0.5),
	 gravity(20.0),
	 crystal(numAtoms),
	 domain(Point(-60.0,-36.0,0.0),Point(60.0,60.0,96.0)),
	 atomStates(0)
	{
	/* Initialize the Jell-O crystal: */
	JelloAtom::initClass();
	JelloAtom::setMass(atomMass);
	setNumAtoms(numAtoms);
	}

JelloCrystal::JelloCrystal(const JelloCrystal::Index& numAtoms,const JelloCrystal::Box& sDomain)
	:atomMass(1.0),
	 attenuation(0.5),
	 gravity(20.0),
	 crystal(numAtoms),
	 domain(sDomain),
	 atomStates(0)
	{
	/* Initialize the Jell-O crystal: */
	JelloAtom::initClass();
	JelloAtom::setMass(atomMass);
	setNumAtoms(numAtoms);
	}

JelloCrystal::~JelloCrystal(void)
	{
	delete[] atomStates;
	}

void JelloCrystal::setNumAtoms(const JelloCrystal::Index& newNumAtoms)
	{
	/* Initialize the Jell-O crystal: */
	crystal.resize(newNumAtoms);
	
	/* Determine the position of the crystal: */
	Scalar atomDist=JelloAtom::getRadius()*Scalar(2);
	Point crystalCenter;
	for(int i=0;i<2;++i)
		crystalCenter[i]=Math::mid(domain.min[i],domain.max[i]);
	crystalCenter[2]=Scalar(crystal.getSize(2)-1)*atomDist*Scalar(0.5)+domain.min[2];
	
	/* Initialize the positions of all atoms and create all bonds: */
	for(Crystal::Index index=crystal.beginIndex();index!=crystal.endIndex();crystal.preInc(index))
		{
		/* Set the atom's position and orientation: */
		for(int i=0;i<3;++i)
			{
			crystal(index).position[i]=crystalCenter[i]+Scalar(index[i])*atomDist-Scalar(newNumAtoms[i]-1)*atomDist*Scalar(0.5);
			// crystal(index).position[i]+=Scalar(Math::randUniformCC(-atomDist*0.4,atomDist*0.4));
			}
		crystal(index).orientation=Rotation::identity;
		
		/* Bond the atom to its neighbours: */
		for(int i=0;i<3;++i)
			{
			if(index[i]>0)
				{
				Crystal::Index ni=index;
				--ni[i];
				bondAtoms(crystal(index),2*i+0,crystal(ni),2*i+1);
				}
			if(index[i]<newNumAtoms[i]-1)
				{
				Crystal::Index ni=index;
				++ni[i];
				bondAtoms(crystal(index),2*i+1,crystal(ni),2*i+0);
				}
			}
		}
	
	/* Initialize the simulation state: */
	delete[] atomStates;
	atomStates=new AtomState[crystal.getNumElements()];
	}

void JelloCrystal::setAtomMass(JelloCrystal::Scalar newAtomMass)
	{
	atomMass=newAtomMass;
	JelloAtom::setMass(atomMass);
	}

void JelloCrystal::setAttenuation(JelloCrystal::Scalar newAttenuation)
	{
	attenuation=newAttenuation;
	}

void JelloCrystal::setGravity(JelloCrystal::Scalar newGravity)
	{
	gravity=newGravity;
	}

void JelloCrystal::setDomain(const JelloCrystal::Box& newDomain)
	{
	/* Set the new domain: */
	domain=newDomain;
	
	/* Re-initialize the atoms: */
	setNumAtoms(crystal.getSize());
	}

JelloCrystal::AtomID JelloCrystal::pickAtom(const JelloCrystal::Point& p) const
	{
	AtomID result=crystal.end();
	
	/* Compare the picking position against each unlocked atom in the crystal: */
	Scalar minDist2=Math::sqr(JelloAtom::getRadius()*Scalar(1.5));
	for(Crystal::const_iterator cIt=crystal.begin();cIt!=crystal.end();++cIt)
		if(!cIt->locked) // No, you can't pick this atom -- not yours!
			{
			Scalar dist2=Geometry::sqrDist(p,cIt->position);
			if(minDist2>dist2)
				{
				result=cIt;
				minDist2=dist2;
				}
			}
	
	return result;
	}

JelloCrystal::AtomID JelloCrystal::pickAtom(const JelloCrystal::Ray& r) const
	{
	AtomID result=crystal.end();
	Scalar minLambda=Math::Constants<Scalar>::max;
	
	/* Intersect the ray with a sphere around each unlocked atom in the crystal: */
	Geometry::Sphere<Scalar,3> sphere(Point::origin,JelloAtom::getRadius()*Scalar(1.5));
	for(Crystal::const_iterator cIt=crystal.begin();cIt!=crystal.end();++cIt)
		if(!cIt->locked) // No, you can't pick this atom -- not yours!
			{
			/* Move the test sphere to the atom's position: */
			sphere.setCenter(cIt->position);
			
			/* Intersect it with the picking ray: */
			Geometry::Sphere<Scalar,3>::HitResult hr=sphere.intersectRay(r);
			
			/* Check if this is the closest valid intersection: */
			if(hr.isValid()&&hr.getParameter()<minLambda)
				{
				result=cIt;
				minLambda=hr.getParameter();
				}
			}
	
	return result;
	}

bool JelloCrystal::lockAtom(JelloCrystal::AtomID atom)
	{
	/* Check if the atom is valid and not yet locked: */
	if(atom!=crystal.end()&&!atom->locked)
		{
		/* Lock the atom: */
		const_cast<Crystal::iterator>(atom)->locked=true;
		
		return true;
		}
	else
		return false;
	}

void JelloCrystal::setAtomState(JelloCrystal::AtomID atom,const JelloCrystal::ONTransform& newAtomState)
	{
	Crystal::iterator cIt=const_cast<Crystal::iterator>(atom);
	cIt->position=newAtomState.getOrigin();
	cIt->orientation=newAtomState.getRotation();
	cIt->linearVelocity=Vector::zero;
	cIt->angularVelocity=Vector::zero;
	cIt->linearAcceleration=Vector::zero;
	cIt->angularAcceleration=Vector::zero;
	}

void JelloCrystal::unlockAtom(JelloCrystal::AtomID atom)
	{
	const_cast<Crystal::iterator>(atom)->locked=false;
	}

void JelloCrystal::simulate(JelloCrystal::Scalar timeStep)
	{
	/* Calculate the effective velocity attenuation for this time step: */
	Scalar att=Math::pow(attenuation,timeStep);
	
	/***********************************************************
	Perform a fourth-order Runge-Kutta-Nystrom integration step:
	***********************************************************/
	
	AtomState* asPtr;
	Scalar f1,f2;
	
	/* Save initial atom states and calculate accelerations on all atoms: */
	asPtr=atomStates;
	for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt,++asPtr)
		{
		/* Save initial atom state: */
		asPtr->position=aIt->position;
		asPtr->orientation=aIt->orientation;
		
		/* Calculate interaction forces: */
		aIt->calculateForces();
		
		/* Add gravity: */
		if(aIt->position[2]>domain.min[2])
			aIt->linearAcceleration[2]-=gravity;
		
		/* Store acceleration: */
		asPtr->linearAcceleration[0]=aIt->linearAcceleration;
		asPtr->angularAcceleration[0]=aIt->angularAcceleration;
		}
	
	/* Move all atoms to the first evaluation position: */
	f1=timeStep*Scalar(0.5);
	f2=timeStep*timeStep*Scalar(0.125);
	asPtr=atomStates;
	for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt,++asPtr)
		{
		/* Update the atom's position and orientation: */
		Vector dP=aIt->linearVelocity*f1;
		dP+=asPtr->linearAcceleration[0]*f2;
		aIt->position+=dP;
		Vector dO=aIt->angularVelocity*f1;
		dO+=asPtr->angularAcceleration[0]*f2;
		aIt->orientation.leftMultiply(Rotation(dO));
		}
	
	/* Calculate accelerations on all atoms: */
	asPtr=atomStates;
	for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt,++asPtr)
		{
		/* Calculate interaction forces: */
		aIt->calculateForces();
		
		/* Add gravity: */
		if(aIt->position[2]>domain.min[2])
			aIt->linearAcceleration[2]-=gravity;
		
		/* Store acceleration: */
		asPtr->linearAcceleration[1]=aIt->linearAcceleration;
		asPtr->angularAcceleration[1]=aIt->angularAcceleration;
		}
	
	/* Move all atoms to the second evaluation position: */
	f1=timeStep;
	f2=timeStep*timeStep*Scalar(0.5);
	asPtr=atomStates;
	for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt,++asPtr)
		{
		/* Update the atom's position and orientation: */
		Vector dP=aIt->linearVelocity*f1;
		dP+=asPtr->linearAcceleration[1]*f2;
		aIt->position=asPtr->position;
		aIt->position+=dP;
		Vector dO=aIt->angularVelocity*f1;
		dO+=asPtr->angularAcceleration[0]*f2;
		aIt->orientation=asPtr->orientation;
		aIt->orientation.leftMultiply(Rotation(dO));
		}
	
	/* Calculate accelerations on all atoms: */
	asPtr=atomStates;
	for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt,++asPtr)
		{
		/* Calculate interaction forces: */
		aIt->calculateForces();
		
		/* Add gravity: */
		if(aIt->position[2]>domain.min[2])
			aIt->linearAcceleration[2]-=gravity;
		
		/* Store acceleration: */
		asPtr->linearAcceleration[2]=aIt->linearAcceleration;
		asPtr->angularAcceleration[2]=aIt->angularAcceleration;
		}
	
	/* Move all atoms to the end of the time step: */
	f1=timeStep;
	f2=timeStep*timeStep/Scalar(6);
	Scalar f3=timeStep/Scalar(6);
	asPtr=atomStates;
	for(Crystal::iterator aIt=crystal.begin();aIt!=crystal.end();++aIt,++asPtr)
		{
		/* Update the atom's position and orientation: */
		Vector dP=aIt->linearVelocity*f1;
		dP+=(asPtr->linearAcceleration[0]+asPtr->linearAcceleration[1]*Scalar(2))*f2;
		aIt->position=asPtr->position;
		aIt->position+=dP;
		Vector dO=aIt->angularVelocity*f1;
		dO+=(asPtr->angularAcceleration[0]+asPtr->angularAcceleration[1]*Scalar(2))*f2;
		aIt->orientation=asPtr->orientation;
		aIt->orientation.leftMultiply(Rotation(dO));
		aIt->orientation.renormalize();
		
		/* Update the atom's linear and angular velocities: */
		aIt->linearVelocity+=(asPtr->linearAcceleration[0]+asPtr->linearAcceleration[1]*Scalar(4)+asPtr->linearAcceleration[2])*f3;
		aIt->angularVelocity+=(asPtr->angularAcceleration[0]+asPtr->angularAcceleration[1]*Scalar(4)+asPtr->angularAcceleration[2])*f3;
		
		/* Limit the atom to the domain box: */
		for(int i=0;i<3;++i)
			{
			if(aIt->position[i]<domain.min[i])
				{
				aIt->position[i]=Scalar(2)*domain.min[i]-aIt->position[i];
				aIt->linearVelocity[i]=-aIt->linearVelocity[i];
				}
			else if(aIt->position[i]>domain.max[i])
				{
				aIt->position[i]=Scalar(2)*domain.max[i]-aIt->position[i];
				aIt->linearVelocity[i]=-aIt->linearVelocity[i];
				}
			}
		
		/* Attenuate the atom's velocities: */
		aIt->linearVelocity*=att;
		aIt->angularVelocity*=att;
		}
	}
