/***********************************************************************
ImageProjectionNode - Node to calculate texture coordinates for geometry
vertices based on potentially non-linear image and geometry
transformations.
Copyright (c) 2013 Oliver Kreylos

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

#include <SceneGraph/ImageProjectionNode.h>

#include <string.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/************************************
Methods of class ImageProjectionNode:
************************************/

ImageProjectionNode::ImageProjectionNode(void)
	:imageWidth(0),imageHeight(0),flipVertical(true)
	{
	/* Initialize the image pixel transformation: */
	for(int i=0;i<2;++i)
		for(int j=0;j<3;++j)
			imagePixelTransform.appendValue(i==j?TScalar(1):TScalar(0));
	}

const char* ImageProjectionNode::getStaticClassName(void)
	{
	return "ImageProjection";
	}

const char* ImageProjectionNode::getClassName(void) const
	{
	return "ImageProjection";
	}

void ImageProjectionNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"geometryTransform")==0)
		{
		vrmlFile.parseSFNode(geometryTransform);
		}
	else if(strcmp(fieldName,"imageTransform")==0)
		{
		vrmlFile.parseSFNode(imageTransform);
		}
	else if(strcmp(fieldName,"imagePixelTransform")==0)
		{
		vrmlFile.parseField(imagePixelTransform);
		}
	else if(strcmp(fieldName,"imageWidth")==0)
		{
		vrmlFile.parseField(imageWidth);
		}
	else if(strcmp(fieldName,"imageHeight")==0)
		{
		vrmlFile.parseField(imageHeight);
		}
	else if(strcmp(fieldName,"flipVertical")==0)
		{
		vrmlFile.parseField(flipVertical);
		}
	else
		Node::parseField(fieldName,vrmlFile);
	}

void ImageProjectionNode::update(void)
	{
	/* Convert the image transformation matrix to an affine transformation: */
	inverseImageTransform=ATransform::identity;
	int index=0;
	for(MFTScalar::ValueList::const_iterator mIt=imagePixelTransform.getValues().begin();mIt!=imagePixelTransform.getValues().end()&&index<6;++mIt,++index)
		inverseImageTransform.getMatrix()(index/3,index%3)=*mIt;
	
	/* Invert the image pixel transformation: */
	inverseImageTransform.doInvert();
	
	/* Apply the image scaling factor from integer pixel coordinates to [0,1] texture coordinates: */
	for(int j=0;j<3;++j)
		{
		inverseImageTransform.getMatrix()(0,j)/=TScalar(imageWidth.getValue());
		inverseImageTransform.getMatrix()(1,j)/=TScalar(imageHeight.getValue());
		}
	
	if(flipVertical.getValue())
		{
		/* Flip the Y texture coordinate: */
		for(int j=0;j<2;++j)
			inverseImageTransform.getMatrix()(1,j)=-inverseImageTransform.getMatrix()(1,j);
		inverseImageTransform.getMatrix()(1,2)=TScalar(1)-inverseImageTransform.getMatrix()(1,2);
		}
	}

}
