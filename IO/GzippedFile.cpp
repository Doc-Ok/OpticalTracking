/***********************************************************************
GzippedFile - Class for high-performance reading from gzip-compressed
standard operating system files.
Copyright (c) 2011-2012 Oliver Kreylos

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

#include <IO/GzippedFile.h>

#include <Misc/ThrowStdErr.h>

namespace IO {

/****************************
Methods of class GzippedFile:
****************************/

size_t GzippedFile::readData(File::Byte* buffer,size_t bufferSize)
	{
	/* Read at most one buffer's worth of data from the compressed input file: */
	int readSize=gzread(inputFile,buffer,bufferSize);
	
	/* Check for fatal read errors: */
	if(readSize<0)
		throw Error(Misc::printStdErrMsg("IO::GzippedFile: Fatal error while reading from file"));
	
	return size_t(readSize);
	}

GzippedFile::GzippedFile(const char* inputFileName)
	:File(ReadOnly),
	 inputFile(0)
	{
	/* Open the compressed input file: */
	if((inputFile=gzopen(inputFileName,"r"))==0)
		throw OpenError(Misc::printStdErrMsg("IO::GzippedFile: Error while opening gzipped input file %s",inputFileName));
	}

GzippedFile::~GzippedFile(void)
	{
	/* Close the compressed input file: */
	if(inputFile!=0)
		gzclose(inputFile);
	}

}
