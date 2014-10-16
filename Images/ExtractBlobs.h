/***********************************************************************
ExtractBlobs - Function to find 8-connected sets of "foreground" pixels
in images of arbitrary pixel types.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef IMAGES_EXTRACTBLOBS_INCLUDED
#define IMAGES_EXTRACTBLOBS_INCLUDED

#include <vector>

namespace Images {

/***************************************************************
Classes for extracted blobs with optional additional properties:
***************************************************************/

template <class PixelParam>
struct Blob // Structure for blobs tracking the number of pixels inside them
	{
	/* Embedded classes: */
	public:
	typedef PixelParam Pixel; // Data type for image pixels
	
	struct Creator // Helper structure to create and modify blobs
		{
		/* Elements: */
		public:
		};
	
	/* Elements: */
	public:
	unsigned int blobId; // Unique identifier for this blob
	size_t numPixels; // Number of foreground pixels inside the blob
	
	/* Constructors and destructors: */
	Blob(unsigned int x,unsigned int y,const PixelParam& pixel,const Creator& creator)
		:blobId(~0x0U),numPixels(1)
		{
		}
	
	/* Methods: */
	void addPixel(unsigned int x,unsigned int y,const PixelParam& pixel,const Creator& creator)
		{
		++numPixels;
		}
	void merge(const Blob& other,const Creator& creator)
		{
		numPixels+=other.numPixels;
		}
	};

template <class BlobParam>
class BboxBlob:public BlobParam // Structure for blobs tracking their bounding boxes
	{
	/* Embedded classes: */
	public:
	typedef typename BlobParam::Pixel Pixel;
	typedef typename BlobParam::Creator Creator;
	
	/* Elements: */
	public:
	unsigned int bbMin[2],bbMax[2]; // Closed bounding box of blob in image space
	
	/* Constructors and destructors: */
	BboxBlob(unsigned int x,unsigned int y,const Pixel& pixel,const Creator& creator)
		:BlobParam(x,y,pixel,creator)
		{
		bbMin[0]=bbMax[0]=x;
		bbMin[1]=bbMax[1]=y;
		}
	
	/* Methods: */
	void addPixel(unsigned int x,unsigned int y,const Pixel& pixel,const Creator& creator)
		{
		BlobParam::addPixel(x,y,pixel,creator);
		bbMax[0]=x;
		}
	void merge(const BboxBlob& other,const Creator& creator)
		{
		BlobParam::merge(other,creator);
		for(int i=0;i<2;++i)
			{
			if(bbMin[i]>other.bbMin[i])
				bbMin[i]=other.bbMin[i];
			if(bbMax[i]<other.bbMax[i])
				bbMax[i]=other.bbMax[i];
			}
		}
	};

template <class BlobParam>
class BottomLeftBlob:public BlobParam // Structure for blobs tracking their bottom-left corners
	{
	/* Embedded classes: */
	public:
	typedef typename BlobParam::Pixel Pixel;
	typedef typename BlobParam::Creator Creator;
	
	/* Elements: */
	public:
	unsigned int bottomLeft[2]; // Coordinates of blob's bottom-left-most pixel
	
	/* Constructors and destructors: */
	BottomLeftBlob(unsigned int x,unsigned int y,const Pixel& pixel,const Creator& creator)
		:BlobParam(x,y,pixel,creator)
		{
		bottomLeft[0]=x;
		bottomLeft[1]=y;
		}
	
	/* Methods: */
	void merge(const BottomLeftBlob& other,const Creator& creator)
		{
		BlobParam::merge(other,creator);
		if(bottomLeft[1]>other.bottomLeft[1]||(bottomLeft[1]==other.bottomLeft[1]&&bottomLeft[0]>other.bottomLeft[0]))
			{
			for(int i=0;i<2;++i)
				bottomLeft[i]=other.bottomLeft[i];
			}
		}
	};

template <class BlobParam>
class CentroidBlob:public BlobParam // Structure for blobs tracking their unweighted centroids
	{
	/* Embedded classes: */
	public:
	typedef typename BlobParam::Pixel Pixel;
	typedef typename BlobParam::Creator Creator;
	
	/* Elements: */
	public:
	double cx,cy,cw; // Centroid accumulator
	
	/* Constructors and destructors: */
	CentroidBlob(unsigned int x,unsigned int y,const Pixel& pixel,const Creator& creator)
		:BlobParam(x,y,pixel,creator),
		 cx(double(x)),cy(double(y)),cw(1.0)
		{
		}
	
	/* Methods: */
	void addPixel(unsigned int x,unsigned int y,const Pixel& pixel,const Creator& creator)
		{
		BlobParam::addPixel(x,y,pixel,creator);
		cx+=double(x);
		cy+=double(y);
		cw+=1.0;
		}
	void merge(const CentroidBlob& other,const Creator& creator)
		{
		BlobParam::merge(other,creator);
		cx+=other.cx;
		cy+=other.cy;
		cw+=other.cw;
		}
	};

/***************************************************
Dummy class selecting foreground pixels in an image:
***************************************************/

template <class PixelParam>
class BlobForegroundSelector // Class to identify foreground pixels in an image
	{
	/* Methods: */
	public:
	bool operator()(unsigned int x,unsigned int y,const PixelParam& pixelValue) const // Returns true if the given pixel value at the given image position is a foreground pixel
		{
		return false;
		}
	};

/***********************************************************************
Dummy class checking whether two neighboring pixels can belong to the
same blob:
***********************************************************************/

template <class PixelParam>
class BlobMergeChecker // Class to check whether two pixels can be merged
	{
	/* Methods: */
	public:
	bool operator()(unsigned int x1,unsigned int y1,const PixelParam& pixelValue,unsigned int x2,unsigned int y2,const PixelParam& pixelValue2) const // Returns true if the two given pixels can be merged
		{
		return true;
		}
	};

/***************************************************************
Functions extracting blobs from images of arbitrary pixel types:
***************************************************************/

template <class BlobParam,class PixelParam,class ForegroundSelectorParam>
std::vector<BlobParam> extractBlobs(const unsigned int size[2],const PixelParam* image,const ForegroundSelectorParam& foregroundSelector,const typename BlobParam::Creator& blobCreator,unsigned int* blobIdImage =0); // Extracts blobs from the given image; if blobIdImage is !=0, creates per-pixel blob ID array

template <class BlobParam,class PixelParam,class ForegroundSelectorParam,class MergeCheckerParam>
std::vector<BlobParam> extractBlobs(const unsigned int size[2],const PixelParam* image,const ForegroundSelectorParam& foregroundSelector,const MergeCheckerParam& mergeChecker,const typename BlobParam::Creator& blobCreator,unsigned int* blobIdImage =0); // Ditto, with merge checker

}

#ifndef IMAGES_EXTRACTBLOBS_IMPLEMENTATION
#include <Images/ExtractBlobs.icpp>
#endif

#endif
