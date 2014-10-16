/***********************************************************************
ReadPNGImage - Functions to read RGB or RGBA images from image files in
PNG formats over an IO::File abstraction.
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

#include <Images/ReadPNGImage.h>

#include <Images/Config.h>

#if IMAGES_CONFIG_HAVE_PNG

#include <png.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>

namespace Images {

namespace {

/***************************************************************
Helper functions to redirect PNG I/O to an IO::File data source:
***************************************************************/

void pngReadDataFunction(png_structp pngReadStruct,png_bytep buffer,png_size_t size)
	{
	/* Get the pointer to the IO::File object: */
	IO::File* source=static_cast<IO::File*>(png_get_io_ptr(pngReadStruct));
	
	/* Read the requested number of bytes from the source, and let the source handle errors: */
	source->read(buffer,size);
	}

void pngErrorFunction(png_structp pngReadStruct,png_const_charp errorMsg)
	{
	/* Throw an exception: */
	throw std::runtime_error(errorMsg);
	}

void pngWarningFunction(png_structp pngReadStruct,png_const_charp warningMsg)
	{
	/* Ignore warnings */
	}

}

RGBImage readPNGImage(const char* imageName,IO::File& source)
	{
	/* Check for PNG file signature: */
	unsigned char pngSignature[8];
	source.read(pngSignature,8);
	if(!png_check_sig(pngSignature,8))
		Misc::throwStdErr("Images::readPNGImage: illegal PNG header in image \"%s\"",imageName);
	
	/* Allocate the PNG library data structures: */
	png_structp pngReadStruct=png_create_read_struct(PNG_LIBPNG_VER_STRING,0,pngErrorFunction,pngWarningFunction);
	if(pngReadStruct==0)
		Misc::throwStdErr("Images::readPNGImage: Internal error in PNG library");
	png_infop pngInfoStruct=png_create_info_struct(pngReadStruct);
	if(pngInfoStruct==0)
		{
		png_destroy_read_struct(&pngReadStruct,0,0);
		Misc::throwStdErr("Images::readPNGImage: Internal error in PNG library");
		}
	
	/* Initialize PNG I/O to read from the supplied data source: */
	png_set_read_fn(pngReadStruct,&source,pngReadDataFunction);
	
	RGBImage result;
	RGBImage::Color** rowPointers=0;
	try
		{
		/* Read PNG image header: */
		png_set_sig_bytes(pngReadStruct,8);
		png_read_info(pngReadStruct,pngInfoStruct);
		png_uint_32 imageSize[2];
		int elementSize;
		int colorType;
		png_get_IHDR(pngReadStruct,pngInfoStruct,&imageSize[0],&imageSize[1],&elementSize,&colorType,0,0,0);
		
		/* Set up image processing: */
		if(colorType==PNG_COLOR_TYPE_PALETTE)
			png_set_expand(pngReadStruct);
		else if(colorType==PNG_COLOR_TYPE_GRAY&&elementSize<8)
			png_set_expand(pngReadStruct);
		if(elementSize==16)
			png_set_strip_16(pngReadStruct);
		if(colorType==PNG_COLOR_TYPE_GRAY||colorType==PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(pngReadStruct);
		if(colorType==PNG_COLOR_TYPE_GRAY_ALPHA||colorType==PNG_COLOR_TYPE_RGB_ALPHA)
			png_set_strip_alpha(pngReadStruct);
		double gamma;
		if(png_get_gAMA(pngReadStruct,pngInfoStruct,&gamma))
			png_set_gamma(pngReadStruct,2.2,gamma);
		png_read_update_info(pngReadStruct,pngInfoStruct);
		
		/* Initialize the result image: */
		result=RGBImage(imageSize[0],imageSize[1]);
		
		/* Create row pointers to flip the image during reading: */
		rowPointers=new RGBImage::Color*[result.getHeight()];
		for(unsigned int y=0;y<result.getHeight();++y)
			rowPointers[y]=result.modifyPixelRow(result.getHeight()-1-y);
		
		/* Read the PNG image: */
		png_read_image(pngReadStruct,reinterpret_cast<png_byte**>(rowPointers));
		
		/* Finish reading image: */
		png_read_end(pngReadStruct,0);
		}
	catch(std::runtime_error err)
		{
		/* Clean up: */
		delete[] rowPointers;
		png_destroy_read_struct(&pngReadStruct,&pngInfoStruct,0);
		
		/* Wrap and re-throw the exception: */
		Misc::throwStdErr("Images::readPNGImage: Caught exception \"%s\" while reading image \"%s\"",err.what(),imageName);
		}
	
	/* Clean up: */
	delete[] rowPointers;
	png_destroy_read_struct(&pngReadStruct,&pngInfoStruct,0);
	
	/* Return the read image: */
	return result;
	}

RGBAImage readTransparentPNGImage(const char* imageName,IO::File& source)
	{
	/* Check for PNG file signature: */
	unsigned char pngSignature[8];
	source.read(pngSignature,8);
	if(!png_check_sig(pngSignature,8))
		Misc::throwStdErr("Images::readTransparentPNGImage: illegal PNG header in image \"%s\"",imageName);
	
	/* Allocate the PNG library data structures: */
	png_structp pngReadStruct=png_create_read_struct(PNG_LIBPNG_VER_STRING,0,pngErrorFunction,pngWarningFunction);
	if(pngReadStruct==0)
		Misc::throwStdErr("Images::readTransparentPNGImage: Internal error in PNG library");
	png_infop pngInfoStruct=png_create_info_struct(pngReadStruct);
	if(pngInfoStruct==0)
		{
		png_destroy_read_struct(&pngReadStruct,0,0);
		Misc::throwStdErr("Images::readTransparentPNGImage: Internal error in PNG library");
		}
	
	/* Initialize PNG I/O to read from the supplied data source: */
	png_set_read_fn(pngReadStruct,&source,pngReadDataFunction);
	
	RGBAImage result;
	RGBAImage::Color** rowPointers=0;
	try
		{
		/* Read PNG image header: */
		png_set_sig_bytes(pngReadStruct,8);
		png_read_info(pngReadStruct,pngInfoStruct);
		png_uint_32 imageSize[2];
		int elementSize;
		int colorType;
		png_get_IHDR(pngReadStruct,pngInfoStruct,&imageSize[0],&imageSize[1],&elementSize,&colorType,0,0,0);
		
		/* Set up image processing: */
		if(colorType==PNG_COLOR_TYPE_PALETTE)
			png_set_expand(pngReadStruct);
		else if(colorType==PNG_COLOR_TYPE_GRAY&&elementSize<8)
			png_set_expand(pngReadStruct);
		if(elementSize==16)
			png_set_strip_16(pngReadStruct);
		if(colorType==PNG_COLOR_TYPE_GRAY||colorType==PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(pngReadStruct);
		if(colorType==PNG_COLOR_TYPE_GRAY||colorType==PNG_COLOR_TYPE_RGB)
			png_set_filler(pngReadStruct,0xff,PNG_FILLER_AFTER);
		double gamma;
		if(png_get_gAMA(pngReadStruct,pngInfoStruct,&gamma))
			png_set_gamma(pngReadStruct,2.2,gamma);
		png_read_update_info(pngReadStruct,pngInfoStruct);
		
		/* Initialize the result image: */
		result=RGBAImage(imageSize[0],imageSize[1]);
		
		/* Create row pointers to flip the image during reading: */
		rowPointers=new RGBAImage::Color*[result.getHeight()];
		for(unsigned int y=0;y<result.getHeight();++y)
			rowPointers[y]=result.modifyPixelRow(result.getHeight()-1-y);
		
		/* Read the PNG image: */
		png_read_image(pngReadStruct,reinterpret_cast<png_byte**>(rowPointers));
		
		/* Finish reading image: */
		png_read_end(pngReadStruct,0);
		}
	catch(std::runtime_error err)
		{
		/* Clean up: */
		delete[] rowPointers;
		png_destroy_read_struct(&pngReadStruct,&pngInfoStruct,0);
		
		/* Wrap and re-throw the exception: */
		Misc::throwStdErr("Images::readTransparentPNGImage: Caught exception \"%s\" while reading image \"%s\"",err.what(),imageName);
		}
	
	/* Clean up: */
	delete[] rowPointers;
	png_destroy_read_struct(&pngReadStruct,&pngInfoStruct,0);
	
	/* Return the read image: */
	return result;
	}

}

#endif
