/***********************************************************************
ValueSource - Class to read strings or numbers from files.
Copyright (c) 2009-2011 Oliver Kreylos

This file is part of the I/O Support Library (IO).

The I/O Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The I/O Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the I/O Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef IO_VALUESOURCE_INCLUDED
#define IO_VALUESOURCE_INCLUDED

#include <string>
#include <stdexcept>
#include <IO/File.h>

namespace IO {

class ValueSource
	{
	/* Embedded classes: */
	public:
	class NumberError:public std::runtime_error // Class to signal errors while reading numbers
		{
		/* Constructors and destructors: */
		public:
		NumberError(void)
			:std::runtime_error("IO::ValueSource: Number format error")
			{
			}
		};
	
	private:
	enum CharacterClasses // Enumerated type for character class bit masks to speed up tokenization
		{
		NONE=0x0,
		WHITESPACE=0x1, // Class for characters skipped by skipWs()
		PUNCTUATION=0x2, // User-definable class for syntactic punctuation, i.e., single-character tokens
		DIGIT=0x4, // Class for characters allowed in numbers
		STRING=0x8, // Class for characters allowed in unquoted strings
		QUOTE=0x10, // Class for characters that start / end quoted strings
		QUOTEDSTRING=0x20 // Class for characters allowed in quoted strings
		};
	
	/* Elements: */
	FilePtr source; // Data source for value reader
	unsigned char characterClasses[257]; // Array of character type bit flags for quicker classification, with extra space for EOF
	unsigned char* cc; // Pointer into character classes array to account for EOF==-1
	int escapeChar; // Escape character for quoted and non-quoted strings; -1 if escape sequences should be ignored
	int lastChar; // Last character read from character source
	
	/* Private methods: */
	char processEscape(void); // Processes an escape sequence from the character source
	
	/* Constructors and destructors: */
	public:
	ValueSource(FilePtr sSource); // Creates a value source for the given character source
	~ValueSource(void);
	
	/* Methods: */
	void resetCharacterClasses(void); // Resets the character classes to the default
	void setWhitespace(int character,bool whitespace); // Sets the given character's whitespace flag
	void setWhitespace(const char* whitespace); // Sets the whitespace character set to the contents of the given string
	void setPunctuation(int character,bool punctuation); // Sets the given character's punctuation flag
	void setPunctuation(const char* punctuation); // Sets the punctuation character set to the contents of the given string
	void setQuote(int character,bool quote); // Sets the given character's quote flag
	void setQuotes(const char* quotes); // Sets the quote character set to the contents of the given string
	void setEscape(int newEscapeChar); // Sets the escape character for strings; escapes are ignored if equal to -1
	
	bool eof(void) const // Returns true if the entire character source has been read
		{
		return lastChar<0;
		}
	void skipWs(void) // Skips whitespace in the character source
		{
		/* Skip all whitespace characters: */
		while(cc[lastChar]&WHITESPACE)
			lastChar=source->getChar();
		}
	void skipLine(void); // Skips characters up to and including the next newline character
	int peekc(void) const // Returns the next character that will be read, without reading it
		{
		return lastChar;
		}
	int getChar(void) // Returns the next character
		{
		int result=lastChar;
		lastChar=source->getChar();
		return result;
		}
	void ungetChar(int character) // Puts the given character back into the character source
		{
		source->ungetChar(lastChar);
		lastChar=character;
		}
	
	int readChar(void) // Reads and returns a single character
		{
		int result=lastChar;
		lastChar=source->getChar();
		skipWs();
		
		return result;
		}
	std::string readLine(void); // Reads characters until the end of the current line, and skips the newline character
	unsigned int matchString(const char* string); // Tries to match the given string against characters from the current position; ignores character classes and does not skip whitespace; returns position of first mismatch (or length of string)
	bool isString(const char* string) // Tries to match the given string against characters from the current position; returns true if exact match; stops reading before first mismatch
		{
		/* Check if the string was completely matched: */
		bool result=string[matchString(string)]=='\0';
		
		/* Check if the source's string was completely matched, and gobble it up: */
		while(cc[lastChar]&STRING)
			{
			result=false;
			lastChar=source->getChar();
			}
		skipWs();
		
		return result;
		}
	bool isLiteral(const char* literal); // Returns true if the next string or punctuation character exactly matches the given literal
	bool isLiteral(char literal); // Ditto, for single-character literal
	bool isCaseLiteral(const char* literal); // Returns true if the next string or punctuation character matches the given literal up to case
	bool isCaseLiteral(char literal); // Ditto, for single-character literal
	void skipString(void); // Skips the next string or punctuation character from the character source
	std::string readString(void); // Reads the next string or punctuation character from the character source
	int readInteger(void); // Reads the next signed integer from the character source
	unsigned int readUnsignedInteger(void); // Reads the next unsigned integer from the character source
	double readNumber(void); // Reads the next floating-point number from the character source
	};

}

#endif
