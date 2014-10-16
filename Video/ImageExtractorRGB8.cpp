/***********************************************************************
ImageExtractorRGB8 - Class to extract images from video frames in RGB8
format.
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

#include <Video/ImageExtractorRGB8.h>

#include <string.h>
#include <Video/FrameBuffer.h>
#include <Video/Colorspaces.h>

namespace Video {

/***********************************
Methods of class ImageExtractorRGB8:
***********************************/

ImageExtractorRGB8::ImageExtractorRGB8(const unsigned int sSize[2])
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	}

void ImageExtractorRGB8::extractGrey(const FrameBuffer* frame,void* image)
	{
	/* Convert the frame's pixels to grey: */
	const unsigned char* rRowPtr=frame->start;
	unsigned char* gRowPtr=static_cast<unsigned char*>(image);
	gRowPtr+=(size[1]-1)*size[0];
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=size[0]*3,gRowPtr-=size[0])
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* gPtr=gRowPtr;
		for(unsigned int x=0;x<size[0];++x,++gPtr,rPtr+=3)
			{
			/* Convert RGB to grey: */
			*gPtr=(unsigned char)(((unsigned int)rPtr[0]*306U+(unsigned int)rPtr[1]*601U+(unsigned int)rPtr[2]*117U)>>10);
			}
		}
	}

void ImageExtractorRGB8::extractRGB(const FrameBuffer* frame,void* image)
	{
	/* Copy the frame directly to the result image: */
	memcpy(image,frame->start,size[0]*size[1]*3);
	}

void ImageExtractorRGB8::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Process pixels in 2x2 blocks: */
	const unsigned char* fRowPtr=frame->start+(size[1]-1)*size[0]*3;
	unsigned char* ypRowPtr=static_cast<unsigned char*>(yp);
	unsigned char* cbRowPtr=static_cast<unsigned char*>(cb);
	unsigned char* crRowPtr=static_cast<unsigned char*>(cr);
	for(unsigned int y=0;y<size[1];y+=2)
		{
		const unsigned char* fPtr=fRowPtr;
		unsigned char* ypPtr=ypRowPtr;
		unsigned char* cbPtr=cbRowPtr;
		unsigned char* crPtr=crRowPtr;
		for(unsigned int x=0;x<size[0];x+=2)
			{
			/* Convert the 2x2 pixel block to Y'CbCr: */
			unsigned char ypcbcr[4][3];
			Video::rgbToYpcbcr(fPtr,ypcbcr[0]);
			Video::rgbToYpcbcr(fPtr+3,ypcbcr[1]);
			Video::rgbToYpcbcr(fPtr-size[0]*3,ypcbcr[2]);
			Video::rgbToYpcbcr(fPtr-size[0]*3+3,ypcbcr[3]);
			
			/* Subsample and store the Y'CbCr components: */
			ypPtr[0]=ypcbcr[0][0];
			ypPtr[1]=ypcbcr[1][0];
			ypPtr[ypStride]=ypcbcr[2][0];
			ypPtr[ypStride+1]=ypcbcr[3][0];
			*cbPtr=(unsigned char)((int(ypcbcr[0][1])+int(ypcbcr[1][1])+int(ypcbcr[2][1])+int(ypcbcr[3][1])+2)>>2);
			*crPtr=(unsigned char)((int(ypcbcr[0][2])+int(ypcbcr[1][2])+int(ypcbcr[2][2])+int(ypcbcr[3][2])+2)>>2);
			
			/* Go to the next pixel: */
			fPtr+=3*2;
			ypPtr+=2;
			++cbPtr;
			++crPtr;
			}
		
		/* Go to the next pixel row: */
		fRowPtr-=size[0]*3*2;
		ypRowPtr+=ypStride*2;
		cbRowPtr+=cbStride;
		crRowPtr+=crStride;
		}
	}

}
