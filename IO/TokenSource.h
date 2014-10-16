/***********************************************************************
TokenSource - Class to read tokens from files.
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

#ifndef IO_TOKENSOURCE_INCLUDED
#define IO_TOKENSOURCE_INCLUDED

#include <stddef.h>
#include <IO/File.h>

namespace IO {

class TokenSource
	{
	/* Embedded classes: */
	private:
	enum CharacterClasses // Enumerated type for character class bit masks to speed up tokenization
		{
		NONE=0x0,
		WHITESPACE=0x1, // Class for characters skipped by skipWs()
		PUNCTUATION=0x2, // User-definable class for syntactic punctuation, i.e., single-character tokens
		TOKEN=0x4, // Class for characters allowed in non-quoted tokens
		QUOTE=0x8, // Class for characters that start / end quoted tokens
		QUOTEDTOKEN=0x10 // Class for characters allowed in quoted tokens
		};
	
	/* Elements: */
	FilePtr source; // Data source for token reader
	unsigned char characterClasses[257]; // Array of character type bit flags for quicker classification, with extra space for EOF
	unsigned char* cc; // Pointer into character classes array to account for EOF==-1
	int lastChar; // Last character read from character source
	size_t tokenBufferSize; // Current size of token buffer
	char* tokenBuffer; // Pointer to token buffer
	size_t tokenSize; // Length of the current token excluding terminator character
	
	/* Private methods: */
	void initCharacterClasses(void); // Initializes the character classes array
	void resizeTokenBuffer(void); // Creates additional room in the token buffer
	
	/* Constructors and destructors: */
	public:
	TokenSource(FilePtr sSource); // Creates a token source for the given character source
	~TokenSource(void);
	
	/* Methods: */
	void setWhitespace(int character,bool whitespace); // Sets the given character's whitespace flag
	void setWhitespace(const char* whitespace); // Sets the whitespace character set to the contents of the given string
	void setPunctuation(int character,bool punctuation); // Sets the given character's punctuation flag
	void setPunctuation(const char* punctuation); // Sets the punctuation character set to the contents of the given string
	void setQuote(int character,bool quote); // Sets the given character's quote flag
	void setQuotes(const char* quotes); // Sets the quote character set to the contents of the given string
	bool eof(void) const // Returns true if the entire character source has been read
		{
		return lastChar<0;
		}
	void skipWs(void); // Skips whitespace in the character source
	void skipLine(void); // Skips characters up to and including the next newline character
	int peekc(void) const // Returns the next character that will be read, without reading it
		{
		return lastChar;
		}
	const char* readNextToken(void); // Reads the next token, i.e., either a single punctuation character, or a sequence of non-whitespace and non-punctuation characters, then skips whitespace
	size_t getTokenSize(void) const // Returns the length of the most recently read token
		{
		return tokenSize;
		}
	const char* getToken(void) // Returns the most recently read token; readNextToken must be called first
		{
		return tokenBuffer;
		}
	bool isToken(const char* token) const; // Returns true if the most recently read token matches the given string
	bool isCaseToken(const char* token) const; // Returns true if the most recently read token matches the given string up to case
	};

}

#endif
