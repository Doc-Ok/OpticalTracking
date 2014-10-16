/***********************************************************************
ImageExtractorYV12 - Class to extract images from raw video frames
encoded in YpCbCr 4:2:0 format.
Copyright (c) 2013 Oliver Kreylos

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

#include <Video/ImageExtractorYV12.h>

#include <string.h>
#include <Video/FrameBuffer.h>
#include <Video/Colorspaces.h>

namespace Video {

/***********************************
Methods of class ImageExtractorYV12:
***********************************/

ImageExtractorYV12::ImageExtractorYV12(const unsigned int sSize[2],ptrdiff_t ypOffset,ptrdiff_t ypStride,ptrdiff_t cbOffset,ptrdiff_t cbStride,ptrdiff_t crOffset,ptrdiff_t crStride)
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	
	/* Copy the planes' layouts: */
	planes[0].offset=ypOffset;
	planes[0].stride=ypStride;
	planes[1].offset=cbOffset;
	planes[1].stride=cbStride;
	planes[2].offset=crOffset;
	planes[2].stride=crStride;
	}

void ImageExtractorYV12::extractGrey(const FrameBuffer* frame,void* image)
	{
	/* Convert the frame's Y' channel to Y: */
	const unsigned char* rRowPtr=frame->start+planes[0].offset;
	unsigned char* gRowPtr=static_cast<unsigned char*>(image);
	gRowPtr+=(size[1]-1)*size[0];
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=planes[0].stride,gRowPtr-=size[0])
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* gPtr=gRowPtr;
		for(unsigned int x=0;x<size[0];++x,++rPtr,++gPtr)
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

void ImageExtractorYV12::extractRGB(const FrameBuffer* frame,void* image)
	{
	/* Convert the frame from Y'CbCr 4:2:0 to RGB by processing blocks of 2x2 pixels: */
	unsigned char* resultRowPtr=static_cast<unsigned char*>(image)+(size[1]-1)*size[0]*3;
	const unsigned char* ypRowPtr=frame->start+planes[0].offset;
	const unsigned char* cbRowPtr=frame->start+planes[1].offset;
	const unsigned char* crRowPtr=frame->start+planes[2].offset;
	for(unsigned int y=0;y<size[1];y+=2)
		{
		unsigned char* resultPtr=resultRowPtr;
		const unsigned char* ypPtr=ypRowPtr;
		const unsigned char* cbPtr=cbRowPtr;
		const unsigned char* crPtr=crRowPtr;
		for(unsigned int x=0;x<size[0];x+=2)
			{
			/* Convert the four pixels in the 2x2 block from Y'CbCr to RGB: */
			unsigned char ypcbcr[3];
			ypcbcr[0]=ypPtr[0];
			ypcbcr[1]=*cbPtr;
			ypcbcr[2]=*crPtr;
			Video::ypcbcrToRgb(ypcbcr,resultPtr);
			
			ypcbcr[0]=ypPtr[1];
			Video::ypcbcrToRgb(ypcbcr,resultPtr+3);
			
			ypcbcr[0]=ypPtr[planes[0].stride];
			Video::ypcbcrToRgb(ypcbcr,resultPtr-size[0]*3);
			
			ypcbcr[0]=ypPtr[planes[0].stride+1];
			Video::ypcbcrToRgb(ypcbcr,resultPtr-size[0]*3+3);
			
			/* Go to the next pixel: */
			resultPtr+=2*3;
			ypPtr+=2;
			++cbPtr;
			++crPtr;
			}

		/* Go to the next row: */
		resultRowPtr-=2*size[0]*3;
		ypRowPtr+=2*planes[0].stride;
		cbRowPtr+=planes[1].stride;
		crRowPtr+=planes[2].stride;
		}
	}


void ImageExtractorYV12::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Copy the Y' plane directly: */
	const unsigned char* ypSrcRowPtr=frame->start+planes[0].offset;
	unsigned char* ypRowPtr=static_cast<unsigned char*>(yp);
	for(unsigned int y=0;y<size[1];++y)
		{
		memcpy(ypRowPtr,ypSrcRowPtr,size[0]);
		ypSrcRowPtr+=planes[0].stride;
		ypRowPtr+=ypStride;
		}
	
	/* Copy the Cb and Cr planes directly: */
	for(int cbcr=0;cbcr<2;++cbcr)
		{
		const unsigned char* cbcrSrcRowPtr=frame->start+planes[cbcr+1].offset;
		unsigned char* cbcrRowPtr=static_cast<unsigned char*>(cbcr==1?cr:cb);
		unsigned int cbcrStride=cbcr==1?crStride:cbStride;
		for(unsigned int y=0;y<size[1];++y)
			{
			memcpy(cbcrRowPtr,cbcrSrcRowPtr,size[0]/2);
			cbcrSrcRowPtr+=planes[cbcr+1].stride;
			cbcrRowPtr+=cbcrStride;
			}
		}
	}

}
