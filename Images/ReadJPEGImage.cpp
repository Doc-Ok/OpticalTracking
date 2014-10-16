/***********************************************************************
ReadJPEGImage - Functions to read RGB images from image files in JPEG
formats over an IO::File abstraction.
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

#include <Images/ReadJPEGImage.h>

#include <Images/Config.h>

#if IMAGES_CONFIG_HAVE_JPEG

#include <stdio.h>
#include <jpeglib.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>

namespace Images {

namespace {

class JPEGExceptionErrorManager:public jpeg_error_mgr
	{
	/* Private methods: */
	private:
	static void errorExitFunction(j_common_ptr cinfo)
		{
		JPEGExceptionErrorManager* thisPtr=static_cast<JPEGExceptionErrorManager*>(cinfo->err);
		
		/* Throw an exception: */
		Misc::throwStdErr(thisPtr->jpeg_message_table[thisPtr->msg_code],thisPtr->msg_parm.i[0],thisPtr->msg_parm.i[1],thisPtr->msg_parm.i[2],thisPtr->msg_parm.i[3],thisPtr->msg_parm.i[4],thisPtr->msg_parm.i[5],thisPtr->msg_parm.i[6],thisPtr->msg_parm.i[7]);
		}
	
	/* Constructors and destructors: */
	public:
	JPEGExceptionErrorManager(void)
		{
		/* Set the method pointer(s) in the base class object: */
		jpeg_std_error(this);
		error_exit=errorExitFunction;
		}
	};

class JPEGFileSourceManager:public jpeg_source_mgr
	{
	/* Methods: */
	private:
	IO::File& source; // Reference to the source stream
	
	/* Private methods: */
	static void initSourceFunction(j_decompress_ptr cinfo)
		{
		// JPEGFileSourceManager* thisPtr=static_cast<JPEGFileSourceManager*>(cinfo->src);
		}
	static boolean fillInputBufferFunction(j_decompress_ptr cinfo)
		{
		JPEGFileSourceManager* thisPtr=static_cast<JPEGFileSourceManager*>(cinfo->src);
		
		/* Fill the JPEG decoder's input buffer directly from the file's read buffer: */
		void* buffer;
		size_t bufferSize=thisPtr->source.readInBuffer(buffer);
		thisPtr->bytes_in_buffer=bufferSize;
		thisPtr->next_input_byte=static_cast<JOCTET*>(buffer);
		
		/* Return true if all data has been read: */
		return bufferSize!=0;
		}
	static void skipInputDataFunction(j_decompress_ptr cinfo,long count)
		{
		JPEGFileSourceManager* thisPtr=static_cast<JPEGFileSourceManager*>(cinfo->src);
		
		size_t skip=size_t(count);
		if(skip<thisPtr->bytes_in_buffer)
			{
			/* Skip inside the decompressor's read buffer: */
			thisPtr->next_input_byte+=skip;
			thisPtr->bytes_in_buffer-=skip;
			}
		else
			{
			/* Flush the decompressor's read buffer and skip in the source: */
			skip-=thisPtr->bytes_in_buffer;
			thisPtr->bytes_in_buffer=0;
			thisPtr->source.skip<JOCTET>(skip);
			}
		}
	static void termSourceFunction(j_decompress_ptr cinfo)
		{
		// JPEGFileSourceManager* thisPtr=static_cast<JPEGFileSourceManager*>(cinfo->src);
		}
	
	/* Constructors and destructors: */
	public:
	JPEGFileSourceManager(IO::File& sSource)
		:source(sSource)
		{
		/* Install the hook functions: */
		init_source=initSourceFunction;
		fill_input_buffer=fillInputBufferFunction;
		skip_input_data=skipInputDataFunction;
		resync_to_restart=jpeg_resync_to_restart; // Use default function
		term_source=termSourceFunction;
		
		/* Clear the input buffer: */
		bytes_in_buffer=0;
		next_input_byte=0;
		}
	~JPEGFileSourceManager(void)
		{
		}
	};

}

RGBImage readJPEGImage(const char* imageName,IO::File& source)
	{
	/* Create a JPEG error handler and a JPEG decompression object: */
	JPEGExceptionErrorManager jpegErrorManager;
	jpeg_decompress_struct jpegDecompressStruct;
	jpegDecompressStruct.err=&jpegErrorManager;
	jpegDecompressStruct.client_data=0;
	jpeg_create_decompress(&jpegDecompressStruct);
	
	/* Associate the decompression object with the source stream: */
	JPEGFileSourceManager jpegSourceManager(source);
	jpegDecompressStruct.src=&jpegSourceManager;
	
	RGBImage result;
	RGBImage::Color** rowPointers=0;
	try
		{
		/* Read the JPEG file header: */
		jpeg_read_header(&jpegDecompressStruct,true);
		
		/* Prepare for decompression: */
		jpeg_start_decompress(&jpegDecompressStruct);
		
		/* Create the result image: */
		result=RGBImage(jpegDecompressStruct.output_width,jpegDecompressStruct.output_height);
		
		/* Create row pointers to flip the image during reading: */
		rowPointers=new RGBImage::Color*[result.getHeight()];
		for(unsigned int y=0;y<result.getHeight();++y)
			rowPointers[y]=result.modifyPixelRow(result.getHeight()-1-y);
		
		/* Read the JPEG image's scan lines: */
		JDIMENSION scanline=0;
		while(scanline<jpegDecompressStruct.output_height)
			scanline+=jpeg_read_scanlines(&jpegDecompressStruct,reinterpret_cast<JSAMPLE**>(rowPointers+scanline),jpegDecompressStruct.output_height-scanline);
		
		/* Finish reading image: */
		jpeg_finish_decompress(&jpegDecompressStruct);
		}
	catch(std::runtime_error err)
		{
		/* Clean up: */
		delete[] rowPointers;
		jpeg_destroy_decompress(&jpegDecompressStruct);
		
		/* Wrap and re-throw the exception: */
		Misc::throwStdErr("Images::readJPEGImage: Caught exception \"%s\" while reading image \"%s\"",err.what(),imageName);
		}
	
	/* Clean up: */
	delete[] rowPointers;
	jpeg_destroy_decompress(&jpegDecompressStruct);
	
	/* Return the result image: */
	return result;
	}

}

#endif
