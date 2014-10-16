/***********************************************************************
ImageExtractorBA81 - Class to extract images from raw video frames
encoded using an eight-bit Bayer pattern.
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

#include <Video/ImageExtractorBA81.h>

#include <Misc/SizedTypes.h>
#include <Video/FrameBuffer.h>
#include <Video/Colorspaces.h>

namespace Video {

namespace {

/****************
Helper functions:
****************/

inline unsigned char avg(unsigned char v1,unsigned char v2)
	{
	return (unsigned char)(((unsigned int)(v1)+(unsigned int)(v2)+1U)/2U);
	}

inline unsigned char avg(unsigned char v1,unsigned char v2,unsigned char v3)
	{
	return (unsigned char)(((unsigned int)(v1)+(unsigned int)(v2)+(unsigned int)(v3)+1U)/3U);
	}

inline unsigned char avg(unsigned char v1,unsigned char v2,unsigned char v3,unsigned char v4)
	{
	return (unsigned char)(((unsigned int)(v1)+(unsigned int)(v2)+(unsigned int)(v3)+(unsigned int)(v4)+2U)/4U);
	}

inline unsigned char rgbToGrey(unsigned char r,unsigned char g,unsigned char b)
	{
	return (unsigned char)(((unsigned int)r*306U+(unsigned int)g*601U+(unsigned int)b*117U+512U)>>10);
	}

}

/***********************************
Methods of class ImageExtractorBA81:
***********************************/

void ImageExtractorBA81::extractGreyFromBGGR(const FrameBuffer* frame,void* image)
	{
	/* Convert the Bayer-filtered image to greyscale via RGB: */
	int stride=size[0];
	const unsigned char* rRowPtr=frame->start;
	unsigned char* cRowPtr=reinterpret_cast<unsigned char*>(image);
	cRowPtr+=(size[1]-1)*stride;
	
	/* Convert the first row: */
	const unsigned char* rPtr=rRowPtr;
	unsigned char* cPtr=cRowPtr;
	
	/* Convert the first row's first (B) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[stride+1],avg(rPtr[1],rPtr[stride]),rPtr[0]);
	++rPtr;
	
	/* Convert the first row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (G) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[stride],rPtr[0],avg(rPtr[-1],rPtr[1]));
		++rPtr;
		
		/* Convert the even (B) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[stride-1],rPtr[stride+1]),avg(rPtr[-1],rPtr[1],rPtr[stride]),rPtr[0]);
		++rPtr;
		}
	
	/* Convert the first row's last (G) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[stride],rPtr[0],rPtr[-1]);
	++rPtr;
	
	rRowPtr+=stride;
	cRowPtr-=stride;
	
	/* Convert the central rows: */
	for(unsigned int y=1;y<size[1]-1;y+=2)
		{
		/* Convert the odd row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the odd row's first (G) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[1],rPtr[0],avg(rPtr[-stride],rPtr[stride]));
		++rPtr;
		
		/* Convert the odd row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (R) pixel: */
			*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]),avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]));
			++rPtr;
			
			/* Convert the even (G) pixel: */
			*(cPtr++)=rgbToGrey(avg(rPtr[-1],rPtr[1]),rPtr[0],avg(rPtr[-stride],rPtr[stride]));
			++rPtr;
			}
		
		/* Convert the odd row's last (R) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-stride],rPtr[-1],rPtr[stride]),avg(rPtr[-stride-1],rPtr[stride-1]));
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride;
		
		/* Convert the even row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the even row's first (B) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-stride+1],rPtr[stride+1]),avg(rPtr[-stride],rPtr[1],rPtr[stride]),rPtr[0]);
		++rPtr;
		
		/* Convert the even row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (G) pixel: */
			*(cPtr++)=rgbToGrey(avg(rPtr[-stride],rPtr[stride]),rPtr[0],avg(rPtr[-1],rPtr[1]));
			++rPtr;
			
			/* Convert the even (B) pixel: */
			*(cPtr++)=rgbToGrey(avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]),avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]),rPtr[0]);
			++rPtr;
			}
		
		/* Convert the even row's last (G) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-stride],rPtr[stride]),rPtr[0],rPtr[-1]);
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride;
		}
	
	/* Convert the last row: */
	rPtr=rRowPtr;
	cPtr=cRowPtr;
	
	/* Convert the last row's first (G) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[1],rPtr[0],rPtr[-stride]);
	++rPtr;
	
	/* Convert the last row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (R) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-stride],rPtr[-1],rPtr[1]),avg(rPtr[-stride-1],rPtr[-stride+1]));
		++rPtr;
		
		/* Convert the even (G) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-1],rPtr[1]),rPtr[0],rPtr[-stride]);
		++rPtr;
		}
	
	/* Convert the last row's last (R) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-stride],rPtr[-1]),rPtr[-stride-1]);
	}

void ImageExtractorBA81::extractGreyFromRGGB(const FrameBuffer* frame,void* image)
	{
	/* Convert the Bayer-filtered image to greyscale via RGB: */
	int stride=size[0];
	const unsigned char* rRowPtr=frame->start;
	unsigned char* cRowPtr=reinterpret_cast<unsigned char*>(image);
	cRowPtr+=(size[1]-1)*stride;
	
	/* Convert the first row: */
	const unsigned char* rPtr=rRowPtr;
	unsigned char* cPtr=cRowPtr;
	
	/* Convert the first row's first (R) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[1],rPtr[stride]),rPtr[stride+1]);
	++rPtr;
	
	/* Convert the first row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (G) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-1],rPtr[1]),rPtr[0],rPtr[stride]);
		++rPtr;
		
		/* Convert the even (R) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-1],rPtr[1],rPtr[stride]),avg(rPtr[stride-1],rPtr[stride+1]));
		++rPtr;
		}
	
	/* Convert the first row's last (G) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[-1],rPtr[0],rPtr[stride]);
	++rPtr;
	
	rRowPtr+=stride;
	cRowPtr-=stride;
	
	/* Convert the central rows: */
	for(unsigned int y=1;y<size[1]-1;y+=2)
		{
		/* Convert the odd row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the odd row's first (G) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-stride],rPtr[stride]),rPtr[0],rPtr[1]);
		++rPtr;
		
		/* Convert the odd row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (B) pixel: */
			*(cPtr++)=rgbToGrey(avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]),avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]),rPtr[0]);
			++rPtr;
			
			/* Convert the even (G) pixel: */
			*(cPtr++)=rgbToGrey(avg(rPtr[-stride],rPtr[stride]),rPtr[0],avg(rPtr[-1],rPtr[1]));
			++rPtr;
			}
		
		/* Convert the odd row's last (B) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-stride-1],rPtr[stride-1]),avg(rPtr[-stride],rPtr[-1],rPtr[stride]),rPtr[0]);
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride;
		
		/* Convert the even row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the even row's first (R) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-stride],rPtr[1],rPtr[stride]),avg(rPtr[-stride+1],rPtr[stride+1]));
		++rPtr;
		
		/* Convert the even row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (G) pixel: */
			*(cPtr++)=rgbToGrey(avg(rPtr[-1],rPtr[1]),rPtr[0],avg(rPtr[-stride],rPtr[stride]));
			++rPtr;
			
			/* Convert the even (R) pixel: */
			*(cPtr++)=rgbToGrey(rPtr[0],avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]),avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]));
			++rPtr;
			}
		
		/* Convert the even row's last (G) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[-1],rPtr[0],avg(rPtr[-stride],rPtr[stride]));
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride;
		}
	
	/* Convert the last row: */
	rPtr=rRowPtr;
	cPtr=cRowPtr;
	
	/* Convert the last row's first (G) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[-stride],rPtr[0],rPtr[1]);
	++rPtr;
	
	/* Convert the last row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (B) pixel: */
		*(cPtr++)=rgbToGrey(avg(rPtr[-stride-1],rPtr[-stride+1]),avg(rPtr[-stride],rPtr[-1],rPtr[1]),rPtr[0]);
		++rPtr;
		
		/* Convert the even (G) pixel: */
		*(cPtr++)=rgbToGrey(rPtr[-stride],rPtr[0],avg(rPtr[-1],rPtr[1]));
		++rPtr;
		}
	
	/* Convert the last row's last (B) pixel: */
	*(cPtr++)=rgbToGrey(rPtr[-stride-1],avg(rPtr[-stride],rPtr[-1]),rPtr[0]);
	}

void ImageExtractorBA81::extractRGBFromBGGR(const FrameBuffer* frame,void* image)
	{
	/* Convert the Bayer-filtered image to RGB: */
	int stride=size[0];
	const unsigned char* rRowPtr=frame->start;
	unsigned char* cRowPtr=reinterpret_cast<unsigned char*>(image);
	cRowPtr+=(size[1]-1)*stride*3;
	
	/* Convert the first row: */
	const unsigned char* rPtr=rRowPtr;
	unsigned char* cPtr=cRowPtr;
	
	/* Convert the first row's first (B) pixel: */
	*(cPtr++)=rPtr[stride+1];
	*(cPtr++)=avg(rPtr[1],rPtr[stride]);
	*(cPtr++)=rPtr[0];
	++rPtr;
	
	/* Convert the first row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (G) pixel: */
		*(cPtr++)=rPtr[stride];
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-1],rPtr[1]);
		++rPtr;
		
		/* Convert the even (B) pixel: */
		*(cPtr++)=avg(rPtr[stride-1],rPtr[stride+1]);
		*(cPtr++)=avg(rPtr[-1],rPtr[1],rPtr[stride]);
		*(cPtr++)=rPtr[0];
		++rPtr;
		}
	
	/* Convert the first row's last (G) pixel: */
	*(cPtr++)=rPtr[stride];
	*(cPtr++)=rPtr[0];
	*(cPtr++)=rPtr[-1];
	++rPtr;
	
	rRowPtr+=stride;
	cRowPtr-=stride*3;
	
	/* Convert the central rows: */
	for(unsigned int y=1;y<size[1]-1;y+=2)
		{
		/* Convert the odd row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the odd row's first (G) pixel: */
		*(cPtr++)=rPtr[1];
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
		++rPtr;
		
		/* Convert the odd row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (R) pixel: */
			*(cPtr++)=rPtr[0];
			*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]);
			*(cPtr++)=avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]);
			++rPtr;
			
			/* Convert the even (G) pixel: */
			*(cPtr++)=avg(rPtr[-1],rPtr[1]);
			*(cPtr++)=rPtr[0];
			*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
			++rPtr;
			}
		
		/* Convert the odd row's last (R) pixel: */
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[stride]);
		*(cPtr++)=avg(rPtr[-stride-1],rPtr[stride-1]);
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride*3;
		
		/* Convert the even row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the even row's first (B) pixel: */
		*(cPtr++)=avg(rPtr[-stride+1],rPtr[stride+1]);
		*(cPtr++)=avg(rPtr[-stride],rPtr[1],rPtr[stride]);
		*(cPtr++)=rPtr[0];
		++rPtr;
		
		/* Convert the even row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (G) pixel: */
			*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
			*(cPtr++)=rPtr[0];
			*(cPtr++)=avg(rPtr[-1],rPtr[1]);
			++rPtr;
			
			/* Convert the even (B) pixel: */
			*(cPtr++)=avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]);
			*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]);
			*(cPtr++)=rPtr[0];
			++rPtr;
			}
		
		/* Convert the even row's last (G) pixel: */
		*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
		*(cPtr++)=rPtr[0];
		*(cPtr++)=rPtr[-1];
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride*3;
		}
	
	/* Convert the last row: */
	rPtr=rRowPtr;
	cPtr=cRowPtr;
	
	/* Convert the last row's first (G) pixel: */
	*(cPtr++)=rPtr[1];
	*(cPtr++)=rPtr[0];
	*(cPtr++)=rPtr[-stride];
	++rPtr;
	
	/* Convert the last row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (R) pixel: */
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[1]);
		*(cPtr++)=avg(rPtr[-stride-1],rPtr[-stride+1]);
		++rPtr;
		
		/* Convert the even (G) pixel: */
		*(cPtr++)=avg(rPtr[-1],rPtr[1]);
		*(cPtr++)=rPtr[0];
		*(cPtr++)=rPtr[-stride];
		++rPtr;
		}
	
	/* Convert the last row's last (R) pixel: */
	*(cPtr++)=rPtr[0];
	*(cPtr++)=avg(rPtr[-stride],rPtr[-1]);
	*(cPtr++)=rPtr[-stride-1];
	}

void ImageExtractorBA81::extractRGBFromRGGB(const FrameBuffer* frame,void* image)
	{
	/* Convert the Bayer-filtered image to RGB: */
	int stride=size[0];
	const unsigned char* rRowPtr=frame->start;
	unsigned char* cRowPtr=reinterpret_cast<unsigned char*>(image);
	cRowPtr+=(size[1]-1)*stride*3;
	
	/* Convert the first row: */
	const unsigned char* rPtr=rRowPtr;
	unsigned char* cPtr=cRowPtr;
	
	/* Convert the first row's first (R) pixel: */
	*(cPtr++)=rPtr[0];
	*(cPtr++)=avg(rPtr[1],rPtr[stride]);
	*(cPtr++)=rPtr[stride+1];
	++rPtr;
	
	/* Convert the first row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (G) pixel: */
		*(cPtr++)=avg(rPtr[-1],rPtr[1]);
		*(cPtr++)=rPtr[0];
		*(cPtr++)=rPtr[stride];
		++rPtr;
		
		/* Convert the even (R) pixel: */
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-1],rPtr[1],rPtr[stride]);
		*(cPtr++)=avg(rPtr[stride-1],rPtr[stride+1]);
		++rPtr;
		}
	
	/* Convert the first row's last (G) pixel: */
	*(cPtr++)=rPtr[-1];
	*(cPtr++)=rPtr[0];
	*(cPtr++)=rPtr[stride];
	++rPtr;
	
	rRowPtr+=stride;
	cRowPtr-=stride*3;
	
	/* Convert the central rows: */
	for(unsigned int y=1;y<size[1]-1;y+=2)
		{
		/* Convert the odd row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the odd row's first (G) pixel: */
		*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
		*(cPtr++)=rPtr[0];
		*(cPtr++)=rPtr[1];
		++rPtr;
		
		/* Convert the odd row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (B) pixel: */
			*(cPtr++)=avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]);
			*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]);
			*(cPtr++)=rPtr[0];
			++rPtr;
			
			/* Convert the even (G) pixel: */
			*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
			*(cPtr++)=rPtr[0];
			*(cPtr++)=avg(rPtr[-1],rPtr[1]);
			++rPtr;
			}
		
		/* Convert the odd row's last (B) pixel: */
		*(cPtr++)=avg(rPtr[-stride-1],rPtr[stride-1]);
		*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[stride]);
		*(cPtr++)=rPtr[0];
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride*3;
		
		/* Convert the even row: */
		rPtr=rRowPtr;
		cPtr=cRowPtr;
		
		/* Convert the even row's first (R) pixel: */
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-stride],rPtr[1],rPtr[stride]);
		*(cPtr++)=avg(rPtr[-stride+1],rPtr[stride+1]);
		++rPtr;
		
		/* Convert the even row's central pixels: */
		for(unsigned x=1;x<size[0]-1;x+=2)
			{
			/* Convert the odd (G) pixel: */
			*(cPtr++)=avg(rPtr[-1],rPtr[1]);
			*(cPtr++)=rPtr[0];
			*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
			++rPtr;
			
			/* Convert the even (R) pixel: */
			*(cPtr++)=rPtr[0];
			*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[1],rPtr[stride]);
			*(cPtr++)=avg(rPtr[-stride-1],rPtr[-stride+1],rPtr[stride-1],rPtr[stride+1]);
			++rPtr;
			}
		
		/* Convert the even row's last (G) pixel: */
		*(cPtr++)=rPtr[-1];
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-stride],rPtr[stride]);
		++rPtr;
		
		rRowPtr+=stride;
		cRowPtr-=stride*3;
		}
	
	/* Convert the last row: */
	rPtr=rRowPtr;
	cPtr=cRowPtr;
	
	/* Convert the last row's first (G) pixel: */
	*(cPtr++)=rPtr[-stride];
	*(cPtr++)=rPtr[0];
	*(cPtr++)=rPtr[1];
	++rPtr;
	
	/* Convert the last row's central pixels: */
	for(unsigned int x=1;x<size[0]-1;x+=2)
		{
		/* Convert the odd (B) pixel: */
		*(cPtr++)=avg(rPtr[-stride-1],rPtr[-stride+1]);
		*(cPtr++)=avg(rPtr[-stride],rPtr[-1],rPtr[1]);
		*(cPtr++)=rPtr[0];
		++rPtr;
		
		/* Convert the even (G) pixel: */
		*(cPtr++)=rPtr[-stride];
		*(cPtr++)=rPtr[0];
		*(cPtr++)=avg(rPtr[-1],rPtr[1]);
		++rPtr;
		}
	
	/* Convert the last row's last (B) pixel: */
	*(cPtr++)=rPtr[-stride-1];
	*(cPtr++)=avg(rPtr[-stride],rPtr[-1]);
	*(cPtr++)=rPtr[0];
	}

ImageExtractorBA81::ImageExtractorBA81(const unsigned int sSize[2],BayerPattern sBayerPattern)
	:bayerPattern(sBayerPattern)
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	}

void ImageExtractorBA81::extractGrey(const FrameBuffer* frame,void* image)
	{
	switch(bayerPattern)
		{
		case BAYER_RGGB:
			extractGreyFromRGGB(frame,image);
			break;
		
		case BAYER_BGGR:
			extractGreyFromBGGR(frame,image);
			break;
		
		default:
			;
		}
	}

void ImageExtractorBA81::extractRGB(const FrameBuffer* frame,void* image)
	{
	switch(bayerPattern)
		{
		case BAYER_RGGB:
			extractRGBFromRGGB(frame,image);
			break;
		
		case BAYER_BGGR:
			extractRGBFromBGGR(frame,image);
			break;
		
		default:
			;
		}
	}

void ImageExtractorBA81::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Convert the raw image into a temporary RGB image (not very efficient): */
	unsigned char* tempImage=new unsigned char[size[0]*size[1]*3];
	switch(bayerPattern)
		{
		case BAYER_RGGB:
			extractRGBFromRGGB(frame,tempImage);
			break;
		
		case BAYER_BGGR:
			extractRGBFromBGGR(frame,tempImage);
			break;
		
		default:
			;
		}
	
	/* Process temporary pixels in 2x2 blocks: */
	const unsigned char* fRowPtr=tempImage+(size[1]-1)*size[0]*3;
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
	
	/* Delete the temporary RGB image: */
	delete[] tempImage;
	}

}
