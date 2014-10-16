/***********************************************************************
ReadPNMImage - Functions to read RGB images from image files in PNM
(Portable AnyMap) formats over an IO::File abstraction.
Copyright (c) 2011 Oliver Kreylos

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

#include <Images/ReadPNMImage.h>

#include <stdexcept>
#include <Misc/SizedTypes.h>
#include <Misc/SelfDestructPointer.h>
#include <Misc/SelfDestructArray.h>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>
#include <IO/ValueSource.h>

namespace Images {

namespace {

/***********************************
Helper functions to read PNM images:
***********************************/

inline void skipComments(IO::ValueSource& header)
	{
	while(header.peekc()=='#')
		{
		header.skipLine();
		header.skipWs();
		}
	}

}

RGBImage readPNMImage(const char* imageName,IO::File& source)
	{
	RGBImage result;
	try
		{
		/* Parse the file's header: */
		Misc::SelfDestructPointer<IO::ValueSource> header(new IO::ValueSource(&source));
		header->skipWs();
		
		/* Read the magic field including the image type indicator: */
		int magic=header->getChar();
		int imageType=header->getChar();
		if(magic!='P'||imageType<'1'||imageType>'6')
			throw std::runtime_error("Invalid PNM header");
		header->skipWs();
		skipComments(*header);
		
		/* Read the image width, height, and maximal pixel component value: */
		unsigned int width,height,maxValue;
		width=header->readUnsignedInteger();
		skipComments(*header);
		if(imageType=='1'||imageType=='4') // PBM files don't have the maxValue field
			{
			header->setWhitespace(""); // Disable all whitespace to read the last header field
			height=header->readUnsignedInteger();
			maxValue=1;
			}
		else
			{
			height=header->readUnsignedInteger();
			skipComments(*header);
			header->setWhitespace(""); // Disable all whitespace to read the last header field
			maxValue=header->readUnsignedInteger();
			}
		
		/* Read the separating whitespace character: */
		header->getChar();
		
		/* Delete the header parser if the rest of the file is in binary; otherwise re-enable whitespace: */
		if(imageType>='4')
			{
			header.setTarget(0);
			source.setEndianness(Misc::BigEndian);
			}
		else
			header->resetCharacterClasses();
		
		/* Read the image: */
		result=RGBImage(width,height);
		switch(imageType)
			{
			case '1': // ASCII bitmap image
				{
				/* Read each row of the image file: */
				for(unsigned int y=height;y>0;--y)
					{
					RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
					for(unsigned int x=0;x<width;++x,++rowPtr)
						(*rowPtr)[0]=(*rowPtr)[1]=(*rowPtr)[2]=RGBImage::Color::Scalar(header->readUnsignedInteger()!=0U?255U:0U);
					}
				
				break;
				}
			
			case '2': // ASCII greyscale image
				{
				/* Read each row of the image file: */
				for(unsigned int y=height;y>0;--y)
					{
					RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
					for(unsigned int x=0;x<width;++x,++rowPtr)
						(*rowPtr)[0]=(*rowPtr)[1]=(*rowPtr)[2]=RGBImage::Color::Scalar((header->readUnsignedInteger()*256U)/(maxValue+1));
					}
				
				break;
				}
			
			case '3': // ASCII RGB color image
				{
				/* Read each row of the image file: */
				for(unsigned int y=height;y>0;--y)
					{
					RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
					for(unsigned int x=0;x<width;++x,++rowPtr)
						for(int i=0;i<3;++i)
							(*rowPtr)[i]=RGBImage::Color::Scalar((header->readUnsignedInteger()*256U)/(maxValue+1));
					}
				
				break;
				}
			
			case '4': // Binary bitmap image
				{
				/* Allocate a row buffer: */
				unsigned int rawWidth=(width+7)>>3;
				Misc::SelfDestructArray<Misc::UInt8> tempRow(rawWidth);
				
				/* Read each row of the image file: */
				for(unsigned int y=height;y>0;--y)
					{
					RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
					source.read(tempRow.getArray(),rawWidth);
					
					/* Convert pixel values: */
					Misc::UInt8* tempRowPtr=tempRow;
					for(unsigned int x=0;x<width;++tempRowPtr)
						for(Misc::UInt8 mask=0x80U;mask!=0x0U&&x<width;mask>>=1,++x,++rowPtr)
							(*rowPtr)[0]=(*rowPtr)[1]=(*rowPtr)[2]=RGBImage::Color::Scalar(((*tempRowPtr)&mask)!=0x0U?255U:0U);
					}
				
				break;
				}
			
			case '5': // Binary greyscale image
				{
				/* Check if the pixel values are 8 or 16 bit: */
				if(maxValue<256) // 8-bit pixel values
					{
					/* Allocate a row buffer: */
					Misc::SelfDestructArray<Misc::UInt8> tempRow(width);
					
					/* Read each row of the image file: */
					for(unsigned int y=height;y>0;--y)
						{
						/* Read the row of raw pixel values from the source: */
						source.read(tempRow.getArray(),width);
						
						/* Convert pixel values: */
						RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
						Misc::UInt8* tempRowPtr=tempRow;
						for(unsigned int x=0;x<width;++x,++rowPtr,++tempRowPtr)
							(*rowPtr)[0]=(*rowPtr)[1]=(*rowPtr)[2]=RGBImage::Scalar(*tempRowPtr);
						}
					}
				else // 16-bit pixel values
					{
					/* Allocate a row buffer: */
					Misc::SelfDestructArray<Misc::UInt16> tempRow(width);
					
					/* Read each row of the image file: */
					for(unsigned int y=height;y>0;--y)
						{
						/* Read the row of raw pixel values from the source: */
						source.read(tempRow.getArray(),width);
						
						/* Convert pixel values: */
						RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
						Misc::UInt16* tempRowPtr=tempRow;
						for(unsigned int x=0;x<width;++x,++rowPtr,++tempRowPtr)
							(*rowPtr)[0]=(*rowPtr)[1]=(*rowPtr)[2]=RGBImage::Scalar(((unsigned int)(*tempRowPtr)*256U)/(maxValue+1));
						}
					}
				
				break;
				}
			
			case '6': // Binary RGB color image
				{
				/* Check if the pixel values are 8 or 16 bit: */
				if(maxValue<256) // 8-bit pixel values
					{
					/* Read each row of the image file: */
					for(unsigned int y=height;y>0;--y)
						{
						/* Read the row of raw pixel values from the source: */
						source.read(result.modifyPixelRow(y-1)->getRgba(),width*3);
						}
					}
				else // 16-bit pixel values
					{
					/* Allocate a row buffer: */
					Misc::SelfDestructArray<Misc::UInt16> tempRow(width*3);
					
					/* Read each row of the image file: */
					for(unsigned int y=height;y>0;--y)
						{
						/* Read the row of raw pixel values from the source: */
						source.read(tempRow.getArray(),width*3);
						
						/* Convert pixel values: */
						RGBImage::Color* rowPtr=result.modifyPixelRow(y-1);
						Misc::UInt16* tempRowPtr=tempRow;
						for(unsigned int x=0;x<width;++x,++rowPtr,tempRowPtr+=3)
							for(int i=0;i<3;++i)
								(*rowPtr)[i]=RGBImage::Scalar(((unsigned int)(tempRowPtr[i])*256U)/(maxValue+1));
						}
					}
				
				break;
				}
			}
		}
	catch(std::runtime_error err)
		{
		/* Wrap and re-throw the exception: */
		Misc::throwStdErr("Images::readPNMImage: Caught exception \"%s\" while reading image \"%s\"",err.what(),imageName);
		}
	
	/* Return the result image: */
	return result;
	}

}
