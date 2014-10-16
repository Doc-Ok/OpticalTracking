/***********************************************************************
SceneGraphSupport - Helper functions to simplify adding scene graphs to
Vrui applications.
Copyright (c) 2013 Oliver Kreylos

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

#include <Vrui/SceneGraphSupport.h>

#include <SceneGraph/GLRenderState.h>
#include <SceneGraph/GraphNode.h>
#include <Vrui/Vrui.h>
#include <Vrui/Viewer.h>
#include <Vrui/DisplayState.h>

namespace Vrui {

SceneGraph::GLRenderState* createRenderState(bool navigational,GLContextData& contextData)
	{
	/* Get the physical-space modelview matrix to transform head position and up direction from physical to eye space: */
	const NavTransform& mvp=getDisplayState(contextData).modelviewPhysical;
	
	/* Get the initial transformation: */
	const NavTransform& initial=navigational?getDisplayState(contextData).modelviewNavigational:mvp;
	
	/* Return the render state object: */
	return new SceneGraph::GLRenderState(contextData,initial,mvp.transform(getMainViewer()->getHeadPosition()),mvp.transform(getUpDirection()));
	}

SceneGraph::GLRenderState* createRenderState(const NavTransform& transform,bool navigational,GLContextData& contextData)
	{
	/* Get the physical-space modelview matrix to transform head position and up direction from physical to eye space: */
	const NavTransform& mvp=getDisplayState(contextData).modelviewPhysical;
	
	/* Get the initial transformation: */
	NavTransform initial=navigational?getDisplayState(contextData).modelviewNavigational:mvp;
	initial*=transform;
	initial.renormalize();
	
	/* Return the render state object: */
	return new SceneGraph::GLRenderState(contextData,initial,mvp.transform(getMainViewer()->getHeadPosition()),mvp.transform(getUpDirection()));
	}

void renderSceneGraph(const SceneGraph::GraphNode* root,bool navigational,GLContextData& contextData)
	{
	/* Save the current modelview matrix: */
	glPushMatrix();
	
	/* Get the physical-space modelview matrix to transform head position and up direction from physical to eye space: */
	const NavTransform& mvp=getDisplayState(contextData).modelviewPhysical;
	
	/* Get the initial transformation: */
	const NavTransform& initial=navigational?getDisplayState(contextData).modelviewNavigational:mvp;
	
	/* Create the render state object: */
	SceneGraph::GLRenderState renderState(contextData,initial,mvp.transform(getMainViewer()->getHeadPosition()),mvp.transform(getUpDirection()));
	
	/* Render the scene graph: */
	root->glRenderAction(renderState);
	
	/* Restore the original modelview matrix: */
	glPopMatrix();
	}

void renderSceneGraph(const SceneGraph::GraphNode* root,const NavTransform& transform,bool navigational,GLContextData& contextData)
	{
	/* Save the current modelview matrix: */
	glPushMatrix();
	
	/* Get the physical-space modelview matrix to transform head position and up direction from physical to eye space: */
	const NavTransform& mvp=getDisplayState(contextData).modelviewPhysical;
	
	/* Get the initial transformation: */
	NavTransform initial=navigational?getDisplayState(contextData).modelviewNavigational:mvp;
	initial*=transform;
	initial.renormalize();
	
	/* Create the render state object: */
	SceneGraph::GLRenderState renderState(contextData,initial,mvp.transform(getMainViewer()->getHeadPosition()),mvp.transform(getUpDirection()));
	
	/* Render the scene graph: */
	root->glRenderAction(renderState);
	
	/* Restore the original modelview matrix: */
	glPopMatrix();
	}

}
