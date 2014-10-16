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

#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

#include "JelloAtom.h"

/**********************************
Static elements of class JelloAtom:
**********************************/

JelloAtom::Scalar JelloAtom::vertexForceRadius;
JelloAtom::Scalar JelloAtom::vertexForceRadius2;
JelloAtom::Scalar JelloAtom::vertexForceStrength;
JelloAtom::Scalar JelloAtom::radius;
JelloAtom::Scalar JelloAtom::radius2;
JelloAtom::Scalar JelloAtom::centralForceOvershoot;
JelloAtom::Scalar JelloAtom::centralForceRadius;
JelloAtom::Scalar JelloAtom::centralForceRadius2;
JelloAtom::Scalar JelloAtom::centralForceStrength;
JelloAtom::Scalar JelloAtom::mass;
JelloAtom::Scalar JelloAtom::inertia;
JelloAtom::Vector JelloAtom::vertexOffsets[6];

/**************************
Methods of class JelloAtom:
**************************/

void JelloAtom::removeBond(int vertexIndex)
	{
	Bond& b=bonds[vertexIndex];
	if(b.atom!=0)
		{
		Bond& o=b.atom->bonds[b.vertexIndex];
		o.atom=0;
		o.vertexIndex=-1;
		b.atom=0;
		b.vertexIndex=-1;
		}
	}

void JelloAtom::initClass(void)
	{
	/* Initialize force computation formula coefficients: */
	vertexForceRadius=Scalar(1.0);
	vertexForceRadius2=Math::sqr(vertexForceRadius);
	vertexForceStrength=Scalar(1250.0);
	radius=Scalar(1.0);
	radius2=Math::sqr(radius);
	centralForceOvershoot=Scalar(1.0/3.0);
	centralForceRadius=Scalar(2)*radius+centralForceOvershoot;
	centralForceRadius2=Math::sqr(centralForceRadius);
	centralForceStrength=Scalar(2000.0);
	mass=Scalar(1.0);
	inertia=mass*radius2;
	
	/* Calculate vertex offset radius based on force computation formula coefficients: */
	Scalar vertexRadius=radius*(Scalar(1)-(vertexForceRadius*centralForceStrength*centralForceOvershoot)/(Math::sqr(centralForceRadius)*vertexForceStrength));
	
	/* Compute vertex offsets: */
	for(int i=0;i<6;++i)
		vertexOffsets[i]=Vector::zero;
	for(int i=0;i<3;++i)
		{
		vertexOffsets[2*i+0][i]=-vertexRadius;
		vertexOffsets[2*i+1][i]=vertexRadius;
		}
	}

JelloAtom::JelloAtom(void)
	:locked(false),
	 linearVelocity(Vector::zero),angularVelocity(Vector::zero),
	 linearAcceleration(Vector::zero),angularAcceleration(Vector::zero)
	{
	}

JelloAtom::JelloAtom(const JelloAtom::Point& sPosition,const JelloAtom::Rotation& sOrientation)
	:locked(false),
	 position(sPosition),orientation(sOrientation),
	 linearVelocity(Vector::zero),angularVelocity(Vector::zero),
	 linearAcceleration(Vector::zero),angularAcceleration(Vector::zero)
	{
	}

JelloAtom::~JelloAtom(void)
	{
	/* Break all bonds involving this atom: */
	for(int i=0;i<6;++i)
		removeBond(i);
	}

void JelloAtom::setMass(JelloAtom::Scalar newMass)
	{
	mass=newMass;
	inertia=mass*radius2;
	}

void bondAtoms(JelloAtom& atom1,int vertexIndex1,JelloAtom& atom2,int vertexIndex2)
	{
	/* Break any existing bonds involving either atom vertex: */
	JelloAtom::Bond& b1=atom1.bonds[vertexIndex1];
	if(b1.atom!=0)
		{
		JelloAtom::Bond& o=b1.atom->bonds[b1.vertexIndex];
		o.atom=0;
		o.vertexIndex=-1;
		}
	JelloAtom::Bond& b2=atom2.bonds[vertexIndex2];
	if(b2.atom!=0)
		{
		JelloAtom::Bond& o=b2.atom->bonds[b2.vertexIndex];
		o.atom=0;
		o.vertexIndex=-1;
		}
	
	/* Bond the two atoms: */
	b1.atom=&atom2;
	b1.vertexIndex=vertexIndex2;
	b2.atom=&atom1;
	b2.vertexIndex=vertexIndex1;
	}

void JelloAtom::calculateForces(void)
	{
	/* Reset accelerations: */
	linearAcceleration=Vector::zero;
	angularAcceleration=Vector::zero;
	
	if(!locked)
		{
		/* Calculate forces exerted by bonds: */
		for(int i=0;i<6;++i)
			{
			Bond& b=bonds[i];
			if(b.atom!=0)
				{
				/* Calculate the repelling force between the atoms' centers: */
				Vector cdist=b.atom->position-position;
				Scalar cdistLen2=Geometry::sqr(cdist);
				if(cdistLen2<centralForceRadius2)
					{
					/* Calculate centroid repelling force: */
					Scalar cdistLen=Math::sqrt(cdistLen2);
					linearAcceleration+=cdist*(centralForceStrength*(cdistLen-centralForceRadius)/(centralForceRadius2*mass));
					}
				
				/* Calculate the global positions of both bond vertices: */
				Vector o1=orientation.transform(vertexOffsets[i]);
				Vector dist=cdist+b.atom->orientation.transform(vertexOffsets[b.vertexIndex])-o1;
				Scalar distLen2=Geometry::sqr(dist);
				
				/* Calculate vertex attracting force: */
				dist*=vertexForceStrength/(vertexForceRadius*mass);
				
				/* Apply linear acceleration: */
				linearAcceleration+=dist;
				
				/* Apply angular acceleration: */
				Vector torque=Geometry::cross(o1,dist);
				angularAcceleration+=torque*(mass/inertia);
				}
			}
		}
	}

void JelloAtom::glRenderAction(GLContextData& contextData) const
	{
	/* Show the atom's position: */
	glPointSize(5.0f);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_POINTS);
	glVertex(position);
	glEnd();
	
	/* Show the position of each bond vertex: */
	glPointSize(3.0f);
	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_POINTS);
	for(int i=0;i<6;++i)
		glVertex(position+orientation.transform(vertexOffsets[i]));
	glEnd();
	
	/* Draw a line to each bonded atom: */
	glLineWidth(1.0f);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_LINES);
	for(int i=0;i<6;++i)
		{
		const Bond& b=bonds[i];
		if(b.atom!=0)
			{
			glVertex(position+orientation.transform(vertexOffsets[i]));
			glVertex(b.atom->position+b.atom->orientation.transform(vertexOffsets[b.vertexIndex]));
			}
		}
	glEnd();
	}
