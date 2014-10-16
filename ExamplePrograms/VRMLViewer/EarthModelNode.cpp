/***********************************************************************
EarthModelNode - Class for high-level nodes that render a model of
Earth.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/SFInt32.h"
#include "Fields/SFColor.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "EarthModelNode.h"

/*****************************************
Methods of class EarthModelNode::DataItem:
*****************************************/

EarthModelNode::DataItem::DataItem(void)
	:displayListIdBase(0),
	 surfaceVersion(0),gridVersion(0),outerCoreVersion(0),innerCoreVersion(0)
	{
	/* Generate display lists for the Earth model components: */
	displayListIdBase=glGenLists(4);
	}

EarthModelNode::DataItem::~DataItem(void)
	{
	/* Delete the Earth model components display lists: */
	glDeleteLists(displayListIdBase,4);
	}

/***************************************
Static elements of class EarthModelNode:
***************************************/

const double EarthModelNode::earthSurfaceRadius=6378137.0;
const double EarthModelNode::earthSurfaceFlatteningFactor=1.0/298.257223563;
const double EarthModelNode::earthOuterCoreRadius=3480000.0;
const double EarthModelNode::earthInnerCoreRadius=1221000.0;

/*******************************
Methods of class EarthModelNode:
*******************************/

void EarthModelNode::renderSurface(void) const
	{
	const double pi=Math::Constants<double>::pi;
	const int baseNumStrips=18; // Number of circles of constant latitude for lowest-detail model
	const int baseNumQuads=36; // Number of meridians for lowest-detail model
	
	int numStrips=baseNumStrips*int(surfaceDetail);
	int numQuads=baseNumQuads*int(surfaceDetail);
	
	/* Set up the ellipsoid formulas: */
	double a=earthSurfaceRadius*scaleFactor;
	double e2=(2.0-flatteningFactor)*flatteningFactor;
	
	/* Draw latitude quad strips starting at the south pole: */
	float texY1=float(0)/float(numStrips);
	double lat1=(pi*double(0))/double(numStrips)-0.5*pi;
	double s1=Math::sin(lat1);
	double c1=Math::cos(lat1);
	double r1=a/Math::sqrt(1.0-e2*s1*s1);
	double xy1=r1*c1;
	double z1=r1*(1.0-e2)*s1;
	double nxy1=(e2*s1*(1.0-e2*s1*s1)+1.0)*(1.0-e2)*c1*c1;
	double nz1=-(e2*c1*c1*(1.0-e2*s1*s1)-1.0)*s1*c1;
	double nlen=Math::sqrt(nxy1*nxy1+nz1*nz1);
	nxy1/=nlen;
	nz1/=nlen;
	
	/* Draw latitude quad strips: */
	for(int i=1;i<=numStrips;++i)
		{
		float texY0=texY1;
		double lat0=lat1;
		double s0=s1;
		double c0=c1;
		double r0=r1;
		double xy0=xy1;
		double z0=z1;
		double nxy0=nxy1;
		double nz0=nz1;
		texY1=float(i)/float(numStrips);
		lat1=(pi*double(i))/double(numStrips)-0.5*pi;
		s1=Math::sin(lat1);
		c1=Math::cos(lat1);
		r1=a/Math::sqrt(1.0-e2*s1*s1);
		xy1=r1*c1;
		z1=r1*(1.0-e2)*s1;
		nxy1=(e2*s1*(1.0-e2*s1*s1)+1.0)*(1.0-e2)*c1*c1;
		nz1=-(e2*c1*c1*(1.0-e2*s1*s1)-1.0)*s1*c1;
		nlen=Math::sqrt(nxy1*nxy1+nz1*nz1);
		nxy1/=nlen;
		nz1/=nlen;
		
		glBegin(GL_QUAD_STRIP);
		for(int j=0;j<=numQuads;++j)
			{
			float texX=float(j)/float(numQuads)+0.5f;
			double lng=(2.0*pi*double(j))/double(numQuads);
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			
			glTexCoord2f(texX,texY1);
			glNormal3f(float(nxy1*cl),float(nxy1*sl),float(nz1));
			glVertex3f(float(xy1*cl),float(xy1*sl),float(z1));
			
			glTexCoord2f(texX,texY0);
			glNormal3f(float(nxy0*cl),float(nxy0*sl),float(nz0));
			glVertex3f(float(xy0*cl),float(xy0*sl),float(z0));
			}
		glEnd();
		}
	}

void EarthModelNode::renderGrid(void) const
	{
	const double pi=Math::Constants<double>::pi;
	const int baseNumStrips=18; // Number of circles of constant latitude for lowest-detail model
	const int baseNumQuads=36; // Number of meridians for lowest-detail model
	
	int numStrips=baseNumStrips*int(gridDetail);
	int numQuads=baseNumQuads*int(gridDetail);
	
	/* Set up the ellipsoid formulas: */
	double a=earthSurfaceRadius*scaleFactor;
	double e2=(2.0-flatteningFactor)*flatteningFactor;
	
	/* Draw circles of constant latitude (what are they called?): */
	for(int i=1;i<baseNumStrips;++i)
		{
		double lat=(pi*double(i))/double(baseNumStrips)-0.5*pi;
		double s=Math::sin(lat);
		double c=Math::cos(lat);
		double r=a/Math::sqrt(1.0-e2*s*s);
		double xy=r*c;
		double z=r*(1.0-e2)*s;
		
		glBegin(GL_LINE_LOOP);
		for(int j=0;j<numQuads;++j)
			{
			double lng=(2.0*pi*double(j))/double(numQuads);
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			glVertex3f(float(xy*cl),float(xy*sl),float(z));
			}
		glEnd();
		}
	
	/* Draw meridians: */
	for(int i=0;i<baseNumQuads;++i)
		{
		double lng=(2.0*pi*double(i))/double(baseNumQuads);
		double cl=Math::cos(lng);
		double sl=Math::sin(lng);
		
		glBegin(GL_LINE_STRIP);
		for(int j=0;j<=numStrips;++j)
			{
			double lat=(pi*double(j))/double(numStrips)-0.5*pi;
			double s=Math::sin(lat);
			double c=Math::cos(lat);
			double r=a/Math::sqrt(1.0-e2*s*s);
			double xy=r*c;
			double z=r*(1.0-e2)*s;
			glVertex3f(float(xy*cl),float(xy*sl),float(z));
			}
		glEnd();
		}
	}

void EarthModelNode::renderOuterCore(void) const
	{
	glDrawSphereIcosahedron(float(earthOuterCoreRadius*scaleFactor),outerCoreDetail);
	}

void EarthModelNode::renderInnerCore(void) const
	{
	glDrawSphereIcosahedron(float(earthInnerCoreRadius*scaleFactor),innerCoreDetail);
	}

EarthModelNode::EarthModelNode(VRMLParser& parser)
	:scaleFactor(1.0e-3),
	 flatteningFactor(earthSurfaceFlatteningFactor),
	 surface(true),surfaceDetail(1),
	 grid(true),gridColor(0,255,0),gridDetail(1),
	 outerCore(false),outerCoreDetail(1),
	 innerCore(false),innerCoreDetail(1)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("EarthModelNode::EarthModelNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("scaleFactor"))
			{
			/* Parse the scale factor: */
			parser.getNextToken();
			scaleFactor=atof(parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("flatteningFactor"))
			{
			/* Parse the inverse flattening factor: */
			parser.getNextToken();
			flatteningFactor=1.0/atof(parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("surface"))
			{
			/* Parse the surface flag: */
			parser.getNextToken();
			surface=SFBool::parse(parser);
			}
		else if(parser.isToken("surfaceMaterial"))
			{
			/* Parse the surface material node: */
			parser.getNextToken();
			surfaceMaterial=parser.getNextNode();
			}
		else if(parser.isToken("surfaceTexture"))
			{
			/* Parse the surface texture node: */
			parser.getNextToken();
			surfaceTexture=parser.getNextNode();
			}
		else if(parser.isToken("surfaceDetail"))
			{
			/* Parse the surface detail level: */
			parser.getNextToken();
			surfaceDetail=SFInt32::parse(parser);
			if(surfaceDetail<1)
				surfaceDetail=1;
			}
		else if(parser.isToken("grid"))
			{
			/* Parse the grid flag: */
			parser.getNextToken();
			grid=SFBool::parse(parser);
			}
		else if(parser.isToken("gridColor"))
			{
			/* Parse the grid color: */
			parser.getNextToken();
			gridColor=SFColor::parse(parser);
			}
		else if(parser.isToken("gridDetail"))
			{
			/* Parse the grid detail level: */
			parser.getNextToken();
			gridDetail=SFInt32::parse(parser);
			if(gridDetail<1)
				gridDetail=1;
			}
		else if(parser.isToken("outerCore"))
			{
			/* Parse the outer core flag: */
			parser.getNextToken();
			outerCore=SFBool::parse(parser);
			}
		else if(parser.isToken("outerCoreMaterial"))
			{
			/* Parse the outer core material node: */
			parser.getNextToken();
			outerCoreMaterial=parser.getNextNode();
			}
		else if(parser.isToken("outerCoreDetail"))
			{
			/* Parse the outer core detail level: */
			parser.getNextToken();
			outerCoreDetail=SFInt32::parse(parser);
			if(outerCoreDetail<1)
				outerCoreDetail=1;
			}
		else if(parser.isToken("innerCore"))
			{
			/* Parse the inner core flag: */
			parser.getNextToken();
			innerCore=SFBool::parse(parser);
			}
		else if(parser.isToken("innerCoreMaterial"))
			{
			/* Parse the inner core material node: */
			parser.getNextToken();
			innerCoreMaterial=parser.getNextNode();
			}
		else if(parser.isToken("innerCoreDetail"))
			{
			/* Parse the inner core detail level: */
			parser.getNextToken();
			innerCoreDetail=SFInt32::parse(parser);
			if(innerCoreDetail<1)
				innerCoreDetail=1;
			}
		else
			Misc::throwStdErr("EarthModelNode::EarthModelNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

EarthModelNode::~EarthModelNode(void)
	{
	}

void EarthModelNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the model part display lists: */
	glNewList(dataItem->displayListIdBase+0,GL_COMPILE);
	renderSurface();
	glEndList();
	
	glNewList(dataItem->displayListIdBase+1,GL_COMPILE);
	renderGrid();
	glEndList();
	
	glNewList(dataItem->displayListIdBase+2,GL_COMPILE);
	renderOuterCore();
	glEndList();
	
	glNewList(dataItem->displayListIdBase+3,GL_COMPILE);
	renderInnerCore();
	glEndList();
	}

VRMLNode::Box EarthModelNode::calcBoundingBox(void) const
	{
	/* Return a box containing the entire Earth model: */
	double e2=(2.0-flatteningFactor)*flatteningFactor;
	Box::Point::Vector size(earthSurfaceRadius*scaleFactor,earthSurfaceRadius*scaleFactor,earthSurfaceRadius*Math::sqrt(1.0-e2)*scaleFactor);
	return Box(Box::Point::origin-size,Box::Point::origin+size);
	}

void EarthModelNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	/* Do the render thing... */
	if(surface)
		{
		if(surfaceMaterial!=0)
			surfaceMaterial->setGLState(renderState);
		else
			glDisable(GL_LIGHTING);
		if(surfaceTexture!=0)
			{
			surfaceTexture->setGLState(renderState);
			if(surfaceMaterial!=0)
				{
				glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
				}
			else
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			}
		glCallList(dataItem->displayListIdBase+0);
		if(surfaceTexture!=0)
			{
			if(surfaceMaterial!=0)
				glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
			surfaceTexture->resetGLState(renderState);
			}
		if(surfaceMaterial!=0)
			surfaceMaterial->resetGLState(renderState);
		else
			glEnable(GL_LIGHTING);
		}
	}
