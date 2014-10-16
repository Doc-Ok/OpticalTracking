/***********************************************************************
ImageReaderPNM - Class to read images from files in Portable aNyMap
format.
Copyright (c) 2013 Oliver Kreylos

This file is part of the Image Handling Library (Images).

The Image Handling Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Image Handling Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Image Handling Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Images/ImageReaderPNM.h>

#include <Misc/SizedTypes.h>
#include <Misc/ThrowStdErr.h>
#include <IO/ValueSource.h>

namespace Images {

namespace {

/****************
Helper functions:
****************/

inline void skipComments(IO::ValueSource& source)
	{
	/* Skip all consecutive comment indicators: */
	while(source.peekc()=='#')
		{
		/* Skip until the first non-whitespace character on the next line: */
		source.skipLine();
		source.skipWs();
		}
	}

template <class PixelChannelParam>
inline
void
readASCII(
	IO::ValueSource& source,
	const ImageReader::ImageSpec& imageSpec,
	const ImageReader::ImagePlane imagePlanes[])
	{
	/* Convert image strides to multiples of pixel channel type: */
	ptrdiff_t pixelStrides[3],rowStrides[3];
	PixelChannelParam* rowPtrs[3];
	for(unsigned int i=0;i<imageSpec.numChannels;++i)
		{
		pixelStrides[i]=imagePlanes[i].pixelStride/sizeof(PixelChannelParam);
		rowStrides[i]=imagePlanes[i].rowStride/sizeof(PixelChannelParam);
		rowPtrs[i]=static_cast<PixelChannelParam*>(imagePlanes[i].basePtr);
		
		/* Flip the image vertically: */
		rowPtrs[i]+=(imageSpec.size[1]-1)*rowStrides[i];
		}
	
	/* Read the image one row at a time: */
	for(unsigned int y=0;y<imageSpec.size[1];++y)
		{
		/* Read the row one pixel at a time: */
		PixelChannelParam* pPtrs[3];
		for(unsigned int i=0;i<imageSpec.numChannels;++i)
			pPtrs[i]=rowPtrs[i];
		for(unsigned int x=0;x<imageSpec.size[0];++x)
			{
			for(unsigned int i=0;i<imageSpec.numChannels;++i)
				{
				/* Read the next pixel channel value from the ASCII file: */
				skipComments(source);
				*(pPtrs[i])=source.readUnsignedInteger();
				
				/* Go to the next pixel in this channel: */
				pPtrs[i]+=pixelStrides[i];
				}
			}
		
		/* Go to the next pixel rows for each channel (decrement pointers to flip image vertically): */
		for(unsigned int i=0;i<imageSpec.numChannels;++i)
			rowPtrs[i]-=rowStrides[i];
		}
	}

template <class PixelChannelParam>
inline
void
readBinary(
	IO::FilePtr file,
	const ImageReader::ImageSpec& imageSpec,
	const ImageReader::ImagePlane imagePlanes[])
	{
	/* Convert image strides to multiples of pixel channel type: */
	ptrdiff_t pixelStrides[3],rowStrides[3];
	PixelChannelParam* rowPtrs[3];
	for(unsigned int i=0;i<imageSpec.numChannels;++i)
		{
		pixelStrides[i]=imagePlanes[i].pixelStride/sizeof(PixelChannelParam);
		rowStrides[i]=imagePlanes[i].rowStride/sizeof(PixelChannelParam);
		rowPtrs[i]=static_cast<PixelChannelParam*>(imagePlanes[i].basePtr);
		
		/* Flip the image vertically: */
		rowPtrs[i]+=(imageSpec.size[1]-1)*rowStrides[i];
		}
	
	/* Read the image one row at a time: */
	for(unsigned int y=0;y<imageSpec.size[1];++y)
		{
		/* Read the row one pixel at a time: */
		PixelChannelParam* pPtrs[3];
		for(unsigned int i=0;i<imageSpec.numChannels;++i)
			pPtrs[i]=rowPtrs[i];
		for(unsigned int i=imageSpec.numChannels;i<3;++i)
			pPtrs[i]=0;
		for(unsigned int x=0;x<imageSpec.size[0];++x)
			{
			for(unsigned int i=0;i<imageSpec.numChannels;++i)
				{
				/* Read the next pixel channel value from the binary file: */
				*(pPtrs[i])=file->read<PixelChannelParam>();
				
				/* Go to the next pixel in this channel: */
				pPtrs[i]+=pixelStrides[i];
				}
			}
		
		/* Go to the next pixel rows for each channel (decrement pointers to flip image vertically): */
		for(unsigned int i=0;i<imageSpec.numChannels;++i)
			rowPtrs[i]-=rowStrides[i];
		}
	}

}

/*******************************
Methods of class ImageReaderPNM:
*******************************/

ImageReaderPNM::ImageReaderPNM(IO::FilePtr sFile)
	:ImageReader(sFile),
	 done(false)
	{
	/* Attach a value source to the file to read the ASCII file header: */
	IO::ValueSource header(file);
	header.skipWs();
	
	/* Read the magic field including the image type indicator: */
	int magic=header.getChar();
	imageType=header.getChar();
	if(magic!='P'||imageType<'1'||imageType>'6')
		Misc::throwStdErr("Images::ImageReaderPNM: Invalid PNM header");
	header.skipWs();
	
	/* Read the image width, height, and maximal pixel component value: */
	skipComments(header);
	imageSpec.size[0]=header.readUnsignedInteger();
	if(imageType=='1'||imageType=='4') // PBM files don't have the maxValue field
		{
		skipComments(header);
		header.setWhitespace(""); // Disable all whitespace to read the last header field
		imageSpec.size[1]=header.readUnsignedInteger();
		maxValue=1;
		}
	else
		{
		skipComments(header);
		imageSpec.size[1]=header.readUnsignedInteger();
		
		skipComments(header);
		header.setWhitespace(""); // Disable all whitespace to read the last header field
		maxValue=header.readUnsignedInteger();
		}
	
	/* Read the single (whitespace) character separating the header from the image data: */
	header.getChar();
	
	/* Fill in the rest of the image specification: */
	for(int i=0;i<2;++i)
		canvasSize[i]=imageSpec.size[i];
	imageSpec.offset[0]=imageSpec.offset[1]=0U;
	switch(imageType)
		{
		case '1': // ASCII bitmap
		case '4': // Binary bitmap
			imageSpec.colorSpace=Grayscale;
			imageSpec.hasAlpha=false;
			imageSpec.numChannels=1;
			imageSpec.channelSpecs=new ChannelSpec[1];
			imageSpec.channelSpecs[0].valueType=UnsignedInt;
			imageSpec.channelSpecs[0].numFieldBits=1U;
			imageSpec.channelSpecs[0].numValueBits=1U;
			break;
		
		case '2': // ASCII grayscale image
		case '5': // Binary grayscale image
			imageSpec.colorSpace=Grayscale;
			imageSpec.hasAlpha=false;
			imageSpec.numChannels=1;
			imageSpec.channelSpecs=new ChannelSpec[1];
			imageSpec.channelSpecs[0].valueType=UnsignedInt;
			if(maxValue<256U)
				{
				/* 8-bit channel width: */
				imageSpec.channelSpecs[0].numFieldBits=8U;
				imageSpec.channelSpecs[0].numValueBits=8U;
				}
			else
				{
				/* 16-bit channel width: */
				imageSpec.channelSpecs[0].numFieldBits=16U;
				imageSpec.channelSpecs[0].numValueBits=16U;
				}
			break;
		
		case '3': // ASCII RGB image
		case '6': // Binary RGB image
			{
			imageSpec.colorSpace=RGB;
			imageSpec.hasAlpha=false;
			imageSpec.numChannels=3;
			imageSpec.channelSpecs=new ChannelSpec[3];
			for(int i=0;i<3;++i)
				{
				imageSpec.channelSpecs[i].valueType=UnsignedInt;
				if(maxValue<256U)
					{
					/* 8-bit channel width: */
					imageSpec.channelSpecs[i].numFieldBits=8U;
					imageSpec.channelSpecs[i].numValueBits=8U;
					}
				else
					{
					/* 16-bit channel width: */
					imageSpec.channelSpecs[i].numFieldBits=16U;
					imageSpec.channelSpecs[i].numValueBits=16U;
					}
				}
			break;
			}
		}
	}

bool ImageReaderPNM::eof(void) const
	{
	return done;
	}

void ImageReaderPNM::readNative(ImageReader::ImagePlane imagePlanes[])
	{
	switch(imageType)
		{
		case '1': // ASCII bitmap
			{
			/* Create a value source to parse ASCII pixel values: */
			IO::ValueSource image(file);
			image.skipWs();
			
			/* Read the image one row at a time: */
			Misc::UInt8* rowPtr=static_cast<Misc::UInt8*>(imagePlanes[0].basePtr);
			for(unsigned int y=0;y<imageSpec.size[1];++y,rowPtr+=imagePlanes[0].rowStride)
				{
				/* Read the row one pixel at a time: */
				Misc::UInt8* pPtr=rowPtr;
				for(unsigned int x=0;x<imageSpec.size[0];++x,pPtr+=imagePlanes[0].pixelStride)
					;
				}
			
			break;
			}
		
		case '2': // ASCII grayscale image
		case '3': // ASCII RGB image
			{
			/* Create a value source to parse ASCII pixel values: */
			IO::ValueSource image(file);
			image.skipWs();
			
			/* Determine the native pixel size: */
			if(maxValue<256U)
				{
				/* Read 8-bit pixels: */
				readASCII<Misc::UInt8>(image,imageSpec,imagePlanes);
				}
			else
				{
				/* Read 16-bit pixels: */
				readASCII<Misc::UInt16>(image,imageSpec,imagePlanes);
				}
			break;
			}
		
		case '4': // Binary bitmap
			{
			/* Read the image one row at a time: */
			Misc::UInt8* rowPtr=static_cast<Misc::UInt8*>(imagePlanes[0].basePtr);
			for(unsigned int y=0;y<imageSpec.size[1];++y,rowPtr+=imagePlanes[0].rowStride)
				{
				/* Read the row one pixel at a time: */
				Misc::UInt8* pPtr=rowPtr;
				for(unsigned int x=0;x<imageSpec.size[0];++x,pPtr+=imagePlanes[0].pixelStride)
					;
				}
			break;
			}
		
		case '5': // Binary grayscale image
		case '6': // Binary RGB image
			{
			/* Determine the native pixel size: */
			if(maxValue<256U)
				{
				/* Read 8-bit pixels: */
				readBinary<Misc::UInt8>(file,imageSpec,imagePlanes);
				}
			else
				{
				/* Read 16-bit pixels: */
				readBinary<Misc::UInt16>(file,imageSpec,imagePlanes);
				}
			break;
			}
		
		}
	
	/* There can be only one image in a PNM file: */
	done=true;
	}

}
