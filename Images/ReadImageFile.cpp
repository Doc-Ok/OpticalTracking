/***********************************************************************
ReadImageFile - Functions to read RGB images from a variety of file
formats.
Copyright (c) 2005-2011 Oliver Kreylos

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
#include <string.h>
#include <Misc/Utility.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FileNameExtensions.h>
#include <IO/SeekableFile.h>
#include <IO/OpenFile.h>
#include <Images/ReadPNMImage.h>
#include <Images/ReadPNGImage.h>
#include <Images/ReadJPEGImage.h>
#include <Images/ReadTIFFImage.h>

namespace Images {

/***************************************************************
Function to check whether the image file has a supported format:
***************************************************************/

bool canReadImageFileType(const char* imageFileName)
	{
	/* Try to determine image file format from file name extension: */
	const char* ext=Misc::getExtension(imageFileName);
	int extLen=strlen(ext);
	if(strcasecmp(ext,".gz")==0)
		{
		/* Strip the gzip extension and try again: */
		const char* gzExt=ext;
		ext=Misc::getExtension(imageFileName,gzExt);
		extLen=gzExt-ext;
		}
	
	if(extLen==4
	   &&ext[0]=='.'
	   &&tolower(ext[1])=='p'
	   &&(tolower(ext[2])=='b'
	      ||tolower(ext[2])=='g'
	      ||tolower(ext[2])=='n'
	      ||tolower(ext[2])=='p')
	   &&tolower(ext[3])=='m') // It's a Portable AnyMap image
		return true;
	#if IMAGES_CONFIG_HAVE_PNG
	else if(strncasecmp(ext,".png",extLen)==0) // It's a PNG image
		return true;
	#endif
	#if IMAGES_CONFIG_HAVE_JPEG
	else if(strncasecmp(ext,".jpg",extLen)==0||strncasecmp(ext,".jpeg",extLen)==0) // It's a JPEG image
		return true;
	#endif
	#if IMAGES_CONFIG_HAVE_TIFF
	else if(strncasecmp(ext,".tif",extLen)==0||strncasecmp(ext,".tiff",extLen)==0) // It's a TIFF image
		return true;
	#endif
	else
		return false;
	}

/**********************************************************
Function to read images files in several supported formats:
**********************************************************/

RGBImage readImageFile(const char* imageFileName,IO::FilePtr file)
	{
	RGBImage result;
	
	/* Try to determine image file format from file name extension: */
	const char* ext=Misc::getExtension(imageFileName);
	int extLen=strlen(ext);
	if(strcasecmp(ext,".gz")==0)
		{
		/* Strip the gzip extension and try again: */
		const char* gzExt=ext;
		ext=Misc::getExtension(imageFileName,gzExt);
		extLen=gzExt-ext;
		}
	
	if(extLen==4
	   &&ext[0]=='.'
	   &&tolower(ext[1])=='p'
	   &&(tolower(ext[2])=='b'
	      ||tolower(ext[2])=='g'
	      ||tolower(ext[2])=='n'
	      ||tolower(ext[2])=='p')
	   &&tolower(ext[3])=='m') // It's a Portable AnyMap image
		{
		/* Read a PNM image from the given file: */
		result=readPNMImage(imageFileName,*file);
		}
	#if IMAGES_CONFIG_HAVE_PNG
	else if(strncasecmp(ext,".png",extLen)==0) // It's a PNG image
		{
		/* Read a PNG image from the given file: */
		result=readPNGImage(imageFileName,*file);
		}
	#endif
	#if IMAGES_CONFIG_HAVE_JPEG
	else if(strncasecmp(ext,".jpg",extLen)==0||strncasecmp(ext,".jpeg",extLen)==0) // It's a JPEG image
		{
		/* Read a JPEG image from the given file: */
		result=readJPEGImage(imageFileName,*file);
		}
	#endif
	#if IMAGES_CONFIG_HAVE_TIFF
	else if(strncasecmp(ext,".tif",extLen)==0||strncasecmp(ext,".tiff",extLen)==0) // It's a TIFF image
		{
		/* Read a TIFF image from the given file: */
		result=readTIFFImage(imageFileName,*file);
		}
	#endif
	else
		Misc::throwStdErr("Images::readImageFile: Unknown extension in image file name \"%s\"",imageFileName);
	
	return result;
	}

RGBImage readImageFile(const char* imageFileName)
	{
	/* Open the image file: */
	IO::FilePtr file(IO::openFile(imageFileName));
	
	/* Call the general function: */
	return readImageFile(imageFileName,file);
	}

RGBAImage readTransparentImageFile(const char* imageFileName,IO::FilePtr file)
	{
	RGBAImage result;
	
	/* Try to determine image file format from file name extension: */
	const char* ext=Misc::getExtension(imageFileName);
	if(strcasecmp(ext,".gz")==0)
		{
		/* Strip the gzip extension and try again: */
		ext=Misc::getExtension(imageFileName,ext);
		}
	
	if(ext[0]=='.'
	   &&tolower(ext[1])=='p'
	   &&(tolower(ext[2])=='b'
	      ||tolower(ext[2])=='g'
	      ||tolower(ext[2])=='n'
	      ||tolower(ext[2])=='p')
	   &&tolower(ext[3])=='m') // It's a Portable AnyMap image
		{
		/* Read a PNM image from the given file and add an alpha channel automatically: */
		result=readPNMImage(imageFileName,*file);
		}
	#if IMAGES_CONFIG_HAVE_PNG
	else if(strcasecmp(ext,".png")==0) // It's a PNG image
		{
		/* Read a transparent PNG image from the given file: */
		result=readTransparentPNGImage(imageFileName,*file);
		}
	#endif
	#if IMAGES_CONFIG_HAVE_JPEG
	else if(strcasecmp(ext,".jpg")==0||strcasecmp(ext,".jpeg")==0) // It's a JPEG image
		{
		/* Read a JPEG image from the given file and add an alpha channel automatically: */
		result=readJPEGImage(imageFileName,*file);
		}
	#endif
	#if IMAGES_CONFIG_HAVE_TIFF
	else if(strcasecmp(ext,".tif")==0||strcasecmp(ext,".tiff")==0) // It's a TIFF image
		{
		/* Read a transparent TIFF image from the given file: */
		result=readTIFFImage(imageFileName,*file);
		}
	#endif
	else
		Misc::throwStdErr("Images::readTransparentImageFile: Unknown extension in image file name \"%s\"",imageFileName);
	
	return result;
	}

RGBAImage readTransparentImageFile(const char* imageFileName)
	{
	/* Open the image file: */
	IO::FilePtr file(IO::openFile(imageFileName));
	
	/* Call the general function: */
	return readTransparentImageFile(imageFileName,file);
	}

namespace {

/********************************************
Helper structures for the cursor file reader:
********************************************/

struct CursorFileHeader
	{
	/* Elements: */
	public:
	unsigned int magic;
	unsigned int headerSize;
	unsigned int version;
	unsigned int numTOCEntries;
	};

struct CursorTOCEntry
	{
	/* Elements: */
	public:
	unsigned int chunkType;
	unsigned int chunkSubtype;
	unsigned int chunkPosition;
	};

struct CursorCommentChunkHeader
	{
	/* Elements: */
	public:
	unsigned int headerSize;
	unsigned int chunkType; // 0xfffe0001U
	unsigned int chunkSubtype;
	unsigned int version;
	unsigned int commentLength;
	/* Comment characters follow */
	};

struct CursorImageChunkHeader
	{
	/* Elements: */
	public:
	unsigned int headerSize;
	unsigned int chunkType; // 0xfffd0002U
	unsigned int chunkSubtype;
	unsigned int version;
	unsigned int size[2];
	unsigned int hotspot[2];
	unsigned int delay;
	/* ARGB pixel data follows */
	};

}

/***********************************************
Function to read cursor files in Xcursor format:
***********************************************/

RGBAImage readCursorFile(const char* cursorFileName,IO::FilePtr file,unsigned int nominalSize,unsigned int* hotspot)
	{
	/* Check if the file is seekable: */
	IO::SeekableFilePtr seekableFile=file;
	if(seekableFile==0)
		Misc::throwStdErr("Images::readCursorFile: Cannot read cursor from non-seekable file \"%s\"",cursorFileName);
	
	/* Read the magic value to determine file endianness: */
	CursorFileHeader fh;
	fh.magic=seekableFile->read<unsigned int>();
	if(fh.magic==0x58637572U)
		seekableFile->setSwapOnRead(true);
	else if(fh.magic!=0x72756358U)
		Misc::throwStdErr("Images::readCursorFile: Invalid cursor file header in \"%s\"",cursorFileName);
	
	/* Read the rest of the file header: */
	fh.headerSize=seekableFile->read<unsigned int>();
	fh.version=seekableFile->read<unsigned int>();
	fh.numTOCEntries=seekableFile->read<unsigned int>();
	
	/* Read the table of contents: */
	IO::SeekableFile::Offset imageChunkOffset=0;
	for(unsigned int i=0;i<fh.numTOCEntries;++i)
		{
		CursorTOCEntry te;
		te.chunkType=seekableFile->read<unsigned int>();
		te.chunkSubtype=seekableFile->read<unsigned int>();
		te.chunkPosition=seekableFile->read<unsigned int>();
		
		if(te.chunkType==0xfffd0002U&&te.chunkSubtype==nominalSize)
			{
			imageChunkOffset=IO::SeekableFile::Offset(te.chunkPosition);
			break;
			}
		}
	
	if(imageChunkOffset==0)
		Misc::throwStdErr("Images::readCursorFile: No matching image found in \"%s\"",cursorFileName);
	
	/* Read the image chunk: */
	seekableFile->setReadPosAbs(imageChunkOffset);
	CursorImageChunkHeader ich;
	ich.headerSize=seekableFile->read<unsigned int>();
	ich.chunkType=seekableFile->read<unsigned int>();
	ich.chunkSubtype=seekableFile->read<unsigned int>();
	ich.version=seekableFile->read<unsigned int>();
	for(int i=0;i<2;++i)
		ich.size[i]=seekableFile->read<unsigned int>();
	for(int i=0;i<2;++i)
		ich.hotspot[i]=seekableFile->read<unsigned int>();
	if(hotspot!=0)
		{
		for(int i=0;i<2;++i)
			hotspot[i]=ich.hotspot[i];
		}
	ich.delay=seekableFile->read<unsigned int>();
	if(ich.headerSize!=9*sizeof(unsigned int)||ich.chunkType!=0xfffd0002U||ich.version!=1)
		Misc::throwStdErr("Images::readCursorFile: Invalid image chunk header in \"%s\"",cursorFileName);
	
	/* Create the result image: */
	RGBAImage result(ich.size[0],ich.size[1]);
	
	/* Read the image row-by-row: */
	for(unsigned int row=result.getHeight();row>0;--row)
		{
		RGBAImage::Color* rowPtr=result.modifyPixelRow(row-1);
		seekableFile->read(rowPtr->getRgba(),result.getWidth()*4);
		
		/* Convert BGRA data into RGBA data: */
		for(unsigned int i=0;i<result.getWidth();++i)
			Misc::swap(rowPtr[i][0],rowPtr[i][2]);
		}
	
	/* Return the result image: */
	return result;
	}

RGBAImage readCursorFile(const char* cursorFileName,unsigned int nominalSize,unsigned int* hotspot)
	{
	/* Open the cursor file: */
	IO::FilePtr file(IO::openFile(cursorFileName));
	
	/* Call the general method: */
	return readCursorFile(cursorFileName,file,nominalSize,hotspot);
	}

}
