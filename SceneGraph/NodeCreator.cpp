/***********************************************************************
NodeCreator - Class to create node objects based on a node type name.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <SceneGraph/NodeCreator.h>

#include <SceneGraph/NodeFactory.h>
#include <SceneGraph/GroupNode.h>
#include <SceneGraph/TransformNode.h>
#include <SceneGraph/BillboardNode.h>
#include <SceneGraph/LODNode.h>
#include <SceneGraph/ReferenceEllipsoidNode.h>
#include <SceneGraph/GeodeticToCartesianTransformNode.h>
#include <SceneGraph/InlineNode.h>
#include <SceneGraph/MaterialNode.h>
#include <SceneGraph/ImageTextureNode.h>
#include <SceneGraph/AppearanceNode.h>
#include <SceneGraph/AffinePointTransformNode.h>
#include <SceneGraph/GeodeticToCartesianPointTransformNode.h>
#include <SceneGraph/UTMPointTransformNode.h>
#include <SceneGraph/ImageProjectionNode.h>
#include <SceneGraph/BoxNode.h>
#include <SceneGraph/SphereNode.h>
#include <SceneGraph/ConeNode.h>
#include <SceneGraph/CylinderNode.h>
#include <SceneGraph/TextureCoordinateNode.h>
#include <SceneGraph/ColorNode.h>
#include <SceneGraph/NormalNode.h>
#include <SceneGraph/CoordinateNode.h>
#include <SceneGraph/ColorMapNode.h>
#include <SceneGraph/PointSetNode.h>
#include <SceneGraph/IndexedLineSetNode.h>
#include <SceneGraph/CurveSetNode.h>
#include <SceneGraph/ElevationGridNode.h>
#include <SceneGraph/QuadSetNode.h>
#include <SceneGraph/IndexedFaceSetNode.h>
#include <SceneGraph/ShapeNode.h>
#include <SceneGraph/FontStyleNode.h>
#include <SceneGraph/TextNode.h>
#include <SceneGraph/LabelSetNode.h>
#include <SceneGraph/TSurfFileNode.h>
#include <SceneGraph/ArcInfoExportFileNode.h>
#include <SceneGraph/ESRIShapeFileNode.h>
#include <SceneGraph/Doom3DataContextNode.h>
#include <SceneGraph/Doom3ModelNode.h>
#include <SceneGraph/Doom3MD5MeshNode.h>

namespace SceneGraph {

/****************************
Methods of class NodeCreator:
****************************/

NodeCreator::NodeCreator(void)
	:nodeFactoryMap(31)
	{
	/* Register the standard node types: */
	registerNodeType(new GenericNodeFactory<GroupNode>());
	registerNodeType(new GenericNodeFactory<TransformNode>());
	registerNodeType(new GenericNodeFactory<BillboardNode>());
	registerNodeType(new GenericNodeFactory<LODNode>());
	registerNodeType(new GenericNodeFactory<ReferenceEllipsoidNode>());
	registerNodeType(new GenericNodeFactory<GeodeticToCartesianTransformNode>());
	registerNodeType(new GenericNodeFactory<InlineNode>());
	registerNodeType(new GenericNodeFactory<MaterialNode>());
	registerNodeType(new GenericNodeFactory<ImageTextureNode>());
	registerNodeType(new GenericNodeFactory<AppearanceNode>());
	registerNodeType(new GenericNodeFactory<AffinePointTransformNode>());
	registerNodeType(new GenericNodeFactory<GeodeticToCartesianPointTransformNode>());
	registerNodeType(new GenericNodeFactory<UTMPointTransformNode>());
	registerNodeType(new GenericNodeFactory<ImageProjectionNode>());
	registerNodeType(new GenericNodeFactory<BoxNode>());
	registerNodeType(new GenericNodeFactory<SphereNode>());
	registerNodeType(new GenericNodeFactory<ConeNode>());
	registerNodeType(new GenericNodeFactory<CylinderNode>());
	registerNodeType(new GenericNodeFactory<TextureCoordinateNode>());
	registerNodeType(new GenericNodeFactory<ColorNode>());
	registerNodeType(new GenericNodeFactory<NormalNode>());
	registerNodeType(new GenericNodeFactory<CoordinateNode>());
	registerNodeType(new GenericNodeFactory<ColorMapNode>());
	registerNodeType(new GenericNodeFactory<PointSetNode>());
	registerNodeType(new GenericNodeFactory<IndexedLineSetNode>());
	registerNodeType(new GenericNodeFactory<CurveSetNode>());
	registerNodeType(new GenericNodeFactory<ElevationGridNode>());
	registerNodeType(new GenericNodeFactory<QuadSetNode>());
	registerNodeType(new GenericNodeFactory<IndexedFaceSetNode>());
	registerNodeType(new GenericNodeFactory<ShapeNode>());
	registerNodeType(new GenericNodeFactory<FontStyleNode>());
	registerNodeType(new GenericNodeFactory<TextNode>());
	registerNodeType(new GenericNodeFactory<LabelSetNode>());
	registerNodeType(new GenericNodeFactory<TSurfFileNode>());
	registerNodeType(new GenericNodeFactory<ArcInfoExportFileNode>());
	registerNodeType(new GenericNodeFactory<ESRIShapeFileNode>());
	registerNodeType(new GenericNodeFactory<Doom3DataContextNode>());
	registerNodeType(new GenericNodeFactory<Doom3ModelNode>());
	registerNodeType(new GenericNodeFactory<Doom3MD5MeshNode>());
	}

NodeCreator::~NodeCreator(void)
	{
	/* Destroy all node factories: */
	for(NodeFactoryMap::Iterator nfmIt=nodeFactoryMap.begin();!nfmIt.isFinished();++nfmIt)
		delete nfmIt->getDest();
	}

void NodeCreator::registerNodeType(NodeFactory* nodeFactory)
	{
	nodeFactoryMap.setEntry(NodeFactoryMap::Entry(nodeFactory->getClassName(),nodeFactory));
	}

Node* NodeCreator::createNode(const char* nodeType)
	{
	NodeFactoryMap::Iterator nfIt=nodeFactoryMap.findEntry(nodeType);
	if(nfIt.isFinished())
		return 0;
	else
		return nfIt->getDest()->createNode();
	}

}
