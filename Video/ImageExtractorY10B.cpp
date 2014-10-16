/***********************************************************************
ImageExtractorY10B - Class to extract images from raw video frames
encoded 10-bit byte-packed greyscale format.
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

#include <Video/ImageExtractorY10B.h>

#include <string.h>
#include <Video/FrameBuffer.h>
#include <Video/Colorspaces.h>

namespace Video {

/***********************************
Methods of class ImageExtractorY10B:
***********************************/

ImageExtractorY10B::ImageExtractorY10B(const unsigned int sSize[2])
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	}

void ImageExtractorY10B::extractGrey(const FrameBuffer* frame,void* image)
	{
	/* Unpack pixel bits and convert the frame's Y' channel to Y: */
	const unsigned char* rRowPtr=frame->start;
	unsigned char* gRowPtr=static_cast<unsigned char*>(image);
	gRowPtr+=(size[1]-1)*size[0];
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=(size[0]*5)/4,gRowPtr-=size[0])
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* gPtr=gRowPtr;
		for(unsigned int x=0;x<size[0];x+=4,gPtr+=4,rPtr+=5)
			{
			/* Extract the pixel values from a run of four pixels: */
			unsigned int yps[4];
			yps[0]=((unsigned int)rPtr[0]<<2)|((unsigned int)rPtr[1]>>6);
			yps[1]=(((unsigned int)rPtr[1]&0x3fU)<<4)|((unsigned int)rPtr[2]>>4);
			yps[2]=(((unsigned int)rPtr[2]&0x0fU)<<6)|((unsigned int)rPtr[3]>>2);
			yps[3]=(((unsigned int)rPtr[3]&0x03U)<<8)|(unsigned int)rPtr[4];
			
			/* Convert the four pixel values from Y' to Y: */
			for(int i=0;i<4;++i)
				{
				/* Convert from Y' to Y: */
				if(yps[i]<=64U)
					gPtr[i]=0U;
				else if(yps[i]>=944U)
					gPtr[i]=255U;
				else
					gPtr[i]=(unsigned char)(((yps[i]-64U)*256U)/880U);
				}
			}
		}
	}

void ImageExtractorY10B::extractRGB(const FrameBuffer* frame,void* image)
	{
	/* Unpack pixel bits and convert the frame's Y' channel to Y and then to RGB: */
	const unsigned char* rRowPtr=frame->start;
	unsigned char* rgbRowPtr=static_cast<unsigned char*>(image);
	rgbRowPtr+=(size[1]-1)*size[0]*3;
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=(size[0]*5)/4,rgbRowPtr-=size[0]*3)
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* rgbPtr=rgbRowPtr;
		for(unsigned int x=0;x<size[0];x+=4,rPtr+=5)
			{
			/* Extract the pixel values from a run of four pixels: */
			unsigned int yps[4];
			yps[0]=((unsigned int)rPtr[0]<<2)|((unsigned int)rPtr[1]>>6);
			yps[1]=(((unsigned int)rPtr[1]&0x3fU)<<4)|((unsigned int)rPtr[2]>>4);
			yps[2]=(((unsigned int)rPtr[2]&0x0fU)<<6)|((unsigned int)rPtr[3]>>2);
			yps[3]=(((unsigned int)rPtr[3]&0x03U)<<8)|(unsigned int)rPtr[4];
			
			/* Convert the four pixel values from Y' to Y: */
			for(int i=0;i<4;++i)
				{
				/* Convert from Y' to Y: */
				unsigned char y;
				if(yps[i]<=64U)
					y=0U;
				else if(yps[i]>=944U)
					y=255U;
				else
					y=(unsigned char)(((yps[i]-62U)*256U)/880U);
				for(int j=0;j<3;++j,++rgbPtr)
					*rgbPtr=y;
				}
			}
		}
	}

void ImageExtractorY10B::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Unpack pixel bits and copy the frame's Y' channel into the Y' plane: */
	const unsigned char* rRowPtr=frame->start;
	unsigned char* ypRowPtr=static_cast<unsigned char*>(yp);
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=(size[0]*5)/4,ypRowPtr+=ypStride)
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* ypPtr=ypRowPtr;
		for(unsigned int x=0;x<size[0];x+=4,rPtr+=5,ypPtr+=4)
			{
			/* Extract the pixel values from a run of four pixels: */
			ypPtr[0]=(unsigned char)(((((unsigned int)rPtr[0]<<2)|((unsigned int)rPtr[1]>>6))+2U)>>2);
			ypPtr[1]=(unsigned char)((((((unsigned int)rPtr[1]&0x3fU)<<4)|((unsigned int)rPtr[2]>>4))+2U)>>2);
			ypPtr[2]=(unsigned char)((((((unsigned int)rPtr[2]&0x0fU)<<6)|((unsigned int)rPtr[3]>>2))+2U)>>2);
			ypPtr[3]=(unsigned char)((((((unsigned int)rPtr[3]&0x03U)<<8)|(unsigned int)rPtr[4])+2U)>>2);
			}
		}
	
	/* Reset the Cb and Cr planes to zero: */
	unsigned char* cbRowPtr=static_cast<unsigned char*>(cb);
	unsigned char* crRowPtr=static_cast<unsigned char*>(cr);
	for(unsigned int y=0;y<size[1];y+=2,cbRowPtr+=cbStride,crRowPtr+=crStride)
		{
		/* Reset the two planes' pixel row to zero: */
		memset(cbRowPtr,0,size[0]/2);
		memset(crRowPtr,0,size[0]/2);
		}
	}

}
