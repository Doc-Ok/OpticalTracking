/***********************************************************************
ClipPlaneManager - Class to manage clipping planes in virtual
environments. Maps created ClipPlane objects to OpenGL clipping planes.
Copyright (c) 2009-2012 Oliver Kreylos

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

#include <Vrui/ClipPlaneManager.h>

#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/GLContextData.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/DisplayState.h>

namespace Vrui {

/*********************************
Methods of class ClipPlaneManager:
*********************************/

ClipPlaneManager::ClipPlaneManager(void)
	:firstClipPlane(0),lastClipPlane(0)
	{
	}

ClipPlaneManager::~ClipPlaneManager(void)
	{
	/* Destroy all clipping planes: */
	while(firstClipPlane!=0)
		{
		ClipPlaneListItem* succ=firstClipPlane->succ;
		delete firstClipPlane;
		firstClipPlane=succ;
		}
	}

ClipPlane* ClipPlaneManager::createClipPlane(bool physical)
	{
	/* Create a new clipping plane: */
	ClipPlaneListItem* newClipPlane=new ClipPlaneListItem(physical);
	
	/* Add the new clipping plane to the list: */
	if(lastClipPlane!=0)
		lastClipPlane->succ=newClipPlane;
	else
		firstClipPlane=newClipPlane;
	lastClipPlane=newClipPlane;
	
	/* Return the new clipping plane: */
	return newClipPlane;
	}

ClipPlane* ClipPlaneManager::createClipPlane(bool physical,const Plane& sPlane)
	{
	/* Create a new clipping plane: */
	ClipPlaneListItem* newClipPlane=new ClipPlaneListItem(physical,sPlane);
	
	/* Add the new clipping plane to the list: */
	if(lastClipPlane!=0)
		lastClipPlane->succ=newClipPlane;
	else
		firstClipPlane=newClipPlane;
	lastClipPlane=newClipPlane;
	
	/* Return the new clipping plane: */
	return newClipPlane;
	}

void ClipPlaneManager::destroyClipPlane(ClipPlane* clipPlane)
	{
	/* Find the clipping plane in the list: */
	ClipPlaneListItem* lsPtr1;
	ClipPlaneListItem* lsPtr2;
	for(lsPtr1=0,lsPtr2=firstClipPlane;lsPtr2!=0&&lsPtr2!=clipPlane;lsPtr1=lsPtr2,lsPtr2=lsPtr2->succ)
		;
	
	/* Remove the clipping plane if it was found (ignore otherwise): */
	if(lsPtr2!=0)
		{
		if(lsPtr1!=0)
			lsPtr1->succ=lsPtr2->succ;
		else
			firstClipPlane=lsPtr2->succ;
		if(lastClipPlane==lsPtr2)
			lastClipPlane=lsPtr1;
		delete lsPtr2;
		}
	}

void ClipPlaneManager::setClipPlanes(bool navigationEnabled,DisplayState* displayState,GLContextData& contextData) const
	{
	/* Get the clipping plane state tracker: */
	GLClipPlaneTracker* cpt=contextData.getClipPlaneTracker();
	
	/* Process all physical clipping planes first: */
	GLsizei clipPlaneIndex=0;
	bool haveNavigationalClipPlanes=false;
	for(const ClipPlaneListItem* cpPtr=firstClipPlane;cpPtr!=0&&clipPlaneIndex<cpt->getMaxNumClipPlanes();cpPtr=cpPtr->succ)
		{
		if(cpPtr->isEnabled())
			{
			/* Only set clipping plane now if it is physical, or if there is no navigation transformation: */
			if(cpPtr->physical||!navigationEnabled)
				{
				/* Set the clipping plane in the clipping plane tracker and OpenGL: */
				GLClipPlaneTracker::Plane plane;
				for(int i=0;i<3;++i)
					plane[i]=cpPtr->getPlane().getNormal()[i];
				plane[3]=-cpPtr->getPlane().getOffset();
				cpt->enableClipPlane(clipPlaneIndex,plane);
				
				/* Increment the clipping plane index: */
				++clipPlaneIndex;
				}
			else
				haveNavigationalClipPlanes=true;
			}
		}
	
	if(haveNavigationalClipPlanes&&clipPlaneIndex<cpt->getMaxNumClipPlanes())
		{
		/* Temporarily go to navigational coordinates: */
		glPushMatrix();
		glLoadIdentity();
		glMultMatrix(displayState->modelviewNavigational);
		
		/* Process all navigational clipping planes: */
		for(const ClipPlaneListItem* cpPtr=firstClipPlane;cpPtr!=0&&clipPlaneIndex<cpt->getMaxNumClipPlanes();cpPtr=cpPtr->succ)
			{
			if(cpPtr->isEnabled()&&!cpPtr->physical)
				{
				/* Set the clipping plane in the clipping plane tracker and OpenGL: */
				GLClipPlaneTracker::Plane plane;
				for(int i=0;i<3;++i)
					plane[i]=cpPtr->getPlane().getNormal()[i];
				plane[3]=-cpPtr->getPlane().getOffset();
				cpt->enableClipPlane(clipPlaneIndex,plane);
				
				/* Increment the clipping plane index: */
				++clipPlaneIndex;
				}
			}
		
		/* Return to physical coordinates: */
		glPopMatrix();
		}
	
	/* Disable all unused clipping planes still enabled from the last pass: */
	while(clipPlaneIndex<cpt->getMaxNumClipPlanes())
		{
		cpt->disableClipPlane(clipPlaneIndex);
		++clipPlaneIndex;
		}
	}

void ClipPlaneManager::clipRay(bool physical,Ray& ray,Scalar& lambdaMax) const
	{
	/* Calculate the ray interval inside all clipping planes: */
	Scalar lambda1=Scalar(0);
	Scalar lambda2=lambdaMax;
	for(const ClipPlaneListItem* cpPtr=firstClipPlane;cpPtr!=0;cpPtr=cpPtr->succ)
		if(cpPtr->isEnabled())
			{
			/* Get the clipping plane's plane equation in the same coordinate system as the ray's: */
			Plane plane=cpPtr->getPlane();
			if(physical&&!cpPtr->physical)
				{
				/* Transform the clipping plane to physical space: */
				plane.transform(getNavigationTransformation());
				}
			else if(!physical&&cpPtr->physical)
				{
				/* Transform the clipping plane to navigational space: */
				plane.transform(getInverseNavigationTransformation());
				}
			
			/* Intersect the plane and the ray: */
			Scalar divisor=plane.getNormal()*ray.getDirection();
			if(divisor!=Scalar(0))
				{
				Scalar lambda=(plane.getOffset()-plane.getNormal()*ray.getOrigin())/divisor;
				
				/* Check if the ray enters or exits the clipping plane's half-space: */
				if(divisor<Scalar(0))
					{
					/* Ray exits: */
					if(lambda2>lambda)
						lambda2=lambda;
					}
				else
					{
					/* Ray enters: */
					if(lambda1<lambda)
						lambda1=lambda;
					}
				}
			}
	
	/* Adjust the ray: */
	if(lambda1<lambda2)
		{
		if(lambda1>Scalar(0))
			{
			/* Adjust the ray's origin: */
			ray.setOrigin(ray(lambda1));
			lambda2-=lambda1;
			}
		
		/* Adjust the maximum ray intercept: */
		lambdaMax=lambda2;
		}
	else
		{
		/* Invalidate the ray: */
		lambdaMax=Scalar(0);
		}
	}

}
