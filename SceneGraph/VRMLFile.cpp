/***********************************************************************
VRMLFile - Class to represent a VRML 2.0 file and state required to
parse its contents.
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

#include <SceneGraph/VRMLFile.h>

#include <stdlib.h>
#include <Misc/StringPrintf.h>
#include <Misc/ThrowStdErr.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/NodeCreator.h>
#include <SceneGraph/GraphNode.h>

namespace SceneGraph {

namespace {

/*************************************************************************
Helper function to parse route statements (will move into VRMLFile class):
*************************************************************************/

void parseRoute(VRMLFile& vrmlFile)
	{
	/* Read the event source name: */
	const char* source=vrmlFile.readNextToken();
	
	/* Split the event source into node name and field name: */
	const char* periodPtr=0;
	for(const char* sPtr=source;*sPtr!='\0';++sPtr)
		if(*sPtr=='.')
			{
			if(periodPtr!=0)
				throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("multiple periods in event source %s",source));
			periodPtr=sPtr;
			}
	if(periodPtr==0)
		throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("missing period in event source %s",source));
	
	/* Retrieve the event source: */
	EventOut* eventOut=0;
	try
		{
		std::string sourceNode(source,periodPtr);
		eventOut=vrmlFile.useNode(sourceNode.c_str())->getEventOut(periodPtr+1);
		}
	catch(Node::FieldError err)
		{
		throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("unknown field \"%s\" in event source",periodPtr+1));
		}
	
	/* Check the TO keyword: */
	vrmlFile.readNextToken();
	if(!vrmlFile.isToken("TO"))
		throw VRMLFile::ParseError(vrmlFile,"missing TO keyword in route definition");
		
	/* Read the event sink name: */
	const char* sink=vrmlFile.readNextToken();
	
	/* Split the event sink into node name and field name: */
	periodPtr=0;
	for(const char* sPtr=sink;*sPtr!='\0';++sPtr)
		if(*sPtr=='.')
			{
			if(periodPtr!=0)
				throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("multiple periods in event sink %s",sink));
			periodPtr=sPtr;
			}
	if(periodPtr==0)
		throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("missing period in event sink %s",sink));
	
	/* Retrieve the event sink: */
	EventIn* eventIn=0;
	try
		{
		std::string sinkNode(sink,periodPtr);
		eventIn=vrmlFile.useNode(sinkNode.c_str())->getEventIn(periodPtr+1);
		}
	catch(Node::FieldError err)
		{
		throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("unknown field \"%s\" in event sink",periodPtr+1));
		}
	
	/* Create a route: */
	Route* route=0;
	try
		{
		route=eventOut->connectTo(eventIn);
		}
	catch(Route::TypeMismatchError err)
		{
		throw VRMLFile::ParseError(vrmlFile,"mismatching field types in route definition");
		}
	
	/* For now, just delete the route again: */
	delete route;
	}

/********************************************************************
Helper functions to parse floating-point values and component arrays:
********************************************************************/

template <class ScalarParam>
inline
ScalarParam
parseFloatingPoint(
	VRMLFile& vrmlFile)
	{
	/* Read the next token: */
	const char* token=vrmlFile.readNextToken();
	
	/* Convert it to floating-point: */
	char* endPtr=0;
	ScalarParam result=ScalarParam(strtod(token,&endPtr));
	
	/* Ensure that the entire token was converted: */
	if(endPtr!=token+vrmlFile.getTokenSize())
		throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("%s is not a valid floating-point value",token));
	
	return result;
	}

template <class ComponentArrayParam>
inline
void
parseComponentArray(
	ComponentArrayParam& value,
	VRMLFile& vrmlFile)
	{
	/* Parse the components of the given component array: */
	for(int i=0;i<ComponentArrayParam::dimension;++i)
		{
		/* Read the next token: */
		const char* token=vrmlFile.readNextToken();
		
		/* Convert it to floating-point: */
		char* endPtr=0;
		value[i]=typename ComponentArrayParam::Scalar(strtod(token,&endPtr));
		
		/* Ensure that the entire token was converted: */
		if(endPtr!=token+vrmlFile.getTokenSize())
			throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("%s is not a valid floating-point value",token));
		}
	}

/***********************************************************
Templatized helper class to parse values from token sources:
***********************************************************/

template <class ValueParam>
class ValueParser // Generic class to parse values from token sources
	{
	};

template <>
class ValueParser<bool>
	{
	/* Methods: */
	public:
	static bool parseValue(VRMLFile& vrmlFile)
		{
		/* Read the next token: */
		vrmlFile.readNextToken();
		
		/* Parse the token's value: */
		if(vrmlFile.isToken("TRUE"))
			return true;
		else if(vrmlFile.isToken("FALSE"))
			return false;
		else
			throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("%s is not a valid boolean value",vrmlFile.getToken()));
		}
	};

template <>
class ValueParser<std::string>
	{
	/* Methods: */
	public:
	static std::string parseValue(VRMLFile& vrmlFile)
		{
		/* Return the next token: */
		return std::string(vrmlFile.readNextToken());
		}
	};

template <>
class ValueParser<int>
	{
	/* Methods: */
	public:
	static int parseValue(VRMLFile& vrmlFile)
		{
		/* Read the next token: */
		const char* token=vrmlFile.readNextToken();
		
		/* Convert it to integer: */
		char* endPtr=0;
		int result=int(strtol(token,&endPtr,10));
		
		/* Ensure that the entire token was converted: */
		if(endPtr!=token+vrmlFile.getTokenSize())
			throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("%s is not a valid integer value",token));
		
		return result;
		}
	};

template <>
class ValueParser<Scalar>
	{
	/* Methods: */
	public:
	static Scalar parseValue(VRMLFile& vrmlFile)
		{
		return parseFloatingPoint<Scalar>(vrmlFile);
		}
	};

template <>
class ValueParser<double>
	{
	/* Methods: */
	public:
	static double parseValue(VRMLFile& vrmlFile)
		{
		return parseFloatingPoint<double>(vrmlFile);
		}
	};

template <>
class ValueParser<Size>
	{
	/* Methods: */
	public:
	static Size parseValue(VRMLFile& vrmlFile)
		{
		Size result;
		parseComponentArray(result,vrmlFile);
		return result;
		}
	};

template <class ScalarParam>
class ValueParser<Geometry::Point<ScalarParam,3> >
	{
	/* Methods: */
	public:
	static Geometry::Point<ScalarParam,3> parseValue(VRMLFile& vrmlFile)
		{
		Geometry::Point<ScalarParam,3> result;
		parseComponentArray(result,vrmlFile);
		return result;
		}
	};

template <class ScalarParam>
class ValueParser<Geometry::Vector<ScalarParam,3> >
	{
	/* Methods: */
	public:
	static Geometry::Vector<ScalarParam,3> parseValue(VRMLFile& vrmlFile)
		{
		Geometry::Vector<ScalarParam,3> result;
		parseComponentArray(result,vrmlFile);
		return result;
		}
	};

template <>
class ValueParser<Rotation>
	{
	/* Methods: */
	public:
	static Rotation parseValue(VRMLFile& vrmlFile)
		{
		/* Parse the rotation axis: */
		Rotation::Vector axis;
		parseComponentArray(axis,vrmlFile);
		
		/* Parse the rotation angle: */
		Rotation::Scalar angle=parseFloatingPoint<Rotation::Scalar>(vrmlFile);
		
		/* Return the rotation: */
		return Rotation::rotateAxis(axis,angle);
		}
	};

template <class ScalarParam,int numComponentsParam>
class ValueParser<GLColor<ScalarParam,numComponentsParam> >
	{
	/* Methods: */
	public:
	static GLColor<ScalarParam,numComponentsParam> parseValue(VRMLFile& vrmlFile)
		{
		/* Parse the color's components: */
		GLColor<ScalarParam,numComponentsParam> result;
		for(int i=0;i<numComponentsParam;++i)
			result[i]=parseFloatingPoint<ScalarParam>(vrmlFile);
		
		return result;
		}
	};

template <>
class ValueParser<TexCoord>
	{
	/* Methods: */
	public:
	static TexCoord parseValue(VRMLFile& vrmlFile)
		{
		TexCoord result;
		parseComponentArray(result,vrmlFile);
		return result;
		}
	};

template <>
class ValueParser<NodePointer>
	{
	/* Methods: */
	public:
	static NodePointer parseValue(VRMLFile& vrmlFile)
		{
		NodePointer result;
		
		/* Read the node type name: */
		vrmlFile.readNextToken();
		if(vrmlFile.isToken("ROUTE"))
			{
			/* Parse a route statement: */
			parseRoute(vrmlFile);
			}
		else if(vrmlFile.isToken("USE"))
			{
			/* Retrieve a named node from the VRML file: */
			result=vrmlFile.useNode(vrmlFile.readNextToken());
			}
		else
			{
			/* Check for the optional DEF keyword: */
			std::string defName;
			if(vrmlFile.isToken("DEF"))
				{
				/* Read the new node name: */
				defName=vrmlFile.readNextToken();
				
				/* Read the node type name: */
				vrmlFile.readNextToken();
				}
			
			if(!vrmlFile.isToken("NULL"))
				{
				/* Create the result node: */
				if((result=vrmlFile.createNode(vrmlFile.getToken()))==0)
					throw VRMLFile::ParseError(vrmlFile,Misc::stringPrintf("Unknown node type %s",vrmlFile.getToken()));
				
				/* Check for and skip the opening brace: */
				vrmlFile.readNextToken();
				if(!vrmlFile.isToken("{"))
					throw VRMLFile::ParseError(vrmlFile,"Missing opening brace in node definition");
				
				while(!vrmlFile.eof()&&vrmlFile.peekc()!='}')
					{
					vrmlFile.readNextToken();
					
					if(vrmlFile.isToken("ROUTE"))
						{
						/* Parse a route statement: */
						parseRoute(vrmlFile);
						}
					else
						{
						/* Parse a field value: */
						result->parseField(vrmlFile.getToken(),vrmlFile);
						}
					}
				
				/* Check for and skip the closing brace: */
				if(vrmlFile.eof())
					throw VRMLFile::ParseError(vrmlFile,"Missing closing brace in node definition");
				vrmlFile.readNextToken();
				
				/* Finalize the node: */
				result->update();
				}
			
			if(!defName.empty())
				{
				/* Store the named node in the VRML file: */
				vrmlFile.defineNode(defName.c_str(),result);
				}
			}
		
		return result;
		}
	};

/***********************************************************
Templatized helper class to parse fields from token sources:
***********************************************************/

template <class FieldParam>
class FieldParser
	{
	};

/**************************************
Specialization for single-value fields:
**************************************/

template <class ValueParam>
class FieldParser<SF<ValueParam> >
	{
	/* Methods: */
	public:
	static void parseField(SF<ValueParam>& field,VRMLFile& vrmlFile)
		{
		/* Just read the field's value: */
		field.setValue(ValueParser<ValueParam>::parseValue(vrmlFile));
		}
	};

/*************************************
Specialization for multi-value fields:
*************************************/

template <class ValueParam>
class FieldParser<MF<ValueParam> >
	{
	/* Methods: */
	public:
	static void parseField(MF<ValueParam>& field,VRMLFile& vrmlFile)
		{
		/* Clear the field: */
		field.clearValues();
		
		/* Check for opening bracket: */
		if(vrmlFile.peekc()=='[')
			{
			/* Skip the opening bracket: */
			vrmlFile.readNextToken();
			
			/* Read a list of values: */
			while(!vrmlFile.eof()&&vrmlFile.peekc()!=']')
				{
				/* Read a single value: */
				field.appendValue(ValueParser<ValueParam>::parseValue(vrmlFile));
				}
			
			/* Skip the closing bracket: */
			if(vrmlFile.eof())
				throw VRMLFile::ParseError(vrmlFile,"Missing closing bracket in multi-valued field");
			vrmlFile.readNextToken();
			}
		else
			{
			/* Read a single value: */
			field.appendValue(ValueParser<ValueParam>::parseValue(vrmlFile));
			}
		}
	};

}

/*************************************
Methods of class VRMLFile::ParseError:
*************************************/

VRMLFile::ParseError::ParseError(const VRMLFile& vrmlFile,std::string error)
	:std::runtime_error(Misc::stringPrintf("%s, line %u: %s",vrmlFile.sourceUrl.c_str(),(unsigned int)vrmlFile.currentLine,error.c_str()))
	{
	}

/*************************
Methods of class VRMLFile:
*************************/

VRMLFile::VRMLFile(std::string sSourceUrl,IO::FilePtr sSource,NodeCreator& sNodeCreator,Cluster::Multiplexer* sMultiplexer)
	:IO::TokenSource(sSource),
	 sourceUrl(sSourceUrl),
	 nodeCreator(sNodeCreator),
	 multiplexer(sMultiplexer),
	 nodeMap(101),
	 currentLine(1)
	{
	/* Initialize the token source: */
	setWhitespace(',',true); // Comma is treated as whitespace
	setPunctuation("#[]{}\n"); // Newline is treated as punctuation to count lines
	setQuotes("\"\'");
	
	/* Check the VRML file header: */
	IO::TokenSource::readNextToken();
	if(!isToken("#"))
		Misc::throwStdErr("VRMLFile: %s is not a valid VRML 2.0 file",sourceUrl.c_str());
	IO::TokenSource::readNextToken();
	if(!isToken("VRML"))
		Misc::throwStdErr("VRMLFile: %s is not a valid VRML 2.0 file",sourceUrl.c_str());
	IO::TokenSource::readNextToken();
	if(!isToken("V2.0"))
		Misc::throwStdErr("VRMLFile: %s is not a valid VRML 2.0 file",sourceUrl.c_str());
	IO::TokenSource::readNextToken();
	if(!isToken("utf8"))
		Misc::throwStdErr("VRMLFile: %s is not a valid VRML 2.0 file",sourceUrl.c_str());
	
	/* Extract the URL prefix: */
	urlPrefix=sourceUrl.begin();
	for(std::string::const_iterator suIt=sourceUrl.begin();suIt!=sourceUrl.end();++suIt)
		if(*suIt=='/')
			urlPrefix=suIt+1;
	}

void VRMLFile::parse(GroupNodePointer root)
	{
	/* Read nodes until end of file: */
	while(!eof())
		{
		SF<GraphNodePointer> node;
		parseSFNode(node);
		if(node.getValue()!=0)
			root->children.appendValue(node.getValue());
		}
	}

template <class ValueParam>
ValueParam
VRMLFile::parseValue(
	void)
	{
	/* Call on the templatized value parser helper class: */
	return ValueParser<ValueParam>::parseValue(*this);
	}

template <class FieldParam>
void
VRMLFile::parseField(
	FieldParam& field)
	{
	/* Call on the templatized field parser helper class: */
	FieldParser<FieldParam>::parseField(field,*this);
	}

NodePointer VRMLFile::createNode(const char* nodeType)
	{
	return nodeCreator.createNode(nodeType);
	}

void VRMLFile::defineNode(const char* nodeName,NodePointer node)
	{
	nodeMap.setEntry(NodeMap::Entry(nodeName,node));
	}

NodePointer VRMLFile::useNode(const char* nodeName)
	{
	NodeMap::Iterator nIt=nodeMap.findEntry(nodeName);
	if(nIt.isFinished())
		throw ParseError(*this,Misc::stringPrintf("Undefined node name %s",nodeName));
	
	return nIt->getDest();
	}

std::string VRMLFile::getFullUrl(std::string localUrl) const
	{
	/* Check if the local URL is already fully qualified: */
	if(localUrl[0]!='/')
		{
		/* Prepend the base URL prefix: */
		std::string result(sourceUrl.begin(),urlPrefix);
		result+=localUrl;
		return result;
		}
	else
		return localUrl;
	}

/********************************************************************
Force instantiation of value parser methods for standard value types:
********************************************************************/

template NodePointer VRMLFile::parseValue<NodePointer>();

/********************************************************************
Force instantiation of field parser methods for standard field types:
********************************************************************/

template void VRMLFile::parseField(SFBool&);
template void VRMLFile::parseField(SFString&);
template void VRMLFile::parseField(SFInt&);
template void VRMLFile::parseField(SFFloat&);
template void VRMLFile::parseField(SFSize&);
template void VRMLFile::parseField(SFPoint&);
template void VRMLFile::parseField(SFVector&);
template void VRMLFile::parseField(SFRotation&);
template void VRMLFile::parseField(SFColor&);
template void VRMLFile::parseField(SFTexCoord&);
template void VRMLFile::parseField(SFNode&);
template void VRMLFile::parseField(MFBool&);
template void VRMLFile::parseField(MFString&);
template void VRMLFile::parseField(MFInt&);
template void VRMLFile::parseField(MFFloat&);
template void VRMLFile::parseField(MFSize&);
template void VRMLFile::parseField(MFPoint&);
template void VRMLFile::parseField(MFVector&);
template void VRMLFile::parseField(MFRotation&);
template void VRMLFile::parseField(MFColor&);
template void VRMLFile::parseField(MFTexCoord&);
template void VRMLFile::parseField(MFNode&);

template void VRMLFile::parseField(SF<double>&);
template void VRMLFile::parseField(MF<double>&);
template void VRMLFile::parseField(SF<Geometry::Point<double,3> >&);
template void VRMLFile::parseField(MF<Geometry::Point<double,3> >&);
template void VRMLFile::parseField(SF<Geometry::Vector<double,3> >&);
template void VRMLFile::parseField(MF<Geometry::Vector<double,3> >&);

}
