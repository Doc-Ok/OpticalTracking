/***********************************************************************
SeekableFilter - Class to convert a streaming file into a seekable
file by caching its contents in memory.
Copyright (c) 2011 Oliver Kreylos

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

#include <IO/SeekableFilter.h>

namespace IO {

/*******************************
Methods of class SeekableFilter:
*******************************/

size_t SeekableFilter::readData(File::Byte* buffer,size_t bufferSize)
	{
	/* Check if the file position needs to be moved to the read position: */
	if(filePos!=readPos)
		{
		if(readPos>filePos)
			{
			/* Skip forward in buffer chain from the current position: */
			currentPos+=(readPos-filePos);
			
			/* Skip buffers until the current buffer offset is within the current buffer: */
			while(currentPos>Offset(current->size))
				{
				/* Check if there is a next buffer: */
				if(current->succ!=0)
					{
					/* Go to the next buffer: */
					currentPos-=current->size;
					current=current->succ;
					}
				else
					{
					/* Check for end-of-file: */
					if(source->eof())
						throw SeekError(readPos);
					
					/* Read more data: */
					readFromSource();
					}
				}
			}
		else
			{
			/* Skip from the beginning of the buffer chain: */
			current=head;
			currentPos=readPos;
			
			/* Skip buffers until the current buffer offset is within the current buffer: */
			while(currentPos>Offset(current->size))
				{
				/* Go to the next buffer: */
				currentPos-=current->size;
				current=current->succ;
				}
			}
		
		/* Update the file position: */
		filePos=readPos;
		}
	
	/* Check if the file position is at the end of the read data: */
	if(filePos==totalReadSize)
		{
		/* Check for end-of-file: */
		if(source->eof())
			return 0;
		
		/* Read more data: */
		readFromSource();
		}
	
	/* Read from the current file position: */
	if(currentPos==Offset(current->size))
		{
		/* Go to the next buffer: */
		current=current->succ;
		currentPos=0;
		}
	size_t copySize=current->size-size_t(currentPos);
	setReadBuffer(copySize,reinterpret_cast<Byte*>(current+1)+currentPos,false);
	readPos+=copySize;
	filePos+=copySize;
	currentPos+=copySize;
	
	return copySize;
	}

void SeekableFilter::readFromSource(void)
	{
	/* Determine amount of space left in tail buffer: */
	size_t tailSpace=memBufferSize-tail->size;
	if(tailSpace==0)
		{
		/* Allocate a new buffer: */
		Byte* newBuffer=new Byte[memBufferSize+sizeof(BufferHeader)];
		BufferHeader* newTail=new(newBuffer) BufferHeader;
		tail->succ=newTail;
		tail=newTail;
		tailSpace=memBufferSize;
		}
	
	/* Read into the tail buffer: */
	size_t readSize=source->readUpTo(reinterpret_cast<Byte*>(tail+1)+tail->size,tailSpace);
	tail->size+=readSize;
	
	/* Update the total read file size: */
	totalReadSize+=Offset(readSize);
	}

SeekableFilter::SeekableFilter(FilePtr sSource,size_t sMemBufferSize)
	:SeekableFile(),
	 source(sSource),
	 memBufferSize(sMemBufferSize),
	 head(0),tail(0),
	 totalReadSize(0)
	{
	/* Allocate the first buffer: */
	Byte* newBuffer=new Byte[memBufferSize+sizeof(BufferHeader)];
	tail=new(newBuffer) BufferHeader;
	head=tail;
	
	/* Initialize the file position: */
	filePos=0;
	current=head;
	currentPos=0;
	
	/* Disable read-through: */
	canReadThrough=false;
	}

SeekableFilter::~SeekableFilter(void)
	{
	/* Uninstall the buffered file's read buffer: */
	setReadBuffer(0,0,false);
	
	/* Delete the buffer list: */
	while(head!=0)
		{
		BufferHeader* succ=head->succ;
		delete[] reinterpret_cast<Byte*>(head);
		head=succ;
		}
	}

size_t SeekableFilter::getReadBufferSize(void) const
	{
	return memBufferSize;
	}

size_t SeekableFilter::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Can't change it; just return the current value: */
	return memBufferSize;
	}

void SeekableFilter::resizeWriteBuffer(size_t newWriteBufferSize)
	{
	/* Just ignore it */
	}

SeekableFile::Offset SeekableFilter::getSize(void) const
	{
	/* Unfortunately, we'll have to read the entire source file to determine its size: */
	while(!source->eof())
		{
		/* Read more data: */
		const_cast<SeekableFilter*>(this)->readFromSource();
		}
	
	/* Return the total file size: */
	return totalReadSize;
	}

}
