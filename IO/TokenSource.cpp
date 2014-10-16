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

#include <IO/TokenSource.h>

#include <ctype.h>
#include <string.h>

namespace IO {

/****************************
Methods of class TokenSource:
****************************/

void TokenSource::initCharacterClasses(void)
	{
	cc[-1]=NONE; // EOF is nothing; nothing, I tell you!
	
	/* Set up the basic character classes: */
	for(int i=0;i<256;++i)
		{
		cc[i]=NONE;
		if(isspace(i))
			cc[i]|=WHITESPACE;
		else
			cc[i]|=TOKEN; // Everything that's not a space can be a token for now
		cc[i]|=QUOTEDTOKEN; // Pretty much everything is a quoted token
		}
	
	/* Set token terminators: */
	cc['\n']&=~QUOTEDTOKEN; // Newlines terminate quoted tokens
	}

void TokenSource::resizeTokenBuffer(void)
	{
	tokenBufferSize=(tokenBufferSize*5)/4+10;
	char* newTokenBuffer=new char[tokenBufferSize+1]; // Allocate one extra character for token terminator
	if(tokenSize>0)
		memcpy(newTokenBuffer,tokenBuffer,tokenSize);
	delete[] tokenBuffer;
	tokenBuffer=newTokenBuffer;
	}

TokenSource::TokenSource(FilePtr sSource)
	:source(sSource),
	 cc(characterClasses+1),
	 tokenBufferSize(40),tokenBuffer(new char[tokenBufferSize+1]),tokenSize(0)
	{
	/* Initialize the character classes: */
	initCharacterClasses();
	
	/* Read the first character from the character source: */
	lastChar=source->getChar();
	}

TokenSource::~TokenSource(void)
	{
	/* Put the last read character back into the character source: */
	if(lastChar>=0)
		source->ungetChar(lastChar);
	
	/* Delete the allocated token buffer: */
	delete[] tokenBuffer;
	}

void TokenSource::setWhitespace(int character,bool whitespace)
	{
	if(whitespace)
		{
		cc[character]|=WHITESPACE;
		cc[character]&=~(PUNCTUATION|QUOTE|TOKEN);
		}
	else
		{
		cc[character]&=~WHITESPACE;
		if(!(cc[character]&(PUNCTUATION|QUOTE)))
			cc[character]|=TOKEN;
		}
	}

void TokenSource::setWhitespace(const char* whitespace)
	{
	/* Clear the whitespace set: */
	for(int i=0;i<256;++i)
		if(cc[i]&WHITESPACE)
			{
			cc[i]&=~WHITESPACE;
			if(!(cc[i]&(PUNCTUATION|QUOTE)))
				cc[i]|=TOKEN;
			}
	
	/* Mark all characters in the given string as whitespace: */
	for(const char* wPtr=whitespace;*wPtr!='\0';++wPtr)
		{
		cc[int(*wPtr)]|=WHITESPACE;
		cc[int(*wPtr)]&=~(PUNCTUATION|QUOTE|TOKEN);
		}
	}

void TokenSource::setPunctuation(int character,bool punctuation)
	{
	if(punctuation)
		{
		cc[character]|=PUNCTUATION;
		cc[character]&=~(WHITESPACE|QUOTE|TOKEN);
		}
	else
		{
		cc[character]&=~PUNCTUATION;
		if(!(cc[character]&(WHITESPACE|QUOTE)))
			cc[character]|=TOKEN;
		}
	}

void TokenSource::setPunctuation(const char* punctuation)
	{
	/* Clear the punctuation set: */
	for(int i=0;i<256;++i)
		if(cc[i]&PUNCTUATION)
			{
			cc[i]&=~PUNCTUATION;
			if(!(cc[i]&(WHITESPACE|QUOTE)))
				cc[i]|=TOKEN;
			}
	
	/* Mark all characters in the given string as punctuation: */
	for(const char* pPtr=punctuation;*pPtr!='\0';++pPtr)
		{
		cc[int(*pPtr)]|=PUNCTUATION;
		cc[int(*pPtr)]&=~(WHITESPACE|QUOTE|TOKEN);
		}
	}

void TokenSource::setQuote(int character,bool quote)
	{
	if(quote)
		{
		cc[character]|=QUOTE;
		cc[character]&=~(WHITESPACE|PUNCTUATION|TOKEN);
		}
	else
		{
		cc[character]&=~QUOTE;
		if(!(cc[character]&(WHITESPACE|PUNCTUATION)))
			cc[character]|=TOKEN;
		}
	}

void TokenSource::setQuotes(const char* quotes)
	{
	/* Clear the quote set: */
	for(int i=0;i<256;++i)
		if(cc[i]&QUOTE)
			{
			cc[i]&=~QUOTE;
			if(!(cc[i]&(WHITESPACE|PUNCTUATION)))
				cc[i]|=TOKEN;
			}
	
	/* Mark all characters in the given string as quotes: */
	for(const char* qPtr=quotes;*qPtr!='\0';++qPtr)
		{
		cc[int(*qPtr)]|=QUOTE;
		cc[int(*qPtr)]&=~(WHITESPACE|PUNCTUATION|TOKEN);
		}
	}

void TokenSource::skipWs(void)
	{
	/* Skip all whitespace characters: */
	while(cc[lastChar]&WHITESPACE)
		lastChar=source->getChar();
	}

void TokenSource::skipLine(void)
	{
	/* Skip everything until the next newline: */
	while(lastChar>=0&&lastChar!='\n')
		lastChar=source->getChar();
	
	/* Skip the newline: */
	if(lastChar=='\n')
		lastChar=source->getChar();
	}

const char* TokenSource::readNextToken(void)
	{
	/* Reset the token: */
	tokenSize=0;
	
	/* Check for punctuation or quotes: */
	if(cc[lastChar]&PUNCTUATION)
		{
		/* Read a single punctuation character: */
		tokenBuffer[tokenSize++]=lastChar;
		lastChar=source->getChar();
		}
	else if(cc[lastChar]&QUOTE)
		{
		/* Read the quote character and temporarily remove it from the set of quoted string characters: */
		int quote=lastChar;
		cc[quote]&=~QUOTEDTOKEN;
		lastChar=source->getChar();
		
		/* Read characters until the matching quote, endline, or EOF: */
		while(cc[lastChar]&QUOTEDTOKEN)
			{
			if(tokenSize>=tokenBufferSize)
				resizeTokenBuffer();
			tokenBuffer[tokenSize++]=lastChar;
			lastChar=source->getChar();
			}
		
		/* Read the terminating quote, if there is one: */
		if(lastChar==quote)
			lastChar=source->getChar();
		
		/* Add the quote character to the set of quoted token characters again: */
		cc[quote]|=QUOTEDTOKEN;
		}
	else
		{
		/* Read a non-quoted token: */
		while(cc[lastChar]&TOKEN)
			{
			if(tokenSize>=tokenBufferSize)
				resizeTokenBuffer();
			tokenBuffer[tokenSize++]=lastChar;
			lastChar=source->getChar();
			}
		}
	
	/* Terminate the token: */
	tokenBuffer[tokenSize]='\0';
	
	/* Skip whitespace: */
	while(cc[lastChar]&WHITESPACE)
		lastChar=source->getChar();
	
	return tokenBuffer;
	}

bool TokenSource::isToken(const char* token) const
	{
	return strcmp(tokenBuffer,token)==0;
	}

bool TokenSource::isCaseToken(const char* token) const
	{
	return strcasecmp(tokenBuffer,token)==0;
	}

}
