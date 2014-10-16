/***********************************************************************
LoadElevationGrid - Function to load an elevation grid's height values
from an external file.
Copyright (c) 2010-2012 Oliver Kreylos

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

#include <SceneGraph/Internal/LoadElevationGrid.h>

#include <utility>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>
#include <IO/SeekableFile.h>
#include <IO/ValueSource.h>
#include <Cluster/OpenFile.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>
#include <SceneGraph/ElevationGridNode.h>

namespace SceneGraph {

namespace {

/****************
Helper functions:
****************/

std::string createHeaderFileName(const std::string& bilFileName)
	{
	/* Find the BIL file name's extension: */
	std::string::const_iterator extPtr=bilFileName.end();
	for(std::string::const_iterator sPtr=bilFileName.begin();sPtr!=bilFileName.end();++sPtr)
		if(*sPtr=='.')
			extPtr=sPtr;
	
	/* Create the header file name: */
	std::string result=std::string(bilFileName.begin(),extPtr);
	result.append(".hdr");
	
	return result;
	}

void loadBILGrid(ElevationGridNode& node,Cluster::Multiplexer* multiplexer)
	{
	/* Open the header file: */
	std::string bilFileName=node.heightUrl.getValue(0);
	IO::ValueSource header(Cluster::openFile(multiplexer,createHeaderFileName(bilFileName).c_str()));
	header.skipWs();
	
	/* Parse the header file: */
	typedef IO::SeekableFile::Offset Offset;
	int size[2]={-1,-1};
	int numBits=16;
	Offset bandGapBytes=0;
	Offset bandRowBytes=0;
	Offset totalRowBytes=0;
	Misc::Endianness endianness=Misc::HostEndianness;
	Scalar cellSize[2]={Scalar(1),Scalar(1)};
	while(!header.eof())
		{
		/* Read the next token: */
		std::string token=header.readString();
		
		if(token=="LAYOUT")
			{
			std::string layout=header.readString();
			if(layout!="BIL")
				Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s does not have BIL layout",bilFileName.c_str());
			}
		else if(token=="NBANDS")
			{
			int numBands=header.readInteger();
			if(numBands!=1)
				Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s has %d bands instead of 1",bilFileName.c_str(),numBands);
			}
		else if(token=="NCOLS")
			size[0]=header.readInteger();
		else if(token=="NROWS")
			size[1]=header.readInteger();
		else if(token=="NBITS")
			{
			numBits=header.readInteger();
			if(numBits!=16&&numBits!=32)
				Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s has unsupported number of bits per sample %d",bilFileName.c_str(),numBits);
			}
		else if(token=="BANDGAPBYTES")
			bandGapBytes=Offset(header.readInteger());
		else if(token=="BANDROWBYTES")
			bandRowBytes=Offset(header.readInteger());
		else if(token=="TOTALROWBYTES")
			totalRowBytes=Offset(header.readInteger());
		else if(token=="BYTEORDER")
			{
			std::string byteOrder=header.readString();
			if(byteOrder=="LSBFIRST"||byteOrder=="I")
				endianness=Misc::LittleEndian;
			else if(byteOrder=="MSBFIRST"||byteOrder=="M")
				endianness=Misc::BigEndian;
			else
				Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s has unrecognized byte order %s",bilFileName.c_str(),byteOrder.c_str());
			}
		else if(token=="CELLSIZE")
			{
			Scalar cs=Scalar(header.readNumber());
			for(int i=0;i<2;++i)
				cellSize[i]=cs;
			}
		else if(token=="XDIM")
			cellSize[0]=Scalar(header.readNumber());
		else if(token=="YDIM")
			cellSize[1]=Scalar(header.readNumber());
		else if(token=="NODATA_VALUE")
			{
			/* Set the node's invalid removal flag and invalid height value: */
			node.removeInvalids.setValue(true);
			node.invalidHeight.setValue(header.readNumber());
			}
		}
	
	/* Check the image layout: */
	int numBytes=(numBits+7)/8;
	if(totalRowBytes!=bandRowBytes||bandRowBytes!=Offset(size[0])*Offset(numBytes))
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s has mismatching row size",bilFileName.c_str());
	if(bandGapBytes!=0)
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s has nonzero band gap",bilFileName.c_str());
	
	/* Read the image: */
	IO::SeekableFilePtr imageFile(Cluster::openSeekableFile(multiplexer,bilFileName.c_str()));
	imageFile->setEndianness(endianness);
	std::vector<Scalar> heights;
	heights.reserve(size_t(size[0])*size_t(size[1]));
	if(numBits==16)
		{
		signed short int* rowBuffer=new signed short int[size[0]];
		for(int y=size[1]-1;y>=0;--y)
			{
			/* Read the raw image row: */
			imageFile->setReadPosAbs(totalRowBytes*Offset(y));
			imageFile->read<signed short int>(rowBuffer,size[0]);
			for(int x=0;x<size[0];++x)
				heights.push_back(Scalar(rowBuffer[x]));
			}
		delete[] rowBuffer;
		}
	else if(numBits==32)
		{
		float* rowBuffer=new float[size[0]];
		for(int y=size[1]-1;y>=0;--y)
			{
			/* Read the raw image row: */
			imageFile->setReadPosAbs(totalRowBytes*Offset(y));
			imageFile->read<float>(rowBuffer,size[0]);
			for(int x=0;x<size[0];++x)
				heights.push_back(Scalar(rowBuffer[x]));
			}
		delete[] rowBuffer;
		}
	
	/* Install the height field: */
	node.xDimension.setValue(size[0]);
	node.xSpacing.setValue(cellSize[0]);
	node.zDimension.setValue(size[1]);
	node.zSpacing.setValue(cellSize[1]);
	std::swap(node.height.getValues(),heights);
	}

void loadAGRGrid(ElevationGridNode& node,Cluster::Multiplexer* multiplexer)
	{
	/* Open the grid file: */
	std::string gridFileName=node.heightUrl.getValue(0);
	IO::ValueSource grid(Cluster::openFile(multiplexer,gridFileName.c_str()));
	grid.skipWs();
	
	/* Read the grid header: */
	unsigned int gridSize[2];
	double gridOrigin[2];
	double cellSize;
	double nodata;
	if(grid.readString()!="ncols")
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s is not an ARC/INFO ASCII grid",gridFileName.c_str());
	gridSize[0]=grid.readUnsignedInteger();
	if(grid.readString()!="nrows")
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s is not an ARC/INFO ASCII grid",gridFileName.c_str());
	gridSize[1]=grid.readUnsignedInteger();
	if(grid.readString()!="xllcorner")
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s is not an ARC/INFO ASCII grid",gridFileName.c_str());
	gridOrigin[0]=grid.readNumber();
	if(grid.readString()!="yllcorner")
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s is not an ARC/INFO ASCII grid",gridFileName.c_str());
	gridOrigin[1]=grid.readNumber();
	if(grid.readString()!="cellsize")
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s is not an ARC/INFO ASCII grid",gridFileName.c_str());
	cellSize=grid.readNumber();
	if(grid.readString()!="NODATA_value")
		Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s is not an ARC/INFO ASCII grid",gridFileName.c_str());
	nodata=grid.readNumber();
	
	/* Read the grid: */
	std::vector<Scalar> heights;
	heights.reserve(size_t(gridSize[0])*size_t(gridSize[1]));
	for(size_t i=size_t(gridSize[0])*size_t(gridSize[1]);i>0;--i)
		heights.push_back(Scalar(0));
	for(unsigned int y=gridSize[1];y>0;--y)
		for(unsigned int x=0;x<gridSize[0];++x)
			heights[(y-1)*gridSize[0]+x]=grid.readNumber();
	
	/* Install the height field: */
	Point origin=node.origin.getValue();
	origin[0]=Scalar(gridOrigin[0]+cellSize*Scalar(0.5));
	if(node.heightIsY.getValue())
		origin[2]=Scalar(gridOrigin[1]+cellSize*Scalar(0.5));
	else
		origin[1]=Scalar(gridOrigin[1]+cellSize*Scalar(0.5));
	node.origin.setValue(origin);
	node.xDimension.setValue(gridSize[0]);
	node.xSpacing.setValue(cellSize);
	node.zDimension.setValue(gridSize[1]);
	node.zSpacing.setValue(cellSize);
	std::swap(node.height.getValues(),heights);
	
	/* Set the node's invalid removal flag and invalid height value: */
	node.removeInvalids.setValue(true);
	node.invalidHeight.setValue(nodata);
	}

void loadImageGrid(ElevationGridNode& node,Cluster::Multiplexer* multiplexer)
	{
	/* Open and read the image file: */
	IO::FilePtr imageFile(Cluster::openFile(multiplexer,node.heightUrl.getValue(0).c_str()));
	Images::RGBImage image=Images::readImageFile(node.heightUrl.getValue(0).c_str(),imageFile);
	
	/* Read the grid: */
	std::vector<Scalar> heights;
	heights.reserve(size_t(image.getHeight())*size_t(image.getWidth()));
	const Images::RGBImage::Color* imgPtr=image.getPixels();
	for(unsigned int y=0;y<image.getHeight();++y)
		for(unsigned int x=0;x<image.getWidth();++x,++imgPtr)
			{
			/* Convert the image pixel to greyscale: */
			unsigned int grey=((unsigned int)(*imgPtr)[0]*306U+(unsigned int)(*imgPtr)[1]*601U+(unsigned int)(*imgPtr)[2]*117U)>>10;
			
			/* Store the height value: */
			heights.push_back(Scalar(grey));
			}
	
	/* Install the height field: */
	node.xDimension.setValue(image.getWidth());
	node.zDimension.setValue(image.getHeight());
	std::swap(node.height.getValues(),heights);
	}

}

void loadElevationGrid(ElevationGridNode& node,Cluster::Multiplexer* multiplexer)
	{
	/* Determine the format of the height file: */
	if(node.heightUrlFormat.getNumValues()>=1&&node.heightUrlFormat.getValue(0)=="BIL")
		{
		/* Load an elevation grid in BIL format: */
		loadBILGrid(node,multiplexer);
		}
	else if(node.heightUrlFormat.getNumValues()>=1&&node.heightUrlFormat.getValue(0)=="ARC/INFO ASCII GRID")
		{
		/* Load an elevation grid in ARC/INFO ASCII GRID format: */
		loadAGRGrid(node,multiplexer);
		}
	else
		{
		/* Find the height file name's extension: */
		std::string::const_iterator extPtr=node.heightUrl.getValue(0).end();
		for(std::string::const_iterator sPtr=node.heightUrl.getValue(0).begin();sPtr!=node.heightUrl.getValue(0).end();++sPtr)
			if(*sPtr=='.')
				extPtr=sPtr;
		std::string extension(extPtr,node.heightUrl.getValue(0).end());
		
		if(extension==".bil")
			{
			/* Load an elevation grid in BIL format: */
			loadBILGrid(node,multiplexer);
			}
		else if(Images::canReadImageFileType(node.heightUrl.getValue(0).c_str()))
			{
			/* Load the elevation grid as an image file with height defined by luminance: */
			loadImageGrid(node,multiplexer);
			}
		else
			Misc::throwStdErr("SceneGraph::loadElevationGrid: File %s has unknown format",node.heightUrl.getValue(0).c_str());
		}
	}

}
