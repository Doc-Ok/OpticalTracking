/***********************************************************************
ImageExtractorYUYV - Class to extract images from raw video frames
encoded in YpCbCr 4:2:2 format.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Basic Video Library (Video).

The Basic Video Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The Basic Video Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Basic Video Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Video/ImageExtractorYUYV.h>

#include <Video/FrameBuffer.h>
#include <Video/Colorspaces.h>

namespace Video {

/***********************************
Methods of class ImageExtractorYUYV:
***********************************/

ImageExtractorYUYV::ImageExtractorYUYV(const unsigned int sSize[2])
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	}

void ImageExtractorYUYV::extractGrey(const FrameBuffer* frame,void* image)
	{
	/* Convert the frame's Y' channel to Y: */
	const unsigned char* rRowPtr=frame->start;
	unsigned char* gRowPtr=static_cast<unsigned char*>(image);
	gRowPtr+=(size[1]-1)*size[0];
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=size[0]*2,gRowPtr-=size[0])
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* gPtr=gRowPtr;
		for(unsigned int x=0;x<size[0];++x,++gPtr,rPtr+=2)
			{
			/* Convert from Y' to Y: */
			if(*rPtr<=16)
				*gPtr=0;
			else if(*rPtr>=236)
				*gPtr=255;
			else
				*gPtr=(unsigned char)(((int(rPtr[0])-16)*256)/220);
			}
		}
	}

void ImageExtractorYUYV::extractRGB(const FrameBuffer* frame,void* image)
	{
	/* Convert the frame from Y'CbCr to RGB: */
	const unsigned char* rRowPtr=frame->start;
	unsigned char* cRowPtr=static_cast<unsigned char*>(image);
	cRowPtr+=(size[1]-1)*size[0]*3;
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=size[0]*2,cRowPtr-=size[0]*3)
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* cPtr=cRowPtr;
		for(unsigned int x=0;x<size[0];x+=2,cPtr+=2*3,rPtr+=4)
			{
			/* Convert first pixel: */
			unsigned char ypcbcr[3];
			ypcbcr[0]=rPtr[0];
			ypcbcr[1]=rPtr[1];
			ypcbcr[2]=rPtr[3];
			Video::ypcbcrToRgb(ypcbcr,cPtr);
			
			/* Convert second pixel: */
			ypcbcr[0]=rPtr[2];
			Video::ypcbcrToRgb(ypcbcr,cPtr+3);
			}
		}
	}

void ImageExtractorYUYV::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Process all blocks of two pixel rows: */
	const unsigned char* framePtr=frame->start;
	unsigned char* ypRowPtr=static_cast<unsigned char*>(yp);
	unsigned char* cbRowPtr=static_cast<unsigned char*>(cb);
	unsigned char* crRowPtr=static_cast<unsigned char*>(cr);
	for(unsigned int y=0;y<size[1];y+=2)
		{
		/* Process an even row by keeping its Cb values: */
		unsigned char* ypPtr=ypRowPtr;
		unsigned char* cbPtr=cbRowPtr;
		for(unsigned int x=0;x<size[0];x+=2)
			{
			/* Get Yp and Cb from even pixel: */
			*(ypPtr++)=*(framePtr++);
			*(cbPtr++)=*(framePtr++);
			
			/* Get Yp from odd pixel: */
			*(ypPtr++)=*(framePtr++);
			++framePtr;
			}
		ypRowPtr+=ypStride;
		cbRowPtr+=cbStride;
		
		/* Process an odd row by keeping its Cr values: */
		ypPtr=ypRowPtr;
		unsigned char* crPtr=crRowPtr;
		for(unsigned int x=0;x<size[0];x+=2)
			{
			/* Get Yp from even pixel: */
			*(ypPtr++)=*(framePtr++);
			++framePtr;
			
			/* Get Yp and Cr from odd pixel: */
			*(ypPtr++)=*(framePtr++);
			*(crPtr++)=*(framePtr++);
			}
		ypRowPtr+=ypStride;
		crRowPtr+=crStride;
		}
	}

}
