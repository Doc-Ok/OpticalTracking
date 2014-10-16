/***********************************************************************
ImageExtractorMJPG - Class to extract images from raw video frames
encoded in Motion JPEG format.
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

#include <Video/ImageExtractorMJPG.h>

#include <stdio.h>
#include <jpeglib.h>
#include <Video/FrameBuffer.h>

namespace Video {

namespace {

/*******************************************************
Helper class to read from Motion JPEG-compressed frames:
*******************************************************/

class MJPEGReader:public jpeg_source_mgr // A JPEG data source manager to read from compressed MJPEG frames
	{
	/* Private methods: */
	private:
	static void initSourceMethod(j_decompress_ptr cinfo)
		{
		/* No-op */
		}
	static boolean fillInputBufferMethod(j_decompress_ptr cinfo)
		{
		/* No-op */
		return TRUE;
		}
	static void skipInputDataMethod(j_decompress_ptr cinfo,long num_bytes)
		{
		/* Just advance the read pointer: */
		if(num_bytes>0)
			cinfo->src->next_input_byte+=num_bytes;
		}
	static void termSourceMethod(j_decompress_ptr cinfo)
		{
		/* No-op */
		}
	
	/* Constructors and destructors: */
	public:
	MJPEGReader(const FrameBuffer* frame) // Creates an MJPEG reader for the given raw frame
		{
		/* Initialize read buffer: */
		next_input_byte=frame->start;
		bytes_in_buffer=frame->used;
		
		/* Set the function pointers: */
		init_source=initSourceMethod;
		fill_input_buffer=fillInputBufferMethod;
		skip_input_data=skipInputDataMethod;
		resync_to_restart=jpeg_resync_to_restart;
		term_source=termSourceMethod;
		}
	};

}

/*******************************************
Static elements of class ImageExtractorMJPG:
*******************************************/

const unsigned char ImageExtractorMJPG::huffmanBits[4][17]=
	{
	{0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0},
	{0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
	{0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125},
	{0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119}
	};

const unsigned char ImageExtractorMJPG::huffmanValues[4][256]=
	{
	{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
	},
	{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
	},
	{
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
	0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
	0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
	},
	{
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
	0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
	}
	};

/***********************************
Methods of class ImageExtractorMJPG:
***********************************/

ImageExtractorMJPG::ImageExtractorMJPG(const unsigned int sSize[2])
	:jpegErrorManager(0),jpegStruct(0),
	 imageRows(0)
	{
	/* Copy the frame size: */
	for(int i=0;i<2;++i)
		size[i]=sSize[i];
	
	/* Initialize the JPEG decompression objects: */
	jpegErrorManager=new jpeg_error_mgr;
	jpegStruct=new jpeg_decompress_struct;
	jpeg_create_decompress(jpegStruct);
	jpegStruct->err=jpeg_std_error(jpegErrorManager);
	
	/* Create the standard Motion JPEG Huffman tables: */
	for(int tableIndex=0;tableIndex<4;++tableIndex)
		{
		/* Allocate the table: */
		JHUFF_TBL* huffmanTable=jpeg_alloc_huff_table(reinterpret_cast<jpeg_common_struct*>(jpegStruct));
		if(tableIndex<2)
			jpegStruct->dc_huff_tbl_ptrs[tableIndex]=huffmanTable;
		else
			jpegStruct->ac_huff_tbl_ptrs[tableIndex-2]=huffmanTable;
		
		/* Copy Huffman bits and values: */
		int numValues=0;
		for(int i=1;i<=16;++i)
			numValues+=huffmanTable->bits[i]=huffmanBits[tableIndex][i];
		for(int i=0;i<numValues;++i)
			huffmanTable->huffval[i]=huffmanValues[tableIndex][i];
		}
	
	/* Allocate the image row array: */
	imageRows=new unsigned char*[size[1]];
	}

ImageExtractorMJPG::~ImageExtractorMJPG(void)
	{
	/* Delete the image row array: */
	delete[] imageRows;
	
	/* Destroy the JPEG decompression objects: */
	if(jpegStruct!=0)
		jpeg_destroy(reinterpret_cast<jpeg_common_struct*>(jpegStruct));
	delete jpegStruct;
	delete jpegErrorManager;
	}

void ImageExtractorMJPG::extractGrey(const FrameBuffer* frame,void* image)
	{
	/* Attach a JPEG source to the raw frame: */
	MJPEGReader mjr(frame);
	jpegStruct->src=&mjr;
	
	/* Read the abbreviated image file header: */
	jpeg_read_header(jpegStruct,true);
	
	/* Set the decompressor's output color space to Y'CbCr: */
	jpegStruct->out_color_space=JCS_YCbCr;
	
	/* Prepare the decompressor: */
	jpeg_start_decompress(jpegStruct);
	
	/* Create a temporary image array and set the image row pointers: */
	unsigned char* tempImage=new unsigned char[size[0]*size[1]*3];
	for(unsigned int y=0;y<size[1];++y)
		imageRows[y]=tempImage+y*size[0]*3;
	
	/* Decompress the video frame: */
	unsigned int numLines=0;
	while(numLines<size[1])
		numLines+=jpeg_read_scanlines(jpegStruct,reinterpret_cast<JSAMPLE**>(imageRows+numLines),size[1]-numLines);
	
	/* Finish decompression: */
	jpeg_finish_decompress(jpegStruct);
	jpegStruct->src=0;
	
	/* Convert the frame's Y' channel to Y: */
	const unsigned char* rRowPtr=tempImage;
	unsigned char* gRowPtr=static_cast<unsigned char*>(image);
	gRowPtr+=(size[1]-1)*size[0];
	for(unsigned int y=0;y<size[1];++y,rRowPtr+=size[0]*3,gRowPtr-=size[0])
		{
		const unsigned char* rPtr=rRowPtr;
		unsigned char* gPtr=gRowPtr;
		for(unsigned int x=0;x<size[0];++x,++gPtr,rPtr+=3)
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
	
	/* Clean up: */
	delete[] tempImage;
	}

void ImageExtractorMJPG::extractRGB(const FrameBuffer* frame,void* image)
	{
	/* Attach a JPEG source to the raw frame: */
	MJPEGReader mjr(frame);
	jpegStruct->src=&mjr;
	
	/* Read the abbreviated image file header: */
	jpeg_read_header(jpegStruct,true);
	
	/* Prepare the decompressor: */
	jpeg_start_decompress(jpegStruct);
	
	/* Set the image row pointers to flip the image vertically: */
	for(unsigned int y=0;y<size[1];++y)
		imageRows[y]=reinterpret_cast<unsigned char*>(image)+(size[1]-1-y)*size[0]*3;
	
	/* Decompress the video frame: */
	unsigned int numLines=0;
	while(numLines<size[1])
		numLines+=jpeg_read_scanlines(jpegStruct,reinterpret_cast<JSAMPLE**>(imageRows+numLines),size[1]-numLines);
	
	/* Finish decompression: */
	jpeg_finish_decompress(jpegStruct);
	jpegStruct->src=0;
	}

void ImageExtractorMJPG::extractYpCbCr420(const FrameBuffer* frame,void* yp,unsigned int ypStride,void* cb,unsigned int cbStride,void* cr,unsigned int crStride)
	{
	/* Attach a JPEG source to the raw frame: */
	MJPEGReader mjr(frame);
	jpegStruct->src=&mjr;
	
	/* Read the abbreviated image file header: */
	jpeg_read_header(jpegStruct,true);
	
	/* Set the decompressor's output color space to Y'CbCr: */
	jpegStruct->out_color_space=JCS_YCbCr;
	
	/* Prepare the decompressor: */
	jpeg_start_decompress(jpegStruct);
	
	/* Create a temporary image array and set the image row pointers: */
	unsigned char* tempImage=new unsigned char[size[0]*size[1]*3];
	for(unsigned int y=0;y<size[1];++y)
		imageRows[y]=tempImage+y*size[0]*3;
	
	/* Decompress the video frame: */
	unsigned int numLines=0;
	while(numLines<size[1])
		numLines+=jpeg_read_scanlines(jpegStruct,reinterpret_cast<JSAMPLE**>(imageRows+numLines),size[1]-numLines);
	
	/* Finish decompression: */
	jpeg_finish_decompress(jpegStruct);
	jpegStruct->src=0;
	
	/* Downsample the image to 4:2:0: */
	unsigned char* framePtr=tempImage;
	unsigned char* ypRowPtr=static_cast<unsigned char*>(yp);
	unsigned char* cbRowPtr=static_cast<unsigned char*>(cb);
	unsigned char* crRowPtr=static_cast<unsigned char*>(cr);;
	for(unsigned int y=0;y<size[1];y+=2)
		{
		/* Process an even row by keeping its Cb values: */
		unsigned char* ypPtr=ypRowPtr;
		unsigned char* cbPtr=cbRowPtr;
		for(unsigned int x=0;x<size[0];x+=2)
			{
			/* Get Yp and Cb from even pixel: */
			*(ypPtr++)=*(framePtr++);
			*(cbPtr++)=*(framePtr++);
			++framePtr;
			
			/* Get Yp from odd pixel: */
			*(ypPtr++)=*(framePtr++);
			++framePtr;
			++framePtr;
			}
		ypRowPtr+=ypStride;
		cbRowPtr+=cbStride;
		
		/* Process an odd row by keeping its Cr values: */
		ypPtr=ypRowPtr;
		unsigned char* crPtr=crRowPtr;
		for(unsigned int x=0;x<size[0];x+=2)
			{
			/* Get Yp from even pixel: */
			*(ypPtr++)=*(framePtr++);
			++framePtr;
			++framePtr;
			
			/* Get Yp and Cr from odd pixel: */
			*(ypPtr++)=*(framePtr++);
			++framePtr;
			*(crPtr++)=*(framePtr++);
			}
		ypRowPtr+=ypStride;
		crRowPtr+=crStride;
		}
	
	/* Clean up: */
	delete[] tempImage;
	}

}
