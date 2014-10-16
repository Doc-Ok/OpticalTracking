/***********************************************************************
Doom3ValueSource - Derived Misc::ValueSource class to count lines and
skip comments in Doom3 files.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef SCENEGRAPH_INTERNAL_DOOM3VALUESOURCE_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3VALUESOURCE_INCLUDED

#include <string>
#include <Misc/StringPrintf.h>
#include <IO/ValueSource.h>

namespace SceneGraph {

class Doom3ValueSource:public IO::ValueSource
	{
	/* Embedded classes: */
	private:
	typedef IO::ValueSource Base; // Base class type
	
	/* Elements: */
	private:
	std::string fileName; // Name of source file
	unsigned int lineNumber; // Current line number
	
	/* Private methods: */
	void skipBlockComment(void) // Skips a single block comment
		{
		/* Skip characters to the closing tag: */
		while(!Base::eof())
			{
			/* Read the next character: */
			int c=Base::getChar();
			
			/* Check for newline: */
			if(c=='\n')
				++lineNumber;
			
			/* Check for closing tag: */
			if(c=='*'&&Base::peekc()=='/')
				break;
			}
		Base::getChar(); // Skip the closing slash
		}
	void skipComments(void) // Skip newlines and line or block comments in the character source
		{
		while(Base::peekc()=='\n'||Base::peekc()=='/')
			{
			if(Base::peekc()=='\n')
				{
				/* Skip the newline: */
				Base::getChar();
				++lineNumber;
				Base::skipWs();
				}
			else
				{
				/* Skip the slash: */
				Base::getChar();
				
				/* Check the next character: */
				if(Base::peekc()=='/') // Line comment
					{
					/* Skip to the end of the line: */
					Base::skipLine();
					++lineNumber;
					Base::skipWs();
					}
				else if(Base::peekc()=='*') // Block comment
					{
					Base::getChar();
					skipBlockComment();
					Base::skipWs();
					}
				else // Not a comment after all
					{
					/* Return the slash to the character source: */
					Base::ungetChar('/');
					break;
					}
				}
			}
		}
	
	/* Constructors and destructors: */
	public:
	Doom3ValueSource(IO::FilePtr sSource,std::string sFileName)
		:Base(sSource),fileName(sFileName),lineNumber(1)
		{
		/* Set default punctuation and quote characters: */
		Base::setPunctuation(",;:?!<>()[]{}\n");
		Base::setQuotes("'\"");
		Base::skipWs();
		skipComments();
		}
	
	/* Overloaded methods from IO::ValueSource: */
	void skipWs(void)
		{
		Base::skipWs();
		skipComments();
		}
	void skipLine(void)
		{
		while(!Base::eof()&&Base::peekc()!='\n')
			{
			/* Check for a block comment, which has to be skipped completely even if it spans multiple lines: */
			if(Base::getChar()=='/'&&Base::peekc()=='*')
				{
				Base::getChar();
				skipBlockComment();
				}
			}
		Base::getChar(); // Skip the newline character
		++lineNumber;
		}
	int readChar(void)
		{
		int result=Base::readChar();
		skipComments();
		return result;
		}
	bool isString(const char* string)
		{
		bool result=Base::isString(string);
		skipComments();
		return result;
		}
	bool isLiteral(const char* literal)
		{
		bool result=Base::isLiteral(literal);
		skipComments();
		return result;
		}
	bool isLiteral(char literal)
		{
		bool result=Base::isLiteral(literal);
		skipComments();
		return result;
		}
	bool isCaseLiteral(const char* literal)
		{
		bool result=Base::isCaseLiteral(literal);
		skipComments();
		return result;
		}
	bool isCaseLiteral(char literal)
		{
		bool result=Base::isCaseLiteral(literal);
		skipComments();
		return result;
		}
	void skipString(void)
		{
		Base::skipString();
		skipComments();
		}
	std::string readString(void)
		{
		std::string result=Base::readString();
		skipComments();
		return result;
		}
	int readInteger(void)
		{
		int result=0;
		try
			{
			result=Base::readInteger();
			skipComments();
			}
		catch(Base::NumberError err)
			{
			Misc::throwStdErr("Doom3ValueSource: Number format error at %s:%s",fileName.c_str(),lineNumber);
			}
		return result;
		}
	unsigned int readUnsignedInteger(void)
		{
		unsigned int result=0;
		try
			{
			result=Base::readUnsignedInteger();
			skipComments();
			}
		catch(Base::NumberError err)
			{
			Misc::throwStdErr("Doom3ValueSource: Number format error at %s:%u",fileName.c_str(),lineNumber);
			}
		return result;
		}
	double readNumber(void)
		{
		double result=0.0;
		try
			{
			result=Base::readNumber();
			if(Base::peekc()=='f') // Check for superfluous "float" marker
				Base::readChar(); // Eat it
			skipComments();
			}
		catch(Base::NumberError err)
			{
			Misc::throwStdErr("Doom3ValueSource: Number format error at %s:%u",fileName.c_str(),lineNumber);
			}
		return result;
		}
	
	/* New methods: */
	std::string where(void) const // Returns a string with the current file:line location
		{
		return Misc::stringPrintf("%s:%u",fileName.c_str(),lineNumber);
		}
	};

}

#endif
