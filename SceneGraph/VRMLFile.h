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

#ifndef SCENEGRAPH_VRMLFILE_INCLUDED
#define SCENEGRAPH_VRMLFILE_INCLUDED

#include <string>
#include <stdexcept>
#include <Misc/StringHashFunctions.h>
#include <Misc/HashTable.h>
#include <IO/File.h>
#include <IO/TokenSource.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/GroupNode.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}
namespace SceneGraph {
class NodeCreator;
}

namespace SceneGraph {

class VRMLFile:public IO::TokenSource
	{
	/* Embedded classes: */
	private:
	typedef Misc::HashTable<std::string,NodePointer> NodeMap; // Hash table type to store named nodes
	
	public:
	class ParseError:public std::runtime_error // Exception class to signal errors while parsing a VRML file
		{
		/* Constructors and destructors: */
		public:
		ParseError(const VRMLFile& vrmlFile,std::string error);
		};
	
	friend class ParseError;
	
	/* Elements: */
	private:
	std::string sourceUrl; // Full URL of the VRML file
	std::string::const_iterator urlPrefix; // Prefix for relative URLs
	NodeCreator& nodeCreator; // Reference to the node creator
	Cluster::Multiplexer* multiplexer; // Pointer to a multicast pipe multiplexer when parsing VRML files in a cluster environment
	NodeMap nodeMap; // Map of named nodes
	size_t currentLine; // Number of currently processed line
	
	/* Private methods: */
	void skipExtendedWhitespace(void) // Skips over "extended" whitespace, i.e., line comments and newlines
		{
		while(true)
			{
			if(IO::TokenSource::peekc()=='\n')
				{
				/* Increase the line number: */
				++currentLine;
				
				/* Skip the newline: */
				IO::TokenSource::readNextToken();
				}
			else if(IO::TokenSource::peekc()=='#')
				{
				/* Skip the rest of the line: */
				IO::TokenSource::skipLine();
				
				/* Increase the line number: */
				++currentLine;
				
				/* Skip whitespace at the beginning of the next line: */
				IO::TokenSource::skipWs();
				}
			else
				break;
			}
		}
	
	/* Constructors and destructors: */
	public:
	VRMLFile(std::string sSourceUrl,IO::FilePtr sSource,NodeCreator& sNodeCreator,Cluster::Multiplexer* sMultiplexer =0); // Creates a VRML parser for the given character source and node creator
	
	/* Overloaded methods from IO::TokenSource: */
	bool eof(void)
		{
		skipExtendedWhitespace();
		return IO::TokenSource::eof();
		}
	int peekc(void)
		{
		skipExtendedWhitespace();
		return IO::TokenSource::peekc();
		}
	const char* readNextToken(void) // Reads the next token while skiping line comments
		{
		skipExtendedWhitespace();
		return IO::TokenSource::readNextToken();
		}
	
	/* Main method: */
	void parse(GroupNodePointer root); // Adds top-level nodes from the VRML file to the given group node
	
	/* Methods called during parsing: */
	template <class ValueParam>
	ValueParam parseValue(void); // Parses a value of the given type from the VRML file
	template <class FieldParam>
	void parseField(FieldParam& field); // Sets the given field's value by reading from the VRML file
	template <class NodePointerParam>
	void parseSFNode(SF<NodePointerParam>& field) // Parses a single-valued node field
		{
		/* Read the base-class node: */
		NodePointer node=parseValue<NodePointer>();
		
		/* Check if the node type matches: */
		if(node!=0&&dynamic_cast<typename NodePointerParam::Target*>(node.getPointer())==0)
			throw ParseError(*this,"Mismatching node type");
		
		/* Set the field's node pointer: */
		field.setValue(node);
		}
	template <class NodePointerParam>
	void parseMFNode(MF<NodePointerParam>& field) // Parses a multi-valued node field
		{
		/* Clear the field: */
		field.clearValues();
		
		/* Check for opening bracket: */
		if(peekc()=='[')
			{
			/* Skip the opening bracket: */
			readNextToken();
			
			/* Read a list of values: */
			while(!eof()&&peekc()!=']')
				{
				/* Read a base-class node: */
				NodePointer node=parseValue<NodePointer>();
				
				/* Check if the node type matches: */
				if(node!=0&&dynamic_cast<typename NodePointerParam::Target*>(node.getPointer())==0)
					throw ParseError(*this,"Mismatching node type");
				
				/* Set the field's node pointer: */
				field.appendValue(node);
				}
			
			/* Skip the closing bracket: */
			if(eof())
				throw ParseError(*this,"Missing closing bracket in multi-valued field");
			readNextToken();
			}
		else
			{
			/* Read a base-class node: */
			NodePointer node=parseValue<NodePointer>();
			
			/* Check if the node type matches: */
			if(node!=0&&dynamic_cast<typename NodePointerParam::Target*>(node.getPointer())==0)
				throw ParseError(*this,"Mismatching node type");
			
			/* Set the field's node pointer: */
			field.appendValue(node);
			}
		}
	NodeCreator& getNodeCreator(void) // Returns the VRML file's node creator
		{
		return nodeCreator;
		}
	Cluster::Multiplexer* getMultiplexer(void) const // Returns the optional multicast pipe multiplexer
		{
		return multiplexer;
		}
	NodePointer createNode(const char* nodeType); // Creates a new node of the given type
	void defineNode(const char* nodeName,NodePointer node); // Stores the given node under the given name, for future instantiation
	NodePointer useNode(const char* nodeName); // Retrieves the node most recently stored under the given name
	std::string getFullUrl(std::string localUrl) const; // Converts a file-relative URL into a fully-qualified URL
	};

}

#endif
