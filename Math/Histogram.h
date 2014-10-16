/***********************************************************************
Histogram - Class to create histograms of values of arbitrary scalar
types.
Copyright (c) 2014 Oliver Kreylos

This file is part of the Templatized Math Library (Math).

The Templatized Math Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Math Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Math Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef MATH_HISTOGRAM_INCLUDED
#define MATH_HISTOGRAM_INCLUDED

#include <stddef.h>
#include <Math/Constants.h>

namespace Math {

template <class ScalarParam>
class Histogram
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Type for scalars inserted into histogram
	
	/* Elements: */
	private:
	Scalar binSize; // Size of a histogram bin in microseconds
	Scalar minBinValue; // Minimum value for which to create a bin
	Scalar maxBinValue; // Maximum value for which to create a bin
	size_t numBins; // Number of bins in the histogram
	size_t* bins; // Array of histogram bins
	size_t numSamples; // Number of samples in current observation period
	double valueSum; // Sum of all values in current observation period to calculate average
	Scalar minValue,maxValue; // Value range in current observation period
	size_t maxBinSize; // Maximum number of samples in any bin
	
	/* Private methods: */
	size_t getBinIndex(Scalar value) const
		{
		if(value<minBinValue)
			return 0; // Put value in negative outlier bin
		if(value>maxBinValue)
			return numBins-1; // Put value in positive outlier bin
		return size_t((value-minBinValue)/binSize)+1;
		}
	
	/* Constructors and destructors: */
	public:
	Histogram(Scalar sBinSize,Scalar sMinBinValue,Scalar sMaxBinValue)
		:binSize(sBinSize),minBinValue(sMinBinValue),maxBinValue(sMaxBinValue),
		 numBins(size_t((maxBinValue-minBinValue)/binSize)+3),bins(new size_t[numBins])
		{
		/* Initialize the histogram: */
		reset();
		}
	private:
	Histogram(const Histogram& source); // Prohibit copy constructor
	Histogram& operator=(const Histogram& source); // Prohibit assignment operator
	public:
	~Histogram(void)
		{
		delete[] bins;
		}
	
	/* Methods: */
	Scalar getBinMin(size_t binIndex) const // Returns the lower bound of the given bin
		{
		if(binIndex==0)
			return Math::Constants<Scalar>::min;
		else
			return minBinValue+binSize*Scalar(binIndex-1);
		}
	Scalar getBinMax(size_t binIndex) const // Returns the upper bound of the given bin
		{
		if(binIndex==numBins-1)
			return Math::Constants<Scalar>::max;
		else
			return minBinValue+binSize*Scalar(binIndex);
		}
	void reset(void) // Resets the histogram for the next observation period
		{
		/* Clear the histogram: */
		for(size_t i=0;i<numBins;++i)
			bins[i]=0;
		
		/* Reset the latency counter and range: */
		numSamples=0;
		valueSum=0.0;
		minValue=Math::Constants<Scalar>::max;
		maxValue=Math::Constants<Scalar>::min;
		maxBinSize=0U;
		}
	void addSample(Scalar value) // Adds a sample into the histogram
		{
		/* Update the histogram: */
		size_t binIndex=getBinIndex(value);
		++bins[binIndex];
		if(maxBinSize<bins[binIndex])
			maxBinSize=bins[binIndex];
		
		/* Update sample counter and range: */
		++numSamples;
		valueSum+=double(value);
		if(minValue>value)
			minValue=value;
		if(maxValue<value)
			maxValue=value;
		}
	size_t getNumSamples(void) const
		{
		return numSamples;
		}
	size_t getMaxBinSize(void) const // Returns the maximum number of samples in any bin
		{
		return maxBinSize;
		}
	size_t getFirstBinIndex(void) const // Returns the index of the first bin that contains samples
		{
		return getBinIndex(minValue);
		}
	size_t getLastBinIndex(void) const // Returns the index of the last bin that contains samples
		{
		return getBinIndex(maxValue);
		}
	size_t getBinSize(size_t binIndex) const // Returns the number of samples in the given bin
		{
		return bins[binIndex];
		}
	Scalar getAverageValue(void) const // Returns the arithmetic average of all samples in the histogram
		{
		return Scalar(valueSum/double(numSamples));
		}
	};

}

#endif
