/***********************************************************************
WriteImageFile - Functions to write RGB images to a variety of file
formats.
Copyright (c) 2006-2013 Oliver Kreylos

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

#include <Images/ReadImageFile.h>

#include <Images/Config.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#if IMAGES_CONFIG_HAVE_PNG
#include <png.h>
#endif
#if IMAGES_CONFIG_HAVE_TIFF
#include <tiffio.h>
#endif
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>

namespace Images {

namespace {

/****************************************
Function to write binary PNM image files:
****************************************/

void writePnmFile(unsigned int width,unsigned int height,const unsigned char* image,const char* imageFileName)
	{
	/* Open the PNM file: */
	Misc::File pnmFile(imageFileName,"wb",Misc::File::DontCare);
	
	/* Write the PNM header: */
	fprintf(pnmFile.getFilePtr(),"P6\n");
	fprintf(pnmFile.getFilePtr(),"%d %d\n",width,height);
	fprintf(pnmFile.getFilePtr(),"255\n");
	
	/* Write each row of the image file: */
	for(unsigned int row=height;row>0;--row)
		pnmFile.write(image+(row-1)*width*3,width*3);
	}

#if IMAGES_CONFIG_HAVE_PNG

/*********************************
Function to write PNG image files:
*********************************/

void writePngFile(unsigned int width,unsigned int height,const unsigned char* image,const char* imageFileName)
	{
	/* Open output file: */
	Misc::File pngFile(imageFileName,"wb",Misc::File::DontCare);
	
	/* Allocate the PNG library data structures: */
	png_structp pngWriteStruct=png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
	if(pngWriteStruct==0)
		Misc::throwStdErr("Images::writePngFile: Internal error in PNG library");
	png_infop pngInfoStruct=png_create_info_struct(pngWriteStruct);
	if(pngInfoStruct==0)
		{
		png_destroy_write_struct(&pngWriteStruct,0);
		Misc::throwStdErr("Images::writePngFile: Internal error in PNG library");
		}
	
	/* Set up longjump facility for PNG error handling (ouch): */
	if(setjmp(png_jmpbuf(pngWriteStruct)))
		{
		png_destroy_write_struct(&pngWriteStruct,&pngInfoStruct);
		Misc::throwStdErr("Images::writePngFile: Error while setting up PNG library error handling");
		}
	
	/* Initialize PNG I/O: */
	png_init_io(pngWriteStruct,pngFile.getFilePtr());
	
	/* Set and write PNG image header: */
	png_set_IHDR(pngWriteStruct,pngInfoStruct,width,height,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	png_write_info(pngWriteStruct,pngInfoStruct);
	
	/* Write all image rows in reverse order: */
	for(unsigned int row=height;row>0;--row)
		png_write_row(pngWriteStruct,const_cast<png_byte*>(reinterpret_cast<const png_byte*>(image+(row-1)*width*3)));
	
	/* Finish writing image: */
	png_write_end(pngWriteStruct,0);
	
	/* Clean up and return: */
	png_destroy_write_struct(&pngWriteStruct,&pngInfoStruct);
	}

#endif

#if IMAGES_CONFIG_HAVE_TIFF

/**********************************
Function to write TIFF image files:
**********************************/

void tiffErrorFunction(const char* module,const char* fmt,va_list ap)
	{
	/* Throw an exception with the error message: */
	char msg[1024];
	vsnprintf(msg,sizeof(msg),fmt,ap);
	throw std::runtime_error(msg);
	}

void tiffWarningFunction(const char* module,const char* fmt,va_list ap)
	{
	/* Ignore warnings */
	}

void writeTiffFile(unsigned int width,unsigned int height,const unsigned char* image,const char* imageFileName)
	{
	/* Set the TIFF error handler: */
	TIFFSetErrorHandler(tiffErrorFunction);
	TIFFSetWarningHandler(tiffWarningFunction);
	
	TIFF* tiff=0;
	try
		{
		/* Open the TIFF file: */
		tiff=TIFFOpen(imageFileName,"w");
		if(tiff==0)
			throw std::runtime_error("Error while opening image file");
		
		/* Write the TIFF image layout tags: */
		TIFFSetField(tiff,TIFFTAG_IMAGEWIDTH,width);
		TIFFSetField(tiff,TIFFTAG_IMAGELENGTH,height);
		TIFFSetField(tiff,TIFFTAG_BITSPERSAMPLE,8);
		TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,3);
		TIFFSetField(tiff,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
		TIFFSetField(tiff,TIFFTAG_COMPRESSION,COMPRESSION_NONE);
		TIFFSetField(tiff,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_RGB);
		TIFFSetField(tiff,TIFFTAG_ROWSPERSTRIP,TIFFDefaultStripSize(tiff,width*3));
		
		/* Write the image data to the TIFF file one scan line at a time, top-to-bottom: */
		for(unsigned int row=height;row>0;--row)
			if(TIFFWriteScanline(tiff,reinterpret_cast<tdata_t>(const_cast<unsigned char*>(image+(row-1)*width*3)),height-row)<0)
				throw std::runtime_error("Error while writing image");
		
		/* Close the TIFF file: */
		TIFFClose(tiff);
		}
	catch(std::runtime_error err)
		{
		/* Clean up: */
		if(tiff!=0)
			TIFFClose(tiff);
		
		/* Wrap and re-throw the exception: */
		Misc::throwStdErr("Images::writeTiffFile: Caught exception \"%s\" while writing image \"%s\"",err.what(),imageFileName);
		}
	}

#endif

}

/***********************************************************
Function to write images files in several supported formats:
***********************************************************/

void writeImageFile(const RGBImage& image,const char* imageFileName)
	{
	/* Try to determine image file format from file name extension: */
	
	/* Find position of last dot in image file name: */
	const char* extStart=0;
	const char* cPtr;
	for(cPtr=imageFileName;*cPtr!='\0';++cPtr)
		if(*cPtr=='.')
			extStart=cPtr+1;
	if(extStart==0)
		Misc::throwStdErr("Images::writeImageFile: no extension in image file name \"%s\"",imageFileName);
	
	if(cPtr-extStart==3&&tolower(extStart[0])=='p'&&tolower(extStart[2])=='m'&&
	   (tolower(extStart[1])=='b'||tolower(extStart[1])=='g'||tolower(extStart[1])=='n'||tolower(extStart[1])=='p'))
		writePnmFile(image.getWidth(),image.getHeight(),image.getPixels()[0].getRgba(),imageFileName);
	#if IMAGES_CONFIG_HAVE_PNG
	else if(strcasecmp(extStart,"png")==0)
		writePngFile(image.getWidth(),image.getHeight(),image.getPixels()[0].getRgba(),imageFileName);
	#endif
	#if IMAGES_CONFIG_HAVE_TIFF
	else if(strcasecmp(extStart,"tif")==0||strcasecmp(extStart,"tiff")==0)
		writeTiffFile(image.getWidth(),image.getHeight(),image.getPixels()[0].getRgba(),imageFileName);
	#endif
	else
		Misc::throwStdErr("Images::writeImageFile: unknown extension in image file name \"%s\"",imageFileName);
	}

void writeImageFile(unsigned int width,unsigned int height,const unsigned char* image,const char* imageFileName)
	{
	/* Try to determine image file format from file name extension: */
	
	/* Find position of last dot in image file name: */
	const char* extStart=0;
	const char* cPtr;
	for(cPtr=imageFileName;*cPtr!='\0';++cPtr)
		if(*cPtr=='.')
			extStart=cPtr+1;
	if(extStart==0)
		Misc::throwStdErr("Images::writeImageFile: no extension in image file name \"%s\"",imageFileName);
	
	if(cPtr-extStart==3&&tolower(extStart[0])=='p'&&tolower(extStart[2])=='m'&&
	   (tolower(extStart[1])=='b'||tolower(extStart[1])=='g'||tolower(extStart[1])=='n'||tolower(extStart[1])=='p'))
		writePnmFile(width,height,image,imageFileName);
	#if IMAGES_CONFIG_HAVE_PNG
	else if(strcasecmp(extStart,"png")==0)
		writePngFile(width,height,image,imageFileName);
	#endif
	#if IMAGES_CONFIG_HAVE_TIFF
	else if(strcasecmp(extStart,"tif")==0||strcasecmp(extStart,"tiff")==0)
		writeTiffFile(width,height,image,imageFileName);
	#endif
	else
		Misc::throwStdErr("Images::writeImageFile: unknown extension in image file name \"%s\"",imageFileName);
	}

}
