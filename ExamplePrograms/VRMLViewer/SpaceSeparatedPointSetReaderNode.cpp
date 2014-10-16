/***********************************************************************
SpaceSeparatedPointSetReaderNode - Point set reader class for space-
separated ASCII files.
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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/SFInt32.h"
#include "Fields/SFString.h"

#include "VRMLParser.h"
#include "CoordinateNode.h"
#include "ColorNode.h"
#include "ColorInterpolatorNode.h"
#include "EllipsoidNode.h"

#include "SpaceSeparatedPointSetReaderNode.h"

namespace {

/**************
Helper classes:
**************/

class Tokenizer:public Misc::File
	{
	/* Elements: */
	private:
	int nextChar; // The next character to be read from the input file
	size_t tokenBufferSize; // Allocated size of the token buffer
	char* token; // Pointer to the beginning of the token buffer
	size_t tokenLength; // Length of the token currently in the buffer
	
	/* Private methods: */
	void pushChar(char newChar)
		{
		/* Increase the size of the token buffer if the buffer is full: */
		if(tokenLength==tokenBufferSize)
			{
			/* Increase token buffer size exponentially: */
			tokenBufferSize=(tokenBufferSize*3+1)/2;
			
			/* Copy the current token into the new buffer: */
			char* newToken=new char[tokenBufferSize];
			memcpy(newToken,token,tokenLength);
			delete[] token;
			token=newToken;
			}
		
		/* Add the new character to the token: */
		token[tokenLength]=newChar;
		++tokenLength;
		}
	
	/* Constructors and destructors: */
	public:
	Tokenizer(const char* fileName) // Creates a tokenizer for the given file
		:Misc::File(fileName,"rt"),
		 nextChar(Misc::File::getc()),
		 tokenBufferSize(10),token(new char[tokenBufferSize]),
		 tokenLength(0)
		{
		/* Initialize the token: */
		token[0]='\0';
		}
	~Tokenizer(void)
		{
		delete[] token;
		}
	
	/* Methods: */
	bool eof(void) const // Returns true if tokenizer has read entire file
		{
		return nextChar==EOF;
		}
	void skipLine(void) // Skips to the beginning of the next line in the file
		{
		/* Read until the next end-of-line character: */
		while(nextChar!=EOF&&nextChar!='\n'&&nextChar!='\r')
			nextChar=Misc::File::getc();
		
		if(nextChar==EOF)
			return;
		
		/* Skip the end-of-line character: */
		int eol=nextChar;
		nextChar=Misc::File::getc();
		
		/* If the end-of-line sequence is CR-LF, skip the LF as well: */
		if(eol=='\r'&&nextChar=='\n')
			nextChar=Misc::File::getc();
		}
	const char* getNextToken(void) // Reads the next token and returns its value
		{
		/* Skip whitespace from the current file position: */
		while(isspace(nextChar)&&nextChar!=EOF)
			nextChar=Misc::File::getc();

		/* Process this token: */
		tokenLength=0;
		if(nextChar=='"') // Check for quoted tokens
			{
			/* Skip the opening quote: */
			nextChar=Misc::File::getc();
			
			/* Process characters until the closing quote: */
			while(nextChar!='"'&&nextChar!=EOF)
				{
				/* Add the character to the curren token: */
				pushChar(char(nextChar));
				
				/* Read the next character: */
				nextChar=Misc::File::getc();
				}
			
			if(nextChar=='"')
				{
				/* Skip the closing quote: */
				nextChar=Misc::File::getc();
				}
			}
		else
			{
			/* Process characters until the next separator: */
			while(!isspace(nextChar)&&nextChar!=EOF)
				{
				/* Add the character to the curren token: */
				pushChar(char(nextChar));
				
				/* Read the next character: */
				nextChar=Misc::File::getc();
				}
			}
		
		/* Add a terminating NUL to the token: */
		pushChar('\0');
		--tokenLength; // The NUL doesn't count for token length
		
		return token;
		}
	size_t getTokenLength(void) const // Returns the length of the last read token
		{
		return tokenLength;
		}
	const char* getToken(void) const // Returns the last read token
		{
		return token;
		}
	};

}

/*************************************************
Methods of class SpaceSeparatedPointSetReaderNode:
*************************************************/

SpaceSeparatedPointSetReaderNode::SpaceSeparatedPointSetReaderNode(VRMLParser& parser)
	:radians(false),colatitude(false),depth(false),radialScale(1000.0),
	 numHeaderLines(0)
	{
	for(int i=0;i<4;++i)
		columnIndices[i]=-1;
	
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("SpaceSeparatedPointSetReaderNode::SpaceSeparatedPointSetReaderNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("url"))
			{
			/* Read the external point file's URL: */
			parser.getNextToken();
			url=SFString::parse(parser);
			}
		else if(parser.isToken("ellipsoid"))
			{
			/* Read the ellipsoid node: */
			parser.getNextToken();
			ellipsoid=parser.getNextNode();
			}
		else if(parser.isToken("colorMap"))
			{
			/* Read the color map node: */
			parser.getNextToken();
			colorMap=parser.getNextNode();
			}
		else if(parser.isToken("coordColumnIndices"))
			{
			/* Read the array of coordinate column indices: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				columnIndices[i]=SFInt32::parse(parser);
			}
		else if(parser.isToken("valueColumnIndex"))
			{
			/* Read the value column index: */
			parser.getNextToken();
			columnIndices[3]=SFInt32::parse(parser);
			}
		else if(parser.isToken("radians"))
			{
			/* Read the radians flag: */
			parser.getNextToken();
			radians=SFBool::parse(parser);
			}
		else if(parser.isToken("colatitude"))
			{
			/* Read the colatitude flag: */
			parser.getNextToken();
			colatitude=SFBool::parse(parser);
			}
		else if(parser.isToken("depth"))
			{
			/* Read the depth flag: */
			parser.getNextToken();
			depth=SFBool::parse(parser);
			}
		else if(parser.isToken("radialScale"))
			{
			/* Read the radial unit scale: */
			parser.getNextToken();
			radialScale=atof(parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("numHeaderLines"))
			{
			/* Read the number of header lines to skip: */
			parser.getNextToken();
			numHeaderLines=SFInt32::parse(parser);
			}
		else
			Misc::throwStdErr("SpaceSeparatedPointSetReaderNode::SpaceSeparatedPointSetReaderNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

bool SpaceSeparatedPointSetReaderNode::hasColors(void) const
	{
	return columnIndices[3]>=0&&dynamic_cast<const ColorInterpolatorNode*>(colorMap.getPointer())!=0;
	}

void SpaceSeparatedPointSetReaderNode::readPoints(CoordinateNode* coordNode,ColorNode* colorNode) const
	{
	CoordinateNode::PointList& points=coordNode->getPoints();
	ColorNode::ColorList* colors=colorNode!=0?&colorNode->getColors():0;
	const EllipsoidNode* e=dynamic_cast<const EllipsoidNode*>(ellipsoid.getPointer());
	const ColorInterpolatorNode* c=dynamic_cast<const ColorInterpolatorNode*>(colorMap.getPointer());
	
	/* Determine the number of relevant columns: */
	int maxColumn=columnIndices[0];
	for(int i=1;i<4;++i)
		if(maxColumn<columnIndices[i])
			maxColumn=columnIndices[i];
	
	/* Create the map from column index to value index: */
	int* columnMap=new int[maxColumn+1];
	for(int i=0;i<=maxColumn;++i)
		columnMap[i]=-1;
	for(int i=0;i<4;++i)
		if(columnIndices[i]>=0)
			columnMap[columnIndices[i]]=i;
	
	/* Open the input file: */
	Tokenizer pointFile(url.c_str());
	
	/* Skip the header lines: */
	for(int i=0;i<numHeaderLines;++i)
		pointFile.skipLine();
	
	/* Calculate the effective radial scale: */
	double radScale=radialScale;
	if(depth)
		radScale=-radScale;
	
	/* Read all lines in the point file: */
	while(!pointFile.eof())
		{
		/* Extract relevant columns from the current line: */
		double values[4];
		bool pointValid=true;
		for(int i=0;i<=maxColumn;++i)
			{
			const char* token=pointFile.getNextToken();
			if(columnMap[i]>=0)
				{
				double v=atof(token);
				values[columnMap[i]]=v;
				pointValid=pointValid&&!isnan(v);
				}
			}
		
		/* Skip to the end of the current line: */
		pointFile.skipLine();
		
		if(pointValid)
			{
			if(e!=0)
				{
				/* Convert the point to Cartesian coordinates and store it: */
				if(!radians)
					for(int i=0;i<2;++i)
						values[i]=Math::rad(values[i]);
				if(colatitude)
					values[0]=0.5*Math::Constants<double>::pi-values[0];
				values[2]*=radScale;
				points.push_back(e->sphericalToCartesian(values));
				}
			else
				{
				/* Store the point in the given coordinates: */
				points.push_back(CoordinateNode::Point(values));
				}
			
			if(columnIndices[3]>=0&&c!=0)
				{
				/* Convert the point value to a color and store it: */
				colors->push_back(c->interpolate(float(values[3])));
				}
			}
		}
	
	std::cout<<"Read "<<points.size()<<" points"<<std::endl;
	}
